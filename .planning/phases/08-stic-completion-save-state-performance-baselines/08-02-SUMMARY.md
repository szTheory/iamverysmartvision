---
phase: 08-stic-completion-save-state-performance-baselines
plan: 02
subsystem: Phase 8 wave D1 - release machinery, second half - version policy check, read-only readiness generator, claim table rendered from data/claims.json and gated, pinned history disposition, local release rehearsal, orchestrator-only release note
tags: [D1, REL-01, version-policy, release-readiness, claims-table, release-rehearse, hygiene-history-pinned, D-27, D-30, D-31, D-33, D-36, D-42, T-08-05, T-08-06, T-08-07, T-08-08]
status: complete

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-01 end gate: steps=37/37 controls=366/366 faults=222/222 tests-expected=4628 tests-executed=4624 tests-skipped=4 tests-missing=0 ci-uses=2 ci-workflows=2 release-selftest=5/5 pkg-selftest=17/17 (re-measured at the start of this plan, identical); HEAD d108fd5 at the start; the release-assets library and step, tools/release.sh, release.yml, CHANGELOG.md with the claim-table placeholder"
provides:
  - "tools/check/release-assets.sh modes version (tag == IVS_API_VERSION macros == installed PACKAGE_VERSION, strict tag form; tokens release-version-skew, -tag-form, -unreadable) and changelog (a section for the version; tokens release-changelog-section-missing, -unreadable)"
  - "tools/release.sh: the version check on the staged install before the install archive is written; the flag --release (the CHANGELOG section is the first step, before the gate; --publish implies it)"
  - "tools/check/claims-table.cmake: the claim table rendered from data/claims.json (id, tier, standing, artifact; totals line), refusing a measured-hardware claim; CHANGELOG.md holds it between claims-table-begin and claims-table-end and the release-assets step fails on drift (release-changelog-claims-drift) or on a missing notes file (release-changelog-missing)"
  - "tools/release-readiness.sh: read-only generator of close-record sections A, F, G, J and row P4 with a four-case selftest; the history findings are compared by (token, short sha) pair with data/expected/hygiene-history-pinned.ids; --hosted is the only mode that names gh and is orchestrator-only"
  - "tools/release-rehearse.sh: two dry runs, byte-equal source archives, install archive comparison, tag glob samples, readiness sections A F G; never calls gh"
  - ".planning/ops/RELEASE-OPS.md: the orchestrator-only release actions (git.create_tag, stage 2, tag on the artifact commit, draft flow with P1 to P8, hosted reads, the 0.1.0 section halt)"
affects: [08-08, 08-20, 08-21]

actuals:
  tokens: 17846
  tasks: 3
  commits: 3
plan_head_before: d108fd5ce6744972cd1ec010e52e2aaef035a85c
plan_head_after: bc7f33c220b88ddc324229d27673e23ebea9e619

key-decisions:
  - "Version policy (D-30): the software version is the IVS_API_VERSION macros, the package version is the PACKAGE_VERSION of the installed ivsConfigVersion.cmake (ivs_VERSION), the milestone label and the format versions are other numbers that never name a tag; the check is a mode of the asset library, so the gate proves it can fail and release.sh runs it on the staged install"
  - "The release-only CHANGELOG section check is a clean halt, never a gate failure: tools/release.sh --release and --publish, tools/release-readiness.sh --release (row P4) and tools/release-rehearse.sh --release halt on release-changelog-section-missing until plans 08-20 and 08-21 write the 0.1.0 section; the plain gate, dry run and rehearsal stay green without it"
  - "Any commit that changes data/claims.json regenerates the CHANGELOG claim table in the same commit (the release-assets step compares the region byte for byte with claims-table.cmake)"
  - "History findings are pinned by (token, short sha) pair, never by token (T-08-06): an accepted pair prints FAIL with its recorded disposition, any other pair prints NEW FINDING and fails the run; no pass verdict is ever printed for a history run that had findings, the baseline file is untouched and no rule is widened"
  - "The rehearsal accepts exactly one named host effect: on Darwin the install step restamps the time field of the first member header of lib/libivs_core.a (bytes 25 to 36); that pair prints install-archive: equal-except-ranlib-stamp, every other difference and every Linux difference is DIFFERENT and fails"

