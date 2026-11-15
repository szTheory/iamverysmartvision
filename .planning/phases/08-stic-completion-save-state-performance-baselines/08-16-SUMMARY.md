---
phase: 08-stic-completion-save-state-performance-baselines
plan: 16
subsystem: Phase 8 wave B, plan 8 - the host surfaces of the save state - the round-trip hook of ivs-run (save, power-on reset and restore at every multiple of an option, with the report unchanged), its process cases on five scenario families with a self-control, its in-process ids, and the fifth installed-consumer replay of the snapshot calls on Lamp Tone through the installed package only
tags: [B, STA-02, runner, roundtrip-hook, installed-consumer, snapshot, ivs-run, D-18, D-43, D-44, T-08-37, T-08-38]
status: complete

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-15 end state (the four public snapshot calls, the corruption and fuzz family, the 18 cuts); counts at plan start steps=38/38 controls=378/378 faults=235/235 fault-runs=235 tests-expected=4912 tests-executed=4908 tests-skipped=4 tests-missing=0 pkg-selftest=17/17 consumer-lines=135 goldens=27 api-calls=19 public-symbols=19 (re-measured with sh tools/gate.sh before the first edit)"
provides:
  - "runner/ivs_runner.h, runner/exec.c, runner/main.c: ivs_runner_options.roundtrip_ticks and the flag IVS_RUNNER_OPT_ROUNDTRIP, the cap IVS_RUNNER_ROUNDTRIP_MAX_CUTS (64), the hook in the slice loop (the slice that would cross a multiple ends on it; after a slice at which the loop goes on, ivs_machine_snapshot_save into an executor-owned buffer, ivs_machine_reset power-on, ivs_machine_snapshot_restore; machine time read after the reset and the restore so a cut that did not happen is a harness failure), ivs_runner_result.roundtrip_cuts as a reading for tests, and ivs-run --roundtrip-at-ticks <n>; no report key, no golden, no file, no core file"
  - "tests/runner/runner_exec.c: runner.exec.roundtrip-equals-straight, -option-range and -cut-cap (tests-expected 4912 to 4920 with the first process case); tests/runner-process-tap.sh: runner-process.roundtrip.<id> for diag-boot-pass, diag-ctrl-psg-pass, diag-stic-fgbg-pass, demo-lamp-tone-pass and diag-stic2-collision-pass with a host.calls vacuity guard, runner-process.roundtrip.refuse-range and the self-control runner-process.judge.roundtrip-differs (tests-expected 4920 to 4932)"
  - "tests/installed-consumer/main.c: snap_session, the fifth replay, 29 snap. lines; data/expected/pkg-consumer.golden (additions only), pkg-consumer-calls.ids (+4 names), pkg-selftests.ids (+snap-domain-missing, +snap-digest-drift), tools/check/steps/pkg-install.sh (the snap prefix against the runner golden of Lamp Tone, the two self-tests), docs/packaging.md; consumer-lines 135 to 164, pkg-selftest 17/17 to 19/19"
affects: [08-17]

actuals:
  tokens: 16204
  tasks: 3
  commits: 5
plan_head_before: 2eaab6e7359e53ad9de0e662447f924d3d60b23f
plan_head_after: 98b63025a8e99dc61f2bba924244371b6bc73a33

key-decisions:
  - "The cut is made only when the slice loop goes on within a run-to (the stop was the target and machine time is below the limit); a multiple that machine time passed at the end of a run-to, or inside one unit that spans several multiples, moves the next multiple past machine time and makes no cut. The slice that would cross a multiple ends on it and the machine stops at the first unit boundary at or after it, so every cut is at a unit boundary of a powered machine and the slice and stop accounting (ran, overshoot, stop reason, input consumed) is untouched: the reports of the five scenario families equal their plain reports apart from host.calls"
  - "A cut that did not happen must not read as one: after the power-on reset machine time is read and must be the power-on time, after the restore it must be the time of the cut; otherwise the run ends harness-fail with roundtrip-reset or roundtrip-restore. Without this a hook whose body did nothing passed every equality test (an uncut run has the plain report and host.calls only sees the shortened slices); with it, a probe that skips the reset and the restore fails all three in-process ids. Two read-only ivs_machine_info calls per cut"
  - "The process comparison has a vacuity guard: every cut is one more slice and ivs-run makes one cancel check and one timer check per slice, so host.calls of the hook run must exceed the plain run's by two per cut (roundtrip-no-cut); the self-control feeds the comparison a report of another scenario (body-differs) and a plain report replayed as if no cut had been made (roundtrip-no-cut)"
  - "The consumer proves the restore on a poisoned target through the public API only: machine B (the same images) is run on the idle input to another tick before the restore, so its CPU, memory, instance and composite digests differ from A's; a damaged copy is refused by its trailer and B is left as it was; the restored B equals A through nine digests and the CPU and STIC reports, a save of B is the snapshot, and the end digests are those of the straight run through the same comparison as the runner golden"
  - "Cost (the stop rule): the 65 cuts of runner.exec.roundtrip-cut-cap run whole in the normal executables of both presets and are only reported under the 19 runner fault executables (IVS_RUNNER_FAULT_RUN, the precedent of IVS_STATE_FAULT_RUN), and the in-process equality is held to the diagnostic program, because the five process scenarios cover streams, input, display and collision state in both presets; about 1.5 local cpu-s less"

