---
phase: 06-stic-video-first-pass
plan: 12
subsystem: runner, tests/runner, tools/check (golden step and controls), docs
tags: [runner, ivsr-3, capture-video, frame-asserts, video-chain-recheck, operator-outputs, frame-ppm, golden-video-row, checkpoint-C, D-16, D-17, D-21, VID-03, VID-04]
status: complete

requires:
  - phase: 06-stic-video-first-pass
    provides: "plan 06-09 video slots, domain 16 chain, report v3 fields, runner buffer sizing; plan 06-10 and 06-11 renderers for both card modes; plan 05-09 audio path as the analog; plan 05-REVIEW.md WR-01 and WR-02 as the output-file rules; gate steps=36/36 controls=332/332 faults=219/219"
provides:
  - "runner/runner_vchain.{c,h}: runner-side chain h0 = SHA-256 of IVS-VIDEO-LOG-V1, h(n+1) = SHA-256(h(n) || slot), the domain-16 digest it implies, slot header checks (index, width, height, reserved, flags, pixel range, tick order and spacing), and the canonical text hash of a plane"
  - "IVSS: capture video first count, assert frame pixel|hash|displayed, assert video count|chain; reasons 50 to 52 (IVS_SCN_R_COUNT 53), subjects 8 and 9"
  - "ivsr 3: output.video.frames, video.geometry, video.row_lines, video.frame.last_tick, video.chain.check after audio.chain.check; the video row of the golden keep list (six keys, only while video is observed)"
  - "operator outputs --video-raw, --video-out-max-bytes, --frame-ppm (mode 0600, exclusive create, removal by identity on harness-fail, update-golden refusal, user-content refusal); runner/frame_export.{c,h}"
  - "faults RUNNER_VIDEO_ASSERT_IGNORED and RUNNER_VIDEO_CHAIN_UNCHECKED; controls golden-domains-video-missing, golden-digest-video-missing, golden-video-chain-check-missing"
affects: [06-13, 06-14]

actuals:
  tokens: 70000
  tasks: 4
  commits: 4
plan_head_before: f75cd5986a44755faee307786e911b18c80af06e
plan_head_after: 21e8b71a9201d3a982337df422dd17a649479794

key-decisions:
  - "The golden keep list moved from main.c into the library (ivs_runner_golden_keep, report.c) so the in-process suite can test runner.golden.keys-video-live; main.c calls it, golden.sh and docs/runner.md carry the same list"
  - "The vtable gained write_video_raw and write_frame_ppm; the frame hook receives the PPM bytes (the converter runs in the library, so the byte bound and the user-content refusal are enforced in one place)"
  - "Frame asserts use ivs_scenario_assert.index for x and .addr for y (not addr_lo and addr_hi, which the parser fuzz invariant keeps ordered)"
  - "A capture, frame assert or video assert without a delivered video buffer is stream-without-buffer target=video; a pixel outside the plane is frame-pixel-range, never a vacuous != pass"

key-files:
  created:
    - runner/runner_vchain.c, runner/runner_vchain.h, runner/frame_export.c, runner/frame_export.h
    - tests/runner/runner_vchain.c
    - tools/check/controls/golden-domains-video-missing/, golden-digest-video-missing/, golden-video-chain-check-missing/
  modified:
    - runner/ivs_runner.h, scenario.c, exec.c, report.c, main.c, hostio.c, hostio.h, CMakeLists.txt
    - tests/runner/runner_parse.c, runner_exec.c, runner_fuzz.c, runner_main.c, runner_cases.h, tests/CMakeLists.txt, tests/runner-process-tap.sh
    - tools/check/steps/golden.sh, the seven toy reports of tools/check/controls, data/expected/controls/golden.ids
    - data/expected/tests/runner.ids, runner-process.ids, data/expected/faults/runner.ids, data/stic/tests-pending.ids
    - docs/runner.md, .planning/CI-EFFICIENCY.md

duration: about 6 hours elapsed on a loaded laptop (1-minute load 13 to 55)
completed: 2026-11-12
---

# Phase 6 Plan 12: runner and scenarios, ivsr 3, operator outputs, checkpoint C Summary

**A scenario can capture frames and assert pixels, the canonical text hash, the displayed flag, the delivered count and the recomputed chain; the runner re-hashes the core's frame chain from the slots it received and fails the run when the core and the delivery disagree; the report moved once to `ivsr 3` with five video keys; and an operator can export the raw slots or the last frame as a PPM through outputs that are created mode 0600, bounded, removed on a failed run and never evidence.** No golden, scenario or consumer golden moved.

Oracle commit this plan follows: `eae8ca5` (06-01 task 5). First commit of the plan (FIRST): `9aae179`. The 06-01 oracle files were read (the h2 chain literal of `data/video/chain-framing.literal.txt` and the hand text hash of a display-off frame are the independent expected values of the exec tests), never edited.

