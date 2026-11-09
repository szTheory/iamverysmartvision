---
phase: 01-evidence-profile-and-rights-foundations
plan: 02
subsystem: reference-notes
tags: [cp1610, reference-notes, page-image-reading, refs-cache, c17, c19]
requires:
  - phase: 01-evidence-profile-and-rights-foundations
    provides: "data/sources.json registry with the pinned GI manual digest (plan 01-14); the gate (plan 01-01)"
provides:
  - "docs/cp1610/reference-notes.md: 15 sections, page-cited from page images (formats, addressing, stack, SDBD, unsupported encodings, interruptibility, reset/MSYNC/IAB, interrupt sequence, bus requests, start/stop, branch timing, discrepancy candidates, Phase 2 open items)"
  - "Line `C17 status: resolved-primary`, derived from this phase's reading of pdf 24, 46, 48 and 62"
  - "Local, ignored refs/ cache holding the manual (sha256 and size verified)"
affects: [01-03 opcode families, 01-04 discrepancies, 01-09 notes gate checks, Phase 2 CPU vectors]
actuals:
  tokens: 7100
  tasks: 3
  commits: 3
plan_head_before: 1dc48f0055ecf44ce2f381806a7c620800982ba2
plan_head_after: c07778af39d6c9ade5ff7760d5e543911f26c9de
tech-stack:
  added: []
  patterns:
    - "Every fact line carries a `pdf <n>` cite and an [OBS] or [INF] label; one sentence per line"
key-files:
  created:
    - docs/cp1610/reference-notes.md
  modified: []
key-decisions:
  - "C17 recorded as resolved-primary only after the page images (pdf 24, 46, 48, 62) were read in this phase; the image reading matched D-21"
  - "Interrupt-entry (nine microcycles before the first service-routine BAR) and reset (IAB in the second, first BAR in the sixth microcycle) counts are labelled [INF] because they come from counting time-slot labels on the timing diagrams, not from a printed table"
  - "Candidate (e) stack-direction was not listed in Discrepancy candidates because C17 is resolved-primary"
requirements-completed: []  # FND-06 also depends on 01-03, 01-09 and 01-10; left open on purpose
duration: about 35min
completed: 2026-11-09
status: complete
---

# Phase 1 Plan 02: CP-1610 reference notes Summary

**`docs/cp1610/reference-notes.md` records the CP-1600 manual's formats, addressing, stack, SDBD, interrupt, reset, bus-request, start/stop and branch-timing facts with `pdf <n>` cites read from page images, and states `C17 status: resolved-primary`.**

## Performance

- **Duration:** about 35 min
- **Tasks:** 3 of 3 (tracer, SDBD/interruptibility, reset/interrupt/bus/branch and discrepancies)
- **Files:** 1 created (`docs/cp1610/reference-notes.md`, about 28 KB, 15 sections)

## Accomplishments

- Cache setup (this plan is the single owner): `/refs/` is present in `.git/info/exclude` (and also in `.gitignore` from plan 01-01); the manual was copied from the local copy named in D-16 (route: local copy, not download) to `refs/SRC-gi-cp1600-um-1975.pdf`; `cmake -E sha256sum` printed `ecdabe5c52b5e4429a40d1779d82315ab74bae9db00df774e605f8a882dc2b72` and `wc -c` printed 9917417. `git check-ignore -q` exits 0 and `git ls-files refs` is empty.
- Page images were rendered with `pdftoppm -r 110` for pdf 12-14, 21-25, 27, 32, 35, 38, 46-58, 62, 74 and 75 and each was opened with the Read tool; pdf 35, 27 and 52 were re-rendered at higher resolution and cropped to count time slots or read a smudged digit. OCR text was used only to locate pages.
- **C17 status: resolved-primary.** pdf 62 (R6 post-incremented on output, pre-decremented before input), pdf 46 (stack expands upward, low to high address), pdf 48 Fig. 3.2.4 and pdf 24 (interrupt PC store then increment) agree with D-21. No D-21 deviation.
- **C19: partial** recorded exactly as D-21 states: shifts (pdf 53) and EIS, DIS, TCI, CLRC, SETC (pdf 56) are marked not interruptable; HLT, SDBD and MVO are `manual-silent`.
- Reset: pdf 21 and 27 state that registers and status bits are undefined and that the start PC is read from the bus by IAB after MSYNC* goes high; the notes cross-reference the profile fields `reset.r7`, `reset.registers_other` and `reset.flags` as non-manual defaults.
- Seven discrepancy candidates (unlabelled `7/9`, untabulated interrupt-entry cycles, CP-1600 vs CP-1610, device-supplied reset PC, shift mode-bit naming vs mnemonic table, SDBD indirect cycle ambiguity, stop wording) and seven Phase 2 open items listed for plan 01-04.