key-files:
  created:
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-16-SUMMARY.md
  modified:
    - runner/ivs_runner.h
    - runner/exec.c
    - runner/main.c
    - docs/runner.md
    - tests/runner/runner_exec.c
    - tests/runner-process-tap.sh
    - tests/CMakeLists.txt
    - tests/installed-consumer/main.c
    - tools/check/steps/pkg-install.sh
    - data/expected/tests/runner.ids
    - data/expected/tests/runner-process.ids
    - data/expected/pkg-consumer.golden
    - data/expected/pkg-consumer-calls.ids
    - data/expected/pkg-selftests.ids
    - docs/packaging.md
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-14
---

# Phase 8 Plan 16: the host surfaces of the save state Summary

**`ivs-run --roundtrip-at-ticks <n>` makes the executor save the machine into a buffer of its own, reset it to power-on and restore the bytes at the first unit boundary at or after every multiple of n, and the report is the plain report apart from `host.calls` for all five scenario families the project has: the boot diagnostic, the controller and PSG diagnostic with its seven input events, the first STIC diagnostic in a displayed frame, Lamp Tone across twelve frames and six events, and the collision scenario whose MOB, border, delay and collision registers a cut carries (a core that skips the STIC register words on restore fails exactly the two STIC scenarios). The option is refused as a usage error for a zero, a value above the maximum, a missing value and a repeated option; more than 64 cuts is the harness failure `roundtrip-cut-cap`; a cut that did not happen is a harness failure too. The fifth installed-consumer replay, built against the installed headers and archive only, saves Lamp Tone mid-frame, reads the bytes without an instance, refuses a damaged copy, restores into a machine another run dirtied and finishes the run with the digests of the straight run, which the gate compares with the runner golden. 10 test ids (4 and 6; tests-expected 4912 to 4932), 2 pkg self-tests, no fault, no control, no golden, no core file; the gates pass under sh, dash and the Ubuntu container with the plan-end counts `steps=38/38 controls=378/378 faults=235/235 fault-runs=235 tests-expected=4932 tests-executed=4928 tests-skipped=4 tests-missing=0 goldens=27 pkg-selftest=19/19 consumer-lines=164 api-calls=19 public-symbols=19 hdr-alone=10/10 prefix-files=11`. STA-01 and STA-02 stay Pending until plan 08-17 closes them on its terms; the file options of the runner are deferred (D-18).**

## Commits

| Commit | Task | What |
|--------|------|------|
| `7f98e77` | 1 (tracer) | the option, flag, cap and hook, the CLI and docs, `runner.exec.roundtrip-equals-straight`, `-option-range`, `-cut-cap`, `runner-process.roundtrip.diag-boot-pass` (tests-expected 4912 to 4920) |
| `336b3cb` | 1 (fix) | machine time read after the reset and the restore: a cut that did not happen is a harness failure (deviation 2); the determinism sentence of docs/runner.md for the five process cases landed here too (deviation 7) |
| `5a71493` | 2 | the four other scenario families, `runner-process.roundtrip.refuse-range`, `runner-process.judge.roundtrip-differs` (tests-expected 4920 to 4932) |
| `f195ffe` | cost | the cut-cap id reports under the runner fault executables only, the in-process equality on the diagnostic program only (deviation 5); no id, no count |
| `98b6302` | 3 | `snap_session`, the 29 golden lines, the four call names, the prefix comparison and two self-tests, `docs/packaging.md` |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md |

## The hook

The slice loop of `runner/exec.c` is the insertion point (08-RESEARCH 3.4): `ivs_machine_run(target = min(limit, now + slice_ticks))` as before, but the target is cut to the next multiple when the multiple falls inside the slice, so the machine stops at the first unit boundary at or after it. After the slice's streams are captured and the harness-fail check, `more` is true when the stop was the target and machine time is below the limit; if machine time reached the next multiple, that multiple moves to the first multiple above machine time (saturating at 2^64 - 1), and when `more` holds the cut is made: `ivs_machine_snapshot_save` into `rt_buf` (an `IVS_SNAPSHOT_MAX_BYTES` array of the executor state, written by the save and read by the restore of the same cut, never written to a file or a report), `ivs_machine_reset(IVS_RESET_POWER_ON)`, `ivs_machine_snapshot_restore` of the bytes just written, each its own statement. The options validation is one function (only the one flag bit; the flag requires `roundtrip_ticks` in 1 to `IVS_RUN_TARGET_MAX`; the flag clear requires 0). The failure details are `roundtrip-cut-cap`, `roundtrip-save`, `roundtrip-reset` and `roundtrip-restore`; a harness failure writes the usual report (the detail is in it), with every assert `not-run` and the step stopped at the target.

