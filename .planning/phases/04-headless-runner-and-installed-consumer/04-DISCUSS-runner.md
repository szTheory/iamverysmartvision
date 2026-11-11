# Phase 4 discuss, decision point B: the headless runner (RUN-01, RUN-02, RUN-04 for the runner)

Labels: [OBS] read or ran in this session; [INF] inference; [REC] recommendation; [UNK] unknown; [BG] background knowledge, not re-fetched.
No web fetch was needed: every load-bearing fact is in the repo. No emulator source was opened. Ecosystem lessons are [BG] and reasoned from published documentation only.

## 0. Facts that shape the decision

- [OBS] The public API (`core/include/ivs/ivs_machine.h`) has create, destroy, info, state_digest (domains 0..4; 16..18 are reserved and refused), load, unload and reset. It has no run function, no register or memory inspection, no output buffer, no input call. `ivs_machine_step_unit_internal` is internal only (`machine.h`, "Phase 4's bounded run API is built on it").
- [OBS] `docs/time-base.md` (D2): the canonical unit is the master tick (`uint64_t`); the core contract is `run(target_tick)` returning `now_tick`, `overshoot_ticks`, stop reason; a target above 2^63 is refused; "a CPU-cycle convenience helper lives in the runner, not in the core API". API-02's text says "absolute cycle target". [INF] The runner scenario therefore accepts `ticks` as canonical and `cpu-cycles` and `frames` as sugar converted in the runner.
- [OBS] CPU guest faults are sticky (`run_state` FAULTED, `fault_reason`), HLT latches HALTED; neither is an API refusal (Phase 2 D-12). That is the natural source of the "guest failure" outcome.
- [OBS] `.gitignore` ignores `*.bin *.rom *.ivsc *.int ...`. Tracked fixtures are therefore text (decle lists, `data/cart/*.json`); binary containers exist only in build trees. Existing precedent: CMake `-P` generators turn text into `.inc` tables, and `string(SHA256)` is an independent oracle (`IvsShaVectors.cmake`).
- [OBS] `tools/check/steps/ci-parity.sh` forbids any workflow `run:` other than `sh tools/gate.sh` and `cmake --version`. So every new check lives inside the gate, never as a workflow step.
- [OBS] Gate machinery to reuse, not duplicate: ctest suites emit TAP and `tests-inventory` judges ids, plan, nonce, `skip-ok` for both presets (precedent `tests/headers-tap.sh`, a shell TAP suite); `c-faults` plus `faults-verdict` run and judge named faults per suite; `run-controls.sh` runs negative controls against a synthetic tree, so steps that controls drive must be pure checkers over files (the producer/judge split of `c-tests`/`tests-inventory`).
- [OBS] The fault framework (`tests/faults.cmake`) is keyed to `IVS_CORE_SOURCES` relative to `core/`, and `faults-verdict` scans `core/src/**` for `IVS_FAULT_<ID>` sites. Runner faults need this generalized to repo-relative owner paths (see D-B16).
- [OBS] Hosted gate: 30 to 49 s against a 40 s budget (trigger 50 s); two hosted-only failures in Phase 3 were gcc 13 or ELF specifics. Strict `-std=c17` (extensions off) hides POSIX names (`clock_gettime`, `stat`) on glibc unless `_POSIX_C_SOURCE` is defined.

## 1. Recommended decision (summary; the numbered list is repeated in the reply)

B1 shape, B2 scenario format, B3 report, B4 outcomes and exit codes, B5 timeout and cancellation, B6 file IO and trust, B7 isolation, B8 test strategy, B9 build and install, B10 portability, B11 security. Details and rejected options follow.

## 2. Executable, location, build and install (B1, B9)

Options.
1. `runner/` top-level directory, library `ivs_runner_lib` (parser, executor, report writer, outcome mapping, host interface) plus a thin `main.c` and `hostio.c` (POSIX files, clock, signals); executable target `ivs_run`, file `ivs-run`. Links only `ivs_core` (public headers). Built by default so both presets compile it under Werror.
2. A mode of an existing test binary. Rejected: tests link `ivs_core_internal` and ASan; a product runner must be a pure consumer of the public API.
3. `tools/` shell script driving a C consumer. Rejected: exit codes, signals and bounded IO are exactly what shell does badly.
4. Put the runner in core. Rejected: violates API-05 and the c-purity rules (no libc IO, clock).

