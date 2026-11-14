---
phase: 07-first-visible-original-scenario
plan: 10
subsystem: the SCN-02 owner-local route, part one - template scenario as a counted skip, the printed SKIPPED line, the user-content consistency rule, SCN-02 in the important-gates registry, the owner-images page and the two non-redistributable manifests (W6a)
tags: [W6a, SCN-02, skip-ok, SKIPPED-line, tests-inventory, golden-consistency-rule, important-gates, owner-images, FX-owner-exec, FX-owner-grom, D-30, D-31, D-32, D-36, A-13, A-21, A-22, T-07-23, T-07-24, T-07-25, V13, V15, V16, V17]
status: complete

requires:
  - phase: 07-first-visible-original-scenario
    provides: "plan 07-09: gate steps=36/36 controls=354/354 faults=222/222 manifests=8 effective-no=1 tests-skipped=2 tests-missing=0, HEAD 2215d7f at the start of this plan; the toy important-gates registry with the SCN-01 toy line and the nine whole-file important-gates control overlays of plans 07-02 and earlier"
provides:
  - "data/scenarios/owner-exec-title.ivss: the template of the owner-local route (content exec user, content grom user, no cartridge role, run-to 600 frames, a marker comment for the helper, no assert), ends missing-fixture exit 13 with detail missing role=exec user"
  - "data/expected/runner-scenarios.ids and data/expected/tests/runner-process.ids: the skip-ok rows; tests-skipped 2 to 4, tests-expected 4596 to 4598"
  - "tools/check/steps/tests-inventory.sh: one printed line -- SKIPPED <preset>/<suite>/<id>: <reason> per skipped id and preset, before the pass line; every token unchanged"
  - "tools/check/steps/golden.sh: the user-content consistency rule in both directions (golden-user-content-not-skip-ok, golden-skip-ok-without-user-content) with two controls; controls 354 to 356"
  - "tools/check/steps/important-gates.sh accepts SCN-02; two registry lines (gates 75 to 76, lines 261 to 263); the toy registry and the nine whole-file overlays gain the appended toy line toy-owner"
  - "docs/owner-images.md and the two owner-exec anchors and docs/runner.md linking it; fixtures FX-owner-exec and FX-owner-grom (uncommitted system-image manifests, redistributable no); .gitignore gains *.grom and *.img"
affects: [plan-07-11-owner-run-helper, plan-07-12-closeout]

actuals:
  tokens: 4760
  tasks: 4
  commits: 3
plan_head_before: 2215d7fddeba6effee9a6a64b4c7dfc971ac104f
plan_head_after: bdf0e503b72fb41f50a816b77eba129af6fd517f

key-decisions:
  - "The SKIPPED line carries the reason text after `# SKIP` of the TAP result line unchanged, so the id appears twice (`<preset>/<suite>/<id>: <id>: needs operator content ...`); the D11 and no-claim wording of the research note belongs to the helper's own line of plan 07-11"
  - "The two consistency controls use their own toy scenario or row (a new toy-user scenario for the first, the toy-wall row for the second) so each reports only its own token, and the existing golden-user-scenario-tracked control keeps its toy shape"
  - "SCN-02 stays Pending in REQUIREMENTS.md: the helper and its refusals are plan 07-11 and the close-out is plan 07-12 (the research note S5-9 states the helper refusals among the close-out terms)"
  - "The page claims only what the code does: a tracked path under refs/ fails the files step; the manifests step guards the outputs of an uncommitted manifest, not its local_path"

key-files:
  created:
    - data/scenarios/owner-exec-title.ivss
    - docs/owner-images.md
    - fixtures/FX-owner-exec/manifest.json
    - fixtures/FX-owner-grom/manifest.json
    - tools/check/controls/golden-user-content-not-skip-ok/
    - tools/check/controls/golden-skip-ok-without-user-content/
    - .planning/phases/07-first-visible-original-scenario/07-10-SUMMARY.md
  modified:
    - data/expected/runner-scenarios.ids
    - data/expected/tests/runner-process.ids
    - data/expected/controls/golden.ids
    - data/expected/important-gates.ids
    - data/expected/fixtures.ids
    - tools/check/steps/tests-inventory.sh
    - tools/check/steps/golden.sh
    - tools/check/steps/important-gates.sh
    - tools/check/controls/_good/tree/data/expected/important-gates.ids
    - tools/check/controls/important-gates-*/overlay/data/expected/important-gates.ids (nine)
    - docs/runner.md
    - docs/stic/d10-procedure.md
    - docs/input/d10-procedure.md
    - .gitignore
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-13
---

