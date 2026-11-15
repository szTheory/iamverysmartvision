---
phase: 08-stic-completion-save-state-performance-baselines
plan: 08
subsystem: Phase 8 wave A, plan 6 - the close-out of the STIC completion - claims CL-0025 to CL-0027 (provisional, with written exclusions) and the regenerated CHANGELOG table, VID-05 accepted by the important-gates registry with 18 lines over six gates, the final discrepancy evidence and the own-bit pin, the VID-05 completion on recorded terms and the checkpoint A cost line
tags: [A, VID-05, claims, important-gates, discrepancies, requirements, checkpoint-A, D-02, D-09, D-10, D-36, D-37, D-43, T-08-22, T-08-23]
status: complete

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-07 end state (the program ivs-diag-stic-2, six scenarios, two pair lines, six Release-only mutants); the frozen oracle of plan 08-03 (docs/video/diag-stic-2-derivation.md sections (a) to (j), frames F-M0..F-M9, blind-read record, frozen since 9250265); counts at plan start steps=37/37 controls=371/371 faults=226/226 tests-expected=4772 tests-executed=4768 tests-skipped=4 claims=24 provisional=23 gates=77 lines=264 discrepancies=110 goldens=27 profile-keys=154 (re-measured with a full gate run at the start, identical)"
provides:
  - "data/claims.json CL-0025 (pictures, source-expectation, tests/video/video_mob.c), CL-0026 (collision, source-expectation, tests/video/video_collision.c), CL-0027 (the diagnostic, original-diagnostic, fixtures/FX-diag-stic-2/diag-stic-2.listing.txt): all provisional, each note with the oracle and blind-read record, the shared-misreading risk and the Excluded on purpose list; claims.ids; the CHANGELOG claim table regenerated in the same commit (claims=27 provisional=26)"
  - "tools/check/steps/important-gates.sh accepts VID-05; data/expected/important-gates.ids gains 18 lines over six gates (vid05-mob-faults, vid05-collision-fault, vid05-delay-fault, vid05-judges, vid05-program-mutants, vid05-frame-grammar); the toy registry of the known-good tree and nine overlay registries carry one valid toy VID-05 line; important-gates-bad-req-phase6 uses the never-valid id VID-99 on its line 24 (gates=83 lines=282)"
  - "data/discrepancies.json: final evidence of C107 to C110, C87, C92, C105 and C81 (profile fields, default-pinned ids with their flips, frames, faults, D10 hand-offs, claim exclusions); the own bit on a CPU write as an explicit unknown-default-chosen pin; collision on the undelayed scene recorded; statuses and resolvers unchanged"
  - ".planning/REQUIREMENTS.md VID-05 complete with the Phase 8 wave A close-out note; .planning/CI-EFFICIENCY.md checkpoint A entry ending checkpointA-08: lever-required"
affects: [08-09, 08-10, 08-11, 08-12, 08-13, 08-14, 08-15, 08-20, 08-21]

actuals:
  tokens: 12623
  tasks: 3
  commits: 3
plan_head_before: 8e67c2aa7aefd3bb8f64356dc89cc8c52717e430
plan_head_after: 88e6abe8f77955b5da69da618469f9c2594fc427

key-decisions:
  - "Own bit on a CPU write (orchestrator decision, carried from 08-03 and 08-05): the source says it reads 0 (SRC-community-emu-docs@programming-stic-mob-controls) but the default keeps the write mask 03FF, so a written own bit is stored and read back; the frame writer never sets it. It is worded as an explicit unknown-default-chosen pinned default in the STIC-R44 note, C87, the reference notes and an additive amendment for PD-7; the frozen Phase 6 regmap literal and every test are unchanged and no scenario or frame depends on it (the program writes only 0)"
  - "Collision on the undelayed scene (orchestrator decision, 08-06 open item): a claim exclusion of CL-0026 and a statement in C110; it is a choice of plan 08-06, not a pin of the oracle, and the owner hand-off D10-VID-12 now also names it"
  - "The frame counter at 02F1 relies on the zero power-on RAM default of the profile (orchestrator decision, 08-07 open item): an assumption written into the CL-0027 note, with power_on_fill.ram in its depends_on; specification A5 is not changed"
  - "The three video.*delay* field names stay exempt from the glossary unit-suffix rule (orchestrator confirmation of the 08-06 deviation); nothing was renamed"
  - "checkpoint A reads lever-required: the projection by the rule is 8.06 to 8.18 against the line 8.0 of the orchestrator's decision (median 7.78, budget 7.8, revisit line 8.3); the line is the rule's first answer and the hosted row of the wave A head is the arbiter"

