---
phase: 01-evidence-profile-and-rights-foundations
fixed_at: 2026-11-09T19:30:00Z
review_path: .planning/phases/01-evidence-profile-and-rights-foundations/01-REVIEW.md
iteration: 1
findings_in_scope: 7
fixed: 7
skipped: 0
status: all_fixed
---

# Phase 1: Code Review Fix Report

**Fixed at:** 2026-11-09
**Source review:** `.planning/phases/01-evidence-profile-and-rights-foundations/01-REVIEW.md`
**Iteration:** 1

**Summary:**
- Findings in scope: 7 (CR-01, WR-01 to WR-06; Info items IN-01 to IN-05 were out of scope)
- Fixed: 7
- Skipped: 0
- Negative controls: 84 before, 112 after (28 added, all built at run time with MAKE verbs or tracked as text-only overlays, all registered in `data/expected/controls/review-*.ids`)
- Gate steps: 16 before, 17 after (new step `hygiene-history`)
- Final gate line: `gate: PASS steps=17/17 controls=112/112 ...` under both `sh tools/gate.sh` and `dash tools/gate.sh`
- Verification ran in the main checkout (no worktree), with the gate commands above plus `sh tools/hygiene.sh --tree` and `--history`.

Every logic-shaped fix below was mutation-checked or reproduced before commit (see each entry). Status values marked "requires human verification" are the ones that change what the gate accepts rather than just adding a check.

## Fixed Issues

### CR-01: A NUL byte in a text file hides it from hygiene, denylist, R1/R2 and the ASCII rule

**Files modified:** `tools/check/binscan.sh` (new), `tools/check/binary-ext.txt` (new), `tools/check/steps/integrity.sh`, `tools/check/integrity.cmake`, `tools/hygiene.sh`, `tools/check/steps/denylist.sh`, `tools/check/steps/text.sh`, `tools/check/steps/glossary.sh`, `tools/check/steps/sources.sh`, `tools/check/run-controls.sh`, `SKELETON.md`
**Commit:** 95c627b
**Status:** fixed: requires human verification (logic of what counts as a text file)
**Applied fix:** `integrity` now fails any tracked file that is not binary by extension and contains a NUL byte (`integrity-nul-byte: <file>`), found by comparing the stream with and without NULs (`tr -d` plus `cmp`, no grep heuristics). Every content scan (hygiene tree and history, denylist, text, glossary, sources) now reads with `grep -a` (`git grep -a` in history), and the second grep in the two-grep pipelines also uses `-a` so a NUL on the matched line cannot turn into a "Binary file matches" message. Binary-by-extension handling stays with the files step; the extension list is one shared file.
**Controls added (5):** `integrity-nul-byte`, `hygiene-nul-home-path`, `denylist-nul-hit`, `text-nul-untagged-claim-word`, `glossary-nul-bare-clock`. New MAKE verb `nul` (printf at run time).
**Scanner-with-NUL controls were reachable:** each scanner step runs independently of `integrity`, so a file with a NUL plus a leak still reaches it. The four scanner controls were mutation-tested: restoring `-I` made each of them exit zero (`control-exit-zero`), so they prove the `-a` change and are not redundant with the NUL ban.
**Residual risk:** `grep -a` is supported by GNU grep, BSD grep and ugrep but is not POSIX. This machine's grep is ugrep; GNU grep was not available to re-check. A committed effective-yes binary that has no extension from the binary list now fails `integrity-nul-byte` (documented in `docs/RIGHTS.md`).

### WR-01: ci-parity cannot detect a CI job that no longer runs the gate, or a second workflow

