---
phase: 01-evidence-profile-and-rights-foundations
plan: 06
subsystem: rights-and-fixtures
tags: [fixture-manifest, redistributable, rights-policy, gate-step, negative-controls, fnd-04]
requires:
  - phase: 01-evidence-profile-and-rights-foundations
    provides: "gate, step and control contracts, lib.cmake helpers and the controls runner (plan 01-01)"
provides:
  - "docs/fixture-manifest.md: schema_version 1 with every D-13 key and the effective-redistributable rule"
  - "fixtures/FX-sample-rights-unknown/manifest.json: sample entry (unknown, uncommitted, local-test) counted as effective no"
  - "manifests gate step (tools/check/manifests.cmake, manifest-lib.cmake) with stats manifests=, effective-yes=, effective-no="
  - "files gate step (tools/check/files.cmake) with stats tracked=, binaries-listed="
  - "docs/RIGHTS.md: MIT licence, public repository, never-committed list, refs/ cache, force-push prohibition"
  - "Fifteen negative controls (nine named by the plan plus six extra) and a FX-good good-tree fixture"
affects: [01-07 sources and denylist steps (scan docs/RIGHTS.md and docs/fixture-manifest.md), 01-11 and 01-12 (FND-05 provenance and hygiene), every later plan that adds a fixture]
actuals:
  tokens: 11300
  tasks: 2
  commits: 2
plan_head_before: 8da63bb9e5888f8315a017979ae2d4ef331a3a87
plan_head_after: c62738ef597b228873add6188483de21ba1e6ff4
tech-stack:
  added: []
  patterns:
    - "Effective redistributable is a least fixpoint over a registry of manifests (derived_from), so unknown ids and cycles resolve to no"
    - "One check_shape helper enforces presence, type and allowed keys for every manifest object (top level and nested)"
    - "Manifests and fixture-directory rules are anchored at the repository root (^fixtures/), so control overlays under tools/check/controls never trip the real-tree steps"
key-files:
  created:
    - docs/fixture-manifest.md
    - docs/RIGHTS.md
    - fixtures/FX-sample-rights-unknown/manifest.json
    - data/expected/fixtures.ids
    - data/expected/gate-steps/rights.ids
    - data/expected/controls/rights.ids
    - tools/check/manifest-lib.cmake
    - tools/check/manifests.cmake
    - tools/check/files.cmake
    - tools/check/steps/manifests.sh
    - tools/check/steps/files.sh
    - tools/check/controls/_good/tree/fixtures/FX-good/manifest.json
    - tools/check/controls/_good/tree/fixtures/FX-good/payload.txt
    - tools/check/controls/_good/tree/fixtures/FX-sample-rights-unknown/manifest.json
    - tools/check/controls/_good/tree/data/expected/fixtures.ids
    - "tools/check/controls/manifests-* (9 directories)"
    - "tools/check/controls/files-* (5 directories)"
  modified: []
key-decisions:
  - "The files step reports its count as tracked=<n> instead of files=<n>, because the integrity step already emits files=<n> and the gate summary does not de-duplicate keys"
  - "Unknown derived_from ids, cycles and unparseable manifests resolve to effective no (fail closed)"
  - "A committed manifest must list its primary path among outputs and must list at least one output"
requirements-completed: [FND-04]
duration: about 35min
completed: 2026-11-09
status: complete
---

# Phase 1 Plan 06: Fixture manifests and rights checks Summary

**A schema_version 1 fixture manifest is checked fail-closed by a `manifests` step, and a `files` step refuses tracked `refs/` paths, `.env` files, unlisted ROM/PDF/image-like files and unlisted fixture files, backed by fifteen negative controls.**

## Performance

- **Duration:** about 35 min
- **Tasks:** 2 of 2 (tracer, then rights policy, files step and remaining controls)
- **Files:** 60 changed in two commits (counting control files individually)

## Accomplishments

- Schema: every D-13 key is required at the top level and inside `source`, `recipe`, `licence_evidence`, `rights_review`, `outputs` and `notices`; unknown keys, missing keys, wrong types, bad enums, bad digests and unsafe paths each have a token (`manifest-unknown-key`, `manifest-missing-key`, `manifest-format`).
- Effective rule: `manifest_effective` returns yes only for `redistributable: yes` with licence evidence (non-blank url and a 64-hex `text_sha256`), a review that is not `unreviewed`, and every `derived_from` input itself effective yes. The sample entry reports `effective-no=1`; its `committed: true` copy fails `manifest-committed-not-redistributable`.
- Bytes: a committed manifest's outputs must exist with the recorded size and sha256; an uncommitted manifest's outputs must not be tracked.
- Files step: `files-tracked-refs`, `files-env`, `files-unlisted-binary`, `files-unlisted-fixture-file`, `files-fixture-without-manifest`, `files-filelist-empty`. A listed binary output of a committed effective-yes manifest is accepted and counted in `binaries-listed=` (checked manually in a scratch tree, see Verification).
- `docs/RIGHTS.md` states the final MIT decision (D-04) and public visibility, not an open question.
- Tracer gate re-run before expansion: `-- manifests pass manifests=1 effective-yes=0 effective-no=1`, the flipped-copy control ok, then expanded.

## Task Commits

1. Task 1 (tracer): `2166e6f` feat(01-06): fixture-manifest schema, fail-closed manifests step and flipped-copy control
2. Task 2: `c62738e` feat(01-06): rights policy, files step and fourteen rights controls

