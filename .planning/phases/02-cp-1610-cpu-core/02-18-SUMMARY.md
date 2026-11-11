---
phase: 02-cp-1610-cpu-core
plan: 18
subsystem: testing
tags: [cp1610, reference-notes, D-13, D-22, D-24, gate-budget, control-runner, evidence]

requires:
  - phase: 02-cp-1610-cpu-core
    provides: "412 vectors with 12 second-read markers (02-17), 67 named faults, 190 controls, 24 gate steps, bus-figure audit (02-15), discrepancy log C29-C58"
provides:
  - "docs/cp1610/reference-notes.md: D-13 page readings (N bit, C34, C29 NOPP twin, timing figures pdf 27/30/33/35, ADAR pdf 11, BDRDY pdf 40, 8+2W pdf 43, DW kind, STPST edge, JSR PC+3 as C58/G8, A-06 gap map) and a `## Vector second reading` section"
  - "tools/check/run-controls.sh: controls run in 4 lanes with per-control work dirs and sorted output (D-22 lever 2); output byte-identical to the sequential runner"
  - ".planning/CI-EFFICIENCY.md: local measurement with the full C build and 190 controls, D-24 supersedes the macOS-leg line, hosted gate budget as an open item"
  - "Phase evidence table: ROADMAP SC1-SC5 and CPU-01..05, API-05, PKG-02, RUN-03 each tied to a gate stat and named controls"
affects: [verify-work, phase-03, ci-efficiency]

actuals:
  tokens: 6805
  tasks: 3
  commits: 2
plan_head_before: 5d9ddb7b57191f422a9f8ec055b9ecefd60a084d
plan_head_after: 64a7730b165244a29668e8831573e7a46e2f3f26

tech-stack:
  added: []
  patterns:
    - "Lane-parallel control runner: lane i takes every 4th entry of the sorted list, each control owns its work dir, results are printed afterwards in sorted order so output is independent of scheduling"
    - "Speed levers are accepted only with a byte-identical output diff against the previous runner on both the pass path and every failure token"

key-files:
  created: []
  modified:
    - docs/cp1610/reference-notes.md
    - tools/check/run-controls.sh
    - .planning/CI-EFFICIENCY.md

key-decisions:
  - "D-22 lever 2 applied (parallel control runner, 4 lanes): runner alone 73.0 s -> 32.3 s under load 16-24, output byte-identical on 190 controls and on all nine runner failure tokens under sh and dash"
  - "D-22 levers 1 and 3 not applied: c-build is already the first step and costs 0.5 s warm; c-faults is 3.2 s with an incremental ivs_faults build"
  - "The host was never idle (load 13-33), so the 25 s local budget stays unjudged; overlapping the control runner with the steps in gate.sh is recorded as the next candidate, not applied"
  - "Hosted gate budget stays an explicit open item: last hosted figure 19 s gate step at 182 controls; the final 190-control gate with the parallel runner is unmeasured on the hosted runner"
  - "Reference notes keep C34 open: the project reads 4 + 10 = 14 for both SDBD forms, the isolated readers read the immediate 14 as target-only (18)"

patterns-established:
  - "A gate speed change is shown safe by diffing old and new runner output on the real controls and on a synthetic set that hits every runner failure token"

requirements-completed: [CPU-01, CPU-02, CPU-03, CPU-04, CPU-05, API-05, PKG-02, RUN-03]