# Phase 7 Plan 10: SCN-02 template scenario, counted SKIPPED line, consistency rule, owner-images page and labels Summary

**The owner-local EXEC and GROM route now exists as documented, labelled and gate-counted material: a template scenario that ends `missing-fixture` in the gate and prints a visible `-- SKIPPED` line per preset, a rule that ties every `content <role> user` scenario to a `skip-ok` row in both directions, SCN-02 as a covered important-gates requirement, a page with three identity layers and no digest, and two uncommitted `redistributable: no` manifests; no image is placed, run or hashed and D11 stays open.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `238c76f` | 1 (tracer) | the template scenario, the two skip-ok rows, the SKIPPED line in `tests-inventory.sh` |
| `adba1ea` | 2 | the consistency rule in `golden.sh`, two controls, SCN-02 accepted by `important-gates.sh`, registry and toy lines |
| `bdf0e50` | 3 | `docs/owner-images.md`, the three links, the two manifests, `fixtures.ids`, `.gitignore` |
| the commit that adds this file | | this SUMMARY, STATE.md, ROADMAP.md |

Task 4 (the three gates) changed nothing, so it has no commit of its own.

## What was built

- **Template** (D-30, D-31): `data/scenarios/owner-exec-title.ivss`. Comment block, `ivss 1`, id, profile `intv-2609-ntsc-family-p1`, `content exec user`, `content grom user`, the marker line `# owner-run: expect-sha256 lines are inserted after this line`, `run-to 600 frames`, `# expect-detail: missing role=exec user`, no `assert` line (`grep -c '^assert '` prints 0). The 600 frames are 35,841,600 ticks (59,736 per frame): a generous bound so a startup and title sequence is attempted, stated in the file as a bound and not as a claim about any image.
- **Parse result of the template** (assumption A4): run once through `ivs-run` with no content, it parses and ends `outcome missing-fixture`, `outcome.detail missing role=exec user`, `exit.code 13`. The exec-plus-grom-only form with no cartridge role is accepted by the grammar as written; no adjustment was needed.
- **SKIPPED line**: the awk of `tests-inventory.sh` keeps the words after `# SKIP` (default `no reason given`), the shell loop collects `<preset>/<suite>/<id>: <reason>` and prints `-- SKIPPED ...` once per skipped id and preset, before the FAIL block and the pass line. It starts with `-- SKIPPED`, so the gate's FAIL summary (`keep_fails` greps `^-- FAIL `) and the ci-summary self-tests (11/11) are unaffected. The four lines of the gate output:

```
-- SKIPPED dev/runner-process/runner-process.scenario.needs-user-exec: needs-user-exec: needs operator content (ended missing-fixture, exit 13, as expected)
-- SKIPPED dev/runner-process/runner-process.scenario.owner-exec-title: owner-exec-title: needs operator content (ended missing-fixture, exit 13, as expected)
-- SKIPPED release/runner-process/runner-process.scenario.needs-user-exec: needs-user-exec: needs operator content (ended missing-fixture, exit 13, as expected)
-- SKIPPED release/runner-process/runner-process.scenario.owner-exec-title: owner-exec-title: needs operator content (ended missing-fixture, exit 13, as expected)
```

  The same four lines are in the sh, dash and container gate output (A-22(4): two for `owner-exec-title`, two for `needs-user-exec`; the acceptance greps name `owner-exec-title` only).