**Files modified:** `tools/check/steps/ci-parity.sh`
**Commit:** 8556280
**Status:** fixed: requires human verification (new YAML-by-line rules)
**Applied fix:** Rewrote the step as a spacing-tolerant awk pass over every `.github/workflows/*` path in `FILELIST` (not only `gate.yml`). It rejects `continue-on-error`, `if:`, `pull_request_target`, flow-style `{` mappings outside `${{ }}`, any `run:` other than the gate step or `cmake --version` (also with `run :` spacing), any `uses:` not pinned to a 40-hex SHA, and any `permissions` other than a top-level (or job-level) block holding only `contents: read`. `gate.yml` must still contain the gate step.
**Controls added (6):** `ci-parity-continue-on-error`, `ci-parity-conditional`, `ci-parity-write-permissions`, `ci-parity-run-spacing`, `ci-parity-extra-workflow-target`, `ci-parity-extra-workflow-unpinned`. Each was run by hand and fails only with its intended token.
**Residual risk:** line-based, not a YAML parser. Anchors, multi-document files and block-scalar bodies are not interpreted; a key-looking line inside a `run: |` body is reported (fail closed). A second workflow is allowed if it passes the same rules; the review suggested rejecting extra workflows outright, which remains a one-line policy change if wanted.

### WR-02: The rights gate is extension-based; archives and extensionless binaries pass

**Files modified:** `tools/check/binscan.sh`, `tools/check/binary-ext.txt`, `tools/check/text-ext.txt` (new), `tools/check/steps/files.sh`, `tools/check/files.cmake`, `tools/hygiene.sh`, `tools/check/run-controls.sh`, `docs/RIGHTS.md`, `SKELETON.md`
**Commits:** c44aa48 (content-based detection), cdf8e43 (batching and text-extension skip, needed to keep the gate fast)
**Status:** fixed: requires human verification (changes what the rights gate treats as binary)
**Applied fix:** `files-unlisted-binary` now fires for a tracked file that is binary by name (list extended with `zip 7z gz tar bz2 xz dat bmp webp svg ico a26`) or by content (a NUL byte, or more than 30 percent bytes outside tab, LF, CR and 0x20-0x7E), unless it is a committed output of an effective-yes manifest. Files with a text extension (`tools/check/text-ext.txt`) are not content-classified in `files` because `integrity` already bans NUL in them. The extension list is shared by integrity, files and `hygiene --history`.
**Controls added (3):** `files-unlisted-binary-content` (extensionless, 16 KiB with NULs), `files-unlisted-binary-archive` (`.zip`), `files-unlisted-binary-nonul` (extensionless, high bytes, no NUL). New MAKE verbs `binary` and `highbytes` generate the bytes with `awk` and `tr`.
**Residual risk:** a NUL-free binary that carries a text extension is not detected (stated in `docs/RIGHTS.md`). The 30 percent threshold is a heuristic.

### WR-03: hygiene password and token rules miss common secret shapes

**Files modified:** `tools/hygiene.sh`, `tools/check/hygiene-baseline.txt` (new), `tools/check/run-controls.sh`, `01-REVIEW.md` (two documentation lines reworded), `SKELETON.md`
**Commit:** 8907ff7
**Status:** fixed
**Applied fix:** New rule set `PW_NEW`/`TOKEN_NEW`: closing-quote key (JSON `"password": "..."`), `passwd`, quoted `secret`/`api_key`/`token` assignments with a 16-plus-character value, `aws_secret_access_key`, `xox[baprs]-`, `sk-ant-`, `AIza`, `glpat-`, `ASIA`. Each rule puts a bracket expression right after its literal prefix, so `tools/hygiene.sh` and the docs never match themselves (no file exclusions). Held against the tree and against every commit after the history baseline (see WR-04).
**Controls added (5):** `hygiene-credential-json-password`, `-api-key`, `-slack`, `-anthropic`, `-google`. New MAKE verb `credential <path> <kind>` assembles each string from split pieces at run time.
**Published history check:** the new shapes were run over all published commits and found nothing, so no real credential is in history.
**Residual risk:** `secret`/`api_key`/`token` assignments need a quote around the value; an unquoted `token = <long value>` is not caught (kept low to avoid prose false positives).

### WR-04: R4 "no scratch path" is documented but not enforced

