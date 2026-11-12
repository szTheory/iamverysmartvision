---
phase: 04-headless-runner-and-installed-consumer
plan: 24
subsystem: tools/check, data/expected, tests
tags: [run-04, important-gates, registry, controls, mutants, faults, audit, D-46, RUN-04]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-01..04-23: the Phase 4 faults (run, runner, machine digest rows), controls (ci-bans, ci-log, runner-hygiene, define pins, listing, golden, faults-verdict runner sites), in-step selftests (pkg-selftests.ids, ci-summary-selftests.ids) and suite self-controls (runner-process.judge.*, runner-process.mutant.*)"
provides:
  - "tools/check/steps/important-gates.sh: pure awk checker (about 0.15 s); registry ids resolved per mechanism (fault: data/expected/faults/*.ids; control: data/expected/controls/*.ids; mutant: pkg-selftests.ids, ci-summary-selftests.ids, or a `<suite>.judge.<slug>` / `<suite>.mutant.<slug>` test id); every Phase 4 requirement must have a line; tokens important-gates-unknown-id, important-gates-uncovered-gate; stats gates=<n> lines=<n>"
  - "data/expected/important-gates.ids: 102 lines over 26 gates, every Phase 4 requirement covered"
  - "gap fills: control api-stop-orphan, suite self-controls pack.judge.patched-word and runner-process.judge.outcome-uncovered"
  - "controls important-gates-unknown-id, important-gates-uncovered-gate; toy registry, toy controls inventory and toy pkg-selftests inventory in the good tree"
affects: [04-25]

actuals:
  tokens: 14000
  tasks: 3
  commits: 3
plan_head_before: c1cf910a9b60f8170bbd5e29e41095d9f25ba487
plan_head_after: badf00483c5833553aecf2219aaa7a0aa9c0709b

tech-stack:
  added: []
  patterns:
    - "Registry over inventories: a free gate name, a mechanism and an id per line; the id must exist in its mechanism's inventory, so a renamed or deleted fault, control or selftest fails the gate"
    - "Suite self-control (`<suite>.judge.<slug>`): the suite runs its own comparison function on a broken input and passes only when that function fails for the intended finding (the runner-process pattern, now also in pack)"

key-files:
  created:
    - tools/check/steps/important-gates.sh
    - data/expected/important-gates.ids
    - data/expected/gate-steps/important-gates.ids
    - data/expected/controls/important-gates.ids
    - tools/check/controls/important-gates-unknown-id/
    - tools/check/controls/important-gates-uncovered-gate/
    - tools/check/controls/api-stop-orphan/
    - tools/check/controls/_good/tree/data/expected/important-gates.ids
    - tools/check/controls/_good/tree/data/expected/controls/toy.ids
    - tools/check/controls/_good/tree/data/expected/pkg-selftests.ids
  modified:
    - data/expected/controls/api-effects.ids
    - tests/pack-tap.sh
    - data/expected/tests/pack.ids
    - tests/runner-process-tap.sh
    - data/expected/tests/runner-process.ids

key-decisions:
  - "The unit of the audit is the gate (a step, or a test family of a suite), not each test case; cases without a fault of their own are listed as residuals with where their negative evidence lies"
  - "Mutant ids also resolve as `<suite>.mutant.<slug>` and `<suite>.judge.<slug>` of any suite (not only runner-process.judge.*), so the D-32 program mutations and the new pack self-control can be registered"
  - "Gap fills are a control and two suite self-controls, no new fault (A-06, T-04-74): faults=149/149 unchanged"
  - "RUN-04 is marked Complete: every important Phase 4 gate is registered with a mechanism that the gate shows failing for its intended reason, and the registry itself is gate-checked with two controls"

duration: about 35 min of edits plus gates (first task commit 20:30, last 20:50), plus one container gate
completed: 2026-11-11
---

# Phase 4 Plan 24: the important-gate registry Summary

