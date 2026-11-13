---
phase: 06-stic-video-first-pass
plan: 14
subsystem: claims, discrepancies, important-gates registry, traceability, Decision Register, CI close-out, verification
tags: [close-out, claims, discrepancies, d10-inventory, important-gates, verification, D-22, D-45, D-21, A-11, VID-01, VID-02, VID-03, VID-04, VID-06]
status: complete

requires:
  - phase: 06-stic-video-first-pass
    provides: "plans 06-01 to 06-13; oracle commit eae8ca5 (06-01 task 5); gate steps=36/36 controls=336/336 faults=221/221 tests-expected=4422 at plan start; HEAD at start 0094f33"
provides:
  - "claims CL-0016 to CL-0022 (provisional; none measured-hardware): 22 claims, 21 provisional"
  - "final Phase 6 state of the discrepancy records C1 to C7, C10, C21, C38, C41, C52, C60, C61 and C81 to C105: each evidence field names the profile field or code that carries the default and the test ids that exercise it; resolver plan references corrected; C10 stays partial"
  - "important-gate registry for VID-01, VID-02, VID-03, VID-04, VID-06 (gates=71 lines=238, 63 new lines) and three new controls (339 in total)"
  - "REQUIREMENTS.md: VID-01 to VID-04 complete (VID-06 already complete since plan 06-07), VID-05 Pending; ROADMAP.md Phase 6 14/14"
  - "PROJECT.md Decision Register: D8 corrected in place, D3 and D4 notes"
  - "CI-EFFICIENCY.md Phase 6 close-out (checkpoint D): phase6-close: no-lever"
  - "06-VERIFICATION.md with three tables; handoff-items=42"
affects: [phase-6-verification, phase-7]

actuals:
  tasks: 4
  commits: 5
plan_head_before: 0094f33
oracle_commit: eae8ca5

key-decisions:
  - "The D-45 arbiter is the re-baselined set (gate median 90 s and trigger last-5 median above 110 s or any run above 130 s; cpu median 300 and trigger above 330; r budget 7.8, revisit above 8.3), not the older figures the plan text quotes (75/90/120 and 260/280); both are stated in the entry"
  - "phase6-close: no-lever. Last-5 medians gate 101 s, cpu 315, r 7.64; the single 131 s run (43359195974) is judged runner noise"
  - "Discrepancy updates append a 'Phase 6 final (plan 06-14)' sentence to the evidence field (carrier and test ids) instead of rewriting the seeded notes; no status or default policy changed, C10 stays partial"

key-files:
  created:
    - .planning/phases/06-stic-video-first-pass/06-VERIFICATION.md
    - tools/check/controls/important-gates-req-uncovered-phase6/, important-gates-unknown-id-phase6/, important-gates-bad-req-phase6/
  modified:
    - data/claims.json, data/expected/claims.ids, data/discrepancies.json
    - data/expected/important-gates.ids, data/expected/controls/important-gates.ids
    - tools/check/steps/important-gates.sh
    - tools/check/controls/_good/tree/data/expected/important-gates.ids and the five earlier important-gates control overlays
    - docs/stic/second-read-request.md
    - .planning/REQUIREMENTS.md, .planning/ROADMAP.md, .planning/PROJECT.md, .planning/CI-EFFICIENCY.md, .planning/STATE.md

completed: 2026-11-12
---

# Phase 6 Plan 14: close-out Summary

**Seven provisional claims, the final discrepancy state, an important-gate registry that ties each of VID-01, VID-02, VID-03, VID-04 and VID-06 to named mechanisms shown to fail for their intended reasons, the Decision Register notes, the cost evaluation (`phase6-close: no-lever`) and a verification record with three tables (automated, handed off with `handoff-items=42`, accepted caveats); the gate ends `steps=36/36 controls=339/339 faults=221/221 claims=22 discrepancies=105 d10-items=42 tests-missing=0` under `sh`, `dash` and the Ubuntu container.**

Oracle commit this plan follows (06-01 task 5): `eae8ca5`. No oracle file was edited by this plan; the claims cite them.

## Commits

| Commit | Task | What |
|--------|------|------|
| `c379a7a` | 1 | claims CL-0016..CL-0022, final discrepancy states, second-read request state line |
| `f78cc33` | 2 | registry step and lines for the five Phase 6 requirements, three controls, REQUIREMENTS and ROADMAP traceability |
| `735fd11` | 3 | Decision Register (D8 in place, D3, D4), CI-EFFICIENCY close-out |
| the commit that adds `06-VERIFICATION.md` | 4 | 06-VERIFICATION.md with its covered files and digest |
| this commit | | this SUMMARY, STATE, ROADMAP |

