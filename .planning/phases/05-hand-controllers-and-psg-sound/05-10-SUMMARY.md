---
phase: 05-hand-controllers-and-psg-sound
plan: 10
subsystem: tools/pcm, runner, tools/check, tests/runner, docs
tags: [reference-pcm, wav, d7, d-21, d-22, a-07, guards, judge-self-controls, listen-demo, AUD-02]
status: complete

requires:
  - phase: 05-hand-controllers-and-psg-sound
    provides: "plan 05-01 oracle commit 14a1904; plan 05-07 psg.dac.levels probe (cmake/IvsPsgProfileProbe.cmake); plan 05-09 operator files in hostio.c, the audio chain check, runner_sha256 (HEAD at start 0d8d81d)"
provides:
  - "tools/pcm: ivs_refpcm.h/.c, a pure integer reference PCM converter (exact rational box filter, one-pole DC block with caller-owned state, signed 16-bit mono, 44-byte WAV header builder), internal static target, no install rule"
  - "runner --audio-wav <file> with --wav-rate 44100|48000 (default 48000), report keys refpcm.rate, refpcm.frames, refpcm.sha256 only with the option, right before report.end"
  - "tools/listen-demo.sh: operator helper that renders one scenario to build/listen/<id>.wav and refuses any scenario with a content-user line (exit 2) before building"
  - "the four guards that never covered tools/pcm extended with 5 controls (A-07): integrity SPDX, c-build unlisted-source list, runner-scan include allowance, core-symbols defined-symbol rule; plus the .refpcm golden grammar, wav as a binary extension and *.wav in .gitignore"
  - "16 converter tests and judge self-controls in suite runner, 4 runner-library tests, 2 named faults, 2 process tests"
affects: [05-11, 05-12]

actuals:
  tokens: 118000
  tasks: 5
  commits: 5
plan_head_before: 0d8d81d45e55a130f8a1c16337b2aa82538f43a7
plan_head_after: eefc23135ba8e811f848ccbfd918c7e55451fbb8

tech-stack:
  added: []
  patterns:
    - "A converter that sits outside every installed artifact and is guarded by the exact install inventory plus a defined-symbol rule, not by a deny-list label"
    - "A WAV written through an exclusively created read-write file: 44 reserved bytes, PCM appended slice by slice, header written at offset 0 after the stream, the finished file read back and hashed"
    - "Judge self-controls in suite (a predicate applied to a deliberately perturbed result must fail) where the fault framework cannot host a module (core and runner only)"
    - "The control tree is a toy CMake project, so the c-build step has a positive and a negative control"

key-files:
  created:
    - tools/pcm/ivs_refpcm.h
    - tools/pcm/ivs_refpcm.c
    - tools/pcm/CMakeLists.txt
    - tools/listen-demo.sh
    - tests/runner/runner_refpcm.c
    - data/expected/controls/core-symbols.ids
    - tools/check/controls/integrity-missing-spdx-tools-pcm/
    - tools/check/controls/c-build-unlisted-pcm-source/
    - tools/check/controls/runner-scan-header-tools-other/
    - tools/check/controls/core-symbols-pcm-defined/
    - tools/check/controls/golden-refpcm-grammar/
    - tools/check/controls/_good/tree/CMakeLists.txt
    - tools/check/controls/_good/tree/CMakePresets.json
    - tools/check/controls/_good/tree/tools/pcm/good.c
    - tools/check/controls/_good/tree/data/golden/toy.refpcm
  modified:
    - CMakeLists.txt, runner/CMakeLists.txt, runner/main.c, runner/exec.c, runner/report.c, runner/hostio.c, runner/hostio.h, runner/ivs_runner.h
    - tools/check/integrity.cmake, tools/check/steps/c-build.sh, tools/check/steps/runner-scan.sh, tools/check/steps/golden.sh
    - tools/check/core-symbols.sh, tools/check/core-symbols-deny.txt, tools/check/binary-ext.txt, .gitignore
    - tests/CMakeLists.txt, tests/faults.cmake, tests/runner-process-tap.sh, tests/runner/runner_exec.c, runner_parse.c, runner_cases.h, runner_main.c
    - data/expected/tests/runner.ids, runner-process.ids, faults/runner.ids, controls/runner-hygiene.ids, controls/golden.ids
    - docs/runner.md