coverage:
  - id: D1
    description: "Reference notes record the Phase 2 page readings (D-13) and the vector second reading with counts and limits"
    requirement: CPU-01
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (text, glossary, denylist, integrity and C17 rules scan the notes): gate: PASS"
        status: pass
      - kind: other
        ref: "grep -c -F '## Vector second reading' = 1; grep -c '^C17 status: ' = 1; grep -c -F C58 = 2; grep -c -E 'pdf (27|30|33|35|40|43)' 4 -> 17; grep -c -F DW = 6"
        status: pass
    human_judgment: true
    rationale: "Each [OBS] line is a reading of a page image; the gate checks wording rules and the C17 line, not whether a reading matches the page"
  - id: D2
    description: "Parallel control runner (D-22 lever 2) with identical tokens, counts and output order"
    requirement: PKG-02
    verification:
      - kind: other
        ref: "cmp of sequential vs 4-lane runner output on all 190 controls (sh and dash): identical"
        status: pass
      - kind: other
        ref: "cmp on a synthetic controls set hitting all nine runner failure tokens (old sh, new sh, new dash): identical"
        status: pass
      - kind: integration
        ref: "sh tools/gate.sh and dash tools/gate.sh: gate: PASS controls=190/190, runner-selftest cases=4"
        status: pass
    human_judgment: false
  - id: D3
    description: "CI-EFFICIENCY record of the local gate measurement, D-24 amendment and the hosted budget open item"
    requirement: PKG-02
    verification:
      - kind: other
        ref: "grep -c 'D-24' .planning/CI-EFFICIENCY.md = 1; grep -c -i 'first hosted C run' = 2"
        status: pass
    human_judgment: false
  - id: D4
    description: "Evidence table tying ROADMAP SC1-SC5 and the eight requirements to green gate stats and named controls"
    requirement: RUN-03
    verification:
      - kind: integration
        ref: "sh tools/gate.sh: steps=24/24 controls=190/190 faults=67/67 rows-pending=0 policies-pending=0 second-read=12/12 tests-missing=0 purity-selftest=5/5"
        status: pass
    human_judgment: true
    rationale: "Whether each stat and control is sufficient evidence for its criterion is a verifier judgment"

duration: 15min
completed: 2026-11-11
status: complete
---

# Phase 2 Plan 18: Reference-note close-out, D-22 gate measurement and phase evidence Summary

**The reference notes now carry the Phase 2 page readings (timing-figure offsets, ADAR, the BDRDY wait rule, DW as a no-wait access kind, the STPST edge, JSR PC+3 as C58) and a second-reading section with its limits. The control runner now runs in 4 lanes with byte-identical output, cutting it from 73 s to 32 s under load. Every ROADMAP criterion and requirement maps to a green gate stat and named controls: `gate: PASS steps=24/24 controls=190/190 faults=67/67 rows-pending=0 policies-pending=0 second-read=12/12 tests-missing=0`.**

## Performance

- **Duration:** about 15 min, timed from the recorded start (reading the plan and context before it is not counted)
- **Started:** 2026-11-11T01:26:13Z
- **Completed:** 2026-11-11T01:41:22Z
- **Tasks:** 3 of 3 (Task 3 changed no file; its output is the evidence table below)
- **Files modified:** 3 (2 task commits)

## Accomplishments

- **D-13 notes.** `docs/cp1610/reference-notes.md` gains the following:
  - NOP and SIN read as the GSWD R4-R7 forms, with N = DDD bit 0.
  - The C34 reading (4 + 10 = 14 for both SDBD forms). The readers' 18 alternative is recorded and C34 stays open.
  - NOPP as the never-taken twin of B (C29, POL-BR-01).
  - JSR PC+3, legible on pdf 56, settling the smudge on pdf 52 (C58, gap G8).
  - A new "Timing figures and wait states" section: the offsets of pdf 27, 30, 33 and 35, ADAR (pdf 11), BDRDY sampling (pdf 40), 8 + 2W (pdf 43), DW as its own no-wait kind (A-11) and the STPST edge (pdf 14, pdf 40).
  - An "Inferences recorded against log entries" subsection with the A-06 map (G1-G6 = C46-C51, G7 = C37, G8 = C58) and the orchestrator items: the SARC N = 1 link layout as a C48 inference, JE_AFTER_BOUNDARY as an approximation (C41), and RSWD from R7 reading the already incremented PC (POL-STK-01, inference).