`ivs_runner_result` gained `roundtrip_cuts` (the plan's test-only reading of the executor result; no report key). `ivs-run` takes `--roundtrip-at-ticks <n>` in the table of `main.c` in the manner of `--slice-ticks` (range 1 to 9223372036854775808, once). The usage text had to be split into two string literals, because one more option took it past the 4,095 characters a compiler must accept (`-Woverlength-strings` under `-Werror`; deviation 1); the text printed is the one a single literal would give, the outcome table still last.

## What each id holds

| Id | What it holds |
|----|---------------|
| `runner.exec.roundtrip-equals-straight` | the diagnostic program (halts at tick 2,084) plain and with the hook at 600 through `ivs_runner_execute` on two fake hosts: both pass, the reports are byte-equal apart from the `host.calls` line, the plain run made no cut, the hook run made 3 (the cuts at 620, 1,212 and 1,812), and `host.calls` of the hook run is the plain run's plus the cuts (the fake host has a cancel hook and no timer) |
| `runner.exec.roundtrip-option-range` | twelve rows: the flag with 0, with `IVS_RUN_TARGET_MAX + 1` and with 2^64 - 1; a value without the flag (5,000 and the maximum); an unknown flag bit alone, with a value, together with the valid pair, and the top bit: each outcome 14 with no byte written, no line counted, no host call and no cut; the valid rows: the flag with 1,500 (one cut), with exactly `IVS_RUN_TARGET_MAX` (no multiple is ever reached: no cut) and no flag with 0, each a complete pass report |
| `runner.exec.roundtrip-cut-cap` | a value of 1 on the diagnostic program, which has more than 64 unit boundaries before its halt: the 65th cut is refused, exit 14, the detail `roundtrip-cut-cap`, exactly 64 cuts made, the step stopped at the target and the first and the last of the 18 assert lines `not-run` |
| `runner-process.roundtrip.<id>` (five) | the real `ivs-run` with the hook: the judge passes, the report equals the scenario case's report apart from `host.calls`, and `host.calls` exceeds the plain run's by at least two per expected cut |
| `runner-process.roundtrip.refuse-range` | `--roundtrip-at-ticks 0`, `9223372036854775809`, the option last on the line and the option twice: exit 14, no report on stdout, the first stderr line `ivs-run: option value out of range`, `option without a value` or `option given twice`, the second line the usage line, and the usage text names the option |
| `runner-process.judge.roundtrip-differs` | the comparison is rejected for `body-differs` on a fake runner that replays the report of another scenario and for `roundtrip-no-cut` on one that replays the plain report as if no cut had been made |

### The five scenario families (the values, derived by arithmetic from the run length)

A frame is 59,736 master ticks; no value is a multiple of it. A cut is at the first unit boundary at or after a multiple. The run ends and cut ticks below are those of `build/dev` and `build/release` (they agree), read once with a temporary print in the executor and reverted; host.calls: plain run, hook run, so cuts = (hook - plain) / 2.

| Scenario | Run length | Value | Cuts | Cut ticks, frame and offset, events applied | host.calls |
|----------|-----------|-------|------|---------------------------------------------|------------|
| `diag-boot-pass` | halts at 2,084 | 600 (2,400 lies past the halt) | 3 | 620, 1,212, 1,812: frame 0, no display | 5, 11 |
| `diag-ctrl-psg-pass` | halts at 16,752, seven events at ticks 4,000 to 8,800 | 5,000 | 3 | 5,012 (two events applied, the second one a pull-down in force), 10,004 and 15,020 (all seven): frame 0, no display | 5, 11 |
| `diag-stic-fgbg-pass` | halts at 88,476, one frame delivered (ends at 59,736), the display on | 20,000 | 4 | 20,024 and 40,016 (frame 0, displayed), 60,008 (frame 1, 272 ticks in: 68 CPU cycles, inside the interrupt service of the frame end), 80,024 (frame 1, 20,288 in) | 5, 13 |
| `demo-lamp-tone-pass` | the four bounds 200,000, 440,000, 640,000, 806,568; six events at 100,000 to 700,000; 13 frames | 90,000 (810,000 lies past the end; no multiple is a step bound or an event tick) | 8 | 90,020 (frame 1, 30,284 in, the first displayed frame, none applied), 180,036 (frame 3, 828 in, 1), 270,008 (frame 4, 31,064 in, 2), 360,032 (frame 6, 1,616 in, 3), 450,032 (frame 7, 31,880 in, 3), 540,016 (frame 9, 2,392 in, 4), 630,008 (frame 10, 32,648 in, 5), 720,012 (frame 12, 3,180 in, 6) | 11, 27 |
| `diag-stic2-collision-pass` | the one bound 640,000 (ends at 640,004), the program never halts, the display on from frame 1, ten frames delivered | 150,000 | 4 | 150,020 (frame 2, 30,548 in, the one event applied), 300,016 (frame 5, 1,336 in: inside the routine of the frame), 450,020 (frame 7, 31,868 in), 600,024 (frame 10, 2,664 in) | 5, 13 |

**The collision scenario at the first cut.** Frame 2 spans 119,472 to 179,207. The routine of frame 1 cleared the collision registers C0 to C7 and wrote scene 7, whose render at the end of frame 1 set them (sticky) to `0002 0001 0008 0004 0000 0000 0100 0000`; the routine of frame 2 wrote the scene 8 MOB words (eight MOBs at Y word `015C`, apart, so no new bit). The cut at 150,020 is 30,548 ticks into frame 2: the MOB registers hold scene 8 and the collision bits of frame 1 are in force. The routine of frame 3 reads C0 to C7 and stores them at `0300` to `0307`; the scenario's own asserts `mem 0300 == 0002`, `0301 == 0001`, `0302 == 0008`, `0303 == 0004`, `0304 == 0000`, `0305 == 0000`, `0306 == 0100` and `0307 == 0000` therefore hold in the hook run only if the restore brought the collision bits and the registers back, and the frame 5 pixel asserts and `video count == 10` judge the MOB scenes after the later cuts. The three scene facts are from `docs/video/diag-stic-2-derivation.md` sections (c) and (d), not from the core.

## What each new test can fail on (probes in the working tree, reverted)

| Probe | Failing ids |
|-------|-------------|
| `exec.c`: the restore call removed (before the time checks) | `runner.exec.roundtrip-equals-straight` |
| `exec.c`: the reset and the restore both skipped (after the time checks, so the cut is a no-op) | `runner.exec.roundtrip-equals-straight`, `-option-range`, `-cut-cap` (harness-fail `roundtrip-reset`) |
| `exec.c`: the slice not shortened (no cut ever) | all three in-process ids (zero cuts) |
| `exec.c`: the cap compared with `>` (65 cuts) | `runner.exec.roundtrip-cut-cap` |
| `exec.c`: the flag-clear-with-value check removed | `runner.exec.roundtrip-option-range` (row 3) |
| `machine_snapshot_restore.c`: the stored STIC register words not copied | `runner-process.roundtrip.diag-stic-fgbg-pass` and `.diag-stic2-collision-pass` (the other three do not move: the boot and controller diagnostics do not use the STIC, and the Lamp Tone routine rewrites the registers in every frame) |
| the consumer against a core that skips the PSG tone rows on restore | `snap.restore.immediate digests-equal=no`, `snap.fixed-point equal=no`, `snap.equal-straight equal=no`, exit 1 (`FAIL snap state after restore`, `snap fixed point`, `snap equal straight`) |
| the consumer against a core that skips the STIC register words | **not seen**: A and B run the same program, so the STIC register file is the same in the dirty target; the state suite and the two process cases above are the ones that see it (open item 2) |

## The fifth installed-consumer replay

Built by the gate step `pkg-install` against the installed, relocated prefix with strict C17 flags; no new image argument (`argc` stays 7), the Lamp Tone images of the fourth replay.

**The cut.** The target is 450,000 and the run ends at the first unit boundary at or after it, tick 450,032 (the overshoots of the scn steps are 24, 12 and 4, here 32). Frame 7 spans 418,152 (7 x 59,736) to 477,887, so the cut is 31,880 ticks into the frame and 27,856 before its end, at least 10,000 from either, and 450,000 is not a multiple of 59,736. It is above the stop of the second step (440,012) and below the fourth event (460,000): the events at 100,000, 220,000 and 340,000 are applied at the cut, three, the others are the host's to resubmit. The replay checks these figures against the machine (`snap cut position`) rather than trusting them.

**What it does.** Machine A runs the first two steps of the fourth replay (the same events, the same guest words) and the run to the cut; `ivs_machine_snapshot_size` gives 4,299, a buffer of exactly that size is allocated (no size is hard-coded; the public bound `IVS_SNAPSHOT_MAX_BYTES` is only a check) and `ivs_machine_snapshot_save` fills it. `ivs_snapshot_describe` of the same bytes: format 1, schema 1, total 4,299, the profile id, the three slots (kind, loaded, length, SHA-256) and the composite digest equal the machine's. Machine B (the same three images, powered, an idle run to tick 300,000 so that CPU, memory, instance and composite digests differ from A's) refuses a copy of the snapshot with one payload byte flipped (`IVS_E_DIGEST_MISMATCH`, reason 70, its nine digests unchanged), restores the real snapshot (the report equals the one describe wrote), and immediately has A's nine digests and A's CPU and STIC reports; a save of B is the snapshot. B then runs the last two steps with the events resubmitted from index 3, the applied count A's run calls reported, the guest words at each stop equal to the scn constants. The streams of the two legs, concatenated in the kept buffers, pass the frame, audio and write-log checks of the fourth replay (13 frames, 198,848 video bytes, 201,640 audio bytes, 480 write-log bytes), the nine final digests are those of the straight run (`snap.equal-straight equal=yes`) and equal `data/golden/demo-lamp-tone-pass.digests` through the same function as the `scn.` lines; a second instance repeats it (`snap.second-instance equal=yes`).