- **Consistency rule** (D-31, T-07-24): `golden.sh` keeps the inventory's column 5 in `$tmp/invskip`, collects every FILELIST `data/scenarios/*.ivss` with a `content <role> user` line, and reports `golden-user-content-not-skip-ok <id>` for one whose row is not `skip-ok` (an absent row counts as not skip-ok) and `golden-skip-ok-without-user-content <id>` for a `skip-ok` row whose scenario has no such line. Header documents both tokens.
- **Controls** (356, two added): `golden-user-content-not-skip-ok` overlays a new toy scenario `toy-user` (`content exec user`) with a toy row `toy-user ... run`; `golden-skip-ok-without-user-content` overlays a toy inventory whose `toy-wall` row says `skip-ok`. Each reports exactly its own finding (output shown in the induced-failure table below); the pristine toy passes (the `_good/golden` positive control is `control ok`).
- **important-gates** (A-13, A-21): `SCN-02` joins the accepted set (header text updated); the registry gains `scn2-user-content control golden-user-content-not-skip-ok SCN-02` and `scn2-user-content control golden-skip-ok-without-user-content SCN-02`; `toy-owner control toy-control SCN-02` is appended after the last line of the `_good` toy registry and of each of the nine whole-file overlays, no comment line and no existing line touched.
- **Page** `docs/owner-images.md` (D-30, D-32): the label paragraph and section; inputs and shapes (EXEC 8192 bytes, 4096 words, high byte first, bits 15..10 zero; GROM 2048 bytes; the refusal reasons 21, 22, 23 of the public header); location `refs/FX-owner-exec.bin` and `refs/FX-owner-grom.bin` and the ignore patterns; three identity layers (shape, owner digest lock `refs/owner-images.sha256`, `expect-sha256` at load inserted into a temporary copy of the template); the helper of plan 07-11 with its refusals and its exit 77 SKIPPED line, the manual route until then; the template and the gate; record keeping in the owner's private notes; what the route does not do (no known digest, no statement that any real EXEC runs correctly, D11 open and not performed, `ivs-demo-lamp-tone-1` not part of the route). Linked from `docs/stic/d10-procedure.md`, `docs/input/d10-procedure.md` and `docs/runner.md`. The page holds no 64-hex string (`grep -Eiq '[0-9a-f]{64}'` finds none).
- **Labels**: `fixtures/FX-owner-exec/manifest.json` and `fixtures/FX-owner-grom/manifest.json` (kind `system-image`, `committed: false`, `path: null`, `local_path` under `refs/`, licence `NOASSERTION`, `redistributable: no`, `allowed_use: [local-test]`, `rights_review: unreviewed`, a note pointing to the page); `fixtures.ids` gains both; `.gitignore` gains `*.grom` and `*.img` with a comment saying why `*.exec` is left out. Counts: `manifests` 8 to 10, `effective-no` 1 to 3.
- **A-22(3)**: the one sentence in each of the two procedure pages that called the reader cartridge a Phase 7 deliverable now says it is deferred beyond Phase 7 (no Phase 7 requirement needs it; revisited with the D10 evidence) and that Lamp Tone is a scenario program, not a reader. `data/discrepancies.json` and the profile wording were left as the amendment says.

## Induced failures and checks that were shown able to fail

| Check | How shown | Result |
|-------|-----------|--------|
| `golden-user-content-not-skip-ok` | the control's tree, step run directly | `-- FAIL golden-user-content-not-skip-ok: toy-user (data/scenarios/toy-user.ivss has a content ... user line, its row in data/expected/runner-scenarios.ids has column 5 'run', not skip-ok)`, and nothing else |
| `golden-skip-ok-without-user-content` | the control's tree, step run directly | `-- FAIL golden-skip-ok-without-user-content: toy-wall (its row in data/expected/runner-scenarios.ids is skip-ok, data/scenarios/toy-wall.ivss has no content ... user line)`, and nothing else |
| existing `golden-user-scenario-tracked` | its tree, step run directly | now also reports `golden-user-content-not-skip-ok: toy` (its toy scenario has `content exec user` and a `run` row); it has no FORBID file, so it still passes on its own EXPECT line (`control ok` in the isolated run and in the gate); no edit to that control |
| nine important-gates overlays | the step on a copy of the toy tree with each overlay applied (before the commit) | each reports exactly its own finding (`unknown-id` `fault NO_SUCH_FAULT line 13`, `uncovered-gate` `PKG-04`, `req-uncovered-phase5` `AUD-02`, `unknown-id-phase5` `line 19`, `bad-req-phase5` `line 19`, `req-uncovered-phase6` `VID-06`, `unknown-id-phase6` `line 24`, `bad-req-phase6` `line 24`, `req-uncovered-phase7` `SCN-01`); no output line names SCN-02; the pristine toy gives `important-gates pass gates=22 lines=23` |
| numstat acceptance | `git diff --numstat 903276757b0d...` over the eight Phase 4-6 overlays | `24 0` (three appended lines each, none lost) |
| template judge | the registered row | the scenario must end `missing-fixture`, exit 13, detail `missing role=exec user`; a changed template fails the `runner-process` case the same way as any scenario, only a passing judgement is reported as the skip |
| text tripwire | first gate run of Task 3 | `text-claim-untagged: docs/owner-images.md:53` (the draft said the gate "passes" an image); reworded to "supplies", then green |

## Cost (D-34; the forward note of checkpoint C-C)

This plan adds one process case per preset (`runner-process.scenario.owner-exec-title`, which ends at the load with no machine run), two pure controls and a small awk and loop in the golden step. Local figures, laptop at load average 15 to 17 (so comparisons, not absolutes):

