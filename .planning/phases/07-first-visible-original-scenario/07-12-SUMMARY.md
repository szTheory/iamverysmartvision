---
phase: 07-first-visible-original-scenario
plan: 12
subsystem: Phase 7 close-out (W7) - claims CL-0023 and CL-0024, the play-word tripwire, true public text, Decision Register, verification record and cost close
tags: [W7, SCN-01, SCN-02, claims, tripwire, text-step, README, release-content, D4, D11, D9, verification-record, phase7-close, V14, V20, T-07-30, T-07-31]
status: complete

requires:
  - phase: 07-first-visible-original-scenario
    provides: "plan 07-11: gate steps=36/36 controls=356/356 faults=222/222 claims=22 provisional=21 tests-skipped=4 tests-expected=4624 gates=77, HEAD f71572e at the start of this plan; the derivation section (h) wording, the pass scenario and its expectation, the fourth installed-consumer replay, the owner-run route"
provides:
  - "data/claims.json and data/expected/claims.ids: CL-0023 (tier playable-guest-progress, provisional, artifact data/golden/demo-lamp-tone-pass.expect, the only allowed wording 'playable guest progress, for this original program only', contract 2 with $5000 and $5004, exclusions printed in the note) and CL-0024 (tier installed-host-behavior, provisional, artifact tests/installed-consumer/main.c)"
  - "tools/check/steps/text.sh: plays and playable are claim words; the exact string playable-guest-progress is exempt (removed from a line before the match, only for files that contain it); control text-untagged-playable (controls 356 to 357)"
  - "README, docs/evidence-tiers.md, docs/packaging.md (Release content), PROJECT.md (D4, D10, D11 rows, Phase 7 notes, stale licence text), REQUIREMENTS.md and ROADMAP.md (REL-01, D4, D11)"
  - "07-VERIFICATION.md (V1 to V20, H7-1 to H7-5, caveats, oracle order) and the CI-EFFICIENCY close entry ending phase7-close: no-lever"
affects: [phase-8]

actuals:
  tokens: 36000
  tasks: 3
  commits: 3
plan_head_before: f71572e59a448f0b911b8d2eaf45db26df386932
plan_head_after: e503f608402144a78d9bf1a1fc27ac6f83d3d070

key-decisions:
  - "The tier identifier exemption is by exact string removal before the match, and only files that contain the identifier pass through sed (one grep -l over all scanned files finds them): a first variant ran sed on every file and cost about +0.2 local cpu-s per gate; the final form costs about +0.06"
  - "The derivation line that states CL-0023 carries the bare tag [claim:CL-0023] instead of [nc] now that the claim exists; the freeze command strips both forms, so sections (a) to (j) and the F-S literals are unchanged"
  - "SCN-01 and SCN-02 are marked complete in REQUIREMENTS.md and the Phase 7 line of ROADMAP.md in the final docs commit, after the verification record and the three final gates exist, because the plan conditions the marks on every V-row being satisfied; SCN-02 is complete on documented-route terms only (D11 open, not performed, no claim, no tier)"
  - "D10 and D11 got rows in the PROJECT.md Decision Register (the register had none) so that 'the D11 row still reads open' has a row to read"
  - "phase7-close: no-lever, because the two raw triggers that fire on the newest hosted row (gate 146 s, cpu median 335) fire at the slowest build_s of the file while the last-five median r is 7.78, inside the budget 7.8: a runner-speed event by the rule of the Phase 6 start entry"

key-files:
  created:
    - tools/check/controls/text-untagged-playable/CHECK
    - tools/check/controls/text-untagged-playable/EXPECT
    - tools/check/controls/text-untagged-playable/FORBID
    - tools/check/controls/text-untagged-playable/overlay/docs/x.md
    - .planning/phases/07-first-visible-original-scenario/07-VERIFICATION.md
    - .planning/phases/07-first-visible-original-scenario/07-12-SUMMARY.md
  modified:
    - data/claims.json
    - data/expected/claims.ids
    - data/expected/controls/claims.ids
    - tools/check/steps/text.sh
    - docs/video/demo-lamp-tone-1-derivation.md
    - README.md
    - docs/evidence-tiers.md
    - docs/packaging.md
    - .planning/PROJECT.md
    - .planning/REQUIREMENTS.md
    - .planning/ROADMAP.md
    - .planning/STATE.md
    - .planning/CI-EFFICIENCY.md

requirements-completed: [SCN-01, SCN-02]

completed: 2026-11-13
---

# Phase 7 Plan 12: close-out Summary

