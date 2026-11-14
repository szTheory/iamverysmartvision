---
phase: 08-stic-completion-save-state-performance-baselines
plan: 01
subsystem: Phase 8 wave D1 - release machinery - content-based release asset checker and its gate step, tag-triggered release workflow under a ci-parity exemption table, the release script with the consumer run against the shipped archive, CHANGELOG under the claim scan, the Phase 8 start cost entry
tags: [D1, REL-01, release-assets, release.yml, release.sh, ci-parity, CHANGELOG, text-scope, phase8-start, D-29, D-31, D-32, D-36, D-37, D-43, T-08-01, T-08-02, T-08-03, T-08-04]
status: complete

requires:
  - phase: 07-first-visible-original-scenario
    provides: "Phase 7 end gate: steps=36/36 controls=357/357 faults=222/222 tests-expected=4628 tests-executed=4624 tests-skipped=4 tests-missing=0 ci-uses=1 ci-workflows=1 pkg-selftest=17/17 (re-measured at the start of this plan, identical); HEAD 181dce1 at the start; docs/packaging.md Release content (D-33); the installed consumer and its golden"
provides:
  - "tools/check/release-assets.sh: member-level checker of a gzip tar release archive (modes source and install; tokens release-assets-unreadable, -path, -prefix, -extra-member, -missing-member, -binary-member); the extension list is read from tools/check/binary-ext.txt, the outer file name is never judged"
  - "gate step release-assets (step 37): FILELIST binary-extension scan (release-assets-source-binary) and a five-case in-step selftest of the library on synthetic archives (release-selftest=5/5); the later state step becomes step 38"
  - "tools/release.sh: tag, gate, deterministic source and install archives, content check, SHA256SUMS, the installed consumer built against the very install archive that ships and compared with data/expected/pkg-consumer.golden, guarded draft pre-release; dry run by default"
  - ".github/workflows/release.yml: strict tag trigger, job-level contents write on the one job, one GITHUB_TOKEN reference, timeout 15, two run lines; tools/check/steps/ci-parity.sh per-file exemption table with nine new controls"
  - "CHANGELOG.md (Unreleased section, no claims table yet) under the text claim scan with control text-changelog-untagged; .planning/CI-EFFICIENCY.md entry ending phase8-start: no-lever"
affects: [08-02, 08-08, 08-13, 08-15, 08-20, 08-21]

actuals:
  tokens: 11984
  tasks: 3
  commits: 3
plan_head_before: 181dce1bbed1f104774010bab7544adfc17174d3
plan_head_after: 9f0e4d20ce88ffb7331801bacf44893495cb292a

key-decisions:
  - "The new gate step is release-assets and takes the number 37 (08-CONTEXT D-38 numbered the later state step 37; it becomes 38): the library is judged by a gate step so a broken checker cannot reach a tag unnoticed, and the step costs one in-step selftest instead of a new job"
  - "The check is content-based: it judges archive members, never the outer name, because binary-ext.txt lists gz and tar; the binary rule is independent of the list (a .rom member that the list itself names is still refused), proved by the rom-member case, and the positive outer-name-pass case proves a clean archive called .tar.gz passes"
  - "--publish is refused with exit 2 unless GITHUB_ACTIONS is true, and also when combined with --no-gate (a published release always runs the gate); the notes section of CHANGELOG.md is checked before any gh call"
  - "release.yml gets exactly one exemption row per dimension in ci-parity (trigger, job-level write, run allowlist, one secret, timeout 15); the job-level write is accepted only as contents write in release.yml, and a job-level write in gate.yml or any other file still fails"
  - "phase8-start: no-lever on a last-five median r of 7.78 against the line 8.3; the projection with this plan's measured additions is about 7.84"