**The lines.** 29 `snap.` lines are added to the consumer golden and nothing else (a comment block explains them; `git show` of the commit removes 0 lines of the golden): `snap.step.1`, `.step.1.mem`, `.step.2`, `.step.2.mem`, `snap.cut`, `snap.size`, `snap.describe`, `snap.dirty`, `snap.refusal.damaged`, `snap.restore`, `snap.restore.immediate`, `snap.fixed-point`, `snap.step.3`, `.step.3.mem`, `.step.4`, `.step.4.mem`, `snap.input`, `snap.streams`, `snap.equal-straight`, the nine `snap.digest.<domain>` lines and `snap.second-instance`. **`consumer-lines` is 164** (135 plus the 29 printed). No timing, path or host value is printed. The gate step compares the `snap.digest.*` lines and observed domains with the runner golden through `check_one_golden` and the prefix `snap\.` (a fifth golden variable pointing at the same file as the fourth), `pkg-consumer-calls.ids` gains `ivs_machine_snapshot_size`, `ivs_machine_snapshot_save`, `ivs_machine_snapshot_restore` and `ivs_snapshot_describe`, and the self-tests `snap-domain-missing` (the `snap.digest.audio` line dropped: the observed domains differ) and `snap-digest-drift` (one `snap.digest.video` nibble changed: `pkg-runner-golden-diff`) are cloned from the scn ones (`pkg-selftest=19/19`). The agreement with the runner is agreement of one core with itself, stated in `docs/packaging.md`; it is not an independent oracle.

