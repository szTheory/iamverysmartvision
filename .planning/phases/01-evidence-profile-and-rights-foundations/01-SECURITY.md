---
phase: 01-evidence-profile-and-rights-foundations
asvs_level: 1
block_on: high
status: secured
threats_total: 40
threats_closed: 40
threats_open: 0
audited: 2026-11-09
---

# Phase 1 Security Verification

Phase 1 (Evidence, Profile and Rights Foundations) delivers tooling, records and docs only: no emulator code, no network service, no runtime data handling. The attack surface is the public repository itself (leaks, rights-bearing bytes, history), the CI workflow, and the checker scripts that read tracked text.

Method: each declared mitigation was looked for in implemented code or config, not in plan or summary prose. `sh tools/gate.sh` was run on the audited tree and ended `gate: PASS steps=17/17 controls=112/112`. `sh tools/hygiene.sh --history` passed over 74 commits. Independent scans (below) were run over every reachable blob and commit, not only through the repository's own scripts.

## Independent checks on the public repository

| Check | Result |
|-------|--------|
| `git ls-files refs` | empty (0 tracked paths) |
| `refs/` ignored | `.gitignore:2` rule `/refs/`; also `.git/info/exclude`; `git check-ignore` confirms a sample path under it |
| Tracked rights-bearing bytes (PDF, images, ROM-like, archives) | none; 532 tracked paths, every extension from `tools/check/binary-ext.txt` absent outside `tools/check/controls` |
| History: any file ever added with a binary or `refs/` name | none (`git log --all --diff-filter=A`) |
| History: any blob containing a NUL byte | 0 of 585 blobs (stream compared with and without NULs); largest blob 73 KB of text |
| Secret shapes (GitHub token, fine-grained PAT, AWS key id, private-key header, Slack, `sk-`, Google, GitLab) in HEAD and all history | no value; the only hit is the literal prefix `glpat-` inside the regex source (`tools/hygiene.sh:24`) and two review documents |
| Quoted credential assignments (password, secret, api_key, token with 16+ chars) in HEAD | none |
| Home, scratch and temp paths, tree | none; matches are regex source in `tools/hygiene.sh:40` and `01-REVIEW.md:191` only |
| Home, scratch and temp paths, history | no username or private path; only the documented pre-baseline fragments (see Residual Risks) |
| Identities | one author and committer identity in every commit, a `users.noreply.github.com` address; third-party noreply trailers (`noreply@anthropic.com`, `noreply@github.com`) are allowlisted |
| Remote state | `origin/main` is an ancestor of local `main` (fast-forward only, no sign of a rewrite after publication); no tags, no stashes |
| Workflows | exactly one: `.github/workflows/gate.yml` |

## Workflow review (`.github/workflows/gate.yml`)

- Trigger: `push` to main and `pull_request` (not `pull_request_target`): fork PRs get no secrets.
- `permissions: contents: read` at top level (line 7-8); no job-level override.
- The only `uses:` is `actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1`; the SHA resolves to the upstream commit "prep v7.0.1 release" (checked against the GitHub API).
- `persist-credentials: false` (line 19); no secrets referenced; steps are `cmake --version` and `sh tools/gate.sh` only.
- Machine-enforced on every gate run by `tools/check/steps/ci-parity.sh` (awk rules for `permissions`, `uses` SHA pinning, extra `run:`, `if:`, `continue-on-error`, `pull_request_target`, flow-style maps); 8 negative controls `tools/check/controls/ci-parity-*`.

## Threat verification

Severity is from the plan `threat_model` tables. Dispositions are `mitigate` unless stated. "Control" names a negative control directory under `tools/check/controls/` that the gate runs and requires to fail with the named token.

