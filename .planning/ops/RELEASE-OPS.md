# Release operations: the orchestrator-only actions (plan 08-02; D-27 to D-33, D-42)

Written by plan 08-02 for the orchestrator. An executor never performs anything on this page: it never calls `gh`, never pushes, never tags, and never stages `.planning/config.json` or `.planning/state.json` (D-31, D-42). Paths are repository-relative.

What an executor may do locally, and what each tool reads or writes:

- `sh tools/release.sh` (no `--publish`) is a dry run. It never calls `gh`, builds the two archives and `SHA256SUMS` under `build/release-assets` or the directory given with `--out`, and runs the installed consumer against the install archive.
- `sh tools/release-rehearse.sh [--no-gate] [--release] [--source-route]` runs that dry run twice and compares the archives (source archive byte-equal; install archive equal, or on Darwin equal except the ranlib stamp it names), checks the tag glob of `.github/workflows/release.yml`, and prints readiness sections A, F and G. It never calls `gh`.
- `sh tools/release-readiness.sh --allow-dirty` prints the computable sections of the close record (A identity, F claim table, G rights and package audit, J tree hygiene). `--history` runs the full history scan (minutes; a release step, run by the orchestrator at the close) and `--hosted` makes read-only `gh` calls (item e).

## (a) The git.create_tag footgun (D-31, D-42)

`.planning/config.json` carries `git.create_tag: true`. The milestone tool (`gsd-complete-milestone`) would create, and offer to push, a tag in the style of the milestone label (v1.0). That tag would fire the release workflow if its trigger were wide; the trigger is the strict glob `v[0-9]*.[0-9]*.[0-9]*` and `tools/release.sh` and the version check refuse a tag that is not `v<digits>.<digits>.<digits>` and equal to the header macros, but the tool must still not make the tag.

Action: set that one key to false in a separate, deliberate commit made by the orchestrator, with no other file in it. An executor never stages `.planning/config.json` or `.planning/state.json`; both are modified in the working tree during Phase 8 and stay uncommitted by executors.

**Correction, plan 09-09 (2026-11-25), measured:** the key already reads `"create_tag": false` at `.planning/config.json:15`, set by `e0c88b4` (the records commit that closed stage 2 of REL-01, 2026-11-24) before the milestone tool was used. The two sentences above are kept as written (lesson 21, a correction to a frozen record is an appended note); item (a) was therefore a prose fix in this page and in the D-31 row of `.planning/PROJECT.md`, and no executor touched the key.

## (b) Stage 2 of REL-01 (D-28)

REL-01 completes in two stages. Stage 1 is the close record in the phase pull request, with every stage-2 field a literal `<stage2>` placeholder. Stage 2: after the phase merge, a `gsd/records-m1-close` pull request records the push run on the merge commit (run id, head sha, event `push`, conclusion; the row appended to `.planning/ci-runs.tsv` by `tools/ci-record.sh`), the open pull request and issue counts with their dispositions, and the ruleset read, replacing every `<stage2>` placeholder. A one-line check in that pull request fails if a placeholder remains. REL-01 is marked complete only after stage 2.

## (c) Tag the artifact commit (D-29)

The tag goes on the Phase 8 merge commit on `main` (the artifact commit), not on the records commit. The tag is `v0.1.0`: strictly `v<digits>.<digits>.<digits>`, equal to the `IVS_API_VERSION` macros of `core/include/ivs/ivs.h` and to the installed package version (D-30). Apply a tag ruleset only from a green `main` (no update, no deletion), and record its id in `.planning/AUTONOMY.md` or write "not applied". An executor does not tag.

## (d) The draft release flow and the checks P1 to P8 (D-29)

The tag push starts `.github/workflows/release.yml`; its one job runs `sh tools/release.sh --publish`, which creates the release as a draft pre-release (`--draft --prerelease --verify-tag --latest=false`) with the source archive, the install archive and `SHA256SUMS`. The orchestrator then reads the draft with read-only `gh release view` calls: asset names, sizes, the content of `SHA256SUMS`, the notes body, the pre-release flag, and that no unexpected asset is present. It checks P1 to P8, then publishes the draft. Deleting a published release is a hard stop; a failing draft is deleted before publication and fixed.