key-files:
  created:
    - tools/check/release-assets.sh
    - tools/check/steps/release-assets.sh
    - tools/release.sh
    - .github/workflows/release.yml
    - CHANGELOG.md
    - data/expected/gate-steps/release.ids
    - data/expected/release-assets-selftests.ids
    - data/expected/controls/release.ids
    - tools/check/controls/_good/tree/data/expected/release-assets-selftests.ids
    - tools/check/controls/_good/tree/.github/workflows/release.yml
    - tools/check/controls/release-assets-source-binary/
    - tools/check/controls/ci-parity-release-extra-run/
    - tools/check/controls/ci-parity-release-pull-request/
    - tools/check/controls/ci-parity-release-tag-wide/
    - tools/check/controls/ci-parity-release-secret/
    - tools/check/controls/ci-parity-release-top-write/
    - tools/check/controls/ci-parity-gate-job-write/
    - tools/check/controls/ci-parity-release-no-tag-trigger/
    - tools/check/controls/text-changelog-untagged/
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-01-SUMMARY.md
  modified:
    - tools/check/steps/ci-parity.sh
    - tools/check/steps/text.sh
    - data/expected/controls/ci.ids
    - data/expected/controls/claims.ids
    - .planning/CI-EFFICIENCY.md
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-14
---

# Phase 8 Plan 01: release machinery Summary

**The release machinery now sits in the gate before any machine behaviour changes: a member-level asset checker with a gate step that proves it can fail four ways and pass once, a tag-triggered `release.yml` that `ci-parity` accepts only through a per-file exemption table that nine controls show cannot widen, a release script whose dry run builds the source and install archives, checks their members and runs the installed consumer against the very archive it ships, and a `CHANGELOG.md` under the claim scan; the last-five median `r` is 7.78 and the start line is `phase8-start: no-lever`; the final gates pass under sh, dash and the Ubuntu container with `steps=37/37 controls=366/366 faults=222/222 tests-expected=4628 tests-executed=4624 tests-skipped=4 tests-missing=0 ci-uses=2 ci-workflows=2 release-selftest=5/5 pkg-selftest=17/17`. REL-01 is started, not closed: it completes after the stage 2 records pull request (D-28).**

## Commits

| Commit | Task | What |
|--------|------|------|
| `7a61a5c` | 1 (tracer) | `tools/check/release-assets.sh`, gate step `release-assets` with the five selftest cases, inventories, toy-tree inventory, control `release-assets-source-binary` (steps 36 to 37, controls 357 to 358) |
| `31e25cd` | 2 | `release.yml`, `tools/release.sh`, the `ci-parity` per-file branch for `release.yml`, toy-tree `release.yml`, seven controls (controls 358 to 365, `ci-uses=2 ci-workflows=2`) |
| `9f0e4d2` | 3 | `CHANGELOG.md`, `text.sh` scope, control `text-changelog-untagged` (controls 365 to 366), the `phase8-start` entry in `CI-EFFICIENCY.md` |
| the commit that adds this file | | this SUMMARY, STATE.md (phase 8 begun, plan 1 recorded), ROADMAP.md (plan list and progress) |

## What was built