key-files:
  created:
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-08-SUMMARY.md
  modified:
    - data/claims.json
    - data/expected/claims.ids
    - CHANGELOG.md
    - docs/evidence-tiers.md
    - tools/check/steps/important-gates.sh
    - data/expected/important-gates.ids
    - tools/check/controls/_good/tree/data/expected/important-gates.ids
    - tools/check/controls/important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-bad-req-phase6/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-req-uncovered-phase5/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-req-uncovered-phase6/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-req-uncovered-phase7/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-uncovered-gate/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-unknown-id-phase5/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-unknown-id-phase6/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-unknown-id/overlay/data/expected/important-gates.ids
    - data/discrepancies.json
    - data/stic/rules.json
    - data/expected/d10-measurements.ids
    - docs/stic/d10-procedure.md
    - docs/stic/reference-notes.md
    - docs/video/diag-stic-2-derivation.md
    - .planning/REQUIREMENTS.md
    - .planning/CI-EFFICIENCY.md
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: [VID-05]

completed: 2026-11-14
---

# Phase 8 Plan 08: the close-out of the STIC completion Summary

**Wave A is closed on recorded terms. The pictures of the MOB, border-extension and delay scenarios (F-M0 to F-M6), the sixteen collision words of the truth table and the original program `ivs-diag-stic-2` with its six scenarios are three provisional claims (CL-0025 and CL-0026 at `source-expectation`, CL-0027 at `original-diagnostic`), each with a written `Excluded on purpose` list that names every pinned item, the own bit on a CPU write, collision on the undelayed scene and the zero power-on RAM assumption of the frame counter; the CHANGELOG table was regenerated in the same commit. VID-05 is accepted by the important-gates registry through 18 lines over six gates (four core faults, four frame judges, six program mutants, four frame-grammar controls) and the control that used VID-05 as its never-valid id fails for the same reason with VID-99. The evidence of C107 to C110, C87, C92, C105 and C81 now names the profile field, the `default-pinned` test with its flip, the frame, the guarding fault, the D10 hand-off (none performed) and the claim exclusion of every pin, VID-05 is marked complete in REQUIREMENTS on stated terms (no claim above provisional, no tier above `source-expectation` and `original-diagnostic`), and the checkpoint A entry of `.planning/CI-EFFICIENCY.md` ends `checkpointA-08: lever-required` (median `r` 7.78, projection 8.06 to 8.18). No C, scenario, golden, frame or derivation line (a) to (j) changed. The final gates pass under sh, dash and the Ubuntu container with `steps=37/37 controls=371/371 faults=226/226 fault-runs=226 tests-expected=4772 tests-executed=4768 tests-skipped=4 tests-missing=0 claims=27 provisional=26 gates=83 lines=282 discrepancies=110 open=101 pending-resolvers=20 d10-items=47 stic-rules=67 video-frames=26 video-frame-pairs=9 video-hash-ties=18 goldens=27 profile-keys=154`. REL-01 stays Pending.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `07cd417` | 1 (tracer) | CL-0025, CL-0026, CL-0027 in `data/claims.json` and `data/expected/claims.ids`, the CHANGELOG claim table regenerated, `docs/evidence-tiers.md` lists the three claims (claims 24 to 27, provisional 23 to 26) |
| `de7d5cb` | 2 | VID-05 in the `nreq` list and header of `important-gates.sh`, 18 registry lines, one toy VID-05 line in the known-good registry and in nine overlay registries, `important-gates-bad-req-phase6` retargeted to VID-99 on line 24 (gates 77 to 83, lines 264 to 282) |
| `88e6abe` | 3 | discrepancy finals, the own-bit pin (R44 note, C87, derivation amendment, reference notes, D10 text), VID-05 complete in REQUIREMENTS with its close-out note, the checkpoint A entry |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md |

