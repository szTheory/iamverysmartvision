---
phase: 01-evidence-profile-and-rights-foundations
reviewed: 2026-11-09T00:00:00Z
depth: standard
files_reviewed: 45
files_reviewed_list:
  - .github/workflows/gate.yml
  - tools/check/c17-consistency.cmake
  - tools/check/coverage.cmake
  - tools/check/denylist-cite.txt
  - tools/check/denylist.txt
  - tools/check/files.cmake
  - tools/check/integrity.cmake
  - tools/check/kinds/claim.cmake
  - tools/check/kinds/cp1610-row.cmake
  - tools/check/kinds/discrepancy.cmake
  - tools/check/kinds/evidence-tiers.cmake
  - tools/check/kinds/profile.cmake
  - tools/check/kinds/second-reader.cmake
  - tools/check/kinds/source.cmake
  - tools/check/lib.cmake
  - tools/check/manifest-lib.cmake
  - tools/check/manifests.cmake
  - tools/check/records.cmake
  - tools/check/run-controls.sh
  - tools/check/schema/claim.txt
  - tools/check/schema/cp1610-row.txt
  - tools/check/schema/discrepancy.txt
  - tools/check/schema/evidence-tiers.txt
  - tools/check/schema/profile.txt
  - tools/check/schema/second-reader.txt
  - tools/check/schema/source.txt
  - tools/check/schema/tags.txt
  - tools/check/steps/ci-parity.sh
  - tools/check/steps/claims.sh
  - tools/check/steps/cp1610-reader.sh
  - tools/check/steps/cp1610.sh
  - tools/check/steps/denylist.sh
  - tools/check/steps/discrepancies.sh
  - tools/check/steps/files.sh
  - tools/check/steps/glossary.sh
  - tools/check/steps/hygiene.sh
  - tools/check/steps/integrity.sh
  - tools/check/steps/manifests.sh
  - tools/check/steps/profile.sh
  - tools/check/steps/runner-selftest.sh
  - tools/check/steps/sources.sh
  - tools/check/steps/text.sh
  - tools/check/steps/vocab.sh
  - tools/gate.sh
  - tools/hygiene.sh
findings:
  critical: 1
  warning: 6
  info: 5
  total: 12
status: issues_found
---

# Phase 1: Code Review Report

**Reviewed:** 2026-11-09
**Depth:** standard
**Files Reviewed:** 45
**Status:** issues_found

## Summary

All 45 files were read in full and checked against SKELETON.md sections 1-8. The gate itself is
well built: every step runs to completion, exit codes are captured correctly, the control runner
rejects blank EXPECT lines, vacuous controls, uninventoried controls and wrong tokens, and
`sh tools/gate.sh` passes locally (steps 16/16, controls 84/84). The `actions/checkout` pin was
confirmed against the upstream repo (`3d3c42e5...` is tag `v7.0.1`).

The weaknesses are all fail-open paths where a scanner never examines its input. Each finding below
marked "reproduced" was demonstrated in a throwaway copy of the repo under the session scratchpad
(not the real tree); the real tree was not modified.

The one blocker is that a single NUL byte makes a tracked text file invisible to every grep-based
scanner (hygiene, denylist, R1/R2 tripwires) and to the integrity ASCII rule, so a secret, home path,
denylisted name or claim word passes the gate. UTF-16 files hit this accidentally.

Not verified on GNU grep or dash: this machine's `grep` is ugrep. The NUL behaviour is the documented
meaning of `grep -I` and of GNU "binary file matches", but the reproduction ran under ugrep.

## Critical Issues

### CR-01: A NUL byte in a text file hides it from hygiene, denylist, R1/R2 and the ASCII rule (gate passes)

**File:** `tools/hygiene.sh:43,71` (`grep -I`), `tools/check/steps/denylist.sh:51,55` (`grep -I`),
`tools/check/steps/text.sh:34`, `tools/check/steps/glossary.sh:49` (no `-a`, binary files print
"Binary file matches" with no line number), `tools/check/integrity.cmake:47-60`
**Issue:** `grep -I` treats any file containing a NUL byte as binary and skips it. The text and
glossary scans pipe `grep -n` into `sed -n 's/^\([0-9]*\):.*/\1/p'`, so a binary-match message yields
no finding. `integrity.cmake` only compares `string(LENGTH)` with `file(SIZE)` and tests for CR, so NUL
passes; its `[^\t\n -~]` non-ASCII test does not see bytes after a NUL (the regex engine stops at NUL).
Nothing else classifies "this tracked file is binary".