**RUN-04 is now checked by the gate, not just stated. `data/expected/important-gates.ids` ties each important Phase 4 gate to named faults, controls or mutants, 102 lines over 26 gates. The pure step `important-gates` refuses an id that no inventory of its mechanism lists, and refuses a Phase 4 requirement that has no line. The audit found three important gates with no negative evidence, and each now has one: the orphan direction of the define pins (control `api-stop-orphan`), the two-parser word comparison of suite `pack` (`pack.judge.patched-word`) and the outcome-coverage check of suite `runner-process` (`runner-process.judge.outcome-uncovered`).**

## What was built

- Task 1 (tracer, `587f7fc`): the step, the gate-steps inventory and a first registry with one line per requirement (`gates=10 lines=10`, `steps=33/33`).
- Task 2 (`ed6a358`): the audit below, the full registry and the three gap fills (`gates=25 lines=100`, `controls=274/274`, tests-expected 2776 -> 2780).
- Task 3 (`badf004`): toy registry and inventories in the good tree, controls `important-gates-unknown-id` and `important-gates-uncovered-gate`, their two registry lines (`gates=26 lines=102`, `controls=276/276`).

## Audit of the Phase 4 gates

The criteria (D-46): (1) sole evidence for a requirement or claim; (2) can pass vacuously (inventory, digest or golden comparison, outcome classifier, refusal/no-state-change check, verdict over producer output); (3) its failure class escaped once.

