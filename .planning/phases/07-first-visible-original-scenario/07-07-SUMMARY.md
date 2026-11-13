---
phase: 07-first-visible-original-scenario
plan: 07
subsystem: the four Lamp Tone scenarios with their hand-derived and self-recorded goldens, the metamorphic pair judge and its self-controls (W4a)
tags: [W4a, lamp-tone, scenarios, goldens, playable-guest-progress, golden-shape, metamorphic-judge, repeatability, D-01, D-04, D-05, D-06, D-20, D-21, D-22, D-23, D-24, A-06, A-08, A-16, A-20, T-07-17, T-07-18, V4, V6, V7, V8, V9, V12]
status: complete

requires:
  - phase: 07-first-visible-original-scenario
    provides: "plan 07-06: gate steps=36/36 controls=354/354 faults=222/222 goldens=11 tests-skipped=2 tests-missing=0, HEAD a34dda1 at the start of this plan; the Lamp Tone program, lamp-tone.ivsc and boot-stub-2.exec in the runner-process content directory; the frozen oracle (oracle commit 4e0febef675c2e81fe38d6c0caf353625218b1eb of plan 07-02, blind record 1621dae)"
provides:
  - "data/scenarios/demo-lamp-tone-{pass,idle,shift,swap}.ivss: the input, run-to and assert lines are the lines of the committed derivation blocks, line for line and in order (pass 55 asserts, idle 29, shift 55, swap 49; block command exits 0 for all four)"
  - "data/golden/demo-lamp-tone-pass.expect (tier playable-guest-progress, the first file of that tier; passes golden-shape) and demo-lamp-tone-idle.expect (tier original-diagnostic), assert lines equal the scenarios' in order (golden-expect-drift) and tied to F-S1..F-S6 by nine '# frame:' header lines (video-hash-ties 2 to 11)"
  - "data/golden/demo-lamp-tone-pass.digests (video, audio, input and psglog observed) and demo-lamp-tone-idle.digests (video, audio and psglog observed, input none), both written by ivs-run --update-golden only"
  - "data/expected/scenario-pairs.ids (2 lines), the pair judge in tests/runner-process-tap.sh and runner-process.pair.<idA>.<idB>; three self-controls runner-process.judge.{pair-video-equal,input-not-consumed,idle-audible}; repeat, slices and cwd cases for the pass scenario only; 12 new runner-process ids per preset"
  - "important-gates: three scn-differential mutant lines for SCN-01 (judge ids 7 to 10); docs/runner.md: conventions for a program that never halts, the Lamp Tone process cases and the second tier of .expect files"
affects: [plan-07-08-mutants, plan-07-09-consumer-replay, plan-07-12-closeout]

actuals:
  tokens: 12045
  tasks: 4
  commits: 3
plan_head_before: a34dda179042262b8f711dbd30f4abf8bb80611f
plan_head_after: d533c1f25fc73e85bde324c7315d4eb2409c7467

key-decisions:
  - "The scenario files and the two .expect files were produced mechanically from the derivation blocks (a scratch script that selects the fenced block between the asserts-begin and asserts-end markers and keeps the input, run-to and assert lines); no assert, literal or block was edited, and every scenario ended pass at its first execution"
  - "The pair judge decides 'idle' from the report (a scenario of the pair with input.events 0 must deliver output.audio.changes 0), not from a hard-coded scenario id, so a renamed idle scenario keeps the clause; the idle pair member of this plan is demo-lamp-tone-idle"
  - "The pair inventory lines are exactly the plan's: pass against idle differs in digest.video, digest.audio, digest.psglog, digest.input and digest.mem (equal=-), pass against shift is equal in digest.video only (differ=-); digest.images equality (S4-3) was left out"
  - "The self-controls build their fake report pairs from this run's own reports of demo-lamp-tone-pass and demo-lamp-tone-idle with one awk-replaced key each, and take the differ and equal lists from the live pass/idle line, so removing that line makes them fail as vacuous"

