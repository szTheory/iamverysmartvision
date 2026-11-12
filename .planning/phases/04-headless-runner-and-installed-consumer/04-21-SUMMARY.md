---
phase: 04-headless-runner-and-installed-consumer
plan: 21
subsystem: runner, tools/check, data/golden, docs
tags: [runner, ivs-run, golden, update-golden, digests, evidence-tier, dev-vs-release, controls, RUN-05, D-11, D-19, D-21, A-01]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-18: closed report key table (digest.<cpu|mem|images|instance|composite|video|audio|input>, host.calls); 04-19: ivs-run; 04-20: suite runner-process, data/expected/runner-scenarios.ids (six columns, class column 4) and build/gate/<preset>/runner/<id>.report; 04-16: data/golden/diag-cpu-1.expect"
provides:
  - "ivs-run --update-golden <file>: writes a .digests golden (tier, oracle, scenario, profile, domains header; the report filtered to the golden key list); refuses `user` content before the run and host-dependent outcomes after it (exit 14)"
  - "data/golden/diag-cpu-1.digests, diag-input-pass.digests (input=observed), timeout-budget.digests (budget outcome and stop time)"
  - "pure gate step `golden` (tools/check/steps/golden.sh): header rules, .expect drift, .digests body against the dev report, dev versus release equality apart from host.calls with host-dependent ids excluded; stats goldens=4 reports=10 host-dependent=2"
  - "toy golden tree in tools/check/controls/_good/tree (toy and toy-wall scenarios, goldens, reports) and ten golden-* controls"
  - "docs/runner.md: the option, a Goldens section with the golden key list and the exact commands"
affects: [04-23, 04-24]

actuals:
  tokens: 10750
  tasks: 3
  commits: 3
plan_head_before: bf4c20dc7e8888382491d8eae0f35a320e166093
plan_head_after: 8c3b8568dfa630ae67dadcd3a9e1c8d38838e6ab

tech-stack:
  added: []
  patterns:
    - "One golden key list, written twice on purpose: in C by the writer (runner/main.c main_golden_keep) and in awk by the judge (golden.sh filter_report); a divergence shows as golden-mismatch on the real tree"
    - "A live domain is observed only when its report count is a nonzero decimal (output.video.bytes, output.audio.bytes, input.events); otherwise `none` and its digest line is left out"
    - "Positive control proves an exclusion: the toy host-dependent scenario's dev and release reports differ, so the good tree passes only because column 4 says host-dependent"

key-files:
  created:
    - tools/check/steps/golden.sh
    - data/expected/gate-steps/golden.ids
    - data/expected/controls/golden.ids
    - data/golden/diag-cpu-1.digests
    - data/golden/diag-input-pass.digests
    - data/golden/timeout-budget.digests
    - tools/check/controls/_good/tree/data/golden/toy.expect
    - tools/check/controls/_good/tree/data/golden/toy.digests
    - tools/check/controls/_good/tree/data/scenarios/toy.ivss
    - tools/check/controls/_good/tree/data/scenarios/toy-wall.ivss
    - tools/check/controls/_good/tree/data/expected/runner-scenarios.ids
    - tools/check/controls/_good/tree/build/gate/{dev,release}/runner/{toy,toy-wall}.report
    - tools/check/controls/golden-*/ (ten controls)
  modified:
    - runner/main.c
    - docs/runner.md

key-decisions:
  - "Golden key list: scenario.id, scenario.profile, outcome, outcome.detail, exit.code, slot.*, step.*, cpu.*, input.*, digest.cpu, digest.mem, digest.images, digest.instance, digest.composite, output.video.bytes, output.audio.bytes, plus digest.video/audio/input of observed live domains, in report order. Left out: ivsr, api.version (a version bump must not churn goldens), the exec:/grom: labels, host.calls, report.end. digest.composite stays: it covers domains 1..4 only (ivs_machine.h)"
  - "The judge derives the live domains from the report and requires the header to agree: observed without content is golden-empty-domain-observed, a digest line of a domain marked none is golden-empty-domain-observed, content marked none is golden-mismatch"
  - "--update-golden also refuses a cancelled outcome and a timeout with detail `wall ...` (D-21), besides `user` content; the report is still printed, the exit is 14 and no file is written"
  - "A .digests golden must have oracle self-recorded (reported as golden-self-recorded-wrong-tier), so a hand-edited digests file cannot claim independent-derivation"
  - "RUN-05 marked complete: digests per domain are produced (04-18), repeated runs, slices and another cwd match (04-20), dev and release match and three digests goldens are re-checked by the gate on every run, and every golden carries tier and oracle headers the gate enforces. Video and audio carry no content in Phase 4 and are recorded `none`, never observed"