## Verification (executed)

- `sh tools/gate.sh` and `dash tools/gate.sh`: `gate: PASS steps=8/8 controls=55/55 claims=4 provisional=3 discrepancies=35 pending-resolvers=18 open=33 tracked=279 binaries-listed=0 files=279 manifests=1 effective-yes=0 effective-no=1 records=23 fields=23 unknown-fields=12 docs=4 claim-refs=2 tags=10 tiers=8`.
- `sh tools/check/run-controls.sh | tail -1` is `controls=55/55`; all nine plan-named controls and the six extra ones report ok.
- Real-tree gate output has no `files-unlisted-fixture-file: tools/` or `files-fixture-without-manifest: tools/` line; `grep -F '^fixtures/' tools/check/files.cmake` matches.
- R1 and R2 greps over `docs/RIGHTS.md` and `docs/fixture-manifest.md` print nothing; no absolute private path in any tracked file written here; `git ls-files` contains no ROM, PDF or image-like file.
- Manual scratch-tree checks (not tracked): an empty FILELIST gives `-- FAIL files-filelist-empty: FILELIST`; a committed effective-yes manifest listing `payload.bin` gives `files pass ... binaries-listed=1`; flipping that review to `unreviewed` gives `files-unlisted-binary`.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] CMake function calls written without parentheses**
- **Found during:** Task 1 (first gate run)
- **Issue:** `json_type`, `manifest_derived` and one more helper were called command-style, which CMake rejects at parse time.
- **Fix:** Rewrote every call as `name(args)`.
- **Files modified:** `tools/check/manifests.cmake`, `tools/check/manifest-lib.cmake`
- **Commit:** `2166e6f`

**2. [Rule 1 - Bug] Variable named `no` in a numeric `if()`**
- **Found during:** Task 1 (review before first run)
- **Issue:** `no` is a CMake false constant; using it as a variable in `if(no GREATER 0)` is a latent misparse.
- **Fix:** Renamed to `nout`.
- **Commit:** `2166e6f`

### Changed from the plan wording (no check weakened)

- **Stat key:** the plan named the files step stat `files=<n>`. The integrity step already prints `files=<n>`, and the gate summary does not de-duplicate keys, so the files step prints `tracked=<n>` instead. `binaries-listed=<k>` is as planned. The two counts are equal today.

### Additions beyond the plan (Rule 2, no check weakened)

- Six extra controls for tokens or must-have truths that the plan listed without a control: `manifests-duplicate-id`, `manifests-inventory-empty`, `manifests-id-missing` (covers zero manifests with a non-empty inventory), `manifests-uncommitted-output-tracked`, `manifests-derived-from-unknown` (least-permissive derived_from rule), `files-fixture-without-manifest`. Total controls added: 15 (plan named 9).
- `manifest-unknown-derived` token for a `derived_from` id that names no manifest, and `manifest-format` checks on `path` (must be null when uncommitted, must be listed in outputs when committed) and on `local_path` (`refs/` only).
- `files-filelist-empty` has no negative control: the controls runner builds `FILELIST` with `find` from a non-empty good tree, so an empty list cannot be produced. It was exercised manually (see Verification).

### Notes (not deviations)

- The control `manifests-unknown-committed` also reports `manifest-format: .../path` and `manifest-missing-output` because the sample has `path: null` and no outputs; the expected token is present and is the rights failure.
- FND-04 is closed by this plan alone and is marked complete. FND-05 is also listed by plans 01-07, 01-11 and 01-12 (provenance rule, denylist, hygiene); this plan delivers only the tracked-file part, so FND-05 is left open.
- Duplicate JSON member keys are not detectable (known limit, SKELETON section 8); a hash does not establish rights, and `docs/fixture-manifest.md` and `docs/RIGHTS.md` say so.
- The `.planning/config.json` modification predates this plan and was not staged.
- The records DSL whitespace limitation (plans 01-04 and 01-05) does not affect this plan: the manifest checkers are dedicated `cmake -P` scripts, and no regex in them contains a space.

## Known Stubs

None. `FX-sample-rights-unknown` is an intentional illustrative entry with no bytes, as the plan requires.

## Threat Flags

None. T-01-15 and T-01-16: `files-tracked-refs`, `files-env`, `files-unlisted-binary` and `files-unlisted-fixture-file` each have a control, and `.gitignore` already carries the class patterns. T-01-17: the effective rule needs licence evidence, a non-unreviewed review and effective-yes inputs; a misspelled key fails; a control shows a derived output cannot exceed its input. The checkers read only FILELIST paths matched by fixed anchored patterns, use no `eval`, and write nothing.

## Issues Encountered

None beyond the two fixes above. A full gate takes about 3 seconds on an idle machine.

## Next Phase Readiness

Plan 01-07 can add the sources and denylist steps over the two new docs (they cite no `SRC-` ids; the `refs/SRC-<slug>.<ext>` placeholder is written with `<`). Later plans add a fixture by creating `fixtures/<id>/manifest.json`, one line in `data/expected/fixtures.ids`, and, when bytes are committed, the licence evidence and review that make the manifest effective yes.

## Self-Check: PASSED

- All created files exist and commits `2166e6f` and `c62738e` are ancestors of HEAD (verified before the docs commit).