- P1: `main` is green at the artifact commit C: the push run is `success` with head equal to C, and the local sh, dash and container gates pass on the same tree; evidence is the run id and the summary lines; if it fails, fix `main` first.
- P2: hygiene: `--tree` clean, secrets and emails clean, and `--history` findings equal the pinned six (`data/expected/hygiene-history-pinned.ids`), nothing new; any new finding blocks.
- P3: rights: the tracked-binary scan is empty, the install list equals the pinned eleven files (`data/expected/pkg-install-files.ids`, measured at 11 non-comment lines by plan 09-09; the earlier wording said "ten" and was stale), the manifest census is unchanged or explained, and no EXEC or GROM byte or digest exists anywhere; any miss blocks.
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

The `## [0.1.0]` section is the stage-1 draft written by plans 08-20 and 08-21. Until it exists, `sh tools/release.sh --release`, `--publish` and `sh tools/release-readiness.sh --release` halt on `release-changelog-section-missing: 0.1.0` by design, and the plain gate, the plain dry run and the plain rehearsal stay green. Before tagging, the orchestrator runs `sh tools/release-rehearse.sh --release --source-route` and expects it to finish with `rehearse: ok`, having printed `rehearse: source-route: <archive>, golden match (report <n> lines, <m> of <m> compared), prefix <prefix>`: that mode discharges PKG-05, because it is the only run that unpacks the source archive a release carries, builds it cold and compares its report with `data/golden/demo-lamp-tone-pass.digests`; a release that skips it records the mode as `not run`, never as a pass.

## (h) The stage 2 check and the order of the orchestrator's actions (plan 08-21)

The close record `.planning/phases/08-stic-completion-save-state-performance-baselines/08-MILESTONE-CLOSE.md` holds every fact that exists only after the merge as a literal placeholder: the word stage2 inside angle brackets. The records pull request proves that none remains with the command below, run from the repository root; it prints each remaining line and exits 1, and it exits 0 when the record is complete. If the milestone tool has archived the phase directory, use the path of the record under `.planning/milestones/`. The explanatory text of the record never writes the literal, so the grep finds only real placeholders.

```sh
f=.planning/phases/08-stic-completion-save-state-performance-baselines/08-MILESTONE-CLOSE.md
if grep -n '<stage2>' "$f"; then echo "stage2 placeholders remain"; exit 1; fi
```

The order of the orchestrator's actions, as the record needs them. None is an executor action, and at the Phase 8 close none was started because the owner paused the flow after Phase 8 (relayed by the orchestrator).

1. Merge the Phase 8 pull request with green CI. The merge commit is the artifact commit of section A.
2. Record the push run on the merge commit with `tools/ci-record.sh` (an `event=push` row of `.planning/ci-runs.tsv`); this gives the lines of section B.
3. Open the `gsd/records-m1-close` branch and make the `git.create_tag` change of item a as its first commit, alone, before the milestone tool is used. Done: `e0c88b4` made it, and the key reads `false` (plan 09-09, measured 2026-11-25); this step is kept for the record of the order, not as work still to do.
4. Read GitHub once with `sh tools/release-readiness.sh --hosted` (five read-only calls); this gives the counts and dispositions of section C and the ruleset read, and the immutable-release setting of hand-off H8-6 is read at the same time.
5. Tag `v0.1.0` on the artifact commit, let the workflow create the draft, read it with `gh release view --json`, and check P1 to P8 (item d); publish only if all hold, otherwise the decision line reads `READY, NOT PUBLISHED: blocked by P<n>` with the evidence that clears it.
6. Fill every placeholder of the record (sections A, B, C, I and K), run the check above until it prints nothing, and run `sh tools/gate.sh` on the records branch.
7. Mark REL-01 complete in `.planning/REQUIREMENTS.md` (the checkbox and the traceability row, with a note that stage 2 is recorded), record the release in `.planning/STATE.md` (`.planning/AUTONOMY.md` section 5, step 6) and write the CI-efficiency entry for the release workflow.
8. Merge the records pull request; then run the milestone audit, the milestone completion on a records branch and the cleanup (`08-DISCUSS-release.md` section 5).

## (i) The release-decision line for v0.4.0 (plan 15-02; `GUARDRAILS-v0.3` V6 still binds where cited; recomputed by plan 17-02 against `GUARDRAILS-v0.4` section 7.4)

The milestone close carries exactly one release-decision line, in this wording:

> the release ships the static package and the source archive, which carries the source of the build-only tools and of the build-only host shim; no shared object, no built host shim

The line above **replaces** the v0.3.0 wording, which was superseded on 2026-11-26 by plan 17-02 (EV-08): it
adds the relative clause saying the source archive carries the source of the build-only tools and of the
build-only host shim, and it says `no built host shim` rather than `no host shim`. The superseded v0.3.0
wording, superseded 2026-11-26, is quoted here instead of being deleted, because the record of what was said
then is append-only:

> the release ships the static package and the source archive; no shared object, no host shim, no content

It was corrected because it denied a host shim without saying that the source archive carries its source, so a
reader could conclude that the shim's source is withheld too. The inspection-limb clause of the guardrails' close
line is not written here: the tool it names does not exist until Phase 18, and Phase 20 recomputes the line with
it.

Three mechanisms hold that line, and each is a committed check rather than a promise:

1. `tools/check/binary-ext.txt` lists `so`, `dylib`, `dll` and `bundle`, so a shared library is binary by
   name to the `files` step (the tree), the `release-assets` member check (an archive) and the
   binary-by-name hygiene rule. Two controls plant a tracked `.so` and a tracked `.dylib` and must be
   refused by name: `files-unlisted-binary-so` and `files-unlisted-binary-dylib`.
2. The `release-assets` case `so-member` builds a synthetic archive whose own list names
   `lib/libivs_retro.so` and requires `release-assets-binary-member: lib/libivs_retro.so`, with no
   extra-member or missing-member finding: the binary rule does not consult the list, so a list that
   named the object could not smuggle it in.
3. The `pkg-install` selftest `host-so-absent` plants `lib/libivs_retro.so` in a copy of the installed
   prefix and requires `pkg-inventory-extra: lib/libivs_retro.so`, so a future install rule on the host
   target fails there first. The eleven-path inventory (`data/expected/pkg-install-files.ids`) is what
   makes the absence a check; `prefix-files=11` in the gate line is the same statement at every commit.

`docs/packaging.md` states the non-membership for a reader (the build-only paragraph and the release-content
bullet), and no claim cites the object: the shim is never installed, so the installed-host tier's own
definition does not apply (GUARDRAILS-v0.3 section 3, the Phase 15 row). Appended, never a rewrite of an
existing section (section 2.10 of the guardrails).

### (i.1) The line recomputed at plan 16-04, with the committed-direction probe (PKG-06, section 2.10)

Appended by plan 16-04. The line and its three mechanisms above are unchanged; this note adds the
measurement, and says where an earlier sentence is narrower than the evidence rather than rewriting it.

The commit: `4a1e993`, the tree plan 16-04's Task 1 started from. Task 1 changes only this record, so the gate
line below is also the line at the commit that carries this note.

The line, quoted exactly and unchanged in this phase: the release ships the static package and the source archive; no shared object, no host shim, no content.

The values, read from the closing gate line at that commit rather than quoted from a record:

- `binaries-listed=0` — the `files` step's own count of name-listed binaries in the tree: none is committed.
- `prefix-files=11` — the installed package is exactly the eleven paths of
  `data/expected/pkg-install-files.ids`.
- `pkg-selftest=20/20` — all twenty in-step selftests of the `pkg-install` step pass, including
  `host-so-absent`.
- `release-selftest=12/12` — all twelve cases of the `release-assets` step pass, including `so-member`.
- `controls=404/404` — every control passes, including `files-unlisted-binary-so` and
  `files-unlisted-binary-dylib`.

The probe. Both files hold the same ASCII text; only the extension differs, and both are staged with the
force flag so the list the step reads is the list git would ship:

```sh
printf 'probe\n' > probe.so
printf 'probe\n' > probe.txt
git add -f probe.so probe.txt
git ls-files > /tmp/g16-so-fl          # the file list in the shape the gate builds it
ROOT=$PWD CHECK_DIR=$PWD/tools/check FILELIST=/tmp/g16-so-fl sh tools/check/steps/files.sh
```

Its output, in full:

```
-- FAIL files-unlisted-binary: probe.so
```