## Deviations from Plan

**1. [Rule 3 - blocking] The usage text of `ivs-run` is two string literals.** One more option took the single literal past the 4,095 characters ISO C requires a compiler to accept (`-Woverlength-strings`, an error under the project warnings). `main_usage` is now an array of two literals printed in order by `main_usage_write` (stdout for `--help`, stderr after the message line for a usage error); the text is what one literal would give, the outcome table is last (`runner-process.outcomes-consistent` reads the lines after `outcomes (`), and the new option appears in it.

**2. [Rule 2 - correctness, an addition; own commit `336b3cb`] The cut reads machine time after the reset and after the restore.** The plan's hook is save, reset, restore. A first probe showed that a body that did nothing at all passes every equality test: an uncut run has the plain report, and the cut counter and `host.calls` only see the shortened slices. After the reset machine time must be the power-on time, after the restore the time of the cut, or the run ends harness-fail (`roundtrip-reset`, `roundtrip-restore`, a detail the plan does not name; the plan's `roundtrip-save` and `roundtrip-restore` are kept). Two read-only `ivs_machine_info` calls per cut; the probe table above shows all three in-process ids fail on the no-op body.

**3. [Interpretation] "no report" in the cap case.** A harness failure with the detail `roundtrip-cut-cap` can only be read from a report (`outcome.detail`), so the test requires the usual harness-fail report and no pass report: exit 14, the step stopped at the target, every assert `not-run`, 64 cuts made. The option range case is the case with no report at all (nothing written, no line counted).

**4. [Interpretation] "one stderr line" of `refuse-range`.** Every usage error of `ivs-run` prints its one message line followed by the usage text on stderr (docs/runner.md: "one line and the usage"). The case requires exit 14, no report on stdout, the first stderr line equal to the message, the second line the usage line and the usage text naming the option.

**5. [Cost stop rule, own commit `f195ffe`] The cap id and the second in-process variant.** At the end of Task 3 the additions measured 3.2 local cpu-s (about +0.12 in `r`, projecting 8.25 to 8.33 against the line 8.3), of which the 65 cuts of `runner.exec.roundtrip-cut-cap` were 1.1 in the 19 runner fault executables (0.06 s each, 0.09 s on the dev preset and 0.05 on release in the normal executables). The fault executables are now compiled with `IVS_RUNNER_FAULT_RUN` (`FAULT_DEFINITIONS` of `ivs_fault_suite(runner ...)`, one line in `tests/CMakeLists.txt`, the precedent of `IVS_STATE_FAULT_RUN`) and the id only prints that its 64 cuts run in the normal executables, where it runs whole in both presets; no fault aims at the cap. A fault that makes it a must-fail id needs that branch removed (the comment says so). The second variant of the equality id (the looping stand-in under an 8 MiB block) was an addition of mine and is dropped: the five process scenarios cover streams, chains, input, display and collision state in both presets. No id, no count and no expectation moved. `tests/CMakeLists.txt` is not in the plan's `files_modified`.