[REC] Option 1. Rationale by lens:
- Architect: the runner is the first real consumer of the public API. Linking only `ivs_core` (not `ivs_core_internal`) proves the API is sufficient. A gate check (copy of the c-purity include scan, ten lines) asserts `runner/**` includes only `ivs/*.h`, libc and POSIX headers.
- DX: one binary, `ivs-run [options] scenario.ivss`. `--help` prints the exit-code table.
- DevOps: it is about 1,000 lines in five TUs; two presets add roughly 1 to 2 s hosted (cold gcc).
- Packaging: the install export set contains only `ivs::core` and its headers. [REC] The runner is built in-tree and not installed in Phase 4 (YAGNI; an installed-but-untested runner is an unverified claim; the relocated-prefix proof stays simple). Revisit at the first release that wants to ship it, as an optional `runner` install component outside the export set. The installed consumer must not need the runner. [REC] The consumer proves agreement differently: it runs the same project boot scenario by API calls and prints the composite digest; a gate step compares that digest with the runner golden. No dependency between the two, and agreement is evidence that the runner adds nothing the API does not.

## 3. Scenario format (B2)

Options.
- A. JSON subset, hand-written C parser. Pro: familiar, repo records are JSON. Con: [INF] 300 to 450 lines (strings, escapes, numbers, nesting, duplicate keys, depth limits), larger fuzz surface, no comments, quoting noise, awkward diffs, the repo's JSON is read only by CMake `string(JSON)`, not C.
- B. Line-based directive format ("IVSS v1"). Pro: [INF] about 150 to 250 lines, bytes-in/struct-out so trivially fuzzable, comments, line-numbered errors, greppable from POSIX sh in the gate. Con: bespoke; no nesting.
- C. TOML or INI subset. Con: more grammar than needed, still bespoke.
- D. Scenarios as C tables compiled into tests. Con: not a runner input; no standalone DX.
- E. CMake-script syntax. Con: absurd coupling.

[REC] B. Grammar (ASCII, LF, one directive per line, tokens split on spaces and tabs, `#` only at the start of a line, no quoting, no inline comments):

```
ivss 1                                 # first directive, mandatory
id <slug>                              # [a-z0-9-]{1,48}, echoed in the report
profile <profile-id>                   # exact registered id
content <role> <relpath>|user          # role: exec | grom | cart ; "user" = operator must supply via --exec/--grom/--cart
expect-load <role> ok|<IVS_E_NAME> [reason <n>]     # default ok; a matching refusal ends the scenario as pass
expect-sha256 <role> <64 hex>          # maps to IVS_LOAD_F_EXPECT_SHA256
wall-timeout-ms <n>                    # may only lower the operator cap
input <tick> <channel> <hex>           # RESERVED: parsed and validated, refused as unsupported until API-03 (D6)
run-to <n> ticks|cpu-cycles|frames [stop target|halted|faulted]   # absolute, strictly increasing across steps
assert <subject> ==|!= <value>         # evaluated at the end of the preceding run-to (a checkpoint)
```

Subjects: `r0..r7`, `pc`, `cpu.state` (running|halted|faulted|unreset), `cpu.fault`, `flag.s|z|o|c|i|d`, `mem.<view>.<hexaddr>` (side-effect-free peek). Values: decimal or `0x` hex, at most 32 bits for subjects, at most 2^62 for run bounds. Steps run in file order, so a scenario can checkpoint (assert, run on, assert again). Defaults: a `run-to` with no `stop` means `target`.

Rules that make it safe and stable: unknown directive, duplicate singleton directive, wrong order (header directives before steps), CR, NUL, non-ASCII, tab inside a token, or a line over 256 bytes is an error naming the line and a reason token (`scenario-syntax line=7 reason=unknown-directive`). Caps: file 64 KiB, 512 lines, 3 content lines, 64 asserts, 64 steps. Hand-written integer parser (no `strtol`/`sscanf`: locale and overflow hazards), ASCII classification by comparison (no `isdigit`).

Input schedule. [REC] The syntax slot is reserved and tested (valid lines parse and sort; the executor refuses a non-empty schedule with `unsupported-directive`), the report prints `input.events 0` and the Phase 4 input-log digest is the digest of the empty log. [INF] D6 (logical lossless vs electrical per port, C14) is open; fixing an operand syntax now would pre-decide it. Phase 5 supplies the operand grammar. [UNK] If the planner reads RUN-01 as requiring a working schedule in Phase 4, either mark RUN-01's "input schedule" element as completed in Phase 5 (traceability note) or pull a minimal input event API forward. Flagged as the one requirements-text tension.