| Threat ID | Sev | Disposition | Status | Evidence |
|-----------|-----|-------------|--------|----------|
| T-01-01 run-controls MAKE verbs | medium | mitigate | CLOSED | `tools/check/run-controls.sh:23-80`: `apply_make` uses a fixed `case` verb whitelist, unknown verb fails `control-make-verb` (line 77), `/*` and `*..*` paths rejected `control-make-path` (lines 35-36, 43), `set -f` before word-splitting, writes only under `$_t` inside `mktemp -d` (line 15) with trap cleanup (16). `git grep -w eval -- tools` returns nothing |
| T-01-02 path handling | medium | mitigate | CLOSED | `tools/check/integrity.cmake:32-33`: `^[A-Za-z0-9._/-]+$` else `integrity-path-charset`; lists flow through `FILELIST` files |
| T-01-03 workflow privileges | medium | mitigate | CLOSED | `.github/workflows/gate.yml:7-8,19`; enforced by `tools/check/steps/ci-parity.sh` (`ci-permissions`); controls `ci-parity-write-permissions` |
| T-01-04 checkout pin | medium | mitigate | CLOSED | `.github/workflows/gate.yml:17` 40-hex SHA; `ci-parity.sh` `ci-unpinned-action`; controls `ci-parity-unpinned-action`, `ci-parity-extra-workflow-unpinned` |
| T-01-05 control fixtures leak | high | mitigate | CLOSED | Bad strings are built at run time by split-piece `printf` in `run-controls.sh:50-76` (`homepath`, `secret`, `password`, `scratchpath`, `credential`, `email`); `git grep` of tracked tree and all history finds no real home path or token (table above) |
| T-01-06 refs/ cache tracked | high | mitigate | CLOSED | `.gitignore:2`; `.git/info/exclude`; `tools/check/files.cmake:104-106` `files-tracked-refs` (control `files-tracked-refs-file`); `git ls-files refs` empty |
| T-01-07 downloaded manual tampering | medium | mitigate | CLOSED | `tools/check/kinds/source.cmake:55-60` size and sha256 compared when a cached file exists (`source-digest-mismatch`); control `sources-digest-mismatch`; gate reports `refs-verified=1/4` |
| T-01-08 notes text paths | high | mitigate | CLOSED | `hygiene.sh` home and scratch rules (`tools/hygiene.sh:18,40`) run as a gate step; `--tree` and `--history` pass; no absolute path in `docs/` or `data/` |
| T-01-09 page renders tracked | high | mitigate | CLOSED | No image or PDF tracked or ever added; `refs/` ignored and untracked (`refs/pages` and the cached PDF exist locally, untracked); `files-unlisted-binary*` controls |
| T-01-10 row transcription integrity | medium | mitigate | CLOSED | `tools/check/coverage.cmake` printed/pattern and full 1024 decode (`patterns=1024 unclaimed=0 ambiguous=0` in gate line); blind second reader `data/cp1610/second-reader.json` (`agree=71 resolved=3`) |
| T-01-11 provenance of discrepancy text | medium | mitigate | CLOSED | `tools/check/steps/denylist.sh` with `tools/check/denylist.txt` (authors, licence headers, never waivable) and `denylist-cite.txt` (`[cite-only]`); gate `scanned=57 waived=0`; controls `denylist-hit`, `denylist-cite-unwaived`, `denylist-author-not-waivable` |
| T-01-12 discrepancy id drift | low | mitigate | CLOSED | `tools/check/schema/discrepancy.txt`, `kinds/discrepancy.cmake` (`discrepancy-order`, `-duplicate-id`), `data/expected/discrepancies.ids`; controls `discrepancies-*`, `profile-conflict-unknown-id` |
| T-01-13 overclaiming | medium | mitigate | CLOSED | `tools/check/steps/text.sh` claim-word tripwire (`text-claim-untagged`), claim refs; `tools/check/schema/claim.txt` weakest-field hook and WR-05 `when standing=established nonblank depends_on`; controls `text-untagged-claim-word`, `claims-overstated-standing`, `claims-established-without-deps` |
| T-01-14 text step input parsing | low | mitigate | CLOSED | `text.sh:21-27` reads only `FILELIST` paths matching a fixed anchored pattern; `grep -F` and `-x` for ids; `mktemp -d` with trap; no eval anywhere under `tools/` |
| T-01-15 tracked third-party bytes | high | mitigate | CLOSED | `tools/check/files.cmake:104-146` (`files-unlisted-binary` by extension and, since WR-02, by content via `tools/check/binscan.sh`; `files-unlisted-fixture-file`); `.gitignore:5-16` class patterns; controls `files-unlisted-binary`, `-archive`, `-content`, `-nonul`, `files-unlisted-fixture-file` |
| T-01-16 tracked env files | high | mitigate | CLOSED | `files.cmake:107-110` `files-env` (`.env` and `.env.*`); `.gitignore:19-20`; control `files-env-file`; no such file in tree or history |
| T-01-17 spoofed redistributable flag | medium | mitigate | CLOSED | `tools/check/manifest-lib.cmake:4-6,62-80,121,155` effective-yes needs licence evidence, non-unreviewed review, and every `derived_from` effective yes (least fixpoint, fail closed); controls `manifests-yes-without-evidence`, `-misspelled-key`, `-derived-from-unknown`; gate `effective-yes=0 effective-no=1` |
| T-01-18 licence contamination | high | mitigate | CLOSED | `tools/check/steps/denylist.sh` (gate step, `-a` scan after CR-01), `docs/provenance/CONSULTATIONS.md`, `.github/pull_request_template.md` checklist; limit (renamed expression evades scan) stated in `docs/provenance.md` |
| T-01-19 wrong reference document | medium | mitigate | CLOSED | Same evidence as T-01-07 (`source-digest-mismatch`, control `sources-digest-mismatch`) |
| T-01-20 denylist output disclosure | low | mitigate | CLOSED | `denylist.sh:73-78` prints `denylist-hit: <file>` and `denylist-cite-unwaived: <file>` only; matched text is never echoed |
| T-01-21 unit confusion | medium | mitigate | CLOSED | `tools/check/steps/glossary.sh` (`glossary-missing-term`, `glossary-bare-clock`, `glossary-unit-suffix`); controls of the same names; gate `terms=9 profile-keys=23` |
| T-01-22 decision drift | low | mitigate | CLOSED | `glossary.sh:62` `glossary-d2-unlinked`; control `glossary-d2-unlinked` |
| T-01-23 opcode table integrity | medium | mitigate | CLOSED | `tools/check/kinds/cp1610-row.cmake`, `coverage.cmake` (pattern shape line 79, 1024-pattern decode); `cp1610-*` controls; independence rule in `docs/cp1610/reference-notes.md` |
| T-01-24 gate latency (DoS) | low | mitigate | CLOSED | `coverage.cmake:12,22` `CAP 20` findings; rows read once; full gate completes (about 40 s on a loaded host, `.github/workflows/gate.yml` has `timeout-minutes: 10`) |
| T-01-25 second-reader integrity | medium | mitigate | CLOSED | `tools/check/kinds/second-reader.cmake`, `schema/second-reader.txt` (inventory parity, `verdict` enum, note required on `resolved`); gate `agree=71 resolved=3`; controls `second-reader-*` |
| T-01-26 process honesty | low | mitigate | CLOSED | `second-reader.cmake:6,15-40` resolved entries must explain; limits in the notes; controls `second-reader-*` |
| T-01-27 PII paths in tracked files | high | mitigate | CLOSED | `tools/hygiene.sh:18,40` (`hygiene-home-path`) in every gate run (step `hygiene`); controls `hygiene-home-path`, `hygiene-nul-home-path`, `hygiene-scratch-*`; independent `git grep` over tree finds none |
| T-01-28 secrets in tree and history | high | mitigate | CLOSED | `tools/hygiene.sh:21-37` token and password rule sets in `--tree` and `--history`; findings print rule and file, never the match; controls `hygiene-secret`, `hygiene-password`, `hygiene-credential-*`, `hygiene-history-secret`; independent scan of all history finds no secret value |
| T-01-29 commit identity | medium | mitigate | CLOSED | `hygiene.sh` `EMAIL`/`EMAIL_OK` (line 46-48) and `hygiene-identity` in `--history`; control `hygiene-foreign-email`; `git log --all` shows only the noreply identity |
| T-01-30 trademark | medium | mitigate | CLOSED | `README.md:4` name not cleared; `README.md:53-56` non-affiliation notice; no trademark in repo name beyond the owner's working name, which README discloses |
| T-01-31 PII paths in history | high | mitigate | CLOSED | `sh tools/hygiene.sh --history` passes over 74 commits; step `hygiene-history` and controls `hygiene-history-home-path`, `hygiene-history-secret`, `hygiene-history-unreadable-object`; independent scan finds no username or private path in any reachable blob or message |
| T-01-32 author/committer identity | medium | mitigate | CLOSED | Single noreply identity (`git log --all --format='%ae|%ce'`); `hygiene-identity` enforces it |
| T-01-33 history loss on rewrite | medium | mitigate | CLOSED | Rewrite completed before first push; backup bundle kept outside the repository and removed after publication was confirmed (01-12 SUMMARY); local `main` fast-forwards from `origin/main`, nothing lost |
| T-01-34 gh token privileges | medium | mitigate | CLOSED | No token value anywhere in tree or history (scan above); `gate.yml:7-8,19` read-only token, credentials not persisted; no secrets referenced |
| T-01-35 irreversible actions | high | mitigate | CLOSED | `.planning/AUTONOMY.md:43` lists force-push to main and rewriting published history as hard stops; `origin/main` is an ancestor of `main` and no tags exist (no evidence of a post-publication rewrite) |
| T-01-36 workflow integrity | medium | mitigate | CLOSED | `tools/check/steps/ci-parity.sh` runs every gate run (`ci-uses=1 ci-workflows=1`); controls `ci-parity-missing-gate`, `-unpinned-action`, `-write-permissions`, `-continue-on-error`, `-conditional`, `-run-spacing`, `-extra-workflow-target`, `-extra-workflow-unpinned` |
| T-01-37 vacuous control evidence | medium | mitigate | CLOSED | `tools/check/steps/runner-selftest.sh` and `run-controls.sh` (exact inventory under `data/expected/controls/*.ids`, `control-exit-zero` rejection); gate `controls=112/112`; temp tree removed by trap (`run-controls.sh:15-17`) |
| T-01-38 downloaded Blue Sky page | medium | mitigate | CLOSED | No HTML or downloaded page tracked (`.gitignore` and `files.cmake`; `git ls-files refs` empty); `data/sources.json` records only id, digest and metadata |
| T-01-39 registry text disclosure | high | mitigate | CLOSED | `denylist.sh` (author names), `hygiene.sh` (absolute paths) both pass on `data/sources.json` and docs; `sources.sh` rejects local paths other than `refs/SRC-<id>.<ext>` (`source-local-path`) |
| T-01-SC package installs (registered in all 14 plans) | low | accept | CLOSED (accepted) | Acceptance documented in each plan's `<threat_model>`; confirmed: no `package.json`, `requirements`, `Cargo.toml`, `go.mod`, `Gemfile`, `mix.exs`, Dockerfile or lockfile tracked; the single external dependency is the SHA-pinned `actions/checkout` |

