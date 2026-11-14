---
phase: 07-first-visible-original-scenario
plan: 09
subsystem: the installed-consumer fourth replay of the Lamp Tone pass scenario, its pkg-install wiring and self-tests, and cost checkpoint C-C (W5)
tags: [W5, installed-consumer, lamp-tone, frame-loop, region-diff, audio-decode, psglog-decode, pkg-selftest, checkpoint-C-C, no-lever, D-21, D-22, D-26, D-34, T-07-21, T-07-22, V6, V7, V11, V20]
status: complete

requires:
  - phase: 07-first-visible-original-scenario
    provides: "plan 07-08: gate steps=36/36 controls=354/354 faults=222/222 goldens=15 pkg-selftest=15/15 tests-skipped=2 tests-missing=0, HEAD 290f057 at the start of this plan; the pass scenario, its golden data/golden/demo-lamp-tone-pass.digests, the derivation sections (a) to (k), the release images boot-stub-2.exec and lamp-tone.ivsc"
provides:
  - "tests/installed-consumer/main.c: the fourth replay (prefix scn.) through the installed headers and archive only: six image arguments (argc 7), stub 2 as the EXEC role, zero GROM, the program as the cartridge, the six events, four run-to steps, guest words at each stop, the 9 scn.digest lines, 13 slot headers, 6 lamp pictures, 7 declared pair diffs, 4 pixel probes, audio windows and audio-at ticks, 30 write-log records, a 14-call STOP_ON_FRAME frame loop with stream and digest equality, and a second-instance equality line"
  - "tools/check/steps/pkg-install.sh: the two release images passed and checked present, rgold4 and check_one_golden for the scn. prefix, the self-tests scn-digest-drift and scn-domain-missing"
  - "data/expected/pkg-consumer.golden: 67 added lines (8 header, 59 body), 0 removed"
  - "data/expected/pkg-selftests.ids 15 to 17; two pkg-install mutant lines for SCN-01 in the important-gates registry"
  - ".planning/CI-EFFICIENCY.md: Phase 7 checkpoint C-C with checkpointC-07: no-lever"
affects: [plan-07-12-closeout, plan-07-10-scn02, plan-07-11-scn02]

actuals:
  tokens: 17860
  tasks: 3
  commits: 3
plan_head_before: 290f05733eb8ad6459df182ae4f09c044f3dbdae
plan_head_after: 3248f06e7d7a1e461d16d1ac2e3868540b4750fb

key-decisions:
  - "The consumer takes its audio-window starts from the latest-write bounds that specification A8 states (gate write before cycle 1,300, deferred amplitude writes before cycle 250), not from the tighter reference-sequence figures of derivation section (d), because the program as built writes its gate 14 cycles after the reference sequence; the window ends are the section (d) earliest-write figures"
  - "run_step gained keep_audio and keep_plog flags (default off) after the precedent of keep_video; the three earlier replays do not set them, so their code path and their pinned lines are unchanged"
  - "The frame loop compares the concatenated delivered streams as well as the end digests with the single run, and asks for exactly one video slot of capacity per call"
  - "Cost checkpoint C-C ends no-lever by the rule of C-B: this plan's own addition is under 0.6 hosted cpu-s, the last-five median is 8.02 and the projection 8.04"

key-files:
  created:
    - .planning/phases/07-first-visible-original-scenario/07-09-SUMMARY.md
  modified:
    - tests/installed-consumer/main.c
    - tools/check/steps/pkg-install.sh
    - data/expected/pkg-consumer.golden
    - data/expected/pkg-selftests.ids
    - data/expected/important-gates.ids
    - docs/packaging.md
    - .planning/CI-EFFICIENCY.md
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-13
---

# Phase 7 Plan 09: Installed-consumer replay of the Lamp Tone pass scenario and cost checkpoint C-C Summary