- **Second reading.** A `## Vector second reading` section gives:
  - the method and a per-file count table (412 vectors, 382 agreed, 30 disagreements, all 30 resolved to the committed value)
  - the four disagreement groups
  - the fact that the plan 02-17 reader-document clause additions are outside every digest the gate checks
  - five limits: same model family (C43), the same executor adjudicated both sides, isolation attested by transcript not by the gate, policy vectors encode chosen defaults, and D10 hardware evidence is missing.
- **Control runner.** `tools/check/run-controls.sh` now runs controls in 4 lanes, each control with its own tree, filelist, output and result file, and prints results in sorted order. The standalone runner went from 73.0 s to 32.3 s.
- **CI record.** `.planning/CI-EFFICIENCY.md` now has the following:
  - the before/after measurement table and the per-step breakdown
  - the lever decisions
  - the D-24 supersession of the macOS-leg line
  - the hosted gate budget as an explicit open item

## Task Commits

1. **Task 1: Tracer: reference-note updates and the vector second-reading section.** `8084cb1` (docs). Tracer verified end to end: the gate passed with the new notes before any expansion (101.4 s wall, load 15).
2. **Task 2: D-22 gate budget measurement, lever 2, CI-EFFICIENCY record.** `64a7730` (perf).
3. **Task 3: Phase evidence check.** No commit. The gate output did not contradict any count in the notes (412 / 382 / 30 matches `vectors=412` and `second-read=12/12`), so the notes needed no correction. The evidence is the table below.

**Plan metadata:** recorded in the docs(02-18) completion commit.

## Acceptance counts (Task 1)

| Check | Before (HEAD 5d9ddb7) | After |
|-------|-----------------------|-------|
| `grep -c -F 'C58' docs/cp1610/reference-notes.md` | 0 | 2 |
| `grep -c -E 'pdf (27\|30\|33\|35\|40\|43)' docs/cp1610/reference-notes.md` | 4 | 17 (+13) |
| `grep -c -F 'DW' docs/cp1610/reference-notes.md` | 3 | 6 |
| `grep -c -F '## Vector second reading'` | 0 | 1 |
| `grep -c '^C17 status: '` | 1 | 1 |
| `grep -c -F '"id": "C58"' data/discrepancies.json` | 1 | 1 (C58 is resolved-primary, not open) |

Page images read in this plan for the [OBS] labels: `refs/pages/gi-011.png`, `gi-014.png`, `gi-030.png`, `gi-033.png`, `gi-035.png`, `gi-040.png`, `gi-043.png` and `gi-056.png`. They are already rendered and gitignored, and none is committed. Statements not read from an image in this plan carry [INF], for example the pdf 32 BAR@4 offset, which is derived from the sequence recorded in Phase 1.

## Gate budget measurement (Task 2)