**Phase 7 closes with two provisional claims (CL-0023 `playable-guest-progress` and CL-0024 `installed-host-behavior`) in the only allowed wording, a `plays|playable` tripwire with a control that keeps the word out of public text unless it is the allowed phrase with a claim tag or the tier identifier, README and planning text that say what exists and what does not (including the source-only release posture and the D4, D10, D11 and licence records), a verification record with V1 to V20 mapped to the commit that landed each check, and a cost close of `phase7-close: no-lever` on a last-five hosted median `r` of 7.78; the final gates pass under sh, dash and the Ubuntu container with `steps=36/36 controls=357/357 faults=222/222 claims=24 provisional=23 tests-skipped=4 tests-missing=0`.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `9995f60` | 1 (tracer) | CL-0023 and CL-0024, `plays|playable` in `text.sh` with the identifier exemption, control `text-untagged-playable`, one bare claim tag on the derivation's CL-0023 line |
| `eaf713c` | 2 | README, `docs/evidence-tiers.md`, `docs/packaging.md` (Release content), PROJECT (D4, D10, D11, Phase 7 notes, stale licence text), REQUIREMENTS and ROADMAP (REL-01, D4, D11 rows) |
| `e503f60` | 3 | `07-VERIFICATION.md` and the CI-EFFICIENCY close entry (`phase7-close: no-lever`) |
| the commit that adds this file | | this SUMMARY, STATE.md, ROADMAP.md, REQUIREMENTS.md (SCN-01 and SCN-02 complete) |

## What was built