| Item | Before | After | Delta |
|------|--------|-------|-------|
| `golden` step in isolation, user plus sys cpu-s, three alternated old/new pairs | 0.56, 0.57, 0.56 | 0.68, 0.68, 0.57 | at most +0.12 |
| `tests-inventory` step, three alternated pairs | 0.81, 0.78, 0.74 | 0.73, 0.72, 0.80 | within noise |
| one new control (run-controls, good plus N controls) | 0.97, 0.99, 1.02 (good plus one) | 1.40, 1.60, 1.57 (good plus two) | about +0.43 to +0.58 per control, so up to about +1.2 for two |
| the template through `ivs-run` | | 0.00 s real (below the timer resolution) | none |
| process case, per case bound from C-B alternated pairs | | | at most 0.19 s dev and 0.06 s release (this one runs no machine) |

Bound for this plan: about 1.2 + 0.12 + 0.25, at most 1.6 local cpu-s. With the hosted-over-local factor 1.675 of checkpoint C-C that is at most 2.7 hosted cpu-s, a rise of at most 0.07 in `r` (divided by the `build_s` 37 of row 43528527276). On the C-C line (last-five median 8.02, projection 8.04 after 07-09) the projection after this plan is at most 8.11, under 8.3. The whole-gate cpu of this session moved from 178 to 331 cpu-s between two runs of an unchanged workload (t3 and t4 trees equal; machine load), so a whole-gate comparison would measure the load, not the plan; the isolation figures above are the measurement. Plan 07-11's 13 cases are pure `sh` refusals and helper cases with no machine run; at the same per-case bound (0.25 local cpu-s per case pair, 0.42 hosted) they add at most 5.4 hosted cpu-s, +0.15 in `r`, which would put the forward projection at 8.26, below 8.3 but close; plan 07-12 re-reads the last five hosted rows after the push (the stop rule stands). No lever is run or queued by this plan; lever (2) stays queued.

## Deviations from Plan

**1. [Interpretation] The SKIPPED line repeats the id.** The reason is the text after `# SKIP` in the TAP line, which the runner-process suite writes as `<id>: needs operator content (...)`, so the printed line reads `<preset>/<suite>/<id>: <id>: needs operator content ...`. The plan asks for `-- SKIPPED <preset>/<suite>/<id>: <reason>` with the reason kept, and the reason was not edited. The research note S5-7/S5-9 also suggests "(D11)" and "no claim" in the printed line; the suite's reason text was left as it is (changing it would be a change to `tests/runner-process-tap.sh`, outside this plan's file list), and the helper of plan 07-11 prints its own D11 line.

**2. [Interpretation] The controls use their own toy ids.** The plan says to overlay a toy scenario with `content exec user` whose toy row says `run`. The first control does that with a new scenario `toy-user` instead of editing `toy`, so no golden of the toy tree is involved and only its own token fires; the second sets the existing `toy-wall` row to `skip-ok`. The existing `golden-user-scenario-tracked` overlays `toy` with the same shape and now also reports the first token, as the plan predicted; confirmed, no edit.

**3. [Interpretation] Overlay lines carry no comment.** The acceptance criterion counts three appended lines per Phase 4-6 overlay in all since the baseline (two from plan 07-02, one here), so the one line `toy-owner control toy-control SCN-02` is appended without a comment line (the `_good` registry keeps the 07-02 comment above it).