| Gate | Important? | Mechanism ids (registry REQ) |
|---|---|---|
| suite `run`: bound, overshoot, stop, slicing (run.target, .overshoot, .single-step, .stop, .slice, .refuse) | yes (1: sole evidence for API-02 slicing invariance; 2: compared with a stepped reference) | faults RUN_TARGET_UNCHECKED, RUN_STOP_BEFORE_OVERSHOOT, RUN_FAULT_STEPS, RUN_BUDGET_CARRY (API-02) |
| suite `run`: info growth (run.info) | yes (2: write bounded by caller struct_size, A-02) | fault INFO_TAIL_UNGUARDED (API-02) |
| suite `run`: buffers and precedence (run.buffers, run.prec) | yes (1: sole evidence for API-04; 2: refusal before any state change) | faults RUN_CAPACITY_AFTER_STEP, OUTBUF_PARTIAL_WRITE (API-04) |
| suite `run`: observation (run.observe, run.peek, run.cpu-state) | yes (2: no-side-effect check; the runner's assertions read through it) | faults PEEK_ADVANCES_TIME, PEEK_GATE_IGNORED (RUN-01); run.cpu-state: see residuals |
| suite `run`: input (run.input, run.fuzz) | yes (2: timing, refusal and log comparisons) | faults INPUT_APPLY_AT_RUN_START, RUN_EVENT_AT_TARGET_ONLY, INPUT_NO_FLOOR_CHECK, RUN_EVENTS_UNCHECKED (RUN-01); INPUT_LOG_SLICE_RELATIVE_TICK, INPUT_STATIC_CARRY (RUN-05) |
| suite `machine`: digest rows 16..18 (machine.digest, machine.reset) | yes (2: digest comparison) | faults DIGEST_DOMAIN_SWAPPED, OBS_NOT_CLEARED_ON_RESET (RUN-05) |
| suite `run`: boot stub and diagnostic (run.stub, run.diag) | yes (1: in-process RUN-06 predicate) | run.stub.* and run.diag.slice fail under RUN_STOP_BEFORE_OVERSHOOT, RUN_BUDGET_CARRY, INPUT_STATIC_CARRY; the predicate's sensitivity is shown end to end by runner-process.mutant.* (below); run_diag.c also checks the predicate shape (1 halted, 7 reg, 10 mem clauses) |
| suite `run`: v1 sizes, stop names (run.struct, run.stop-name) | not important: fixed literals, cannot pass vacuously; the sizes are pinned again in data/expected/pkg-consumer.golden; names are text, the numbers are pinned by the define pins | not registered |
| suite `runner`: outcome classifier (runner.outcome) | yes (2: outcome classifier) | faults OUTCOME_GUEST_AS_HARNESS, OUTCOME_TIMEOUT_AS_GUEST, OUTCOME_MISSING_FIXTURE_EXIT0, OUTCOME_CANCEL_AS_TIMEOUT (RUN-02) |
| suite `runner`: scenario parser (runner.parse, runner.fuzz) | yes (2: refusal checks, T-04-49/50) | faults RUNNER_DOTDOT_ACCEPTED, RUNNER_LINE_CAP_UNCHECKED (RUN-01) |
| suite `runner`: executor and report (runner.exec) | yes (2: verdict over producer output) | faults RUNNER_ASSERT_ALWAYS_PASS, RUNNER_STOP_EXPECT_IGNORED, RUNNER_KEY_ORDER_SWAPPED, RUNNER_OVERSHOOT_NOT_REPORTED (RUN-01) |
| suite `runner`: slicing, wall cap, cancel (runner.time) | yes (2: classifier) | faults RUNNER_CANCEL_IGNORED, RUNNER_WALL_AS_GUEST_FAIL (RUN-02); RUNNER_SLICE_DROPS_REMAINDER (API-02) |
| suite `runner-process`: scenarios, repeat, slices, cwd | yes (1: the only test of the real executable; 2: exit code and report comparisons) | mutants runner-process.judge.exit-mismatch, .outcome-exit-disagree (RUN-02), .no-report-end (RUN-01), .body-differs (RUN-05) |
| suite `runner-process`: program mutations (D-32) | yes (1: RUN-06 predicate not vacuous) | mutants runner-process.mutant.ADDI-SUBI, .BNZE-BZE, .PSHR-MVOAT, .INCR-DECR (RUN-06): each must be guest-fail naming its clause |
| suite `runner-process`: outcomes-covered | yes (2: inventory; every outcome has a running scenario, RUN-02) | **gap fill** runner-process.judge.outcome-uncovered (RUN-02) |
| suite `runner-process`: outcomes-consistent | not important: --help, docs and inventory agree; no requirement is asserted from it (RUN-02 rests on the scenarios and the outcome faults) | not registered |
| suite `pack` | yes (2: two parsers compared; the image bytes the diagnostic runs equal the reviewed listing, RUN-06) | **gap fill** pack.judge.patched-word (RUN-06) |
| step `listing` | yes (1 and 2) | controls listing-bits-mismatch, listing-row-mismatch, listing-operand-count, listing-range, listing-header, listing-unlisted, bootstub-size, bootstub-row, bootstub-vector (RUN-06) |
| step `golden` | yes (1: golden header tier and oracle, RUN-05; 2: body and dev/release comparisons) | controls golden-header-missing-tier, golden-header-missing-oracle, golden-self-recorded-wrong-tier, golden-empty-domain-observed, golden-mismatch, golden-dev-release-diff, golden-host-dependent-unmarked, golden-host-dependent-golden, golden-user-scenario-tracked (RUN-05); golden-expect-drift (RUN-06) |
| step `pkg-install` | yes (1: sole evidence for PKG-01) | mutants core-src-include, missing-symbol, ghost-decl, unprefixed, extra-file, baked-path, version-refused, old-prefix, leak-detector, golden-drift (PKG-01) |
| step `ci-summary` | yes (1: skips and failures visible in CI, PKG-03) | mutants render-headline, skips-none, skips-listed, fail-cap-40, backticks-stripped, unwritable-keeps-exit, unset-writes-nothing (PKG-03); slowest-eight is presentation only, not registered |
| gate measurement lines (gate-env, gate-cpu) | yes (1: per-run CPU measurement, PKG-04) | mutants cpu-dash-format, cpu-sh-format, cpu-garbage-unknown (PKG-04) |
| ci-parity extensions | yes (1: no cache, upload, secrets, matrix, other runner or timeout, PKG-03) | controls ci-parity-action-cache, -action-upload, -secrets, -matrix, -runs-on, -timeout (PKG-03) |
| step `ci-log` | yes (1: hosted run record format, PKG-04) | controls ci-log-header, ci-log-fields, ci-log-duplicate, ci-log-order (PKG-04) |
| step `runner-scan` | yes (3: the hosted-only POSIX-name class escaped once; it also keeps the runner on the public API) | controls runner-scan-header, runner-scan-internal-include, runner-scan-posix-define (RUN-01) |
| define pins (api-effects section 6) | yes (2: inventory comparison of append-only pins) | controls api-stop-unpinned, api-stop-renumbered (API-02), api-reason-unpinned (API-04), api-input-bit-unpinned, api-input-bit-duplicate (RUN-01); **gap fill** api-stop-orphan (API-02): before it, api-define-orphan appeared only in FORBID lists |
| integrity extensions (SPDX and ASCII over runner/ and tools/**/*.c) | not important: hygiene lint, no escape | controls exist (runner-hygiene.ids), not registered |
| c-build unlisted-source (runner/, tools/pack/) | not important: a source left out of the build carries no evidence; a fault site in it shows as fault-not-detected | not registered |
| fault framework over runner TUs | yes (2: verdict over producer output) | controls faults-orphan-site-runner, faults-missing-site-runner (RUN-04) |
| step `important-gates` | yes (1: sole evidence for RUN-04) | controls important-gates-unknown-id, important-gates-uncovered-gate (RUN-04) |
| tests/expect-inc.cmake | covered: it stops the build on a line it cannot convert or a file without asserts; the predicate shape check and golden-expect-drift guard the rest | golden-expect-drift (RUN-06) |
| tests/faults.cmake stem uniqueness | not important: a configure error, fails loudly, cannot pass vacuously | not registered |
| tools/ci-record.sh | not a gate: orchestrator only, never run by the gate; its rows are checked by ci-log | not registered |

Residual cases (no fault fails them; they are inside a gate whose negative evidence is above): run.cpu-state.* and run.diag.predicate (the runner reads registers through `ivs_machine_cpu_state` and words through `ivs_machine_peek`, and runner-process.mutant.* fail through that path); run.diag.repeat (runner-process.repeat.* with runner-process.judge.body-differs); run.info.timebase (compared with profile literals); run.input.refuse.extra-controller-word (the input refusal family has INPUT_NO_FLOOR_CHECK and RUN_EVENTS_UNCHECKED, but this one comparison has no fault of its own; see open items).

## Gap fills

| Gap | Fill | Shown failing for its reason |
|---|---|---|
| api-define-orphan had no control (a pin whose define was removed would only show as a FORBID miss) | control `api-stop-orphan`: toy pin `IVS_STOP_GHOST 7`, EXPECT `FAIL api-define-orphan: IVS_STOP_GHOST` | control runner: `control ok api-stop-orphan` |
| suite pack: nothing showed the words comparison fails on a difference | `pack.judge.patched-word`: the comparison, factored into `dumps_agree`, runs on `ivs_pack --dump-words --patch <first addr>=<other word>` and must fail with a diff of exactly the patched line | a scratch copy with the `cmp` test disabled gives `not ok 4 pack.judge.patched-word` / `judge-vacuous` |
| suite runner-process: nothing showed outcomes-covered fails when a demonstration is gone | `runner-process.judge.outcome-uncovered`: the check, factored into `check_covered`, runs with every scenario of the first outcome removed and must fail with `outcome-uncovered` | a scratch copy with that finding removed gives `not ok 27 runner-process.judge.outcome-uncovered` / `judge-vacuous` |

Each gap fill landed with its inventory line in the same commit (`ed6a358`). No new fault: `faults=149/149` unchanged.

## Performance

- `important-gates` takes 0.15 s standalone; `step-time important-gates 0` locally, `2` in the container (amd64 emulated on arm64 while the laptop was loaded; the step starts about 55 short awk processes, one per inventory file, which emulation makes slow). The hosted native value goes to 04-25 with the other step times.
- The three gap fills add one control and two TAP cases per preset (each well under 0.1 s: pack-tap 0.56 s and runner-process-tap 3.5 s standalone, the same as before within noise).
- Gate total 26 s in Task 1, then 42-45 s with load average about 12 on the laptop (other work running beside it); not attributable to this plan's checks.

## Counts

Start: `steps=32/32 controls=273/273 faults=149/149 tests-expected=2776 tests-skipped=2`.
End (for plan 04-25): `steps=33/33 controls=276/276 faults=149/149 tests-expected=2780 tests-executed=2778 tests-skipped=2 tests-missing=0`, `gates=26 lines=102`, `pkg-selftest=10/10`. Controls: 273 + 2 (important-gates) + 1 gap fill (api-stop-orphan) = 276.

Acceptance checks: the requirement loop over `data/expected/important-gates.ids` prints nothing; `sh tools/check/run-controls.sh | grep -c -E 'control ok +important-gates-(unknown-id|uncovered-gate)$'` = 2; the three task verify commands exit 0.

## Deviations from Plan

**1. [Rule 2] Mutant resolution widened.** The must-have lists `runner-process.judge.*` as the suite mutant source. The step accepts any `<suite>.judge.<slug>` or `<suite>.mutant.<slug>` test id of `data/expected/tests/*.ids`. This is a superset: the 04-20 SUMMARY points at runner-process.mutant.* for D-32, and the pack gap fill is a judge case of suite pack.

**2. [Rule 2] Gap fills in suites.** Two of the three gap fills are suite self-controls (new test ids in pack and runner-process, tests-expected +4 over both presets) rather than controls, because the gaps sit in TAP scripts that no pure step can drive. Both scripts' comparisons were factored into functions so the self-control runs the same code as the real case.

**3. `runner-process` finding names.** The outcomes-covered findings are now `outcome-uncovered:` and `outcome-unknown:` (they were all `outcomes:`), so the self-control can require its intended reason. The empty-inventory finding stays `outcomes:`.

**4. Registry header and REQ mapping.** Gates that serve two requirements appear under each (for example golden-expect-drift under RUN-06, the other golden controls under RUN-05). Gate names in the registry are free names, not step names, so test families of one suite are separate gates.

## Open Items

- run.input.refuse.extra-controller-word has no fault of its own (one comparison, index at or above the profile's controller count). A fault `INPUT_EXTRA_CONTROLLER_UNCHECKED` in core/src/machine/machine_input.c would close it at the cost of one fault executable; left for the input work of Phase 5 or for 04-25 if the cost review allows it.
- Plan 04-25 measures the gate cost of this plan's additions (T-04-74) and re-lists RUN-04 at close-out.
- Carried: C14 and C63-C68 revisited in Phase 5/6/7; `API-02`, `PKG-03`, `PKG-04` Pending; `pending-resolvers=17`, `unknown-fields=16`. `sh tools/hygiene.sh --history` still fails on earlier planning commits (this plan adds no home paths). The orchestrator records hosted runs with `sh tools/ci-record.sh <run-id>`; the hosted `step-time pkg-install` still goes to CI-EFFICIENCY.md.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=33/33 controls=276/276 ... faults=149/149 ... gates=26 lines=102 ... pkg-selftest=10/10 ... tests-expected=2780 tests-executed=2778 tests-skipped=2 tests-missing=0 ...`, `gate-time total=44`.
- `dash tools/gate.sh`: the same line, `gate: PASS`, `gate-time total=45`.
- `sh tools/gate-linux.sh` (Ubuntu 24.04 container, x86_64, gcc 13.3.0, cmake 3.28.3): `gate: PASS steps=33/33 controls=276/276 ... cc=GNU c-presets=2 fault-runs=149 ... faults=149/149 ... gates=26 lines=102 ... archive=sha256 ... consumer-lines=26 pkg-selftest=10/10 ... tests-expected=2780 tests-executed=2778 tests-skipped=2 tests-missing=0`, `gate-time total=184`, last line `gate-linux: exit 0 (log build/gate-linux.log)`. The positive control and both important-gates controls also pass there (Ubuntu /bin/sh is dash).

## Threat Flags

None. The step reads only `data/expected/`; the two TAP additions write only under their mktemp dirs.

Mitigations from the plan's threat model:
- T-04-73: ids resolved against the real inventories; every Phase 4 requirement must appear; two controls.
- T-04-74: no new fault; one control and two cheap TAP cases; measured in 04-25.
- T-04-SC: no package installs.

## Self-Check: PASSED

- Files exist: tools/check/steps/important-gates.sh, data/expected/important-gates.ids, data/expected/gate-steps/important-gates.ids, data/expected/controls/important-gates.ids, tools/check/controls/{important-gates-unknown-id,important-gates-uncovered-gate,api-stop-orphan}/, the three good-tree inventories.
- Commits on HEAD: 587f7fc, ed6a358, badf004.
