---
phase: 05-hand-controllers-and-psg-sound
plan: 09
subsystem: runner, tests/runner, tests/installed-consumer, tools/check/steps, data/golden, data/scenarios, docs
tags: [ivsr-2, audio-chain-recheck, write-log-delivery, ivss-capture, operator-files, golden-rebless-4, consumer-alignment, D-18, D-19, D-20, A-09, A-11, A-12, AUD-02, API-03]
status: complete

requires:
  - phase: 05-hand-controllers-and-psg-sound
    provides: "plan 05-01 oracle commit 14a1904; plan 05-02 request and report v2 and ivs_info v3; plan 05-06 real audio layout, domain-17 chain, consumer count rule; plan 05-08 write log, domain-19 chain, W_log = 2,152 records per frame slot (HEAD at start 0253292)"
provides:
  - "ivs-run allocates one output block (8 MiB bound) and cuts it per slice into video, audio and write-log buffers from ivs_info; every scenario delivers audio (output.audio.bytes nonzero) and the log"
  - "report format ivsr 2: digest.psglog, output.audio.samples, output.audio.changes, output.psglog.records, output.psglog.bytes, audio.sample_ticks, audio.chain.check; every ivsr 1 literal moved in one plan"
  - "audio chain re-hash: runner_chain.c rebuilds the level-change chain and the domain-17 digest from the delivered bytes and compares with the core's digest; a mismatch is harness-fail audio-chain; the domain-17 preimage is documented in ivs_machine.h"
  - "golden key list with psglog in main.c, golden.sh and docs/runner.md, one count-based observed rule, # domains grammar with psglog=, 4 controls (307)"
  - "IVSS capture psglog|audio, assert psglog count and record forms, assert audio emitted and at, digest domain psglog; reasons 45..49"
  - "operator-only --audio-raw, --psg-log-out, --audio-out-max-bytes, exclusive create in hostio.c; hooks in the host vtable"
  - "golden re-bless #4 (classified) and the installed consumer agreeing with the runner golden on digest.audio and the observed set"
affects: [05-10, 05-11, 05-12]

actuals:
  tokens: 48600
  tasks: 5
  commits: 6
plan_head_before: 025329289e4b23353b5cc7de4ca4f9af49c80e14
plan_head_after: d6f7ab48d2b30b663740874be52ba06ead909b74

tech-stack:
  added: []
  patterns:
    - "A runner-local copy of the core SHA-256 (renamed symbols, no fault blocks) with its FIPS vectors in suite runner, because the runner library links only the public core"
    - "A corrupted-stream test seam through an operator hook: the fake host flips a delivered byte in its own block before the chain check reads it, so the check is shown to be a comparison"
    - "One shared assert clause function (ivs_runner_assert_clause) for the report line and the outcome detail"
    - "Capture storage lives in host storage after the instance and the read buffer (the library allocates nothing)"

key-files:
  created:
    - runner/runner_chain.c
    - runner/runner_chain.h
    - runner/runner_sha256.c
    - runner/runner_sha256.h
    - data/scenarios/psg-log-assert-mismatch.ivss
    - tools/check/controls/golden-domains-psglog-missing/
    - tools/check/controls/golden-digest-psglog-missing/
    - tools/check/controls/golden-output-key-unknown/
    - tools/check/controls/golden-refpcm-key/
  modified:
    - runner/main.c, exec.c, report.c, scenario.c, hostio.c, hostio.h, ivs_runner.h, CMakeLists.txt
    - core/include/ivs/ivs_machine.h
    - tests/runner/runner_exec.c, runner_parse.c, runner_fuzz.c
    - tests/runner-process-tap.sh
    - tests/installed-consumer/main.c
    - tools/check/steps/golden.sh
    - data/golden/diag-cpu-1.digests, diag-input-pass.digests, timeout-budget.digests, data/expected/pkg-consumer.golden
    - data/expected/runner-scenarios.ids, tests/runner.ids, tests/runner-process.ids, faults/runner.ids, controls/golden.ids
    - docs/runner.md
    - tools/check/controls/_good/tree (toy reports and golden), five control overlays