## What was built

- Task 1, tracer (`9aae179`): `runner_vchain`, the grammar, the executor (capture store of 16 slots, the five asserts, `video-chain`, `frame-not-captured`, `stream-without-buffer target=video`, `frame-pixel-range`, `hash-needs-colors-only`), the raw video hook with its byte bound, two faults, 28 test ids (parse 17, exec 5, vchain 6), seeded fuzz rows (a video base and dictionary tokens, new invariants).
- Task 2 (`6dcd44a`): `ivsr 3` and the five keys in every list at once, the golden video row, `golden.sh`, the seven toy reports, three controls, `runner.exec.report-keys-ivsr3`, `runner.golden.keys-video-live`.
- Task 3 (`0f47ce7`): `frame_export`, the mode argument of the create functions, `--video-raw`, `--video-out-max-bytes`, `--frame-ppm`, in-process and process-level tests.
- Task 4 (`21e8b71`): `docs/runner.md` grammar and options, checkpoint C, then the gates below.

## Key list as built (`ivsr 3`)

After `audio.chain.check` and before `host.calls`: `output.video.frames` (sum of `video_frames_completed`), `video.geometry` (`159x96`, `-` before a machine), `video.row_lines` (2), `video.frame.last_tick` (`-` before a frame), `video.chain.check` (`pass`, `fail`, `-`). The table lives in `runner/report.c`, `runner/ivs_runner.h`, the library golden keep list (`ivs_runner_golden_keep`), `tools/check/steps/golden.sh`, `docs/runner.md` and the independent table of `tests/runner/runner_exec.c` (`runner.exec.key-order`, `runner.exec.report-keys-ivsr3`). The video row of the golden keeps `digest.video` and the five keys only while `output.video.bytes` is a decimal other than 0 (the Phase 5 count rule); `audio.chain.check` stays outside every golden.

## Golden classification

None moved. `git diff --stat 9aae179^..HEAD -- data/golden data/scenarios data/expected/pkg-consumer.golden` prints nothing and `git status --short` of those paths is empty after every gate: the video row is live-gated, no gate scenario reaches its first frame end (the longest ends at 16,752 ticks, the first frame end is 59,736), so every golden body is byte-identical (`runner.golden.keys-video-live` also proves the filtered body equals the pre-ivsr-3 key list for a no-frame report). The installed consumer is untouched: video is not observed, with `digest.*` lines equal to the runner golden's (compared by `pkg-install` at every gate). Control files moved (the seven toy reports now carry `ivsr 3` and the five keys, three new controls); that is the whole re-bless.

## Tests, faults and controls

| Fault | Site | Must fail |
|-------|------|-----------|
| RUNNER_VIDEO_ASSERT_IGNORED | runner/exec.c | runner.exec.video-assert |
| RUNNER_VIDEO_CHAIN_UNCHECKED | runner/exec.c | runner.exec.video-chain-check |

Controls (335): `golden-domains-video-missing` (video observed, header says `video=none`), `golden-digest-video-missing`, `golden-video-chain-check-missing` (the golden body lacks the key while video is observed). Test ids added: 78 (`tests-expected` 4,304 to 4,382). Pending lines deleted with the first id of each family: `runner.vchain.` (task 1), `runner.golden.` (task 2), `runner.operator.` and `runner.frameppm.` (task 3).

Reviewed test edit (named): `runner.parse.refuse.bad-capture` used `capture video` as its example of a bad target; video is now a target (and has its own arity rules), so the case uses `capture colors`. No other existing test changed apart from the `ivsr 2` header literals and the key table (to `ivsr 3`) and the plain tail of `runner.exec.refpcm-keys-only-with-option` (now `video.chain.check` before `host.calls`).

## Checkpoint C (cost)

`checkpointC: no-lever`, recorded in `.planning/CI-EFFICIENCY.md` (section `Phase 6 checkpoint C (plan 06-12)`). Normalised metric `r = cpu_s / build_s` of the last five successful hosted rows is 7.88, 7.90, 5.95, 7.64, 7.68, median **7.68** against the budget median 7.8 and the trigger 8.3: not tripped, so **no lever pass is required**. The raw projection of the 06-02 formula is 307.0 (cumulative adds since 06-02: faults 193 to 221, controls 328 to 335, ids 3,778 to 4,382; `suite_term` 19.33 from loaded-laptop suite times), above the raw budget of 300, which the arbiter reads together with `r` (the last three hosted rows already contain plans 06-09 to 06-11 and sit at or under the budget). Local gates, four runs: 258, 170, 162, 142 s (373, 353, 354, 342 cpu-s) at a load of 21 to 36; the `controls` step is the largest in every run. `runner-process` ctest: dev 15.0 s, release 8.0 s. The orchestrator should read `r` from the first hosted row of this plan: at 8.3 or above the lever queue of the 06-07 review runs before plan 06-13.