Doc: `docs/runner.md` holds grammar, outcome table, exit codes, report keys and the trust model. The `--help` text, the doc table and `data/expected/runner-outcomes.ids` are compared by one TAP id (anti-drift).

## 4. Report format (B3)

Options: JSON (jq-friendly; needs a writer and escaping; key order and number formatting need discipline; poor diffs), `key value` lines (diffable, awk/grep in gate sh, trivial writer), TAP (wrong shape for one run). [REC] flat dotted `key value` lines, fixed order, ASCII, LF, first line `ivsr 1`, last line `report.end <number of preceding lines>` (truncation sentinel: a crash mid-write cannot read as pass). `--format json` is a mechanical later addition (reversal trigger: an integrator needs it).

Fixed key order (all keys always present; `-` when not applicable):
```
ivsr 1
scenario.id / profile / api.version
outcome <pass|guest-fail|timeout|cancelled|missing-fixture|harness-fail>
outcome.detail <token list, e.g. budget | wall | assert=3 | load-refused status=IVS_E_MALFORMED reason=1 | missing role=cart path=x/y.ivsc | scenario-syntax line=7 reason=...>
exit.code <n>
slot.<exec|grom|cart>.{loaded,length_bytes,sha256,status,reason}     # from the core load report
step.<k>.{target_ticks,now_ticks,overshoot_ticks,stop_reason,expected_stop}
step.<k>.assert.<j> pass|fail <subject> <op> <expected> got <actual>
cpu.run_state / cpu.fault_reason
digest.{cpu,mem,images,instance,composite,video,audio,input}         # 64 hex each
output.video.bytes / output.audio.samples
host.calls                                                           # only host-dependent key; ignored when comparing slice variants
report.end <n>
```
Never in the report: wall time, paths, pids, environment, free text from the scenario beyond charset-restricted tokens (so no newline injection, and tracked goldens cannot carry a private path, per the hygiene rule). Timing, if wanted, goes to stderr with `--timing`.
Digests are the core's per-domain digests. Raw video and audio outputs are caller-owned buffers sized by the core's size query (API-04); `--dump-video FILE` and `--dump-audio FILE` are operator-only flags.

Golden files: `data/golden/runner/<id>.report`, preceded by `#` header lines `tier:`, `oracle:`, `scenario:`, `profile:`. The check requires tier to be in `data/expected/evidence-tiers.ids` and oracle non-empty, strips the `#` lines and compares byte-for-byte. A manual `tools/runner-golden-refresh.sh` prints a diff and never runs in the gate (anti-pattern avoided: auto-blessing). Goldens exist only for scenarios without `user` content (a user image digest must never become a tracked fact by accident).

## 5. Outcome taxonomy and exit codes (B4)

| Outcome | Exit | Meaning | Demonstrating scenario (id) |
|---|---|---|---|
| pass | 0 | loaded, ran, stop and all asserts as expected | `diag-boot-pass` (RUN-06 original diagnostic) |
| guest-fail | 10 | the guest did not do what the scenario states: failed assert, guest halted or faulted when `stop target` was expected | `guest-assert-mismatch` (correct run, deliberately wrong expected value; also proves the assert engine can fail) |
| timeout | 11 | `budget`: the machine-time bound ended with the guest still running while `stop halted` was expected (deterministic); `wall`: the host watchdog fired (host-dependent) | `timeout-budget` (golden) and `timeout-wall` (outcome only) |
| cancelled | 12 | SIGINT or SIGTERM observed | `cancel-signal` (outcome only) |
| missing-fixture | 13 | a content file is absent, unreadable or not a regular file, or a `user` role has no operator path | `missing-cart-file`, `needs-user-exec` (`skip-ok`) |
| harness-fail | 14 | usage, scenario syntax or unsupported directive, unexpected load refusal, IO or allocation error, internal error | `bad-syntax`, `unexpected-refusal` |