key-decisions:
  - "audio.sample_ticks and audio.chain.check sit after output.psglog.bytes and before host.calls; digest.psglog follows digest.input"
  - "audio.chain.check is `-` unless an audio buffer was delivered on every slice of at least one slice; a mismatch is harness-fail audio-chain"
  - "The golden keeps, for an observed live domain, its digest and its count keys; audio.chain.check and refpcm.* are never in a golden, and golden.sh refuses a refpcm. key or an output. or audio. key outside the list with its own message"
  - "The operator hooks are vtable entries write_audio_raw and write_psg_log (NULL: not requested) and the byte bound audio_out_max_bytes is an ivs_runner_options field; the bound applies to each file; the hook runs before the chain check reads the bytes"
  - "A capture or a psglog or audio assert with no output buffer is a harness failure (stream-without-buffer), never a vacuous pass"
  - "assert audio at <tick>: the levels of the last level-change entry at or before sample tick/16, else 0 0 0; a tick beyond the emitted samples fails the assert (not-emitted); a tick off the real sample grid is a harness failure (audio-tick-misaligned)"

duration: about 3 h of edits plus about a dozen gate runs
completed: 2026-11-12
---

# Phase 5 Plan 09: Runner and scenarios Summary

**`ivs-run` now requests the audio and write-log buffers through real memory, prints report format `ivsr 2`, rebuilds the AUDIO level-change chain from the bytes it received and compares it with the core's domain-17 digest (`audio.chain.check`), reads PSG log records and audio levels in IVSS asserts through `capture`, exports the raw streams on operator request into new files only, and the three goldens and the installed consumer agree on `digest.audio` and on the observed set; the gate ends `steps=35/35 controls=307/307 faults=189/189 tests-missing=0` under `sh`, `dash` and the Ubuntu container.**

Oracle commit (05-01): `14a1904` (no oracle file was edited). Plan start `0253292`.

## What was built

- Commit `b2bff1a` (Task 1, tracer): `main.c` allocates the block; `exec.c` cuts it three ways per slice (`bytes_per_frame * (slice_ticks / F + 1)`: audio 71 x 15,060 = 1,069,260 bytes and log 71 x 34,432 = 2,444,672 bytes at the default slice, about 3.5 MB), accumulates the three byte counts, feeds the delivered samples to `runner_chain.c` and checks the result against the core's domain-17 digest. `runner_sha256.[ch]` is a renamed copy of `core/src/common/ivs_sha256.[ch]` (the runner library may not reach core/src). The key table, every `ivsr 1` literal, the toy reports. Tests `runner.exec.ivsr2-keys`, `audio-chain-check`, `sha256-fips`. The domain-17 preimage is documented in `ivs_machine.h` (documentation only).
- Commit `cf3dbf8` (Task 2 and the golden re-bless): the golden key list in `main_golden_keep`, `golden.sh` and `docs/runner.md`; the count-based observed rule for video, audio, input and psglog; the `# domains:` grammar with `psglog=`; the consumer replay now passes audio and log buffers sized from `ivs_info` and decides observed by delivered bytes; four controls; the three `.digests` goldens and `pkg-consumer.golden` regenerated.
- Commit `6ccd0d8` (Task 3): the IVSS grammar and executor for `capture`, `assert psglog`, `assert audio`, digest domain `psglog`; parser reasons, fuzz, the new scenario.
- Commit `132db0a` (Task 4): `--audio-raw`, `--psg-log-out`, `--audio-out-max-bytes`; exclusive create in `hostio.c`; in-process and process tests.
- Commit `619db7a` (Task 5): the three named faults; commit `d6f7ab4`: the gcc 13 `-Wformat-truncation` fix (see Deviations).

## Golden classification (re-bless #4, commit cf3dbf8)

