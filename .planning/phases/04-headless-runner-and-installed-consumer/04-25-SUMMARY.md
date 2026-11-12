---
phase: 04-headless-runner-and-installed-consumer
plan: 25
subsystem: planning records, CI budget
tags: [close-out, evidence, D-45, D-47, records, API-02, API-04, RUN-01, RUN-02, RUN-04, RUN-05, RUN-06, PKG-01, PKG-03, PKG-04]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-01..04-24 and 04-LEVER-01/02: run API, input, digests, boot stub and diagnostic, runner and process suite, goldens, installed consumer, CI record and summary, important-gates registry"
provides:
  - "SC1-SC5 and ten-requirement evidence table from one fresh gate run"
  - "C24, C25 and ROADMAP decision-gate rows D4, D6, D12 reconciled, checked by a fail-first per-id script"
  - "CI-EFFICIENCY.md Phase 4 close-out: D-45 evaluated on 23 hosted rows; hosted budget re-baselined (median gate_s 75 s, median cpu_s 200)"
  - "AUTONOMY.md: branch-protection ruleset recommendation with the exact gh commands, and the release workflow design (recorded, not implemented)"
affects: [phase-05, verify-work]

actuals:
  tokens: 16000
  tasks: 3
  commits: 2
plan_head_before: 944a958913fe381f2f23b6d801d63bbe09a1dabb
plan_head_after: 17aa8192cfbb6a8cf836fea78370c8195bfa1e68

tech-stack:
  added: []
  patterns:
    - "Hosted budget on two medians: wall (gate_s) and cost (cpu_s), each over recorded rows, never a single run"

key-files:
  created:
    - .planning/phases/04-headless-runner-and-installed-consumer/04-25-SUMMARY.md
  modified:
    - data/discrepancies.json
    - .planning/ROADMAP.md
    - .planning/CI-EFFICIENCY.md
    - .planning/AUTONOMY.md

key-decisions:
  - "Hosted gate budget re-baselined: the 30 s and 40 s wall budgets are superseded by a median gate_s of 75 s plus a median cpu_s of 200; revisit when the median of the last 5 gate_s exceeds 90 s, any run exceeds 120 s, or the last-5 cpu_s median exceeds 200. Reason: wall time is dominated by hosted machine speed (1.7x on the same code) and the gate now carries Phase 3-4 evidence; no check weakened"
  - "C24 and C25 stay open with unknown-default-chosen policies that name the Phase 4 decisions (D12 as D-41..D-45; project boot stub contract 1) and test resolvers"
  - "Branch protection and the release workflow are recorded as owner/orchestrator actions with exact commands; no agent applies settings"
  - "API-02, PKG-03 and PKG-04 marked Complete at close-out: each has a green stat, a named family or step and registered faults, controls or selftests (table below); PKG-03/04 rest on 22 successful hosted PR runs recorded in ci-runs.tsv"

requirements-completed: [API-02, API-04, RUN-01, RUN-02, RUN-04, RUN-05, RUN-06, PKG-01, PKG-03, PKG-04]

duration: about 40 min (2026-11-11 20:57 to about 21:40, including six gate runs and one container gate)
completed: 2026-11-11
---

# Phase 4 Plan 25: close-out Summary

**Phase 4 closes on one fresh gate run mapped to SC1-SC5 and the ten requirements, every row with a green stat, a test family or step and registered negative evidence. The C24/C25 records and the D4, D6, D12 decision-gate rows now say what the phase decided (shown by a per-id script that failed on exactly those five ids first). The D-45 rule fired on the hosted rows; the hosted budget is re-baselined to a 75 s median `gate_s` plus a 200 median `cpu_s`, because the rows show machine speed, not any one step, dominating wall time. Branch protection and the release design are recorded for the owner/orchestrator, not applied.**

## Fresh gate run (Task 1 tracer)

`sh tools/gate.sh` at 944a958 (load 6.1), exit 0, `gate-time total=50`:

```
gate: PASS steps=33/33 controls=276/276 api-calls=14 api-statuses=14 api-rows=38 pinned-defines=90 cc=AppleClang c-presets=2 fault-runs=149 core-imports=4 purity-selftest=7/7 ctest-runs=2 ci-runs=23 ci-uses=1 ci-workflows=1 ci-summary-selftest=11/11 claims=8 provisional=7 ... discrepancies=68 pending-resolvers=17 open=63 faults=149/149 ... goldens=4 reports=10 host-dependent=2 ... gates=26 lines=102 ... listings=2 listing-words=78 manifests=4 ... prefix-files=10 archive=members public-symbols=14 defined=55 hdr-alone=8/8 consumer-lines=26 cxx-link=ok pkg-selftest=10/10 ... unknown-fields=16 runner-files=8 ... tests-expected=2780 tests-executed=2778 tests-skipped=2 tests-missing=0 ...
```

The tracer changed no tracked file, so it has no commit. The two skips are `needs-user-exec` (`skip-ok`, missing-fixture outcome, exit 13) in both presets.

## Evidence table

| Row | Gate stats | Test families / steps | Negative evidence (fault, control, selftest or mutant; registered in `data/expected/important-gates.ids`) |
|-----|------------|-----------------------|---------------------------------------------------------------------|
| SC1 scenario on fresh state; original diagnostic; boot path labelled | `tests-missing=0`, `reports=10`, `listings=2 listing-words=78`, `claims=8` | `runner-process.scenario.diag-boot-pass` (real `ivs-run`, exit 0); report line `exec: project boot stub, not an EXEC sha256=2b7cfb09...` in `build/gate/<preset>/runner/diag-boot-pass.report`; predicate `data/golden/diag-cpu-1.expect` (`# tier: original-diagnostic`, `# oracle: independent-derivation`); `run.stub.*`, `run.diag.*`; step `listing` | mutants `runner-process.mutant.ADDI-SUBI`, `.BNZE-BZE`, `.PSHR-MVOAT`, `.INCR-DECR` (each guest-fail naming its clause); `pack.judge.patched-word`; controls `listing-*`, `bootstub-size`, `bootstub-row`, `bootstub-vector`, `golden-expect-drift` |
| SC2 distinct outcomes, one scenario each; important gates controlled | `gates=26 lines=102`, `reports=10 host-dependent=2` | `data/expected/runner-scenarios.ids` (every outcome has a running scenario, checked by outcomes-covered); `runner.outcome.*`, `runner.time.*`; step `important-gates` | faults OUTCOME_GUEST_AS_HARNESS, OUTCOME_TIMEOUT_AS_GUEST, OUTCOME_MISSING_FIXTURE_EXIT0, OUTCOME_CANCEL_AS_TIMEOUT; `runner-process.judge.outcome-uncovered`; controls `important-gates-unknown-id`, `important-gates-uncovered-gate` |
| SC3 cycle target, overshoot, slicing, repeat, goldens, buffer refusal | `faults=149/149`, `goldens=4`, `pinned-defines=90` | `run.target.*`, `run.slice.*`, `run.buffers.*`, `run.diag.slice`, `machine.digest.*`, `runner-process.repeat.*`; step `golden` | faults RUN_TARGET_UNCHECKED, RUN_STOP_BEFORE_OVERSHOOT, RUN_BUDGET_CARRY, RUN_CAPACITY_AFTER_STEP, OUTBUF_PARTIAL_WRITE, DIGEST_DOMAIN_SWAPPED; controls `golden-mismatch`, `golden-dev-release-diff`, `golden-header-missing-tier`, `golden-header-missing-oracle` |
| SC4 installed consumer in a relocated prefix | `prefix-files=10 archive=members public-symbols=14 defined=55 hdr-alone=8/8 consumer-lines=26 cxx-link=ok pkg-selftest=10/10` | step `pkg-install` (`step-time pkg-install` 2-4 s locally) | selftests baked-path, core-src-include, extra-file, ghost-decl, golden-drift, leak-detector, missing-symbol, old-prefix, unprefixed, version-refused |
| SC5 CI = local gate; skips visible; no caches/uploads; per-run durations | `ci-workflows=1 ci-uses=1 ci-summary-selftest=11/11 ci-runs=23` | steps `ci-parity`, `ci-summary`, `ci-log`; `gate-env`/`gate-cpu` lines; `.planning/ci-runs.tsv` (queue, setup, build, test, gate, total, cpu per run) | controls `ci-parity-action-cache`, `-action-upload`, `-secrets`, `-matrix`, `-runs-on`, `-timeout`, `ci-log-header`, `-fields`, `-duplicate`, `-order`; selftests render-headline, skips-listed, skips-none, fail-cap-40, cpu-sh-format, cpu-dash-format, cpu-garbage-unknown |
| API-02 bounded run, overshoot, slicing invariance | `api-calls=14`, `pinned-defines=90`, `faults=149/149` | `run.target.*`, `run.overshoot.*`, `run.stop.*`, `run.slice.*`, `run.diag.slice`, `runner.exec.*` (frames as a nominal conversion) | faults RUN_TARGET_UNCHECKED, RUN_STOP_BEFORE_OVERSHOOT, RUN_FAULT_STEPS, RUN_BUDGET_CARRY, INFO_TAIL_UNGUARDED, RUNNER_SLICE_DROPS_REMAINDER; controls `api-stop-unpinned`, `api-stop-renumbered`, `api-stop-orphan` |
| API-04 caller-owned buffers, refusal before state change | `api-statuses=14`, `tests-missing=0` | `run.buffers.*`, `run.prec.*` | faults RUN_CAPACITY_AFTER_STEP, OUTBUF_PARTIAL_WRITE; control `api-reason-unpinned` |
| RUN-01 scripted scenario reports time, stop, outputs, assertions | `reports=10`, `runner-files=8` | `runner.parse.*`, `runner.exec.*`, `runner.fuzz.*`, `run.input.*`, `run.observe.*`, `run.peek.*`, `runner-process.*`; step `runner-scan` | faults RUNNER_ASSERT_ALWAYS_PASS, RUNNER_STOP_EXPECT_IGNORED, RUNNER_KEY_ORDER_SWAPPED, RUNNER_OVERSHOOT_NOT_REPORTED, RUNNER_DOTDOT_ACCEPTED, RUNNER_LINE_CAP_UNCHECKED, INPUT_APPLY_AT_RUN_START, INPUT_NO_FLOOR_CHECK, RUN_EVENTS_UNCHECKED, PEEK_ADVANCES_TIME; controls `runner-scan-*`, `api-input-bit-*`; mutant `runner-process.judge.no-report-end` |
| RUN-02 distinct outcomes | `reports=10 host-dependent=2` | `runner.outcome.*`, `runner.time.*`, `runner-process.scenario.*` (exit codes) | faults OUTCOME_* (four), RUNNER_CANCEL_IGNORED, RUNNER_WALL_AS_GUEST_FAIL; mutants `runner-process.judge.exit-mismatch`, `.outcome-exit-disagree`, `.outcome-uncovered` |
| RUN-04 important gates have named-fault controls | `gates=26 lines=102`, `faults=149/149`, `controls=276/276` | step `important-gates` over the real inventories; `faults-verdict` over runner TUs | controls `important-gates-unknown-id`, `important-gates-uncovered-gate`, `faults-orphan-site-runner`, `faults-missing-site-runner` |
| RUN-05 per-domain digests, repeat match, golden headers | `goldens=4 reports=10 host-dependent=2` | `machine.digest.*`, `run.input.*` (log digest against a literal oracle), `runner-process.repeat.*`; step `golden` | faults DIGEST_DOMAIN_SWAPPED, OBS_NOT_CLEARED_ON_RESET, INPUT_LOG_SLICE_RELATIVE_TICK, INPUT_STATIC_CARRY; controls `golden-*` (nine); mutant `runner-process.judge.body-differs` |
| RUN-06 original diagnostic from reset via project boot path | `listings=2 listing-words=78`, `claims=8 provisional=7` | `run.stub.*`, `run.diag.*`, `runner-process.scenario.diag-boot-pass`, `pack.*`; step `listing` | mutants `runner-process.mutant.*` (four); `pack.judge.patched-word`; controls `listing-*` (six), `bootstub-*` (three), `golden-expect-drift` |
| PKG-01 installed consumer, relocated prefix | `consumer-lines=26 cxx-link=ok pkg-selftest=10/10` | step `pkg-install` (moved prefix, deleted original, `ninja -t deps` and link-line leak checks, C++ link, version tie) | the ten pkg selftests (row SC4) |
| PKG-03 CI fast gate, visible skips, no caches/uploads | `ci-workflows=1 ci-uses=1 ci-summary-selftest=11/11`; 22 successful hosted PR runs in `ci-runs.tsv` | steps `ci-parity`, `ci-summary` | controls `ci-parity-*` (six); selftests render-headline, skips-none, skips-listed, fail-cap-40, backticks-stripped, unwritable-keeps-exit, unset-writes-nothing |
| PKG-04 per-run critical path recorded | `ci-runs=23` | step `ci-log`; `gate-env`, `gate-cpu`, `step-time` lines; `tools/ci-record.sh` (orchestrator) | controls `ci-log-header`, `-fields`, `-duplicate`, `-order`; selftests cpu-sh-format, cpu-dash-format, cpu-garbage-unknown |

