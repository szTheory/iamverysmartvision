---
phase: 07-first-visible-original-scenario
plan: 11
subsystem: the SCN-02 owner-local route, part two - runner refusal of derived audio and log output for user content, the owner-run helper with tested refusals, skip and identity cases, and the helper judge self-control (W6b)
tags: [W6b, SCN-02, owner-run, runner-refusal, audio-raw, psg-log-out, audio-wav, skip-77, expect-sha256, identity-mismatch, runner-process, important-gates, D-30, D-31, D-32, D-36, T-07-26, T-07-27, T-07-28, T-07-29, V18, V19]
status: complete

requires:
  - phase: 07-first-visible-original-scenario
    provides: "plan 07-10: gate steps=36/36 controls=356/356 faults=222/222 manifests=10 tests-skipped=4 tests-missing=0, HEAD 1de640d at the start of this plan; the template scenario owner-exec-title.ivss with its marker comment line, the owner-images page, the non-redistributable manifests, the SKIPPED line of the inventory"
provides:
  - "runner/main.c: --audio-raw, --psg-log-out and --audio-wav are refused for a scenario with a content user line (exit 14, one stderr line `<option> refused: content user`, no report, no file), before any storage or output is created; three process cases runner-process.refuse-user.*"
  - "tools/owner-run.sh (80 lines, POSIX sh and dash): the owner-local helper; refuses a CI environment, bad arguments, an in-tree path outside refs/, a wrong size and a missing or malformed lock (exit 2, one line each); SKIPPED line and exit 77 when no image exists; otherwise inserts expect-sha256 lines into a temp copy of the template and starts ivs-run with no output option; --record writes the lock"
  - "nine runner-process.owner-run.* cases and the self-control runner-process.judge.owner-run-refusal; tests-expected 4598 to 4624; one important-gates line (gates 76 to 77, lines 263 to 264)"
  - "docs/owner-images.md and docs/runner.md: the helper's final usage, check order, exit codes, banner and refusals; the `until it lands` sentences of plan 07-10 are gone"
affects: [plan-07-12-closeout]

actuals:
  tokens: 8023
  tasks: 3
  commits: 3
plan_head_before: 1de640dd621ed7d754fa3420bd95606fb2e13c42
plan_head_after: a147b63e452e0cf8afd5b3d5fb550817af165228

key-decisions:
  - "The three audio refusals sit after the frame and video refusal and before the golden refusal, so a scenario with user content and several of the options names the first of the fixed order --audio-raw, --psg-log-out, --audio-wav; frame and video options keep their precedence and no existing text moved"
  - "The helper never writes to the work tree except refs/owner-images.sha256 under --record, and --record refuses to replace an existing lock, so the lock stays the owner's deliberate statement and is not reset by a run"
  - "The helper passes no output-file option to the runner; the runner refuses all six derived outputs for user content as well, so the rule holds even if a later edit adds one"
  - "The helper tests use run_one (watchdog) like every other process case but compare output with shell builtins, to keep the per-case cost small; each case works on a fresh scratch copy of the helper outside the work tree and never touches the real refs/"
  - "SCN-02 stays Pending in REQUIREMENTS.md: plan 07-12 closes SCN-01 and SCN-02 together on its verification record (as for SCN-01 through plans 07-01 to 07-09)"

key-files:
  created:
    - tools/owner-run.sh
    - .planning/phases/07-first-visible-original-scenario/07-11-SUMMARY.md
  modified:
    - runner/main.c
    - tests/runner-process-tap.sh
    - data/expected/tests/runner-process.ids
    - data/expected/important-gates.ids
    - docs/owner-images.md
    - docs/runner.md
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-13
---

# Phase 7 Plan 11: owner-local helper and the runner refusal of derived audio and log output Summary