Reproduced (each file added untracked, then `sh tools/gate.sh`; every run ended `gate: PASS steps=16/16`):
- `docs/n.md` containing a 36-char `ghp_` token plus a NUL: hygiene did not flag it.
- `docs/n2.md` containing a `/Users/<name>/` path plus a NUL: not flagged.
- `docs/n3.md` containing a denylisted author name plus a NUL: not flagged.
- `docs/x.md` = "This emulator supports every game and the clock is 3.58." plus a NUL: both
  `text-claim-untagged` and `glossary-bare-clock` suppressed (the same line without the NUL fails both).
- `data/x.txt` = `ok\n<NUL>\xC3\xA9\n`: `integrity-non-ascii` suppressed.
- `hygiene --history` uses `git grep -I`, so a NUL-containing blob in any commit is also skipped.

Failure scenario: an editor saves a doc as UTF-16 (NUL between every character). It contains a home
path or a token. Every scanner skips it and CI reports green, defeating FND-05, R1-R4 and AUTONOMY.md
section 6 ("PII, credentials, tokens or private paths") for a public repo.
**Fix:** Add an explicit binary-content check and use `-a` for the content scans so nothing is skipped:
```sh
# in integrity step (or hygiene.sh --tree), for every in-scope non-listed-binary file "$f":
if [ "$(tr -d '\000' < "$f" | wc -c)" -ne "$(wc -c < "$f")" ]; then
  printf -- '-- FAIL integrity-nul-byte: %s\n' "$f"; rc=1
fi
```
then replace `grep -I` with `grep -a` in hygiene.sh / denylist.sh (and `grep -a` in text.sh and
glossary.sh) once the NUL check guarantees text. Add MAKE verb `nul` plus controls for hygiene, denylist,
text and integrity so the hole stays closed. In `--history`, flag any blob that `git grep -I` would skip
(`git grep -a`).

## Warnings

### WR-01: ci-parity cannot detect a CI job that no longer runs the gate, or a second workflow

**File:** `tools/check/steps/ci-parity.sh:15,24-49`
**Issue:** The step reads only `.github/workflows/gate.yml` line by line and checks for the literal
line `run: sh tools/gate.sh`, SHA-pinned `uses:`, and a `contents: read` line anywhere. It does not
look at `if:`, `continue-on-error:`, `on:` triggers, `permissions:` structure, or any other file in
`.github/workflows/`.

Reproduced (gate stayed `PASS steps=16/16`):
- Adding `continue-on-error: true` and `if: ${{ false }}` under the gate step: CI would run nothing
  or ignore its result.
- Adding `.github/workflows/evil.yml` with `on: pull_request_target`, `permissions: write-all` and
  `uses: actions/checkout@main`: no finding (SPDX line present, `integrity` is satisfied).
- `run : curl ...` (space before the colon) matches neither `run:*` nor the allowed lines.

**Fix:** In ci-parity: (1) enumerate `.github/workflows/*` from FILELIST and fail
(`ci-parity-extra-workflow`) on anything other than `gate.yml`; (2) fail on lines matching
`continue-on-error`, `^[[:space:]]*if:`, `pull_request_target`, `write-all`, `: write` ; (3) require the
`contents: read` line to be the child of a top-level `permissions:` key (check the line directly after
`^permissions:`). Add controls `ci-parity-continue-on-error` and `ci-parity-extra-workflow`.

### WR-02: The rights gate is extension-based; archives and extensionless binaries pass

**File:** `tools/check/files.cmake:26,93-102`, `tools/check/integrity.cmake:18,51-55`
**Issue:** `files-unlisted-binary` triggers only on `int bin rom itv cfg cc3 pdf png jpg jpeg gif`.
`.zip`, `.7z`, `.gz`, `.tar`, `.dat`, `.a26`, `.bmp`, `.webp`, an extensionless name, or a trailing dot
(`exec.bin.` gives `LAST_EXT` ".") are not flagged. The only incidental backstop is `integrity-crlf`,
which fires merely because random bytes usually contain 0x0D.

Reproduced: 8 KiB of random bytes with 0x0D removed, saved as `docs/exec.zip` and as `exec`, passed the
whole gate (`gate: PASS`). With 0x0D present the same file failed only with the misleading
`integrity-crlf`. Hygiene and denylist also skip such files (`grep -I`), so nothing else sees them.