Summary: 39 numbered threats plus T-01-SC = 40 distinct ids; 40 closed, 0 open. `transfer` dispositions: none declared.

Unregistered flags: none. Every SUMMARY.md `## Threat Flags` section reads "None" or maps to an existing threat id.

## Review findings (01-REVIEW.md) re-verified in code

| Finding | Fix present |
|---------|-------------|
| CR-01 NUL byte hides file from scanners | `tools/check/steps/integrity.sh:26` `integrity-nul-byte`; `grep -a` in `hygiene.sh:72`, `denylist.sh`, `sources.sh:42`; controls `review-nul.ids` set (5); independent check: 0 NUL blobs in history |
| WR-01 ci-parity strictness | `tools/check/steps/ci-parity.sh` covers every workflow file; 6 `ci-parity-*` controls |
| WR-02 content-based binary detection | `tools/check/binscan.sh`, `tools/check/files.cmake:116-126`, 3 controls |
| WR-03 / WR-04 credential and scratch-path rules | `tools/hygiene.sh:21-41`; 10 controls |
| WR-05 weakest-field rule | `tools/check/schema/claim.txt` `when standing=established nonblank depends_on` |
| WR-06 grep-error masking | `tools/hygiene.sh:66-80` (`hygiene-grep-error`), step `hygiene-history`, 3 controls |

