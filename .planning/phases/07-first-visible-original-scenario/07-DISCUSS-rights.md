# Phase 7 discuss, decision point S-5: rights, manifest and SCN-02

Research agent E. Read-only on the repository except this file. Labels: [OBS] read or measured this session, [INF] inference, [REC] recommendation, [UNK] unknown, [BG] background not re-fetched. Nothing here is a statement about real hardware. No third-party source was opened for this note; every [OBS] is a read of this repository.

## 0. Recommendation on one screen

1. **S5-1 Manifest shape.** One new manifest `FX-demo-<slug>-1` for the program: `kind: original-source`, `committed: true`, outputs = the one listing (plus the program source file if D4 changes), `licence: MIT` with the same `LICENSE` evidence record the three diagnostics use, `redistributable: yes`, `allowed_use: [local-test, ci, repo, release]`, `rights_review: author-attested` by `project contributors`. The first project fixture with a **non-null `recipe`**: success criterion 1 asks for one, and the existing diagnostics all use `recipe: null` plus prose.
2. **S5-2 Make the recipe checkable.** Extend the `manifests` step (about 20 lines of CMake, one control) so every `recipe.sources[]` entry must exist and match its digest (`manifest-recipe-source-mismatch`). Today the checker validates the digest format of recipe sources and `tool_sha256` but never hashes them [OBS: `tools/check/manifests.cmake`], so a non-null recipe would otherwise be an unchecked claim that rots on the next listing edit.
3. **S5-3 Licence.** MIT, no owner gate. D9 recorded MIT as an owner instruction and `LICENSE` exists [OBS]. The scattered "licence selection is an owner gate" lines in PROJECT.md are stale (Out of Scope, the 2026-11-09 seed row, open question 11) and should be tidied at phase transition, not re-asked. Add a short, checkable third-party-content attestation (no copied glyphs, no recognisable tune, no EXEC call, no GROM read) to the program's derivation document and the manifest note.
4. **S5-4 Naming.** Descriptive, brand-free ids on the existing pattern: fixture `FX-demo-<slug>-1`, program id `ivs-demo-<slug>-1`, scenario `demo-<slug>-pass`. No on-screen text, no title string, no game-like name. Exclusion list in section S5-4. A 10-line `names` rule in the `files` step makes the "Intellivision in a name" hard stop machine-checked (nothing checks it today [OBS: grep of `tools/check`]).
5. **S5-5 Owner-local route, content.** One new page `docs/owner-images.md`. Inputs are two files the owner supplies (EXEC 8192 bytes, GROM 2048 bytes, the shapes the core already checks) kept in the gitignored `refs/` cache. Identity is three layers: byte shape (core), a local owner-written digest lock, and the core's own `expect-sha256` check at load. The project records **no** known-good EXEC or GROM digest in Phase 7.
6. **S5-5b Label in the manifest system.** Two uncommitted manifests `FX-owner-exec` and `FX-owner-grom` (`kind: system-image`, `redistributable: no`, `committed: false`, `allowed_use: [local-test]`, `local_path` under `refs/`). This puts the "non-redistributable" label where the gate reads it, and the existing control `manifests-unknown-committed` already proves a flip to `committed: true` fails.
7. **S5-6 Runner and helper.** A tracked template scenario `data/scenarios/owner-exec-title.ivss` (`content exec user`, `content grom user`, no cartridge, `run-to <n> frames`, no assert that names a result), plus a small operator helper `tools/owner-run.sh` modelled on `tools/listen-demo.sh`: refuses in CI, refuses paths inside the work tree but outside `refs/`, checks sizes, enforces the digest lock, injects `expect-sha256` lines into a temp copy of the template, prints a banner. Harden one gap in the runner: `--audio-raw`, `--psg-log-out` and `--audio-wav` are **not** refused for `user` content today, while video and golden outputs are [OBS: `runner/main.c`]; refuse them too (3 lines and one process test).
8. **S5-7 SKIPPED visibility.** Add `owner-exec-title` as a second `skip-ok` scenario (`missing-fixture`, exit 13, `# expect-detail: missing role=exec user`), which raises `tests-skipped` from 2 to 4 (one id, two presets) and is counted exactly by `tests-inventory`. Make it **visible**: today the per-id skip exists only in TAP files under `build/gate/` and as the number `tests-skipped=N` in the summary [OBS]; have `tests-inventory` print one `-- SKIPPED <preset>/<suite>/<id>: <reason>` line per skipped id.
9. **S5-8 Out of CI, out of repo.** Enforce by construction (the gate never passes `--exec`, no secrets, no cache, no artifact upload, D12) plus two cheap new guards: every scenario with a `content <role> user` line must be `skip-ok` in `data/expected/runner-scenarios.ids` and the reverse; the helper refuses when `CI` or `GITHUB_ACTIONS` is set.
10. **S5-9 D11 wording.** SCN-02 is complete when the route is documented and the gate-checked refusal and SKIPPED line exist; D11 stays open and "not performed" (precedent: Phase 5 and Phase 6 close-out notes for D10). No claim-ledger entry, no evidence tier, no result for any real EXEC, GROM or commercial content. Proposed text in S5-9. Footgun: the `text` step rejects the words supports, compatible, passes, accurate, verified and runs in `docs/` and `README.md` unless a line carries a claim tag or `[nc]`.
11. **S5-10 Releases.** The program ships only as tracked source inside the source tarball (listing, manifest, scenario, goldens, derivation). Built `.ivsc` images are not release assets under the recorded release design (untracked and binary-extension assets are refused) and the runner and `ivs_pack` are not installed, so a release cannot be "downloaded and run" yet. Say that plainly in the notes; do not change the allowlist in Phase 7.
12. **S5-11 Hygiene.** Existing gate steps already cover tracked binaries, `refs/`, secrets and paths. Add only: the recipe-digest check, the user-content-skip rule, the `names` rule, the audio-output refusal, and a rights line in the program's derivation. Fix the stale README ("Phase 1 foundation only, no machine code, no software release") before the first release.

## 1. Facts the recommendations rest on

