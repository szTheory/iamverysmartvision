---
phase: 01-evidence-profile-and-rights-foundations
plan: 07
subsystem: provenance
tags: [provenance, denylist, sources-registry, consultation-log, gate-step, negative-controls, fnd-05]
requires:
  - phase: 01-evidence-profile-and-rights-foundations
    provides: "gate, step and control contracts, records.cmake and lib.cmake (plan 01-01); data/sources.json registry (plan 01-14); files and manifests steps (plan 01-06)"
provides:
  - "docs/provenance.md: documented independent derivation, D-14 no-copy rule, review checklist, honest limits"
  - "docs/provenance/CONSULTATIONS.md: log of the two research-era emulator readings (one permissive, one disclosed GPL licence read)"
  - ".github/pull_request_template.md: the review checklist as task-list items"
  - "denylist step with a never-waivable author and licence-header list and a [cite-only] waiver for project names"
  - "source record kind (schema, hook, inventory) and sources step: registry check, refs/ cache verification, SRC token resolution"
  - "Nine negative controls (seven planned plus two extra)"
affects: [01-04 and later plans citing SRC ids (every SRC token now resolves or fails), 01-11 and 01-12 (FND-05 hygiene), every later plan that adds docs or data]
actuals:
  tokens: 6800
  tasks: 2
  commits: 2
plan_head_before: 2a7fe14e5a7ec31c822c4288a2889dab4597f9e0
plan_head_after: 1d436d609631b179fcbe71329f9b3882292a6621
tech-stack:
  added: []
  patterns:
    - "Lists (denylist.txt, denylist-cite.txt) are read from CHECK_DIR and cleaned of comment and blank lines, because a blank fixed-string pattern would match every line"
    - "Findings name the file only and never print the matched text (T-01-20)"
    - "A cached refs/ file is optional: present means size and sha256 must match; absent is the visible 0/N state in CI"
key-files:
  created:
    - docs/provenance.md
    - docs/provenance/CONSULTATIONS.md
    - .github/pull_request_template.md
    - tools/check/denylist.txt
    - tools/check/denylist-cite.txt
    - tools/check/steps/denylist.sh
    - tools/check/steps/sources.sh
    - tools/check/schema/source.txt
    - tools/check/kinds/source.cmake
    - data/expected/sources.ids
    - data/expected/gate-steps/provenance.ids
    - data/expected/controls/provenance.ids
    - tools/check/controls/_good/tree/data/sources.json
    - tools/check/controls/_good/tree/data/expected/sources.ids
    - "tools/check/controls/denylist-* (3 directories)"
    - "tools/check/controls/sources-* (6 directories)"
  modified: []
key-decisions:
  - "The denylist step reports scanned=<n> instead of files=<n>, because the integrity step already emits files=<n> and the gate summary does not de-duplicate keys (same reason plan 01-06 used tracked=)"
  - "A reference-only registry entry must have null sha256, size_bytes and local_path (reported as source-format) and a note that mentions hashing (source-reference-only-unexplained otherwise)"
  - "data/sources.json needed no change: every entry already satisfied the new schema, and the community-docs note already qualified 'never opened for implementation' and pointed to the consultation log"
requirements-completed: []  # FND-05 also depends on plans 01-11 and 01-12 (hygiene, history); FND-06 depends on plans 01-02 and 01-03
duration: about 25min
completed: 2026-11-09
status: complete
---

# Phase 1 Plan 07: Provenance rule, denylist and source registry check Summary

**The no-copy rule is documented as documented independent derivation, a scoped denylist fails author names and licence headers (project names only with `[cite-only]`), and `data/sources.json` is a checked record kind verified against the local `refs/` cache.**

## Performance

- **Duration:** about 25 min
- **Tasks:** 2 of 2 (tracer, then log, template, registry check and remaining controls)
- **Files:** 41 changed in two commits (counting control files individually)

## Accomplishments