**6. [Interpretation] The applied count of the restored instance.** The plan says the events are resubmitted "from B's restored applied count (the count is read from the input digest domain's report path the scn replay uses, never assumed)". The public API has no call that reports an instance's applied-event count: the count is in the snapshot's INPUT section and in the INPUT digest, not in any report. The consumer resubmits from the count A's run calls reported (3, the sum of `input_consumed`, checked against the arithmetic of the cut), as a host that keeps the count beside its snapshot does, and the proof that B's own count equals it is that the INPUT digest equals A's right after the restore and equals the straight run's at the end (a count off by one re-applies or skips an event and moves the input log). Recorded in `docs/packaging.md`; a getter would be an API addition outside this plan (open item 1).

**7. [Information] The determinism sentence of `docs/runner.md` for the five process cases landed in the fix commit.** The Task 2 patch had edited the file before the fix commit staged it, so `336b3cb` carries the sentence about five scenarios one commit before the cases exist; `5a71493` carries the cases. No history was rewritten.

**8. [Information] The plan's list of consumer lines is a floor.** `snap.dirty`, `snap.refusal.damaged`, `snap.restore.immediate`, `snap.streams`, `snap.input` and the step lines are added to the lines the plan names (`snap.size`, `snap.describe`, `snap.cut`, `snap.restore`, `snap.fixed-point`, `snap.equal-straight`, the digests, `snap.second-instance`); the refusal of a damaged copy through the installed package is one extra call to a name the consumer already pins.

**9. [Information] Files not touched that the plan lists.** `tests/runner/runner_cases.h`, `tests/runner/runner_main.c` and `data/expected/faults/runner.ids`: the three new ids sit in `exec_cases[]` of `runner_exec.c`, and the faults that share the slice code (`RUNNER_SLICE_DROPS_REMAINDER`) already allow the prefix `runner.exec.`; no must-fail column is touched. `tests/runner/runner_exec.c` does the hook's tests through the new fields of its fake host.

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag, no `tools/ci-record.sh`; `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` were never staged. No core file changed (`git show --stat` of the five commits lists no `core/` file and no `data/golden/` file). D10 and D11 are not performed and no claim above is more than provisional: every statement is about the repeatability of one build and one profile, and nothing is measured on hardware.

## Decisions Made

- The hook is a test of the host surface, not a feature of the machine: it uses the four public calls and `ivs_machine_reset` and nothing else, so a defect of the snapshot shows as a different report and a defect of the hook shows as a harness failure.
- A harness failure of the hook is loud: the cap, a refused call, and an ineffective reset or restore each end the run with a named detail, so the hook cannot silently do less than it says.
- What the consumer proves and what it does not: it proves the four calls work through the installed package into a machine another run dirtied, with the streams and digests of the straight run; it does not prove sensitivity to every row (it cannot dirty a STIC register word through the public API when both machines run the same program), which is the job of the state suite and the process cases.

## Cost (laptop)

| Item | Value |
|------|-------|
| estimate of the plan | 150,000 tokens (raw, confidence low), 3 tasks |
| actuals | 16,204 tokens (chars/4 over the added lines of the 5 commits outside `.planning/`), 3 tasks, 5 commits (measured from the ledger of the plan head) |
| gate total, sh and dash, Task 1 tree | 84 s and 100 s, identical summary line (md5 bf72f63f5898d1d0ecb58820d065ce2a); container exit 0 (318 s) |
| gate total, sh and dash, Task 2 tree | 97 s and 97 s, identical summary line (md5 1c3fbdc21b65eb4b024eacaa413523ec) |
| gate total, sh and dash, Task 3 tree | 180 s and 135 s (the laptop at load average 40 to 70: the `c-faults` step read 88 s against 24 s in the Task 2 run, with `gate-cpu` unchanged), identical summary line (md5 85df9854fe7b681008bb9ae238b80963); container exit 0 (452 s) |
| gate total, sh and dash, final tree (after the cost commit) | 95 s and 100 s, the identical summary line (md5 85df9854fe7b681008bb9ae238b80963); container exit 0, 0 FAIL lines (357 s) |
| the three in-process ids, one id per process (user cpu) | dev preset (sanitized, `-O1`): equality 0.01, option range 0.01, cut cap 0.09 (0.08 of system time is the sanitizer start-up of every process); release: 0.00, 0.00, 0.05 |
| `runner.exec.roundtrip-cut-cap` before the cost commit, in a fault executable | 0.06 s; 64 cuts and the refused 65th, about 0.9 to 1.1 ms a cut (a save hashes the image arrays); after the commit the three ids are 0.01 s there |
| the whole runner suite in one fault executable | 0.69 s before the cost commit, 0.55 s after; 19 executables |
| `ivs-run` launches added to `runner-process`, per preset | 5 hook runs and 4 usage refusals, about 0.11 s each on the dev preset (0.08 is the sanitizer start-up), about 0.01 or less on release; the two fake-runner runs of the self-control are shell `cat` |
| the consumer | the five replays together 0.07 s in the release build, the two `snap_session` runs a part of it; about 290 more lines to compile; two pkg self-tests of a few `awk` and `cmp` calls |