- **Asset checker (T-08-02).** `tools/check/release-assets.sh source|install <archive> <prefix> <listfile>` lists the members with `tar -tzf` (GNU tar and bsdtar), applies the path and prefix rules to every entry, drops directory entries, compares the sorted member set with the list (extra, missing, duplicate) and refuses any member whose last extension is a line of `tools/check/binary-ext.txt`, whatever the list says. Findings are `<token>: <detail>`, at most 20 per token, exit 1; clean is one line `release-assets <mode> ok members=<n>`; usage errors exit 2. It writes only into a mktemp directory.
- **Gate step `release-assets` (step 37).** (a) every FILELIST path with a last extension on the binary list is `release-assets-source-binary` (zero on the real tree); (b) five selftest cases on synthetic archives built with `tar -czf` under a temp dir, inventory `data/expected/release-assets-selftests.ids`: `source-extra-member`, `source-missing-member`, `install-extra-member` (an extra library member), `rom-member` (the list names a `.rom` member and it is still refused) and `outer-name-pass` (the positive case, a clean archive named `ivs-test.tar.gz` passes). Control `release-assets-source-binary` (a toy `data/x/diag.rom`) and the toy-tree inventory keep the positive `_good` control green.
- **`tools/release.sh`.** Order: tag (strict `v<digits>.<digits>.<digits>`, from `--tag`, else `GITHUB_REF_NAME`, else the three `IVS_API_VERSION_*` macros), the CHANGELOG section for the version, the gate (unless `--no-gate`), the source archive (`git archive` with the prefix `ivs-<tag>/`, then `gzip -n`), the install archive of `build/release` (staging prefix, GNU tar with `--sort=name --mtime=@<commit time> --owner=0 --group=0 --numeric-owner` when GNU tar is found, else the sorted member list through `cmake -E tar` with `--format=gnutar --mtime=<UTC time>`; the chosen mode is printed; `gzip -n`), the member check of both archives (source against `git ls-files`, install against `data/expected/pkg-install-files.ids`), `SHA256SUMS` by `cmake -E sha256sum` in sorted order, the extraction of the install archive with a scan of the extracted tree for the path of this checkout, and the installed consumer (copy of `tests/installed-consumer`, only `-DCMAKE_PREFIX_PATH`, package registries off, path variables unset in a subshell, strict C17 build, run on the six images of `build/release/images`, stdout equal to `data/expected/pkg-consumer.golden` without its `#` lines). Only `--publish` reaches `gh release create <tag> <three assets> --draft --prerelease --verify-tag --latest=false --title <tag> --notes-file <section>`.
- **`release.yml` and `ci-parity` (T-08-01, T-08-03).** Trigger exactly `push` / `tags` / the single strict glob `v[0-9]*.[0-9]*.[0-9]*` (a GSD milestone label like v1.0 does not match); top-level `contents: read`; one job `release` on `ubuntu-24.04`, `timeout-minutes: 15`, job-level `contents: write`; the SHA-pinned checkout of `gate.yml` with `persist-credentials: false`; run lines `cmake --version` and `sh tools/release.sh --publish`; one `env` entry `GH_TOKEN` from `secrets.GITHUB_TOKEN`. In `ci-parity.sh` an `isrel` branch keyed by file name carries the exemption table (trigger state machine over `on` / `push` / `tags`, job-level write only in `release.yml`, run allowlist, `secrets.GITHUB_TOKEN` as the only secret with a suffix guard, job `release` timeout exactly 15); every `gate.yml` rule and token is unchanged.
- **CHANGELOG and text scope (T-08-04).** `CHANGELOG.md` holds `# Changelog`, one format sentence and `## [Unreleased]` with the scope sentence, what a release contains (source archive, the ten-file static install archive, `SHA256SUMS`), what it does not contain, the not-qualified list (D10 and D11 not performed, nobody looked or listened, tier 4 or inference rules), the working-name line, the non-affiliation lines and the placeholder `<!-- claims-table: rendered by plan 08-02 -->`. `text.sh` scans it (`docs=51`).
- **Cost entry.** `.planning/CI-EFFICIENCY.md` has the Phase 8 start entry: the five run ids and `r` values (8.17, 7.73, 7.78, 8.37, 7.20, median 7.78), the D-37 wave budget, this plan's measured additions, the checkpoints (plans 08-08, 08-13, 08-15 and the close) and the stop rule, the lever order, and the one line `phase8-start: no-lever`.

## Dry run of `tools/release.sh --no-gate --tag v0.1.0` (information for plan 08-02)

On the laptop (Darwin arm64, bsdtar, after a local gate run), on the committed tree of `31e25cd` (2317 tracked files), output directory outside the repository; wall time about 1.5 s; exit 0:

| Asset | Bytes |
|-------|-------|
| `ivs-v0.1.0-source.tar.gz` | 4526857 |
| `ivs-v0.1.0-install-darwin-arm64.tar.gz` | 61218 |
| `SHA256SUMS` | 196 |