key-decisions:
  - "D7 is implemented as decided (D-21): no PCM, resampling, filtering, DAC-table use or WAV code in ivs_core or any installed artifact; the converter is internal, linked only by the runner library (for --audio-wav) and suite runner; nothing in the digests, claims or important-gates rows for AUD-02 depends on it"
  - "Sample phase (not specified by the plan): the 0-based sample i is stamped (i + 1) * P and is held from its stamp; the first P ticks hold level 0, one implicit silent cell before sample 0 (the first run of the converter treated cell i as [iP, (i+1)P), which is one sample early against the core's stamps; corrected in e98c1ff and all hand numbers re-derived)"
  - "WAV header decision: the file is created exclusively read-write with 44 reserved bytes, the PCM follows slice by slice, and after the stream the header is written at offset 0 (pwrite) and the finished file is read back and hashed, so refpcm.sha256 is the SHA-256 of the file as it lies on disk; the PCM is never buffered in memory"
  - "The hook contract: write_wav_pcm gets little-endian PCM bytes in order, finish_wav gets the 44-byte header and returns the file hash; both hooks or neither (one alone is harness-fail without a report); a write error, the byte bound or a missing audio buffer ends harness-fail and the file is not finished (sha256 `-`)"
  - "The --wav-rate check is the library function ivs_runner_wav_rate_valid (exec.c): runner/main.c sits outside the fault framework, so the check the in-process judge and the fault RUNNER_WAV_RATE_UNCHECKED reach had to be a library function that main.c calls"
  - "The defined-symbol rule of core-symbols.sh is always on (no flag) and covers file-local symbols as well: the core may DEFINE no name containing refpcm, resample, wav or pcm in any letter case"
  - "A .refpcm golden is validated for grammar only (tier, oracle self-recorded, label line, three body keys); it is never compared with a runner report, no .refpcm golden exists until a later plan records one"

duration: about 4 h of edits plus 7 gate runs (3 sh, 1 dash, 1 container)
completed: 2026-11-12
---

# Phase 5 Plan 10: Reference PCM converter and WAV helper Summary

**`tools/pcm` holds a pure integer reference PCM converter ("reference conversion v1, converter repeatability only") that only the runner's operator-side `--audio-wav` and suite `runner` use; the four guards that never covered it are extended with five controls, the core is proven free of PCM symbols and files, and the gate ends `steps=35/35 controls=312/312 faults=191/191 tests-missing=0` under `sh`, `dash` and the Ubuntu container.**

Oracle commit (05-01): `14a1904` (no oracle file was edited). Plan start `0d8d81d`.

## What was built

- `7607dea` (Task 1, tracer): `tools/pcm/ivs_refpcm.[ch]` (caller-owned state, `init`, `feed`, `out_bound`, `floor_div`, `dac_level`, `wav_header`; includes only `<stdint.h>` and `<stddef.h>`; no `float` or `double` token), `tools/pcm/CMakeLists.txt` (internal STATIC target `ivs_refpcm`, explicit source list written to `build/<preset>/ivs-pcm-sources.txt`, no install rule), `runner.refpcm.box-exact-vector` with toy rational parameters.
- `0ec7ac7` (Task 2): 12 `runner.refpcm.*` cases (hand-derived frame counts at both rates, split invariance, DAC copy against the profile probe, WAV header literals, refusals that consume nothing, the pitch check through a core run) and the three judge self-controls.
- `4f49ff1` (Task 3): the guard extensions and their 5 controls (below).
- `e98c1ff`: the sample-phase correction (Deviation 3).
- `eefc231` (Task 4, with the faults of Task 5 because a fault site must be registered to keep the gate green): the runner wiring, `tools/listen-demo.sh`, the tests, `docs/runner.md`.
- Task 5 is this record: the three D7 proofs and the three gates (no code).

## The guards extended (A-07), five controls