## Task Commits

1. Task 1 (tracer): `1962ae2` docs(01-02): cp1610 notes with refs cache, C17 stack fact, formats and addressing
2. Task 2: `186add5` docs(01-02): cp1610 notes on SDBD, unsupported encodings and interruptibility
3. Task 3: `c07778a` docs(01-02): cp1610 notes on reset, interrupts, bus requests, start/stop and branch timing

## Verification (executed)

- Tracer gate (re-run before expansion): `grep -c -E 'pdf [0-9]+'` printed 62 (threshold 10), exactly one `C17 status:` line, 5 sections; passed, then expanded.
- Final: `grep -c '^## '` = 15 (threshold 12); `grep -c -E 'pdf (12|13|14|21|...|75)'` = 54 (threshold 10); `grep -c -E 'pdf 5[3-8]'` = 30 (threshold 8); `IAB` matches 12 lines; `Tentative` matches; 7 candidates in the Discrepancy section (minimum 4); `manual-silent`, `C19: partial` and `modes 0 and 6` all match.
- SKELETON §7 R1 (claim words), R2 (bare clock), R3 (provenance names) and R4 (private paths) negated greps all print nothing; the file is ASCII-clean apart from section signs, LF only, no BOM.
- `git ls-files refs` prints nothing; no PDF or page image is tracked.
- `sh tools/gate.sh`: `gate: PASS steps=3/3 controls=20/20 files=131 records=23 fields=23 unknown-fields=12 tags=10 tiers=8`.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Unread claim] "Tentative" electrical specifications cite**
- **Found during:** Task 3 self-check
- **Issue:** Task 1's notes asserted the electrical specifications are marked Tentative (D-17) before any page image showing it had been read in this phase.
- **Fix:** Located the marking by OCR (page finding only), read pdf 38 (§2.4, heading "(Tentative)") and cited `§2.4, pdf 38` [OBS] in Source and method and in the CP-1600/CP-1610 candidate.
- **Commit:** `c07778a`

**2. [Rule 1 - Overreach] Removed two unsupported lines before the Task 3 commit**
- **Found during:** Task 3 self-check
- **Issue:** A speculative line about BUSRQ* sampling during wait states and a "candidate" that was in fact a consistency note (stack direction, no disagreement) would have implied discrepancies the manual does not contain.
- **Fix:** Both removed; the stack-direction candidate (e) stays out because C17 is `resolved-primary`.

### Notes (not deviations)

- Section numbers printed as `C.1`, `C.5` and `C.6` on pdf 21, 24 and 25 are cited as printed (`§C.n`); the pin description pages 12-14 carry no printed heading and are cited by pdf page alone.
- The last digit of the saved-return-address offset on pdf 52 is smudged in the scan even at 200 dpi; the notes say so and call 3 consistent with the three-location instruction length [INF].
- Fig. 3.3.2 (pdf 50) and the shift table (pdf 53) disagree in naming for left shifts (SLL is the pattern with A = 1, but A = 1 is "arithmetic"); logged as a candidate and left for Phase 2 to resolve, with the table treated as the operative statement.
- pdf 32 (BEXT timing) does not resolve the `7/9` branch count: it stops during the displacement fetch. No other branch timing figure was found among pdf 26-37 by page title, so the question remains open.
- The commit trailer names Sonnet 5.5 (the running model, per the runtime attribution reminder).
- FND-06 was not marked complete: it also depends on plans 01-03, 01-09 and 01-10.

## Known Stubs

None.

## Threat Flags

None. T-01-06 (refs cache): `.git/info/exclude` entry present before any file was placed under `refs/`, `git ls-files refs` empty. T-01-07: sha256 and size matched before any page was read. T-01-08: no absolute home or scratch path in the notes or this SUMMARY.

## Issues Encountered

A zsh word-splitting slip in the first `pdftoppm` loop printed usage text and rendered nothing; the pages were then rendered with explicit arguments. No effect on the notes.

## Next Phase Readiness

Plan 01-04 can turn the seven discrepancy candidates into C29+ entries (no C numbers assigned in the notes). Plan 01-09 can check `C17 status: resolved-primary`. Plan 01-03 can use pdf 53-58 readings (confirmed in this plan's SDBD and unsupported-encoding sections) as a cross-check but must hand-read its own rows. The second-reader pass (D-20) remains a separate agent's work.

## Self-Check: PASSED

- `docs/cp1610/reference-notes.md` exists; commits `1962ae2`, `186add5` and `c07778a` are ancestors of HEAD.