- Tar mode `cmake-tar` (the host tar is bsdtar). `release-assets source ok members=2317`, `release-assets install ok members=10`, the consumer against the shipped install archive printed 135 lines equal to `data/expected/pkg-consumer.golden`.
- Two dry runs on this host: the source archive has the same SHA-256 both times (`2d2767f5...`); the install archive differs (`b1479830...` against `f6f6f666...`). The tar and gzip layers are deterministic; the one differing member is `lib/libivs_core.a`, because CMake re-runs ranlib at install on Apple and restamps the symbol-index member header (the same effect `pkg-install` documents). On Darwin the `cmake -E tar` mode also records the host user and group, so this mode is for local rehearsal only.
- Rehearsal of the full script, gate included, in an Ubuntu 24.04 container (x86_64, emulated on this arm64 laptop; dash as `/bin/sh`, GNU tar 1.35, gcc 13.3.0, cmake 3.28.3) on the tree of `9f0e4d2` plus a scratch commit: exit 0; tar mode `gnu-tar`; `release-assets source ok members=2321`, `install ok members=10`; assets `ivs-v0.1.0-source.tar.gz` 4528346 bytes, `ivs-v0.1.0-install-linux-x86_64.tar.gz` 62857 bytes (members owner 0/0, mtime of the commit), `SHA256SUMS` 196 bytes; the consumer output equals the golden; a second run with `--no-gate` gave a byte-identical `SHA256SUMS` (source `c349b03f...`, install `aa9c28c3...`), so the GNU path is reproducible on one host; `--publish` outside GitHub Actions exited 2. No `gh` call was made anywhere.

## Isolated step timings (laptop, user plus sys cpu, three alternated runs; load average 10 to 14)

| Step | Runs (cpu-s) | Median |
|------|--------------|--------|
| `release-assets` | 0.31, 0.34, 0.33 | 0.33 |
| `ci-parity`, before this plan | 0.01, 0.01, 0.01 | 0.01 |
| `ci-parity`, after (two workflow files) | 0.03, 0.03, 0.03 | 0.03 |

The `release-assets` step runs three times in a gate (the step, its positive `_good` control, its negative control), about 1.0 local cpu-s; with eight pure controls and the `ci-parity` delta the plan adds about 1.6 local, about 2.6 hosted cpu-s, a projected median `r` of about 7.84 (the plan estimated about 1.4 hosted; the selftest cost is the difference). Details and the rule are in `.planning/CI-EFFICIENCY.md`.

## Checks shown able to fail

- **Selftest cases against mutated libraries** (copies outside the tree): an inventory name the step does not know gives `release-selftest-missing`; a library that always exits 0 fails all five cases; a library without the binary rule fails `rom-member` only; a library that judges the outer name fails all five including `outer-name-pass`.
- **`ci-parity` exemptions**: each of the seven new controls fails with its intended token on its own tree (extra run, `pull_request` key, wide tag item, second secret, top-level write, gate.yml job-level write, no tag trigger; also a second token at line 0 where no tag item exists). Further ad hoc probes, not controls, all fail with a `ci-parity` token: `sh tools/gate.sh` as a run line of `release.yml`, `secrets.GITHUB_TOKEN_X`, timeout 5, job-level `contents: read` in `release.yml`, a `workflow_dispatch` key, a `tags-ignore` key, a second tag item, `on: push` as a shorthand, an `if:`, a job not named `release`, and a job-level write in a third workflow file.
- **`text-changelog-untagged`**: against the step as it was before this plan the control's tree gives `-- text pass` and exit 0, which the control runner reports as `control-exit-zero`; with the final step it fails on `CHANGELOG.md:1`.
- **Guards exist where the controls point (lesson 7)**: `release-assets-source-binary` lives in `tools/check/steps/release-assets.sh`; `ci-parity-release-trigger`, `ci-parity-extra-run`, `ci-parity-secrets` and `ci-permissions` live in `tools/check/steps/ci-parity.sh`; the scope token is in `tools/check/steps/text.sh`.