No row lacks a green stat or a mechanism, so there is no `## Gaps` section.

## Record reconciliation (Task 2)

Throwaway script `build/phase4-records.cmake` (gitignored, not committed): `string(JSON ...)` over `data/discrepancies.json`, `data/claims.json` and the profile, `file(STRINGS ...)` over the ROADMAP decision-gate rows. Stale rules: C24 resolver `pending: D12`; C25 policy "no boot stub" or resolver `pending: Phase 4`; ROADMAP D4, D6, D12 status `Open`. Current rules: C14 policy has `logical switch state`; C15 open; C63 onward match the schema (status, `unknown-default-chosen:`, resolver prefix `pending:`/`measurement:`/`test:`) and number at least six (C63-C67 input, C68 stub convention); CL-0008 tier `original-diagnostic`, standing `provisional`, artifact exists; profile `input.controllers` value 2.

Before the edits (`cmake -DROOT="$PWD" -P build/phase4-records.cmake`, exit 1):

```
FAIL C24: resolver still starts pending: D12
FAIL C25: default_policy says no boot stub or resolver starts pending: Phase 4
FAIL D4: ROADMAP decision-gate row status is Open
FAIL D6: ROADMAP decision-gate row status is Open
FAIL D12: ROADMAP decision-gate row status is Open
CMake Error at build/phase4-records.cmake:156 (message):
  phase4-records: 5 failing rule(s)
```