Why 10 to 14: [OBS-by-reasoning] AddressSanitizer exits with 1 by default and aborts give 134, so `1` as "guest failure" would let a sanitizer finding masquerade as a verdict in the dev preset; 77 is this project's ctest skip code (automake convention [BG]); 126 to 255 belong to the shell and signals. pytest's distinct small codes (0 failed 1, interrupted 2, internal 3, usage 4, no tests 5 [BG]) are the precedent for distinct classes; the project's range just avoids the collisions. Any other code, or no report, or a report whose `outcome` and `exit.code` disagree, is `runner-crash` in the judge.
Precedence when several apply: harness-fail > missing-fixture > (load refusal handled per `expect-load`) > cancelled > timeout(wall) > timeout(budget) > guest-fail > pass. A matching `expect-load` refusal ends the scenario as pass: that makes the refusal matrix runnable through the runner (`expect-load cart IVS_E_MALFORMED reason 3`).
A user-image scenario in CI exits 13; the inventory marks it `skip-ok`, so the skip is printed as `# SKIP` by the TAP suite and counted by `tests-inventory` (PKG-03 "shows skips").

## 6. Timeout and cancellation without core clocks (B5)

Two different things share the word "timeout", and conflating them is the classic footgun [BG]:
1. Machine-time budget: the scenario's own run bound. Deterministic, goldens allowed. Reaching it is a normal stop (`stop_reason target`); it becomes outcome `timeout` only when the scenario asserts the guest halts (`stop halted`) and it did not.
2. Wall-clock watchdog: protects the host against an absurd bound in an untrusted scenario (a target near 2^62 never finishes). Host-dependent; never golden. Exists only in `hostio.c`.

Mechanism. The runner never passes the final target in one call; it runs in slices of at most K ticks (default 1<<22, a constant of the runner), `ivs_machine_run(target = min(final, now + K))`. Between slices it reads `volatile sig_atomic_t cancel` (set by a `signal()` handler for SIGINT and SIGTERM) and the host monotonic clock against the deadline. Safe only because API-02 guarantees slicing invariance, which the gate also tests. [INF] Cancel and wall latency is one slice (milliseconds); `host.calls` is the one report key that varies. The state at interruption is a deterministic prefix state, and the report says `outcome.detail wall` or `signal`, with digests of that prefix, tagged not comparable to goldens.
Deadline: operator `--wall-timeout-ms` (default 30000; ceiling is the operator's) and scenario `wall-timeout-ms` may only lower it. Hard ceiling `--max-ticks` (default 2^40) refuses larger bounds as harness-fail `bound-too-large`.
Test hooks (documented, not hidden): `--self-signal-after-slices N` calls `raise(SIGINT)` in-process, so the real handler path is exercised with no race and no sleep; the wall case uses `--wall-timeout-ms 1` with a huge bound, which can only end by watchdog (no flake, because slower hosts just do fewer slices).
Host services sit behind a 3-function vtable (`read_file`, `now_ms`, `cancel_requested`) so the in-process unit suite injects a fake clock and an in-memory file provider: deterministic timeout and cancel tests with no wall time at all.
Alternatives rejected: SIGALRM (async-signal-safety traps, no easy fake), threads (a dependency on the threading model, determinism worries), core-side deadlines (violates API-05), external `timeout(1)` (absent on macOS; also loses the report).

## 7. File IO and paths (B6)

Trust model [REC]: the scenario file is untrusted inert data; the operator's command line is trusted. A scenario can only name relative paths under `--content-dir` (default: the scenario's own directory), can never name an output file, run a command, read the environment or raise a limit. Rights: system images come only from `--exec PATH` and `--grom PATH` (and `--cart`) given by the operator, never from tracked data; the runner never writes, copies or caches image bytes; CI never passes them.
Path rule (copy of the fixture-manifest path rule, which the repo already checks): `[A-Za-z0-9._/-]`, not starting with `/`, no `..` or empty segment, at most 200 bytes, so `..` traversal and absolute paths are refused by the parser before any open. `stat` must report a regular file (a FIFO or `/dev/zero` under the content dir would hang or run unbounded); symlinks inside an operator-chosen content dir are the operator's choice (documented).
Bounded reads: `fread` at most cap+1 bytes into a runner buffer, where cap is the public bound of the role (EXEC 8192, GROM 2048, cartridge 131,256, scenario 64 KiB). Passing cap+1 bytes lets the core report its own authentic OVERSIZED refusal, so a 4 GiB file costs 131,257 bytes of reading. Short reads, `ferror` and a failed `fclose` on the report are harness failures (a truncated report must never look like a pass).
CWD independence: all content paths are resolved against the content dir, never the cwd, and the gate runs scenarios from a different cwd as one TAP id.

