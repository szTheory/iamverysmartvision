# The headless runner (`ivs-run`)

`ivs-run` executes one IVSS v1 scenario on one freshly created machine instance and prints a flat, deterministic report on stdout (RUN-01, RUN-02).
It is a host of the machine core like any other: it links only the public `ivs_core` library and uses the public API (`ivs/ivs.h`), so it can see nothing the installed package does not offer.
The machine core keeps no host time; timeouts and cancellation exist only in the runner (D-21).

Layout (D-17): the library `ivs_runner_lib` (`runner/scenario.c` parser, `runner/exec.c` executor, `runner/report.c` report writer, `runner/outcome.c` outcome mapping) and the thin executable built from `runner/main.c` (options, exit code) and `runner/hostio.c` (the only file that uses POSIX: files, the monotonic timer, signals).
One process handles one scenario on one instance; several scenarios mean several invocations.
The program keeps no global state except the cancel flag.
The runner provides one output block of at most 8 MiB and cuts it per slice into the video, audio and PSG write-log buffers from `ivs_info` (`bytes_per_frame * (slice_ticks / nominal_frame_ticks + 1)` for audio and the log; video is `video_bytes_per_frame * (slice_ticks / nominal_frame_ticks + 1 + video_slack_frames)`, because a call can complete one frame event more than that count; the video slot is the 32-byte header plus 159 x 96 index bytes, 15,296 bytes, with a slack of one frame; at the default slice of 4,194,304 ticks that is 72 video slots of 15,296 bytes, 1,101,312 bytes, and 4,615,244 bytes for the three channels, inside the 8 MiB block; a slice of 1 tick gets two video slots), so every scenario delivers audio and the log through the public run call and a run that crosses a frame end delivers its slot; the files of the three options above are written in `runner/hostio.c` only, and an operator file belongs outside the repository.

The runner is not installed (D-24): it is built in both presets (`build/<preset>/runner/ivs-run`) for the gate, and the install package and the installed consumer do not need it.

## Command line

```
ivs-run [options] <scenario.ivss>
```

| Option | Value | Meaning |
|---|---|---|
| `--content-dir <dir>` | a directory | Where the scenario's relative content paths are resolved; default: the directory of the scenario file. Never the working directory. |
| `--exec <file>` | a file | The operator's image for `content exec user`. |
| `--grom <file>` | a file | The operator's image for `content grom user`. |
| `--cart <file>` | a file | The operator's image for `content cart user`. |
| `--max-ticks <n>` | 1 to 9223372036854775808 | Machine-time ceiling in master ticks (default 1099511627776, that is 2^40). A run-to whose machine time stops below its bound because of the ceiling ends as timeout (budget); a last unit that reaches the bound anyway is not a timeout. |
| `--wall-timeout-ms <n>` | 1 to 4294967295 | The host wall-time cap in milliseconds (default 30000). A scenario's `wall-timeout-ms` can only lower it. |
| `--slice-ticks <n>` | 1 to 4194304 | Master ticks per slice (default 4194304, that is 1 << 22). |
| `--self-signal-after-slices <n>` | 0 to 4294967295 | Test hook: raise SIGINT after slice n (0: before the first slice). |
| `--self-signal-count <n>` | 1 to 4 | Test hook: raise SIGINT n times at that point (default 1); needs `--self-signal-after-slices`. |
| `--update-golden <file>` | a file | After the run, also write the run's repeatability golden (`.digests`, below) to the file, overwriting it if it exists. Refused (exit 14, one line on stderr) for a scenario with `user` content (before the run, no report), for a host-dependent outcome (cancelled, or timeout with detail `wall`), and when the path is one of the operator output files below (review WR-02): the same string is refused before anything is created, and the same file under another spelling or link (`./x` for `x`) is refused once the output files exist, which are then removed again; no report, one line `--update-golden must not name an operator output file`. |
| `--audio-raw <file>` | a new file | Also write the dense audio sample bytes (4 per sample: levels A, B, C, 0), slice by slice, to the file. Operator only: a scenario cannot name a file or a rate (plan 05-09, D-20). The file is created exclusively; an existing path of any kind is refused (exit 14, one line on stderr, no report) and is never overwritten or truncated. A run that ends harness-fail (exit 14, whatever the cause: a bound, a failed write, a refused run, a failed report write or golden) removes the files it created, one stderr line per file, so a failed run never leaves a short or unfinished file and the retry with the same path is not blocked (review WR-01); the removal compares the identity (device and inode) recorded at creation, so a file put at that path meanwhile is left alone. Every other outcome keeps the files: they hold the whole run up to its end (a budget timeout, a guest failure, a cancellation). |
| `--psg-log-out <file>` | a new file | Also write one ASCII line per PSG write-log record, `index tick addr value source` (decimal fields: the record number, the master tick of the write, the low nibble of the register address, the raw 16-bit value, source 1 for a CPU write and 2 for the write of an interrupt-entry cycle). Same exclusive-create rule. |
| `--audio-wav <file>` | a new file | Also write the dense audio as a mono 16-bit WAV made by the reference converter (section "The operator WAV", label `reference conversion v1, converter repeatability only`). Operator only; the same exclusive-create rule; the report then ends with `refpcm.rate`, `refpcm.frames` and `refpcm.sha256`. Never a golden, never a claim (plan 05-10, D-21). |
| `--wav-rate <n>` | `44100` or `48000` | The WAV rate (default 48000); any other value is a usage error (exit 14, no report, no file); needs `--audio-wav`. |
| `--audio-out-max-bytes <n>` | 1 to 1099511627776 | The byte bound of each of the three files above (default 268435456, 256 MiB; the WAV counts its 44 header bytes). Passing it is a harness failure (detail `audio-out-max-bytes`, `psg-log-out-max-bytes` or `wav-out-max-bytes`, exit 14) and the files are removed (above): never a truncation on disk. |
| `--video-raw <file>` | a new file | Also write the delivered video frame slots, each with its 32-byte header (a self-describing stream), slice by slice, to the file (plan 06-12, D-17). Operator only: a scenario cannot name a file. Created exclusively with mode 0600 (the audio files keep 0644): an existing path is refused (exit 14), and the file of a run that ends harness-fail is removed by device and inode identity, as for the audio files. `--update-golden` is refused when it names this path (the same string, or the same file under another spelling). Never a golden, a claim, a CI upload or evidence. |
| `--video-out-max-bytes <n>` | 1 to 1099511627776 | The byte bound of `--video-raw` and of `--frame-ppm` (default 268435456, 256 MiB). Passing it is a harness failure (detail `video-out-max-bytes` or `frame-ppm-out-max-bytes`, exit 14) and the file is removed, never a truncation. |
| `--frame-ppm <file>` | a new file | Also write the LAST delivered frame as a binary PPM (P6, 255) after the run, in the operator preview palette: sixteen arbitrary project-chosen colors plus one for a blank position (`operator preview palette, not STIC colour data`, runner/frame_export.h). Created exclusively with mode 0600 and cleaned up like `--video-raw`; a run that delivers no frame leaves an empty file. Refused (exit 14, one stderr line, no report, no file) for a scenario that names `content ... user`: a frame rendered from an owner image is derived art of rights-bearing content. An exported frame is never evidence: no golden, claim, digest, CI upload or test reads it, and a viewer's judgement is never a test. |
| `--help` | | Print the options and one `<outcome> <exit>` line per outcome, exit 0. |

