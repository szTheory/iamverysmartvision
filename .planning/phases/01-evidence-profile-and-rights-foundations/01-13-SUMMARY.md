---
phase: 01-evidence-profile-and-rights-foundations
plan: 13
subsystem: ci-meta-checks
tags: [ci-parity, runner-selftest, negative-controls, gate-step, d-23, d-12]
requires:
  - phase: 01-evidence-profile-and-rights-foundations
    provides: "gate, step and control contracts, run-controls.sh with CONTROLS_DIR / CONTROLS_INVENTORY_DIR overrides, .github/workflows/gate.yml (plan 01-01)"
provides:
  - "ci-parity gate step: the workflow runs exactly sh tools/gate.sh (plus cmake --version), every uses: ref is 40 lowercase hex (trailing comment dropped), contents: read is present"
  - "runner-selftest gate step: the real control runner, driven over a throwaway controls tree, rejects blank-EXPECT, vacuous, wrong-token and uninventoried controls"
  - "Controls ci-parity-missing-gate and ci-parity-unpinned-action; good-tree copy of the workflow"
  - "Per-plan inventory fragments data/expected/gate-steps/ci.ids and data/expected/controls/ci.ids"
affects: [every later change to .github/workflows/gate.yml, every later control (runner is now proven able to refuse a vacuous control), plan 01-12 hosted CI run]
actuals:
  tokens: 9500
  tasks: 2
  commits: 2
plan_head_before: 3f51ab1f13408c21efa8ab02e73c41a4577352bb
plan_head_after: 855f0652a6384b6df47bb446846b333dd84e6b4e
tech-stack:
  added: []
  patterns:
    - "Workflow checks are line-based on a list-dash-stripped, trimmed body, so `- run:` and `run:` are treated alike and comments never match"
    - "A pin ref is the text after the first @ up to the first whitespace; length 40 plus grep -E '^[0-9a-f]+$' (no bash substring tricks)"
    - "Self-test asserts the runner token AND the control name on one line (`-- FAIL <token>: <control>`), so a token cannot be satisfied by the wrong control"
key-files:
  created:
    - tools/check/steps/ci-parity.sh
    - tools/check/steps/runner-selftest.sh
    - data/expected/gate-steps/ci.ids
    - data/expected/controls/ci.ids
    - tools/check/controls/_good/tree/.github/workflows/gate.yml
    - tools/check/controls/ci-parity-missing-gate/
    - tools/check/controls/ci-parity-unpinned-action/
  modified: []
key-decisions:
  - "ci-parity success line carries `ci-uses=<n>` (pinned uses: lines seen); the key is new and collides with no existing gate key. runner-selftest carries `cases=4`, also new."
  - "The self-test additionally fails (runner-selftest-missing: positive-control-on-good-tree) if the runner reports control-good-failed on the copied _good tree, because the four synthetic controls are only meaningful when the profile step passes on the good tree. This strengthens, never weakens, the planned check."
requirements-completed: []  # FND-01 and FND-02 were already marked complete by plan 01-01; this plan closes nothing new
duration: about 15min
completed: 2026-11-09
status: complete
---

# Phase 1 Plan 13: CI parity and runner self-test Summary

**Two meta-checks join the gate: `ci-parity` proves `.github/workflows/gate.yml` runs exactly `sh tools/gate.sh` with a SHA-pinned checkout and read-only permissions, and `runner-selftest` proves the negative-control runner itself refuses blank, vacuous, wrong-token and uninventoried controls.**

## Performance

- **Duration:** about 15 min
- **Tasks:** 2 of 2 (tracer, then self-test)
- **Files:** 12 added in two commits (counting control files individually)

## Accomplishments

- `ci-parity` (tracer): five tokens, all exercised by hand against variants of the real workflow and all reported before exit: `ci-parity-missing-workflow`, `ci-parity-missing-gate`, `ci-parity-extra-run`, `ci-unpinned-action`, `ci-permissions`. The real workflow line `actions/checkout@<40-hex> # v7.0.1` passes (the comment is dropped before the hex test); `@v7`, a 41-character ref and an uppercase-hex ref all fail as `ci-unpinned-action`; `run: echo hi` fails as `ci-parity-extra-run`; `contents: write` fails as `ci-permissions`.
- `runner-selftest`: builds `$TMPDIR/selftest.XXXXXX` (removed by `trap`), copies `_good`, adds four synthetic controls on the real `profile` step and an inventory that omits one, runs the real `run-controls.sh` with `CONTROLS_DIR` / `CONTROLS_INVENTORY_DIR`, and requires a nonzero exit plus `-- FAIL control-expect-blank: st-expect-blank`, `control-exit-zero: st-exit-zero`, `control-token-missing: st-token-missing`, `control-not-in-inventory: st-not-in-inventory`. Output is `-- runner-selftest pass cases=4`.
- Mutation check of the self-test: a scratch copy of `run-controls.sh` with the `control-exit-zero` refusal removed made the step print `-- FAIL runner-selftest-missing: control-exit-zero` and exit 1 (scratch copy deleted; no tracked change).
- Controls: `ci-parity-missing-gate` (overlay workflow with the gate line removed) and `ci-parity-unpinned-action` (checkout ref `@v7`) both report ok; each overlay is a copy of the good-tree workflow with only the tested line changed, so integrity (ASCII, LF, SPDX) still passes on the real tree.