## Residual Risks

None of these meets the `block_on: high` threshold; they are recorded, not counted in `threats_open`.

1. **History baseline weakens pre-baseline commits (low).** `tools/check/hygiene-baseline.txt` holds commits at or before `8907ff7` to the older rule set because published history cannot be rewritten. An independent scan found only documented regex-source fragments (a scratch-directory prefix and a UID in review text) in those commits, no username, no secret. The baseline must never be moved forward to hide a finding.
2. **Scanners are line and regex based (low).** A renamed or reformatted copy of third-party expression, an unquoted `token = <long value>`, or a NUL-free binary carrying a text extension evades the gate (documented in `docs/RIGHTS.md`, `docs/provenance.md`, and the WR-02/WR-03 residuals).
3. **`denylist.sh` swallows a grep error (low, new finding).** `tools/check/steps/denylist.sh:55` runs the cite-list grep with `2>/dev/null || : > lines`, and the author-list grep at line 51 sits in an `if`, so a grep error (status 2) on one file reads as "no hit" in both. Exploitation needs a file that grep cannot read but `integrity` can, which is not a realistic path. Suggest mapping status 2 to a `denylist-grep-error` finding, as `tools/hygiene.sh` already does.
4. **No negative control for a `--tree` grep error (low).** Acknowledged in 01-REVIEW-FIX (WR-06 residual).
5. **`grep -a` is not POSIX (low).** Supported by GNU grep, BSD grep and ugrep; CI runs ubuntu-24.04 with GNU grep, and the gate also passed locally with ugrep. Not re-run under GNU grep in this audit.
6. **ci-parity is not a YAML parser (low).** Anchors, multi-document files and block scalars are not interpreted; it fails closed on key-looking lines. A second workflow is allowed if it passes the same rules.
7. **Transcribed manual data is committed (low, legal not security).** `data/cp1610/opcode-families.json` holds facts transcribed from a third-party manual, with cites by `SRC-` id and no page images. The rights position (facts and paraphrase, not expression) is an owner and counsel question; nothing rights-bearing in binary form is tracked.
8. **Repository settings are out of scope.** Branch protection, fork-PR approval policy and Actions default-permission setting on the GitHub side were not inspected; the workflow itself is least-privilege either way.
9. **Local-only state.** `refs/` holds a cached third-party PDF and page images, correctly ignored; they are not in history. `.planning/config.json` has an unstaged local modification that is not part of any commit.