## Deviations from Plan

**1. [Rule 3 - ordering] The raw video hook and its byte bound landed in Task 1, not Task 3.** The corruption test of `runner.exec.video-chain-check` (and its fault) needs a host that flips a delivered byte, which is the hook beside `write_audio_raw`; the `ivs_runner_options.video_out_max_bytes` field and the exec side of `video-out` and `video-out-max-bytes` came with it. Task 3 added the file side (`main.c`, `hostio.c`) and the PPM export.

**2. [Interpretation] Golden keep list moved into the library** (`ivs_runner_golden_keep` in `report.c`, declared in `ivs_runner.h`), so `runner.golden.keys-video-live` is an in-process test; `main.c` and the golden writer behave as before.

**3. [Interpretation] Process-level ids for the file behaviour.** The plan names `runner.operator.video-raw-*` for exclusive create, removal by identity and the update-golden refusal; those need `main.c` and `hostio.c`, which no in-process suite reaches, so they are `runner-process.operator.video-raw`, `.video-raw-empty-run`, `.video-raw-failed-run`, `.video-raw-update-golden-refused` and `.frame-ppm-refused-user-content`. The in-process ids are `runner.operator.video-raw-hook-and-byte-cap`, `runner.operator.frame-ppm-bytes-and-refusals`, `runner.frameppm.palette-covers-16` and `runner.frameppm.header-and-size`.

**4. [Interpretation] The frame hook takes PPM bytes** (not a plane): the library converts, so the byte bound, the conversion errors and the `content ... user` refusal (also in the library, detail `frame-ppm-user-content`, beside the process-level refusal of `main.c`) are enforced in one place.

**5. [Interpretation] Scenario token cap raised to 9** (`SCN_TOKS`) for the eight-token `assert frame <n> pixel <x> <y> <op> <v>`; every existing form checks an exact or bounded token count, so no accepted or refused text changed.

**6. [Rule 3] One test id depended on a fault it should not.** `runner.operator.frame-ppm-bytes-and-refusals` first also checked the ordinary missing-fixture outcome, which fails under `OUTCOME_MISSING_FIXTURE_EXIT0`; the fault gate named it, and the incidental check was removed.

## Decisions Made

- No requirement is closed: VID-03 and VID-04 stay Pending (plan 06-13 adds the scenarios and the two goldens, plan 06-14 closes the phase).
- `hash-needs-colors-only` is exercised on a synthetic plane (a display-on frame with a blank position needs the Foreground/Background cut-off column, which the cropped plane does not hold for this profile's first frames).

## Open Items

- Hosted `r` for this plan is not yet known; the arbiter is the first hosted row (the raw projection is above the raw budget, see checkpoint C).
- `runner.golden.keys-video-live` proves the keep list and the unchanged no-frame bodies; the golden of a scenario that DOES show frames arrives with plan 06-13 (`video.chain.check` pinned as `pass` there).
- Carried unchanged: Phase 4 and 5 deferrals, `sh tools/hygiene.sh --history` on earlier planning commits, the carried residual that the golden itself is still written with `fopen`.

## Final gate

```
sh tools/gate.sh:       gate: PASS steps=36/36 controls=335/335 faults=221/221 tests-expected=4382 tests-executed=4380 tests-skipped=2 tests-missing=0
dash tools/gate.sh:     the same summary line (identical md5 13b0192d4ec17b7f811c577d6c249f82 for the sh and dash runs)
sh tools/gate-linux.sh: GATE_LINUX_PLATFORM= run, exit 0, last line of build/gate-linux.log starts gate: PASS, x86_64 gcc 13.3.0 cmake 3.28.3, steps=36/36 controls=335/335 faults=221/221 tests-missing=0, 0 warnings (gate-time total=315, first run, no hosted-only finding)
```

## Known Stubs

None. (The `--frame-ppm` palette is documented as an arbitrary preview palette, not a stub.)

## Threat Flags

None new. T-06-16 is mitigated: exclusive create, removal by device and inode, update-golden refusal for the same string and for another spelling, byte bound, POSIX only in `hostio.c`, mode 0600, `runner-process.operator.video-raw*` and `runner.operator.*`. T-06-17 is mitigated: `--frame-ppm` is refused for `content ... user` in `main.c` (one stderr line, no file) and in the library (`frame-ppm-user-content`). T-06-SC: no installs.

## Self-Check: PASSED

Created files exist (`runner/runner_vchain.c`, `runner/frame_export.c`, `tests/runner/runner_vchain.c`, the three control directories); commits `9aae179`, `6dcd44a`, `0f47ce7`, `21e8b71` are ancestors of HEAD; `grep -c 'IVS_SCN_R_COUNT 53u' runner/ivs_runner.h` is 1; `grep -c '^checkpointC: no-lever$' .planning/CI-EFFICIENCY.md` is 1.