- [OBS] `LICENSE` is MIT, `shasum -a 256` of it equals the `licence_evidence.text_sha256` in the four committed fixture manifests (`aeb83b57...c26`). README, `docs/RIGHTS.md` and D9 say MIT and a public repository.
- [OBS] D9 (PROJECT.md Decision Register): "MIT licence; public GitHub repo ... after hygiene check; keep working name; never 'Intellivision' in product/package name; README non-affiliation notice", source "Owner instruction". ROADMAP lists D9 as "Delegated decision 2026-11-09".
- [OBS] AUTONOMY.md section 1: releases, public publication and frequent 0.x releases are delegated; hard stops are third-party rights-bearing material, PII and paths, D10 and D11 runs, destructive history actions, the trademark name, paid services. AGENTS.md says AUTONOMY.md takes precedence over its own stop rule.
- [OBS] ROADMAP (REL-01 text and the intro) still says publication "remains an owner gate (D9)". AUTONOMY.md precedence resolves that conflict in favour of the delegation; it is stale text, not a live gate.
- [OBS] Manifest schema v1 (`docs/fixture-manifest.md`): all keys required, `recipe` is an object or `null`, effective `yes` needs evidence, a review status other than `unreviewed`, and effective-yes `derived_from`. `kind` already offers `original-source`, `original-built` and `system-image`. A committed manifest must list outputs that exist with matching size and SHA-256. Every file inside a fixture directory must be listed by its manifest (`files` step).
- [OBS] `tools/check/manifests.cmake` hashes `outputs` only. `recipe.sources[].sha256` and `recipe.tool_sha256` are checked for format only (64 hex).
- [OBS] The three existing diagnostics and the boot stub are `kind: original-source`, `recipe: null`, `licence: MIT`, `author-attested`, `holder: project contributors`, and put the recipe in the `note` ("tools/pack builds ... no container is committed").
- [OBS] `.gitignore` ignores `/refs/` and the extensions `*.bin *.rom *.itv *.cfg *.cc3 *.ivsc *.int *.pdf *.png *.wav *.ppm *.vraw`. It does not ignore `.exec` or `.img`. `tools/check/binary-ext.txt` is the list the `files` step reads.
- [OBS] `docs/RIGHTS.md` allows "a single identity hash of a system image, recorded as a fact" and forbids bulk hash databases from other projects.
- [OBS] The core already checks image shape only (profile `system_images.exec_grom` note: "Identity is a recorded fact: the core checks shape only (D-27)"), and loads with an optional `expect_sha256` that yields `DIGEST_MISMATCH` (`core/src/machine/machine_load.c`, step 9). The scenario directive `expect-sha256 <role> <64 hex>` exposes it.
- [OBS] `ivs-run` takes `--exec`, `--grom`, `--cart` for `content <role> user` (`docs/runner.md`). It refuses `--update-golden`, `--frame-ppm` and `--video-raw` for a scenario that names user content; `tools/listen-demo.sh` refuses such a scenario for WAV export. The runner itself does not refuse `--audio-raw`, `--psg-log-out`, `--audio-wav` for user content [OBS: `runner/main.c` lines around 697 to 712 guard only frame, video and golden].
- [OBS] `data/scenarios/needs-user-exec.ivss` is the existing user-content scenario; `data/expected/runner-scenarios.ids` marks it `skip-ok`; `data/expected/tests/runner-process.ids` marks `runner-process.scenario.needs-user-exec skip-ok`; the TAP line is `ok 7 runner-process.scenario.needs-user-exec # SKIP needs-user-exec: needs operator content (ended missing-fixture, exit 13, as expected)` in `build/gate/<preset>/runner-process.txt`.
- [OBS] A prior gate summary line (a local log of the Phase 6 run) shows `tests-expected=3814 tests-executed=3812 tests-skipped=2 manifests=5 effective-yes=4 effective-no=1`. `tests-inventory` fails with `tests-unexpected-skip` for any skip not marked `skip-ok`, so counting is exact.
- [OBS] `tools/check/ci-summary.sh` renders a "Skips and missing" table in the hosted job summary from every `gate:` token whose key contains skipped, missing, pending or unknown. So the count is visible in the PR check today; the id is not.
- [OBS] `.github/workflows/gate.yml`: one job, `permissions: contents: read`, no cache, no artifact step, no secrets, `sh tools/gate.sh`. No `.gitattributes`, so `git archive` exports every tracked file including `.planning/`. No git tag exists and the README says there is no release.
- [OBS] The `text` step scans `README.md` and `docs/**/*.md` for the words `supports|compatible|passes|accurate|verified|runs` and requires `[claim:CL-NNNN]` or `[nc]` on the same line. The `denylist` step bans emulator project names outside `[cite-only]` lines in docs and data.
- [OBS] The boot-stub anti-HLE guard (`listing` step): at most 16 non-zero words in `1000-101F`, only rows from `bootstub-rows.ids`. The project's EXEC substitute must stay a stub.

## S5-1 Fixture manifest shape for the redistributable original program

**Question.** What record makes the program "redistributable: yes" in the gate's sense, and what does it list?

**Options.**

A. One manifest, `kind: original-source`, committed listing as the only output, recipe `null`, recipe in `note` (the pattern of all four existing fixtures). Zero new machinery. Does not meet the letter of criterion 1 ("recipe ... in the fixture manifest").

B. **One manifest, same kind and outputs, with a filled `recipe` object.** `recipe.tool` = `ivs_pack (in-tree, tools/pack/pack.c)`, `tool_version` = `in-tree at the commit that carries this manifest`, `tool_sha256` = SHA-256 of `tools/pack/pack.c`, `tool_licence` = `MIT`, `command` = `ivs_pack --ivsc <listing> build/<preset>/images/<id>.ivsc`, `sources` = the listing and `tools/pack/pack.c` with digests. Output bytes stay uncommitted.