## 8. Isolation (B7)

One process runs one scenario on one freshly created instance in runner-owned, aligned storage sized by `ivs_machine_storage_requirements` (capped at 64 MiB). The runner has no globals except the signal flag. Several scenarios means several invocations from sh (about 10 ms each). Rejected: an in-process multi-scenario loop (a shared-state leak would hide behind the loop; process exit is the strongest isolation and costs nothing here). Aligned allocation by hand (over-allocate and align), not `aligned_alloc` (size-multiple rule, macOS availability).

## 9. How the runner is tested (B8, RUN-04)

Two suites, both inside existing machinery, no new gate step and no new workflow step.
1. Suite `runner` (C, in-process, harness TAP, both presets, fault-capable). It tests the library through the host vtable with fakes: parser table (about 40 accept and reject rows, one per reason token and each cap and each path rule), a seeded mutation fuzz of the valid corpus under ASan/UBSan (mirror of `ivsc_fuzz`: never crash, accept only with the invariants, reject with a reason; optional libFuzzer recipe like `fuzz_ivsc.c`), outcome precedence, exit-code mapping, assertion engine, report key order and `report.end`, bounded read (cap+1), fake-clock timeout and fake cancel, slicing invariance (slice sizes 1, 7, 4093, huge produce equal reports modulo `host.calls`), repeated-run equality.
2. Suite `runner-process` (POSIX sh TAP script via ctest, precedent `headers-tap.sh`; ids in `data/expected/tests/runner-process.ids`, `skip-ok` for `needs-user-exec`). It runs the real `ivs-run`: each scenario of `data/expected/runner-scenarios.ids` (`<id> <exit> <outcome> [host-dependent] [skip-ok]`), compares exit code, `outcome` line and, unless host-dependent, the golden body; runs release and dev binaries (the dev ASan build and the release build must agree byte for byte: a free UB detector); runs slice variants via `--slice-ticks` on the release binary only; runs from another cwd; checks the outcome set covered equals the full taxonomy (token `runner-outcome-uncovered`); checks `--help`, docs and inventory agree. Real signal and real wall timeout use the hooks above.
The judge of suite 2 has its own named-fault controls, as TAP ids of the same suite: it is pointed at a fake `ivs-run` shell script that returns a wrong exit code, a mutated golden body, a missing `report.end`, an `outcome`/exit disagreement, and it must report not ok with the intended reason. No framework change is needed for these.
Named faults (RUN-04 for the runner), in `data/expected/faults/runner.ids`, detected by suite `runner`: `RUNNER_ASSERT_ALWAYS_PASS`, `RUNNER_EXIT_ZERO_ON_GUEST_FAIL`, `RUNNER_OVERSHOOT_NOT_REPORTED`, `RUNNER_DOTDOT_ACCEPTED`, `RUNNER_READ_UNBOUNDED`, `RUNNER_CANCEL_IGNORED`, `RUNNER_WALL_AS_GUEST_FAIL`, `RUNNER_MISSING_AS_HARNESS`, `RUNNER_KEY_ORDER_SWAPPED`, `RUNNER_SLICE_DROPS_REMAINDER`, `RUNNER_STOP_EXPECT_IGNORED`. Each must fail only its intended vectors (existing allowed-prefix columns). This requires generalizing the fault framework from `core/`-relative owner TUs to repo-relative ones (D-B16); [UNK] that refactor's size (tests/faults.cmake owner lookup, `faults-verdict` site scan, `c-purity`-like scans stay core-only).
Negative controls (`tools/check/controls`): the existing controls cover tests-inventory and faults-verdict generically; add four for new pure checks only: runner include scan (`runner-include-forbidden`), scenario inventory vs files (`runner-scenario-unlisted`, `runner-scenario-missing`), golden header (`runner-golden-no-tier`), tracked-golden-for-user-scenario. Each needs `CHECK`, `EXPECT`, an inventory line.
Gate cost [INF, to be measured]: c-build +1 to 2 s (two presets, five TUs), `runner` suite under 1 s, `runner-process` 1 to 3 s (about 12 scenarios x 2 presets plus 3 slice runs, tiny guests), 11 faults about +0.5 s, 4 controls overlapped. Estimated +3 to 5 s hosted against a 40 s budget that already runs 30 to 49 s. [REC] Measure after the first runner plan lands (D-38 pattern); the first lever is to run slice variants and the dev/release cross-check only in the release preset, the second is the existing owning-TU rebuild lever.