- Tracer: `denylist` step, the two lists, `docs/provenance.md` and the seeded-hit control `denylist-hit` (a surname in `src/x.c` gives `FAIL denylist-hit: src/x.c`). The gate was green end to end before expansion.
- `docs/provenance.md`: facts versus expression, tiers, copyleft and permissive emulator source rules (including the disclosed research-era licence reads), the name of the process, AI assistance, the seven-item checklist, how the checks work (scope, waiver, why `.claude/` is excluded), honest limits and what the file does not do.
- `docs/provenance/CONSULTATIONS.md`: row 1, the permissive driver (`SRC-mame-intv`) read for licence headers and crystal-divisor lines, re-derived from `SRC-gi-1980-catalog` pdf 191 (research-era reading, not re-read here; CPU divisor 4 stays an inference); row 2, the GPL fork (`SRC-community-emu-docs`) read for licence determination only, marked a disclosed prior exception, no value taken, no copy kept.
- `.github/pull_request_template.md`: the seven D-14 items plus claim-citation, no-private-path and gate items (10 task-list lines).
- `source` kind: all 13 keys have `req` lines; hashed entries need a 64-hex digest, a numeric size and `refs/<id>.<ext>`; a present cached file is compared on size and sha256. Real tree: `sources=9 refs-verified=1/4` (only the GI manual is cached locally).
- `sources` step: SRC tokens from root-anchored `data/*.json`, `docs/*.md` and `README.md` must be registered; the unknown-ref control overlay lives under `tools/check/` so it never fails the real tree.
- Existing tracked tree was checked against the denylist before enabling the step: no hits, so no paraphrasing or widened exclusions were needed.

## Task Commits

1. Task 1 (tracer): `76d59a1` feat(01-07): tracer denylist step, provenance rule and seeded-hit control
2. Task 2: `1d436d6` feat(01-07): consultation log, PR template, source-registry check and provenance controls

## Verification (executed)

- `sh tools/gate.sh` and `dash tools/gate.sh`: `gate: PASS steps=10/10 controls=64/64 claims=4 provisional=3 scanned=32 waived=0 discrepancies=35 pending-resolvers=18 open=33 tracked=321 binaries-listed=0 files=321 manifests=1 effective-yes=0 effective-no=1 records=23 fields=23 unknown-fields=12 sources=9 refs-verified=1/4 docs=6 claim-refs=2 tags=10 tiers=8`.
- `sh tools/check/run-controls.sh | tail -1` is `controls=64/64`; all nine provenance controls report ok.
- Acceptance greps: denylist lists have 4 and 1 lines; `docs/provenance.md` has 11 `##` headings and both required phrases; PR template has 10 task-list lines; R1 and R2 greps over both new docs print nothing; the SRC scan uses `^docs/` and `^README` anchors and `source-unknown-ref: SRC-nope` does not appear on the real tree; all four GPL-fork file names are in the log; all nine plan-01-14 ids are still in `data/expected/sources.ids` (add-only).

## Deviations from Plan

### Changed from the plan wording (no check weakened)

- **Stat key:** the plan named the denylist stat `files=<n>`; the step prints `scanned=<n>` because `files=` is already emitted by the integrity step (see key-decisions). `waived=<k>` is as planned.
- **Control inventory:** `data/expected/controls/provenance.ids` lists nine controls, not seven. Extra: `sources-local-path` (hashed entry with a `local_path` that is not `refs/<id>.<ext>`), `sources-reference-only-unexplained` (reference-only entry whose note does not explain the missing hash), both because the hook tokens `source-local-path` and `source-reference-only-unexplained` were otherwise untested. The seven planned controls plus these two make nine.
- **`sources-bad-digest-format` token:** the `match sha256` rule reports `FAIL source-format: SRC-bsr-2609/sha256`; the hook also reports the length failure under the same token.

### Auto-fixed Issues

None.

### Notes (not deviations)

- Controls that change registry content (`sources-bad-digest-format`, `sources-missing-key`, `sources-local-path`, `sources-reference-only-unexplained`) overlay a whole `data/sources.json` because `MAKE` has no JSON edit verb; `sources-digest-mismatch` uses the `write` verb.
- `data/sources.json` is unchanged by this plan.
- The `.planning/config.json` modification predates this plan and was not staged.
- FND-05 and FND-06 are not marked complete: FND-05 also depends on plans 01-11 and 01-12, and FND-06 on plans 01-02 and 01-03.
- The records DSL whitespace limitation noted by plans 01-04 and 01-05 does not affect this plan: no regex in `source.txt` contains a space.
- Duplicate JSON keys are not detectable, and the checks catch accidents, not authorship; both limits are stated in `docs/provenance.md`.

## Known Stubs

None.

## Threat Flags

None. T-01-18: the denylist step, consultation log and checklist exist, and the limit is stated. T-01-19: `source-digest-mismatch` has a control. T-01-20: the step prints file paths only, never matched text. No package installs occurred.

## Issues Encountered

None. A full gate takes about 3 seconds on an idle machine.

## Next Phase Readiness

Every later plan that cites a source must use a registered `SRC-<slug>` id (placeholders as `SRC-<slug>`). A new registry entry needs one line in `data/expected/sources.ids`. Emulator project names in docs or data need `[cite-only]`, and author names and licence headers are never allowed outside the excluded paths.

## Self-Check: PASSED

- All files listed under key-files exist, and commits `76d59a1` and `1d436d6` are ancestors of HEAD.