| Guard | Edit | Control |
|---|---|---|
| integrity SPDX rule | `tools/(pack\|cart)/` becomes `tools/(pack\|cart\|pcm)/` | `integrity-missing-spdx-tools-pcm` |
| c-build unlisted-source guard | fourth item `^tools/pcm/.*[.]c$ ivs-pcm-sources.txt`, written by `tools/pcm/CMakeLists.txt` | `c-build-unlisted-pcm-source` |
| runner-scan include rule | allows exactly the quoted operand `ivs_refpcm.h`; any other `tools/` header, a neighbour of that name, the angle form fail `runner-scan-header` | `runner-scan-header-tools-other` (line 5 allowed and in FORBID, lines 6 to 8 expected failures) |
| core-symbols.sh | defined-symbol rule `core-symbols-pcm-defined` (refpcm, resample, wav, pcm; deny-list labels added) | `core-symbols-pcm-defined`, driven through the c-purity step with an overlay core source, so the existing step and archive path are used |
| golden.sh | `.refpcm` golden grammar: `# tier: deterministic-repeatability`, `# oracle: self-recorded`, `# label: reference conversion v1, converter repeatability only`, `# scenario:`, body `refpcm.rate`, `refpcm.frames`, `refpcm.sha256` only | `golden-refpcm-grammar` (no label line and a non-refpcm body key, both expected) |

Also: `wav` in `tools/check/binary-ext.txt` and `*.wav` in `.gitignore`. The c-build control needed the control tree to be a toy CMake project (Deviation 2).

## The three D7 proofs

1. `data/expected/pkg-install-files.ids` is unchanged since the plan start (`git diff 0d8d81d -- data/expected/pkg-install-files.ids` is empty) and the gate step `pkg-install` installs the release tree and compares it with that list: no PCM, resample, WAV or converter file is in the install prefix.
2. The public-symbol rule of `pkg-install` (`public-symbols=14`) and the new `core-symbols-pcm-defined` rule (inside `c-purity`, on fresh `-O0` and `-O2` archives, and in the ctest suite `core-symbols` on the shipped archive) pass on the real core; a direct `nm -P` of `build/release/core/libivs_core.a` and the dev archive lists no defined symbol matching `refpcm|resample|wav|pcm`.
3. `git ls-files | grep -i 'refpcm\|resample'` lists `tools/pcm/` (2), `tests/runner/runner_refpcm.c` and the control files whose names carry `refpcm` by design (`golden-refpcm-grammar`, the older `golden-refpcm-key`, `_good/tree/data/golden/toy.refpcm`); no file under `core/`, `runner/`, `include/` or the install tree. `git ls-files '*.wav'` is empty; `git diff --stat 0d8d81d -- data/golden` is empty.

## Evidence numbers

- Counts at plan end: `steps=35/35 controls=312/312 faults=191/191 tests-expected=3724 tests-executed=3722 tests-skipped=2 tests-missing=0` (05-09 end: 3680 expected, 307 controls, 189 faults).
- Suite `runner`: 185 ids at the 05-09 end, 205 now (16 converter and judge ids, `runner.exec.refpcm-keys-only-with-option`, `runner.exec.wav-hook`, `runner.exec.wav-failures`, `runner.parse.wav-rate-refused`). Plain-run times (median of 3, laptop at load average 6 to 8): `runner` dev (ASan and UBSan) 0.67 s, release 0.31 s (05-09: 0.65 and 0.29); the whole `runner.refpcm.` family 0.10 s dev, 0.01 s release, of which the pitch case is 0.10 s dev and 0.00 s release (a 1.1 s emulated run of the core is a few hundred thousand cheap steps).
- Suite `runner-process`: two new cases (`runner-process.refpcm.wav-written`, 4 ivs-run spawns per preset; `runner-process.listen-demo.refuses-user`, no ivs-run spawn); plain-run `ctest -R runner-process` dev 4.5 s, release 2.6 s (05-09: 4.2 and 2.3; the machine was loaded at load average 6 to 11).
- Pitch check (`runner.refpcm.pitch-within-1pct`, TP = 291, a project stand-in that writes mixer `$3E`, amplitude `$0F`, fine `$23`, coarse `$01`, run to 4,000,000 ticks = 1.117 s, 53,638 frames at 48,000 Hz): measured error -20 ppm against `3579545 / (32 * 291)` = 384.4 Hz, bound 1 percent.
- Gate cost (local, loaded laptop, load average 6 to 11): `sh` `gate-time total=36` and `gate-cpu` 114 cpu-s at the end of Task 3, `total=41` and 131 cpu-s with everything in; `dash` `total=41`, 135 cpu-s; 05-09 ended at 117 cpu-s. The additions that cost something are the toy `c-build` control (two configures and builds of an empty project in two controls, under 1 s each), two fault executables with one run each (about 0.14 cpu-s each local) and four ivs-run spawns per preset. Hosted cost is unmeasured until the next hosted rows (last-5 medians gate 78 s, cpu 231 against the triggers 90 and 280; the last row was an outlier at 103 and 323).
- Container (Ubuntu 24.04, gcc 13.3, cmake 3.28, dash): `gate-time total=175`, exit 0, `build/gate-linux.log` ends with the `gate: PASS` line.

