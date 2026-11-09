---
phase: 01-evidence-profile-and-rights-foundations
plan: 01
subsystem: infra
tags: [evidence-loop, gate, cmake-script, posix-sh, negative-controls, profile-record, ci]
requires: []
provides:
  - "sh tools/gate.sh: single local/CI command (steps, exact inventories, negative controls, one summary line)"
  - "tools/check/records.cmake: generic JSON record checker driven by schema/<kind>.txt (SKELETON section 4 DSL) plus kind hooks"
  - "tools/check/integrity.cmake: path charset, empty-file, CRLF, BOM, ASCII-only and SPDX guard"
  - "tools/check/run-controls.sh: negative-control runner (overlay, MAKE verbs, EXPECT/FORBID, inventory checks)"
  - "data/profiles/intv-2609-ntsc-family-p1.json: first profile record (23 fields, per-field tag, cite, conflict, resolving evidence)"
  - "data/tiers.json: two separate vocabularies (10 source-authority tags, 8 evidence tiers)"
  - "20 negative controls and a known-good tree"
  - ".github/workflows/gate.yml: CI job running exactly the gate"
affects: [every later plan in phase 1, phase 2 build and CTest steps appended to the same gate]
actuals:
  tokens: 15600
  tasks: 2
  commits: 2
plan_head_before: bf14264b7b1e6a69a72f74033529cfde18a2dce8
plan_head_after: e20b051b76ff5b43ec4ef9bce56c627b89d427b1
tech-stack:
  added: []
  patterns:
    - "STATUS-line failure tokens (-- FAIL <token>: <detail>) with one final FATAL_ERROR"
    - "Schema-driven record checking (file/topkeys/top/array/members/idkey/inventory/keys/req/nonblank/match/enumfrom/when/hook)"
    - "Overlay-on-good-tree controls with run-time generated bad bytes (MAKE verbs)"
key-files:
  created:
    - .gitignore
    - .github/workflows/gate.yml
    - tools/gate.sh
    - tools/check/lib.cmake
    - tools/check/records.cmake
    - tools/check/integrity.cmake
    - tools/check/run-controls.sh
    - tools/check/steps/vocab.sh
    - tools/check/steps/profile.sh
    - tools/check/steps/integrity.sh
    - tools/check/schema/tags.txt
    - tools/check/schema/evidence-tiers.txt
    - tools/check/schema/profile.txt
    - tools/check/kinds/evidence-tiers.cmake
    - tools/check/kinds/profile.cmake
    - data/tiers.json
    - data/profiles/intv-2609-ntsc-family-p1.json
    - data/expected/profile-fields.ids
    - data/expected/source-authority-tags.ids
    - data/expected/evidence-tiers.ids
    - data/expected/gate-steps/skeleton.ids
    - data/expected/controls/skeleton.ids
    - tools/check/controls/
  modified: []
key-decisions:
  - "Gate summary steps=<passed>/<expected>: the first number counts steps that ran and exited 0"
  - "The vocab step prints its own -- vocab pass line; the inner tags and evidence-tiers lines are echoed too"
  - "Gate FILELIST keeps unsafe-charset paths even when git quotes them, so they reach integrity-path-charset instead of being dropped"
  - "Duplicate JSON member keys stay a documented non-detectable limit (CMake keeps the last)"
requirements-completed: [FND-01, FND-02]
---

# Phase 1 Plan 01: Evidence Loop Skeleton Summary

`sh tools/gate.sh` now runs three steps (vocab, profile, integrity) and 20 negative controls over the `intv-2609-ntsc-family-p1` profile and the two tiers vocabularies, using only POSIX sh, `cmake -P` and git; CI runs exactly that command.

## Final gate output

Last line of `sh tools/gate.sh` (also identical under `dash tools/gate.sh`):

`gate: PASS steps=3/3 controls=20/20 files=127 records=23 fields=23 unknown-fields=12 tags=10 tiers=8`

## Accomplishments