duration: about 20 min of edits and gates (first commit 19:51, last 19:55), plus one container gate
completed: 2026-11-11
---

# Phase 4 Plan 21: goldens and the golden step Summary

**`ivs-run --update-golden` writes tier-labelled `.digests` goldens for the diagnostic, the input schedule and the deterministic budget timeout. A new pure gate step `golden` checks every golden's header, the `.expect` assert set and the `.digests` body against the dev report on each gate run. It also requires dev and release reports to be byte-equal apart from `host.calls`, with the two host-dependent scenarios excluded. Ten controls on a toy tree show that each rule fails for its own token, and the positive control passes only because of the host-dependent exclusion.**

## What was built

- Task 1 (tracer, `1d1425b`): `runner/main.c` captures the report (cap 512 KiB) when `--update-golden <file>` is given and writes the golden after the run; `docs/runner.md` option row and Goldens section; the three `.digests`; `tools/check/steps/golden.sh`; `data/expected/gate-steps/golden.ids`.
- Task 2 (`ab0afed`): toy tree (toy scenario with two asserts, `toy.expect`, `toy.digests`, dev/release `toy.report` differing only in `host.calls`, `toy-wall` host-dependent with dev/release reports differing in `step.1.now_ticks`, six-column toy inventory) and four header controls.
- Task 3 (`8c3b856`): six body, dev/release and scope controls.

Commands used (from the repo root, after a gate build):

```
build/dev/runner/ivs-run --content-dir build/dev/images --update-golden data/golden/diag-cpu-1.digests data/scenarios/diag-boot-pass.ivss
build/dev/runner/ivs-run --content-dir build/dev/images --update-golden data/golden/diag-input-pass.digests data/scenarios/diag-input-pass.ivss
build/dev/runner/ivs-run --content-dir build/dev/images --update-golden data/golden/timeout-budget.digests data/scenarios/timeout-budget.ivss
```

Exits 0, 0, 11. The release binary with `--slice-ticks 7` writes byte-identical files for all three. Refusals checked by hand: needs-user-exec gives `golden refused: content user` (14, no report, no file); cancel-signal and timeout-wall give `golden refused: host-dependent outcome` (14, no file).

| Control | Overlay | Fails with |
|---|---|---|
| golden-header-missing-tier | toy.digests without `# tier:` | `golden-header-missing-tier: data/golden/toy.digests` |
| golden-header-missing-oracle | toy.expect with an empty `# oracle:` | `golden-header-missing-oracle: data/golden/toy.expect` |
| golden-self-recorded-wrong-tier | toy.expect with oracle self-recorded | `golden-self-recorded-wrong-tier: data/golden/toy.expect` |
| golden-empty-domain-observed | toy.digests `video=observed` plus a `digest.video` line | `golden-empty-domain-observed: data/golden/toy.digests` |
| golden-mismatch | toy.digests with a changed `digest.cpu` | `golden-mismatch: data/golden/toy.digests` |
| golden-expect-drift | toy.expect with one assert changed | `golden-expect-drift: data/golden/toy.expect` |
| golden-dev-release-diff | release toy.report with a changed `digest.mem` | `golden-dev-release-diff: toy (` |
| golden-user-scenario-tracked | toy.ivss `content exec user` | `golden-user-scenario-tracked: data/golden/toy` |
| golden-host-dependent-unmarked | inventory marks toy-wall deterministic | `golden-dev-release-diff: toy-wall` |
| golden-host-dependent-golden | adds a well-formed data/golden/toy-wall.digests | `golden-host-dependent: data/golden/toy-wall.digests` |

Each control was also run alone under dash: every one fails only for its intended token, except golden-empty-domain-observed, which also reports golden-mismatch (its body has the extra digest line).

## Performance

The step is a shell and awk checker over 4 goldens and 20 reports: `step-time golden 0` (under 1 s). Gate total was 25-27 s under sh and dash (load as usual).

## Counts

Start: `steps=30/30 controls=263/263 faults=149/149 tests-expected=2776 tests-skipped=2`.
End: `steps=31/31 controls=273/273 fault-runs=149 faults=149/149 tests-expected=2776 tests-executed=2774 tests-skipped=2 tests-missing=0`, step stats `goldens=4 reports=10 host-dependent=2`.