key-files:
  created:
    - tools/check/claims-table.cmake
    - tools/release-readiness.sh
    - tools/release-rehearse.sh
    - data/expected/hygiene-history-pinned.ids
    - tools/check/controls/release-assets-claims-drift/CHECK
    - tools/check/controls/release-assets-claims-drift/EXPECT
    - tools/check/controls/release-assets-claims-drift/overlay/CHANGELOG.md
    - .planning/ops/RELEASE-OPS.md
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-02-SUMMARY.md
  modified:
    - tools/check/release-assets.sh
    - tools/check/steps/release-assets.sh
    - tools/release.sh
    - CHANGELOG.md
    - docs/packaging.md
    - data/expected/release-assets-selftests.ids
    - data/expected/controls/release.ids
    - tools/check/controls/_good/tree/data/expected/release-assets-selftests.ids
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-14
---

# Phase 8 Plan 02: version policy, release readiness and rehearsal Summary

**A tag that disagrees with the header macros or the installed package is refused by a check the gate proves able to fail; the computable parts of the milestone close record are generated by a read-only script that can fail four ways; the CHANGELOG claim table is rendered from `data/claims.json` and held by a drift check with one control; the six accepted history findings are pinned by (token, sha) pair; and the release job has been rehearsed twice with byte-equal source archives and a checked tag glob. The final gates pass under sh, dash and the Ubuntu container with `steps=37/37 controls=367/367 faults=222/222 tests-expected=4628 tests-executed=4624 tests-skipped=4 tests-missing=0 ci-uses=2 ci-workflows=2 release-selftest=11/11 pkg-selftest=17/17`. REL-01 stays Pending: it completes after the stage 2 records pull request (D-28).**

## Commits

| Commit | Task | What |
|--------|------|------|
| `fdb62c3` | 1 (tracer) | version and changelog modes of `tools/check/release-assets.sh`, five selftest cases (release-selftest 5/5 to 10/10), `tools/release.sh` wiring and `--release`, `docs/packaging.md` version rule and release workflow |
| `db4c9b8` | 2 | `claims-table.cmake`, `tools/release-readiness.sh`, the CHANGELOG table region and its drift check, the pinned history list, `readiness-selftest` case (10/10 to 11/11), control `release-assets-claims-drift` (controls 366 to 367) |
| `bc7f33c` | 3 | `tools/release-rehearse.sh`, `.planning/ops/RELEASE-OPS.md`, one bullet in `docs/packaging.md` |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md |

## What was built

