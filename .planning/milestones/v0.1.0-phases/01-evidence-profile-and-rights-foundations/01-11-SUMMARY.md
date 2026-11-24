---
phase: 01-evidence-profile-and-rights-foundations
plan: 11
subsystem: public-repo-hygiene
tags: [hygiene, secrets, home-paths, readme, licence, trademark, fnd-02, fnd-05]
requires:
  - phase: 01-evidence-profile-and-rights-foundations
    provides: "gate and MAKE-verb control runner (01-01), rights steps (01-06), provenance steps (01-07), glossary step (01-08), CI parity (01-13)"
provides:
  - "tools/hygiene.sh with --tree (gate step) and --history (publication tool) modes"
  - "gate step hygiene and four controls: hygiene-home-path, hygiene-secret, hygiene-password, hygiene-foreign-email"
  - "README.md (evidence-bound wording, claim tags, non-affiliation and trademark notice) and MIT LICENSE"
  - "tracked tree scrubbed of absolute home and scratch paths"
affects: [01-12 history rewrite and publication, every later gate run]
actuals:
  tokens: 3100
  tasks: 2
  commits: 2
plan_head_before: b66f224e3e9edc39eef56e7b4cd50438bc05f177
plan_head_after: 38481c4d64a8800f21cb860a6eb75a613e454625
tech-stack:
  added: []
  patterns:
    - "Batch candidate scan with xargs -0 grep -l, then per-candidate filtering with grep -o against an anchored allow pattern"
    - "Secret-shaped regexes written so the script does not match itself (bracketed first character on the password pattern)"
key-files:
  created:
    - tools/hygiene.sh
    - tools/check/steps/hygiene.sh
    - data/expected/gate-steps/hygiene.ids
    - data/expected/controls/hygiene.ids
    - tools/check/controls/hygiene-home-path/
    - tools/check/controls/hygiene-secret/
    - tools/check/controls/hygiene-password/
    - tools/check/controls/hygiene-foreign-email/
    - README.md
    - LICENSE
  modified:
    - .claude/CLAUDE.md
    - .planning/research/ARCHITECTURE.md
    - .planning/research/FEATURES.md
    - .planning/research/STACK.md
    - .planning/research/SUMMARY.md
    - .planning/phases/01-evidence-profile-and-rights-foundations/01-CONTEXT.md
key-decisions:
  - "The hygiene success line uses the key hygiene-files=<n>, not files=<n>, because the integrity step already prints files=<n> and the gate summary would show a duplicate key"
  - "Allow patterns (home and email) are applied to grep -o output and anchored at the end, so a look-alike suffix domain such as an example domain followed by another host is still a finding"
  - "FND-05 is not marked complete here: plan 01-12 also lists it and the publication step is the last piece; FND-02 was already complete"
patterns-established:
  - "A script that holds secret-shaped regexes must not match itself; there is no self-exemption in hygiene.sh"
requirements-completed: []  # FND-02 already complete; FND-05 left to 01-12 on purpose
duration: about 40 min of tool time
completed: 2026-11-09
status: complete
---

# Phase 1 Plan 11: Public-repo hygiene, README and LICENSE Summary

**`sh tools/hygiene.sh --tree` now runs on every gate run and fails on home or scratch paths, token-shaped secrets, password assignments and non-noreply emails (findings name rule and file only), the tracked tree is scrubbed of absolute paths, and README.md and an MIT LICENSE exist; a clean clone passes `dash tools/gate.sh`.**

## Performance

- **Gate latency, all 81 controls and 15 steps (measured, host heavily loaded: load average 11 to 17):** `sh tools/gate.sh` took 8.11, 7.85 and 8.08 s wall time. The median is 8.08 s, below the 10 s budget, so the parallel/batch fallback was not applied (the plan applies it only when the idle-host figure also exceeds 10 s; the figure was already under budget on a loaded host). Time split: `sh tools/check/run-controls.sh` alone is 6.0 s, and no single step takes more than 1 s (cp1610 and runner-selftest are the longest at about 1 s). The hygiene step itself takes about 0.5 s on 407 files.
- **cp1610 step against the 2 s rule (orchestrator disposition I-4):** about 1 s, under the 2 s trigger, so the 1024-pattern decode stays in the gate and nothing moved to a Phase 2 C test.
- **Clean-clone rehearsal:** `dash tools/gate.sh` in a fresh clone outside the repository took 7.95 s.
- **Tasks:** 2 of 2.

## Accomplishments