## Deviations from Plan

**1. [Rule 3 - blocking] The mtime format for `cmake -E tar`.** The first dry run failed with `unable to parse mtime` for the ISO form with a `T` separator. The accepted form is `YYYY-MM-DD HH:MM:SS UTC`; `tools/release.sh` uses it. Found during the dry run, fixed before the Task 2 commit.

**2. [Rule 2 - hardening] `--publish` also refuses `--no-gate`.** The plan lets `--no-gate` skip the gate for local rehearsal and makes `--publish` refuse outside GitHub Actions. A published release that skipped the gate would be exactly the mistake the machinery exists to prevent, so the combination is refused with exit 2 as well. The CHANGELOG section check also runs before the gate for `--publish`, so a missing release body costs no gate time.

**3. [Interpretation] The dry run was recorded on the committed tree.** The plan lists the dry run before the Task 2 gates and commit. `git archive` ships HEAD and the member check compares with `git ls-files` (the index), so the recorded run is on the committed tree of Task 2; an earlier run on the uncommitted tree exercised the same path and found the mtime defect above.

**4. [Interpretation] The scope sentence of the CHANGELOG says "executes", not "runs".** `runs` is a claim word, and the plan's wording would have needed a waiver; the sentence now reads "nothing downloadable executes a program". The meaning is the same.

**5. [Addition] A scan of the extracted install tree for the path of this checkout.** The plan truth "no absolute home path is written into any asset" had no check in the script; `tools/release.sh` greps the extracted install tree for the checkout path in its logical and physical forms and fails on a hit. Zero hits on this laptop and in the container.

**6. [Information] STATE.md and ROADMAP.md.** STATE.md still named Phase 7 at the start of this plan, so `state begin-phase --phase 8 --plans 21` was run before the plan-complete calls, and `roadmap update-plan-progress` wrote the Phase 8 plan list into ROADMAP.md (the section read `Plans: TBD`). The D-34 stale-text sweep of plan 08-20 is unchanged by this.

**7. [Information] Container platform.** `GATE_LINUX_PLATFORM=` was set empty as instructed; the container nevertheless reported `x86_64` (the image on this host is the amd64 one: Docker printed the platform mismatch warning), so the container figures are emulated amd64, not native arm64 (gate-time total 323 s, gate-cpu user 1274 s is emulation).

No Rule 1 bug in earlier code; no auth gates; no package installs (T-08-SC); no `gh` call, no push, no tag; `.planning/config.json` and `.planning/state.json` were never staged (D-42; `state.json` and `config.json` stay modified in the working tree as they were).

## Decisions Made

- Step 37 is `release-assets`; the later `state` step is 38 (recorded for plans 08-09 onward: the step count after that plan is 38, not 37).
- The binary rule is checked on members and is independent of the list; the outer name is never judged.
- A single `--publish` guard order: refusal outside GitHub Actions first (exit 2, no other work), refusal with `--no-gate`, `gh` only at the very end after every check.
- The per-file exemption table keys on the exact file name `.github/workflows/release.yml`; a copy under another name gets no exemption.
- `release.yml` keeps the shallow checkout of `gate.yml` (`git archive`, `git ls-files` and the commit time need no history).

## Open Items