key-files:
  created:
    - data/scenarios/demo-lamp-tone-pass.ivss
    - data/scenarios/demo-lamp-tone-idle.ivss
    - data/scenarios/demo-lamp-tone-shift.ivss
    - data/scenarios/demo-lamp-tone-swap.ivss
    - data/golden/demo-lamp-tone-pass.expect
    - data/golden/demo-lamp-tone-pass.digests
    - data/golden/demo-lamp-tone-idle.expect
    - data/golden/demo-lamp-tone-idle.digests
    - data/expected/scenario-pairs.ids
  modified:
    - data/expected/runner-scenarios.ids
    - data/expected/tests/runner-process.ids
    - data/expected/important-gates.ids
    - tests/runner-process-tap.sh
    - docs/runner.md
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-13
---

# Phase 7 Plan 07: Lamp Tone scenarios and the metamorphic judge Summary

**The four Lamp Tone scenarios (`pass`, `idle`, `shift`, `swap`) reach every hand-derived value of the frozen oracle through the real `ivs-run` at the first execution (55, 29, 55 and 49 asserts), the pass `.expect` is the first file of tier `playable-guest-progress` and passes the shape lint, and a report-pair judge shows the response comes from the scripted input (pass differs from idle in the video, audio, write-log, input and memory digests; pass equals shift in the video digest; `input.consumed == input.events`; the idle run is silent), with three self-controls that fail a fake pair for their stated reason.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `8cb4329` | 1 (tracer) | `demo-lamp-tone-pass.ivss`, its `.expect` (tier `playable-guest-progress`) and `.digests`, the `runner-scenarios.ids` row and `runner-process.scenario.demo-lamp-tone-pass` (goldens 11 to 13) |
| `2188b4a` | 2 | `-idle` (scenario, `.expect` tier `original-diagnostic`, `.digests`), `-shift` and `-swap` scenarios (no golden of their own), three rows and three ids (goldens 13 to 15) |
| `d533c1f` | 3 | `scenario-pairs.ids`, the pair judge and three self-controls in `tests/runner-process-tap.sh`, repeat/slices/cwd for the pass scenario, registry lines, `docs/runner.md` |
| the commit that adds this file | | this SUMMARY, STATE.md, ROADMAP.md |

Task 4 changed no file (three gates and the freeze, block and golden commands only), so it has no commit.

Oracle order (D-23, A-08): the oracle commit `4e0febef675c2e81fe38d6c0caf353625218b1eb` (plan 07-02) and the blind second-read record (`1621dae`) both precede this plan's first commit `8cb4329` in `git log` (`git merge-base --is-ancestor 4e0febef675c2e81fe38d6c0caf353625218b1eb 8cb4329` exits 0). `git diff --name-only 903276757b0dbd901b15dbc00b9813089c1b578e -- data/golden` lists exactly the four new files (`demo-lamp-tone-pass.{digests,expect}`, `demo-lamp-tone-idle.{digests,expect}`) and `git diff --quiet 903276757b0dbd901b15dbc00b9813089c1b578e` over the eleven earlier golden files exits 0.

## What was built