**An independently built consumer of the installed package now replays the Lamp Tone pass scenario the way a host would: it loads the project boot stub contract 2, the zero GROM and the program through the installed headers alone, reproduces all nine `scn.digest.*` lines and the observed-domain set of the runner golden, checks the slot headers, the six lamp pictures, the seven declared region diffs (outside 0, inside the declared count), the audio windows and the 30 write-log records with code that shares nothing with the runner, and ends a 14-call frame-by-frame pass with the same streams and digests as the single run; cost checkpoint C-C ends `checkpointC-07: no-lever`.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `da07663` | 1 (tracer) | argc 7 and the usage text, `scn_powered`, `scn_session` (six events, four steps, guest words, observed set by the count rule), `scn.digest.*` lines, the two images and `rgold4` in `pkg-install.sh`, golden additions |
| `5ee7d13` | 2 | frame loop, slot headers, lamp pictures, pair diffs, pixel probes, audio and write-log decode, `keep_audio`/`keep_plog`, self-tests `scn-digest-drift` and `scn-domain-missing`, golden additions, `pkg-selftests.ids` 17 |
| `3248f06` | 3 | `docs/packaging.md`, two `pkg-install mutant` lines for SCN-01, `.planning/CI-EFFICIENCY.md` checkpoint C-C |
| the commit that adds this file | | this SUMMARY, STATE.md, ROADMAP.md |

## What was built

- **Replay 4** (D-26, copy-paste, no refactor of the earlier replays): `scn_events` (the six events of the scenario with both controller words each, ticks 100,000 to 700,000), `c_scn_stops` (the four run-to bounds and the guest words `02F0..02F5` at each, derivation section (c)), `scn_powered`, `scn_session`. Every buffer is sized from `ivs_info` in `run_step` (the video buffer with `video_slack_frames`); no image bytes and no SHA code are in the source (the images are `argv` files).
- **Independent checks** (D-21, D-22), each figure a named constant with its derivation section: `scn_check_frames` (13 slot headers: index, tick `(k+1) x 59,736`, geometry, frame 0 not displayed and 1 to 12 displayed, reserved 0; the six lamp pictures of section (e) read from the index planes, every position outside the two lamp rows index 0; the seven declared pairs of `data/expected/video-frame-pairs.ids` over the delivered slots 11, 3, 5, 7, 9, 12; the four pixel probes of the scenario), `scn_check_audio` (every sample `(A,B,C,0)` with levels at most 15; four windows decoded sample by sample; the six `audio at` ticks), `scn_check_psglog` (the 16-byte records decoded as big-endian tick, value, register, source; all 30 records of section (d): register, value, source 1, and a tick inside the named frame).
- **Frame loop** (`scn_frame_stop_pass`): a fresh machine, `IVS_RUN_F_STOP_ON_HALT | IVS_RUN_F_STOP_ON_FRAME`, one video slot of capacity per call, the audio and write-log buffers sized from `ivs_info`, the unconsumed suffix of the events resubmitted on each call; 14 calls (13 stop at a frame end, the 14th at the target 806,568), `consumed=6`, and the concatenated video, audio and write-log bytes plus the end digests equal the single run (`scn.frame-stop equal=yes`).
- **pkg-install**: `img_exec2` and `img_cart4` from the release images directory, the presence check, the six-image call, `rgold4` and `check_one_golden "$rgold4" "scn\\."`, header comments updated. The step already compared digest lines and the observed set per prefix; the `scn.` prefix reuses that code.
- **Self-tests** (clones of the `stic-` pair): `scn-digest-drift` changes one nibble of the `scn.digest.video` line and requires `pkg-runner-golden-diff` (the unchanged output must match first); `scn-domain-missing` drops the `scn.digest.audio` line and requires the `scn.observed domains` finding.

## Consumer golden (additions only)

