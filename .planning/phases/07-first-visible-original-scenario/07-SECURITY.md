---
phase: 07-first-visible-original-scenario
asvs_level: 1
block_on: high
status: secured
threats_total: 32
threats_closed: 32
threats_open: 0
audited: 2026-11-14
audited_commit: a53431a
---

# Phase 7 Security Verification

**Verdict: SECURED.** All 32 threats are closed: 31 numbered threats (T-07-01 to T-07-31, all `mitigate`; 11 high, 16 medium, 4 low) and the shared accept T-07-SC (low), repeated in plans 01 to 08 and 10 to 12. The register was authored at plan time, so the audit verified that each mitigation exists in code, scripts, gate steps, controls, tests or committed records. No new threats were added. Every `## Threat Flags` section in 07-01 to 07-12 reads "None new".

Method: the auditor located each mitigation and re-checked the recipe digests with `shasum` (they match after the review fix 4fa3214). Where a negative control or self-test is cited, its existence and EXPECT token were confirmed; the run evidence is the gate record in `07-VERIFICATION.md` and the final gate (`steps=36/36 controls=357/357`, sh, dash and the gcc container). The code review (`07-REVIEW.md`, status fixed) found WR-01 and WR-02 (both fixed by 4fa3214 and b9d0c7b); they tighten T-07-10, T-07-12 and T-07-27.

## Threat register

| Threat | Sev | Status | Evidence (repo-relative) |
|--------|-----|--------|--------------------------|
| T-07-01 checker lever drops a finding | med | CLOSED | 40 `vectors-*` controls in `tools/check/controls/`, ids `data/expected/controls/vectors.ids` and `vectors-review.ids`; lever commits 4f45832, 5b932a9; summary line byte-identical (07-01-SUMMARY) |
| T-07-02 derived values taken from a run | high | CLOSED | oracle commit 4e0febe precedes program commit 949407e; blind second read `docs/video/demo-lamp-tone-1-second-read-record.md`; claims provisional |
| T-07-03 third-party text in derivation docs | med | CLOSED | `tools/check/steps/text.sh`, `denylist.sh`, `glossary`, `hygiene` |
| T-07-04 vacuous pair or shape checks | med | CLOSED | `tools/check/video.cmake` (`video-frame-pair-region`), `tools/check/steps/golden.sh` (`golden-shape`), drift controls |
| T-07-05 wrong encoding accepted | high | CLOSED | `tests/asm-tap.sh` twins vs hand listings, `asm.judge.mutated-twin`, 304-pair corpus `tests/asm/pairs.txt`, `tools/check/listing.cmake` recomputation |
| T-07-06 assembler DoS on hostile input | med | CLOSED | `tools/asm/asm.c` bounds (line, lines, words, bytes, symbols, path), CR/non-ASCII refusal, `ivs_san` link |
| T-07-07 GPL or third-party table copied | med | CLOSED | `tools/asm/asm.c` header attestation, `denylist.sh` covers `tools/asm/`, 07-03-SUMMARY |
| T-07-08 DATA row hides a word | med | CLOSED | `tools/check/listing.cmake`, controls `listing-data-in-operand-slot`, `listing-data-after-sdbd` |
| T-07-09 pairs corpus drift | low | CLOSED | `tools/check/steps/tests-inventory.sh` (`tests-vectors-digest`), control `tests-inventory-stale-digest` |
| T-07-10 refusal crash, hang or partial file | med | CLOSED | `data/expected/asm-tokens.ids`, `neg_run` in `tests/asm-tap.sh`, temp-file-and-rename in `tools/asm/asm.c` |
| T-07-11 stub grows services | med | CLOSED | `tools/check/listing.cmake` `bootstub-size`, `-row`, `-vector` and controls |
| T-07-12 stale recipe digest | high | CLOSED | control `manifests-recipe-source-digest`; digests re-verified; `asm.regen-*` cases |
| T-07-13 out-of-range content kind or role | med | CLOSED | `runner/scenario.c`, `runner/exec.c`, `tests/runner/runner_fuzz.c`, `runner_parse.c` |
| T-07-14 third-party glyph, tune, EXEC or GROM in program | high | CLOSED | `fixtures/FX-demo-lamp-tone-1/lamp-tone.asm`, manifest attestation, scenarios use `boot-stub-2` and `zero-grom` |
| T-07-15 trademark word in a name | med | CLOSED | `tools/check/files.cmake` `files-trademark-name` and control |
| T-07-16 built image committed | med | CLOSED | `.gitignore`, `binary-ext.txt`, `files-unlisted-binary`; no binary types tracked |
| T-07-17 vacuous pass or idle evidence | high | CLOSED | `tests/runner-process-tap.sh` `check_pair`, `data/expected/scenario-pairs.ids`, self-controls |
| T-07-18 host paths in goldens | low | CLOSED | grep of `data/golden` clean; hygiene controls |
| T-07-19 wrong mutant target word | med | CLOSED | `data/expected/diag-mutations-scn.ids` explicit `from`, must-change `cmp`, `fails=` judge |
| T-07-20 mutant loops forever | low | CLOSED | every scenario needs `run-to`; patched images in suite temp dir only |
| T-07-21 consumer and golden skew | high | CLOSED | `tools/check/steps/pkg-install.sh` (`scn.digest.*`), self-tests `scn-digest-drift`, `scn-domain-missing` |
| T-07-22 image bytes in the consumer | med | CLOSED | `tests/installed-consumer/main.c` reads images from argv paths; only small pattern tables |
| T-07-23 owner images or digests enter the tree | high | CLOSED | `.gitignore` (`/refs/`, `*.bin`, `*.grom`, `*.img`), FX-owner-* `committed: false`, `files-tracked-refs` |
| T-07-24 user-content scenario as ordinary test | med | CLOSED | `tools/check/steps/golden.sh` controls, `data/expected/runner-scenarios.ids` skip-ok, `tests-inventory-unexpected-skip` |
| T-07-25 third-party text in the route page | low | CLOSED | `docs/owner-images.md` under text, glossary, denylist and hygiene steps |
| T-07-26 owner-run in CI | high | CLOSED | `tools/owner-run.sh` CI refusal, tests `owner-run.ci-env`, `.github-actions-env`; workflow never invokes it |
| T-07-27 in-tree path outside `refs/` | high | CLOSED (residual R-07-A) | `tools/owner-run.sh` `cd -P --`, physical-path comparison, fail-closed on unresolvable dir (b9d0c7b); test `owner-run.in-tree-path` |
| T-07-28 lock-file injection | med | CLOSED | `tools/owner-run.sh` strict `chk`, no `eval`; tests `owner-run.bad-lock-line`, `.identity-mismatch` |
| T-07-29 derived audio, log or WAV of user content | high | CLOSED | `runner/main.c` refusal of `--audio-raw`, `--psg-log-out`, `--audio-wav` for user content before any file; `refuse-user.*` tests |
| T-07-30 overclaim | high | CLOSED | `data/claims.json` CL-0023, CL-0024 provisional; `claim-overstated`, `claim-dangling-path`; `plays|playable` tripwire with control `text-untagged-playable` |
| T-07-31 stale or false public statements | med | CLOSED | `README.md`, `.planning/PROJECT.md`, `REQUIREMENTS.md`, hand-off table H7-1..H7-5 in `07-VERIFICATION.md` |
| T-07-SC npm, pip or cargo installs | low | CLOSED (accepted) | no package manifests or lockfiles tracked; every SUMMARY says no installs |