- **`tools/hygiene.sh`.** POSIX sh, `LC_ALL=C`, `set -eu`. Usage error (exit 2) on anything but `--tree` or `--history`. Regex set from RESEARCH (home path with alphanumeric first character, scratch slug and Windows forms; four token shapes; email shape) plus the W6 password-assignment pattern (case-insensitive word, optional spaces, `=` or `:`, optional quote, value starting alphanumeric with at least 8 value characters). The pattern does not match the bare word, a backticked prefix or placeholders such as `<...>` and `$VAR`; it had zero hits on the real tree before the control was added. Findings are `-- FAIL hygiene-<rule>: <file>` and never print matched text.
- **`--tree`.** Reads `ROOT` and `FILELIST` (the gate seam) or falls back to git. Readability is checked per file (`hygiene-unreadable`), the content scans are batched with `xargs -0 grep -l` over `./`-prefixed names, and only candidate files are filtered one by one. Success line: `-- hygiene pass mode=tree hygiene-files=<n>`.
- **`--history`.** For every commit in `git rev-list --all`: tree scan for home paths and secrets, message scan for paths, secrets and emails, author and committer identity (`hygiene-identity`), and every historical file name against the ROM/PDF/image extension set, `refs/` and `.env*` (`hygiene-history-binary`). It is a publication and release step, not a gate step.
- **Gate step and controls.** `tools/check/steps/hygiene.sh` runs `hygiene.sh --tree` with `ROOT` and `FILELIST`. Four controls build their bad bytes at run time with the MAKE verbs (`homepath`, `secret`, `password`, `email`); none carries a bad byte in a tracked file. All four report ok and the pristine good tree passes the step (`control ok   _good/hygiene`).
- **Scrub.** Home-path substrings in `.claude/CLAUDE.md` (one line, path substrings only), `.planning/research/{ARCHITECTURE,FEATURES,STACK,SUMMARY}.md` and `01-CONTEXT.md` were replaced with `../<sibling>` names, repo-relative `./` paths, and a neutral phrase for the scratch copy of the CP-1600 manual (path intentionally not recorded). The sed used the portable `-E -e ... > tmp && mv` form; each diff was reviewed. No other tracked file was flagged, so no earlier plan's SUMMARY needed a scrub.
- **README.md and LICENSE.** README follows the plan sections, one sentence per line, with `[claim:CL-0002]` and `[claim:CL-0003]` on the lines that state those facts, no bare unit word and no unqualified compatibility wording. It carries the non-affiliation and trademark notice and says no machine code exists yet and no release exists. LICENSE is the standard MIT text with `Copyright (c) 2026 iamverysmartvision contributors` (adapted from the sibling form).

## Task Commits

1. Task 1 (tracer): `7eaeba1` feat(01-11): hygiene --tree gate step, home-path control and tracked-path scrub
2. Task 2: `38481c4` feat(01-11): README, MIT LICENSE and hygiene secret, password and email controls

## Verification (executed)

- Tracer gate (auto mode), after Task 1: `sh tools/gate.sh` and `dash tools/gate.sh` ended `gate: PASS steps=15/15 controls=78/78` with `-- hygiene pass mode=tree`; expanded afterwards.
- **`sh tools/hygiene.sh --tree` result line:** `-- hygiene pass mode=tree hygiene-files=407`.
- **Final gate (sh and dash):** `gate: PASS steps=15/15 controls=81/81 ci-uses=1 claims=5 ... refs-verified=1/4 ...` (the tail carries the remaining stat tokens, including `mode=tree hygiene-files=409`).
- **Clean clone (no push; cloned into a scratch directory outside the repository, deleted afterwards; refs/ absent):** `dash tools/gate.sh` ended `gate: PASS steps=15/15 controls=81/81 ... refs-verified=0/4 ...` with `-- hygiene pass mode=tree hygiene-files=407`; exit code 0.
- `git grep -n -I -E -e '/Users/[A-Za-z0-9]' -e '-Users-[A-Za-z0-9]'` prints nothing; a search for scratch-directory slugs found only the word "scratchpad" in prose, no path.
- `sh tools/hygiene.sh --bogus` prints the usage line and exits 2.
- `sh tools/check/run-controls.sh | tail -1` is `controls=81/81`; hygiene-home-path, hygiene-secret, hygiene-password and hygiene-foreign-email all report ok.
- Acceptance greps: `Copyright (c) 2026 iamverysmartvision contributors` and `Permission is hereby granted, free of charge` match in LICENSE; `not affiliated` and `sh tools/gate.sh` match in README.md; the count of `[claim:CL-0002]` or `[claim:CL-0003]` in README.md is 2.
- **`--history` informational run (expected, history is rewritten in 01-12):** 57 commits scanned. Counts per rule: `hygiene-history-home-path` 53 (commits whose trees contain a home path); `hygiene-history-secret` 0; `hygiene-message-home-path` 0; `hygiene-message-secret` 0; `hygiene-message-email` 0; `hygiene-identity` 0; `hygiene-history-binary` 0. Matched text is not recorded.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Hygiene script matched its own password pattern**
- **Found during:** Task 1 first `--tree` run (`hygiene-secret: tools/hygiene.sh`)
- **Issue:** The variable assignment itself (name, quote, then an alphanumeric run) satisfies the case-insensitive pattern (it reads as a password assignment).
- **Fix:** Renamed the variable to `PW_ASSIGN` and wrote the pattern with a bracketed first character (`[p]assword`), so the script has no self-exemption.
- **Files modified:** `tools/hygiene.sh`
- **Commit:** `7eaeba1`