After the edits (exit 0):

```
phase4-records: clean
```

Edits (commit `0e045df`): C24 policy `unknown-default-chosen: one gate job (D12, delegated decision); reverse on ...`, resolver `test: ci-parity bans and ci-log`; C25 policy `unknown-default-chosen: project boot stub contract 1 (plan 04-14), labelled "project boot stub, not an EXEC"; no EXEC replacement`, resolver `test: run.stub.path, run.stub.trap and the listing step's bootstub guards`; notes extended with the deciding plans; ROADMAP rows D4, D6, D12 now `Delegated decision 2026-11-11 (PROJECT.md; Phase 4 D-25, D-12, D-41)`. Both C24 and C25 stay `open` (schema: open/partial/resolved only; they remain defaults with reversal triggers). Gate effect: `pending-resolvers=17` -> `15`.

## Budget evaluation and records (Task 3)

- Local: three sh gates 50, 45, 53 s (median 50 s; about 166 cpu-s; load 6-9), dash 45 s. Largest steps: `controls` 24-28 s (overlapped), `c-faults` 11-12, `c-tests` 6-7, `cp1610-vectors` 5-6, `c-purity` 5; `pkg-install` 2-4; `important-gates` 0 (T-04-74: the 04-24 additions cost nothing measurable).
- Hosted (23 orchestrator-recorded rows): `gate_s` 39-75, median 59; last 3 (59, 59, 75) median 59 > 40: lever trigger fires; one run 75 > 60 and last-5 median 59 > 50: revisit trigger fires. `cpu_s` 114-225, median 164.
- Decision (recorded in `.planning/CI-EFFICIENCY.md`, section "Phase 4 close-out"): the 30 s then 40 s wall budgets are superseded; new budget median `gate_s` 75 s and median `cpu_s` 200; revisit when the last-5 `gate_s` median exceeds 90 s, any run exceeds 120 s, or the last-5 `cpu_s` median exceeds 200. This is a re-baseline made because the gate now carries Phase 3-4 evidence (controls 190 -> 276 since the 30 s budget, faults 113 -> 149 since the 40 s budget, four new suites in two presets, the consumer build) and because same-code hosted runs swing about 1.7x (41 s vs 67 s, `c-build` 21 vs 29 s, `cpu_s` 125 vs 211). The remaining levers (background `c-build` only on idle cores; batch the cp1610-vectors JSON re-parse, about 5 cpu-s; cut `c-faults` per-fault suite reruns; two-job split last) each add structural risk for a gain inside the noise, so none is applied now. No check weakened; no CI shape change.
- `.planning/AUTONOMY.md`: section 1 "Branch protection on main (Phase 4 D-47, recommended, not applied by agents)" with the ruleset JSON and the exact `gh api` create and read commands; section 5 "Release workflow design (Phase 4 D-47, recorded; implemented at the first release)".