**The stop rule.** After the cost commit the additions of this plan are, in local cpu-s (user plus system): the three ids 0.11 dev and 0.05 release, the 19 fault executables 0.2, the nine `ivs-run` launches 1.0 dev and 0.05 release, the consumer and its self-tests about 0.2, no step, control or golden: about 1.6, hosted about 2.7 at the factor 1.675 of the record. Over the `build_s` of the hosted rows (40 for the fastest of the last five, 46 for the newest recorded row 43658364307, 63 for the slowest) that is +0.07, +0.06 and +0.04 in `r`, and the projection after plan 08-15 (8.13 to 8.21) reads **about 8.18 to 8.28** after this plan, under the line 8.3 by 0.02 to 0.12. Before the cost commit the additions were 3.2 local cpu-s (+0.12 at `build_s` 46: 8.25 to 8.33), so the stop rule would have fired on the upper end; the cost commit (deviation 5) is the response, and no test case of the plan is withheld. The margin left for plan 08-17 is therefore about 0.02 to 0.1 in `r` (the hosted row after the push settles the reading; `.planning/ci-runs.tsv` was not touched); if a hosted row reads 8.3 or more the first trims are the `ivs-run` launches of `runner-process.roundtrip.refuse-range` on the dev preset (4 launches, the cheapest to make static, as the mutant inventories are) and then the cut cap id on the dev preset.

## Open Items