## 10. Portability (B10)

- `-std=c17 -pedantic`, extensions off: `hostio.c` alone defines `_POSIX_C_SOURCE 200809L` before any include and holds the only non-ISO calls (`clock_gettime(CLOCK_MONOTONIC)`, `stat`). `signal()` (ISO) is used for handlers, not `sigaction`. SIGPIPE ignored and every write checked.
- No locale dependence: hand ASCII classification, `LC_ALL=C` in scripts. `uint64_t` printed with `PRIu64` or casts to `unsigned long long`.
- gcc 13 lessons from Phase 3: no function call whose arguments depend on each other's side effects (evaluation order); `-Wstringop-overflow` friendliness through explicit lengths; the runner is not subject to the core symbol allowlist.
- POSIX sh and dash: no `local`, `echo -e`, `sed -i`, `grep -P`, `timeout`, `date +%N`, fractional `sleep`; hang guard by copying the `c-faults` background-watchdog pattern; compare with `cmp -s`.
- Windows/MSVC is deferred (stack decision); `hostio.c` is the only file that would change.

## 11. Security (B11)

Untrusted scenario: size, line, token, directive and step caps; strict charset; integer parsing without overflow; duplicates and unknown directives refused; report echoes only charset-restricted tokens (no injection, no paths); resource ceilings (`--max-ticks`, wall cap, storage 64 MiB, output buffers at most 8 MiB, `malloc` failure is a harness-fail). Untrusted images: bounded read, core loader (already fuzzed) does validation, runner never interprets bytes. Operator-only output paths. Exit code and report are written even for failures so failures are diagnosable. Rights: no image bytes in any report except identity SHA-256 of an operator-supplied file in that operator's own run (the AGENTS.md allowance for an identity hash); goldens and the gate never see one.

## 12. Lenses, condensed

- Emulator architect: runner proves the run API; slicing-through-watchdog doubles as continuous slicing-invariance exercise. Needs inspection, run and size-query public API (dependency on decision point A).
- Hardware/EE: nothing electrical; input channel naming deferred to D6 so either electrical or logical representation fits.
- QA: independent expected values: the diagnostic's expected predicate must be derived outside the emulator (hand-computed from the GI manual or by a CMake script); the runner never computes an expected value from emulator output. The `guest-assert-mismatch` scenario keeps the assert engine honest.
- Security/rights: trust split above; never-committed images; goldens guarded.
- DevOps: in-gate only, existing TAP and fault judges reused, budget measured, `gate-linux.sh` run before the push of the first runner plan.
- API/integrator: runner = reference consumer; report keys are the stable observable contract; exit codes documented.
- Product: `ivs-run scenario.ivss` is the demo a reviewer can run; failures say which kind.
- DX: line-numbered errors, comments, `--check` (parse only, exit 0 or 14), `--help` table, one doc.

Lessons [BG, reasoned]: test-ROM suites that report through a memory byte and a stop condition (blargg-style) map exactly onto `stop halted` plus `assert mem`; frame-count run bounds in emulator harnesses become non-comparable across cores, hence the absolute-tick bound; harnesses that only check the exit code let crashes read as failures-by-design, hence `report.end` plus outcome/exit agreement; golden auto-bless culture rots evidence, hence manual refresh only.

## 13. Adversarial pass

1. "A bespoke format will bite integrators." Mitigation: tiny grammar, one doc, `--check`; the parser is isolated behind a struct, so a JSON front end can be added without touching the executor. Reverse if scenarios need nesting or are machine-generated by external tools.
2. "Slicing for the watchdog makes every run depend on slicing invariance." True, and intended; the invariance test runs in the gate at four slice sizes, and a fault (`RUNNER_SLICE_DROPS_REMAINDER`) proves the test can fail. If the run API cannot be sliced cheaply (per-call overhead), K grows; the cancel latency grows with it.
3. "Exit codes 10 to 14 are unusual." Chosen to avoid ASan (1), skip (77) and shell (126+) collisions; documented in `--help`. Reverse only if a hosting convention demands otherwise.
4. "Two suites plus faults is heavy for a 1,000-line tool." The runner is the evidence gateway for every later phase (RUN-05, SCN); the machinery reuses existing judges, so new surface is ids, not new steps. The heaviest piece is D-B16 (fault framework generalization): if it proves large, fall back to testing the same faults by building the runner library with `-DIVS_FAULT_X` into one extra suite executable list in `tests/CMakeLists.txt`, accepting weaker site checks.
5. "Not installing the runner makes it unreleasable." Deferred, not forbidden; the consumer digest cross-check preserves the claim that the installed API alone reproduces the runner's evidence.
6. "Assertions need peek and register reads that the public API lacks." Correct: the side-effect-free inspection API is a prerequisite (Phase 6 wants it anyway). If rejected, assertions degrade to digest equality, which is not an independent oracle, so RUN-06's independent predicate would be unprovable: treat as a blocker, not a trade-off.