## Honest caveats

- **CL-0008 is provisional, with shared-misreading risk.** The listing, the hand derivation of `data/golden/diag-cpu-1.expect` and the core were all written by the same model family from the same manual pdf pages; agreement among them is not independent evidence about hardware. The claim excludes R7, the flags at halt and machine time (and the stack word [0340]). Its tier is original-diagnostic; `reset.r7` (tier 4) and `cartridge.slots` (unknown-default-chosen) keep it provisional. The optional libasm cross-check of the diagnostic's encoding was **not run** (D4: libasm is a non-gating optional check; no assembler is a dependency).
- **Digest goldens are repeatability, not accuracy.** `data/golden/*.digests` pin what this core produced (self-recorded tier); they detect change, not error.
- **Video and audio domains carry no content in Phase 4.** The buffers, refusal and digest plumbing exist; no device writes pixels or samples until Phases 6-7, so their digests are `none` in the goldens.
- **Input is guest-invisible until Phase 5.** `run.input.guest-invisible` asserts that; no guest reads a PSG port yet. Events apply at the first CPU-unit start at or after `at_tick` (C67): the granularity is one instruction, and the input floor (`input_floor_tick` = start of the last begun unit + 1; an event below it is refused) is a contract, not a hardware observation; hardware may differ by up to one instruction.
- **The INTERNAL stop reason is untestable through the public API.** No public input reaches it; it is a defensive path. Its presence is pinned by the define pins only.
- **The package proof is independence of build graph and paths, not of toolchain.** The consumer is built by the same compiler on the same machine by the same author; static only; LP64 only for the request size. The CMake 3.25 floor is untested (local 4.x, hosted and container 3.28).
- **Host-dependent runner scenarios.** The wall timeout and signal cancellation scenarios depend on the host and are never golden (`host-dependent=2`).
- **Hosted evidence is orchestrator-recorded.** `tools/ci-record.sh` and the job summary are exercised only on hosted runs read by the orchestrator; executors never call `gh`. The 23 rows of `.planning/ci-runs.tsv` were appended by the orchestrator; the `ci-log` step checks their format, not that every run was recorded.
- **D10 and D11 evidence remains missing.** No measured-hardware claim, no owner-local real EXEC/GROM run. C14-C15 and C63-C67 stay open on D10 measurements.
- **Publication scan on old history.** `sh tools/hygiene.sh --history` (not a gate step) still fails on earlier planning commits already on public `main`: a local scratch path appears in old commit text (04-01 open item: 345c48d, cccdfef, b78af4f, 92c4dc7, eaa832e, and the message of 4ccc9bb). Not fixed: it needs rewriting published history, an owner decision (AUTONOMY hard stop).
- **Job-summary skips table noise.** The "Skips and missing" table lists `pending-resolvers=15` and `unknown-fields=16`, which are record stats whose names match the D-42 skip pattern. Still open: an allowlist would change `ci-summary.sh` and its selftests.
- **One residual without its own fault.** `run.input.refuse.extra-controller-word` has no dedicated fault; the optional `INPUT_EXTRA_CONTROLLER_UNCHECKED` (core/src/machine/machine_input.c) was not added here (one more fault executable, deferred to the Phase 5 input work).
- **Nothing is handed to the owner as UAT.** What stays outside agent reach is a settings action (the branch-protection ruleset), for the owner or orchestrator, and it blocks nothing in Phase 4.

## Deviations from Plan