- **Scenarios** (D-01, D-05, D-06, D-23, A-16): each file has a comment block, `ivss 1`, the id, `profile intv-2609-ntsc-family-p1`, `content exec boot-stub-2`, `content grom zero-grom`, `content cart lamp-tone.ivsc`, `capture video 0 13`, `capture psglog`, `capture audio`, then the `input`, `run-to` and `assert` lines of the derivation block. `pass`: six input events (100,000 to 700,000, both controller words each), four `run-to` bounds (200,000, 440,000, 640,000, 806,568 ticks) with the default stop and `halted == no` in every step, 55 asserts. `idle`: no input line, one bound, 29 asserts. `shift`: every event +2,000 ticks, the pass assert lines unchanged. `swap`: `c0` and `c1` exchanged per event, three steps, 49 asserts. All at most 56 asserts (A-06).
- **Expectations** (D-20, D-24, A-08, A-16): `demo-lamp-tone-pass.expect` (tier `playable-guest-progress`, oracle `independent-derivation`, six `# frame: <n> <F-S id>` header lines, the derivation path, the excluded-on-purpose list: machine time, registers, R7, flags, the stack word, the exact interrupt routine duration and re-arm delay, the log tick, tone phase; the shared-misreading caveat) and `demo-lamp-tone-idle.expect` (tier `original-diagnostic`, three `# frame:` lines all F-S1). The body is the block's assert lines with the block's `run-to` lines as `# step:` comments (comments are ignored by `golden-expect-drift`).
- **Repeatability digests** (D-20, T-07-18): written only by `ivs-run --update-golden` with the dev build per `docs/runner.md`; the `# domains:` line of the pass file is `video=observed audio=observed input=observed psglog=observed`, of the idle file `video=observed audio=observed input=none psglog=observed`; pass: `input.events 6`, `input.consumed 6`, `output.video.frames 13`, `output.psglog.records 30`, `output.audio.changes 279`, `video.chain.check pass`; idle: `output.audio.changes 0`, `output.psglog.records 6`. The `docs/runner.md` list of tracked goldens gained the two commands.
- **Pair judge** (D-23, D-06, A-16, T-07-17): `check_pair` in `tests/runner-process-tap.sh` reads the two reports this run made and appends findings by token: `pair-no-report`, `pair-missing-key`, `pair-not-different`, `pair-not-equal`, `input-not-consumed`, `idle-audible`. The loop over `data/expected/scenario-pairs.ids` also refuses a malformed line (columns, key-list syntax, `-` with `-`, an id that is not a deterministic line of `runner-scenarios.ids`, a pair of one scenario) as an `inventory` finding. One case per line: `runner-process.pair.demo-lamp-tone-pass.demo-lamp-tone-idle` and `...-shift`.
- **Self-controls**: `runner-process.judge.pair-video-equal` (idle report with the pass `digest.video`, expects `pair-not-different`), `.input-not-consumed` (pass report with `input.consumed` one below `input.events`), `.idle-audible` (idle report with `output.audio.changes 3`).
- **Cost-capped repeatability**: `runner-process.repeat.demo-lamp-tone-pass`, `.slices.` (`--slice-ticks` 1 and 7) and `.cwd.` for the pass scenario only; dev equals release apart from `host.calls` (the golden step's dev-versus-release check covers all four new reports).
- **Registry and docs**: three `scn-differential mutant runner-process.judge.<slug> SCN-01` lines (`grep -c 'runner-process.judge.' data/expected/important-gates.ids` 7 to 10); `docs/runner.md` gained a subsection of scenario conventions for a never-halting program (events in ticks with both controller words, at least 10,000 ticks from a frame end, bounds mid-frame, `halted == no` instead of a halt stop, at most 56 asserts, no `tick=` in write-log asserts, `audio at` only on constant levels, the settling rule), the Lamp Tone bullet of the process-level tests, and the second tier of `.expect` files.

## Results

| Scenario | Outcome | Asserts | Notes |
|----------|---------|---------|-------|
| `demo-lamp-tone-pass` | pass, exit 0 | 55 of 55 | `input.consumed 6`, six frame hashes equal F-S2, F-S3, F-S4, F-S5, F-S1, F-S6 |
| `demo-lamp-tone-idle` | pass, exit 0 | 29 of 29 | `input.events 0`, silence, init-only log |
| `demo-lamp-tone-shift` | pass, exit 0 | 55 of 55 | video digest equal to pass |
| `demo-lamp-tone-swap` | pass, exit 0 | 49 of 49 | |

The first execution of each scenario passed; the program, the scenario headers, the capture lines and the oracle were not edited to make an assert hold. Gate steps at the tracer: golden step with the first `playable-guest-progress` file (no `golden-shape` finding), `video-hash-ties` 2 to 8 after the pass file and to 11 after the idle file, dev and release reports byte-equal.

Pair results (suite `runner-process`, both presets): `pass`/`idle` holds (the five digests differ; `input.consumed 6 == input.events 6`; idle `output.audio.changes 0`), `pass`/`shift` holds (`digest.video` equal; the shifted run consumes 6 of 6).

Judge trials on a scratch copy of the tree (nothing committed): (1) the pass/idle line with `digest.images` listed as a `differ` key ends `not ok` with `pair-not-different` on `digest.images`; (2) the `pair-not-different` finding disabled makes `runner-process.judge.pair-video-equal` fail as `judge-vacuous`; (3) the `input-not-consumed` and `idle-audible` findings disabled make their two self-controls fail as `judge-vacuous`. The judge and each self-control are therefore shown able to fail.

## Cost against the S-4 ceiling (D-34, A-20)

Cases added per preset: 12 (4 scenarios, 2 pair judgements, 3 repeatability, 3 judge self-controls); executed per preset 71 to 83, skipped 1 unchanged (`tests-expected` 4546 to 4570, 12 per preset). The S-4 overage of 6 is the one stated in the plan objective; this plan is within its own 12.

Local suite time of `tests/runner-process-tap.sh` (median of three runs each, a laptop with other work on it, quiet during these runs; `before` is the tree of `a34dda1` with its own script, `after` the tree of `d533c1f`):

| Preset | Before | After | Delta |
|--------|--------|-------|-------|
| dev | 12.03 s (11.22 to 12.78) | 14.42 s (14.33 to 14.62) | +2.4 s |
| release | 6.24 s (6.21 to 6.31) | 6.78 s (6.73 to 6.81) | +0.5 s |

`ctest --preset dev -R '^runner-process$'` took 12.9 s in one measured run during the gate work. One run of the pass scenario takes 0.06 s on dev and 0.01 s on release, the same under `--slice-ticks` 1 and 7 (single timed runs), and the plan adds 8 `ivs-run` invocations per preset (4 scenario cases, the second run, two slice runs and the directory run); the rest of the suite time is the per-run harness (watchdog arming, report judgement). The dev delta of +2.4 s is larger than that product and the dev spread before the plan was 1.6 s, so part of it is run-to-run noise on a shared laptop; no single new case is a hot spot. These figures are the input of the cost checkpoint C-B of plan 07-08; hosted CPU seconds are measured by the orchestrator, not here.

## Deviations from Plan

**1. [Interpretation] The idle clause of the judge keys on the report, not on the id.** The plan states `output.audio.changes 0` "for `demo-lamp-tone-idle`"; the judge applies it to any scenario of a pair whose report has `input.events 0`. Equivalent for the one idle scenario, and robust to renaming; the self-control `idle-audible` uses the real idle report. Recorded in the header of `scenario-pairs.ids` ("when it has none").

**2. [Interpretation] `.expect` bodies keep the block's step boundaries as `# step: run-to <n> ticks` comment lines.** The plan says "assert lines verbatim"; the comments are mechanical, ignored by `golden-expect-drift` (checked: the assert lines compare equal with `cmp`), and show which `run-to` bound each group belongs to. The `# frame:` lines of the block sit in the `.expect` header (as the plan says) and not in the scenario.

**3. [Process] The container gate ran twice on the same tree.** The first run (`sh tools/gate-linux.sh`, exit 0, `gate-time total=311`) was started in the background and its output kept in a scratch log; a second run was started by a command meant only to read the first run's exit code. The second run rewrote `build/gate-linux.log` (total=259), also ended `gate: PASS`, and its summary line is byte-identical to the first (same md5). No tracked file changed between the two. Both are reported below; nothing is hidden.

**4. [Scope] No `digest.images` equality in the pair lines.** S4-3 lists it; the plan's `must_haves` do not. The lines are exactly the plan's. It can be appended later without a code change (the grammar already takes any report key).

No Rule 1 to 3 fixes; no auth gates; no package installs (T-07-SC); no C or cmake file was edited (the shell suite script was), so the container gate needed no gcc finding fixed; `tools/asm/asm.c` and `tools/pack/pack.c` are unchanged, so no recipe digest was refreshed.

## Decisions Made

- Keep the oracle frozen: every scenario and expectation line is a copy of the committed derivation blocks, extracted by script, and the three freeze commands (block, golden-unmoved, oracle freeze) were run again at the end of the plan and exit 0.
- Put the pair judge in the same suite and the same scenario loop (one more shell function and one loop), without a new step, job, control directory or dependency; the self-controls are in-suite ids like the four existing judge self-controls.
- Make a bad pair line a finding of the pair case itself (`inventory`) rather than a separate gate rule.

## Open Items

- Plan 07-08: `run_mutant` copies only `boot-stub.exec` into a mutant directory; the Lamp Tone mutants need `boot-stub-2.exec` as well, and the stub-2 mutants use `ivs_pack --exec --patch` (PATTERNS "W4b"); section (k) of the derivation has the patch word addresses and the N = 1,023 choice for `SCN-REARM-LONG`. The cost checkpoint C-B reads the 12 cases and the times above; the lever rule is in plan 07-08 Task 3.
- Plan 07-09: the installed-consumer image list and `pkg-install.sh` carry neither `boot-stub-2.exec` nor `lamp-tone.ivsc` yet (carried from 07-05 and 07-06).
- Plan 07-12: the claims CL-0023 and CL-0024 name `data/golden/demo-lamp-tone-pass.expect`; the self-recorded digests are cited as repeatability only. The tier word `playable` joins the text tripwire there; the lines of `docs/runner.md` that name `playable-guest-progress` carry `[nc]`.
- SCN-01 stays Pending: the scenarios, goldens, shape lint and judge exist and pass, but the mutants (07-08), the installed-consumer replay (07-09) and the claims (07-12) are not yet in.
- The `.digests` assert lines for write-log records carry the record tick (`got tick=...`); this is self-recorded repeatability only and no assert uses a tick.
- Carried unchanged: Phase 4 and 5 review deferrals, `sh tools/hygiene.sh --history` on earlier planning commits, the orchestrator-owned hosted CI record (no `gh` was called).

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=36/36 controls=354/354 faults=222/222 ... goldens=15 reports=23 ... gates=74 lines=247 ... video-hash-ties=11 ... tests-expected=4570 tests-executed=4568 tests-skipped=2 tests-missing=0 (0 FAIL lines; gate-time total=69 s)
dash tools/gate.sh:    the identical summary line (md5 ae23b13e267fab17686ba179740efbf9 both; 0 FAIL lines; wall 61 s)
sh tools/gate-linux.sh: exit 0 twice on the same tree, build/gate-linux.log ends gate: PASS (x86_64, gcc 13.3.0, cmake 3.28.3), steps=36/36 controls=354/354 faults=222/222 goldens=15 reports=23 listings=6 manifests=8 tests-expected=4570 tests-executed=4568 tests-skipped=2 tests-missing=0, 0 FAIL lines (gate-time total=311, then 259; both summary lines md5 b73db4d8f6a9eee380e6339336b1f4c7)
```

Counts at plan start were `goldens=11 tests-expected=4546`; `gates` rose 73 to 74 (the new gate name `scn-differential`), `lines` 244 to 247 (three registry lines). Commands re-run at the end: the Task 2 block command exits 0 (all four scenarios and both `.expect` files equal their derivation blocks), the freeze command exits 0, the earlier-golden diff against `903276757b0dbd901b15dbc00b9813089c1b578e` exits 0, and the awk count of the plan's `runner-process` ids prints 12.

## Known Stubs

None. The scenarios and goldens hold derived values only; the `.digests` files are generated records.

## Threat Flags

None new. T-07-17: the idle scenario asserts the idle literal, the init-only log and silence; the judge requires differing digests, `input.consumed == input.events` and an idle `audio.changes` of 0; the three self-controls and the shape lint on the pass file are in. T-07-18: the digests were written by `--update-golden` only, contain no host path (grep for the home, scratch and tmp prefixes in the four golden files prints nothing), dev equals release, and the hygiene step is green. T-07-SC: no installs.

## Self-Check: PASSED

The nine created files exist (`data/scenarios/demo-lamp-tone-{pass,idle,shift,swap}.ivss`, `data/golden/demo-lamp-tone-{pass,idle}.{expect,digests}`, `data/expected/scenario-pairs.ids`); commits `8cb4329`, `2188b4a` and `d533c1f` are ancestors of HEAD (`git rev-list --count a34dda179042262b8f711dbd30f4abf8bb80611f..HEAD` printed 3 before this file's commit); `grep -c '^assert ' data/golden/demo-lamp-tone-pass.expect` prints 55 and for the idle file 29; `grep -c '^demo-lamp-tone-' data/expected/runner-scenarios.ids` prints 4; `grep -c '^input ' data/scenarios/demo-lamp-tone-idle.ivss` prints 0; `grep -v '^#' data/expected/scenario-pairs.ids | grep -c .` prints 2; `grep -c 'runner-process.judge.' data/expected/important-gates.ids` prints 10; no file contains the console brand word (the `files` step, and `grep -rli` over the new files, find none).