**`ivs-run` now refuses `--audio-raw`, `--psg-log-out` and `--audio-wav` for a scenario that names user content, and `tools/owner-run.sh` (80 lines) loads an owner's EXEC and GROM only from outside CI, from `refs/` or outside the work tree, with exact sizes and a well-formed digest lock that the core itself checks at load; every refusal, the exit-77 skip and the identity mismatch are tested in the gate with zero-filled files made by the test, no owner image exists anywhere and D11 is not performed.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `1cb40b0` | 1 (tracer) | the runner refusal block, three `runner-process.refuse-user.*` cases, ids, `docs/runner.md` |
| `813a5aa` | 2 | `tools/owner-run.sh`, nine `runner-process.owner-run.*` cases, the self-control, ids |
| `a147b63` | 3 | `docs/owner-images.md`, `docs/runner.md` helper text, the important-gates line |
| the commit that adds this file | | this SUMMARY, STATE.md, ROADMAP.md |

## What was built

- **Runner refusal** (D-31, T-07-29): one block in `runner/main.c` after the frame and video refusal, before the golden refusal and before any storage is allocated. Any role of kind user plus any of the three output paths gives `ivs-run: <option> refused: content user`, exit 14, no report, no file; `free(scn)` is the only cleanup (text was already freed, no golden buffer exists yet). `grep -o 'refused: content user' runner/main.c | grep -c .` prints 6 (was 3). No `ivsr` change and no golden moved (`git log ... -- data/golden` shows no `(07-11)` commit).
- **Tracer cases**: `refuse_user <slug> <option> <path>` in `tests/runner-process-tap.sh` runs the existing `operator-user.ivss` (`content exec user`) with one option, requires exit 14, not hung, empty stdout, stderr equal to the one expected line and no file. The `audio-raw` case also gives all three options at once (written in reverse order) and requires the `--audio-raw` line and no file of the three.
- **Helper** `tools/owner-run.sh`, 80 lines. Root from `$0` (`cd -P` of the script's directory plus `..`), no `git rev-parse` and no working-directory lookup (`grep -c -E 'git rev-parse|PWD'` prints 0, `eval` count 0). Order: environment (`CI` or `GITHUB_ACTIONS` non-empty), arguments, both images absent (SKIPPED line on stdout, exit 77, before any path or file check), in-tree check by physical directory (`cd -P` in a subshell; inside the root only `refs/` is allowed), regular file and exact size (EXEC 8192, GROM 2048; one missing image lands here), lock (`--record` write or read; every line must be 64 lower-case hex digits, two spaces, `exec` or `grom`, each role once), runner and template existence, temp copy by `mktemp -d` outside the tree (refused when `TMPDIR` is inside it), `awk` insertion of the two `expect-sha256` lines after the marker (two such lines must result), banner `OWNER-LOCAL RUN (D11): inputs are the owner's, not redistributable, not run in CI, not evidence, no claim`, `ivs-run --exec ... --grom ...` with no output option, `trap` removes the copy on exit. The exit code of `ivs-run` is the helper's exit code otherwise.
- **Helper tests** (`tests/runner-process-tap.sh`, all hermetic): `or_scratch` makes a fresh `mktemp -d` root under the suite's own temp directory, copies the helper (`cp "$root/tools/owner-run.sh" ...`, the only line naming the real helper; the grep counts print 1 and 1), copies the template, links `build/release/runner/ivs-run` to the runner under test and creates its own `refs/`; `or_run` runs it under `env CI=... GITHUB_ACTIONS=... TMPDIR=<dir outside every scratch root>`; `or_judge` requires exit code, not hung, and exactly the one stdout line and the one stderr line; `or_end` requires nothing left in the helper's `TMPDIR` and removes the scratch root. Cases: `usage` (no arguments, bad preset), `ci-env` and `github-actions-env` (nonexistent paths, so an unchecked environment would skip with 77), `in-tree-path` (an image under `data/`, and a `refs/../data/` spelling), `wrong-size` (8191 EXEC, 2049 GROM, a missing GROM with a good EXEC: exit 2 and not the skip), `missing-lock` (plus `--record`, see below), `bad-lock-line` (uppercase digit, 63 digits, one space, role `cart`, role missing: five runs, one token each), `no-images-skip` (two runs, the second with a directory that does not exist), `identity-mismatch` (zero-filled files and a lock of 64 zero digits: exit 14, banner first, `outcome.detail load-refused role=exec status=IVS_E_DIGEST_MISMATCH`, `cpu.run_state unreset`, empty stderr, and the scratch root holds only its own six files). Ten ids registered (nine plus the self-control).
- **`--record` coverage**: the `missing-lock` case also runs `--record` with `TMPDIR` pointing at a directory that does not exist, so the helper writes the lock and stops with `cannot make a temporary directory` before anything is executed; the lock must equal two digests pinned in the test (SHA-256 of 8192 and of 2048 zero bytes, taken from `shasum -a 256`, an instrument independent of `cmake -E sha256sum`); a second `--record` must refuse (`refs/owner-images.sha256 exists: remove it to record again`).
- **Self-control** `runner-process.judge.owner-run-refusal`: a fake helper in its own scratch root exits 0 where the CI refusal is required; `or_judge` must report `owner-run-exit`. Registered in `important-gates.ids` as `scn2-owner-run mutant runner-process.judge.owner-run-refusal SCN-02`.
- **Docs**: `docs/owner-images.md` has a section "The helper" (usage, the seven checks in order, the banner, exit codes, `--record`, how the gate tests it) and one paragraph for the six refused runner outputs; layer 2 now says the lock is 64 lower-case hex digits and that `--record` writes it. `docs/runner.md` gets the refusal sentence on each of the three option rows, the helper paragraph beside the listening helper and the two test descriptions. The text step passed (no claim word needed a tag).

## Induced failures and checks that were shown able to fail

The helper cases were run through an extracted harness (the setup of the suite plus the owner section) against a mutated copy of the helper in a mirror root; each mutant failed exactly the cases that name its defect:

| Mutant of the helper | Failing cases |
|----------------------|---------------|
| CI check removed | `ci-env`, `github-actions-env` (exit 77, the skip, instead of 2) |
| in-tree refusal removed | `in-tree-path` |
| GROM size 2048 changed to 2049 | `wrong-size`, `missing-lock`, `bad-lock-line`, `identity-mismatch` (all reach the size check first) |
| `${#1} -eq 64` removed from the digest check | `bad-lock-line` (the 63-digit lock reaches the runner, which rejects the scenario text: a second line of defence) |
| `&&` of the absent check changed to `\|\|` (skips when only one image is missing) | `wrong-size` |
| `expect-sha256` insertion removed | `identity-mismatch` (the helper refuses the template as marker-less, exit 2, instead of exit 14) |
| `--audio-raw` given to the runner | `identity-mismatch` (stderr not empty, no digest refusal reported) |
| copy not removed on exit | `identity-mismatch` (`owner-run-tmpdir`: a leftover in the helper's `TMPDIR`) |
| digest class widened to upper case | `bad-lock-line` |
| `--record` overwrites an existing lock | `missing-lock` |
| `--record` swaps the two roles | `missing-lock` (`owner-run-record`) |
| temp-directory message changed | `missing-lock` |

For the tracer: the stale release `ivs-run` (built before the refusal) given `--audio-raw` and a user-content scenario printed a report and created the file, so `runner-process.refuse-user.audio-raw` would fail against it. The self-control is the CI-refusal fake of the table above.

## Cost (D-34; the forward note of plans 07-09 and 07-10)

Measured in isolation (the suite's setup alone against the setup plus the section, alternated pairs, user plus sys local cpu-s, laptop load average 16 to 28 so comparisons, not absolutes):

| Item | Dev preset | Release preset |
|------|-----------|----------------|
| ten owner-run cases (about 19 helper runs, 10 scratch roots) | 1.22, 1.19, 1.32 | 1.32, 1.11, 1.05, 1.26 |
| three `refuse-user` cases (four runner runs) | 0.46, 0.49, 0.65, 0.52 | 0.05, 0.10, 0.07, 0.10 |
| one `run_one` of a trivial command | about 0.016 | |
| one scratch root (mktemp, mkdir, two cp, ln, rm) | about 0.032 | |

Before the first cut the ten cases cost about 1.8 to 2.0 per preset; the cuts were comparison by shell builtins instead of `cmp` and `ls` per run, one leftover check per case instead of per run, and three variants dropped (a one-argument usage run, a directory as an image, a trailing blank lock line). Total for this plan about 3.0 local cpu-s over both presets; at the hosted-over-local factor 1.675 of checkpoint C-C that is about 5.0 hosted cpu-s, a rise of about 0.14 in `r` over `build_s` 37 (row 43528527276). On the C-C line (last-five median 8.02; at most 8.11 after plan 07-10) the projection after this plan is at most about 8.25, under the trigger 8.3 and close to it (the plan 07-10 forward note said 8.26). This plan adds no step, no control and no job; lever (2) stays queued and is not run; plan 07-12 re-reads the last five hosted rows after the push (the stop rule stands). Gate-time on the final tree under that load: sh 151 s, dash 237 s, container 540 s (emulated amd64); the plan 07-10 figures were 108, 106 and 508 s on a quieter machine, so these show the load, not the plan.

## Deviations from Plan

**1. [Interpretation] `--record` semantics.** The plan names the option and says only "lock read or `--record` write". It writes the lock from the two images (by `cmake -E sha256sum`), refuses to replace an existing lock and then goes on with the run. The refusal to replace keeps the lock the owner's deliberate statement. Tested inside the `missing-lock` case (above) without adding an id.

**2. [Rule 2 - missing precondition checks] Four refusals beyond the plan's list**, each one stderr line and exit 2: a missing runner (`no runner build/<preset>/runner/ivs-run`), a missing template, a `TMPDIR` inside the work tree (otherwise the temp copy of an owner run could land in the tree) and a template without its marker (the insertion must produce exactly two `expect-sha256` lines; T-07-28). A lock must also name each role once. `--preset` outside `dev` and `release` is a usage refusal.

**3. [Interpretation] The helper adds no line of its own to a core refusal.** For a lock that does not match, the helper hands on the runner's report and exit code 14; the refusal is in `outcome.detail load-refused role=exec status=IVS_E_DIGEST_MISMATCH` (reason 24). The test pins the status name, not the reason number.

**4. [Scope] `docs/runner.md` is edited in Task 3 as well as Task 1.** The plan's Task 3 `<files>` names `docs/owner-images.md` and the registry only; `docs/runner.md` is in the plan's `files_modified`, and the helper paragraph and the test descriptions belong with the helper.

**5. [Process] The container gate ran after Task 1** (C changed) before its commit, as the dispatch asked, and again on the final tree; Task 2 (shell only) had sh and dash gates before its commit, and its files are part of the final container run.

**6. [Cost] Three test variants were dropped for cost** (item 5 of the cost section); no refusal is left without a test, each remaining defect class has its own run.

No Rule 1 or 3 fixes; no auth gates; no package installs (T-07-SC); `tools/asm/asm.c`, `tools/pack/pack.c` and the oracle (derivation sections (a) to (j), the F-S literals) are untouched, so no recipe digest was refreshed.

## Decisions Made

- Keep the three refusals in `ivs-run` even though the helper gives the runner no output option: the runner enforces the rights rule for any caller.
- A symbolic link inside `refs/` that points into the work tree outside `refs/` is not detected: the in-tree check is by the physical directory of the path, as the plan states. The owner who links files takes responsibility for the target's location (open item below).
- Do not mark SCN-02 complete: plan 07-12 closes it with the verification record.

## Open Items

- **Plan 07-12**: close SCN-02 on the recorded evidence (route documented, template skipped and counted, helper refuses every unsafe case with tests, runner refuses all six derived outputs for user content, manifests labelled `no`; D11 not performed, no claim, no tier). The SKIPPED line of the inventory does not carry the "D11, no claim" wording (plan 07-10 deviation 1); the helper's own line does. Re-read the last five hosted `r` rows after the push of plans 07-10 and 07-11 (projection at most about 8.25, trigger 8.3).
- **Known gap of the helper, not closed**: the in-tree check follows the plan (physical directory of the path). A symlink in `refs/` to an image elsewhere inside the work tree passes it. A later hardening could refuse symlinked image files or resolve the target; no test exists for it.
- **`--record` is trust on first use**: it records whatever bytes are in the files at that moment. That is the intent (the owner states which bytes are meant) and is written on the page.
- **`--preset dev|release` runner age** is not checked: the helper uses whatever `build/<preset>/runner/ivs-run` holds.
- Carried unchanged: Phase 4 and 5 review deferrals, `sh tools/hygiene.sh --history` on earlier planning commits, the orchestrator-owned hosted CI record (no `gh` was called, `tools/ci-record.sh` was not run).

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=36/36 controls=356/356 faults=222/222 ... goldens=15 ... gates=77 lines=264 ... manifests=10 effective-yes=7 effective-no=3 ... pkg-selftest=17/17 ... tests-expected=4624 tests-executed=4620 tests-skipped=4 tests-missing=0 (0 FAIL lines, four -- SKIPPED lines; gate-time total=151 s on the tree of a147b63, machine load average 16 to 28)
dash tools/gate.sh:    the identical summary line (md5 b767673a2888b9fd314f851fd138fb90 for both shells, gate-time and gate-cpu removed; 0 FAIL lines; total=237 s)
sh tools/gate-linux.sh: exit 0 on the tree of a147b63 (build/gate-linux.log ends gate: PASS; gcc 13.3.0, cmake 3.28.3), steps=36/36 controls=356/356 faults=222/222 gates=77 lines=264 manifests=10 pkg-selftest=17/17 tests-expected=4624 tests-executed=4620 tests-skipped=4 tests-missing=0, 0 FAIL lines, four -- SKIPPED lines (gate-time total=540 s, emulated amd64 on a loaded host)
```

After Task 1 (before its commit): sh and dash `gate: PASS` with the identical summary line (md5 09cd4ec028109d66157bf90fdd046c4c), `tests-expected=4604`, and the container gate `gate: PASS` (gcc 13.3.0, 541 s). After Task 2 (before its commit): sh and dash identical (md5 c2a949831543e8acd8ff3fd21b703eb0), `tests-expected=4624`.

Counts at plan start were `controls=356/356 faults=222/222 tests-skipped=4 tests-expected=4598 gates=76 lines=263`; at the end `gates=77 lines=264 tests-expected=4624`, everything else equal. `ls -lAR refs` of the real directory (present, with the owner's research files, no lock and no image) hashed identically before and after the `runner-process` suite; `git ls-files refs` is empty; `git log 903276757b0dbd901b15dbc00b9813089c1b578e..HEAD -- data/golden` has no `(07-11)` commit; `CI= GITHUB_ACTIONS= sh tools/owner-run.sh /nonexistent-a /nonexistent-b` exits 77.

## Known Stubs

None. The helper, the refusals and the tests are complete for the plan's scope; the template scenario it uses states no result by design (plan 07-10).

## Threat Flags

None new. T-07-26: the CI refusal is tested for both variables and ahead of the skip, and the gate never passes an image. T-07-27: the physical-path comparison is tested for a plain path and a `..` spelling; the symlink gap is recorded above. T-07-28: lock lines are validated before use, the temp copy lives outside the tree, no `eval`, and `bad-lock-line` plus `identity-mismatch` plus the marker check cover injection and drift. T-07-29: the three runner refusals have one case each. T-07-SC: no installs.

## Self-Check: PASSED

`tools/owner-run.sh` exists, is tracked and has 80 lines; commits `1cb40b0`, `813a5aa` and `a147b63` are ancestors of HEAD (`git rev-list --count 1de640dd621ed7d754fa3420bd95606fb2e13c42..HEAD` printed 3 before this file's commit); `git status --short` showed only `.planning/config.json` and `.planning/state.json` (not staged) before this commit; no file name contains the console brand word; no absolute home, scratch or temp path and no digest of any EXEC or GROM image is in the tracked diff (the two digests pinned in the test are those of zero-filled files).