## 14. Numbered decisions for CONTEXT (also in the reply)

D-B1 `runner/` library `ivs_runner_lib` plus `main.c`/`hostio.c`, executable `ivs-run` (target `ivs_run`), links only `ivs_core`, built in both presets.
D-B2 Scenario format IVSS v1: line-based directives, no quoting, strict charset and caps, `ivss 1` first, steps run in order with checkpoint asserts, `input` reserved and refused until API-03.
D-B3 Report: flat `key value` lines, fixed key order, `ivsr 1` first and `report.end <n>` last, no wall time, no paths; goldens carry `# tier:` and `# oracle:` headers and are refreshed only manually.
D-B4 Outcomes and exit codes: pass 0, guest-fail 10, timeout 11 (`budget` or `wall`), cancelled 12, missing-fixture 13, harness-fail 14; fixed precedence; one demonstrating scenario each from `data/expected/runner-scenarios.ids`.
D-B5 Timeout and cancel only in the runner: slices of at most 1<<22 ticks, `sig_atomic_t` flag and monotonic clock between slices, scenario may only lower the wall cap, `--max-ticks` ceiling; deterministic hooks `--self-signal-after-slices` and `--wall-timeout-ms`.
D-B6 Trust split: scenario is inert untrusted data (relative `[A-Za-z0-9._/-]` paths without `..` under `--content-dir`, no output paths); system images only via operator `--exec/--grom/--cart`; bounded cap+1 reads, regular files only.
D-B7 One process, one scenario, one fresh instance, no globals except the signal flag.
D-B8 Tests: in-process suite `runner` (fake host vtable, parser table, seeded fuzz, slicing, named faults) plus shell TAP suite `runner-process` (real exe, dev vs release equality, slice variants, judge self-controls); both under existing inventories.
D-B9 Runner not part of the install package or export set and not required by the installed consumer; consumer digest compared to the runner golden by the gate.
D-B10 Portability rules: `_POSIX_C_SOURCE` only in `hostio.c`, `signal()` not `sigaction`, no locale-dependent calls, POSIX sh/dash scripts, run `tools/gate-linux.sh` before the first runner push.
D-B11 Security caps: scenario 64 KiB, 512 lines, 256 bytes per line, 64 asserts, 64 steps, storage 64 MiB, buffers 8 MiB, default wall cap 30 s.
D-B16 Generalize the named-fault framework to repo-relative owner TUs (`core/src/...` and `runner/...`) and extend the `faults-verdict` site scan to `runner/`.
D-B12 Public API prerequisites owned by decision point A: bounded run with overshoot and stop reason, side-effect-free register and memory inspection, output-buffer size query, profile timing query (ticks per CPU cycle, per frame), digest domains 16 to 18.

## 15. Top risks, reversal, unknowns

Risks: (1) API prerequisites (D-B12) land late or smaller, blocking assertions and RUN-06 independence; (2) gate cost crosses the 50 s trigger on a hosted spread already 30 to 49 s; (3) the fault framework generalization (D-B16) is larger than estimated.
What would reverse it: JSON-needing consumers (add a front end, keep the struct); a hosting convention for exit codes; evidence that slice overhead makes watchdog granularity too coarse; a release that wants a shipped runner (add an install component).
[UNK] blocking planning: (a) exact public run, inspection, size-query and timing-query signatures (decision point A); (b) whether the project boot path needs a project-authored GROM placeholder (the profile marks GROM required at reset, so `ivs_machine_reset` refuses without one; a 2048-byte zero image is trivially original, but it must be named and manifested); (c) RUN-01 "input schedule" scope in Phase 4 versus Phase 5 (D6). Non-blocking: core run speed (sets the slice constant), size of D-B16.