Every option takes its value as the next argument and may appear once; numbers are decimal digits only.
An unknown option, a missing or out-of-range value, a second scenario or no scenario is a usage error: one line and the usage on stderr, exit 14 (harness-fail), no report.
An unreadable scenario file (missing, not a regular file, I/O error) and a scenario the parser refuses (`scenario-syntax line=<n> reason=<token>`) are also exit 14 with one line on stderr and no report.

## Scenario format: IVSS v1

A scenario is ASCII text with LF line ends, one directive per line, tokens split on spaces and tabs, no quoting.
A line whose first byte is `#` is a comment; blank lines are skipped.
The first directive is `ivss 1`; `id` and `profile` are required.
Header directives (`id`, `profile`, `content`, `expect-load`, `expect-sha256`, `wall-timeout-ms`) come before the first `run-to`.
Every directive of an accepted scenario can take effect (review WR-02, WR-03). Roles load in the order exec, grom, cart, and an `expect-load` naming a refusal ends the scenario at that load when it matches, so after it the parser refuses (reason `dead-after-refusal`, at the first such line) a content line of a later role, any `run-to` (and with it any `assert`) and any `input` line. A scenario without an expected refusal needs at least one `run-to` (reason `no-run-to`, line 0).

| Directive | Form | Meaning |
|---|---|---|
| `ivss` | `ivss 1` | Format version; must be first. |
| `id` | `id <[a-z0-9-]{1,48}>` | The scenario id (printed as `scenario.id`). |
| `profile` | `profile <[a-z0-9-]{1,64}>` | The hardware profile id the instance is created with. |
| `content` | `content <exec\|grom\|cart> <relpath>\|user\|boot-stub\|zero-grom` | The image of a role: a relative path under the content directory; `user` (the operator's `--exec`, `--grom` or `--cart`); `boot-stub` (exec only: the project boot stub, the reserved file `boot-stub.exec` under the content directory); `zero-grom` (grom only: 2048 zero bytes made by the runner). |
| `expect-load` | `expect-load <role> ok\|<IVS_E_NAME> [reason <n>]` | The expected load status of a role (default `ok`); a matching refusal ends the scenario as pass, so nothing may follow it (above). |
| `expect-sha256` | `expect-sha256 <role> <64 lower-case hex>` | The role's image must have this SHA-256 (checked by the core at load). |
| `wall-timeout-ms` | `wall-timeout-ms <n>` | Lowers the wall-time cap (1 to 4294967295); a value above the operator's cap is ignored. |
| `run-to` | `run-to <n> ticks\|cpu-cycles\|frames [stop target\|halted\|faulted]` | One step: run to an absolute machine time and expect that stop (default `target`). |
| `assert` | `assert halted ==\|!= yes\|no` | A checkpoint of the step before it. |
| | `assert reg R<0-7> ==\|!= <HHHH>` | A CPU register. |
| | `assert mem <AAAA> ==\|!= <HHHH>` | One memory word. |
| | `assert mem <AAAA>..<BBBB> ==\|!= <HHHH>` | Every word of an inclusive range. |
| | `assert digest <domain> ==\|!= <64 hex>` | A state digest: `composite`, `cpu`, `mem`, `images`, `instance`, `video`, `audio`, `input` or `psglog`. |
| | `assert psglog count ==\|!= <n>` | The number of PSG write-log records delivered so far (needs no capture). |
| | `assert psglog <i> [tick=<n>] [addr=<0..15>] [value=<HHHH>] [source=<cpu-write\|cpu-dw>]` | Record `i` (0-based) of the capture: only the named fields are compared, each at most once and at least one, in any order. `value` is the raw 16-bit bus value before the register's write mask; `addr` is the low nibble of the bus address (`$01F8` is 8). Needs `capture psglog`. |
| | `assert audio emitted ==\|!= <n>` | The number of audio samples delivered so far. |
| | `assert audio at <tick> A=<n> B=<n> C=<n>` | The channel levels of the sample stamped `<tick>`: the last level change at or before that sample, else 0, 0, 0. The tick must be a multiple of the real sample period (`audio.sample_ticks`), checked at run time (a harness failure otherwise, detail `audio-tick-misaligned`); a tick beyond the emitted samples fails the assert. Needs `capture audio`. |
| `capture` | `capture psglog\|audio` | Keeps the first 4096 delivered write-log records, or audio level-change entries, for the asserts above; once per target, anywhere after `ivss 1`. The 4097th entry is a harness failure, detail `capture-overflow target=<t>`, never a silent drop. A scenario names no output file and no rate. |
| `capture` | `capture video <first> <count>` | Keeps the delivered frame slots with index `first` to `first + count - 1` (count 1 to 16, so at most 16 x 15,296 = 244,736 bytes; the library allocates nothing) for the frame asserts below; once, anywhere after `ivss 1`. A capture run without a delivered video buffer is a harness failure, detail `stream-without-buffer target=video`. |
| `assert` | `assert frame <n> pixel <x> <y> ==\|!= <0..15\|blank>` | A position of the published plane of frame `n` (x 0 to 158, y 0 to 95 on the NTSC profile; a position outside the plane is a harness failure, `frame-pixel-range`). Needs `capture video` covering `n` (a missing capture is refused at the end of the parse, `assert-without-capture`; a frame outside the window or never delivered is a harness failure, `frame-not-captured`). |
| | `assert frame <n> hash ==\|!= <64 hex>` | The SHA-256 of the canonical text form of the frame the runner received: the published plane as `height` lines of `width` upper-case hex digits (0 to 9, A to F), each ending in LF, computed in the runner and equal to the `sha256-text` key of the hand-derived frame files. A frame with a blank position has no digit for it and cannot be hash-asserted (harness failure `hash-needs-colors-only`; assert the position with the `pixel` form). |
| | `assert frame <n> displayed ==\|!= yes\|no` | The displayed flag of the slot header (the display was enabled for the frame). |
| | `assert video count ==\|!= <n>` | The number of frame slots delivered so far (needs a video buffer, never a vacuous pass). |
| | `assert video chain ==\|!= <64 hex>` | The chain value the runner recomputed after the last delivered frame (h0 is the SHA-256 of the 16 ASCII bytes `IVS-VIDEO-LOG-V1`, then h(n+1) is the SHA-256 of h(n) followed by the slot bytes); `assert digest video` stays the core's own domain-16 digest. |
| `input` | `input at_tick=<n>\|at_cycle=<n> c0=<names\|0xHEX> c1=<names\|0xHEX>` | Controller words from that machine time on; names are the `IVS_CTRL_*` bit names without the prefix, joined by `+`. |

Steps share one unit and their targets strictly increase; input lines share one unit and are nondecreasing.
The `c0` and `c1` words are indexed by PSG port, not by player: `c0` drives the port a guest reads at `$01FE` and `c1` the port at `$01FF` (plan 05-05); an event is visible to every guest read of the unit that begins at or after its tick, and a read inside the unit that began earlier sees the old state (C67).
`cpu-cycles` and `frames` are host conversions to master ticks through the instance's timebase (`ivs_info`); frames are nominal, not frame events (D-09).
Every run-to stops at a CPU halt (`IVS_RUN_F_STOP_ON_HALT`), so a halt is a stop the scenario must expect.
Caps (D-22): 64 KiB, 512 lines, 256 bytes per line, 64 steps, 64 asserts, 64 input lines, 4096 capture entries per target, 16 captured video frames.
A record or `at` assert without its capture directive anywhere in the file is refused at the end of the parse (reason `assert-without-capture`, at that assert's line). The reasons added in plan 05-09, appended after `no-run-to`: `bad-capture`, `duplicate-capture`, `bad-psglog-assert`, `bad-audio-assert`, `assert-without-capture`. A scenario with a capture or a `psglog` or `audio` assert run without output buffers is a harness failure (`stream-without-buffer target=<t>`), never a vacuous pass. The reasons added in plan 06-12, appended after `assert-without-capture`: `bad-capture-video` (arguments out of range or a count above 16), `bad-frame-assert` and `bad-video-assert`; the assert subjects `frame` and `video` are appended after `audio`, and `at_frame=` stays refused.

The first tracked scenario is `data/scenarios/diag-boot-pass.ivss`: the project boot stub, the zero GROM and the original diagnostic cartridge `diag-cpu-1.ivsc`, `run-to 4000 cpu-cycles stop halted`, and the 18 assert lines of `data/golden/diag-cpu-1.expect`.
Its images are build outputs (`build/<preset>/images`), never committed:

```
build/dev/runner/ivs-run --content-dir build/dev/images data/scenarios/diag-boot-pass.ivss
```

## Report

The report is flat `key value` lines, ASCII and LF, every key of the closed table below in this fixed order, all keys always present (`-` when a value does not apply) (D-19).
The first line is `ivsr 3` and the last is `report.end <n>`, n being the number of lines before it, so a truncated report cannot read as complete.
Never printed: wall time, host paths, the environment, image bytes.
`host.calls` is the one key that depends on the host (it counts host service calls, so it changes with the slice size); every other key is a function of the scenario, the images and the core.

| Key | Value |
|---|---|
| `ivsr` | `3` (report format 3, plan 06-12: one bump for the video keys; format 2, plan 05-09, added the audio and write-log keys) |
| `scenario.id`, `scenario.profile` | scenario tokens |
| `api.version` | `ivs_api_version()`, for example `0.1.0` |
| `outcome` | `pass`, `guest-fail`, `timeout`, `cancelled`, `missing-fixture`, `harness-fail` |
| `outcome.detail` | `-` or one detail (below) |
| `exit.code` | 0, 10, 11, 12, 13 or 14 |
| `slot.<exec\|grom\|cart>.loaded`, `.length_bytes`, `.sha256`, `.status`, `.reason` | 0 or 1; bytes; 64 hex or `-`; `ok`, an `IVS_E_*` name or `-`; the load reason number or `-` |
| `exec:` | `project boot stub, not an EXEC sha256=<hex>` (only for `content exec boot-stub`), else `-` |
| `grom:` | `zero image, not a GROM` (only for `content grom zero-grom`), else `-` |
| `step.<k>.target_ticks` | ticks, then `from=<n> cpu-cycles` or `from=<n> frames nominal, not frame events` for the converted units |
| `step.<k>.now_ticks`, `.overshoot_ticks` | machine time at the step's end; `now - target` when positive, else 0 |
| `step.<k>.stop_reason`, `.expected_stop` | `target`, `halted`, `faulted` or `internal`; `target`, `halted` or `faulted` |
| `step.<k>.assert.<j>` | `pass`, `fail` or `not-run`, then `<subject> <op> <expected> got <actual>` |
| `cpu.run_state`, `cpu.fault_reason` | `unreset`, `reset-pending`, `running`, `halted` or `faulted`; `none` or a fault token |
| `input.events`, `input.consumed`, `input.controller.<0..3>` | counts; 8 upper-case hex digits, or `-` before any run |
| `digest.cpu`, `.mem`, `.images`, `.instance`, `.composite`, `.video`, `.audio`, `.input`, `.psglog` | 64 lower-case hex |
| `output.video.bytes`, `output.audio.bytes` | byte counts delivered through the output buffers (video is the sum of the delivered frame slots, 0 for a run that ends before its first frame end at tick 59,736; audio is the dense sample stream, 4 bytes per sample) |
| `output.audio.samples`, `output.audio.changes` | delivered audio bytes over 4; the level-change records the runner itself counted from the delivered bytes |
| `output.psglog.records`, `output.psglog.bytes` | PSG write-log records and bytes delivered (16 bytes per record) |
| `audio.sample_ticks` | master ticks between samples (`ivs_info` `audio_sample_ticks`, 16 on the NTSC profile) |
| `audio.chain.check` | `pass` when the runner's recomputation of the AUDIO level-change chain from the delivered bytes gives the core's domain-17 digest, `fail` when it does not (the outcome is then `harness-fail`, detail `audio-chain`), `-` when an audio buffer was not delivered on every slice |
| `output.video.frames` | frames the core completed, summed over the run calls (`video_frames_completed`, counted whether or not a video buffer was given) |
| `video.geometry`, `video.row_lines` | `<width>x<height>` and the TV lines one row covers, from `ivs_info` (159x96 and 2 on the NTSC profile), `-` before any machine |
| `video.frame.last_tick` | the last frame end the core reported, `-` before any frame |
| `video.chain.check` | `pass` when the runner's recomputation of the VIDEO frame chain from the delivered slots gives the core's domain-16 digest and every slot header is consistent, `fail` when not (the outcome is then `harness-fail`, detail `video-chain`), `-` when a video buffer was not delivered on every slice |
| `host.calls` | host service calls (read_file, read_user, now_ms, cancel_requested) |
| `refpcm.rate`, `refpcm.frames`, `refpcm.sha256` | present ONLY when `--audio-wav` was given, immediately before `report.end`: the WAV rate, the PCM frames written, and the SHA-256 of the WAV file bytes as written (`-` for the frames and the hash when no finished file remains, that is after a harness failure), so `sha256sum` of the file equals it; never in a golden (plan 05-10) |
| `report.end` | the number of lines before it |

Steps and asserts are numbered from 1 in scenario order; an assert keeps its scenario-wide number.
Execution ends after the first step with a stop mismatch or a failed assert; that step's asserts are all evaluated, later steps print `-` values and `not-run` asserts.

Details (`outcome.detail`), the first one recorded for the winning outcome:
`assert=<j> line=<n> <clause> got <actual>`;
`stop step=<k> line=<n> expected=<t> got=<t>`;
`budget step=<k> line=<n> expected=<t> got=target` (the bound reached where a halt or fault was expected);
`budget step=<k> line=<n> max-ticks=<m>` (the `--max-ticks` ceiling stopped machine time below the step's bound);
`wall step=<k> cap-ms=<n>`;
`signal step=<k>`;
`missing role=<r> path=<rel>` or `... user`, `not-regular role=<r> ...`, `io-error role=<r> ...`;
`load-refused role=<r> status=<IVS_E_*> reason=<n>`, `load-refused-as-expected ...`, `load-not-refused role=<r> expected=<IVS_E_*>`;
`audio-chain` (the runner's recomputation of the AUDIO chain disagreed with the core's digest), `video-chain` (the same for the VIDEO frame chain, or a slot header finding), `frame-not-captured assert=<j> line=<n>`, `hash-needs-colors-only assert=<j> line=<n>`, `frame-pixel-range assert=<j> line=<n>`, `video-slot-too-large`, `video-out`, `video-out-max-bytes`, `frame-ppm-out`, `frame-ppm-out-max-bytes`, `frame-ppm-convert`, `frame-ppm-user-content`, `buffers-too-small`, `capture-overflow target=<psglog|audio>`, `audio-tick-misaligned assert=<j> line=<n>`, `stream-without-buffer target=<psglog|audio|video>`, `audio-out`, `audio-out-max-bytes`, `psg-log-out`, `psg-log-out-max-bytes`, `wav-out`, `wav-out-max-bytes`, `wav-without-audio`, `wav-convert`, `wav-init`,
`profile-refused status=...`, `create-refused status=...`, `storage-too-small`, `reset-refused status=...`, `run-refused step=<k> status=... reason=<n>`, `bound-too-large step=<k> line=<n>`, `input-out-of-range line=<n>`, `no-timebase`, `observe-refused ...`, `stop-internal step=<k> line=<n> got=<t>`.

## Outcomes and exit codes

| Outcome | Exit | When |
|---|---|---|
| `pass` | 0 | Every step stopped as expected and every assert held, or a load refusal matched `expect-load`. |
| `guest-fail` | 10 | An assert failed, or the guest halted or faulted where the scenario did not expect it. |
| `timeout` | 11 | Detail `budget`: deterministic machine-time budget. Detail `wall`: the host wall-time cap. |
| `cancelled` | 12 | SIGINT or SIGTERM arrived (detail `signal`). |
| `missing-fixture` | 13 | A content file or an operator image is missing or not a regular file. |
| `harness-fail` | 14 | Anything else that makes the run meaningless: a refused profile, create, reset, run or observation, an unexpected load refusal, an I/O error, a usage error, a failed report write. |

The codes avoid 1 (the AddressSanitizer default), 77 (the ctest skip code) and 126 and above (the shell's).
When a run observes several outcomes, the highest wins: harness-fail, missing-fixture, cancelled, timeout (wall), timeout (budget), guest-fail, pass.
`ivs-run --help` prints the six `<outcome> <exit>` pairs.

## Slices, budget, wall time and cancellation (D-21)

Every run-to is executed as slices: `ivs_machine_run(target = min(limit, now + slice))`, with the unconsumed input events resubmitted each call, until the CPU stops or machine time reaches the limit (the run-to bound, or the `--max-ticks` ceiling when that is lower).
The core keeps no state between calls, so any slicing gives the same machine as one call (API-02); the in-process suite checks that sliced reports equal the single-call report except `host.calls`.
Before every slice the runner checks the cancel flag once, then the host monotonic timer once.

- **Machine-time budget** (deterministic, may appear in goldens): the run-to bound reached where `stop halted` or `stop faulted` was expected, or the `--max-ticks` ceiling cutting a run-to short. Outcome timeout, detail `budget ...`. The ceiling cuts a run-to short only when machine time stopped below the run-to bound: the last slice ends at the first unit boundary at or after the ceiling, and when that unit carries machine time to or past the bound the run-to completed, the machine equals the uncapped one and the step is judged as usual (its stop expectation and asserts), never as a budget timeout (review WR-01).
- **Wall-time cap** (host-dependent, never in a golden): host time since the first step reached the cap (the operator's `--wall-timeout-ms`, default 30 s, lowered by a scenario's `wall-timeout-ms`). Outcome timeout, detail `wall step=<k> cap-ms=<n>`. It protects the host against an absurd bound in an untrusted scenario.
- **Cancellation**: a handler installed with `signal()` for SIGINT and SIGTERM sets a `volatile sig_atomic_t` flag and does nothing else; the runner reads the flag between slices. Outcome cancelled, detail `signal step=<k>`. `runner/hostio.c` defines `_DEFAULT_SOURCE` so that glibc gives `signal()` BSD semantics under strict C17, as Apple libc does: the handler stays installed, so a second signal is just another cancel request, and an interrupted system call (a blocked stdout write) restarts instead of failing with EINTR.

Latency of a wall cap or a signal is at most one slice. The report of an interrupted run shows the deterministic state at the last slice boundary, with the interrupted step's asserts `not-run`.

Test hooks (documented, deterministic, no sleeps and no racing signals):

- `--self-signal-after-slices N` calls `raise(SIGINT)` inside the cancel check that follows slice N, so the real handler path is exercised in-process; the run ends cancelled (exit 12) whenever it has more than N slices. With `--self-signal-count 2` it raises SIGINT twice there: the scenario `cancel-signal-twice` must still end cancelled with a report, which proves the handler survives a first signal (review WR-04).
- `--wall-timeout-ms 1` with a bound far beyond the guest's work can only end by the wall cap; a slower host just completes fewer slices.
- The in-process suite (`runner.time.*`) uses a fake host: a fake monotonic timer that advances a fixed step per call and a fake cancel flag raised from a chosen check on.

## The operator WAV (`--audio-wav`, plan 05-10, D-21, D-22)

`--audio-wav <file>` writes the dense audio stream as a mono 16-bit WAV through the reference PCM converter of `tools/pcm` (`ivs_refpcm.h`, `ivs_refpcm.c`), label **reference conversion v1, converter repeatability only**.
The converter is a simple aliasing reference, not chip truth and not a high-fidelity renderer, and it sits outside the raw-audio evidence path: no digest, golden of the raw streams, claim or `important-gates` row for AUD-02 depends on it.
Where it lives (D7, decided): no PCM, resampling, filtering, DAC-table use or WAV code is in `ivs_core` or in any installed artifact. The converter is an internal static target linked only by the runner library (for this option) and by suite `runner`; it has no install rule. The gate proves the placement with the exact install inventory (`data/expected/pkg-install-files.ids`) and a defined-symbol rule of `tools/check/core-symbols.sh` (`core-symbols-pcm-defined`: the core archive may define no symbol containing `refpcm`, `resample`, `wav` or `pcm`). Reversal trigger: an adapter that must produce bit-identical PCM across hosts promotes it to an installed `ivs::refpcm` target, still outside the core.

Algorithm (pure integer, no I/O, no allocation, no real-number types, no globals):

- Input: the dense samples `A, B, C, 0` (the post-mixer amplitude indices of ivs_info `audio_sample_bytes` 4). The 0-based sample i is stamped `(i + 1) * P` master ticks (P is `audio_sample_ticks`, 16) and is held from its stamp; the first P ticks before the first stamp hold level 0, one implicit silent cell.
- Mix: `x = dac[A] + dac[B] + dac[C]` from a 16-entry table that is a `static const` copy of the profile field `psg.dac.levels` (units of 1e-8 of full scale: level 15 is 100000000, level 0 is 0; the test `runner.refpcm.dac-table-matches-profile` keeps the copy honest). The core never reads that field.
- Filter: an exact rational box filter. With the master crystal `num / den` Hz (`ivs_info` `crystal_hz_num`, `crystal_hz_den`) and the rate R, output frame j covers master ticks `[j * num / (den * R), (j + 1) * num / (den * R))`. Time is counted in units of `1 / (den * R)` tick, so every window is exactly `num` units, every cell is `P * den * R` units and every overlap is an integer: `out_j = round_half_up(sum(overlap * x) / (num * 10000))` in `uint64_t` (three channels at level 15 map to 30000, so nothing clips).
- DC block: `y_j = out_j - out_{j-1} + floor(y_{j-1} * 32763 / 32768)`, with a floor division helper (no right shift of a negative value). The state is a caller-owned struct, so cutting the conversion at any sample count never changes the output (the runner carries it across slices). One consequence of the floor: a downward step leaves a residual of -1 that never lifts.
- Output: signed 16-bit little-endian mono, a 44-byte RIFF/WAVE header (PCM, one channel, rate, 16 bits).

The WAV is created exclusively (an existing path is refused) and written only through `runner/hostio.c`. Decision: the header is written after the stream is complete. The file is created with 44 reserved bytes, the PCM follows slice by slice, and the finished file is read back from offset 0 to hash it, so `refpcm.sha256` is the SHA-256 of the file as it lies on disk. A failed write, a bound hit or a missing audio buffer is a harness failure: the file is removed by `ivs-run` (never left with its reserved zero header) and `refpcm.frames` and `refpcm.sha256` are both `-`, so the report states no WAV that does not exist (review WR-01).

`tools/listen-demo.sh [scenario-id]` is an operator helper, not a gate step: it builds the release preset, renders one scenario (default `diag-ctrl-psg-pass`) into `build/listen/<id>.wav` (gitignored; a `.wav` is never committed: `wav` is a binary extension of the files step and `*.wav` is in `.gitignore`) and prints the WAV size and `refpcm.sha256`. It refuses, before building anything (exit 2), any scenario whose text has a `content <role> user` line, because derived audio of rights-bearing content stays out of shared records.
The helper proves the export path only (an exclusive file, its size, its hash). The WAV of the project diagnostic is about 5 ms of near-silence: the diagnostic writes its tone period late, a period write takes effect only after the 4096-step first reload (about 65536 ticks), and the program halts near tick 16750 (plan 05-11 measured 16752; the WAV holds 224 frames at 48000 Hz). There is no listenable-tone statement for any Phase 5 artifact: the D-22 listening handoff is recorded as not performable until a later phase has a longer guest, and a rendered file or its hash is never a claim.

## Process-level tests (suite `runner-process`)

`tests/runner-process-tap.sh` (D-23, plan 04-20) drives the real `ivs-run` of each preset, so exit codes and report text are checked at the boundary a user sees.

- Scenarios: every line `<id> <exit> <outcome> <class> <skip> <operator-args>` of `data/expected/runner-scenarios.ids` names `data/scenarios/<id>.ivss`; there is one demonstrating scenario per outcome of the table above, plus an expected load refusal and a two-event input schedule (`diag-input-pass`). The judge requires the exit code, `ivsr 3` first, `report.end <n>` last, one `outcome` and one `exit.code` line forming a pair of `data/expected/runner-outcomes.ids`, and the `outcome.detail` pinned by the scenario's `# expect-detail:` comment (a parse refusal pins its one stderr line with `# expect-stderr:` and prints no report).
- `host-dependent` scenarios (`timeout-wall`, `cancel-signal`) use the two test hooks above; their reports are never compared between presets or kept as goldens. `needs-user-exec` asks for an operator EXEC the gate never has: it must end missing-fixture and is then a visible skip.
- Mutants (D-32): the diagnostic rebuilt with one changed word (`data/expected/diag-mutations.ids`: ADDI to SUBI, BNZE to BZE, PSHR to MVO@, INCR to DECR) must end guest-fail, with the detail naming the first assert the change breaks.
- Determinism: a second run, `--slice-ticks` 1 and 7, and a run from another working directory give the same report apart from `host.calls`.
- Judge self-controls: four fake runners (wrong exit code, no `report.end`, `outcome pass` with `exit.code 10`, a changed digest) must each be rejected for that reason; one more case requires `--help`, the outcome table above and `data/expected/runner-outcomes.ids` to list the same six pairs.
- Operator files: `--audio-raw`, `--psg-log-out` and `--audio-wav` are each written once per preset (`runner-process.operator.*`, `runner-process.refpcm.wav-written`: the file is 44 + 2 x `refpcm.frames` bytes, begins with `RIFF`, hashes to `refpcm.sha256`, and a second run with the same path is refused); `runner-process.operator.failed-run` (review WR-01) hits the byte bound on each of the three files and on all three together and requires exit 14, no file left, `-` for `refpcm.frames` and `refpcm.sha256`, a retry with the same path that succeeds, and a budget timeout (exit 11) that keeps a complete file; `runner-process.operator.golden-distinct` (review WR-02) requires the refusal for `--update-golden` naming each output file by the same string and by `./` spelling, no file left, and a distinct golden path that works; `runner-process.listen-demo.refuses-user` calls the listening helper on `needs-user-exec` and requires exit 2.
- Second diagnostic (plan 05-11, D-24): `diag-ctrl-psg-pass` (seven controller events, exit 0), `diag-ctrl-psg-nokey` (the same cartridge with no input: timeout, detail `budget`, exit 11) and `diag-ctrl-psg-order` (every event split into two `input` lines one tick apart, port 1 first: pass); five mutants of its program in `data/expected/diag-mutations-ctrl-psg.ids` (explicit `from` word, outcome `guest-fail`, `timeout` or `pass`, and exit code per row); the repeat, slicing and directory variants run for `diag-ctrl-psg-pass` only (cost cap V-16).
- Reference PCM golden cases (`runner-process.refpcm.<id>`, plan 05-11): every line `<id> <rate>` of `data/expected/refpcm-scenarios.ids` is run with `--audio-wav` and `--wav-rate` into a temporary directory, which leaves its report in `build/gate/<preset>/runner/<id>.refpcm.report`.
- Each scenario's report is left in `build/gate/<preset>/runner/<id>.report`.

## Goldens (`data/golden/`, D-11, plan 04-21)

Two kinds, each with a header the gate step `golden` checks (`tools/check/steps/golden.sh`):

- `data/golden/<id>.expect`: predicates, tier `original-diagnostic`, oracle `independent-derivation`, with the derivation cites in comments. Its `assert` lines must equal the scenario's `assert` lines.
- `data/golden/<id>.digests`: repeatability digests, tier `deterministic-repeatability`, oracle `self-recorded`. A self-recorded golden only shows that a later run repeats this one; it is no evidence that the run is right.

Header lines: `# tier:` (an id of `data/expected/evidence-tiers.ids`), `# oracle:`, `# scenario:` (the scenario id), `# profile:`, and for `.digests` `# domains: cpu=observed mem=observed images=observed instance=observed video=<v> audio=<v> input=<v> psglog=<v>`.
A live domain (video, audio, input, psglog) is `observed` by one count-based rule: exactly when its report count (`output.video.bytes`, `output.audio.bytes`, `input.events`, `output.psglog.bytes`) is a decimal other than 0; otherwise it is `none` and its keys are left out, so a hash of nothing is never recorded as an observation.
A delivered but silent audio stream has nonzero bytes and is a real observation of silence; a run in which the guest wrote no PSG register has an empty log and `psglog=none`.
The installed consumer decides `observed` by the same rule (delivered bytes nonzero), and the gate step `pkg-install` compares its `digest.<domain>` lines and observed set with `data/golden/diag-input-pass.digests` (plan 05-09) and, for the replay of `diag-ctrl-psg-pass` (lines prefixed `ctrl.`, plan 05-11), with `data/golden/diag-ctrl-psg-pass.digests`, audio and write log included [claim:CL-0015].

The body of a `.digests` golden is the run's report filtered to the golden key list, in report order: `scenario.id`, `scenario.profile`, `outcome`, `outcome.detail`, `exit.code`, every `slot.*`, `step.*`, `cpu.*` and `input.*` line, `digest.cpu`, `digest.mem`, `digest.images`, `digest.instance`, `digest.composite`, `output.video.bytes`, `output.audio.bytes`, and for each observed live domain its keys: video `digest.video`, `output.video.frames`, `video.geometry`, `video.row_lines`, `video.frame.last_tick`, `video.chain.check` (kept only while video is observed, which is the count rule: `output.video.bytes` is a decimal other than 0, so a golden whose scenario delivers no frame is unchanged); audio `digest.audio`, `output.audio.samples`, `output.audio.changes`, `audio.sample_ticks`; input `digest.input`; psglog `digest.psglog`, `output.psglog.records`, `output.psglog.bytes`.
Left out: `ivsr`, `api.version`, the `exec:` and `grom:` labels, `audio.chain.check`, every `refpcm.` key, `host.calls` and `report.end`; a golden that holds a `refpcm.` key or an `output.`, `audio.` or `video.` key outside this list is refused by the step.
The same list is kept in `runner/main.c` (`main_golden_keep`), `tools/check/steps/golden.sh` and here.

Goldens are written only by `ivs-run --update-golden <file>`, run by a person; a changed golden is a reviewed diff. The gate never writes one: the step only compares. The tracked goldens were written with:

```
build/dev/runner/ivs-run --content-dir build/dev/images --update-golden data/golden/diag-cpu-1.digests data/scenarios/diag-boot-pass.ivss
build/dev/runner/ivs-run --content-dir build/dev/images --update-golden data/golden/diag-input-pass.digests data/scenarios/diag-input-pass.ivss
build/dev/runner/ivs-run --content-dir build/dev/images --update-golden data/golden/timeout-budget.digests data/scenarios/timeout-budget.ivss
build/dev/runner/ivs-run --content-dir build/dev/images --update-golden data/golden/diag-stic-fgbg-pass.digests data/scenarios/diag-stic-fgbg-pass.ivss
build/dev/runner/ivs-run --content-dir build/dev/images --update-golden data/golden/diag-stic-cstack-pass.digests data/scenarios/diag-stic-cstack-pass.ivss
```

The composite digest is version 3 since plan 06-05 (D-16): the STIC register file joined the `mem` domain and the STIC device state the `instance` domain, and the framing is unchanged. That plan re-blessed every `.digests` golden and `data/expected/pkg-consumer.golden` once with the recipe above and the consumer refresh of the `pkg-install` step header, and the diff moved only `digest.mem`, `digest.instance` and `digest.composite` (also their `ctrl.`-prefixed twins in the consumer golden); `digest.cpu` and `digest.images` stay equal to the version 2 values pinned in `data/expected/digest-v2-literals.ids` (the version 1 values are in `data/expected/digest-v1-literals.ids`). The tracked goldens are now written by the release runner (`build/release/runner/ivs-run --content-dir build/release/images`; the dev and release reports are byte-equal, which the step checks).

A `.refpcm` golden (plan 05-11, D-21) is the repeatability record of the reference PCM converter ("reference conversion v1, converter repeatability only") for one scenario: header `# tier: deterministic-repeatability`, `# oracle: self-recorded`, that `# label:` line and `# scenario: <id>`, body `refpcm.rate`, `refpcm.frames`, `refpcm.sha256` only. It is written only from a runner report, never by hand, with:

```
d=$(mktemp -d)
build/dev/runner/ivs-run --content-dir build/dev/images --audio-wav "$d/x.wav" --wav-rate 48000 data/scenarios/diag-ctrl-psg-pass.ivss > "$d/report"
{ printf '# tier: deterministic-repeatability\n# oracle: self-recorded\n# label: reference conversion v1, converter repeatability only\n# scenario: diag-ctrl-psg-pass\n'
  awk '$1 ~ /^refpcm\./' "$d/report"; } > data/golden/diag-ctrl-psg-pass.refpcm
```

The step compares its three body lines with the `refpcm.*` lines of the dev report and of the release report `<id>.refpcm.report` that suite `runner-process` leaves, requires the scenario to be listed in `data/expected/refpcm-scenarios.ids` with the same rate, and calls a missing report or a differing value `golden-mismatch` (control `golden-refpcm-mismatch`). It never reads the digests golden and nothing in a digest, claim or `important-gates` row depends on it: the converter sits outside the raw-audio evidence path. The helper `tools/listen-demo.sh` on `diag-ctrl-psg-pass` prints the same WAV size and `refpcm.sha256` (492 bytes, 224 frames); the file is near-silence, so this demonstrates the export path only and makes no listenable-tone statement.

The step `golden` also requires the dev and release reports of every `deterministic` scenario of `data/expected/runner-scenarios.ids` to be byte-equal apart from `host.calls` (one ctest run sees only one preset, so this is judged after both ran).
`host-dependent` scenarios (`timeout-wall`, `cancel-signal`) are left out of that comparison and may have no golden; a scenario with `user` content has no golden either.

## Trust model (D-22)

- A scenario is untrusted inert data. The parser is a pure function with fixed caps; it allocates nothing.
- Content paths are relative, `[A-Za-z0-9._/-]`, without `..`, a leading `/` or an empty segment, at most 200 bytes, and are resolved only under the content directory. A scenario cannot name an output file, run a command, read the environment or raise a limit; it can only lower the wall-time cap.
- Every file (scenario and content) is opened without blocking and checked with `fstat` on the open descriptor: only a regular file is read (a FIFO or a device is missing-fixture, `not-regular`). Reads stop at the role's cap plus one byte (exec 8192, grom 2048, cartridge 131256, scenario 65536 bytes), so an oversized file costs at most cap + 1 bytes and reaches the core's own size refusal. The content directory is an operator trust boundary (review IN-05): the path rules confine a scenario by syntax only, and `open()` follows symbolic links, so a symlink inside `--content-dir` (or inside the scenario's directory when that is the default) lets a scenario read the regular file it points to, wherever it lies, still bounded by the cap and the regular-file check. Point `--content-dir` only at a directory whose links you trust.
- System images (EXEC, GROM) come only from the operator's `--exec` and `--grom` flags (and `--cart`), never from tracked data. The runner never writes, copies or caches image bytes; the files it writes are the operator's `--update-golden` target, which holds hashes and report lines only, and the operator's `--audio-raw`, `--psg-log-out` and `--audio-wav` files (new files, created exclusively, bounded by `--audio-out-max-bytes`, written only in `runner/hostio.c`; they hold the audio sample bytes, the PSG write log and the reference PCM the machine emitted, never image bytes). That target is overwritten without a prompt or a backup (review IN-05); the path comes only from the operator's command line, never from a scenario. The project boot stub and the zero GROM are not system images; their report labels say so.
- Storage: the instance and one read buffer in host memory, at most 64 MiB; output buffers at most 8 MiB.
