# The headless runner (`ivs-run`)

`ivs-run` executes one IVSS v1 scenario on one freshly created machine instance and prints a flat, deterministic report on stdout (RUN-01, RUN-02).
It is a host of the machine core like any other: it links only the public `ivs_core` library and uses the public API (`ivs/ivs.h`), so it can see nothing the installed package does not offer.
The machine core keeps no host time; timeouts and cancellation exist only in the runner (D-21).

Layout (D-17): the library `ivs_runner_lib` (`runner/scenario.c` parser, `runner/exec.c` executor, `runner/report.c` report writer, `runner/outcome.c` outcome mapping) and the thin executable built from `runner/main.c` (options, exit code) and `runner/hostio.c` (the only file that uses POSIX: files, the monotonic timer, signals).
One process handles one scenario on one instance; several scenarios mean several invocations.
The program keeps no global state except the cancel flag.

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
| `--update-golden <file>` | a file | After the run, also write the run's repeatability golden (`.digests`, below) to the file, overwriting it if it exists. Refused (exit 14, one line on stderr) for a scenario with `user` content (before the run, no report) and for a host-dependent outcome (cancelled, or timeout with detail `wall`). |
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
| | `assert digest <domain> ==\|!= <64 hex>` | A state digest: `composite`, `cpu`, `mem`, `images`, `instance`, `video`, `audio` or `input`. |
| `input` | `input at_tick=<n>\|at_cycle=<n> c0=<names\|0xHEX> c1=<names\|0xHEX>` | Controller words from that machine time on; names are the `IVS_CTRL_*` bit names without the prefix, joined by `+`. |

Steps share one unit and their targets strictly increase; input lines share one unit and are nondecreasing.
`cpu-cycles` and `frames` are host conversions to master ticks through the instance's timebase (`ivs_info`); frames are nominal, not frame events (D-09).
Every run-to stops at a CPU halt (`IVS_RUN_F_STOP_ON_HALT`), so a halt is a stop the scenario must expect.
Caps (D-22): 64 KiB, 512 lines, 256 bytes per line, 64 steps, 64 asserts, 64 input lines.

The first tracked scenario is `data/scenarios/diag-boot-pass.ivss`: the project boot stub, the zero GROM and the original diagnostic cartridge `diag-cpu-1.ivsc`, `run-to 4000 cpu-cycles stop halted`, and the 18 assert lines of `data/golden/diag-cpu-1.expect`.
Its images are build outputs (`build/<preset>/images`), never committed:

```
build/dev/runner/ivs-run --content-dir build/dev/images data/scenarios/diag-boot-pass.ivss
```

## Report

The report is flat `key value` lines, ASCII and LF, every key of the closed table below in this fixed order, all keys always present (`-` when a value does not apply) (D-19).
The first line is `ivsr 1` and the last is `report.end <n>`, n being the number of lines before it, so a truncated report cannot read as complete.
Never printed: wall time, host paths, the environment, image bytes.
`host.calls` is the one key that depends on the host (it counts host service calls, so it changes with the slice size); every other key is a function of the scenario, the images and the core.

| Key | Value |
|---|---|
| `ivsr` | `1` |
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
| `digest.cpu`, `.mem`, `.images`, `.instance`, `.composite`, `.video`, `.audio`, `.input` | 64 lower-case hex |
| `output.video.bytes`, `output.audio.bytes` | byte counts (0 in Phase 4) |
| `host.calls` | host service calls (read_file, read_user, now_ms, cancel_requested) |
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

## Process-level tests (suite `runner-process`)

`tests/runner-process-tap.sh` (D-23, plan 04-20) drives the real `ivs-run` of each preset, so exit codes and report text are checked at the boundary a user sees.

- Scenarios: every line `<id> <exit> <outcome> <class> <skip> <operator-args>` of `data/expected/runner-scenarios.ids` names `data/scenarios/<id>.ivss`; there is one demonstrating scenario per outcome of the table above, plus an expected load refusal and a two-event input schedule (`diag-input-pass`). The judge requires the exit code, `ivsr 1` first, `report.end <n>` last, one `outcome` and one `exit.code` line forming a pair of `data/expected/runner-outcomes.ids`, and the `outcome.detail` pinned by the scenario's `# expect-detail:` comment (a parse refusal pins its one stderr line with `# expect-stderr:` and prints no report).
- `host-dependent` scenarios (`timeout-wall`, `cancel-signal`) use the two test hooks above; their reports are never compared between presets or kept as goldens. `needs-user-exec` asks for an operator EXEC the gate never has: it must end missing-fixture and is then a visible skip.
- Mutants (D-32): the diagnostic rebuilt with one changed word (`data/expected/diag-mutations.ids`: ADDI to SUBI, BNZE to BZE, PSHR to MVO@, INCR to DECR) must end guest-fail, with the detail naming the first assert the change breaks.
- Determinism: a second run, `--slice-ticks` 1 and 7, and a run from another working directory give the same report apart from `host.calls`.
- Judge self-controls: four fake runners (wrong exit code, no `report.end`, `outcome pass` with `exit.code 10`, a changed digest) must each be rejected for that reason; one more case requires `--help`, the outcome table above and `data/expected/runner-outcomes.ids` to list the same six pairs.
- Each scenario's report is left in `build/gate/<preset>/runner/<id>.report`.