| File | Change | Class |
|---|---|---|
| `diag-cpu-1.digests`, `diag-input-pass.digests`, `timeout-budget.digests` | `# domains:` line (audio observed, `psglog=none`, `psglog=` added) | allowed |
| the same three | `output.audio.bytes` 0 becomes 520, 520 and 100 (one removed line, one added line each) | allowed |
| the same three | added: `digest.audio`, `output.audio.samples`, `output.audio.changes` (0), `audio.sample_ticks` (16) | allowed (new keys) |
| `pkg-consumer.golden` | added: `digest.audio` (equal to the runner golden's `6dc30474...31ab`) | allowed |
| every golden | `digest.cpu`, `digest.mem`, `digest.images`, `digest.instance`, `digest.composite`, `digest.input`, every `step.*`, `slot.*`, `cpu.*`, `input.*` line | byte-identical to the 05-08 tree (`git diff -U0` shows no removed line other than `# domains:` and `output.audio.bytes`) |

No `digest.psglog` line appears: no golden scenario writes a PSG register, so `psglog=none` for all three. The format line `ivsr` is not part of a golden, so no `ivsr 1` to `ivsr 2` change appears there. The classification was committed with the key-list change (Task 2) rather than in Task 5, so every task commit from Task 2 on is gate green.

## Parser reasons added (appended after `no-run-to` 44; `IVS_SCN_R_COUNT` 50)

`bad-capture` 45, `duplicate-capture` 46, `bad-psglog-assert` 47, `bad-audio-assert` 48, `assert-without-capture` 49: one refuse case each in `runner_parse.c` plus 14 `edge.*` cases and `accept.capture-forms`; `reasons-named` passes; the fuzz dictionary, a capture base and the structural rules cover the new fields (43 distinct reasons seen).

## Hand derivations in the tests

The stand-in EXEC (mixer `$3F`, amplitude A `$0F`, `$00`, `$07`, branch to itself): reset entry 5 CPU cycles, MVII 8, a direct MVO 11 with its write at cycle 6, 4 ticks per cycle, so the four writes are at ticks 76, 152, 228, 304 (addresses 8, 11, 11, 11; raw values `003F`, `000F`, `0000`, `0007`, source cpu-write). A sample at tick 16 k sees writes stamped below it: A is 0 at 144, 15 at 160 and 224, 0 at 240 and 304, 7 at 320. Three level changes, four log records, 64 bytes. The loop program of `runner.exec.capture-overflow` writes one record per 16 CPU cycles and passes 4096 records by tick 400,000.

## Evidence numbers

- Counts at plan end: `steps=35/35 controls=307/307 faults=189/189 tests-expected=3680 tests-executed=3678 tests-skipped=2 tests-missing=0` (05-08 end: 3610 expected, 186 faults, 303 controls).
- Suite `runner`: 153 ids at plan start, 156 at the tracer commit, 185 at the end; plain-run times (median of 3, loaded laptop): `runner` dev (ASan and UBSan) 0.65 s, release 0.29 s.
- Suite `runner-process`: 16 scenario cases (one new, `psg-log-assert-mismatch`) and 5 operator cases (`audio-raw`, `psg-log`, `exclusive`, `max-bytes`, `options`); process spawns added per preset: 1 + 7 = 8 (plan target at most 12); plain-run `ctest -R runner-process`: dev 4.2 s, release 2.3 s (05-08 and earlier: dev about 4.6 s, release about 2.3 s, the previous run on this laptop).
- Gate cost (local, loaded laptop): `sh` `gate-time total=37` with `gate-cpu` user 64.7 plus sys 52.4 = 117 cpu-s (05-08: 35 to 37 s, 107 to 119 cpu-s); `dash` `gate-time total=36`, 110.7 cpu-s; the container run took 107 s of gate time and 590 cpu-s emulated (a correctness check only). Hosted cost is unmeasured until the next hosted rows; the plan adds three fault runs (about 0.14 cpu-s each local) and one scenario run per preset.

## Deviations from Plan

**1. [Rule 3 - Plan ordering] Golden re-bless in Task 2, not Task 5.** The plan keeps the `golden` step red from Task 1 to Task 5. The keys of the goldens only depend on Tasks 1 and 2, so the three `.digests` goldens and `pkg-consumer.golden` were regenerated at the end of Task 2 and Tasks 2 to 5 end gate green; Task 1 alone (`b2bff1a`) ends with `golden` red (34/35), as the plan states. Task 5 carries the faults and the final gates.

**2. [Rule 3 - Blocking] The operator hooks and the audio-out bound are in the library from the Task 3 commit.** The corrupted-stream test of `runner.exec.audio-chain-check` needs a seam between the run call and the chain feed (a fake host cannot reach the library's buffer otherwise); the raw-audio hook is that seam and is also the operator hook, so `write_audio_raw`, `write_psg_log` and `audio_out_max_bytes` landed before the files (Task 4).

**3. [Rule 1 - Plan precision] `runner.parse.options-once` is `runner-process.operator.options`.** The option parser is in `main.c`, outside the library, so it cannot be tested in process; the check (an option twice, a non-digit bound, a zero bound: exit 14, no report, no file created) runs in the process script with 3 spawns.

**4. [Rule 2 - Missing critical functionality] `stream-without-buffer`.** A capture or a psglog or audio assert on a host without output buffers would read nothing and could pass vacuously (a `!=` over an empty log); it is a harness failure instead. The storage for the captures (`IVS_RUNNER_CAPTURE_BYTES`, 114,688 bytes after the read buffer) is added to `main.c` and required only when a scenario captures.

**5. [Rule 1 - Hosted-only hazard] gcc 13 `-Wformat-truncation`.** The first container gate failed the build of `runner_exec.c` (a 511 byte buffer printed into a 488 byte one); the buffers were narrowed (`d6f7ab4`) and the container gate then passed. Clang did not warn.

**6. [Other] Extra ids and files.** `runner.exec.sha256-fips` (the runner-local SHA-256 against FIPS 180-4 and the `data/sha256/vectors.txt` literals), `runner_chain.[ch]` and `runner_sha256.[ch]` (the plan allowed the copy), `psg-log-assert-mismatch` in `runner-scenarios.ids`, `ivs_runner_assert_clause` shared by the report and the detail. `pkg-install.sh` and `pkg-selftests.ids` needed no edit: `check_runner_golden` compares digest lines and the observed set by name, so the ninth domain and the new header grammar are handled as they are.

## Decisions Made

- The runner decides `audio.chain.check` from the bytes it received, not from the layout: no audio buffer on some slice gives `-` (never `pass`), and a failure ends the run as harness-fail rather than guest-fail because it is a disagreement between the core and what it delivered.
- `output.audio.changes` is the runner's own count; it equals the core's change count only through the chain check (a mismatch fails the run).

## Known Stubs

None.

## Open Items

- **AUD-02 and API-03 stay Pending in REQUIREMENTS.md.** The runner and the consumer now deliver, assert and export the log and the raw audio and share the golden rule; the reference PCM location (plan 05-10), the end-to-end evidence (05-11) and the registry edit (05-12) close AUD-02, and the controller evidence of API-03 is closed with the end-to-end plan.
- `audio-out-max-bytes` also bounds the log file (each file separately); a later reader may prefer a second option.
- `--audio-raw` leaves a partial file behind on a harness failure (a file the operator asked for; it is never overwritten later because of exclusive create).
- The `diag-boot-pass` and `diag-input-pass` runs give `output.audio.changes 0` (a silent guest); the three-change evidence is from the in-process stand-in and the process-level stand-in only.
- Carried unchanged: the 05-04 unobtained AY-3-8910 reprint, D10 and D11 items, Phase 4 items IN-04, IN-06, IN-09 to IN-11, the C73 tag wording of 05-05, `psg.mix.truth-table` polarity invariance (05-06), the noise first-bits and envelope phase conventions (05-07), the interrupt-entry bound inputs (05-08).

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=35/35 controls=307/307 ... faults=189/189 ... tests-expected=3680 tests-executed=3678 tests-skipped=2 tests-missing=0`, 0 FAIL lines, `gate-time total=37`.
- `dash tools/gate.sh`: the same line, 0 FAIL lines, `gate-time total=36`.
- `sh tools/gate-linux.sh`: Ubuntu 24.04 container (gcc 13.3, cmake 3.28, dash): `gate: PASS steps=35/35 controls=307/307 ... faults=189/189 ... tests-expected=3680 tests-executed=3678 tests-skipped=2 tests-missing=0`, `cc=GNU` (13.3.0), 0 FAIL lines, exit status 0, `build/gate-linux.log` ends with the `gate: PASS` line.

## Threat Flags

None new. T-05-22 (a directive naming a file or a rate): the grammar has none (parser refuse cases, options are operator flags only). T-05-23 (overwrite or unbounded output): exclusive create (`runner-process.operator.exclusive`), the byte bound (`runner.exec.out-max-bytes`, `runner-process.operator.max-bytes`), POSIX confined to `hostio.c` (runner-scan green). T-05-24 (capture overflow dropping evidence): `capture-overflow`, `runner.exec.capture-overflow` and the fault `RUNNER_CAPTURE_OVERFLOW_SILENT`. T-05-SC: no installs.

## Self-Check: PASSED

- Files exist: runner/runner_chain.c, runner/runner_chain.h, runner/runner_sha256.c, runner/runner_sha256.h, data/scenarios/psg-log-assert-mismatch.ivss, the four new control directories.
- Commits on HEAD: b2bff1a, cf3dbf8, 6ccd0d8, 132db0a, 619db7a, d6f7ab4 (`commits: 6` measured from the ledger, `git rev-list --count 0253292..HEAD`).