- **Version check (T-08-05).** `sh tools/check/release-assets.sh version <tag> <ivs.h> <config-version-file>` reads the three `IVS_API_VERSION_*` defines (a trailing `u` accepted, exactly one line each) and the one `set(PACKAGE_VERSION "X.Y.Z")` line of the installed `ivsConfigVersion.cmake`. The tag must be `v<digits>.<digits>.<digits>` (`release-version-tag-form`, and the tag is then not compared); tag and header, and header and package, must be equal (`release-version-skew: tag 0.1.1 differs from header 0.1.0`, `release-version-skew: header 0.1.0 differs from package 0.1.1`); an unreadable or unparsable input is `release-version-unreadable`. `tools/release.sh` calls it on the staged install prefix before the install archive is written; `--tag v0.2.0` on this tree now exits 1 at that step (verified, with `--no-gate`). The tag, when not given, is derived from the header, so a dry run of a clean tree always agrees.
- **Changelog mode and `--release`.** `changelog <version> <CHANGELOG.md>` requires `## [<version>]` or a line starting with `## [<version>] `. `tools/release.sh --release` (implied by `--publish`) runs it first, before the gate or any asset, and halts with `release halted: CHANGELOG.md has no section for 0.1.0` (exit 1). Today the file holds only the Unreleased section, so `--release` halts by design until plans 08-20 and 08-21 write `## [0.1.0]`.
- **Gate step `release-assets`: eleven cases, plus the drift check.** Five new library cases on synthetic inputs (`version-skew` checks both pairs, `version-tag-form`, `version-agree-pass`, `changelog-section-missing`, `changelog-section-pass`), the sixth listed case `readiness-selftest` (real inventory only; the toy tree has no readiness script) and the table check: the lines strictly between the two markers of `CHANGELOG.md` must equal the output of `claims-table.cmake` byte for byte (`release-changelog-claims-drift`, detail `markers` for a missing or misordered marker pair), and `release-changelog-missing` when `tools/release.sh` is tracked but `CHANGELOG.md` is not. A generator failure surfaces its own token (`release-claims-measured-hardware`, `release-claims-unreadable`). The control `release-assets-claims-drift` overlays a toy `CHANGELOG.md` whose typed row differs from the toy claim ledger.
- **`tools/check/claims-table.cmake`.** Pure cmake, `-DROOT` and an optional `-DCLAIMS`; prints `| Claim | Tier | Standing | Artifact |`, the separator, one row per claim in file order and the totals line `Claims: 24 in total, 23 provisional, 1 established; none at tier measured-hardware.`; no statement or note text is printed. Findings go to stdout with exit 1.
- **`tools/release-readiness.sh`.** Read-only (writes only into a mktemp directory, no git write), POSIX sh and dash clean. Sections A (commit, date, seven tree ids), F (the table, counts by tier), G (pinned install list, tracked-binary scan, fixture census, name check), J (`hygiene --tree`; with `--history` the findings compared by pair) and, with `--release`, row P4. It refuses a dirty tree with exit 2 unless `--allow-dirty`. `--selftest` prints `selftest 4/4`. `--hosted` is the only function that names the GitHub CLI (`grep -n '[^a-z]gh '` outside comments lists lines 170 to 182 only, all inside `hosted()`, lines 169 to 183) and was never run.
- **`tools/release-rehearse.sh`.** See the rehearsal results below.
- **`.planning/ops/RELEASE-OPS.md`.** Items (a) to (g): the `git.create_tag` edit as an orchestrator-only separate commit, stage 2 of REL-01, the tag on the artifact commit, the draft flow with the eight P-checks, the orchestrator-only reads, the first-release amendment, and the 0.1.0 section halt.

## Readiness output (local mode, `--allow-dirty`, on HEAD `db4c9b8`; the two planning files modified in the working tree made the tree dirty)

```
A Identity
- commit: db4c9b8daad45621b4253730150f9bb7bb059129
- commit date (UTC): 2026-11-14T07:05:00Z
- tree core: 3aa1e3edb065d4b51da0728fff5fab89a96a804e
- tree runner: b4f43b77b3f03769d3bd8547b4e94a943197bef0
- tree tools: 765178dc213f4c98518b431ce4a5a9f3f737d53d
- tree data: 823ac3ae93b6317483ee333cdbcfc780f0fe9edb
- tree docs: c4f38451ee6003337e0e90bb4f9c5a2485206dcb
- tree fixtures: 4acb158012848fc05582ca8f17130e775842dfd2
- tree tests: c0fb96bd5be5035ed5b299bcc3f06be845a667b4
F Claims
Claims: 24 in total, 23 provisional, 1 established; none at tier measured-hardware.
By tier: source-expectation=13 original-diagnostic=3 deterministic-repeatability=4 installed-host-behavior=3 playable-guest-progress=1
G Rights and package audit
- pinned install list (data/expected/pkg-install-files.ids): 10 files
- tracked binaries by name (git ls-files against tools/check/binary-ext.txt): 0
- fixture census (data/expected/fixtures.ids): 10 manifests, redistributable yes 7, no 2, unknown or other 1, missing 0
- name check (tracked paths holding the word Intellivision in any case): 0
J Hygiene
- tree scan (sh tools/hygiene.sh --tree), exit 0: -- hygiene pass mode=tree hygiene-files=2325
- history scan: not run (a release step: pass --history; it takes minutes)
```