The host was never idle. Load average was 13 to 33 in every run, from unrelated work (another emulator's tests, a fuzzer, RetroArch).

| Run | Runner | Wall s | User s | Sys s | Load at start |
|-----|--------|--------|--------|-------|---------------|
| `sh tools/gate.sh` 1 | sequential | 83.3 | 30.9 | 44.0 | 16.5 |
| `sh tools/gate.sh` 2 | sequential | 101.4 | 37.8 | 56.1 | 15.3 |
| `sh tools/gate.sh` 3 | sequential | 81.7 | 28.2 | 41.1 | 26.2 |
| `sh tools/check/run-controls.sh` | sequential | 73.0 | 19.2 | 43.5 | 15.8 |
| `sh tools/gate.sh` 1 | 4 lanes | 60.6 | 33.0 | 54.9 | 20.0 |
| `sh tools/gate.sh` 2 | 4 lanes | 87.9 | 39.5 | 72.6 | 19.9 (33.5 at end) |
| `sh tools/gate.sh` 3 | 4 lanes | 67.3 | 37.1 | 59.3 | 33.5 |
| `sh tools/check/run-controls.sh` | 4 lanes | 32.3 | 19.4 | 48.1 | 18.4 |
| `dash tools/check/run-controls.sh` | 4 lanes | 43.9 | 20.3 | 51.0 | 24.3 (at end) |
| `dash tools/gate.sh` (Task 2 verify) | 4 lanes | 48.8 | 32.2 | 46.4 | 17.4 |
| `sh tools/gate.sh` final, unpiped | 4 lanes | 65.5 | 36.2 | 58.7 | 14.9 (24.6 at end) |

- Medians: the gate went from 83.3 s to 67.3 s and the runner alone from 73.0 s to 32.3 s. The after-runs saw higher load, so the runner-alone pair is the cleaner comparison.
- Per-step breakdown (sequential runner, load 13):
  - `run-controls` 55.0 s
  - `hygiene-history` 9.4, `cp1610-vectors` 6.5, `faults-verdict` 4.5, `c-faults` 3.2, `hygiene` 3.0, `denylist` 2.7, `c-purity` 1.9, `c-tests` 1.7, `integrity` 1.3, `cp1610` 1.0
  - the other 13 steps under 0.8 each; `c-build` 0.5 (warm)
- Levers:
  - **(1) background C build: not applied.** `c-build` is the first step in C-locale order and costs 0.5 s warm, so a background start would overlap with nothing.
  - **(2) parallel control runner: applied.** Output is byte-identical to the sequential runner:
    - on the 190 real controls, under sh and dash
    - on a synthetic set that hits all nine runner failure tokens: `control-not-in-inventory`, `control-inventory-orphan`, `control-make-path`, `control-build`, `control-exit-zero`, `control-expect-blank`, `control-forbidden-token`, `control-unknown-step` and `control-token-missing`

    `runner-selftest` still reports `cases=4`.
  - **(3) owning-TU rebuild: not applied.** The `ivs_faults` build is incremental and the whole `c-faults` step takes 3.2 s.
- The 25 s local budget is still unjudged because no idle host was available. The next candidate lever is to overlap the control runner with the steps in `gate.sh` (about 39 s of steps and about 32 s of controls under this load). It is recorded in CI-EFFICIENCY.md, not applied.

## Phase evidence (Task 3)

Final `sh tools/gate.sh` (unpiped, exit 0, 65.5 s wall):

```
gate: PASS steps=24/24 controls=190/190 cc=AppleClang c-presets=2 fault-runs=67 core-imports=2 purity-selftest=5/5 ctest-runs=2 ci-uses=1 ci-workflows=1 claims=5 provisional=4 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=124 waived=0 discrepancies=58 pending-resolvers=18 open=55 faults=67/67 tracked=921 binaries-listed=0 terms=9 doc-files=11 profile-keys=28 mode=tree hygiene-files=921 history-commits=1 files=921 manifests=1 effective-yes=0 effective-no=1 records=28 fields=28 unknown-fields=14 cases=4 sources=9 refs-verified=1/4 tests-expected=1040 tests-executed=1040 tests-skipped=0 tests-missing=0 docs=11 claim-refs=5 tags=10 tiers=8
```

| Criterion | Green gate stats | Controls that show the negative case |
|-----------|------------------|--------------------------------------|
| **SC1 / RUN-03**: the documented command runs the full inventory, reports executed, skipped and missing counts, and fails on a missing, duplicated, skipped or stale test | `tests-expected=1040 tests-executed=1040 tests-skipped=0 tests-missing=0` (520 test ids x 2 presets: cpu-vectors 412, cpu-sweep 75, cpu-api 20, cpu-fuzz 8, harness 4, core-symbols 1); `ctest-runs=2` | `tests-inventory-missing`, `-duplicate`, `-unexpected-skip`, `-stale-nonce`, `-stale-digest`, `-unlisted`, `-ctest-drift`, `-truncated`, `-failed`, `-empty`, `-suite-missing`, `-plan-mismatch` |
| **SC2 / CPU-01**: every row has page-cited vectors checking registers, memory, flags and cycles; a broken opcode fails its vectors | `cp1610-rows=74 vectors=412 source=291 policy=121 rows-pending=0 faults=67/67 fault-runs=67`; second reading `blind-files=12 second-read=12/12` (source-expectation, second-read; see the note below) | `vectors-row-uncovered`, `vectors-flag-uncovered`, `vectors-cycles-uncovered`, `vectors-cycles-basis`, `vectors-bad-cite`, `vectors-tag-false`, `vectors-emulator-name`, `faults-not-detected`, `faults-wrong-reason`, `faults-collateral`, `faults-missing`, `vectors-review-handoff-mismatch`, `vectors-review-stale`, `vectors-review-unresolved`, `vectors-blind-leak`, `vectors-blind-unknown-key` |
| **SC3 / CPU-02, CPU-03, CPU-04**: interrupt and BUSRQ acceptance, SDBD modes and refusal, stack direction, auto-increment and reset entry pass their vectors, and each policy has a fault that fails for the intended reason | `policies-pending=0` (20 policy ids), `faults=67/67`. The faults per area: interrupt `INTFF_IGNORED`, `PREV_INTERRUPTIBLE_IGNORED`, `INT_ENTRY_ORDER`, `JE_AFTER_BOUNDARY`, `IAB_HARDCODED_1004`; BUSRQ `BUSRQ_PRIORITY`, `GRANT_LOSES_INTERRUPTIBLE`, `RELEASE_LATENCY_IGNORED`; SDBD `SDBD_INDIRECT_INCREMENT`, `SDBD_MODE13_ONE_READ`, `SDBD_TARGET_ACCEPTED`, `SDBD_INTERRUPTIBLE`; stack `PUSH_DIRECTION`, `PULL_DIRECTION`; auto-increment `AUTOINC_R4R5_MISSING`, `AUTOINC_R1R3`; reset `RESET_ENTRY`; C17 `c17=resolved-primary` | `vectors-policy-uncovered`, `vectors-policy-no-fault`, `vectors-pending-covered`, `vectors-addr-uncovered`, `vectors-addr-required-format`, `vectors-branch-uncovered`, `vectors-witness-uncovered`, `faults-wrong-reason`, `faults-not-detected`, `cp1610-c17-inconsistent`, `discrepancies-missing-c17` |
| **SC4 / CPU-05**: each instruction reports an ordered, cycle-stamped access list compared against the manual's bus sequence | every row has a vector with a compared bus list (rule held by `cp1610-vectors`, `rows-pending=0`); `bus-figure-pages=35` (figure-basis vectors cite pdf 27-37; per-page audit in the 02-15 SUMMARY) | `vectors-bus-uncovered`, `vectors-cycles-basis`; faults `DIRECT_ADAR_ORDER`, `INT_ENTRY_ORDER`, `TCI_STROBE_STAMP`, `WAIT_ON_DW`, `WAIT_ON_IAB` |
| **SC5 / API-05**: a symbol check fails on host clock, filesystem, environment, thread or I/O references | `core-imports=2 purity-selftest=5/5`; the `core-symbols` ctest suite runs in both presets | `core-purity-clock`, `-fs`, `-env`, `-thread`, `-io`, `-alloc`, `-rand`, `-asm`, `-float`, `-float-literal`, `-header`, `-prelude`, `-hollow` |
| **PKG-02**: one documented local command runs the same checks as the fast PR gate | `ci-uses=1 ci-workflows=1` (CI runs `sh tools/gate.sh` unchanged); `steps=24/24 controls=190/190` | `ci-parity-missing-gate`, `-conditional`, `-continue-on-error`, `-extra-workflow-target`, `-extra-workflow-unpinned`, `-run-spacing`, `-unpinned-action`, `-write-permissions` |

No criterion lacks a green stat or a control.

**Second-reading independence (C43 residual risk).** Isolated subagents of the same model family as the first reader did the second reading. Independence is therefore limited, and the evidence tier is source-expectation, second-read. The same executor saw both sides and adjudicated all 30 disagreements to the committed values. An independent re-adjudication remains a recommended follow-up. The reader-document clause additions of plan 02-17 are outside every digest the gate checks, so the gate neither depends on them nor shows when they were made.

**Vector total.** The gate reports `vectors=412`. Per file: control 27, register 62, immediate 24, shift 43, direct 19, indirect 35, branch 98, jump 11, sdbd 27, encoding 5, interrupt 39, busrq-halt 22.

- This is above D-16's estimate of 300-360.
- The excess comes from A-13's four directions per conditional branch (branch alone is 98), the two BEXT E codes and the extra SDBD refusal targets.
- The plan's projection of about 440 was high: register, shift, indirect and busrq-halt came in below their projections.

**A-08 split.** The walking skeleton was spread over plans 02-02 (build, harness, generator, minimal stepper, control vectors, tests-inventory), 02-03 (cpu-api and harness suites) and 02-04 (the fault path, with its first fault, and the structural vector checker). Plan 02-04 landed the first fault (INFO 14).

**Isolation attestation (A-12).** The orchestrator transcript of plan 02-17 Task 1 attests reader isolation; the gate does not. The gate checks each marker row against the committed handoff record (`derived_sha256`, reviewer, run id), each `file_sha256` against the current vector file, and the counts.

**Missing evidence.**

- D10 hardware timing: no figure offset or cycle count has been compared with a real CPU.
- BUSRQ release latency (C52, inference).
- HLT and interrupt behaviour on hardware (C40, C45, C54, C56).
- The first hosted gcc 13 run of the final gate, which the orchestrator measures after the phase PR's CI.

## Open items

- **Hosted gate budget.** D-22 sets the hosted gate step at 30 s or less.
  - Status: the final Phase 2 gate (190 controls, parallel runner) has not been timed on the hosted runner. The last hosted figure is a 19 s gate step at 182 controls, after plan 02-07. The next hosted run measures it.
  - If it exceeds 30 s, apply the D-22 levers in order: (1) start the C build in the background at the top of `gate.sh`; (2) parallel control runner, a runner rewrite per A-09 with at most 4 at a time and sorted output (done in this plan); (3) rebuild only the owning TU per fault. No ccache and no `actions/cache`.
  - The local measurements above do not close this item.
- **Local 25 s budget.** Not judged, because the host was never idle. The next candidate lever is to overlap the control runner with the steps (see CI-EFFICIENCY.md).
- **C43.** An independent re-adjudication of the 30 second-reading disagreements is recommended.

## Accepted planning risks

Plan-check iteration 3 recorded these once here; no plan was changed for them.

- Some plans exceed 15 files as counted by the plan checker: 02-02 has 27, 02-17 has 19, 02-04 has 15 (16 after the iteration-3 fix added `tools/check/vector-emulator-names.txt`) and 02-13 has 15.
- Twelve plans sit at 0.85-0.98 of the 100k smart-zone budget with uncalibrated (`confidence: low`) estimates, and an executor may split work at runtime and must say so in its SUMMARY. This plan was not split.
- Acceptance-line pipes that could mask a gate status (INFO "pipe masking") stay as written wherever the same plan's `<verify>` block runs the gate unpiped.

## Files Created/Modified

- `docs/cp1610/reference-notes.md`: D-13 updates, a timing-figures section, inferences against log entries, the open-items status line and the vector second-reading section.
- `tools/check/run-controls.sh`: the lane-parallel runner (per-control work dirs, `good_one`, `ctl_one`, `run_lanes`, sorted collection).
- `.planning/CI-EFFICIENCY.md`: the D-24 amendment, the local measurement section and the open items.

## Decisions Made

- Lever 2 was kept because its output is byte-identical on both pass and fail paths. Levers 1 and 3 were skipped as no-gain on a warm tree, with the reasons recorded.
- The gate-level overlap of the control runner with the steps was recorded as the next candidate but not applied. It changes `gate.sh` scheduling and should first be measured on an idle host.
- The notes label as [OBS] only what was read from a page image in this plan or recorded so earlier. The SARC layout, the JE fault approximation and RSWD from R7 are [INF] with their C-ids.

## Deviations from Plan

### Auto-fixed Issues

**1. [Orchestrator direction] Lever 2 applied on a host that was never idle**
- **Found during:** Task 2.
- **Issue:** The plan says that if the host never gets idle, the loaded numbers are recorded and no code is changed. The orchestrator directed that the D-22 levers be applied in order if, and only if, they are clean.
- **Fix:** Lever 2 was applied after proving byte-identical output on all controls and on every runner failure token, under sh and dash. Levers 1 and 3 were measured as no-gain and not applied.
- **Files modified:** `tools/check/run-controls.sh`.
- **Verification:** gate PASS under sh and dash with `controls=190/190`; `runner-selftest cases=4`.
- **Committed in:** `64a7730`.

**2. [Rule 1 - Stale plan text] Hosted budget open-item wording**
- **Found during:** Task 2.
- **Issue:** The plan's text calls the hosted budget "unmeasured for the C build; the first hosted C run measures it". A first hosted C run already exists: run 42773083392, 19 s gate step at 182 controls, recorded in CI-EFFICIENCY.md.
- **Fix:** The open item now says that the final 190-control gate with the parallel runner is unmeasured on the hosted runner and that the next hosted run measures it. The lever order and the no-cache rule are kept verbatim.
- **Files modified:** `.planning/CI-EFFICIENCY.md`.
- **Committed in:** `64a7730`.

**3. [Minor] The vector total is 412, not about 440**
- The plan's context projected about 440. The gate reports 412, and the SUMMARY records the real per-file counts. No vector was added: the plan does not call for any, and adding one would make its second-read marker stale.

**4. [Minor] Task 3 produced no commit**
- The gate output did not contradict any count in the notes, so the notes were not edited. The evidence table lives in this SUMMARY.

---

**Total deviations:** 2 auto-fixed (1 orchestrator-directed, 1 stale plan text), 2 minor.
**Impact on plan:** No scope creep. No control, token, inventory, vector or core change.

## Issues Encountered

- A first old-versus-new failure-path comparison was invalid. The scratch copy of the old runner resolved its step directory from its own location and reported `control-unknown-step` for every control. The comparison was rerun with the old runner placed temporarily beside the real one (removed right after; never committed), and all outputs were identical.

## User Setup Required

None. No external service configuration required.

## Next Phase Readiness

- Phase 2's plans are all executed. The gate is green under sh and dash: `steps=24/24 controls=190/190 faults=67/67 rows-pending=0 policies-pending=0 second-read=12/12 tests-missing=0 purity-selftest=5/5`.
- The phase is ready for verification. The open items are the hosted gate budget after the next push, the local idle budget and the C43 re-adjudication.

## Self-Check: PASSED

- Files: `docs/cp1610/reference-notes.md`, `tools/check/run-controls.sh` and `.planning/CI-EFFICIENCY.md` were found.
- Commits: `8084cb1` and `64a7730` are ancestors of HEAD, and the ledger count from `5d9ddb7` is 2.
- Task 1 acceptance: C58 2 (was 0); the pdf-figure count is 17 (was 4); DW 6; one `## Vector second reading`; one `C17 status:` line.
- Task 2 acceptance: `D-24` 1; `first hosted C run` 2; `## Open items` above names the hosted budget and the lever order.
- Task 3 acceptance: the evidence table, the vector-total note, the A-08 note and the isolation note are present, and the final gate line contains `steps=24/24`.
- Plan verification: gate PASS under sh (65.5 s) and dash (48.8 s; again 61.3 s at load 15.4 with this SUMMARY present), with `steps=24/24 controls=190/190 faults=67/67 rows-pending=0 policies-pending=0 second-read=12/12 tests-missing=0 purity-selftest=5/5`.