**2. [Rule 1 - Bug] Duplicate stat key in the gate summary**
- **Found during:** Task 1 first gate run (`files=396 files=396`)
- **Issue:** The plan's success line `mode=tree files=<n>` reuses the key printed by the integrity step.
- **Fix:** The line is `-- hygiene pass mode=tree hygiene-files=<n>`; the acceptance grep for `-- hygiene pass mode=tree` still matches.
- **Files modified:** `tools/hygiene.sh`
- **Commit:** `7eaeba1`

**3. [Rule 2 - Missing critical] Anchored allow patterns**
- **Found during:** Task 1 design
- **Issue:** The RESEARCH `EMAIL_OK` is unanchored, so a look-alike address (an example domain followed by another host) would be allowed, and `HOME_OK` as a plain substring would allow a hosted-runner look-alike name.
- **Fix:** The allow patterns are applied to `grep -o` output and anchored (the hosted-runner home exactly, with domain suffixes ending the match). `noreply@anthropic.com` is allowed in tree mode as the plan lists it in the set, and in messages.
- **Files modified:** `tools/hygiene.sh`
- **Commit:** `7eaeba1`

**4. [Plan acceptance not met - recorded, not hidden] cmake line budget is exceeded**
- **Found during:** Task 2 step 4
- **Measured:** `cat tools/check/*.cmake tools/check/kinds/*.cmake | wc -l` prints **2154**, above the plan's 1500 limit (GB-CI-004). It is below the roughly 4,000 lines the lesson warns about, and this plan added no cmake code. Per file: manifests.cmake 384, records.cmake 333, coverage.cmake 277, manifest-lib.cmake 216, lib.cmake 152, kinds/claim.cmake 152, files.cmake 137, kinds/cp1610-row.cmake 120, kinds/source.cmake 93, c17-consistency.cmake 74, integrity.cmake 72, and the remaining kinds under 70 each.
- **History of the figure:** 636 after plan 01-01 (limit then 700). The growth came from plans 01-04 to 01-09: the manifest checker with its library (600 lines), the 1024-pattern coverage check, the C17 consistency check and the per-kind hooks. Why the budget is 1500 and not 700 (I2): the generic walker plus per-kind hooks for every record kind, the 1024-pattern coverage check and the C17 consistency check, which stays well below the 4,000 lines of the sibling gate.
- **Action:** Not shrunk. Condensing other plans' checkers to hit a number would risk the controls and would be line-golfing; the budget is a sprawl tripwire, not a gate check. Recommend the owner or orchestrator either revise the budget to a measured figure (about 2200 with headroom) or schedule a consolidation of `manifests.cmake` and `manifest-lib.cmake` in a later plan. Logged to the broken-windows ledger as a deviation.

**5. [Fallback not needed] Gate latency**
- Median 8.08 s under a loaded host, under the 10 s budget; the parallel/batch fallback was not applied (see Performance).

## Auth gates

None.

## Known Stubs

None.

## Threat Flags

None. The only new surface is the hygiene scan itself; it prints no matched text.

## Notes for plan 01-12

- `sh tools/hygiene.sh --history` currently reports home paths in 53 of 57 commits; the history rewrite (RESEARCH "History scrub") must bring this to zero, after which `--history` is the publication gate.
- `.planning/config.json` has a pre-existing uncommitted modification (an `_auto_chain_active` flag) and was deliberately left unstaged.

## Self-Check: PASSED

- Files present: `tools/hygiene.sh`, `tools/check/steps/hygiene.sh`, `data/expected/gate-steps/hygiene.ids`, `data/expected/controls/hygiene.ids`, the four control directories, `README.md`, `LICENSE`.
- Commits `7eaeba1` and `38481c4` are ancestors of HEAD.