- **Plan 08-02**: add the version agreement check (tag, header macros, `ivs_VERSION`) in front of step (c) of `tools/release.sh`; render the claims table at the CHANGELOG placeholder from `data/claims.json` and gate its drift; the determinism proof should use the Linux (GNU tar) path or compare the Darwin install archive member by member apart from the ranlib-restamped index header.
- **Orchestrator**: the `git.create_tag` change in `.planning/config.json` (D-42); the hosted run record of the head of this plan (`tools/ci-record.sh`, never called here); the first hosted rows after this push are the arbiter of the projected `r` of about 7.84. `release.yml` has never run on a hosted runner; the first real run is at the tag, so the container rehearsal above is the only evidence of the GNU tar path and of dash.
- **Information, not a defect**: on this laptop `tools/release.sh` prints `tracked files differ from HEAD` because `.planning/config.json` and `.planning/state.json` are modified; the assets are built from HEAD and are unaffected.
- Carried unchanged: D10 and D11 not performed; `sh tools/hygiene.sh --history` on the six earlier planning commits (D-33).

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=37/37 controls=366/366 faults=222/222 ... ci-uses=2 ci-workflows=2 ... release-selftest=5/5 ... pkg-selftest=17/17 ... tracked=2321 ... tests-expected=4628 tests-executed=4624 tests-skipped=4 tests-missing=0 docs=51 claim-refs=16 (0 FAIL lines; gate-time total=85 s, gate-cpu user=137.0 sys=123.6, laptop under load)
dash tools/gate.sh:    the identical summary line (md5 dcb13a3f20f53734ba23c1faaab13bfb for both shells; 0 FAIL lines; total=86 s)
sh tools/gate-linux.sh: exit 0 on the same tree (build/gate-linux.log ends gate: PASS; gcc 13.3.0, cmake 3.28.3, x86_64 emulated), steps=37/37 controls=366/366 faults=222/222 ci-uses=2 ci-workflows=2 release-selftest=5/5 pkg-selftest=17/17 tests-expected=4628 tests-executed=4624 tests-skipped=4 tests-missing=0 docs=51, 0 FAIL lines (gate-time total=323 s)
```

On the closing tree (this SUMMARY, STATE.md and ROADMAP.md added or edited; `tracked=2322`): sh (84 s) and dash (94 s) `gate: PASS` with the identical summary line (md5 274aeb3e9f5407ccd3e5bac7478605c1), 0 `FAIL` lines.

After Task 1 (before its commit): sh and dash `steps=37/37 controls=358/358 release-selftest=5/5`. After Task 2 (before its commit): sh and dash `controls=365/365 ci-uses=2 ci-workflows=2`. Counts at plan start were `steps=36/36 controls=357/357 faults=222/222 tests-expected=4628 tests-executed=4624 tests-skipped=4 tests-missing=0 ci-uses=1 ci-workflows=1 pkg-selftest=17/17`; at the end `steps=37/37 controls=366/366` (357 + 1 + 7 + 1), everything else equal except `ci-uses=2 ci-workflows=2 release-selftest=5/5`; no test id and no fault was added. `git ls-files '*.ivsc' '*.bin' '*.rom' '*.exec' '*.wav' '*.ppm' '*.vraw'` is empty.

## Known Stubs

None. The `<!-- claims-table: rendered by plan 08-02 -->` line in `CHANGELOG.md` is a deliberate placeholder owned by plan 08-02 (named in the plan), not a stub that blocks this plan's goal.

## Threat Flags

None new. T-08-01 (release workflow privilege): the exemption table and nine controls; `--publish` guard; draft pre-release only. T-08-02 (asset smuggling): member-level check with the binary rule independent of the list, five selftest cases, consumer on the shipped archive. T-08-03 (look-alike tag): strict glob, strict tag check in the script; the version agreement is plan 08-02 and the `git.create_tag` change stays an orchestrator commit. T-08-04 (CHANGELOG overreach): in the claim scan with a control. T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: every file listed under `key-files.created` exists (`tools/check/release-assets.sh`, `tools/check/steps/release-assets.sh`, `tools/release.sh`, `.github/workflows/release.yml`, `CHANGELOG.md`, the three inventories, the toy-tree files and the nine control directories); commits `7a61a5c`, `31e25cd` and `9f0e4d2` are ancestors of HEAD (`git rev-list --count 181dce1bbed1f104774010bab7544adfc17174d3..HEAD` printed 3 before this file's commit); `grep -c '^phase8-start: ' .planning/CI-EFFICIENCY.md` prints 1; `.planning/config.json` and `.planning/state.json` are not staged.