## The three claims (Task 1)

| Claim | Tier | Artifact | Rests on |
|-------|------|----------|----------|
| CL-0025 | `source-expectation` | `tests/video/video_mob.c` | frames F-M0 to F-M6 position by position; the 18 `video.mob.` ids, `video.border.`, `video.hdelay.`, `video.vdelay.`, `video.delay.`, three judges, faults MOB_PRIORITY_SWAP, MOB_FLIP_SWAPPED and DELAY_DIRECTION, the four frame-grammar controls, seven hash ties of five scenarios, four program mutants; 16 `depends_on` fields (the three MOB fields, the four border and delay fields, `stic.regmap`, `stic.backtab` and the existing `video.*` fields) |
| CL-0026 | `source-expectation` | `tests/video/video_collision.c` | the sixteen words T1 to T16; the 16 `video.collision.` ids, the truth-table judge, COLLISION_NOT_STICKY, `diag-stic2-collision-pass` (23 asserts), STIC2-COLLIDE-APART; 13 `depends_on` fields (the four collision fields, `stic.regmap`, `stic.access_effects`, `stic.mode_select_addrs`, `video.mob_field_origin`) |
| CL-0027 | `original-diagnostic` | `fixtures/FX-diag-stic-2/diag-stic-2.listing.txt` | the program (866 words, manifest FX-diag-stic-2, `asm.regen-ivs-diag-stic-2`), six scenario predicates equal to the assert blocks of section (f), the idle control, two pair lines, six program mutants with recorded failing sets; 39 `depends_on` fields (the eleven new profile fields, the boot stub, input, `power_on_fill.ram` and the existing `stic.*` and `video.*` fields) |

All three are `provisional`; the weakest-field rule agrees (every set holds `tier4-community` or `unknown-default-chosen` fields). Each note carries the oracle (commit 641a12e, blind second read closed at 1f6e19c, frozen since 9250265), the shared-misreading risk (one lineage, one author, one model family, C43), the `Excluded on purpose` list and the sentence that no measured tier exists (D10, D11). `grep -c 'Excluded on purpose' data/claims.json` rose from 10 to 13 and `grep -c D10` from 12 to 15; no claim is at `measured-hardware` and none uses the word "supported" or any hardware or compatibility statement. The CHANGELOG region equals the output of `tools/check/claims-table.cmake` byte for byte (27 rows, `Claims: 27 in total, 26 provisional, 1 established; none at tier measured-hardware.`).

## The registry and the retarget (Task 2)

- `data/expected/important-gates.ids` gained 18 lines, each ending `VID-05`: `vid05-mob-faults` (MOB_PRIORITY_SWAP, MOB_FLIP_SWAPPED), `vid05-collision-fault` (COLLISION_NOT_STICKY), `vid05-delay-fault` (DELAY_DIRECTION), `vid05-judges` (`video.judge.mob-frame-corrupt`, `video.judge.mob-role-swapped`, `video.judge.collision-truth-table-corrupt`, `video.judge.delay-frame-corrupt`), `vid05-program-mutants` (the six `runner-process.mutant.STIC2-*` ids) and `vid05-frame-grammar` (`video-frame-unknown-outside-region`, `video-frame-mob-count`, `video-frame-hash-none-without-unknown`, `video-frame-pair-unknown`). The step measures `gates=83 lines=282`, exactly the derived 77 + 6 and 264 + 18.
- Ten toy registries (the known-good tree and nine overlays) got `toy-vid05 fault GOOD_FAULT VID-05` (with a comment line). In `important-gates-bad-req-phase6` the requirement of line 24 changed from `VID-05` to `VID-99` on the same line and the stale comment on line 2 was reworded (no `VID-99` in it, so `grep -c VID-99` is 1); no `CHECK`, `EXPECT` or `FORBID` file of any `important-gates-*` control changed (the acceptance count is 0). `sh tools/check/run-controls.sh` prints `control ok important-gates-bad-req-phase6` and the nine others; the gate keeps `controls=371/371`.