**4. [Rule 1 - premise mismatch] "A tracked file at the local path fails the files and manifests steps" is only half true.** The research note S5-5 says so; the code says a tracked path under `refs/` fails the `files` step (`files-tracked-refs`) and the `manifests` step refuses a tracked output of an uncommitted manifest (`manifest-uncommitted-output-tracked`), but it does not look at `local_path`. The page states only the `files` step guard. No code change (the manifests rule for `local_path` is not in this plan's scope; the `files` step already covers every path under `refs/`).

**5. [Scope] The page names the audio-option refusal as not yet in.** The refusal of `--audio-raw`, `--psg-log-out` and `--audio-wav` for user content is plan 07-11 (D-31); the page says so and tells the owner not to pass those three options with owner images until it lands, instead of claiming the refusal.

No Rule 2 or 3 fixes; no auth gates; no package installs (T-07-SC); no C or cmake file was edited, so `tools/asm/asm.c`, `tools/pack/pack.c` and the oracle (derivation sections (a) to (j), the F-S literals) are untouched and no recipe digest was refreshed. The gcc warning check still ran (container gate, below).

## Decisions Made

- Print the SKIPPED lines before the FAIL block, so a failing gate shows them as well; they are not findings and carry no FAIL prefix.
- Treat an inventory row that is absent for a user-content scenario as "not skip-ok" (a finding), never as a silent pass.
- Leave SCN-02 Pending until the helper of plan 07-11 and the close-out of plan 07-12 are in; mark nothing complete on documentation alone.

## Open Items

- **Plan 07-11**: `tools/owner-run.sh` with its refusals (CI environment, in-tree path outside `refs/`, wrong size, missing lock), exit 77 and its SKIPPED line, the runner refusal of `--audio-raw`, `--psg-log-out` and `--audio-wav` for user content, and the gate tests; the page `docs/owner-images.md` already names the helper and the refusal as coming, so 07-11 should remove the "until it lands" sentences when it is done.
- **Plan 07-12**: SCN-02 close-out note (route documented, template skipped and counted, helper refuses every unsafe case, manifests labelled `no`; D11 not performed, no claim, no tier); re-read the last-five hosted median after the push of plans 07-10 and 07-11 (forward projection 8.26 at the per-case bound above, under 8.3); the SKIPPED line does not carry the "D11, no claim" wording of the research note (deviation 1).
- **SCN-02 stays Pending** in REQUIREMENTS.md.
- Carried unchanged: Phase 4 and 5 review deferrals, `sh tools/hygiene.sh --history` on earlier planning commits, the orchestrator-owned hosted CI record (no `gh` was called, `tools/ci-record.sh` was not run).

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=36/36 controls=356/356 faults=222/222 ... goldens=15 ... gates=76 lines=263 ... manifests=10 effective-yes=7 effective-no=3 ... pkg-selftest=17/17 ... tests-expected=4598 tests-executed=4594 tests-skipped=4 tests-missing=0 (0 FAIL lines; gate-time total=108 s on the tree of bdf0e50, machine load average about 16)
dash tools/gate.sh:    the identical summary line (md5 fb317b1c0ea0a73e02f02ecdf7d205d6 for both shells, gate-time and gate-cpu removed; 0 FAIL lines; total=106 s)
sh tools/gate-linux.sh: exit 0 on the tree of bdf0e50 (build/gate-linux.log ends gate: PASS; gcc 13.3.0), steps=36/36 controls=356/356 faults=222/222 goldens=15 gates=76 lines=263 manifests=10 effective-yes=7 effective-no=3 pkg-selftest=17/17 tests-expected=4598 tests-executed=4594 tests-skipped=4 tests-missing=0, 0 FAIL lines, four -- SKIPPED lines (gate-time total=508 s, emulated amd64 on a loaded host)
```

Counts at plan start were `controls=354/354 manifests=8 effective-no=1 tests-skipped=2 tests-expected=4596 gates=75 lines=261 doc-files=49`; at the end `doc-files=50`. `grep -c ' skip-ok ' data/expected/runner-scenarios.ids` prints 2; `grep -c '^assert ' data/scenarios/owner-exec-title.ivss` prints 0; `grep -v '^#' data/expected/important-gates.ids | grep -c ' SCN-02$'` prints 2; the nine-overlay loop prints the single line `1`; `grep -c 'owner-images.md'` prints 1 for each of the two procedure pages and `docs/runner.md`; `git ls-files refs` is empty.

## Known Stubs

None. The template is a deliberate template: it states no result by design (D-31) and is a counted skip, not a stub that hides a missing wiring; the owner helper it refers to is the named deliverable of plan 07-11.

## Threat Flags

None new. T-07-23: no image or digest is in the tree (`git ls-files refs` empty, no 64-hex string in the page, both manifests `redistributable: no` and uncommitted, `*.grom` and `*.img` ignored, the `files` step green). T-07-24: the consistency rule holds in both directions with two controls that fail for their own token and a third control that now reports the first token too. T-07-25: the page is in the project's own words and passed `text`, `glossary`, `denylist` and `hygiene` after one reworded sentence. T-07-SC: no installs.

## Self-Check: PASSED

`data/scenarios/owner-exec-title.ivss`, `docs/owner-images.md`, `fixtures/FX-owner-exec/manifest.json`, `fixtures/FX-owner-grom/manifest.json`, `tools/check/controls/golden-user-content-not-skip-ok/CHECK` and `tools/check/controls/golden-skip-ok-without-user-content/CHECK` exist and are tracked; commits `238c76f`, `adba1ea` and `bdf0e50` are ancestors of HEAD (`git rev-list --count 2215d7fddeba6effee9a6a64b4c7dfc971ac104f..HEAD` printed 3 before this file's commit); `git status --short` showed only `.planning/config.json` and `.planning/state.json` (not staged) before this commit; no file name contains the console brand word.