## Accepted risks log

- **T-07-SC (low):** no package manager or dependency in Phase 7; the core and tools have zero runtime dependencies. Repeats T-06-SC.
- **R-07-A (T-07-27 residual, low):** the owner-run guard compares the physical directory of an image, not the file leaf, so a symlink in git-ignored `refs/` pointing at a file elsewhere in the work tree passes. The operator creates the link deliberately, the helper stages and writes nothing, and no derived output of user content is produced (T-07-29). Recorded in 07-11-SUMMARY and 07-VERIFICATION; optional hardening listed there.
- **R-07-B (low):** `tools/owner-run.sh --record` is trust-on-first-use. It refuses to overwrite an existing lock, and the lock lives in git-ignored `refs/`.
- **R-07-C (low):** the `.digests` goldens and the consumer golden are self-recorded repeatability evidence, not independent evidence. Independent values are the hand-derived asserts, with the shared-author caveat in 07-VERIFICATION section 3.
- **R-07-D (low):** the absence of a 64-hex digest in `docs/owner-images.md` was checked by grep (zero matches) but no dedicated gate step enforces it; the page is covered by the generic hygiene, text and denylist steps and by review.
- **R-07-E (info):** `lbl: ORG $xxxx` binds the label before the fill gap. Not a security matter; left unchanged (see 07-REVIEW.md fix notes).

## Unregistered flags

None. `ivs_asm` writes one file beside the caller-named listing path (temp file then rename, refused when equal to the source); covered by T-07-06 and T-07-10.

## Hard constraints

No rights-bearing bytes, no absolute home or scratch paths and no trademarked names found in tracked files. D10 and D11 were not performed. Review fixes WR-01, WR-02, IN-01 and IN-02 are present in code.

## Security Audit

| Threats found | Closed | Open |
|---------------|--------|------|
| 32 | 32 | 0 |