- **Claims (D-27, A-07).** The next free ids were re-checked (`CL-0022` was the last; the new ones are `CL-0023` and `CL-0024`). CL-0023 states, from derivation section (h), the project boot stub contract 2 (`boot-stub-2`, reset entry `$5000`, interrupt entry `$5004`), the project's zero graphics ROM and no EXEC or GROM image of any console, the scripted input of the pass scenario, the hand-derived markers, frames F-S1 to F-S6 and the lamp rectangles, the write log and the constant sound levels, the idle literals, and ends with the exact sentence "This is playable guest progress, for this original program only." `depends_on` lists 37 profile fields (controller, STIC, PSG, memory-map and video fields as CL-0014 to CL-0022 list them), so the weakest-field rule forces `provisional` (`claims=24 provisional=23`). The note prints the evidence (the 55-assert pass scenario, the idle, shift and swap contrasts, the pair judge, the shape lint, 12 mutants and one negative row, `run.stub2.*`) and the exclusions (hand derived by the author of the program and the core with one blind read; tier 4 and inference fields; index frames, not pictures; no person looked or listened; no timing claimed; no commercial content and no real EXEC or GROM, D11 open; nothing measured, D10; Phase 8 features not covered), and states the order caveat exactly (the four assert blocks were run once as scratch scenarios in plan 07-06, before the `.expect` files entered the repository in plan 07-07, with no later change). CL-0024 states the fourth installed-consumer replay against the runner's digests, domains, region differences, write-log records and audio levels, with the same self-recorded-golden caveat as CL-0022.
- **Tripwire (D-27).** `tools/check/steps/text.sh` matches `plays` and `playable` besides the six earlier claim words. The tier identifier `playable-guest-progress` is not a claim word: that exact string is removed (by `sed`, which keeps line numbers) from a line before it is matched, and only for the files that one `grep -l` over all scanned files finds to contain it. The control `text-untagged-playable` (toy `docs/x.md`: line 1 `playable`, line 2 the identifier, line 3 `plays`, line 4 `playable` with `[nc]`) expects findings for lines 1 and 3 and forbids findings for lines 2 and 4. The sweep of README and `docs/` found no untagged use: every existing hit was already the identifier or carried `[nc]` (derivation line 318, `docs/runner.md` lines 102 and 234, `docs/evidence-tiers.md` line 45), and the derivation line that states CL-0023 got a bare `[claim:CL-0023]` in place of `[nc]` (the freeze command strips both).
- **Public text (D-33).** README no longer says that no machine code or release exists: it states the core, the runner, the installed consumer and the first original program as tracked source with `[claim:CL-0023]` and `[claim:CL-0024]` and the exclusions, that no release has been published, that a release ships the program only as tracked source and that neither the runner nor `ivs_pack` nor `ivs_asm` is installed (checked: `core/CMakeLists.txt` installs only the archive, headers, package files and `LICENSE`), and where the owner-local route is. `docs/packaging.md` has a section "Release content" for the next release notes; `docs/evidence-tiers.md` says what the new tier does not mean and lists the claim words.
- **Records.** PROJECT.md: the D4 row carries the S-3 wording and the reversal evidence; new D10 and D11 rows (open, not performed; SCN-02 complete on documented-route terms, no claim and no tier); a Phase 7 notes block (D4 re-evaluated and decided with the effort data: 1,137 lines of C, 304 pairs, four twins, the program's 110 statements with zero errors and zero escapes; D5 extended by contract 2; SCN-01 and the first playable-tier claim; SCN-02 and D11; D9 applied to stale text); the Out of Scope line, the owner-gate row and open question 11 no longer call the licence an owner gate (MIT, D9); the "keep one playable scenario visible" recommendation is marked done. REQUIREMENTS.md: REL-01 says publication follows the delegated flow (D9), D4 and D11 rows updated. ROADMAP.md: D4 and D11 rows of the decision-gate table.
- **Verification record and cost close.** `07-VERIFICATION.md` has the success criteria mapped to evidence, 20 V-rows with check, failing direction and the landing commit, the requirements table, the oracle-order table (oracle `4e0febe`, record `1621dae`, program `949407e`, scenarios `8cb4329`, replay `da07663`, ancestry checked), the hand-off table H7-1 to H7-5 (not evidence, not performed), caveats and the items carried to Phase 8. The CI-EFFICIENCY entry carries the counts, the local gate figures, the hosted rows (last five: 43365038335, 43369159094, 43373151931, 43528527276 and 43540936641; median `r` 7.7833, gate median 107, cpu median 335), the checkpoint lines of plans 07-08 and 07-09, the Phase 7 cost story and one line `phase7-close: no-lever`.

## Checks shown able to fail

- **`text-untagged-playable`**: run against the step as it was before this plan (`git show f71572e:tools/check/steps/text.sh`), the control's tree gives `-- text pass` and exit 0, which the control runner reports as `control-exit-zero`; run against a variant that keeps the new words but drops the identifier exemption, the tree gives findings for lines 1, 2 and 3, which the FORBID line (`docs/x.md:2`) turns into `control-forbidden-token`. With the final step it ends `controls=357/357`.
- **The identifier exemption and the other text controls** under sh and dash: `text-untagged-claim-word`, `text-nul-untagged-claim-word` (a document with a NUL byte), `text-missing-claim-ref` keep their outputs; the container gate (GNU sed and grep) passes the step.
- **Claims**: the `claims` step counts `claims=24 provisional=23`; the earlier controls (`claims-overstated-standing`, `claims-dangling-artifact`, `claims-unknown-field`) already police an established standing on a weak field, a missing artifact and an unknown field name, and the new claims use only existing profile field ids.
- **The oracle freeze command** (plan Task 1 and Task 3 verify) passed on the Task 1 tree and on the final tree.

## Deviations from Plan

**1. [Interpretation, order] SCN-01, SCN-02 and the ROADMAP Phase 7 completion are in the final docs commit, not in Task 2.** The plan lists the requirement marks and the roadmap completion in Task 2 but conditions them on every V-row of the verification record being satisfied; the record is written in Task 3 and the V-rows hold only when the final gates pass. Task 2 therefore leaves SCN-01 and SCN-02 unchecked and the Phase 7 line open; the closing commit (after the three final gates) marks them with a close-out note in REQUIREMENTS.md in the form of the Phase 5 and 6 notes.

**2. [Rule 2 - missing record] D10 and D11 rows added to the PROJECT.md register.** The plan speaks of "the D11 row" of PROJECT.md; the register had none (D11 appears only in REQUIREMENTS.md and ROADMAP.md). The two rows were added so the open state has a row in the project record, and the REQUIREMENTS.md and ROADMAP.md D11 rows were reworded (still open, not performed).

**3. [Interpretation] The derivation's CL-0023 line carries `[claim:CL-0023]` instead of `[nc]`.** The plan allows either bare tag; the claim exists now and the line is its statement, so the claim tag is the accurate one. No wording changed.

**4. [Process] The container gate ran three times.** The first run (the first text-step variant, `sed` on every file) was stopped when the step was reworked for cost (it had passed `claims` and `text` and had no failure up to the `sources` step); the second ran on the Task 1 tree with Task 2's files parked (copies kept outside the repository, restored byte-identical, md5 checked), so Task 1 was verified at its own commit state; the third is the final gate of Task 3.

**5. [Cost] The text step was reworked once.** The first form of the exemption ran `sed` on every scanned file (+0.2 local cpu-s per gate); the final form runs it only on the files that contain the identifier (+0.06).

No Rule 1 bug fixes; no auth gates; no package installs (T-07-SC); `tools/asm/asm.c` and `tools/pack/pack.c` are untouched (no commit since 290f057), so no recipe digest was refreshed; the oracle (derivation sections (a) to (j), the F-S literals, the pair inventory) is unchanged apart from the one bare tag.

## Decisions Made

- The only allowed wording for the new tier is "playable guest progress, for this original program only" with a claim tag on the same line; the tier identifier alone is exempt, and the exemption is documented in the step header and in `docs/evidence-tiers.md`.
- The release statement lives in `docs/packaging.md` (the next release notes copy it); nothing in it is a claim of its own.
- D4's reversal evidence is recorded as absent (zero escapes, 1,137 of 1,200 lines), so the decision stands.
- `phase7-close: no-lever` on the rule as written; lever (2) stays queued, with the thin margin to the budget recorded.

## Open Items

- **Orchestrator**: record the hosted row of the head of plan 07-12 after the push (`tools/ci-record.sh`, never called here); re-read the last five. The projection is 7.79 against a budget of 7.8 and a trigger of 8.3, and a row at a normal runner speed is needed to tell whether 7.78 is the level or a high reading.
- **Optional (07-09)**: an erratum note in derivation section (k) that the section (d) gate-window starts (124,132 and 721,848) are reference-sequence figures and the as-built starts are 56 ticks later. Not done: no literal or assert depends on it, and the oracle stays frozen apart from claim tags.
- **Optional**: the SKIPPED inventory line does not carry the "D11, no claim" wording (07-10 and 07-11 open item; the helper's line and the page do); the symlink gap of the owner-run helper (07-11).
- **Owner**: H7-1 to H7-5 of the verification record (listen, view frames, D10, the D11 run, publication); none is evidence.
- **Phase 8**: VID-05, STA-01, STA-02, PRF-01, REL-01.
- Carried unchanged: Phase 4 and 5 review deferrals, `sh tools/hygiene.sh --history` on earlier planning commits.

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=36/36 controls=357/357 faults=222/222 ... claims=24 provisional=23 ... gates=77 lines=264 ... tracked=2252 ... tests-expected=4624 tests-executed=4620 tests-skipped=4 tests-missing=0 (0 FAIL lines, four -- SKIPPED lines; gate-time total=287 s, gate-cpu user=183.3 sys=181.2, machine load average 23 to 58 with the container gate running at the same time)
dash tools/gate.sh:    the identical summary line (md5 efcfa9896f521476000f59c4b8585b66 for both shells, gate-time and gate-cpu removed; 0 FAIL lines; total=386 s)
sh tools/gate-linux.sh: exit 0 on the same tree (build/gate-linux.log ends gate: PASS; gcc 13.3.0, cmake 3.28.3), steps=36/36 controls=357/357 faults=222/222 claims=24 provisional=23 gates=77 lines=264 tests-expected=4624 tests-executed=4620 tests-skipped=4 tests-missing=0, 0 FAIL lines, four -- SKIPPED lines (gate-time total=674 s, emulated amd64 on a loaded host)
```

On the closing tree (SUMMARY, STATE, ROADMAP, REQUIREMENTS and the verification status edited; `tracked=2253`): sh (158 s) and dash (155 s) `gate: PASS` with the identical summary line (md5 980d77ba8e42f7814448b646f9ce9e68), 0 `FAIL` lines.

After Task 1 (before its commit, Task 2's files parked): sh, dash and the container gate `gate: PASS` with `controls=357/357 claims=24 provisional=23` (sh and dash md5 f623b7d8b8ee26046775922db6cbd797, container 429 s). After Task 2 (before its commit): sh and dash identical (md5 efcfa9896f521476000f59c4b8585b66).

Counts at plan start were `controls=356/356 claims=22 provisional=21 tests-skipped=4 tests-expected=4624 gates=77 lines=264`; at the end `controls=357/357 claims=24 provisional=23`, everything else equal. `git ls-files '*.ivsc' '*.bin' '*.rom' '*.exec' '*.wav' '*.ppm' '*.vraw'` is empty; `sh tools/hygiene.sh --tree` passes (`hygiene-files=2252`).

## Known Stubs

None. The claims, tripwire, text and records are complete for the plan's scope.

## Threat Flags

None new. T-07-30 (overclaim): both claims are provisional by the weakest-field rule, the notes print the exclusions, the only allowed phrase is checked by the `plays|playable` tripwire with its control, and the artifacts exist. T-07-31 (stale or false public statements): README, PROJECT.md and REQUIREMENTS.md no longer misstate the licence, the release or D11; D11 is recorded open and not performed; the hand-off table lists what only the owner can do. T-07-SC: no installs.

## Self-Check: PASSED

`data/claims.json` has `CL-0023` and `CL-0024` (`grep -c '"CL-0023"'` prints 1) and `data/expected/claims.ids` lists them; `tools/check/controls/text-untagged-playable/` exists and is registered in `data/expected/controls/claims.ids`; `07-VERIFICATION.md` exists with 20 rows starting `| V` and one `H7-4`; `.planning/CI-EFFICIENCY.md` has exactly one line starting `phase7-close: `; commits `9995f60`, `eaf713c` and `e503f60` are ancestors of HEAD (`git rev-list --count f71572e59a448f0b911b8d2eaf45db26df386932..HEAD` printed 3 before this file's commit); `git status --short` showed only `.planning/config.json` and `.planning/state.json` (not staged) before this commit; no file name contains the console brand word; no absolute home, scratch or temp path and no EXEC or GROM digest is in the tracked diff.