**Files modified:** `tools/hygiene.sh`, `tools/check/hygiene-baseline.txt`, `tools/check/run-controls.sh`, `01-REVIEW.md` and `01-RESEARCH.md` (documented patterns reworded or bracketed), `SKELETON.md`
**Commit:** 1b313b1
**Status:** fixed: requires human verification (history baseline mechanism)
**Applied fix:** `HOME_NEW` adds the macOS private temp and per-user temp-folder forms, the Linux scratch form, the root-home form, the dash-encoded scratch form and tilde-relative paths (3 or more name characters; the tool config directory under the tilde stays allowed through `HOME_OK`, because the planning files cite it). No file is excluded.
**History semantics (hard stop honoured):** published history holds one documented pattern of the old form in `01-RESEARCH.md`, and published history cannot be rewritten. Rather than make the new rules tree-only, `--history` reads `tools/check/hygiene-baseline.txt`: commits at or before the baseline keep the rules they were made under (still passing), every later commit is held to the full rule set, and a repository without a usable baseline is fully strict. The baseline is the commit before this fix (8907ff7), chosen because trees up to that commit still contain the old wording. It must never be moved forward to hide a finding. A control also runs `--history` in a fresh throwaway repository, where everything is strict.
**Controls added (5):** `hygiene-scratch-private-tmp`, `-var-folders`, `-tmp-claude`, `-root-home`, `-tilde`. New MAKE verb `scratchpath <path> <kind>`.
**Residual risk:** the unpublished local commits before the baseline were not held to the new rules (they contain no leak; checked by hand). A tilde path to something other than the tool config directory is now a finding, which may surprise a future document.

### WR-05: The weakest-field rule is vacuous for an established claim with no dependencies

**Files modified:** `tools/check/schema/claim.txt`
**Commit:** cf016fa
**Status:** fixed
**Applied fix:** `when standing=established nonblank depends_on`, so an established claim must name at least one profile field and the weakest-field hook can no longer be skipped by an empty array. Provisional claims may still have an empty list.
**Control added (1):** `claims-established-without-deps` (expects `FAIL claim-blank: CL-0001/depends_on`).
**Residual risk:** none known.

### WR-06: hygiene swallows grep errors; --history has no negative control

**Files modified:** `tools/hygiene.sh`, `tools/check/steps/hygiene-history.sh` (new), `data/expected/gate-steps/review-fix.ids` (new), `SKELETON.md`
**Commit:** 6e3b169
**Status:** fixed: requires human verification (new gate step and error semantics)
**Applied fix:** `file_leaks` and the batch scan treat grep status 2 or more as `hygiene-grep-error` (xargs runs grep through a wrapper that maps errors to 255, so status 1 per batch stays normal); `--history` treats a nonzero `git grep` status, or an `error:`/`fatal:` line on stderr, as `hygiene-history-grep-error` (reproduced: `git grep` exits 1 for a deleted loose object and only prints to stderr, which is why status alone was not enough); stderr is printed, not discarded; `git log` failures are findings; the `cand` scans now run in the main shell so a finding is not lost in a pipeline subshell.
**Runner expressiveness:** no new runner verb was needed. A new gate step `hygiene-history` builds a throwaway one-commit repository at run time (git dir in a temp directory, `ROOT` as work tree, clean git config) and runs `hygiene.sh --history` in it. The controls use ordinary MAKE verbs; the unreadable-object control uses a MAKE-written marker file (documented in `SKELETON.md`).
**Controls added (3):** `hygiene-history-home-path`, `hygiene-history-secret`, `hygiene-history-unreadable-object`.
**Residual risk:** there is no negative control for a grep error in `--tree` mode (the per-file readability check fires first and the batch error path has no cheap reproducer). The gate step adds about 3 to 4 seconds on a heavily loaded host.

## Skipped Issues

None. The Info findings were out of scope. Cheap ones, if wanted later: IN-01 (match the whole `SRC-` token and report non-ids), IN-02 (a `*-no-files` failure in `glossary` and `sources`), IN-04 (count a step as passed only if its pass line is printed). IN-03 and IN-05 are larger.

## Notes

- Gate time rose with the added controls and checks. Measured under the same heavy host load (load average about 15): the pre-fix tree took about 20 s and the fixed tree about 37 to 48 s. The content scans were batched to keep the increase down; the remaining cost is mostly the 28 extra controls and the history step.
- During WR-03 the pre-existing local modification to `.planning/config.json` was staged into the commit by mistake. It was undone with a soft reset of that unpublished commit and a clean re-commit before any other work; no commit in the final history touches that file, and it remains an unstaged local modification.

---

_Fixed: 2026-11-09_
_Fixer: Claude (gsd-code-fixer)_
_Iteration: 1_