C. Two manifests: `FX-demo-<slug>-1` (source) and `FX-demo-<slug>-1-image` (`original-built`, `committed: false`, outputs naming the build-tree `.ivsc` with its digest, `derived_from` the first). Gives a reviewer one place to read the image digest. Cost: the digest of an uncommitted output is not checked by any step (only "must not be tracked"), so it is an unchecked claim; the image digest is already pinned by the `digest.images` line of the golden and `load.cart sha256=` in `data/expected/pkg-consumer.golden` [OBS: that golden prints the diagnostic's image digest].

D. Commit the built `.ivsc` with `kind: original-built`. Needs a manifest output with a binary extension and an effective-yes manifest to pass `files`. Rejected: it changes D4/D-30 ("no container is committed"), duplicates what the listing already says, and a committed binary is a standing hygiene risk for no gain.

**Trade-offs.** B is the smallest option that satisfies the criterion. C adds a second place where a digest can go stale. D adds a binary to a public repo. A leaves criterion 1 arguable; a verifier reading "recipe" in the success criterion would mark it partial.

**Recommendation [REC]: B, plus S5-2.** Concretely:

- `id`: `FX-demo-<slug>-1`; the directory holds the listing (and the program source file if decision point C chooses an assembler) and `manifest.json` only. Add the id to `data/expected/fixtures.ids`.
- `source.origin`: "project-authored program ivs-demo-<slug>-1 (Phase 7)"; `pin` and `accessed` null; `holder`: `project contributors`.
- `outputs`: listing path, size, SHA-256 (verified by the existing checker). If D4 changes to a mini-assembler, outputs list the assembler source file and the recipe command names the assembler; nothing else in this record changes.
- `licence`: `MIT`; `licence_evidence`: `{url: "LICENSE", text_sha256: <same>, accessed: <date>}`; `copyright`: the one project line; `notices`: `[]` (nothing third-party travels with it, and that is a property to keep: any later notice means third-party material has entered).
- `redistributable: yes`; `allowed_use`: `local-test`, `ci`, `repo`, `release`. Adding `release` is the first time a project fixture claims it. It requires effective `yes` (`manifest-committed-not-redistributable` otherwise) and it is the value the recorded release design reads for its asset allowlist [OBS: AUTONOMY.md section 5, "fixture manifests with `redistributable: yes`"].
- `rights_review`: `author-attested`, `by: project contributors`, dated at commit. `owner-confirmed` is not available without an owner action, and AUTONOMY.md says not to ask. [INF] the attestation is true only if the S5-3 checklist is true; write the checklist into the note and the derivation document so the attestation names what it attests.
- `derived_from`: `[]`.
- `note`: states the program is project-authored, hand-derived from cited pages and the derivation document, uses the project boot stub only, calls no EXEC routine, reads no GROM, contains no text and no tune taken from any source, and that built images exist only in the build tree with identity pinned by the golden and `data/expected/pkg-consumer.golden`. Mirror the wording of `FX-diag-stic-1`.

**Reversal evidence.** A reviewer or planner decides the recipe field is not worth a checker change: fall back to A with the full recipe prose in `note` and say in the verification record that criterion 1 is met by documented recipe, not by a machine-checked one. Reverse to C if a consumer (a release script) needs the image digest from the manifest rather than from the golden.

**Risks.** The recipe object exposes `tool_version` as free text; "in-tree at the commit" is true by construction for a commit-pinned release but unfalsifiable by the checker. Acceptable because the release is the exact tested commit (AUTONOMY section 5).

## S5-2 Verify the recipe (small checker extension)

**Question.** Should the `manifests` step hash `recipe.sources[]`?

**Options.** (a) No: leave the recipe as documentation. (b) Yes: for each recipe source, the path must exist, be tracked, and hash to the recorded digest. (c) Also require `tool_sha256` to equal the digest of a listed source when `tool_version` starts with `in-tree`.

**Trade-offs.** (a) keeps the gate unchanged but a manual-refresh rule is the only guard; the project already learned that manual digest refresh is a recurring slip class (the manifest-outputs helper exists because of it [OBS: `tools/cart/manifest-outputs.sh`, "RESEARCH Pitfall 9"]). (b) is about 20 lines mirroring the existing output loop, one new token, one control directory, and one line in the manifest inventory expectations. (c) is more clever than useful.

**Recommendation [REC]: (b).** Token `manifest-recipe-source-mismatch` (missing file and digest mismatch share it, like the output tokens' two forms if the planner prefers two tokens). Add the control `manifests-recipe-source-digest` (edit the listing, keep the old recipe digest; must fail with that token). List `tools/pack/pack.c` as a recipe source so `tool_sha256` cannot drift silently. Side effect to accept: any edit to `tools/pack/pack.c` forces a refresh of this one manifest. If the D4 decision adds a data directive to `pack.c` in Phase 7, the refresh happens in the same commit anyway.

**Reversal evidence.** If a second and third recipe-bearing manifest appear and the pack.c coupling causes churn, drop `pack.c` from `sources` and keep `tool_sha256` informational.

**Risks.** The check passes on a stale `tool_sha256` field that is not a source. Mitigated by setting the field to the same digest as the listed pack.c source in review.

## S5-3 Project licence and ownership of the program

**Question.** Is the licence choice an owner gate here, and what should the program carry?

**Findings.** [OBS] The licence is decided (D9, MIT, owner instruction) and applied: `LICENSE`, README, RIGHTS.md, four manifests. [OBS] Stale text still lists licence selection as an owner gate (PROJECT.md Out of Scope bullet "Public release, license selection ...", the seed row marked "superseded in part", open question 11 "new-code license") and REL-01 still says publication remains gated. AUTONOMY.md supersedes. [REC] Record in the Phase 7 transition note: licence is decided (D9); strike or annotate those lines; do not re-ask.

**Options for the program's licence.** MIT (as everything else); CC0 or dual licence for the program and its art; a separate "demo" licence. **Recommendation [REC]: MIT, no separate licence.** One licence keeps the manifest evidence record identical, keeps the release tarball under one `LICENSE`, and avoids a licence-composition question for contributors. Revisit only if a third party contributes art under another licence (then `notices` and a per-file licence appear).

**Ownership and authorship.** [OBS] Copyright holder in manifests is `project contributors`; AI-assisted commits carry a co-author trailer (`docs/provenance.md`). [INF] Whether machine-assisted material is copyrightable varies by jurisdiction; either way the redistribution position is safe (MIT grant, or no restriction). No action; do not add claims about copyrightability.

**Third-party content hazards specific to a program that shows pictures and makes sound** (this is where a rights slip would actually happen; the diagnostics avoided all of it):

1. **Glyphs and letters.** A font drawn to match another system's character set is a derivative risk; the GROM font is rights-bearing [OBS: AGENTS.md lists GROM as rights-bearing]. The program should show geometric shapes only (as `ivs-diag-stic-1` does: "project-authored geometric shapes" [OBS]). If text is ever wanted, derive each glyph from the derivation document's own grid, not by recalling a known set.
2. **Tunes.** A recognisable melody is a copyrighted work independent of the code that plays it. Use a scale, an arpeggio or a pitch ramp tied to input; no known tune, jingle or sound effect from a commercial title.
3. **Calls into the EXEC and reads of the GROM.** Any `J` into `1000-1FFF` other than the stub's reset path, or any read of `3000-37FF`, makes the program depend on a rights-bearing image. The CI scenario loads `zero-grom` and the boot stub, so a dependency would show up as a changed result [INF]; additionally assert in the derivation that every pattern the program displays is copied to graphics RAM by the program itself (as the third diagnostic does).
4. **Names and strings** (S5-4).
5. **Port of an existing game's mechanics with its art or level data.** Out of scope; a "game" framing invites it. Keep the program a demonstration of input, video and sound.
6. **The boot stub must remain a stub.** If Phase 7 gives the program an interrupt service routine, it belongs in the program (cartridge segment), not in `FX-boot-stub`; the stub's 16-word guard exists to stop a drift into a high-level EXEC replacement (D5, D-29) [OBS]. Another agent owns where the routine lives; this note only requires that the stub label "project boot stub, not an EXEC" stay true.

**Reversal evidence.** The owner changes the licence (D9 row says "Owner instruction", so the owner can). Then every manifest's `licence` and evidence record changes together; the gate fails closed until they do.

**Risks.** The attestation is only as good as the author's diligence; the checker cannot judge it (`docs/fixture-manifest.md` says so). The mitigation is a short written checklist, not another tool.

## S5-4 Program naming (trademark-safe)

**Question.** What is the program called, and what must never appear?

**Findings.** [OBS] The working name `iamverysmartvision` is not trademark-cleared (README, PROJECT.md). "Intellivision" is never in a package, product or repository name (D9, RIGHTS.md Names, AUTONOMY hard stop). Existing ids: `FX-diag-stic-1` / `ivs-diag-stic-1` / scenario `diag-stic-fgbg-pass`. Scenario ids are `[a-z0-9-]{1,48}`; fixture ids `FX-[A-Za-z0-9._-]+`. [OBS] Nothing in the gate enforces the name rule; a grep of `tools/check` for "intellivision" finds no check.

**Options.** (a) A designed title (a game-like name). (b) A descriptive id with the project prefix. (c) Neutral codename.

**Recommendation [REC]: (b)**, owned by decision point A for the slug itself: fixture `FX-demo-<slug>-1`, program id `ivs-demo-<slug>-1`, scenario `demo-<slug>-pass`, header label "ivs-demo-<slug>-1, the first project-authored scenario program". `<slug>` is a functional word for what it does (for example the input-to-colour mapping), not a title. No title screen and no on-screen text, so there is no string to clear.

**Exclusion list (never in an id, label, comment, title, tune name or release note):** "Intellivision" in any product-like use (descriptive nominative use in docs, as the README does, is fine), "Intellivoice", "Master Component", "Keyboard Component", "ECS", "Mattel", "Atari", "INTV", "Sears" as a product, "Blue Sky Rangers", the nickname "Inty" and derived tool or library names that carry it, any commercial game title or its character names, and any word that reads as a clone of one. Also do not name the program after the working name `iamverysmartvision` itself (uncleared); the `ivs-` prefix is the repository's existing short form and is already in use.

**Cheap enforcement [REC, lowest priority].** A `names` rule inside the `files` step (awk over `FILELIST`): a path component under `fixtures/`, `data/scenarios/`, `tools/` and `runner/`, or a CMake `project(`, `add_library(`, `add_executable(` or `install(` name, that matches `[Ii]ntelli` fails with `files-trademark-name`. One control. It makes a hard stop checkable at the point a new title is introduced (this phase) and at each release. If the planner judges the cost not worth it, record the exclusion list in `docs/RIGHTS.md` "Names" and rely on review.

**Reversal evidence.** The owner picks a title (then the exclusion list is a pre-check for it, and the trademark gate becomes live again as a hard stop: a title is a "name" in the AUTONOMY sense).

**Risks.** Over-descriptive ids make the program sound like a test; accepted, the claim tier is "for this original program only".

## S5-5 Owner-local EXEC/GROM route: what is documented and how identity is checked

**Question.** What does "documented with identity checks, labelled non-redistributable and kept outside CI" consist of, concretely?

**What exists** [OBS]: user content roles on the runner (`--exec`, `--grom`); core load with `expect_sha256`; the report prints `slot.<role>.sha256` and `.length_bytes` for every loaded role; refusals on frames, video and golden for user content; `needs-user-exec` as the gate's refusal demonstration; owner evidence items `D11-EXEC-01..03` in `data/expected/d10-measurements.ids` with anchors `#owner-exec` in two procedure pages that say only "run the owner's own system image ...; the image is never committed".

**Gap.** No page tells the owner the file shapes, where to keep the files, how to check that the bytes are the bytes they mean, or what to do with the result.

**Recommendation [REC]: one new page, `docs/owner-images.md`,** linked from both `#owner-exec` anchors and from `docs/runner.md`. Sections:

1. **Label.** "Owner-local. The inputs belong to the owner, are not part of this project, are not redistributable by it, and are never committed, cached, uploaded or executed by CI." (Full text in S5-9.)
2. **Inputs and shape.** EXEC: exactly 8192 bytes, 4096 words, high byte first, bits 15..10 zero (D-26) [OBS: `docs/diagnostics.md`]. GROM: exactly 2048 bytes. A dump in another layout is converted by the owner; the core refuses the wrong shape with `IVS_E_MALFORMED` (reason 21 for a truncated EXEC appears in the consumer golden [OBS]). [UNK] whether a given dump matches the container without conversion; the page must not assert that it does.
3. **Where.** `refs/FX-owner-exec.bin` and `refs/FX-owner-grom.bin`: already ignored twice (`/refs/` and `*.bin`), outside every gate step, outside the install tree, outside `git archive`. Never in `build/` (cleaned and uploaded-looking), never elsewhere inside the work tree (an extension like `.exec` or `.img` is not ignored and `git add -A` would stage it).
4. **Identity, three layers.**
   - Shape (core, automatic).
   - **Owner digest lock** `refs/owner-images.sha256`, lines `<64 hex>  exec` and `<64 hex>  grom`, written by the owner (or by the helper's first-run `--record`, which prints what it recorded). A hash "establishes identity, not rights" and the lock is a convenience for the owner's own reproducibility.
   - **Core check at load.** The helper inserts `expect-sha256 exec <lock hex>` and `expect-sha256 grom <lock hex>` into a temporary copy of the template scenario, so the core compares its own digest of the loaded bytes (no read-then-read gap), and a mismatch ends the run as `unexpected-refusal` (exit 14, [OBS: scenario `unexpected-refusal`]).
5. **Record keeping.** The owner's note of a run states the two digests, the project commit, the scenario id and the observation, in the owner's private notes. Nothing derived from the images is kept in the repository: no frames, no audio, no PSG log, no goldens (runner refusals, plus S5-6 hardening).
6. **What the route does not do.** It does not say which images are valid, does not list known digests, and does not claim the core executes any real EXEC correctly. [UNK] whether a real EXEC reaches its title screen on this core at all; that is exactly D11-EXEC-03 and may fail.

**Why not record known EXEC and GROM digests in the repo?** [REC: no, in Phase 7.] RIGHTS.md allows a single identity hash as a fact, but (1) the digests cannot be confirmed from this repository [UNK], (2) several system images and revisions exist for the family and the profile is family-level (D3), so a "known good" list would read as a compatibility list, which Core Value forbids as an unqualified label, and (3) "Bulk hash databases taken from other projects" are explicitly never committed. If the owner later wants to publish one digest as a fact, that is an owner action with its own source citation.

**Manifest labelling (S5-5b).** [REC] Add `FX-owner-exec` and `FX-owner-grom`:

```
kind system-image, committed false, path null, local_path refs/FX-owner-exec.bin,
source {origin "owner-supplied, not distributed", pin null, accessed null, holder null},
recipe null, inputs [], outputs [], licence NOASSERTION, licence_evidence null,
copyright [], notices [], redistributable no, allowed_use [local-test],
rights_review {status unreviewed, by null, date null}, derived_from [],
note "never committed or redistributed; owner-local route (D11) described in docs/owner-images.md"
```

Effect: the gate counts them (`manifests`, `effective-no`: from 5 and 1 to 8 and 3 with the program's manifest, [INF] from the Phase 6 summary line), a tracked file under that path fails the `files` step and the `manifests` step, and the label is machine-read, not prose. `FX-sample-rights-unknown` stays as the illustrative `unknown` entry. Cost: two JSON files and two lines in `data/expected/fixtures.ids`. [REC] `rights_review.status: unreviewed` is the only value that fits a "no": `owner-confirmed` would assert the owner reviewed it.

**Reversal evidence.** If the planner wants fewer files, one manifest `FX-owner-system-images` with `local_path: null` and the two roles in the note is acceptable; the label value is the point.

**Risks.** A page of commands can rot; S5-6 gives the commands a test. The doc must avoid the tripwire words (S5-9).

## S5-6 Runner and helper support for the route

**Question.** Is the existing runner enough, or does the route need new code?

**Options.**

A. Docs only: the owner runs `ivs-run --exec ... --grom ... <scenario>` after hand-editing a local scenario copy.

B. **Docs plus a tracked template scenario plus a small operator helper `tools/owner-run.sh`** (not a gate step; same status as `tools/listen-demo.sh`).

C. New runner flags (`--exec-sha256`, `--grom-sha256`) so the digest check needs no scenario edit.

**Trade-offs.** A has no new code but also no tested identity check, and the in-tree-path mistake (staging an image) is one `git add -A` away. C changes the runner CLI, the usage text, `docs/runner.md`, the `--help` consistency tests and the golden refusal rules for a convenience a 60-line script gives. B follows an existing precedent (listen-demo has one gate test, `runner-process.listen-demo.refuses-user`) and keeps the runner unchanged apart from one hardening.

**Recommendation [REC]: B.** Components:

1. **Template scenario** `data/scenarios/owner-exec-title.ivss`: `ivss 1`, `id owner-exec-title`, `profile intv-2609-ntsc-family-p1`, `content exec user`, `content grom user`, no cartridge role [INF: the profile requires exec and grom only, `MISSING_SYSTEM_IMAGE` is raised for those two (`core/src/machine/machine_run.c`); the planner confirms a scenario without a cart role parses], a marker comment line for injection, `run-to <n> frames` (n chosen by the planner, large enough for a title sequence to be attempted; no pass/fail meaning), `# expect-detail: missing role=exec user`. No `assert` that encodes a result. The outcome vocabulary for an owner run is therefore only: reached the target (`pass`), stopped early (halted or faulted: `guest-fail`, a fact about the run, not a verdict on the image), refused (identity, shape).
2. **Helper behaviour** (POSIX sh, one screen): refuse when `CI` or `GITHUB_ACTIONS` is non-empty (exit 2); refuse when an image path resolves inside the work tree but not under `refs/` (exit 2); require both files to be regular files with the exact sizes (exit 2); read or, with `--record`, write `refs/owner-images.sha256` using `cmake -E sha256sum` (already a project requirement); write the temporary scenario (inserting two `expect-sha256` lines after the last `content` line) into a `mktemp` directory outside the work tree and delete it on exit; run `build/<preset>/runner/ivs-run --exec ... --grom ... <temp scenario>` with **no** output-file options; print a banner `OWNER-LOCAL RUN (D11): inputs are the owner's, not redistributable, not run in CI, not evidence, no claim`; when the images are absent print `SKIPPED owner-run: no operator images (D11)` and exit 77 (the repository's skip convention: `SKIP_RETURN_CODE 77` [OBS]).
3. **Gate tests** (refusals only, no image bytes needed, all `sh` and sub-second): no-arguments usage (2), path-in-tree-outside-refs (2), `CI=1` (2), wrong size using a temp file (2), missing lock without `--record` (2), images absent gives exit 77 and the SKIPPED line. Where they live is the planner's call; `runner-process.ids` already hosts `runner-process.listen-demo.refuses-user`. Count: about six ids, each judged by exit code and one line of text. Add one `judge`-style self-control so a helper that exits 0 when it should refuse fails a test.
4. **Runner hardening [REC].** Refuse `--audio-raw`, `--psg-log-out` and `--audio-wav` when the scenario names user content, with the exact style of the existing frame and video refusals (`... refused: content user`), before anything is created, and one process test. Reason: the video, frame and golden outputs are refused as "derived art of rights-bearing content" [OBS: comments in `runner/main.c`]; a PSG write log of a real EXEC's startup is the same class of derived material and the asymmetry is only an omission from Phase 5 (when D7 and the WAV path were written) [INF]. Cost is small and it removes the one way a careless owner run produces a derived file next to the repository. The report on stdout still prints digest lines and, for a scenario with asserts, `got <actual>` values; with the template having no asserts, nothing but the 64-hex state digests and register-level run state is printed [INF].

**Is the helper over-building?** [adversarial] The only consumer today is the owner, who has said (AUTONOMY) that D11 runs are a hard stop the agent does not perform. A helper that nobody runs is verification machinery with no feedback. Counter: SCN-02 literally asks for identity checks; the gate cannot test prose, and a mistaken in-tree path is a rights incident in a public repository. Compromise [REC]: keep the helper to the listed behaviour (no UI, no options beyond `--record`, `--preset`), cap it at about 80 lines, and drop item 3's count to the four refusal cases that protect rights (CI, in-tree path, size, lock) if the planner needs to cut.

**Reversal evidence.** If the owner states they never will run D11, drop the helper and keep docs, template scenario, SKIP line and manifests (everything gate-checkable without an owner file).

**Risks.** The helper's `expect-sha256` injection depends on header-directive ordering rules in the parser (header directives before the first `run-to`, any order among them [OBS: `docs/runner.md`]); the gate test for the injected scenario's parse is a `--dry` style check that the planner may need to realise by running the template with a synthetic 8192-byte and 2048-byte pair of zero files created in a temp directory (not real images; the run would end `guest-fail` quickly or time out, and that is a valid outcome for the test). Keep that test to the parse and load stage.

## S5-7 Visible SKIPPED line and inventory counting

**Question.** How does "any job that refers to it prints a visible SKIPPED line, and the inventory gate counts it" become true and testable?

**Today** [OBS]: the TAP file has `# SKIP`; `tests-inventory` counts skips and rejects any skip not marked `skip-ok`; the final `gate:` line prints `tests-skipped=2`; the hosted job summary lists nonzero skipped tokens. The word SKIPPED, with an id, is nowhere on the gate's stdout.

**Options.** (a) Rely on the count. (b) Add a printed per-id line in `tests-inventory`. (c) Make the helper and the scenario each print their own line only.

**Recommendation [REC]: (b), plus the scenario skip.**

- Register `owner-exec-title` in `data/expected/runner-scenarios.ids` as `owner-exec-title 13 missing-fixture deterministic skip-ok -` and `runner-process.scenario.owner-exec-title skip-ok` in `data/expected/tests/runner-process.ids`. It is judged like any scenario (must end `missing-fixture`, exit 13, detail `missing role=exec user`), and only a passing judgement is reported as a skip, so a broken template fails loudly. `tests-skipped` becomes 4 (one id, two presets).
- Change `tests-inventory.sh` so that when it classifies an id as skipped it also prints `-- SKIPPED <preset>/<suite>/<id>: <reason from the TAP line>` (the awk already sees the line; it needs to keep the text after `# SKIP`). Skipped ids are few (one per preset today), so the log stays short. Keep the existing `tests-skipped=` token (the CI summary table depends on it).
- The helper's own `SKIPPED owner-run: ...` line (S5-6) covers "any job that refers to it" for operator use; the gate never invokes the helper without images except in its refusal tests.
- Control: the existing `tests-inventory-unexpected-skip` control covers "skip without skip-ok". Add a unit-level check in an existing self-test (or a new control) that the SKIPPED line carries the id; since a printed line is not a failure token, prove it by a self-test that greps the step's stdout on the toy tree the controls already use [INF; the planner picks the mechanism].

**What counts as "the inventory gate counts it".** `tests-inventory` is the exact-set check (`tests-missing`, `tests-unlisted`, `tests-unexpected-skip`); adding the id to the `.ids` file with `skip-ok` is the counting, and removing the scenario file makes `tests-missing` fire. Nothing further is needed.

**Reversal evidence.** If printing from `tests-inventory` is judged noisy, the TAP `# SKIP` line plus the `tests-skipped=4` token plus the CI job-summary table already give a visible count; criterion 4 then reads as satisfied by the count, and the id is visible only in `build/gate/`. State that choice in the verification record.

**Risks.** A future second operator-only scenario would add skips without anyone noticing; the S5-8 consistency rule catches the unmarked case.

## S5-8 What stays out of CI, out of the repository and in the ignored cache

| Material or derived item | Repository | CI | `refs/` (ignored cache) | Release asset |
|---|---|---|---|---|
| Project listing, manifest, template scenario, derivation, docs | yes (MIT) | yes | no | yes, in the source tarball |
| Built `.ivsc` and `boot-stub.exec` of project programs | no (build tree) | built and used, discarded with the job | no | no (recorded release design refuses untracked and binary assets) |
| Owner EXEC and GROM images | never | never (the gate never passes `--exec` or `--grom`; no image exists on a runner) | yes, `refs/FX-owner-*.bin` | never |
| Owner digest lock `refs/owner-images.sha256` | never | never | yes | never |
| Any frame, WAV, PSG log, golden or digest list derived from an owner image | never | never | not kept (runner refuses frame, video, golden; S5-6 adds audio and log) | never |
| Cartridge images, manual scans, overlays (third party) | never | never | `refs/` only (existing practice) | never |
| Owner's notes of a D11 run | not in the repository | never | optional, private | never |

**Existing guarantees** [OBS]: `gate.yml` has no cache, artifact or secret step and read-only permissions (D12: no cache, no artifacts); `.gitignore` covers `/refs/` and the binary extensions; the `files` step fails on tracked `refs/` paths, tracked `.env`, unlisted binaries and unlisted fixture files, each with a negative control (`files-tracked-refs-file`, `files-unlisted-binary*`, `files-unlisted-fixture-file`); `tools/hygiene.sh --tree` scans tracked and untracked-not-ignored files, so a stray owner image with a non-ignored extension would at least surface as untracked content. `git archive` exports tracked files only.

**New guards [REC].**

1. **User-content consistency rule.** In the `runner-process` suite (or `runner-scan`): every `data/scenarios/*.ivss` with a `content <role> user` line must be `skip-ok` in `runner-scenarios.ids`, and every `skip-ok` row must contain such a line. One awk pass, one failing-case control. This makes "kept out of CI" a gate property instead of a convention: a future scenario that needs owner content cannot enter the gate as an ordinary, failing-or-skipping test unnoticed.
2. **Helper environment refusal** (S5-6): `CI` or `GITHUB_ACTIONS` set means exit 2 even if files exist.
3. **Extend `.gitignore`** with `*.exec`, `*.grom` and `*.img`? [REC: only `*.grom` and `*.img`; not `*.exec`.] `boot-stub.exec` is a build-tree name, already under `/build/`; ignoring `*.exec` globally would hide a mistaken tracked boot stub from `git status` but not from the `files` step. Marginal; planner's call. The helper's in-tree refusal is the real guard.

**Reversal evidence.** None expected; each row is a hard stop in AUTONOMY.md.

## S5-9 D11 wording and SCN-02 completion terms

**What D11 is** [OBS]: an owner gate (REQUIREMENTS, ROADMAP "Open", "Before SCN-02 can be claimed (Phase 7)"); AUTONOMY hard stop: "runs with the owner's real EXEC/GROM (D11) ... Mark the evidence as missing and keep going". ROADMAP Phase 7: "SCN-02 can document the owner-local EXEC/GROM route, but no real-EXEC result is claimed unless the owner chooses to run it."

**Reading [INF].** "Gated on D11" in SCN-02 refers to performing and reporting a run, not to writing the route down: the requirement text says "documented with identity checks, labelled non-redistributable and kept outside CI". Phase 5 and 6 set the precedent: INP-*, AUD-*, VID-* were marked complete with a close-out note "only on these terms" while D10 and D11 items stayed handed off and not performed.

**Recommendation [REC].** SCN-02 is marked complete on these terms, and the same note goes into REQUIREMENTS.md, PROJECT.md (Decision Register note under D11, as a "Delegated decision" row only for the route, not for any run) and the 07 verification record:

> **Phase 7 close-out note.** SCN-02 is marked complete because the owner-local route is documented in `docs/owner-images.md`, the template scenario `owner-exec-title` ends `missing-fixture` in the gate and is reported as a counted, visible skip, the helper refuses every unsafe case, and the owner image manifests are labelled `redistributable: no`. D11 is **not performed**: no real EXEC, GROM or commercial content has been run by this project, and no result about any of them is claimed or implied. Owner evidence items `D11-EXEC-01..03` stay handed off.

**Text for `docs/owner-images.md` (avoid the text-step tripwire words; the line below is checked against it).**

> This page describes how the owner may load two files that belong to the owner, an EXEC image and a GROM image, into the project's headless runner on the owner's own machine. These files are not part of this project, this project does not distribute them, and nothing here makes them redistributable: a digest shows which bytes a file holds, not who may copy it. They are kept in the ignored `refs/` directory, never committed, cached, uploaded or loaded by CI. The project makes no statement about what any real EXEC, GROM or commercial cartridge does on this core. Output of an owner-local run is the owner's private observation. It is not evidence for any claim in this repository, and it is published only if the owner decides to.

Words to avoid or waive with `[nc]`: supports, compatible, passes, accurate, verified, runs. The sentence "the runner loads" is fine; "the route runs" is not.

**SKIPPED line wording [REC].** `SKIPPED owner-exec-title: owner-local EXEC/GROM route (D11); needs operator images the gate never has; not run, no claim`. The generic TAP reason already says "needs operator content (ended missing-fixture, exit 13, as expected)"; the printed line should add "(D11)" and "no claim" so a reader of the CI log cannot read it as a pass.

**No claim-ledger entry and no tier for the route.** [REC] `data/tiers.json` has no tier for owner-image runs and none should be invented; a ledger row would be a statement. SCN-01's program claim (tier `playable-guest-progress`, wording owned by decision point D) must say "for this original program only, project boot stub, no EXEC" and must not mention D11 as support.

**Reversal evidence.** The owner runs D11 and decides to publish a result: then a new, separate claim with its own profile, artifact (the owner's digests as recorded facts) and tier; a separate phase of work, not Phase 7.

**Risks.** A verifier could read "SCN-02 complete" as "real EXEC supported"; the close-out wording above and the absence of a ledger claim are the defence.

## S5-10 Release implications

**Setting** [OBS]: AUTONOMY section 5 and its 2026-11-11 design record: releases are frequent 0.x, the exact commit green on `main`, `.github/workflows/release.yml` (not yet implemented), assets = deterministic `git archive` source tarball, a tarball of the install tree, `SHA256SUMS`; assets come from an allowlist (tracked files, effective-yes manifests, install tree), and a `release-assets` check refuses anything untracked or with a binary extension. Notes state claims only as profile + evidence tier + artifact. The install tree is exactly `data/expected/pkg-install-files.ids` (headers, `libivs_core.a`, CMake package files, `LICENSE`): no images, no runner, no `ivs_pack`.

**Consequences for a project-authored program.**

1. **It ships inside the source tarball only.** Listing, manifest, scenario, goldens, derivation and docs are tracked MIT files, so they are redistributable by construction. `allowed_use: release` is truthful.
2. **No "download and run" yet.** A consumer would need `ivs_pack` and `ivs-run`, neither installed (D-24, D-33). Do not widen the install list or the asset allowlist in Phase 7; say in the release notes that the program is source and is exercised by the project's gate and installed-consumer replay. [REC] Record "a demo bundle asset (image plus scenario) or installing the runner" as a Phase 8 / REL-01 option, with the cost: a binary extension asset needs an allowlist change and a manifest-backed exception.
3. **`git archive` also exports `.planning/`, `.claude/` and the CI files** (no `.gitattributes`). They are tracked and covered by the hygiene scan, so this is not a rights problem; it makes the tarball larger and carries planning prose. [REC] Leave as is; an `export-ignore` for `.planning/` is a release-PR decision, not a Phase 7 one.
4. **Claims in notes.** One sentence per claim as profile + tier + artifact, for example: profile `intv-2609-ntsc-family-p1`, tier `playable-guest-progress` for the original program `ivs-demo-<slug>-1` (listing SHA-256 from the manifest), run through the installed-consumer replay, provisional. Never "plays Intellivision games", never a percentage, never "supports the real EXEC". State what is excluded: no EXEC, no GROM, no commercial content, D10 and D11 not performed.
5. **Names.** Release title and notes use the descriptive form the README uses ("an emulator core targeting the Intellivision family"); the S5-4 exclusion list applies to release names, asset names and tag messages.
6. **Hygiene before each release** (AUTONOMY section 6): author emails noreply only, secrets, private paths, third-party binaries. Note [OBS from the project's own memory and `tools/check/hygiene-baseline.txt`]: public `main` history contains three planning commits with a scratch-path string that cannot be fixed without a history rewrite (an owner decision); the baseline makes the history check pass over them. Do not "fix" it in Phase 7.
7. **README is stale** [OBS]: "This repository holds the Phase 1 evidence foundation only. No machine code exists yet. There is no software release." The first release (the program is the first thing a reader can point to) should not ship that README. [REC] Update the Status section in the Phase 7 close-out, keeping the non-affiliation notice and the no-unqualified-claim sentence.
8. **Frequent releases and drift.** Each release re-ships the same manifest at the tag commit. Because S5-2 makes `recipe.sources` and `outputs` digest-checked in the gate, the manifest in a release is internally consistent with the tarball by the same gate that passed on that commit.
9. **Version.** [OBS] `ivs.h` carries 0.1.0 and the installed package version ties to it. A release is a version-bump commit on `main`, not a Phase 7 task; GSD milestones and software versions stay separate (AUTONOMY, AGENTS.md).

**Reversal evidence.** If the owner wants a downloadable demo, that is a deliberate allowlist and install-list change with its own review.

## S5-11 Hygiene checks (existing and added)

**Existing and sufficient** [OBS]: `files` (tracked refs, `.env`, unlisted binary by name and content, unlisted fixture file) with six controls; `manifests` (fail closed on missing or misspelled keys, unproven `yes`, committed-not-redistributable, digest mismatch, tracked output of an uncommitted manifest); `hygiene` (home and scratch paths, tokens, passwords, non-noreply emails, tree mode in the gate, history mode at publication and release); `denylist` (author names, licence-header strings, emulator project names outside `[cite-only]`); `text` (claim-language tripwire); `listing` (anti-HLE guards for the stub; word, row and branch checks for cartridges).

**Added by this note [REC].**

| Check | Where | Failure class it catches | Control |
|---|---|---|---|
| Recipe source digest | `manifests` | listing or `pack.c` edited, manifest recipe stale | `manifests-recipe-source-digest` |
| User-content scenario must be `skip-ok` | `runner-process` or `runner-scan` | an owner-content scenario entering the gate as a normal test, or a `skip-ok` row with no user content | one control per direction |
| Audio and log output refused for user content | `runner/main.c` plus a process test | derived PSG or audio file of an owner image | the test is its own control; an existing fault-style mutant is optional |
| `names` rule | `files` | "Intellivision" in a fixture, scenario, target or install name | `files-trademark-name` |
| Printed SKIPPED line | `tests-inventory` | a skip hidden behind a number | self-test of the step's stdout |
| Owner manifests labelled `no` | `fixtures.ids` and two manifests | owner images being described as redistributable | covered by `manifests-unknown-committed` logic; no new control |
| Helper refusals | gate tests | CI use, in-tree path, wrong size, missing lock | the tests |

**Not added** (considered and rejected): a hash scan of tracked files against known EXEC digests (needs a digest list that must not be committed); a CI-side `refs/` existence check (it cannot exist in CI); a binary-signature scanner for ROM headers (the NUL and mostly-non-text rules in `binscan.sh` already classify, and a deliberately disguised file is a stated limit of the checker [OBS: `docs/RIGHTS.md`]).

**Review checklist line to add to `.github/pull_request_template.md`** [REC]: "Program content: no glyph, tune, string or table copied or recalled from a commercial title; no EXEC call; no GROM read (derivation section 'Rights')."

## Adversarial pass against this recommendation

1. **"A non-null recipe with an in-tree tool is theatre."** True that `tool_version` is free text. What is real: the sources and outputs are hash-checked, the command is reproducible, and the release is the exact commit. The alternative (recipe null) leaves criterion 1 arguable. Residual risk accepted.
2. **"Extending the manifest checker is scope creep for one fixture."** About 20 lines, one control; the failure class (digest rot after an edit) is one the project has already patched once with a helper script. If the planner disagrees, option A with a candid note is a legitimate fallback.
3. **"The helper and its tests exist for a run the owner may never do."** Yes; hence the trimmed form (four rights-protecting refusals, no options). The in-tree path guard and the CI refusal are the parts that prevent a public-repository incident; the digest lock is the part SCN-02 names.
4. **"Printing SKIPPED from `tests-inventory` is cosmetic."** It converts a number into an id and a reason; criterion 4 uses the word "visible". If the owner prefers silence it is one revert.
5. **"Refusing audio and log outputs for user content blocks D11-EXEC-02 (record real startup sound behaviour)."** It blocks writing a file; the owner can still read the run's digest lines, and a text log of the PSG writes of a rights-bearing program is the same class as its frames. If the owner wants the log, a later owner-approved exception (a flag naming an output outside the repository) is cheap to add.
6. **"No known EXEC digest means the identity check checks nothing."** It checks that the file the owner loaded is the file the owner pinned, which is what reproducibility of an owner-local note needs. Which images are *valid* is a compatibility statement and is out of scope and out of Core Value.
7. **"Putting `release` in `allowed_use` for the program commits the project to ship it."** `allowed_use` states what is permitted, not what is done; the release workflow chooses assets.
8. **"MIT for a demo program whose art might later be reused."** Fine for reuse; the trade is losing any ability to restrict reuse. No evidence the owner wants a restriction.
9. **"The attestation checklist is unverifiable."** Yes; the checker cannot judge a review, and the manifest doc says so. The checklist makes the claim specific and reviewable; the real defence is the design (geometric shapes, scale-based sound, boot stub, zero GROM in CI).
10. **"A real EXEC may never start the project's program, so why does the route mention it?"** Correct: the template runs EXEC and GROM only (D11-EXEC-03, a title-screen look). Whether a real EXEC boots a cartridge that lacks the cartridge header the EXEC expects is [UNK] here and probably not [BG: community documentation describes a header the EXEC checks; not re-fetched]. Running the project program under a real EXEC is out of Phase 7.

## Assumptions from other decision points

- **A (S-1 program).** The program uses geometric patterns, no text, a scale or pitch mapping for sound, the project boot stub, zero GROM, no EXEC call, and is named by a functional slug; the slug is theirs.
- **B (S-2 runtime).** The interrupt routine and display strobe live in the program's cartridge segment, not in `FX-boot-stub`, which stays at its contract-1 shape (at most 16 non-zero words). If B proposes changing the stub, the stub's "not an EXEC" label and the anti-HLE guard must be re-read before that goes in; a stub change also refreshes `FX-boot-stub` outputs and the three diagnostics' goldens (B's problem), and nothing in this note depends on it.
- **C (S-3 assembler).** Either decision fits S5-1: hand-assembled listing (the manifest outputs the listing, the recipe command is `ivs_pack --ivsc`) or a mini-assembler (outputs add the program source, the recipe command names the assembler, the assembler's source is a recipe source). A mini-assembler written in-tree is project code under MIT; no third-party assembler enters the recipe (D4: as1600 is GPL and stays out). If a data directive is added to the listing grammar, `tools/pack/pack.c` changes and S5-2 makes that visible in the manifest.
- **D (S-4 verification).** The program is run in the gate through the runner and the installed-consumer replay (images from the build tree). Claim wording for SCN-01 is theirs; this note only adds that it must not mention D11. The tier for SCN-01 is `playable-guest-progress` [OBS: it exists in `data/tiers.json`], and `important-gates.sh` currently rejects any requirement outside the Phase 4 to 6 sets, so registering SCN-01 or SCN-02 gates requires extending that set (D or the planner).

## Open items for planning

1. Choose the manifest-check scope: S5-2 (a), (b) or (c); decide whether `tools/pack/pack.c` is a listed recipe source.
2. Decide the `names` rule and the `.gitignore` additions (`*.grom`, `*.img`); both are optional.
3. Confirm that a scenario with no cartridge role parses and loads (expected from `MISSING_SYSTEM_IMAGE` covering exec and grom only); pick `n` for the template's `run-to`.
4. Choose where the helper's tests live (`runner-process.ids` is the precedent) and the exact printed SKIPPED line format; update `ci-summary` only if the table should list ids.
5. Add `FX-demo-<slug>-1`, `FX-owner-exec`, `FX-owner-grom` to `data/expected/fixtures.ids` and re-baseline the `manifests` and `listings` counters; add the program to `data/expected/listings.ids`.
6. Update stale text at phase transition: README Status; PROJECT.md licence-gate lines and open question 11; ROADMAP REL-01 "publication remains an owner gate"; REQUIREMENTS SCN-02 note; D11 note (route documented, not performed).
7. Add anchors in `docs/stic/d10-procedure.md` and `docs/input/d10-procedure.md` (`#owner-exec`) pointing to `docs/owner-images.md`, and a `docs/runner.md` pointer; keep `data/expected/d10-measurements.ids` unchanged.
8. Write the derivation "Rights" section (the S5-3 checklist) for the program and copy it into the manifest note; add the PR template line.
9. Decide whether the runner hardening for audio and log outputs ships in this phase (recommended) and add its process test and `docs/runner.md` sentence.
10. Release-side: keep a note for REL-01 about a downloadable demo bundle, the `.planning/` export, and the README; no change to the release design in Phase 7.
11. [UNK] Whether the project should ever record a single EXEC or GROM identity digest as a fact; needs an owner decision and a cited source, not a Phase 7 task.