## What was built

- Task 1: the seven claims with the ids reserved in plan 06-01 (CL-0016 register file and handshake, CL-0017 timeline and bus steal as repeatability, CL-0018 geometry, layout and chain, CL-0019 the ten frames, CL-0020 the diagnostic, CL-0021 inspection without side effects, CL-0022 the installed replay), each note naming its oracle, the shared-misreading risk, the excluded items and the second-read status (all four records of 2026-11-12 exist; none is pending). The discrepancy records gained a final evidence sentence naming the profile field or code and the real test ids (every id was resolved against `data/expected/tests/*.ids` and `data/expected/faults/*.ids`); eight resolver strings that carried stale plan numbers were rewritten to the real families; C84's resolver now says the other value of `stic.alias_read_sensitive` is range-checked only. `d10-measurements.ids` and `docs/stic/d10-procedure.md` were already final from plan 06-01 (42 lines, ten Phase 6 items, one anchor each) and are unchanged.
- Task 2: `important-gates.sh` accepts VID-01, VID-02, VID-03, VID-04 and VID-06 (header, token text, the requirement list in the awk; VID-05 is rejected and one control shows it); 63 registry lines in 18 new gates; three controls with overlay registries; the toy tree and the five earlier control overlays gained five toy Phase 6 lines so each control isolates its own defect.
- Task 3: D8 corrected in place with the full decision and reversal; D3 and D4 notes; the close-out entry of `.planning/CI-EFFICIENCY.md`.
- Task 4: scratch recomputation, end-of-phase registry checks, `06-VERIFICATION.md`, the three gates.

## Registry mapping (plan ids against the inventories)

All 63 ids named by the plan exist; no id was renamed during the phase, so no mapping was needed and no mechanism was added. The `mutant` mechanism resolves `stic.judge.*`, `video.judge.*`, `runner-process.mutant.STIC-*` (test ids of the suites) and the `pkg-selftests.ids` cases `stic-domain-missing`, `stic-digest-drift`.

## Recomputation results (scratch, not committed)

All agree: event ticks 8,000, 15,120, 15,164, 15,392 + 3,648 i, 59,736 from the profile fields; resume `w + 468`; 13 windows against the printed bound 13 or 14; capacity 15,296 and 72 slots = 1,101,312 bytes; F-A3 per-card counts 32, 28, 8, 32, all 96 rows and one 8 x 8 block recomputed from the BACKTAB words and card table, and its canonical text hash; the chain values h0, h1, h2 by the printf recipe; both diagnostic scenarios' delivered slots (header, plane equal to F-D1 and F-D2) and the domain-16 digest recomputed from them equal the runner's `digest.video`. Scenario exit codes 0, 0, 10.

## End-of-phase registry check

`data/stic/fields-pending.ids` and `data/stic/tests-pending.ids`: 0 data lines each (header comments only); `git ls-files refs`: nothing; no `.ppm`, `.vraw`, `.ivsc`, `.bin`, `.rom` or `.wav` tracked.

## Cost close-out (checkpoint D)

Last five hosted rows (runs 43341351141, 43346071659, 43352432394, 43359195974, 43365038335): gate_s median 101, cpu_s median 315, `r` median 7.64 (rows 5.95, 7.64, 7.68, 9.02, 7.53). Arbiter used: the re-baselined figures (gate trigger above 110 or any run above 130, cpu trigger above 330, r revisit above 8.3), not the older 75/90/120 and 260/280 of the plan text; the entry shows both. One run (43359195974, 131 s) exceeded the 130 s any-run bound by one second; judged runner noise (single sample, next row 87 s, the slowest build_s of the five, a test_s jump that two faults and 78 ids cannot explain, neighbours at r 7.68 and 7.53), as the orchestrator reviews said. Projection by the 06-02 formula 307.8 (budget 300, trigger 330). Checkpoints A, B and C were all `no-lever`; plan 06-L1 was never created; the only lever of the phase was 06-02 (`-g0` on fault targets). **`phase6-close: no-lever`.**

## Deviations from Plan

