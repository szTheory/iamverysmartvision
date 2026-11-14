# Release operations: the orchestrator-only actions (plan 08-02; D-27 to D-33, D-42)

Written by plan 08-02 for the orchestrator. An executor never performs anything on this page: it never calls `gh`, never pushes, never tags, and never stages `.planning/config.json` or `.planning/state.json` (D-31, D-42). Paths are repository-relative.

What an executor may do locally, and what each tool reads or writes:

- `sh tools/release.sh` (no `--publish`) is a dry run. It never calls `gh`, builds the two archives and `SHA256SUMS` under `build/release-assets` or the directory given with `--out`, and runs the installed consumer against the install archive.
- `sh tools/release-rehearse.sh [--no-gate] [--release]` runs that dry run twice and compares the archives (source archive byte-equal; install archive equal, or on Darwin equal except the ranlib stamp it names), checks the tag glob of `.github/workflows/release.yml`, and prints readiness sections A, F and G. It never calls `gh`.
- `sh tools/release-readiness.sh --allow-dirty` prints the computable sections of the close record (A identity, F claim table, G rights and package audit, J tree hygiene). `--history` runs the full history scan (minutes; a release step, run by the orchestrator at the close) and `--hosted` makes read-only `gh` calls (item e).

## (a) The git.create_tag footgun (D-31, D-42)

`.planning/config.json` carries `git.create_tag: true`. The milestone tool (`gsd-complete-milestone`) would create, and offer to push, a tag in the style of the milestone label (v1.0). That tag would fire the release workflow if its trigger were wide; the trigger is the strict glob `v[0-9]*.[0-9]*.[0-9]*` and `tools/release.sh` and the version check refuse a tag that is not `v<digits>.<digits>.<digits>` and equal to the header macros, but the tool must still not make the tag.

Action: set that one key to false in a separate, deliberate commit made by the orchestrator, with no other file in it. An executor never stages `.planning/config.json` or `.planning/state.json`; both are modified in the working tree during Phase 8 and stay uncommitted by executors.

## (b) Stage 2 of REL-01 (D-28)

REL-01 completes in two stages. Stage 1 is the close record in the phase pull request, with every stage-2 field a literal `<stage2>` placeholder. Stage 2: after the phase merge, a `gsd/records-m1-close` pull request records the push run on the merge commit (run id, head sha, event `push`, conclusion; the row appended to `.planning/ci-runs.tsv` by `tools/ci-record.sh`), the open pull request and issue counts with their dispositions, and the ruleset read, replacing every `<stage2>` placeholder. A one-line check in that pull request fails if a placeholder remains. REL-01 is marked complete only after stage 2.

## (c) Tag the artifact commit (D-29)

The tag goes on the Phase 8 merge commit on `main` (the artifact commit), not on the records commit. The tag is `v0.1.0`: strictly `v<digits>.<digits>.<digits>`, equal to the `IVS_API_VERSION` macros of `core/include/ivs/ivs.h` and to the installed package version (D-30). Apply a tag ruleset only from a green `main` (no update, no deletion), and record its id in `.planning/AUTONOMY.md` or write "not applied". An executor does not tag.

## (d) The draft release flow and the checks P1 to P8 (D-29)

The tag push starts `.github/workflows/release.yml`; its one job runs `sh tools/release.sh --publish`, which creates the release as a draft pre-release (`--draft --prerelease --verify-tag --latest=false`) with the source archive, the install archive and `SHA256SUMS`. The orchestrator then reads the draft with read-only `gh release view` calls: asset names, sizes, the content of `SHA256SUMS`, the notes body, the pre-release flag, and that no unexpected asset is present. It checks P1 to P8, then publishes the draft. Deleting a published release is a hard stop; a failing draft is deleted before publication and fixed.

- P1: `main` is green at the artifact commit C: the push run is `success` with head equal to C, and the local sh, dash and container gates pass on the same tree; evidence is the run id and the summary lines; if it fails, fix `main` first.
- P2: hygiene: `--tree` clean, secrets and emails clean, and `--history` findings equal the pinned six (`data/expected/hygiene-history-pinned.ids`), nothing new; any new finding blocks.
- P3: rights: the tracked-binary scan is empty, the install list equals the pinned ten files, the manifest census is unchanged or explained, and no EXEC or GROM byte or digest exists anywhere; any miss blocks.
- P4: version agreement: the tag equals the header macros equals the installed package version, and `CHANGELOG.md` has the section for that version (`sh tools/release-readiness.sh --release` prints the row); a miss blocks.
- P5: the notes carry the claim table rendered from `data/claims.json`, no claim word stands outside the allowed wording, and the text step scans `CHANGELOG.md`; a miss blocks.
- P6: the history disposition is recorded (accepted without a rewrite, pinned, owner may veto) as a delegated decision; record "ready, not published" and ask the owner only if the disposition is judged unsafe.
- P7: the release machinery is in place: `release.yml` passes the `ci-parity` step, the `release-assets` step passes in the gate and the rehearsal, and the tag ruleset is applied or recorded as not applied; publication is blocked until the workflow exists.
- P8: the draft is verified with `gh release view --json` (names, sizes, `SHA256SUMS` content, notes body, pre-release flag, no unexpected asset); if it fails, delete the draft and fix; do not publish.

If any P-check fails, the close record says `ready, not published: blocked by P<n>` with the evidence that would clear the blocker.

## (e) Orchestrator-only reads

`sh tools/release-readiness.sh --hosted` and `tools/ci-record.sh` are orchestrator-only: the GitHub API rate limit is shared by every agent, so an executor never polls. `--hosted` makes five read-only calls (run list on `main`, open pull requests, open issues, rulesets, release list); no selftest and no gate step calls it.

## (f) The first-release amendment

The first release is a draft pre-release created by the workflow and published by the orchestrator after the read-only checks, and the workflow trigger is the strict tag glob (D-29, D-31). This is implemented by plans 08-01 (`release.yml`, `tools/release.sh`, the `ci-parity` exemption table, the `release-assets` step) and 08-02 (the version check, the readiness generator, the rehearsal). `.planning/AUTONOMY.md` section 5 is untouched by both plans.

## (g) The 0.1.0 section of CHANGELOG.md

The `## [0.1.0]` section is the stage-1 draft written by plans 08-20 and 08-21. Until it exists, `sh tools/release.sh --release`, `--publish` and `sh tools/release-readiness.sh --release` halt on `release-changelog-section-missing: 0.1.0` by design, and the plain gate, the plain dry run and the plain rehearsal stay green. Before tagging, the orchestrator runs `sh tools/release-rehearse.sh --release` and expects it to finish with `rehearse: ok`.
