---
phase: 01-evidence-profile-and-rights-foundations
plan: 05
subsystem: evidence-records
tags: [claim-ledger, evidence-tiers, claim-language-tripwire, gate-step, negative-controls, fnd-02]
requires:
  - phase: 01-evidence-profile-and-rights-foundations
    provides: "records.cmake generic checker, tiers.json vocabulary, profile record and controls runner (plan 01-01); reference notes with the C17 and C19 sections (plan 01-02); discrepancy log C31 (plan 01-04)"
provides:
  - "data/claims.json: claims CL-0001 to CL-0004, each with profile, evidence tier, existing artifact, depended-on profile fields and standing"
  - "claim record kind (schema, hook, inventory) and the claims gate step with stats claims= and provisional="
  - "text gate step: claim-word tripwire (R1), claim-reference resolution, tier ids documented; stats docs= and claim-refs="
  - "docs/evidence-tiers.md: the two vocabularies, claim ledger fields, weakest-field rule, citation form, honest limits"
  - "Thirteen negative controls (five claims-*, four more claims-* edge cases, four text-*)"
affects: [01-07 sources and denylist steps (scan docs/evidence-tiers.md), 01-09 notes gate checks, 01-11 and 01-13 (also list FND-02), every later plan that cites a claim in docs]
actuals:
  tokens: 21500
  tasks: 2
  commits: 2
plan_head_before: 9cd7a8d4fb5d5d5c00674d9064568110afd49ef0
plan_head_after: b27af7e483dcd856247af196757726128c5028d4
tech-stack:
  added: []
  patterns:
    - "Hook caches a profile (field paths and tags) in GLOBAL properties keyed by profile id; no variable names are built from ids"
    - "Step name differs from record kind: the claims step re-labels the checker success line so the gate collects claims= and provisional="
    - "sh text scan over FILELIST only: grep -n -i -w for the word list, findings sorted by file then numeric line"
key-files:
  created:
    - docs/evidence-tiers.md
    - data/claims.json
    - data/expected/claims.ids
    - data/expected/gate-steps/claims.ids
    - data/expected/controls/claims.ids
    - tools/check/schema/claim.txt
    - tools/check/kinds/claim.cmake
    - tools/check/steps/claims.sh
    - tools/check/steps/text.sh
    - tools/check/controls/_good/tree/data/claims.json
    - tools/check/controls/_good/tree/data/expected/claims.ids
    - tools/check/controls/_good/tree/docs/evidence-tiers.md
    - tools/check/controls/claims-dangling-artifact/
    - tools/check/controls/claims-missing-key/
    - tools/check/controls/claims-unknown-tier/
    - tools/check/controls/claims-overstated-standing/
    - tools/check/controls/claims-unknown-field/
    - tools/check/controls/claims-order/
    - tools/check/controls/claims-duplicate-id/
    - tools/check/controls/claims-empty/
    - tools/check/controls/claims-inventory-empty/
    - tools/check/controls/text-untagged-claim-word/
    - tools/check/controls/text-missing-claim-ref/
    - tools/check/controls/text-undocumented-tier/
    - tools/check/controls/text-no-files/
  modified:
    - docs/cp1610/reference-notes.md
key-decisions:
  - "CL-0001 states the manual's stack reading as resolved-primary (C17), stays provisional because applying the CP-1600 manual to the CP-1610 is C31"
  - "CL-0002 is the only established claim: all three depended-on profile fields carry tier1-primary-doc"
  - "CL-0003 and CL-0004 are provisional (tier4-community and inference fields; C19 partial)"
  - "The documented placeholder [claim:CL-NNNN] is the only claim-reference token the text step exempts from resolution, and it is excluded from the claim-refs count"
requirements-completed: []
duration: about 20min
completed: 2026-11-09
status: complete
---

# Phase 1 Plan 05: Evidence tiers and claim ledger Summary

**Four claims are machine-checked against their profile, evidence tier, artifact and the weakest-field rule, and docs and README are scanned for untagged claim words and unresolved `[claim:CL-NNNN]` references, all proven by thirteen negative controls.**

## Performance

- **Duration:** about 20 min
- **Tasks:** 2 of 2 (tracer, then vocabulary doc, text step, remaining claims and controls)
- **Files:** 25 created, 1 modified (counting control directories as one each)

## Accomplishments

- Claim kind: the schema has a `req` line for all eight keys, `match` on the `CL-NNNN` id and on standing, and `enumfrom tier data/tiers.json evidence_tiers id`. The hook checks ascending ids, profile file exists, artifact is a repository-relative existing file (charset, no leading slash, no `..`), every `depends_on` entry is a member of the profile `fields`, the weakest-field rule and `measured-hardware` without D10.
- Claims: CL-0001 (stack direction, provisional), CL-0002 (NTSC STIC part and crystal, established), CL-0003 (frame timing 262 x 57 = 14,934 CPU cycles = 59,736 master ticks, provisional), CL-0004 (interruptibility as the manual marks it, provisional). Gate summary shows `claims=4 provisional=3`.
- Text step: R1 tripwire exactly as SKELETON section 7, `[claim:...]` resolution against `data/expected/claims.ids`, every tag and tier id backticked in `docs/evidence-tiers.md`, findings in sorted file and numeric line order, `text-no-files` when nothing is scanned. Gate summary shows `docs=2 claim-refs=2`.
- Notes: `[claim:CL-0001]` on the "three pages agree" stack line and `[claim:CL-0004]` on the interruptibility conclusion line; the `C17 status:` line and everything else are unchanged.
- Tracer gate re-run before expansion: `-- claims pass claims=1 provisional=1`, both tracer controls ok, then expanded.