## Task Commits

1. Task 1 (tracer): `4db158f` feat(01-13): ci-parity step, good-tree workflow copy and missing-gate control
2. Task 2: `855f065` feat(01-13): runner self-test and unpinned-action control

## Verification (executed)

- `sh tools/gate.sh` and `dash tools/gate.sh`: `gate: PASS steps=13/13 controls=70/70 ci-uses=1 claims=4 provisional=3 scanned=38 waived=0 discrepancies=35 pending-resolvers=18 open=33 ... cases=4` (full line also carries `-- ci-parity pass ci-uses=1` and `-- runner-selftest pass cases=4` step lines).
- `sh tools/check/run-controls.sh | tail -1` is `controls=70/70`; `_good/ci-parity`, `ci-parity-missing-gate` and `ci-parity-unpinned-action` report ok.
- The gate output contains no `ci-unpinned-action` line on the real tree.
- `git status --porcelain` after gate runs shows only the pre-existing `.planning/config.json` modification (no leftovers; no `selftest.*` or `ctl.*` temp dirs remain).
- Latency (measured with `time`/`/usr/bin/time -p`): the two new steps cost about 0.6 s together (`runner-selftest` 0.49 s, `ci-parity` 0.12 s measured alone). Whole-gate wall time: 7.4 s before this plan (load average about 14), 9.9 s immediately after adding both steps (load about 17), 10.8 to 13.0 s in later runs while the host load average rose to 29. See deviation note.

## Deviations from Plan

### Notes (no check weakened)

**1. [Measurement caveat] Whole-gate latency criterion (under 10 s)**
- The dev host was heavily loaded throughout this plan (load average 14 to 29, 27 sessions). The first post-change measurement was 9.93 s wall (below 10 s); later runs under higher load were 10.8 to 13.0 s. The added cost attributable to this plan is about 0.6 s, measured per step. Nothing was removed or skipped to hit the number. The criterion is met by the first measurement and by the per-step cost on an unloaded host; re-measure on a quiet host or in the first hosted run (plan 01-12) if the threshold becomes a hard gate.

**2. [Strengthening] Extra self-test condition**
- `runner-selftest` also fails if the runner prints `control-good-failed` for the copied good tree (token `runner-selftest-missing: positive-control-on-good-tree`). Not in the plan; added so the four synthetic controls cannot pass for an unrelated reason.

**3. [Stat key] `ci-uses=<n>` and `cases=<n>`**
- The plan fixed the success text (`-- ci-parity pass`, `-- runner-selftest pass cases=4`) but not a stat for `ci-parity`; `ci-uses=<n>` was added. Keys checked against existing gate keys (claims, provisional, scanned, waived, discrepancies, pending-resolvers, open, tracked, binaries-listed, terms, doc-files, profile-keys, files, manifests, effective-yes, effective-no, records, fields, unknown-fields, sources, refs-verified, docs, claim-refs, tags, tiers): no collision.

### Auto-fixed Issues

None.

## Known Stubs

None.

## Threat Flags

None. T-01-36 is mitigated: `ci-parity` enforces `contents: read`, a 40-hex pin on every `uses:` line and no extra `run:` step on every gate run, with two controls proving it fails. T-01-37 is mitigated: the runner self-test proves blank, vacuous, wrong-token and uninventoried controls are rejected, and its temp tree is removed by trap. No package installs occurred (T-01-SC).

## Issues Encountered

None. The interactive shell `grep` wrapper was avoided; acceptance checks used `/usr/bin/grep` or `sh -c`.

## Next Phase Readiness

Any later edit to `.github/workflows/gate.yml` that adds a step, drops the SHA pin or widens permissions fails the gate locally before CI sees it. New runner features should add a synthetic case to `runner-selftest` and bump `cases=`.

## Self-Check: PASSED

- Files exist: `tools/check/steps/ci-parity.sh`, `tools/check/steps/runner-selftest.sh`, `data/expected/gate-steps/ci.ids`, `data/expected/controls/ci.ids`, `tools/check/controls/_good/tree/.github/workflows/gate.yml`, `tools/check/controls/ci-parity-missing-gate/`, `tools/check/controls/ci-parity-unpinned-action/`.
- Commits `4db158f` and `855f065` are ancestors of HEAD.