**1. [Interpretation] The provisional count is 21, not 22.** The acceptance line says `grep -c '"standing": "provisional"' data/claims.json` is at least 22 "every claim of Phases 5 and 6 is provisional"; the ledger also holds the Phase 1 claim CL-0002, which is `established` (all three of its fields are tier 1), so 21 of 22 are provisional and all 14 claims of Phases 5 and 6 are. Not faked: the count stays 21.

**2. [Interpretation] Re-baselined triggers used as the arbiter** (the orchestrator's instruction); stated in the CI entry with the old figures beside them.

**3. [Rule 3] Files outside the plan's list.** `docs/stic/second-read-request.md` (its state line still read `pending: blind reads not yet run` although the four records exist; now `done: blind reads recorded`, as the PSG request reads), the five earlier important-gates control overlays and the toy tree (needed so the new uncovered-requirement check does not fire in them), `.planning/STATE.md`.

**4. [Interpretation] `dash tools/gate.sh` was run at the end of Tasks 2 and 4, not after Task 1 and Task 3**, which change only data and planning documents (no script, no C, no shell construct): the `sh` gate passed after each of them and a `dash` run of the same tree adds nothing for such files.

**5. [Interpretation] VID-06 was already complete** (marked in plan 06-07's close of the inspection foundation); this plan marks VID-01 to VID-04 and leaves VID-05 Pending. The ROADMAP phase checklist and progress row were also set here (the earlier plans did not touch them); the orchestrator may revert them if it wants to mark the phase after its own verification.

**6. [Interpretation] The VERIFICATION record is the executor's own** (status `passed`, with `covered_files` and `covered_digest` computed by `gsd_run query verification.fingerprint`), written in the shape of `05-VERIFICATION.md`; Phase 5 had an independent verifier re-run after its executor, and a later run may refresh this one.

## Decisions Made

- No requirement beyond VID-01 to VID-04 and VID-06 is closed; VID-05 stays Pending (Phase 8). Every claim is provisional and none uses the measured tier; D10 and D11 were not performed.
- C10 stays `partial`; no discrepancy became `resolved-primary`.

## Open Items

- Hosted cost of this plan is unmeasured; the first hosted row after the push is the arbiter (margin to the `r` trigger: 7.64 against 8.3).
- Phase 7 needs recorded by 06-13: a data directive for listings, a project interrupt routine (the diagnostic shows one frame), the D4 mini-assembler re-evaluation, the reader cartridge for the D10-VID items.
- `stic.alias_read_sensitive` = 0 is only range-checked (C84); no test runs the machine with it.
- The derivation's cycle budget ran 31 cycles above the observed run (inference gap); the `listing-bits-mismatch` check was not exercised by a scratch formatter; `.expect` was committed after the first scratch runs (06-13 deviation, unchanged).
- Carried unchanged: Phase 4 and 5 review deferrals, `sh tools/hygiene.sh --history` on earlier planning commits, the carried residual that the golden itself is written with `fopen`.

## Final gate

```
sh tools/gate.sh:       gate: PASS steps=36/36 controls=339/339 faults=221/221 fault-runs=221 claims=22 provisional=21 discrepancies=105 d10-items=42 gates=71 lines=238 listings=4 consumer-lines=76 pkg-selftest=15/15 tests-expected=4422 tests-executed=4420 tests-skipped=2 tests-missing=0 (gate-time total=139, final tree)
dash tools/gate.sh:     the identical summary line (0 FAIL lines; gate-time total=113)
GATE_LINUX_PLATFORM= sh tools/gate-linux.sh: exit 0, last line of build/gate-linux.log starts gate: PASS, x86_64 gcc 13.3.0 cmake 3.28.3, steps=36/36 controls=339/339 faults=221/221 claims=22 discrepancies=105 d10-items=42 listings=4 pkg-selftest=15/15 tests-missing=0, 0 FAIL lines (gate-time total=273)
```

## Known Stubs

None.

## Threat Flags

None new. T-06-20 is mitigated: `d10-measurements.ids` checked in both directions by the `discrepancies` step (`d10-items=42` copied as `handoff-items=42`), claims all provisional by the weakest-field rule, the `claims` step rejects `measured-hardware`, three important-gates controls, the independent recomputation recorded. T-06-SC: no installs.

## Self-Check: PASSED

Created files exist (`06-VERIFICATION.md`, the three control directories); commits `c379a7a`, `f78cc33`, `735fd11` are ancestors of HEAD; `grep -c '^phase6-close: no-lever$' .planning/CI-EFFICIENCY.md` is 1; `grep -c . data/expected/claims.ids` includes CL-0016 to CL-0022.