The table has 24 rows. The `tree tools` and `tree data` ids change with every later commit that touches those directories; the figures above are those of the commit named, not of the release.

## Pinned history list (D-33)

`data/expected/hygiene-history-pinned.ids` holds six `<token> <10-character sha>` lines: `hygiene-history-home-path` for `eaa832e3d3`, `92c4dc7d37`, `b78af4fb9f`, `cccdfefdce`, `345c48dd7f`, and `hygiene-message-home-path` for `955c13fa02` (the reachable commit with the subject "docs(04): drop a scratch-path shape from the research text"; the Phase 4 and 7 records name `4ccc9bb`, which is on no branch). Each sha resolves with `git rev-parse --verify <sha>^{commit}` and satisfies `git merge-base --is-ancestor <sha> HEAD` (all six checked before the file was written; no `pinned-sha-unreachable`). The full `sh tools/hygiene.sh --history` scan was not run (it takes minutes and is the orchestrator's release step, plan 08-21). `git log --format=%s -- tools/check/hygiene-baseline.txt` has no `(08-` subject: the baseline file is untouched.

## Rehearsal (`sh tools/release-rehearse.sh --no-gate`, Darwin arm64, bsdtar)

```
rehearse: dry run 1 (sh tools/release.sh --no-gate)
  notes: CHANGELOG.md section [0.1.0] absent (a dry run does not need it)
  tag v0.1.0, publish=0, release-checks=0, gate=skipped
  install archive built with cmake-tar (members sorted, mtime of the commit)
rehearse: tar mode cmake-tar
rehearse: dry run 2 (sh tools/release.sh --no-gate)
rehearse: changelog-section: not required
rehearse: source-archive: equal (4548876 bytes)
install-archive: equal-except-ranlib-stamp (Darwin: only the time field of the first member header of lib/libivs_core.a differs, bytes 25 to 36; byte equality is decided on Linux)
rehearse: tag-glob: v[0-9]*.[0-9]*.[0-9]* matches v0.1.0 and v1.2.3, not v1.0, v0.1, vfoo, v, 1.0.0
(readiness sections A, F and G, then:) P4: not checked
rehearse: ok
```

- **Determinism, local.** The source archives are byte-equal (4548876 bytes). The install archives differ in exactly two bytes at offsets 33 and 34 of `lib/libivs_core.a`: the seconds digits of the time field of the first member header (the symbol-index member), which `cmake --install` restamps on Apple. Every other member is byte-equal. This is the host effect that 08-01 found; the rehearsal names it and accepts only it, on Darwin only.
- **`--no-gate --release`** halts as designed: `release-changelog-section-missing: 0.1.0`, `release halted: CHANGELOG.md has no section for 0.1.0`, `rehearse: FAIL: dry run 1 exited 1`, exit 1 (expected until plans 08-20 and 08-21 write the section; not a failure of this plan). With a probe section appended to `CHANGELOG.md` (restored byte for byte afterwards, never committed) the same command ends `P4: ok` and `rehearse: ok`.
- **Container rehearsal** (Ubuntu 24.04, `linux/amd64` emulated on this arm64 laptop, the full rehearsal with its gate run inside, a scratch commit made inside the container for the commit time): exit 0 (x86_64 emulated, gcc 13.3.0, cmake 3.28.3, GNU tar 1.35, mawk 1.3.4, `/bin/sh` is dash). Dry run 1 ran the gate inside the container (`gate=run`) and built the install archive with `gnu-tar`; dry run 2 reused it with `--no-gate`. `source-archive: equal (4551863 bytes)`, `install-archive: equal (62860 bytes)` (a byte-equal pair, so no Darwin exception is used on Linux), `tag-glob` accepted v0.1.0 and v1.2.3 and refused v1.0, v0.1, vfoo, v, 1.0.0 under dash and mawk, readiness sections A, F and G printed, `P4: not checked`, `rehearse: ok`, 0 `FAIL` lines. This is the byte-equality proof on the hosted platform that the laptop cannot give (assumption A6); the first hosted draft release remains the last proof on the real runner.

## Checks shown able to fail

- **Library selftest cases against mutated libraries** (copies outside the tree, the step run with `CHECK_DIR` pointing at the copy): a library whose skew comparisons never fire fails `version-skew` ("library exit 0, want 1"); a library that does not judge the tag form fails `version-tag-form`; a library whose changelog mode always succeeds fails `changelog-section-missing`; a library whose version mode always fails fails `version-agree-pass` (the positive case).
- **Readiness selftest against mutated copies** (a mini-root with the real pinned list): a claim generator that accepts a measured-hardware claim fails `claim-measured-hardware`; a comparison that never exits 1 fails `history-new-finding`; a comparison that matches by token only fails `history-new-finding` and `history-pinned-ok` (3 of the 6 lines no longer carry the disposition); a disabled dirty check fails `dirty-tree-refused`. The step case `readiness-selftest` reports `release-selftest-missing` when the script is absent and `release-selftest-readiness-selftest` when its last line is not `selftest 4/4`.
- **The drift check**: one changed cell of the CHANGELOG table gives `release-changelog-claims-drift: CHANGELOG.md`; a removed end marker gives the same token with the detail `markers`; `CHANGELOG.md` absent from the file list while `tools/release.sh` is present gives `release-changelog-missing: CHANGELOG.md`; the control `release-assets-claims-drift` fails on its own tree with the intended token. The guard exists where the control points (lesson 7): `grep -c release-changelog-claims-drift tools/check/steps/release-assets.sh` is 3.
- **The history plumbing** in a throwaway git repository with a stub `tools/hygiene.sh` (never the real history scan): the six pinned pairs print six `FAIL (accepted by recorded disposition, D-33)` lines, `history findings: 6 (accepted 6, new 0)`, exit 0 and no success wording; one more pair prints `NEW FINDING: hygiene-history-secret 0000000000`, `history findings: 7 (accepted 6, new 1)` and exit 1.
- **The rehearsal comparison** against a fake `release.sh` in a scratch root: identical archives give `install-archive: equal`; a changed install member gives `DIFFERENT` and exit 1; a changed library byte outside bytes 25 to 36 gives `DIFFERENT`; a changed library byte inside gives the named Darwin outcome; a differing source archive exits 1; a wide tag glob (`v*`) exits 1 with "matches v1.0".

## Deviations from Plan

**1. [Rule 1 - reality] The Darwin ranlib stamp in the rehearsal.** The plan makes `install-archive: DIFFERENT` a failure and verifies that the local rehearsal exits 0. On this laptop two back-to-back dry runs always differ in two bytes of `lib/libivs_core.a` (the restamped time field of its first member header; found in plan 08-01, reproduced here), so the plan's rule would fail on every Darwin run for a reason that does not exist on the hosted Linux job. The smallest sound deviation: a third outcome `install-archive: equal-except-ranlib-stamp`, accepted only when the host is Darwin, exactly one member (`lib/libivs_core.a`) differs, the two files have equal size and every differing byte lies in bytes 25 to 36. Everything else, and every difference on Linux, is `DIFFERENT` and fails. The container rehearsal is the byte-equality proof on the hosted platform.

**2. [Interpretation] The second dry run always uses `--no-gate`.** The plan says to run the dry run twice; running the gate twice would cost two gate runs for no extra information (the second run repeats the packaging of the same commit and builds). The first run follows the flags, the second is `--no-gate`.

**3. [Interpretation] `release.sh` reads the header macros once at the top.** The tag derivation used to read them only when no tag was given; `--release` needs the version for the CHANGELOG check before the tag step, so the read moved up and both uses share it (`hdr` is now defined on every path).

**4. [Addition] `docs/packaging.md` also fixes one stale sentence and lists the new scripts.** "What the gate step does not prove" said that a consumer check against a release archive belongs to a later phase; plan 08-01 made that check part of the release job, so the sentence now says the consumer is built against the install archive on the host that built it. A bullet names `tools/release-rehearse.sh` and `tools/release-readiness.sh`. The plan lists the file in `files_modified` but names it only in Task 1; the second edit rides in the Task 3 commit.

**5. [Information] Control and inventory shape.** The plan's wording "overlay" is the repository's `overlay/` directory (not the `MAKE` recipe); `release-assets-claims-drift` uses `CHECK`, `EXPECT` and `overlay/CHANGELOG.md`.

**6. [Information] Hosted-only and long actions not performed.** No `gh` call, no `--hosted`, no `--history` scan, no push, no tag; `.planning/config.json` and `.planning/state.json` were never staged (D-42). The `--history` plumbing was probed with a stub in a throwaway repository, as described above.

No auth gates; no package installs (T-08-SC); no Rule 4 question.

## Decisions Made

- The new library tokens are `release-version-skew`, `release-version-tag-form`, `release-version-unreadable`, `release-changelog-section-missing`, `release-changelog-unreadable`; the step adds `release-changelog-claims-drift` and `release-changelog-missing`.
- `tools/release-readiness.sh` exits 2 for a dirty tree, 1 for a failed check or a new history finding, 0 otherwise; the test hook `IVS_READINESS_ROOT` (not `ROOT`, which the gate exports) selects the repository.
- Row P4 prints exactly `P4: ok`, `P4: not checked` or `P4: FAIL release-changelog-section-missing` (then the library finding on the next line).
- The readiness selftest sets `GIT_CONFIG_GLOBAL=/dev/null`, `GIT_CONFIG_NOSYSTEM=1` and `git init --template=` and passes the identity and signing options with `-c` on every git call, so no global or system configuration is read.

## Cost (laptop, user plus sys cpu, three runs each, load from a container gate running at the same time)

| Item | cpu-s |
|------|-------|
| `release-assets` step as of 08-01 (5 cases), re-measured here | 0.46 |
| `release-assets` step now (11 cases, the claim table check, the readiness selftest) | 0.83 (0.31 user, 0.52 sys) |
| `release-readiness.sh --selftest` alone | 0.13 |
| `claims-table.cmake` on the real ledger | 0.02 |

The step runs four times in a gate (the real tree, its positive control and the two negative controls; the toy runs skip the readiness case), so the plan adds about 1.6 local cpu-s, about 2.7 hosted cpu-s at the factor 1.675, about +0.07 in the median `r`: a projection of about 7.9 on the 7.84 left by plan 08-01, under the line 8.3 and inside the phase projection. `.planning/CI-EFFICIENCY.md` was not edited by this plan (it is not in its file list); the figures are here for the orchestrator and for the checkpoint of plan 08-08. Gate-time total: 83 s under sh and dash (laptop, load 10 to 14).

## Open Items

- **Plans 08-20 and 08-21 (the second half)**: write the stage-1 draft `## [0.1.0]` section of `CHANGELOG.md` (date at release time, the claim table inside with its markers kept). Until then `--release` and `--publish` halt on `release-changelog-section-missing: 0.1.0` by design; the orchestrator checks `sh tools/release-rehearse.sh --release` before tagging.
- **Every later plan that changes `data/claims.json`** (08-08, the state claim, any later claim) regenerates the CHANGELOG table in the same commit: `cmake -DROOT=. -P tools/check/claims-table.cmake` between the markers.
- **Orchestrator**: the `git.create_tag` change in `.planning/config.json` (a separate commit, `.planning/ops/RELEASE-OPS.md` item a); the full `sh tools/release-readiness.sh --history` at the close (the six pinned pairs, nothing new); `--hosted` and `tools/ci-record.sh` (item e); the hosted rows after this push are the arbiter of the projected `r`.
- **Information**: the Darwin install archive cannot be byte-equal across two installs; the rehearsal says so by name. The first hosted draft release is the last proof of the GNU tar path on the real runner.
- Carried unchanged: REL-01 stays Pending until stage 2 (D-28); D10 and D11 not performed; no history rewrite.

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=37/37 controls=367/367 faults=222/222 ... ci-uses=2 ci-workflows=2 ... release-selftest=11/11 ... pkg-selftest=17/17 ... tracked=2330 ... tests-expected=4628 tests-executed=4624 tests-skipped=4 tests-missing=0 docs=51 claim-refs=16 (0 FAIL lines; gate-time total=83 s, gate-cpu user=139.25 sys=126.18, laptop under load)
dash tools/gate.sh:    the identical summary line (md5 d3b26903b01ee501194db4c151bc98bf for both shells; 0 FAIL lines; total=83 s)
sh tools/gate-linux.sh: exit 0 on the same tree (build/gate-linux.log ends gate: PASS; gcc 13.3.0, cmake 3.28.3, x86_64 emulated), steps=37/37 controls=367/367 faults=222/222 ci-uses=2 ci-workflows=2 release-selftest=11/11 pkg-selftest=17/17 tests-expected=4628 tests-executed=4624 tests-skipped=4 tests-missing=0 docs=51 claim-refs=16, 0 FAIL lines (gate-time total=409 s with a second emulated container running in parallel; gate-cpu user=1211.16 sys=76.86)
```

On the closing tree (this SUMMARY, STATE.md and ROADMAP.md added or edited; `tracked=2331`): sh (78 s) and dash (79 s) `gate: PASS` with the identical summary line (md5 a1478950eec9716baed3bdc4862af06c), 0 `FAIL` lines.

After Task 1 (before its commit): sh and dash `steps=37/37 controls=366/366 release-selftest=10/10`. After Task 2: sh and dash `controls=367/367 release-selftest=11/11`. Counts at plan start were `steps=37/37 controls=366/366 faults=222/222 tests-expected=4628 tests-executed=4624 tests-skipped=4 tests-missing=0 release-selftest=5/5`; at the end `controls=367/367 release-selftest=11/11`, everything else equal; no test id and no fault was added. `git ls-files '*.ivsc' '*.bin' '*.rom' '*.exec' '*.wav' '*.ppm' '*.vraw'` is empty.

## Known Stubs

None.

## Threat Flags

None new. T-08-05 (tag against header against package): the `version` mode, two failing cases and one positive case in the gate, run by `tools/release.sh` before the archive is written. T-08-06 (history list hiding a new finding): pairs, not tokens; NEW FINDING fails; selftest cases for both outcomes; the baseline file is untouched. T-08-07 (API rate limit): `gh` is named only inside `hosted()`, which no selftest or gate step calls. T-08-08 (`git.create_tag`): recorded as an orchestrator-only separate commit; the strict glob and the tag-form check refuse a v1.0-style tag. T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: every file listed under `key-files.created` exists (including the control directory files and `.planning/ops/RELEASE-OPS.md`); commits `fdb62c3`, `db4c9b8` and `bc7f33c` are ancestors of HEAD and `git rev-list --count d108fd5ce6744972cd1ec010e52e2aaef035a85c..HEAD` printed 3 before this file's commit; `.planning/config.json` and `.planning/state.json` are not staged; `git ls-files` lists no `.ivsc`, `.bin`, `.rom`, `.exec`, `.wav`, `.ppm` or `.vraw` file; the six pinned shas are ancestors of HEAD.