## Discrepancy finals and the orchestrator decisions (Task 3)

The `evidence` fields of C107 (PD-1, `video.mob_prio_hides`, `video.mob.default-pinned.prio-set`, fault MOB_PRIORITY_SWAP, D10-VID-11, CL-0025), C108 (PD-2, `video.mob_line_sample`, `video.mob.default-pinned.sampling-y1x` and `.sampling-8x16`, F-M8 and F-M9, D10-VID-14, CL-0025), C109 (PD-8 and PD-9, `video.border_ext_strip` and `video.border_follows_delay`, `video.border.default-pinned.top-strip-unit` and `.follows-delay`, F-M4 to F-M6, D10-VID-13, CL-0025) and C110 (PD-10 to PD-12, `video.delay_exposed_fill`, `video.delay_mob_in_exposed`, the three `video.hdelay.` and `video.vdelay.` `default-pinned` ids, F-M7, DELAY_DIRECTION, D10-VID-12, CL-0025) were rewritten; C87, C92 (PD-13 and `video.vdelay.no-bus-window-change`), C105 and C81 got a "Final" paragraph appended. Statuses, `default_policy` text (but for the own bit below) and resolvers are as plan 08-03 set them; the gate keeps `discrepancies=110 open=101 pending-resolvers=20 d10-items=47` and `grep -c 'D10-VID-1[0-4]' data/discrepancies.json` is 12.

1. **Own bit (decision 1).** STIC-R44: the statement now says the own bit "is never set by the frame writer" and the note names the pin; C87: `default_policy`, `evidence` and `note` state the pinned default with the one source statement `SRC-community-emu-docs@programming-stic-mob-controls`; `docs/stic/reference-notes.md`: the `[UNK]` bullet; `data/expected/d10-measurements.ids`: the D10-VID-10 text names "own bit after a CPU write" (the procedure item already recorded the value read after a write of the own bit). PD-7 lives in the frozen section (i), so it is not edited: an additive "Amendments of plan 08-08" block at the end of section (k) supersedes its clause (zero deleted lines against 9250265; `git diff --quiet 9250265 -- data/video/frames` exits 0). The Phase 6 regmap literal, the profile and every test are untouched.
2. **Collision on the undelayed scene (decision 2).** Stated as an exclusion in the CL-0026 note, in the evidence of C110 and in the amendment block; `docs/stic/d10-procedure.md` (item D10-VID-12) and its ids line now also ask for the collision bits under a nonzero delay, so the hand-off honestly covers it.
3. **Frame counter (decision 3).** An assumption in the CL-0027 note (specification A5 clears the BACKTAB only; the counter assert of all six scenarios relies on `power_on_fill.ram`), with the field in `depends_on`.
4. **Glossary exemption (decision 4).** `video.border_follows_delay`, `video.delay_exposed_fill` and `video.delay_mob_in_exposed` stay exempt from the `glossary-unit-suffix` rule in `tools/check/steps/glossary.sh`, as plan 08-06 recorded and the orchestrator confirmed; nothing was edited.

## VID-05 and the checkpoint