Acceptance checks: `ls data/golden/*.digests | wc -l` = 3; `grep -c '^# oracle: self-recorded$' data/golden/diag-cpu-1.digests` = 1; `grep -c 'video=none' data/golden/diag-cpu-1.digests` = 1; `grep -c 'input=observed' data/golden/diag-input-pass.digests` = 1; `control ok +golden-*` lines = 10.

## Deviations from Plan

**1. Toy `.expect` tier.** The toy tier list (`_good/tree/data/expected/evidence-tiers.ids`) has only `source-expectation` and `deterministic-repeatability`, and other steps' positive controls pin it, so `toy.expect` uses `source-expectation` (a comment says why) instead of `original-diagnostic`.

**2. Toy dev and release `toy.report` differ in `host.calls` (5 vs 3).** The plan says identical. A difference in `host.calls` only makes the positive control also show that `host.calls` is excluded.

**3. [Rule 2] Extra judge rules, each fail-closed:** a `.digests` golden must have oracle `self-recorded`; a `data/golden/` file that is neither `.expect` nor `.digests` is golden-mismatch; a malformed or missing `# domains:` line, a `# profile:` that differs from the scenario, a malformed inventory line, or no dev report at all is golden-mismatch (never a vacuous pass); a report of one preset missing is golden-dev-release-diff. `--update-golden` also refuses host-dependent outcomes. None has its own control (the plan fixes ten). The domains and oracle rules share code with controlled rules.

## Open Items

- Plan 04-23 (consumer): compare the consumer's `digest.<domain>` lines with `data/golden/diag-input-pass.digests` (it has `digest.input`; video and audio are `none` and absent). The golden body keys are listed in docs/runner.md.
- Plan 04-24 (important-gates) may cite the golden-* controls (T-04-62..65, T-04-79).
- `needs-user-exec` writes a report that is compared dev against release (equal) but has no golden; `bad-syntax` has no report.
- Carried: C14 and C63-C68 revisited in Phase 5/6/7; `API-02`, `PKG-03`, `PKG-04`, RUN-04 Pending; `pending-resolvers=17`, `unknown-fields=16`; `sh tools/hygiene.sh --history` still fails on earlier planning commits (this plan adds no home paths); the orchestrator records hosted runs with `sh tools/ci-record.sh <run-id>`; important-gates (04-24) may register the seven runner faults of 04-18/04-19.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=31/31 controls=273/273 ... cc=AppleClang c-presets=2 fault-runs=149 ... faults=149/149 ... goldens=4 reports=10 host-dependent=2 ... tests-expected=2776 tests-executed=2774 tests-skipped=2 tests-missing=0 ...`, `gate-time total=27`.
- `dash tools/gate.sh`: the same line, `gate: PASS`, `gate-time total=26`.
- `sh tools/gate-linux.sh` (Ubuntu 24.04 container, x86_64, gcc 13.3.0, cmake 3.28.3): `gate: PASS steps=31/31 controls=273/273 cc=GNU c-presets=2 fault-runs=149 faults=149/149 goldens=4 reports=10 host-dependent=2 tests-expected=2776 tests-executed=2774 tests-skipped=2 tests-missing=0`, `gate-time total=116`, `gate-linux: exit 0`.

## Threat Flags

| Flag | File | Description |
|------|------|-------------|
| threat_flag: file-write | runner/main.c | `--update-golden` opens an operator-chosen path with `fopen(path, "w")` and truncates it. The path comes only from the command line, never from a scenario (D-22 holds). The file holds hashes and report lines only, never image bytes. |

Mitigations from the plan's threat model:
- T-04-79: host-dependent ids are skipped in the dev/release judge, a golden for them is refused by the step and by `--update-golden`; controls golden-host-dependent-unmarked and golden-host-dependent-golden.
- T-04-62: tier and oracle header rules; `self-recorded` forces `deterministic-repeatability`; controls for each.
- T-04-63: `# domains:` with `none`; golden-empty-domain-observed.
- T-04-64: only `ivs-run --update-golden` writes; the step writes nothing outside its temp dir.
- T-04-65: `--update-golden` refuses `user` roles; golden-user-scenario-tracked.
- T-04-SC: no package installs.

## Self-Check: PASSED

- Files exist: tools/check/steps/golden.sh, data/golden/diag-cpu-1.digests, data/golden/diag-input-pass.digests, data/golden/timeout-budget.digests, data/expected/controls/golden.ids, data/expected/gate-steps/golden.ids.
- Commits on HEAD: 1d1425b, ab0afed, 8c3b856.