## Task Commits

1. Task 1 (tracer): `15376aa` feat(01-05): claim ledger tracer with CL-0001, schema, hook, step and two controls
2. Task 2: `b27af7e` feat(01-05): evidence-tier doc, claims CL-0002 to CL-0004, text step and eleven controls

## Verification (executed)

- `sh tools/gate.sh` and `dash tools/gate.sh`: `gate: PASS steps=6/6 controls=40/40 claims=4 provisional=3 discrepancies=35 pending-resolvers=18 open=33 files=218 records=23 fields=23 unknown-fields=12 docs=2 claim-refs=2 tags=10 tiers=8`.
- `sh tools/gate.sh | grep -F -- '-- text pass'` matches (`-- text pass docs=2 claim-refs=2`); `grep -c '\[claim:CL-000[14]\]' docs/cp1610/reference-notes.md` is 2; `sh tools/check/run-controls.sh | tail -1` is `controls=40/40`.
- R1 and R2 greps over `docs/evidence-tiers.md`, the good-tree doc and the notes print nothing; no absolute home or scratch path appears in any tracked file written here.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Dropped the profile-id-equals-file-id check from the claim hook**
- **Found during:** Task 1 (first gate run reported `control-good-failed: claims`)
- **Issue:** The hook first required the profile JSON member `id` to equal the file name. The known-good control tree profile fragment is stored as `intv-2609-ntsc-family-p1.json` but carries `"id": "good-profile"`, so the good claim failed `claim-unknown-profile`. The plan only requires that `data/profiles/<profile>.json` exists.
- **Fix:** Removed the extra check rather than editing plan 01-01's good-tree fragment; the plan's requirement (profile file exists, fields are members of its `fields`) is unchanged and `claim-unknown-profile` still fires for a missing or unparsable profile file.
- **Files modified:** `tools/check/kinds/claim.cmake`
- **Commit:** `15376aa`

**2. [Rule 3 - Blocking] `json_members` takes a JSON string, not a path**
- **Found during:** Task 1 (first run)
- **Issue:** The hook called `json_members(paths "${pdoc}" fields)`; the helper in `lib.cmake` has two parameters.
- **Fix:** Extract the `fields` object with `string(JSON ... GET)` first and pass that.
- **Commit:** `15376aa`

### Additions beyond the plan (Rule 2, no check weakened)

- Four extra controls for must-have truths that the plan listed without a control: `claims-duplicate-id`, `claims-empty`, `claims-inventory-empty` and `text-no-files`. Total controls added: 13 (plan named 9).
- The artifact path check also rejects non-files (a directory), paths outside the safe charset, a leading `/` and `..`, to keep a ledger entry from pointing outside the tree.
- The text step exempts only the literal placeholder `CL-NNNN` from reference resolution so the evidence document can state the citation form; a malformed id such as `CL-12` still fails `text-unknown-claim-ref`.

### Notes (not deviations)

- FND-02 is already checked in REQUIREMENTS.md (marked by plan 01-01) and is also listed by plans 01-11 and 01-13, so no requirement was marked here and `requirements-completed` is empty. This plan delivers the vocabulary and ledger half named in ROADMAP success criterion 2.
- The `.planning/config.json` modification in the working tree predates this plan and was not staged.
- The records DSL still splits schema lines on whitespace (plan 01-04 note); no regex in the claim schema holds a space.
- Duplicate JSON member keys are not detectable (known limit, SKELETON section 8).
- Tag truth is review-only and the tripwire word list does not catch paraphrases such as "works with"; both limits are stated in `docs/evidence-tiers.md` under Honest limits.

## Known Stubs

None.

## Threat Flags

None. T-01-13: the tripwire, reference resolution, weakest-field rule and controls are in place and the paraphrase gap is documented. T-01-14: the text step reads only FILELIST paths matching a fixed anchored pattern, uses `grep -F`/`-x` for ids, has no `eval`, and uses only a `mktemp -d` directory with trap cleanup.

## Issues Encountered

None beyond the two fixes above. A full gate takes about 2 to 3 seconds on an idle machine.

## Next Phase Readiness

Plan 01-07 can add the denylist and sources steps over `docs/evidence-tiers.md` (it cites no `SRC-` ids). Plan 01-09 can still check the unchanged `C17 status:` line. Later plans add claims by appending `CL-NNNN` entries and inventory lines and citing them as `[claim:CL-NNNN]`.

## Self-Check: PASSED

- All created files exist and commits `15376aa` and `b27af7e` are ancestors of HEAD (verified before the docs commit).