**1. [Orchestrator data] Budget rule outcome is a re-baseline, not a lever.** The plan expects "the next lever in D-45 order if the rule fires". The rule fired; per the orchestrator's decision the budget is re-baselined instead (CI-EFFICIENCY records the evidence and the ordered levers for when the new trigger fires). The plan's key link (median of last 3 versus 40 s) is evaluated and recorded as fired.

**2. [Rule 2] C24/C25 notes extended.** Beyond `default_policy` and `resolver`, the notes now name the deciding plans, so the record reads without the ROADMAP.

**3. [Rule 2] API-02, PKG-03, PKG-04 marked Complete.** They were Pending through 04-24; their evidence is complete at close-out (rows above). PKG-03/04 rely on orchestrator-recorded hosted runs (caveat above).

**4. Counts.** Controls 276 (not 275) and tests-expected 2780 as in the 04-24 SUMMARY (gap fills).

## Open Items

- **Owner/orchestrator:** apply the branch-protection ruleset with the commands in `.planning/AUTONOMY.md` section 1 and record its id and date there; after that, record commits go through a PR.
- **Owner:** decide whether to rewrite the published planning commits that fail `hygiene.sh --history` (or accept them).
- **Orchestrator:** keep appending hosted rows; apply the new budget rule (last-5 `gate_s` median > 90 s, any run > 120 s, last-5 `cpu_s` median > 200) with the levers in the CI-EFFICIENCY order.
- **Phase 5:** positive PSG-port read test replacing `run.input.guest-invisible`; C14 and C63-C67 with the control-to-byte table and D10; consider fault `INPUT_EXTRA_CONTROLLER_UNCHECKED`; the first appended request/report field needs a v1-size constant (A-02); if a diagnostic needs SDBD, the listing operand rule must learn the prefix first.
- **Later:** release workflow per AUTONOMY section 5 at the first release (with `ci-parity` extension); job-summary skip-pattern allowlist for record stats; libasm cross-check of the diagnostic if a second encoding opinion is wanted; CMake 3.25 floor test only if a consumer needs it.

## Final gate

- `sh tools/gate.sh` (after both task commits' edits, load 6.9): `gate: PASS steps=33/33 controls=276/276 ... ci-summary-selftest=11/11 claims=8 ... faults=149/149 ... pkg-selftest=10/10 ... tests-expected=2780 tests-executed=2778 tests-skipped=2 tests-missing=0 ...`, `gate-time total=46`.
- `dash tools/gate.sh` (load 9.4): the same line, `gate: PASS`, `gate-time total=49`.
- `sh tools/gate-linux.sh` (Ubuntu 24.04 container, x86_64 emulated, gcc 13.3.0, cmake 3.28.3, dash): `gate: PASS steps=33/33 controls=276/276 ... cc=GNU c-presets=2 fault-runs=149 ... faults=149/149 ... gates=26 lines=102 ... consumer-lines=26 pkg-selftest=10/10 ... tests-expected=2780 tests-executed=2778 tests-skipped=2 tests-missing=0`, `gate-time total=199`, last line `gate-linux: exit 0 (log build/gate-linux.log)`. It ran before the final one-line CI-EFFICIENCY edit that records it (text only).
- Records script after the edits: `phase4-records: clean`.
- Counts: start of plan `steps=33/33 controls=276/276 faults=149/149 tests-expected=2780`; end identical (records-only plan). Phase 4 start to end: steps 26 -> 33, controls 231 -> 276, faults 119 -> 149, tests-expected 2144 -> 2780.

## Threat Flags

None. Records and documentation only; no code, no new surface.

Mitigations from the plan's threat model: T-04-75 (one fresh run mapped per SC and requirement; caveats), T-04-76 (per-id script failed first on exactly the stale ids), T-04-77 (branch protection recorded, not applied), T-04-78 (D-45 evaluated on recorded hosted rows; no check dropped), T-04-SC (no package installs).

## Self-Check: PASSED

- Files exist: .planning/CI-EFFICIENCY.md (section "Phase 4 close-out"), .planning/AUTONOMY.md ("Branch protection on main", "release.yml"), data/discrepancies.json (C24, C25), .planning/ROADMAP.md (D4, D6, D12 rows).
- Commits on HEAD: 0e045df, 17aa819.