## Deviations from Plan

**1. [Rule 3 - Blocking] The fault site for the rate check is a library function, not `main.c`.** `runner/main.c` and `hostio.c` are outside the fault framework (no in-process suite reaches them), so a fault there could not fail the in-process `runner.parse.wav-rate-refused`. `--wav-rate` is validated by `ivs_runner_wav_rate_valid` in `exec.c` (which `main.c` calls for the usage error and the executor calls again); `RUNNER_WAV_RATE_UNCHECKED` makes it accept everything. The repeated-option and missing-value shapes of `--wav-rate` are `main.c` behaviour shared with every option (`option given twice`, `option without a value`) and are covered at process level by the existing `runner-process.operator.options` pattern; the new process case covers `--wav-rate 8000` and `--wav-rate` without `--audio-wav`. The collateral failures of the two faults (`runner.exec.ivsr2-keys`, `runner.exec.key-order`, `runner.exec.wav-failures`) are the allowed prefixes of their lines.

**2. [Rule 3 - Blocking] The c-build control needed the control tree to be a toy CMake project.** `c-build` had no control at all and its positive control (every distinct CHECK step must pass on the pristine toy tree) would build the presets. `tools/check/controls/_good/tree` gained `CMakeLists.txt` and `CMakePresets.json` (dev and release presets, the four source lists, a target `ivs_faults`, nothing compiled) and `tools/pcm/good.c`; the step is unchanged apart from the fourth list. Cost: under 1 s of cmake per c-build control run.

**3. [Rule 1 - Plan precision] The sample phase.** The plan does not say whether the 0-based sample i holds from `i * P` or from its stamp `(i + 1) * P`. The first version treated cell i as `[iP, (i+1)P)`, one sample (16 ticks) earlier than the core's stamps (the process test shows the level-15 sample at byte offset 36, stamp 160). Commit `e98c1ff` holds each sample from its stamp with one implicit silent cell before sample 0; the tracer vector, the frame counts, the settling frame and the no-clip numbers were re-derived by hand (an independent test-side model agrees).

**4. [Rule 3 - Blocking] The pitch stand-in lives in `runner_refpcm.c`, not `machine_standin.c`.** The suite `runner` links only the public core; `machine_standin.c` includes core internals. The test builds an instance through the public API with its own 18-word program (the plan 05-06 tone program with TP 291). `tests/machine/machine_standin.c` is unchanged.

**5. [Rule 3 - Blocking] `tests/faults.cmake` and the suite source list.** A plain copy of a runner TU now includes `ivs_refpcm.h`, so `ivs_fault_tu_setup` gives runner TUs the `tools/pcm` include directory and the runner suite's fault objects compile `ivs_refpcm.c` themselves (`ivs_fault_suite(runner SOURCES ... tools/pcm/ivs_refpcm.c)`). Neither is in the plan's file list.