- `.planning/REQUIREMENTS.md`: the checkbox and the traceability row of VID-05 are complete, with the "Phase 8 wave A close-out note (plan 08-08)" in the style of the Phase 6 note (provisional claims CL-0025 to CL-0027, tier 4 single lineage with a blind second read of the same model family, the pinned list C107 to C110 and the C87 pins each with a profile field or a flipping test, a claim exclusion and a hand-off, the five D10 hand-offs not performed, no tier above `source-expectation` and `original-diagnostic`, 159 by 96 kept). No other row was touched: REL-01, STA-01, STA-02 and PRF-01 stay Pending.
- `.planning/CI-EFFICIENCY.md` "Phase 8 checkpoint A (plan 08-08)": last-five median `r` 7.78 (rows 8.17, 7.73, 7.78, 8.37, 7.20; `build_s` of the newest 40); no recorded row contains wave A (`git merge-base --is-ancestor e0c5376 <sha>` is false for all five); additions 15.9 hosted cpu-s at the unit costs of the record, projection **8.18**; cross-checks **8.06** (the measured local deltas of the plan summaries, 6.5 local cpu-s at 1.675) and **8.13** (D-37's +8.7 plus the measured wave D1 cost); local isolated timings (the `video` step 1.62 to 2.26 cpu-s, `release-assets` unchanged at 0.57, three alternated runs each against the tree at the start of wave A). The rule is the orchestrator's decision (8.0 or more), so the line is `checkpointA-08: lever-required`; exactly one such line exists. The hosted row for the wave A head will be recorded by the orchestrator after the first pull-request run.

## Checks shown able to fail

- **VID-05 acceptance.** The step with `VID-05` removed from its `nreq` list gives `important-gates-unknown-id: line 349` and the following lines (the 18 real lines are refused); the real registry with one line naming `NO_SUCH_FAULT` gives `important-gates-unknown-id: fault NO_SUCH_FAULT line 352`; both probes ran on copies outside the tree. The real step prints `-- important-gates pass gates=83 lines=282`.
- **Every overlay still fails for its own reason.** All ten `important-gates` controls report ok after the toy lines (the nine overlays plus the positive `_good` tree), including `important-gates-bad-req-phase6` with its EXPECT `FAIL important-gates-unknown-id: line 24` and its FORBID `important-gates-uncovered-gate`: the retarget changed the requirement id only, so the control still reaches the unknown-id branch and VID-05 remains covered by the valid toy line.
- **The claim table.** The existing control `release-assets-claims-drift` and the step check `release-changelog-claims-drift` hold the CHANGELOG region equal to the generator output; the region was compared with `claims-table.cmake` output by `diff` before the gate (equal) and the gate shows `claims=27 provisional=26` with `release-selftest=11/11`.

## Deviations from Plan

**1. [Interpretation] The pair ids and the six scenario ids are not VID-05 registry lines.** Orchestrator decision 5 asked for the six `runner-process.mutant.STIC2-*` ids, the pair ids and the six scenario ids. The six mutants are registered (they resolve as `<suite>.mutant.<slug>`). The registry resolves only the mechanisms `fault`, `control` and `mutant` (a pkg or ci-summary selftest case, or a test id `<suite>.judge.<slug>` or `<suite>.mutant.<slug>`); `runner-process.scenario.*` and `runner-process.pair.*` are ordinary test ids and the step would reject them as unknown. Accepting them would need a fourth mechanism in the checker and a control that shows it can fail, and this plan has 0 controls and fixed counts (`gates=83 lines=282`). The scenarios are covered through the mutants that fail them (each of the six mutants fails its own scenario), and the pair judge's own self-controls are already registered under SCN-01 (`runner-process.judge.pair-video-equal`). If the owner wants them as registry rows, that is one checker change and one control.

**2. [Interpretation] PD-7 is amended by an addition, not an edit.** The decision asks to reword "STIC-R44 note, C87 and PD-7". PD-7 is a row of section (i) of the frozen derivation, and the freeze checks of plans 08-04 to 08-07 compare sections (a) to (j) against 9250265 with zero deleted lines; an edit would break that. The same effect is an additive block at the end of section (k) ("Amendments of plan 08-08") that names the superseded clause; the R44 note, C87 and the reference notes carry the new wording directly. Files beyond the plan's `files_modified`: `data/stic/rules.json`, `docs/video/diag-stic-2-derivation.md`, `docs/stic/reference-notes.md`, `docs/stic/d10-procedure.md`, `data/expected/d10-measurements.ids` (each a few lines, all records).

**3. [Interpretation] The claim artifacts are the ones of the plan action.** Section (h) of the derivation named `data/video/frames/F-M1.frame`, `F-M0.frame` and "the scenario predicate files" in advance; the plan action names `tests/video/video_mob.c`, `tests/video/video_collision.c` and `fixtures/FX-diag-stic-2/diag-stic-2.listing.txt`. I followed the plan (the later, executable text); each note names the frames and predicate files as the oracle, and the amendment block records the registered artifacts.

**4. [Interpretation] The checkpoint rule.** The plan's parenthetical reads "above 8.3, or a median of 8.0 or more"; the orchestrator's decision says "if the projection reaches 8.0 or more, write lever-required". The median alone is 7.78, so the plan's literal text would give `no-lever`, while every projection (8.06, 8.13, 8.18) reaches 8.0 under the orchestrator's rule. I followed the orchestrator and wrote both readings in the entry. This is the rule's first answer, as `checkpointB-07: lever-required` was in Phase 7; the stop rule and the D-36 split trigger (a `lever-required` checkpoint) are now the orchestrator's to answer.

**5. [Information] Counts.** "72 test ids over both presets" in the plan is 72 ids per preset (144 over both presets: `tests-expected` 4628 to 4772), as the arithmetic of the entry uses; "eleven new `ivs_stic_params` members" are eleven new profile fields that make 13 members (the origin and the strip size are pairs).

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag, no `tools/ci-record.sh`; `.planning/config.json` and `.planning/state.json` were never staged (D-42); no file under `core/`, `runner/`, `tests/`, `fixtures/`, `data/scenarios/`, `data/golden/` or `data/video/frames/` changed (`git status --short` of those paths is empty and none of the three task commits touches them).

## Decisions Made

- The notes carry the pinned items as exclusions instead of qualifiers, so a reader of one claim sees what it does not say; the CHANGELOG table prints only id, tier, standing and artifact.
- The registry gate names group mechanisms by kind of evidence (faults per feature, judges, program mutants, grammar controls) so that one gate can lose one mechanism without hiding the others.
- The discrepancy evidence says "Final (plans ...)" and keeps the plan 08-03 text in front of it, so the history of each pin stays readable.

## Cost (laptop, load average 9 to 13)

| Item | Value |
|------|-------|
| gate total, sh, at plan start | 85 s, gate-cpu user 144.89 sys 124.99 |
| gate total, sh and dash, Task 1 tree | 86 s and 86 s, gate-cpu user 147.80 sys 126.29 (sh), 145.59 and 125.42 (dash); identical summary line, md5 ce84bee4199986ef6b59c28e1bc6f7e8 |
| gate total, sh and dash, Task 2 tree | 91 s and 89 s, gate-cpu user 148.84 sys 133.91 (sh), 147.98 and 133.09 (dash); identical summary line, md5 85c407ee0cdd6488ffdc620e2234f79f |
| gate total, sh and dash, Task 3 tree | 85 s and 86 s, gate-cpu user 144.69 sys 124.83 (sh), 144.53 and 127.56 (dash), same md5 |
| container gate, Task 3 tree | 328 s (gcc 13.3.0, cmake 3.28.3, x86_64 emulated, `core-imports=5`), gate-cpu user 1199.24 sys 70.54 |
| `video` step in isolation | 1.62 to 2.26 cpu-s (median of three, start of wave A against this tree); `release-assets` 0.57 to 0.57 |

The plan adds no test id, fault or control; its own cost is the claim and registry records (the `important-gates` step reads about 350 lines and the toy registries grew by two lines each).

## Open Items

- **Orchestrator: `checkpointA-08: lever-required`.** Resolve it on the hosted row of the wave A head after the first pull-request run (as `checkpointB-07` was resolved on row 43528527276), or answer the stop rule: no further test id, fault or control in waves B and C until then, lever (2) queued as the first task of the next wave (the 29 earlier mutant cases to a Release-only full run and a static check on the sanitized preset; the six STIC2 mutants already are), and the D-36 split trigger (Phase 9 = wave B + C + close) is the orchestrator's decision. Record the hosted row with `tools/ci-record.sh` (orchestrator only).
- **Pair and scenario ids in the registry (Deviation 1).** A fourth registry mechanism would be needed; not done.
- **Own bit on a CPU write.** Stays an explicit pin. If the owner wants the source rule applied instead, the change is the one listed in the 08-05 summary (eight write masks in the Phase 6 regmap literal and the profile, three expectation edits), after which the pin text of R44, C87, the reference notes and the amendment block is reversed.
- **Frame counter assumption.** Unchanged specification A5; a new oracle commit would be needed to make the scenarios independent of `power_on_fill.ram`.
- **Plans 08-20 and 08-21.** The stage-1 `## [0.1.0]` section of CHANGELOG.md keeps the claim table between its markers; any later claim regenerates it in the same commit. `tools/release.sh` lists six images and not `diag-stic-2.ivsc` (a release-plan decision, carried from 08-07).
- **Information.** D10 and D11 are not performed; no claim above provisional exists; the hosted-only checks (hosted runner timings of the wave) are not run; REL-01 stays Pending until stage 2 (D-28).

## Interfaces for wave B (the state waves, plans 08-09 onward)

- `ivs_stic_state` is **528 bytes, unchanged** by the whole of wave A: no hidden state was added, so the snapshot checklist gains no STIC state row (D-20: the STIC state layout is settled). The collision bits live in `mem.stic_reg[0x18..0x1F]`, already serialized; the MOB, border and delay registers are ordinary register-file words.
- `ivs_stic_frame_deliver` takes a **non-const** `ivs_memsys *` (the frame end ORs the collision bits through `ivs_memsys_stic_collision_or_internal`, masked 03FF, no gate, no counter).
- **Eleven new profile fields** (13 members of `ivs_stic_params`, all read by `cmake/IvsSticTable.cmake`): `video.mob_field_origin`, `video.mob_line_sample`, `video.mob_prio_hides`, `video.collision_x0_interacts`, `video.collision_border_bit`, `video.collision_offscreen`, `video.collision_display_off`, `video.border_ext_strip`, `video.border_follows_delay`, `video.delay_exposed_fill`, `video.delay_mob_in_exposed` (`profile-keys=154`, `unknown-fields=47`).
- **For HP-MOB (the VID-05 MOB cut of D-17):** the new image `diag-stic-2.ivsc` (target `ivs_images`, build tree only, not in the install package) and the scenarios `diag-stic2-{idle,mob,border,hdelay,vdelay,collision}-pass` (the mob scenario is selector 7E, KEY_1, scenes change at frames 4 and 7; the collision scenario is selector BD, KEY_5, with sticky bits in the register file across frames), each with a golden in `data/golden/`.
- **Counts to carry forward:** `steps=37/37 controls=371 faults=226 tests-expected=4772 tests-executed=4768 tests-skipped=4 claims=27 provisional=26 gates=83 lines=282 goldens=27 discrepancies=110 open=101 pending-resolvers=20 d10-items=47 stic-rules=67 video-frames=26 video-frame-pairs=9 video-hash-ties=18 profile-keys=154`; per plan 08-01 the later `state` step is step 38, so the step count after it is 38.
- **Stop rule:** `checkpointA-08: lever-required` (above): ids, faults and controls of waves B and C wait for the orchestrator.
- Threat ids **T-08-01 to T-08-23** were used by plans 08-01 to 08-08 (T-08-SC throughout); the state waves continue from T-08-24 or the numbering their plans assign.

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=37/37 controls=371/371 ... fault-runs=226 ... faults=226/226 ... claims=27 provisional=26 ... gates=83 lines=282 ... discrepancies=110 pending-resolvers=20 open=101 d10-items=47 ... profile-keys=154 goldens=27 ... tests-expected=4772 tests-executed=4768 tests-skipped=4 tests-missing=0 ... stic-rules=67 ... video-frames=26 video-hash-ties=18 video-frame-pairs=9 (0 FAIL lines; gate-time total=85 s on the Task 3 tree)
dash tools/gate.sh:    the identical summary line (md5 85c407ee0cdd6488ffdc620e2234f79f for both shells; 0 FAIL lines; total=86 s)
GATE_LINUX_PLATFORM= sh tools/gate-linux.sh: exit 0 on the same tree (build/gate-linux.log ends gate: PASS; gcc 13.3.0, cmake 3.28.3, x86_64 emulated, cc=GNU, core-imports=5), the plan-end counts as above, 0 FAIL lines (gate-time total=328 s)
```

After Task 1 (before its commit): sh and dash `claims=27 provisional=26 gates=77 lines=264` (md5 ce84bee4199986ef6b59c28e1bc6f7e8 for both shells). After Task 2: `gates=83 lines=282` (md5 85c407ee0cdd6488ffdc620e2234f79f for both shells). Task 3 moved no count, so its line has the same md5. The first gate run of every task passed; nothing needed a retry. `git diff --quiet 9250265 -- data/video/frames` exits 0 and the derivation diff against 9250265 has 19 added lines and 0 deleted lines. On the closing tree (this SUMMARY, STATE.md and ROADMAP.md added or edited; `tracked=2391`): sh (84 s) and dash (84 s) `gate: PASS` with the identical summary line (md5 6d4a1b4e7bfdf4b55134d9cdc0030593), 0 `FAIL` lines, and the container gate was run again (`GATE_LINUX_PLATFORM= sh tools/gate-linux.sh`): exit 0, `gate: PASS` with the plan-end counts, 0 `FAIL` lines, gate-time total=330 s; `build/gate-linux.log` holds that run.

## Known Stubs

None.

## Threat Flags

None new. T-08-22 (overclaim, silent pinned defaults): all three claims provisional, tiers `source-expectation` and `original-diagnostic` only, an `Excluded on purpose` list in each note that names every pinned item, the own bit, collision under delay and the frame-counter assumption, the shared-misreading sentence, no measured tier, the CHANGELOG table prints no statement text, the text scan is green; the completion note states the terms in the Phase 6 style. T-08-23 (the retargeted control and the nine toy registries): the retarget changed only the requirement id on the same line, EXPECT and FORBID untouched, the controls run shows it still fails for its reason, `controls=371/371` held, every overlay keeps a valid toy VID-05 line and the 18 real lines resolve to mechanisms listed elsewhere in the gate (two probes show the step refuses an unaccepted requirement and an unknown mechanism). T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: `.planning/phases/08-stic-completion-save-state-performance-baselines/08-08-SUMMARY.md` exists; commits `07cd417`, `de7d5cb` and `88e6abe` are ancestors of HEAD and `git rev-list --count 8e67c2aa7aefd3bb8f64356dc89cc8c52717e430..HEAD` printed 3 before this file's commit; `.planning/config.json` and `.planning/state.json` are not staged; `git ls-files` lists no `.ivsc`, `.bin`, `.rom`, `.exec`, `.wav`, `.ppm`, `.vraw` or `.ivsn` file; the acceptance greps give `grep -c 'vid05-' data/expected/important-gates.ids` 18, `grep -c VID-99` in the retargeted overlay 1, 0 changed `CHECK`, `EXPECT` or `FORBID` files of `important-gates-*` controls, `grep -c '^checkpointA-08: ' .planning/CI-EFFICIENCY.md` 1, `grep -c 'D10-VID-1[0-4]' data/discrepancies.json` 12, `grep -c 'Excluded on purpose' data/claims.json` 13 (10 at plan start), no home, scratch or temp path in any changed tracked file.