`probe.txt` is named nowhere in that output, and the step prints no pass line — so the step is not one that
refuses everything. The content scan over the same list (`binscan.sh binary`) names neither path: it skips
`probe.so` by name, because `so` is in `tools/check/binary-ext.txt`, and `probe.txt` is clean text. Two facts
make this the **committed direction** rather than a copy of a tree: the path was tracked (`git ls-files`
listed `probe.so` before the step ran), and the content scan skips a name-listed extension, so the refusal
above is the **name** and not a byte scan. Both files were unstaged and removed in the same task:
`git ls-files` holds no path with any of the four shared-object extensions, `git status --porcelain` is
empty, and `sh tools/gate.sh` is green afterwards.

Three surfaces, three pieces of evidence, and no one of them covers another:

1. **The tree (committed paths)** — this probe, on the real tree with a tracked path: one finding for the
   shared-object name, none for the same bytes under an ordinary extension. The two
   `files-unlisted-binary-*` controls prove the same rule on a copy of the toy tree; the probe is what
   proves it on a path somebody actually committed, which is the limb Phase 15 could not reach from a
   control.
2. **The archive** — the `release-assets` case `so-member`, a synthetic archive whose own list names
   `lib/libivs_retro.so`, refused because the binary rule does not consult the list.
3. **The installed prefix** — the `pkg-install` selftest `host-so-absent`, an object planted in a copy of
   the installed prefix and refused as `pkg-inventory-extra`, with the eleven-path inventory making the
   absence a check.

The probe is a throwaway check: it is not a gate step, CI never runs it, no test id, control, fault or
golden was added for it, and it left the tree clean. Re-running it is a copy of the four commands above.

## (j) The owner route, rehearsed from the source archive (plan 16-02, PKG-06, GUARDRAILS-v0.3 V10 and section 2.10)

The route is the three commands of `docs/owner-images.md` ("The route from the source archive, in three
commands"), and this section is its transcript. It is a release-time record: no gate step runs it, CI never
runs it, no test id, control, fault or golden was added for it, and a release that skips this mode records the
mode as `not run`, never as a pass.

The command, and the commit it ran on:

```sh
sh tools/release-rehearse.sh --release --source-route    # on 14f81ed
```

The archive, its size, and the prefix the run read from the archive's first member:

- archive `ivs-v0.2.0-source.tar.gz`, 6,759,881 bytes, byte-equal to the second dry run's copy;
- extraction prefix `ivs-v0.2.0`.

The cold configure (`cmake --preset release`) and build (`cmake --build --preset release`) in the extracted
prefix completed. The log holds no `rehearse: FAIL:` line, and the golden comparison below is reached only
after that build succeeds, because a non-zero status at either step ends the rehearsal.

The rehearsal's own lines, quoted:

```
rehearse: source-route: ivs-v0.2.0-source.tar.gz, golden match (report 131 lines, 124 of 124 compared), prefix ivs-v0.2.0
rehearse: owner-route: wrap base=4000 word-order=hi-first digest=8d73b98c58bb75a6515c6a791ebd2925265d05874e324c31a5836e61055b74f5
rehearse: owner-route: run exit=10 outcome=guest-fail exec-label=- cart-digest-matched=yes
rehearse: owner-route: mutation exit=14 status=IVS_E_DIGEST_MISMATCH run-state=unreset
```

The wrap is the archive's own `build/release/tools/flatwrap/ivs_flatwrap` at base `4000` with the default
`hi-first` word order, and the digest it printed is the container's, matched against the run's own
`slot.cart.sha256`. The run is the archive's own `tools/owner-run.sh --record --cart` on the three files, and
the mutation run is the same command without `--record`, because a second `--record` is refused with exit 2
(`refs/owner-images.sha256 exists: remove it to record again`).

The three things a reader must not have to infer:

- The EXEC, GROM and flat images are **project-made zero-filled files** (8,192, 2,048 and 8,192 bytes) that the
  rehearsal writes with `dd` from the zero device inside the extraction's `refs/`; no owner file was read and no
  byte of one was written anywhere. D11 stays `not-performed`.
- The run's values (`guest-fail`, exit 10, `exec: -`) are a statement about these project-made bytes and about
  nothing else. No claim cites this record, and no phase exit criterion reads it as evidence.
- A release that skips this mode records it as `not run`, never as a pass (PLAN-LESSONS 24).

After the run, the rehearsal's workspace (one `mktemp` directory, removed by its `EXIT` trap) was gone and
`git status --porcelain` was empty, so the route wrote nothing into the repository.