**6. [Other] Ids, files and commit grouping.** Extra ids: `runner.refpcm.refusals-consume-nothing`, `runner.exec.wav-failures`. The new control inventory file `data/expected/controls/core-symbols.ids` is created (the plan names it); the three other controls are in `runner-hygiene.ids` and `golden.ids`. The two faults are registered in the Task 4 commit with their sites (a registered-site check keeps the gate red otherwise). The plan text says `311/311` once in a verify line; the count is 312. The default helper scenario `diag-ctrl-psg-pass` does not exist until plan 05-11, so `sh tools/listen-demo.sh` with no argument exits 2 (`no scenario`) today; run on `diag-input-pass` the helper rendered `build/listen/diag-input-pass.wav` (100 bytes, 28 frames) with its sha256.

## Decisions Made

See the key-decisions above. Added: `refpcm.sha256` is the host's answer to `finish_wav`, so a runner library user other than `ivs-run` must hash the file itself; the report prints `-` for a file that was not finished.

## Known Stubs

None. The `.refpcm` golden has grammar rules and no file yet by plan.

## Open Items

- **AUD-02 stays Pending in REQUIREMENTS.md.** This plan closes the D7 location (SC5 second half) with gate evidence; the end-to-end evidence (05-11) and the registry edit (05-12) still close the requirement.
- A `.refpcm` golden is validated for grammar only and is never compared with a rendered WAV or a runner report; the plan that records the first one (05-11, if any) decides how it is produced and compared.
- No listenable-tone statement exists for any Phase 5 artifact: the WAV of the project diagnostic is near-silence (the tone period write takes effect only after the 4096-step first reload), so the D-22 listening handoff is recorded as not performable until a longer guest exists.
- The DC block's floor leaves a residual of -1 after a downward step (documented in the header; accepted).
- A harness failure of `--audio-wav` leaves the partial, header-less file behind (as `--audio-raw` does); exclusive create keeps it from being overwritten later.
- `ivs_runner_options` grew two fields (`wav_rate`, `reserved`, must be 0) and the host vtable two hooks; library users other than `ivs-run` and the tests do not exist.
- Hosted cost of this plan is unmeasured until the next hosted rows.
- Carried unchanged: the 05-04 unobtained AY-3-8910 reprint, D10 and D11 items, Phase 4 items IN-04, IN-06, IN-09 to IN-11, the C73 tag wording of 05-05, `psg.mix.truth-table` polarity invariance (05-06), the noise first-bits and envelope phase conventions (05-07), the interrupt-entry bound inputs (05-08).

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=35/35 controls=312/312 ... faults=191/191 ... tests-expected=3724 tests-executed=3722 tests-skipped=2 tests-missing=0`, 0 FAIL lines.
- `dash tools/gate.sh`: the same line, 0 FAIL lines, `gate-time total=41`.
- `sh tools/gate-linux.sh`: Ubuntu 24.04 container (gcc 13.3, cmake 3.28, dash): `gate: PASS steps=35/35 controls=312/312 ... cc=GNU ... faults=191/191 ... tests-expected=3724 tests-executed=3722 tests-skipped=2 tests-missing=0`, exit status 0, `build/gate-linux.log` ends with the `gate: PASS` line.

## Threat Flags

None new. T-05-25 (PCM or the DAC table in the core or the package): the defined-symbol rule with its control, the unchanged exact install inventory, no install rule for `tools/pcm`, `psg.dac.not-in-core`. T-05-26 (rendered audio of rights-bearing content committed or shared): `wav` is a binary extension and `*.wav` is ignored, the WAV is written only to an operator path or the gitignored build tree, and `tools/listen-demo.sh` refuses `content ... user` scenarios (`runner-process.listen-demo.refuses-user`). T-05-SC: no installs.

## Self-Check: PASSED

- Files exist: tools/pcm/ivs_refpcm.h, tools/pcm/ivs_refpcm.c, tools/pcm/CMakeLists.txt, tools/listen-demo.sh, tests/runner/runner_refpcm.c, data/expected/controls/core-symbols.ids, the five control directories.
- Commits on HEAD: 7607dea, 0ec7ac7, 4f49ff1, e98c1ff, eefc231 (`commits: 5` measured from the ledger, `git rev-list --count 0d8d81d..HEAD`).