`git diff 903276757b0dbd901b15dbc00b9813089c1b578e -- data/expected/pkg-consumer.golden`: 67 added lines, 0 removed. 8 are header comment lines (the step drops `#` lines), 59 are body lines, all prefixed `scn.`, inserted before the unchanged last line `result ok failures=0`: `scn.load.exec`, `scn.load.cart`, four `scn.step.k` and `scn.step.k.mem`, `scn.input events=6 consumed=6`, `scn.video`, `scn.slots`, six `scn.lamps`, seven `scn.diff` (all `outside=0`), `scn.pixels`, `scn.audio`, four `scn.audio.window`, six `scn.audio.at`, eight `scn.psglog.rec`, `scn.psglog`, two `scn.frame-stop`, nine `scn.digest.*`, `scn.second-instance equal=yes`. The golden was regenerated from `build/gate/pkg-consumer.txt` after a gate-step run (header lines kept, body replaced), never by hand. The consumer output is 76 lines before and 135 after.

## Agreement with the runner (PLAN-LESSONS 2, T-07-21)

The consumer's nine `scn.digest.*` lines (cpu, mem, images, instance, composite, video, audio, input, psglog) equal the `digest.*` lines of `data/golden/demo-lamp-tone-pass.digests`, and its observed set (video, audio, input and psglog, with cpu, mem, images, instance) equals the golden's `# domains:` line: it is the first replay that observes every domain the golden lists. The comparison is `check_one_golden` in `check_runner_golden`; the first tracer run already showed `scn.digest.video 66c5747555074a7a8fe1455e331930b3f3d84e06cd6dbb5c92fb490d8fd85546`, equal to the golden. The three earlier replays and their lines are unchanged (0 removed golden lines; the old replays do not set the new keep flags; `consumer-lines` went 76 to 135 by additions only). The scenario, the golden, the oracle (derivation sections (a) to (j), the F-S literals), `tools/asm/asm.c`, `tools/pack/pack.c` and the fixtures are untouched (`git diff` against `290f057` is empty for `docs/video`, `tools/asm`, `tools/pack`, `data/golden`, `data/scenarios`, `fixtures`), so no recipe digest moved.

## Induced failures (the consumer's checks and the self-tests shown able to fail)

Each trial edited one constant in `tests/installed-consumer/main.c`, ran the `pkg-install` step, and restored the file (`cmp` against a saved copy):

| Edit | Result |
|------|--------|
| pair `F-S1,F-S2` count 128 to 127 | consumer exits 1, `FAIL scn diff`; step `pkg-consumer-run: exit 1` |
| write-log record 6 value `007E` to `007F` | `FAIL scn psglog records` |
| window L1 level C 15 to 14 | `FAIL scn audio window` |
| lamp pattern of slot 3 `21111112` to `21111111` | `FAIL scn lamps` |
| final guest counter `000D` to `000C` | `FAIL scn guest words` |
| event 1 moved from tick 100,000 to 100,016 (a replay/scenario skew) | step `pkg-runner-golden-diff: scn.golden only: digest.input ...` (the consumer itself finds nothing wrong; the comparison with the runner golden does) |

The two new pkg self-tests are themselves the induced failures of the golden comparison: they pass in the step only when the dropped audio line and the changed video nibble make the comparison fail for the stated reason.

## Cost checkpoint C-C (D-34)

Full entry in `.planning/CI-EFFICIENCY.md` ("Phase 7 checkpoint C-C (plan 07-09)").