Failure scenario: an EXEC/GROM image or manual scan is committed inside a `.zip` or without an
extension; the rights gate (the project's hard stop) passes.
**Fix:** Decide "binary" by content, not name: reuse the NUL/high-bit check from CR-01 and fail
`files-unlisted-binary` for any tracked file that is not valid text and is not a committed output of an
effective-yes manifest. Keep the extension list as an additional fast path and extend it
(`zip 7z gz tar bz2 xz dat bmp webp svg ico`).

### WR-03: hygiene password and token rules miss common secret shapes

**File:** `tools/hygiene.sh:21,24`
**Issue:** `PW_ASSIGN` is `[p]assword[ ]*[=:][ ]*...`, so JSON/YAML-quoted keys (`"password": "..."`)
never match because a `"` sits between the word and the colon. Only `password` is covered; `secret`,
`api_key`, `token` and `passwd` are not. `TOKEN` covers `ghp_ gho_ ghu_ ghs_ ghr_`, `github_pat_`,
`AKIA` and PEM private keys only.

Reproduced with `sh tools/hygiene.sh --tree` in the copy: a JSON-quoted password key with a plain 19-character value passed;
`DB_PASSWORD="..."` and `password: ...` were caught. Also uncaught: `xoxb-...`, `sk-ant-api03-...`,
an `api_key` assignment with a quoted 22-character value, `aws_secret_access_key = ...`.
**Fix:** Allow an optional closing quote before the separator and widen the keyword set:
```sh
PW_ASSIGN='[p]ass(word|wd)["'"'"']?[ ]*[=:][ ]*["'"'"']?[A-Za-z0-9][A-Za-z0-9._~!@#%^&*+/-]{7,}|(secret|api[_-]?key|token)["'"'"']?[ ]*[=:][ ]*["'"'"'][A-Za-z0-9._~+/-]{16,}'
TOKEN="$TOKEN|xox[baprs]-[A-Za-z0-9-]{10,}|sk-ant-[A-Za-z0-9_-]{20,}|AIza[0-9A-Za-z_-]{35}|glpat-[A-Za-z0-9_-]{20}|ASIA[0-9A-Z]{16}"
```
and add a `hygiene-password-json` control next to `hygiene-password`.

### WR-04: R4 "no scratch path" is documented but not enforced

**File:** `tools/hygiene.sh:3,18` (SKELETON.md section 7, R4)
**Issue:** The header says "no home or scratch path" and R4 requires no "absolute home or scratch path
in any tracked file". `HOME_PATH` only matches the macOS, Linux and Windows home-directory forms and the dash-encoded scratch-directory form.
Reproduced: a macOS private scratch path, a Linux scratch path under the temp directory, a macOS per-user
temp-folder path, a tilde-relative project path and a root-home ssh key path all pass `hygiene --tree`. The
session scratchpad used by this very workflow lives under the macOS private temp directory, so a SUMMARY.md that cites it would ship.
**Fix:** Add `/private/tmp/[A-Za-z0-9._-]+|/private/var/[A-Za-z0-9._-]+|/var/folders/[A-Za-z0-9]|/tmp/claude-[A-Za-z0-9._-]+|/root/[A-Za-z0-9.]|~/[A-Za-z0-9._-]` (and `-private-tmp-`) to `HOME_PATH`, mirror the change in `--history`, and add a `scratchpath` MAKE verb and control.

### WR-05: The weakest-field rule is vacuous for an established claim with no dependencies

**File:** `tools/check/kinds/claim.cmake:115-121,142`, `tools/check/schema/claim.txt:16`
**Issue:** The schema requires `depends_on` to be an ARRAY but has no `nonblank depends_on`, and the
hook returns early on an empty array. `claim-overstated` can then never fire, so "established" is
unconstrained. Reproduced: setting CL-0003 (tier4/inference fields) to `"standing": "established"` and
`"depends_on": []` gave `-- claims pass claims=5 provisional=3` (exit 0).
**Fix:** In `claim.txt` add `nonblank depends_on` (or, in the hook, `gate_fail(claim-established-no-deps)`
when `standing=established` and `depends_on` is empty), with a control `claims-established-without-deps`.

### WR-06: hygiene masks grep and git-grep failures, so unreadable input reads as clean

**File:** `tools/hygiene.sh:43,71,102,105`
**Issue:** `file_leaks` and `cand` send grep stderr to `/dev/null` and treat exit 2 (error) the same as
exit 1 (no match); `cand` ends with `|| true`. In `--history`, `git grep ... 2>/dev/null | ... | grep -q .`
does the same, and `--history` has no negative control at all. Reproduced: committed a file with a home
path (history reported `hygiene-history-home-path`), then deleted that blob's loose object from
`.git/objects`; `sh tools/hygiene.sh --history` printed `-- hygiene pass mode=history commits=64`
(git printed `unable to read`, which was discarded).
**Fix:** Run `git grep` into a temp file, capture its exit status (`0` match, `1` none, anything else is
`found unreadable "$s"`), and stop discarding stderr. In `cand`, treat grep exit status `>= 2` as
`found unreadable`. Add a self-test that `--history` flags a seeded leak in a throwaway repo.

## Info

### IN-01: SRC- reference scan truncates or skips malformed ids (R5)

**File:** `tools/check/steps/sources.sh:36`
**Issue:** `grep -o -E 'SRC-[a-z0-9-]+'` stops at the first character outside the class. Reproduced:
`SRC-bsr-2609_nope` is truncated to the registered `SRC-bsr-2609` and passes; `SRC-Nope-Doc` and
`SRC-Gi-1980` (uppercase after the dash) produce no match at all and pass. A typo'd citation is
therefore not reported.
**Fix:** Match the full token (`SRC-[A-Za-z0-9_.-]*[A-Za-z0-9]`) and fail on any token that is not an id in `sources.ids`.

### IN-02: Scans pass with zero files examined

**File:** `tools/check/steps/glossary.sh:42-57`, `tools/check/steps/sources.sh:29-37`
**Issue:** With a FILELIST that has no README or docs entries, glossary prints `-- glossary pass
doc-files=0` and sources scans nothing (reproduced for glossary: exit 0). `text.sh` has
`text-no-files`, so the whole gate still fails today, but each step claims its check without examining
input.
**Fix:** `[ "$docfiles" -gt 0 ] || bad glossary-no-files ...`, and the equivalent in sources.sh.

### IN-03: Manifest output paths are not anchored to the manifest's own fixture directory

**File:** `tools/check/manifests.cmake:80-86,111`, `tools/check/files.cmake:56-61`
**Issue:** `path_ok` checks charset and no `..` only. An effective-yes committed manifest in
`fixtures/FX-a/` may list an output such as `docs/other.pdf`, which then lands in `binary_ok` and waives
`files-unlisted-binary` for that path repo-wide. Requires a reviewed manifest, so low risk.
**Fix:** In `check_entries` for `outputs`, `notices` and `sources`, require `^fixtures/<dirname>/` (the manifest's own directory).

### IN-04: gate.sh counts a step as passed on exit 0 without its pass line

**File:** `tools/gate.sh:56-60`
**Issue:** SKELETON section 2 requires every step to print `-- <name> pass`. The control runner enforces
that on the good tree, but the real-tree loop trusts the exit status only; a step that exits 0 early
(for example a `cmake -P` path that returns before `gate_finish`) would be counted in `steps=16/16`.
**Fix:** Count a step as passed only if the exit status is 0 and `grep -q -F "-- $name pass" "$GATE_TMP/step.out"`.

### IN-05: Control-runner coverage is thinner than its contract

**File:** `tools/check/steps/runner-selftest.sh:55-67`, `tools/check/run-controls.sh:121-141`
**Issue:** The selftest proves four runner tokens. It does not exercise `control-forbidden-token`,
`control-inventory-orphan`, `control-unknown-step`, `control-make-verb` or `control-good-failed` on
purpose, so a regression in those branches would go unnoticed. EXPECT matching is a bare substring, and
about 20 controls expect only `FAIL <token>:` with no id or field (for example `FAIL source-format:`,
`FAIL profile-unknown-ref:`, `FAIL coverage-unclaimed:`), so they cannot show which record or field
failed. A silently failing MAKE verb (misspelled path) is caught only indirectly via `control-exit-zero`.
**Fix:** Extend the selftest with one synthetic control per remaining runner token; tighten bare-token
EXPECT lines to `<token>: <id>[/<field>]`; have `apply_make` fail when `rm/mv/awk` targets do not exist.

---

_Reviewed: 2026-11-09_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