1. **The applied-event count of an instance is not reported by any public call** (deviation 6). A host that restores a snapshot must keep the count of events its run calls reported next to the snapshot. A getter (in `ivs_info` or the snapshot report) would be an API addition and a reasons or struct-size change; plan 08-17 may record it in the close-out of STA-02 or leave it to a later phase.
2. **The consumer's dirty target cannot differ from A in the STIC register file** (both run the same program, and the public API has no write call), so the consumer is blind to a restore that skips the STIC register words; the state suite (`state.restore.poisoned-target-equals-source`, the row sweep) and the two STIC process cases see it. The known acceptance of `mem.stic_reg` of plan 08-15 stays pinned.
3. **No cut is made at the end of a run-to** (the plan's rule), so the asserts of a step and the final digests are always computed on a machine that a cut preceded in the same run-to or an earlier one, but never on one restored at that very instant; a cut at the end of a non-final step would put the asserts on a freshly restored machine. Not done; a later option if the owner wants it.
4. **The `roundtrip-save`, `-reset` and `-restore` refusal paths of the hook have no test that reaches them** (a refused call would be a defect of the core); the time checks turn an ineffective reset or restore into a failure and the probe above shows it, but a refused save is untested by construction.
5. **File options deferred (D-18):** `--snapshot-out` and `--snapshot-in` are not part of this phase; the close-out record of plan 08-17 lists them as deferred. A snapshot is never written by any code of this plan and no `.ivsn` is tracked; the extension is treated as rights-bearing by plan 08-17 for any future file.
6. **Cost:** the inputs of the `checkpointC-08` entry are above (additions 1.6 local cpu-s, projection 8.18 to 8.28, the cost commit and its reason, the first trims if a hosted row reads 8.3 or more).
7. **Information.** D10 and D11 are not performed; no claim above is more than provisional; no hosted-only check was run (the hosted-style container gate ran green three times); STA-01 and STA-02 stay Pending and REL-01 stays Pending (D-28).

## Interfaces for the next plans

- Runner: `ivs_runner_options.flags` may hold only `IVS_RUNNER_OPT_ROUNDTRIP`; `roundtrip_ticks` is its value; `ivs_runner_result.roundtrip_cuts` is the reading for tests; the details are `roundtrip-cut-cap`, `-save`, `-reset` and `-restore`; `IVS_RUNNER_ROUNDTRIP_MAX_CUTS` is 64. `ivs-run --roundtrip-at-ticks <n>`. The fault executables of the runner suite define `IVS_RUNNER_FAULT_RUN`.
- Tests: `check_roundtrip <runner> <id> <ticks> <min cuts>` of `tests/runner-process-tap.sh` is the comparison (finding tokens `body-differs`, `roundtrip-no-cut`); a sixth scenario is one call and one id line.
- Consumer: `snap_session(print, digests, observed, straight)` of `tests/installed-consumer/main.c`; the constants `C_SNAP_CUT_TARGET`, `C_SNAP_CUT_EVENTS`, `C_SNAP_DIRTY_TARGET`; the gate compares the prefix `snap\.` with `data/golden/demo-lamp-tone-pass.digests` (`rgold5`).
- Gate counts after this plan `steps=38/38 controls=378/378 faults=235/235 fault-runs=235 tests-expected=4932 tests-executed=4928 tests-skipped=4 tests-missing=0 goldens=27 pkg-selftest=19/19 consumer-lines=164 api-calls=19 public-symbols=19 hdr-alone=10/10 prefix-files=11` (plan 08-17 expects `tests-expected=4932`).
- Threat ids T-08-37 and T-08-38 are used here (T-08-SC throughout); the next plans continue from T-08-39 or the numbering their plans assign.

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=38/38 controls=378/378 ... faults=235/235 ... fault-runs=235 ... tests-expected=4932 tests-executed=4928 tests-skipped=4 tests-missing=0 ... goldens=27 ... api-calls=19 ... public-symbols=19 ... hdr-alone=10/10 ... prefix-files=11 ... consumer-lines=164 ... pkg-selftest=19/19 (0 FAIL lines)
dash tools/gate.sh:    the identical summary line (0 FAIL lines)
GATE_LINUX_PLATFORM= sh tools/gate-linux.sh: exit 0, build/gate-linux.log ends gate: PASS (gcc 13.3.0, cmake 3.28.3, x86_64), the plan-end counts, core-imports=5, 0 FAIL lines
```

After the Task 1 tree: `tests-expected=4920 tests-executed=4916`, sh 84 s and dash 100 s with the identical summary line (md5 bf72f63f5898d1d0ecb58820d065ce2a), container exit 0 (318 s; the first sh run of that tree failed `glossary-bare-clock` and `text-claim-untagged` on two lines of `docs/runner.md`, fixed before the commit). After the Task 2 tree: `tests-expected=4932 tests-executed=4928`, sh 97 s and dash 97 s, identical (md5 1c3fbdc21b65eb4b024eacaa413523ec). After the Task 3 tree: `consumer-lines=164 pkg-selftest=19/19`, sh and dash identical (md5 85df9854fe7b681008bb9ae238b80963), container exit 0 (452 s, gate-time under load). After the cost commit (the tree the five code commits end in): sh 95 s and dash 100 s, the identical summary line (md5 85df9854fe7b681008bb9ae238b80963), container exit 0, `gate: PASS`, 0 `FAIL` lines, `gate-time total=357`. On the closing tree (this SUMMARY, STATE.md and ROADMAP.md added or edited; `tracked=2463`): sh (`gate-time total=83`, `gate-cpu` user 145.4 sys 122.6) and dash (92 s, user 152.1 sys 129.6) `gate: PASS` with the identical summary line (md5 73f6a347858a61cb60394bf6e6058835), 0 `FAIL` lines, the plan-end counts above, and `GATE_LINUX_PLATFORM= sh tools/gate-linux.sh`: exit 0, `gate: PASS`, the plan-end counts, `core-imports=5`, 0 `FAIL` lines, `gate-time total=320` (`gate-cpu` user 1,176 sys 71 emulated); `build/gate-linux.log` holds that run. The dev-preset `ctest` suites `runner` (247 ids) and `runner-process` (130 ids, 2 skipped) and the release-preset ones pass; the 19 runner fault executables fail the ids their lines of `data/expected/faults/runner.ids` name and nothing outside the allowed prefixes.

## Known Stubs

None.

## Threat Flags

None new. T-08-37 (the hook changes a report or a golden): five scenario families compared with their plain reports apart from `host.calls` in both presets, a self-control that rejects a report that differs and a hook that did not cut, the 64-cut cap, the machine-time checks that turn a no-op cut into a failure, and every `.digests` golden and the 27 goldens unchanged. T-08-38 (a snapshot of an owner-image run in runner memory): accepted as planned; the buffer is a member of the executor state for the life of the run, is never written to a file, a report or a golden, and no file option exists in this phase; the hook is allowed for `content ... user` for that reason. The consumer writes nothing. T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: the SUMMARY exists; commits `7f98e77`, `336b3cb`, `5a71493`, `f195ffe` and `98b6302` are ancestors of HEAD and `git rev-list --count 2eaab6e7359e53ad9de0e662447f924d3d60b23f..HEAD` printed 5 before this file's commit; the files of the plan exist (FOUND for `runner/exec.c`, `tests/installed-consumer/main.c`, `data/expected/pkg-consumer.golden` and the SUMMARY); nothing is staged but the files of this commit and `.planning/config.json`, `.planning/state.json` and `.planning/ci-runs.tsv` are not staged; `git ls-files` lists no `.ivsc`, `.bin`, `.rom`, `.exec`, `.ivsn`, `.wav`, `.ppm` or `.vraw` file; the diff of the five commits outside `.planning/` holds no home, scratch or temp path and no "Intellivision"; no commit deletes a file and none touches a `core/` or `data/golden/` file; `git show 98b6302 -- data/expected/pkg-consumer.golden` removes 0 lines; `grep -c '^runner-process[.]roundtrip[.]' data/expected/tests/runner-process.ids` prints 6 and the judge id once; `grep -c 'srand\|time(\|clock(' tests/installed-consumer/main.c` prints 0; the claim words of the text step do not occur untagged in the new text of `docs/`.