- Task 1 (tracer): gate, generic records checker, profile and vocabulary records, one control (`profile-blank-value`), CI workflow, `.gitignore`.
- Task 2: integrity step, the full MAKE verb set (`delete empty write rename crlf bom nonascii homepath secret password email`), FORBID support, 19 more controls (profile 9 plus missing-key, vocab 3, integrity 6).
- Every control was inspected for its full output: each fires only its intended token (`profile-empty-inventory` also reports the three `profile-id-extra` lines that follow from an empty inventory, which is expected).
- Throwaway checks run and reverted: removing `steps/profile.sh` gives `gate-step-missing: profile`; a wrong EXPECT gives `control-token-missing`; unknown MAKE verb, blank EXPECT line and a `..` MAKE path are rejected (`control-make-verb`, `control-expect-blank`, `control-make-path`); all four bad-text verbs generated correct lines in a temp tree.
- Measured: `sh tools/gate.sh` takes about 1.8 s wall time locally (limit 10 s); `cat tools/check/*.cmake tools/check/kinds/*.cmake | wc -l` is 636 (limit 700).
- Profile record: 23 fields, 12 of them `unknown-default-chosen` (each with resolving evidence and revisit trigger), 5 conflict references (C7, C8, C9, C11, C27), D3 cited in `selected_by`.

## Task Commits

1. Task 1 (tracer): `6196540` feat(01-01): tracer evidence loop - gate, generic record checker, profile record, first control, CI
2. Task 2: `e20b051` feat(01-01): integrity step, MAKE verbs and the profile/vocab/integrity control set

The plan-metadata commit (this SUMMARY, STATE.md, ROADMAP.md, REQUIREMENTS.md) follows separately and is not counted in `commits: 2`.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Protected-branch commit guard**
- **Found during:** step 0 of the commit protocol, before Task 1.
- **Issue:** the working tree was on `main`, which `gsd_run query git.base-branch --is-protected` reports as protected; the guard says to halt, re-home onto a phase branch, and not to change `git.allow_default_branch_commits` without authority.
- **Fix:** created the phase branch `gsd/phase-1-evidence-profile-and-rights-foundations` from `main` at the plan base and committed there. No config was changed. The orchestrator must merge or fast-forward this branch into `main` (or later plans must start from it).
- **Files modified:** none.
- **Commit:** n/a (branch creation).

**2. [Rule 2 - Missing critical functionality] Extra failure tokens beyond the plan list**
- **Issue:** a few vacuous-pass and safety guards had no token: `<kind>-inventory-missing`, `<kind>-inventory-duplicate`, `<kind>-schema`, `<kind>-enum-file-missing`, `<kind>-type` on a non-object record or root, `control-inventory-empty`, `control-none`, `control-build`, `control-make-path`, `gate-no-cmake`, `gate-controls-failed`, `gate-controls-line`, `gate-controls-empty`.
- **Fix:** added them in `tools/check/lib.cmake`, `tools/check/records.cmake`, `tools/check/run-controls.sh` and `tools/gate.sh` so no empty or malformed input can pass silently. Not covered by a dedicated control yet (plan 01-13 owns the runner self-test).

**3. [Interpretation] Plan wording points resolved**
- `steps=<run>/<expected>` is implemented as steps passed over steps expected.
- The workflow runs `cmake --version` (no `| head -1`) as the plan's step list says.
- The positive good-tree runs print `control ok   _good/<step>` lines; they are not counted in `controls=<ok>/<total>`.

## Known Limits

- Duplicate JSON member keys are not detectable through `string(JSON)`; the checker never claims to catch them.
- The gate builds its path list with a line-based `read` loop; a tracked path containing a newline would split into two lines, and both halves are kept (unsafe charset), so it still fails `integrity-path-charset`.
- Accepted debt from the plan: the `profile` kind is single-file; the first additional profile must promote it to per-file records keyed by profile id.

## Known Stubs

None. The 12 `unknown-default-chosen` profile values are intentional defaults with recorded resolving evidence, not stubs.

## Threat Flags

None. Threat register T-01-01 to T-01-05 mitigations are in place (verb whitelist without eval and with a path guard, path charset check, `permissions: contents: read` with `persist-credentials: false`, SHA-pinned checkout, no tracked bad strings: `git grep` for home paths, token shape and password assignment under `tools` prints nothing).

## Self-Check: PASSED

- All listed key files exist; commits `6196540` and `e20b051` are ancestors of HEAD.
- `sh tools/gate.sh` and `dash tools/gate.sh` both end with `gate: PASS steps=3/3 controls=20/20`.