## Goldens (`data/golden/`, D-11, plan 04-21)

Two kinds, each with a header the gate step `golden` checks (`tools/check/steps/golden.sh`):

- `data/golden/<id>.expect`: predicates, tier `original-diagnostic`, oracle `independent-derivation`, with the derivation cites in comments. Its `assert` lines must equal the scenario's `assert` lines.
- `data/golden/<id>.digests`: repeatability digests, tier `deterministic-repeatability`, oracle `self-recorded`. A self-recorded golden only shows that a later run repeats this one; it is no evidence that the run is right.

Header lines: `# tier:` (an id of `data/expected/evidence-tiers.ids`), `# oracle:`, `# scenario:` (the scenario id), `# profile:`, and for `.digests` `# domains: cpu=observed mem=observed images=observed instance=observed video=<v> audio=<v> input=<v>`.
A live domain (video, audio, input) is `observed` only when its report count (`output.video.bytes`, `output.audio.bytes`, `input.events`) is not 0; otherwise it is `none` and its digest line is left out, so a hash of nothing is never recorded as an observation.

The body of a `.digests` golden is the run's report filtered to the golden key list, in report order: `scenario.id`, `scenario.profile`, `outcome`, `outcome.detail`, `exit.code`, every `slot.*`, `step.*`, `cpu.*` and `input.*` line, `digest.cpu`, `digest.mem`, `digest.images`, `digest.instance`, `digest.composite`, `output.video.bytes`, `output.audio.bytes`, and the digest of each observed live domain.
Left out: `ivsr`, `api.version`, the `exec:` and `grom:` labels, `host.calls` and `report.end`.

Goldens are written only by `ivs-run --update-golden <file>`, run by a person; a changed golden is a reviewed diff. The gate never writes one: the step only compares. The tracked goldens were written with:

```
build/dev/runner/ivs-run --content-dir build/dev/images --update-golden data/golden/diag-cpu-1.digests data/scenarios/diag-boot-pass.ivss
build/dev/runner/ivs-run --content-dir build/dev/images --update-golden data/golden/diag-input-pass.digests data/scenarios/diag-input-pass.ivss
build/dev/runner/ivs-run --content-dir build/dev/images --update-golden data/golden/timeout-budget.digests data/scenarios/timeout-budget.ivss
```

The step `golden` also requires the dev and release reports of every `deterministic` scenario of `data/expected/runner-scenarios.ids` to be byte-equal apart from `host.calls` (one ctest run sees only one preset, so this is judged after both ran).
`host-dependent` scenarios (`timeout-wall`, `cancel-signal`) are left out of that comparison and may have no golden; a scenario with `user` content has no golden either.

## Trust model (D-22)

- A scenario is untrusted inert data. The parser is a pure function with fixed caps; it allocates nothing.
- Content paths are relative, `[A-Za-z0-9._/-]`, without `..`, a leading `/` or an empty segment, at most 200 bytes, and are resolved only under the content directory. A scenario cannot name an output file, run a command, read the environment or raise a limit; it can only lower the wall-time cap.
- Every file (scenario and content) is opened without blocking and checked with `fstat` on the open descriptor: only a regular file is read (a FIFO or a device is missing-fixture, `not-regular`). Reads stop at the role's cap plus one byte (exec 8192, grom 2048, cartridge 131256, scenario 65536 bytes), so an oversized file costs at most cap + 1 bytes and reaches the core's own size refusal. The content directory is an operator trust boundary (review IN-05): the path rules confine a scenario by syntax only, and `open()` follows symbolic links, so a symlink inside `--content-dir` (or inside the scenario's directory when that is the default) lets a scenario read the regular file it points to, wherever it lies, still bounded by the cap and the regular-file check. Point `--content-dir` only at a directory whose links you trust.
- System images (EXEC, GROM) come only from the operator's `--exec` and `--grom` flags (and `--cart`), never from tracked data. The runner never writes, copies or caches image bytes; the one file it writes is the operator's `--update-golden` target, which holds hashes and report lines only. That target is overwritten without a prompt or a backup (review IN-05); the path comes only from the operator's command line, never from a scenario. The project boot stub and the zero GROM are not system images; their report labels say so.
- Storage: the instance and one read buffer in host memory, at most 64 MiB; output buffers at most 8 MiB.