- `pkg-install` step in isolation, local: real 2.40, 2.41, 2.23 s before; 2.33, 2.24, 2.21 and (later, higher load) 2.55, 2.53, 2.48 s after; user plus sys 2.40, 2.40, 2.28 before and 2.41, 2.33, 2.30, 2.60, 2.61, 2.54 after. The difference is inside the load noise; the bound used is at most +0.33 cpu-s. Local gate log `step-time pkg-install 2`. Container (emulated amd64) wall: 9 s, 11 s, 14 s after the three tasks (the third while two local gates ran); the container figure before the plan was not recorded (that log was overwritten by this plan's first container run).
- Hosted: last five successful rows 43359195974, 43365038335, 43369159094, 43373151931, 43528527276, `r` 9.02, 7.53, 8.02, 8.17, 7.73, median **8.02** (8.0204). The newest row (head 785dce0, plans 07-01 to 07-08) does not include this plan's commits, so the projection is the median plus this plan's bounded hosted addition: 0.33 x 1.675 / 37 = 0.015, projection **8.04**, under 8.3.
- Forward note (information, not an input): the 14 case-pairs still to come in plans 07-10 and 07-11, at the S-4 upper unit cost of 0.7 hosted cpu-s each, would bring the projection to about 8.30, on the line. That unit cost overestimated the W4 wave (25 case-pairs, row 43528527276 at `r` 7.73), so it is a bound, not an expectation; plan 07-12 re-reads the last five after the 07-10 and 07-11 push.

```
checkpointC-07: no-lever
```

## Deviations from Plan

**1. [Rule 1 - premise mismatch] The audio window starts of derivation section (d) are tighter than the program as built.** The first full run of `scn_check_audio` failed: 3 samples at the start of window L1 (stamps 124,144 to 124,176) and 4 at the start of L3 (721,856 to 721,904) still read `C=0`. Section (d) starts the gate windows at the reference-sequence write time (`S_2 + 4 x 1,165 = 124,132`, `S_12 + 4 x 1,254 = 721,848`), but section (k) records that the program as built writes 14 cycles later than the reference (gate stamp 1,179 in frames 2, 4, 8 and 1,268 in frame 12: the render loads each last byte once more); the log records tick 124,188 for record 7, and the first `C=15` sample is the one stamped 124,192, consistent with "a write takes effect at the first sample stamped at or after it". The scenario's `audio at` asserts are at least 15,868 ticks inside either choice, so no scenario, literal or golden is affected. Fix under the plan's constraints (the oracle is frozen, and a start taken from the run's own log would be circular): the consumer's window starts are the latest-write bounds that specification A8 states itself (gate write before cycle 1,300: L1 from 124,672 and L3 from 722,032; deferred amplitude writes before cycle 250: L2 from 658,096), the ends stay the section (d) earliest-write figures, and the source comment says why. The program's window boundaries hold with these bounds: samples 3,437, 3,983, 3,437 and 7,756 inside the four windows, 0 mismatches. Section (d) could be annotated by an appended note in section (k) at the close-out; this plan does not touch the derivation. Files: `tests/installed-consumer/main.c`. Commit `5ee7d13`.

**2. [Interpretation] "Seven arguments" is `argc` 7.** The plan text says seven arguments; the consumer takes six image paths, which is `argc != 7` (the acceptance grep) once the program name is counted. The usage line names the six files.

**3. [Interpretation] `run_step` gained two keep flags instead of a copy.** The plan says copy-paste and no refactor of the three replays; keeping the audio and write-log bytes for decoding needed the same hook that `keep_video` gave the third replay in plan 06-13. `keep_audio` and `keep_plog` (default 0) append the delivered bytes through a new `keep_bytes` helper; the earlier replays do not set them, so their calls, buffers and printed lines are unchanged (their golden lines are byte-identical).

**4. [Scope] More checks than the plan lists.** All six frame literals are checked as lamp pictures (the plan names one idle and one response frame), the four scenario pixel probes are included, the full 30-record write list is checked (the plan names the hand-derived addresses), and the frame loop compares the delivered streams as well as the digests. Each is a constant table from the derivation; none weakens a plan check.

**5. [Process] The container "before" figure of the `pkg-install` step is missing.** The container log of plan 07-08 was overwritten by this plan's first container run; the entry says so and uses the local isolation figures as the before and after pair.

No Rule 2 or 3 fixes; no auth gates; no package installs (T-07-SC); no C file under `core`, `runner`, `tools/asm` or `tools/pack` was edited, so no manifest digest was refreshed.

## Decisions Made

- Bound a window by what the specification states (A8), not by a reference figure the program may exceed, and not by the run's own log.
- Put the replay in the existing consumer source with the earlier keep-flag precedent; no new target, step, job, dependency or control.
- Keep the SCN-01 important-gates lines for the two self-tests in the task the plan names (Task 3); the step does not require a registry line for a self-test in the commit that registers it.

## Open Items

- **Plan 07-12**: the claims CL-0023 (tier playable-guest-progress, artifact `data/golden/demo-lamp-tone-pass.expect`) and CL-0024 (installed-host-behavior, artifact `tests/installed-consumer/main.c`) can now cite this replay; the consumer figures are hand-derived by the author of the program and the core (the `docs/packaging.md` caveat says so). Optional: append a note to derivation section (k) that the section (d) gate-window starts (124,132 and 721,848) are reference-sequence figures and the as-built starts are 56 ticks later (deviation 1).
- **Plan 07-12 and the orchestrator (checkpoint C-C)**: `no-lever` on the rule as measured; the forward bound with the SCN-02 cases of plans 07-10 and 07-11 is about 8.30 at the S-4 upper unit cost, so re-read the last-five median after the push of those plans; lever (2) stays queued.
- **SCN-01 stays Pending** in REQUIREMENTS.md: success criterion 1 (CI builds and runs it through the installed consumer) is met on all three gates, but the claims of plan 07-12 are not yet in.
- Carried unchanged: Phase 4 and 5 review deferrals, `sh tools/hygiene.sh --history` on earlier planning commits, the orchestrator-owned hosted CI record (no `gh` was called, `tools/ci-record.sh` was not run).

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=36/36 controls=354/354 faults=222/222 ... goldens=15 ... pkg-selftest=17/17 ... tests-expected=4596 tests-executed=4594 tests-skipped=2 tests-missing=0 (0 FAIL lines; gate-time total=68 s on the tree of 3248f06, with a container gate running at the same time)
dash tools/gate.sh:    the identical summary line (md5 6ed38a54754de1e1493d606077175439 for both shells, gate-time and gate-cpu removed; 0 FAIL lines; total=97 s)
sh tools/gate-linux.sh: exit 0 on the tree of 3248f06 minus the final CI-EFFICIENCY cell edit (build/gate-linux.log ends gate: PASS; gcc 13.3.0, cmake 3.28.3), steps=36/36 controls=354/354 faults=222/222 goldens=15 gates=75 lines=261 pkg-selftest=17/17 tests-expected=4596 tests-executed=4594 tests-skipped=2 tests-missing=0, 0 FAIL lines (gate-time total=297 s); the container gate also passed after each C-changing task (after the tracer on da07663's tree, total=251 s, and after task 2, total=227 s)
```

Counts at plan start were `pkg-selftest=15/15`, `gates=75`, `lines=259`. `grep -c 'argc != 7' tests/installed-consumer/main.c` prints 1; `grep -c '^scn-' data/expected/pkg-selftests.ids` prints 2; `grep -c ' scn-d' data/expected/important-gates.ids` prints 2; `grep -c '^checkpointC-07: ' .planning/CI-EFFICIENCY.md` prints 1; `grep -c '^scn\.' data/expected/pkg-consumer.golden` prints 59.

## Known Stubs

None. The consumer's tables are derived expected values, not placeholders; nothing flows to a display.

## Threat Flags

None new. T-07-21: the `scn.digest.*` lines and the observed set are compared with the runner golden in the step, both new self-tests pass, the consumer golden is additions only, and a skewed event tick is caught by the comparison (induced failure above). T-07-22: the images are `argv` files from the release images directory, the source holds no image byte array (the only byte tables are the earlier 8-word pattern and the new expected-value tables), and `pkg-install` shows no path or leak finding.

## Self-Check: PASSED

`tests/installed-consumer/main.c`, `tools/check/steps/pkg-install.sh`, `data/expected/pkg-consumer.golden`, `data/expected/pkg-selftests.ids`, `data/expected/important-gates.ids`, `docs/packaging.md` and `.planning/CI-EFFICIENCY.md` exist and are tracked; commits `da07663`, `5ee7d13` and `3248f06` are ancestors of HEAD (`git rev-list --count 290f05733eb8ad6459df182ae4f09c044f3dbdae..HEAD` printed 3 before this file's commit); `git status --short` showed only `.planning/config.json` and `.planning/state.json` (not staged) before this commit; no file name contains the console brand word.
