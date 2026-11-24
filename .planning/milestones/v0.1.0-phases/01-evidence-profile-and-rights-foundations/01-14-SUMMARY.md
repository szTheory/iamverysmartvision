---
phase: 01-evidence-profile-and-rights-foundations
plan: 14
subsystem: provenance
tags: [sources-registry, provenance, rights-unknown, reference-documents]
requires:
  - phase: 01-evidence-profile-and-rights-foundations
    provides: "tiers.json authority-tag vocabulary and the gate (plan 01-01)"
provides:
  - "data/sources.json: nine-entry reference-document registry (D-15) with pinned sha256 for hashed copies, rights unknown everywhere"
  - "Corrected bootstrap source defects recorded as reference-only entries (owner-manual 404, iFixit HTML viewer)"
affects: [01-02 refs cache verification, 01-04 discrepancies, 01-07 sources record kind, CP-1610 reference notes]
actuals:
  tokens: 2600
  tasks: 2
  commits: 2
plan_head_before: 036c5531992075e1dba2e5aaaa3b63b934a4b1d0
plan_head_after: c76442941f36e49992243b80394cd28e2e025a45
tech-stack:
  added: []
  patterns:
    - "Registry entry shape: 13 fixed keys, capture hashed|reference-only, null sha256/size/local_path for reference-only"
key-files:
  created:
    - data/sources.json
  modified: []
key-decisions:
  - "Blue Sky page re-hashed in scratch: digest and size equal the research-era values, so no difference is recorded"
  - "Community wiki and community docs tree stay reference-only because a moving wiki or directory tree has no single stable hash"
  - "Owner-manual candidate copy is not hashed or opened; entry stays reference-only"
requirements-completed: []  # FND-06 is only partly served by this plan (sources registry); the opcode/encoding/cycle notes land in plans 01-02 and 01-03, so FND-06 stays open in REQUIREMENTS.md
duration: 6min
completed: 2026-11-09
status: complete
---

# Phase 1 Plan 14: Reference-document registry Summary

**`data/sources.json` registers nine reference documents by stable `SRC-` id with pinned sha256 for the GI manual, both GI catalogs and the Blue Sky page, and every entry marked `rights: unknown`.**

## Performance

- **Duration:** about 6 min
- **Tasks:** 2 of 2 (tracer, then remaining entries)
- **Files:** 1 created (`data/sources.json`)

## Accomplishments

- Tracer: registry shape fixed with the GI CP-1600 manual (sha256 `ecdabe5c...dc2b72`, 9,917,417 bytes, 116 pages) and the MAME driver as reference-only; throwaway parse passed.
- Remaining seven entries added in the plan's order: GI 1980 and 1982 catalogs (notes say hash and page readings are research-era provenance, not re-read in this phase), Blue Sky 2609 page (hashed html, Wayback URL), community wiki, community emulator docs tree, owner manual, iFixit service manual.
- The two bootstrap defects are recorded as corrected reference-only entries: the owner-manual URL returns 404, and the iFixit URL serves an HTML viewer rather than a PDF (21 pages per page metadata; MTE-201 and burn-in claims unverified).
- The Blue Sky page was downloaded with a browser User-Agent into the session scratchpad only, hashed with `cmake -E sha256sum`: 58,009 bytes, sha256 `3d779159...384819`, identical to the research value.

## Task Commits

1. Task 1 (tracer): `21c11a3` feat(01-14): tracer reference registry with manual and one reference-only entry
2. Task 2: `c764429` feat(01-14): register remaining reference documents and corrected bootstrap defects

## Verification (executed)

- Task 1 parse: 2 unique ids, all 13 keys, rights unknown. Re-run end to end before expanding (tracer gate): pass.
- Task 2 parse: 9 unique ids, all 13 keys, capture/sha256 consistent, sha256 64 lowercase hex, reference-only entries have null sha256/size/local_path, notes non-blank: pass.
- `grep -c '"id": "SRC-'` = 9; `research-era` matches 4 lines (two catalogs plus the Blue Sky and community-docs notes); R3 author-name grep and R4 absolute-path grep both clean; non-printable-byte scan empty; no CR bytes.
- `git ls-files refs` empty; `refs/` does not exist; nothing written under `refs/` or `.git/info/exclude`.
- `sh tools/gate.sh`: `gate: PASS steps=3/3 controls=20/20 files=129 records=23 fields=23 unknown-fields=12 tags=10 tiers=8` (run before the Task 2 commit, working tree containing the final `data/sources.json`).

## Deviations from Plan

### Auto-fixed Issues

None for code. Notes:

**1. [Rule 3 - Attribution] Commit trailer names Sonnet 5.5, not Opus 5.5**
- **Found during:** Task 1 commit
- **Issue:** The dispatch prompt asks for an `Opus 5.5` Co-Authored-By trailer; the runtime attribution reminder specifies `Sonnet 5.5`, the model actually running. Prior commits already mix both.
- **Fix:** Used `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>` for accuracy.
- **Commits:** `21c11a3`, `c764429`

**2. [Rule 1 - Premature completion] FND-06 not marked complete**
- **Found during:** state updates
- **Issue:** `requirements.mark-complete FND-06` (from this plan's frontmatter) ticked FND-06, but FND-06 requires the CP-1610 reference notes, which plans 01-02 and 01-03 deliver. Ticking it would overstate evidence.
- **Fix:** Reverted the REQUIREMENTS.md change; FND-06 stays Pending until those plans finish.

**3. [Interpretation] Registry values beyond the plan text**
- `SRC-mattel-owner-manual` carries the bootstrap URL from `.planning/bootstrap/EMULATOR-DNA.md` (the plan did not spell it out); `SRC-intv-wiki` carries the Hand Controller page URL with the research-read revision number in the note; `wayback` is null for entries with no known snapshot (only the Blue Sky page has one).
- `SRC-ifixit-2609-service-manual` has `pages: 21` (page metadata) despite being reference-only; the contract allows a numeric `pages` for any entry.

## Known Stubs

None.

## Threat Flags

None. T-01-38 and T-01-39 mitigations were applied: the downloaded page stayed in the scratchpad, and the author-name and absolute-path greps passed.

## Issues Encountered

None.

## Next Phase Readiness

Plan 01-02 can set up `refs/` and verify cached copies against these digests; plan 01-07's `sources` step can validate the registry as a record kind. The research-era GI catalog digests are not yet independently re-hashed in this phase (declared in their notes).

## Self-Check: PASSED

- `data/sources.json` exists; commits `21c11a3` and `c764429` are ancestors of HEAD.
