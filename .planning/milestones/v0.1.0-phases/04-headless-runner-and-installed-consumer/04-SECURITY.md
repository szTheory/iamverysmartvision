---
phase: 04-headless-runner-and-installed-consumer
asvs_level: 1
block_on: high
status: secured
threats_total: 80
threats_closed: 80
threats_open: 0
audited: 2026-11-11
audited_commit: c07e56f
---

# Phase 4 Security Verification

**Verdict: SECURED.** All 80 declared threats are closed: 77 mitigate, 2 accept, 1 transfer. Every mitigation was found in code, tests or the gate at the cited location. No blocking or non-blocking open threats. The hard constraints pass on the whole branch diff (112 commits). No new attack surface lacks a mapping. One auditor observation (symlinks inside the content directory are followed) is recorded under Residual risks; it does not block.

Phase 4 adds a lot of new surface:
- Public run, input and observation APIs (`ivs_machine_run`, input events, peek, cpu_state, info) with event arrays, output buffers, report out-structs, `struct_size` tails and 64-bit tick targets.
- The headless runner `ivs-run`. It reads untrusted IVSS scenario files and image files under a content directory, takes operator-only `--exec/--grom/--cart` image paths, handles signals, cancellation and wall timeouts, and writes `--update-golden` output.
- `tools/pack` (listing parser and image writer).
- The installed package and its export.
- An independent installed consumer.
- CI and gate helpers: `ci-summary.sh` writing `GITHUB_STEP_SUMMARY`, `ci-record.sh` calling `gh`, and `gate-linux.sh`.

## Verification performed

- **Code reading:**
  - Runner: `runner/{hostio,main,exec,scenario,report,outcome}.c` and `runner/ivs_runner.h`.
  - Core: `core/src/machine/{machine,machine_bound,machine_input,machine_inspect}.c` and `core/include/ivs/ivs_run.h`.
  - Tools and CI: `tools/pack/pack.c`, `tools/check/ci-summary.sh`, `tools/ci-record.sh`, `tools/check/steps/{ci-parity,pkg-install,c-build}.sh`, `tests/faults.cmake`, `tests/runner-process-tap.sh`, `runner/CMakeLists.txt` and `.github/workflows/gate.yml`.
- **Fault wiring:** all 20 named faults cited by the register have an `IVS_FAULT_*` site and an entry in `data/expected/faults/{run,runner,machine}.ids`. Sites:
  - `machine_bound.c:77`, `:309`, `:316`, `:341`, `:361`, `:231`
  - `machine_input.c:29`, `:80`, `:140`
  - `machine_inspect.c:97`, `:113`
  - `machine.c:261`
  - `machine_run.c:58`
  - `runner/scenario.c:178`, `:813`
  - `runner/exec.c:70`, `:92`, `:131`, `:374`
  - `runner/outcome.c:13`
- **Cited controls:** every control the register names is present in `tools/check/controls/`. The one exception is `c-build-unlisted-source`: it has an emit site (`c-build.sh:49`) and the plan only promises a quoted probe.
- **Full local gate (fresh, this audit):** `sh tools/gate.sh` at `c07e56f` printed `gate: PASS steps=33/33 controls=276/276 ... faults=149/149 ... pkg-selftest=10/10 ... tests-expected=2780 tests-executed=2778 tests-skipped=2 tests-missing=0`, with `gate-time total=47`.
- **Sanitized suites:** on `build/dev` (`IVS_SAN=ON`, Debug), `ctest -R '^(run|runner|runner-process|ivsc|machine|pack|headers)$'` passed 7/7 with no ASan or UBSan report. That covers the seeded runner and run fuzz, the refusal matrices and the real SIGINT/SIGTERM process cases.
- **Adversarial runner probe** (auditor-written in the session scratchpad, not committed; ASan+UBSan `ivs-run`, `halt_on_error=1`):
  - Path rules: content paths `../x`, `a/../../x`, `/etc/passwd`, `a//b` and `a%b` are refused at parse time (`path-dotdot`, `path-absolute`, `path-empty-segment`, `path-byte`, exit 14).
  - Non-regular content: a FIFO or a directory as content gives `missing-fixture not-regular` (no hang, thanks to `O_NONBLOCK` then `fstat`). A FIFO, a directory, `/dev/zero` or `/dev/null` as the scenario gives "not a regular file".
  - Oversized input: a 300 KB cart or 131,300 random bytes gives `load-refused IVS_E_OVERSIZED` (bounded read at cap+1). A 300 KB file passed with `--exec` gives OVERSIZED.
  - Scenario caps: 70 KB gives `file-too-large`, a NUL byte gives `nul`, a 400-byte line gives `line-too-long`, and 600 lines give `too-many-lines`.
  - Option ranges: `--max-ticks -1`, `--max-ticks 2^64+`, `--max-ticks 2^63+1`, `--wall-timeout-ms 0` or `2^32`, and `--slice-ticks 0` or `4194305` are all refused "option value out of range".
  - Golden refusal: `--update-golden` on a `content exec user` scenario gives "golden refused: content user" and no file is created.
  - Reports: a valid user-EXEC run passes and its report has no path text.
  - Fuzz: 300 random-byte scenarios and 300 token-mutated valid scenarios produced 0 sanitizer reports, 0 crashes and 0 hangs.

## Hard constraints (whole branch diff vs main, 112 commits)

| Constraint | Result | Evidence |
|---|---|---|
| No rights-bearing material | PASS | No `.ivsc/.bin/.rom/.exec/.int/.itv/.cfg/.pdf` file is added in the diff or appears in any `main..HEAD` commit. No added file has a non-text MIME type. Images are built under `build/<preset>/images/` only; the only fixture is the text listing `fixtures/FX-boot-stub/boot-stub.listing.txt` plus its manifest. `tools/pack --c-include` embeds images only into the build-tree include of the in-process suites (`tests/CMakeLists.txt:142-149`), never into the consumer. Gate: `files ... binaries-listed=0`, `manifests=4` |
| `refs/` ignored and untracked | PASS | `.gitignore:2 /refs/`; `git ls-files refs` is empty. `.gitignore:11,12,16` cover `*.bin`, `*.rom`, `*.ivsc` |
| No PII, credentials, tokens or private paths in the branch diff or HEAD | PASS | ERE scan over every added line of `git diff main...HEAD`, the HEAD tree and all 112 branch commits (patches and messages). Patterns: `/Users/<x>/`, `-Users-<x>-`, `/private/tmp/<x>`, `/private/var/<x>`, `<scratch-path>`, `<scratch-path>`, `/home/<x>/`, `gh[pousr]_`, `github_pat_`, `sk-ant-`, `AKIA`, `xox?-`. Zero concrete hits. The only scratch-related strings in the diff are rule text and placeholders: `04-RESEARCH.md` "`/private/tmp/<name>`, `<scratch-path>`", an elided Linux container path `/tmp/pkg-install...` in `04-22-SUMMARY.md`, and the word "scratchpad" in `04-DISCUSS-diag.md` and `04-DISCUSS-pkg.md`. Every author and committer is `szTheory@users.noreply.github.com` |
| Scratch-path strings already in public history (separate list for the owner) | INFO, not in the branch diff | Three commits on both `main` and `origin/main` (public) added literal scratch-path text in planning docs. Later commits removed each one, and none is in HEAD or `main`'s tree. (1) `c89aa48` (2026-11-09, `01-REVIEW.md`): `<scratch-path>`, `<scratch-path>`, `<scratch-path>`, plus a sentence citing `<scratch-path>`; removed by `1b313b1`. (2) `6008847` (2026-11-09, `01-RESEARCH.md`): the sed regex `s#<scratch-path>` (a pattern, not a path; now defanged as `[c]laude`). (3) `345c48d` (2026-11-11, `03-SECURITY.md`): the list item `` `<scratch-path>` ``; reworded by `b65296f`. None contains a username or a home path. The exposure is the uid `501` and the Claude scratch-directory prefix. Removing them needs a history rewrite of public `main`: an owner decision |
| No copying from other emulators or their format docs | PASS | Outside `.planning/` and the denylist files, added content mentions MAME, jzIntv, Zbiciak, as1600, FRASM, CFGVAR, LUIGI or `[mapping]` only as source citations (`SRC-bsr-2609`, `SRC-intv-wiki` in `data/`). The copyright lines are the project's own. `denylist pass scanned=280 waived=0` |
| "Intellivision" not used as a name | PASS | No tracked path contains it. `project(ivs ...)` (`CMakeLists.txt:20`), export `ivsTargets`, target `ivs::core` (`core/CMakeLists.txt:66`, `:96`), `ivs-run`, `ivs_pack` and format IVSS/IVSC. The profile id uses `intv`, not the trademark |
| No assembler dependency | PASS | No `libasm`, `as1600`, `FetchContent` or `ExternalProject` in any tracked `*.cmake`, `CMakeLists.txt`, `*.sh`, `*.c`, `*.h` or `*.yml`. The one added `find_package` is the consumer's `find_package(ivs 0.1 CONFIG REQUIRED)` of the project's own package |
| CI workflow least-privilege and SHA-pinned | PASS | `git diff main...HEAD -- .github` is one line, from commit `6ba7323` (04-01): `timeout-minutes: 10` became `5`. The workflow keeps `permissions: contents: read`, one action `actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1` with `persist-credentials: false`, and no secrets, cache, upload or matrix. `ci-parity.sh:85` (secrets) and `:131` (allowlist only `actions/checkout`) enforce this. `ci-parity pass ci-uses=1 ci-workflows=1` |
| `tools/gate-linux.sh` | PASS (unchanged since Phase 3) | No diff in this branch. The Phase 3 warning still stands (image by tag, apt install inside the container, accepted under T-03-SC); Phase 4 relies on it as the pre-push hosted proxy (T-04-58) |
| `tools/ci-record.sh` | PASS | Only two `gh run view` calls (read-only), at `:105` and `:107`, and they need `gh` on PATH (`:97`). The gate, the steps and the workflow never call it; `ci-log.sh:4` only reads its output. The `--from-files` offline mode (`:90`) serves the tests |
| `tools/check/ci-summary.sh` | PASS | Returns early when `GITHUB_STEP_SUMMARY` is unset (`:18`). Input is only the gate's own temp files. Backticks, pipes and CR are stripped (`:24`, `:67`). FAIL lines are fenced and capped at 40. It appends (`:76`), always ends `exit 0` (`:77`), and is called `> /dev/null 2>&1 \|\| :` (`tools/gate.sh:20`). Selftests `backticks-stripped` and `unwritable-keeps-exit` (`steps/ci-summary.sh:94`, `:102`). `ci-summary-selftest=11/11` |
| Guest/host boundary bounded | PASS | Overlap tests use `uintptr_t` differences with 64-bit lengths (`machine.c:61-72`, `machine_bound.c:147-158`). The event count is capped before any span arithmetic (`machine_bound.c:161-170`, `:228`). Peek range is checked before the span (`machine_inspect.c:88-91`). Struct sizes are capped at 4096 (`ivs_machine.h:70`). Executor storage is bounded (`runner/exec.c:607-630`). Sanitizer suites and probes above are clean |

## Threat verification

| Threat ID | Category | Severity | Disposition | Status | Evidence |
|---|---|---|---|---|---|
| T-04-01 | Tampering | medium | mitigate | CLOSED | `tools/check/ci-summary.sh:24`, `:38-41`, `:66-68`: input only from gate temp files, backticks stripped, FAIL lines fenced, no `::` annotations. Selftest `backticks-stripped` (`steps/ci-summary.sh:94`) |
| T-04-02 | Repudiation | high | mitigate | CLOSED | `tools/gate.sh:20` (`> /dev/null 2>&1 \|\| :`); `ci-summary.sh:77` `exit 0`; selftest `unwritable-keeps-exit` (`steps/ci-summary.sh:102`) |
| T-04-03 | Information disclosure | high | mitigate | CLOSED | `ci-parity.sh:10-11`, `:131` (allowlist `actions/checkout`). Controls `ci-parity-action-cache`, `ci-parity-action-upload`. Workflow has no cache or upload |
| T-04-04 | Elevation of privilege | medium | mitigate | CLOSED | `ci-parity.sh:85` `ci-parity-secrets`; control present. `gate.yml` has `permissions: contents: read` and `persist-credentials: false`; control `ci-parity-write-permissions` |
| T-04-05 | Tampering | low | mitigate | CLOSED | `tools/check/steps/ci-log.sh`; controls `ci-log-duplicate`, `ci-log-fields`, `ci-log-header`, `ci-log-order`. `ci-log pass ci-runs=23` |
| T-04-06 | Denial of service | medium | mitigate | CLOSED | `tools/ci-record.sh:105`, `:107` (two calls); `--from-files` at `:90`. Not invoked by gate, steps or workflow |
| T-04-07 | Information disclosure | low | accept | CLOSED (accepted) | Accepted in 04-02 `<threat_model>`. Confirmed: `.planning/ci-runs.tsv` rows hold only date, run id, SHA, event, conclusion and numbers. Logged below |
| T-04-08 | Spoofing | high | mitigate | CLOSED | `faults-verdict.sh` scan includes `runner/` (fault sites in `runner/*.c` are judged; `faults=149/149`). Controls `faults-orphan-site-runner`, `faults-missing-site-runner` |
| T-04-09 | Tampering | medium | mitigate | CLOSED | `tests/faults.cmake:54-57`: configure fails on a duplicate stem across `core/src` and `runner/`; rule restated at `runner/CMakeLists.txt:6-7` |
| T-04-10 | Elevation of privilege | medium | mitigate | CLOSED | `runner/CMakeLists.txt:24` (`ivs_runner_lib PUBLIC ivs_core`, no `ivs_core_internal`). No runner include outside `<...>`, `"ivs/..."`, `"hostio.h"` and `"ivs_runner.h"`. Control `runner-scan-internal-include` |
| T-04-11 | Repudiation | medium | mitigate | CLOSED | `runner/outcome.c:7` codes 0, 10, 11, 12, 13, 14 (`runner/main.c:4`, `MAIN_EXIT_HARNESS 14`): none is 1, 77 or 126+. Four `OUTCOME_*` faults in `runner.ids` (site `outcome.c:13`, `:28`) |
| T-04-12 | Tampering | medium | mitigate | CLOSED | `_POSIX_C_SOURCE` only at `runner/hostio.c:8`. Controls `runner-scan-header`, `runner-scan-internal-include`, `runner-scan-posix-define`. `runner-scan pass runner-files=8` |
| T-04-13 | Repudiation | low | mitigate | CLOSED | Controls `integrity-missing-spdx-runner`, `integrity-non-ascii-runner`, `integrity-missing-spdx-tools-c`. SPDX first line on all runner, pack and consumer sources |
| T-04-14 | Spoofing | medium | mitigate | CLOSED | `tools/check/steps/c-build.sh:41-49`: `runner/` and `tools/pack/` loops emit `c-build-unlisted-source`. Probe quoted in 04-04-SUMMARY, as the plan specifies |
| T-04-15 | Tampering | high | mitigate | CLOSED | Case `machine.digest.composite-excludes-16-18` in `data/expected/tests/machine.ids`; stepped-state literals green in the gate |
| T-04-16 | Repudiation | medium | mitigate | CLOSED | Case `machine.digest.coverage` (seven domains) in the inventory; explicit pad row in `machine_digest.c` |
| T-04-17 | Tampering | medium | mitigate | CLOSED | Fault `OBS_NOT_CLEARED_ON_RESET` (`machine_run.c:58`); case `machine.reset.clears-observation` |
| T-04-18 | Tampering | high | mitigate | CLOSED | `machine_inspect.c:50-58` (cpu_state: size cap, then BAD_HANDLE, then overlap), `:88-91` (peek). `ivs_machine_span_overlaps` uses `uintptr_t` (`machine.c:61-72`). Refusal rows with poisoned outs in the `run` suite (ASan pass) |
| T-04-19 | Tampering | high | mitigate | CLOSED | `ivs_machine_peek` takes `const ivs_machine *` (`machine_inspect.c:78`). Faults `PEEK_ADVANCES_TIME` (`:113`) and `PEEK_GATE_IGNORED` (`:97`). `run.peek.*` cases |
| T-04-20 | Information disclosure | low | mitigate | CLOSED | `machine_inspect.c:88`: `addr + count > 0x10000` gives OUT_OF_RANGE before any read; count 0 gives BAD_ARG (`:82`) |
| T-04-21 | Tampering | medium | mitigate | CLOSED | `machine_bound.c:309-314` (`target_ticks > IVS_RUN_TARGET_MAX` gives OUT_OF_RANGE, stage 6, before any unit). Fault `RUN_TARGET_UNCHECKED`. `IVS_RUN_TARGET_MAX` at `ivs_run.h:95` |
| T-04-22 | Tampering | high | mitigate | CLOSED | `machine_bound.c:252-280`: rep size check, then BAD_HANDLE (`:270`), then report overlap (`:276`) |
| T-04-23 | Denial of service | medium | mitigate | CLOSED | `machine_bound.c:54-55`, `:460` (INTERNAL stop on no time advance); faulted CPU not stepped (`:418`). Fault `RUN_FAULT_STEPS` (`:341`) |
| T-04-24 | Tampering | medium | mitigate | CLOSED | Controls `api-stop-unpinned`, `api-stop-renumbered`, `api-reason-unpinned`. `api-effects pass pinned-defines=90` |
| T-04-25 | Repudiation | low | mitigate | CLOSED | `pinned-defines=90` (nonzero) in the gate line; missing pin file fails closed (`api-effects.cmake`) |
| T-04-26 | Tampering | high | mitigate | CLOSED | Fault `INFO_TAIL_UNGUARDED` (`machine.c:261`); case `run.info.v1-size-caller` (poisoned tail) in `run.ids` |
| T-04-27 | Tampering | high | mitigate | CLOSED | Fault `RUN_BUDGET_CARRY` (`machine_bound.c:77`, `:398`, `:473`); slicing cases (`runner-process.slices.*`, run suite); `c-purity` writable-data rule passes (`core-imports=4`) |
| T-04-28 | Tampering | high | mitigate | CLOSED | `machine_bound.c:204-217` (video and audio against the instance, each other and the report; `RUN_BUFFER_OVERLAP`), after BAD_HANDLE; `uintptr_t` with 64-bit lengths (`:147-158`) |
| T-04-29 | Tampering | high | mitigate | CLOSED | `machine_bound.c:316` capacity check in validation before the first unit; fault `RUN_CAPACITY_AFTER_STEP` (`:316`, `:444`) |
| T-04-30 | Information disclosure | medium | mitigate | CLOSED | Fault `OUTBUF_PARTIAL_WRITE` (`machine_bound.c:361`); poison-pattern refusal rows in `run` |
| T-04-31 | Tampering | high | mitigate | CLOSED | `machine_bound.c:221-245` (NULL, `> IVS_INPUT_EVENTS_MAX` 65536, overlap with instance, report, video and audio); `reserved_in` (`:289`); whole-array validation `machine_input.c:14-15`, `:75-98` before any write. Fault `RUN_EVENTS_UNCHECKED` (`:231`) |
| T-04-32 | Tampering | medium | mitigate | CLOSED | `machine_input.c:75` (`t >= IVS_RUN_TARGET_MAX` gives BAD_ARG); floor check at `:80`; fault `INPUT_NO_FLOOR_CHECK` |
| T-04-33 | Repudiation | medium | mitigate | CLOSED | Fault `INPUT_LOG_SLICE_RELATIVE_TICK` (`machine_input.c:140`); `cmake -E sha256sum` oracle in the input-log cases |
| T-04-34 | Tampering | medium | mitigate | CLOSED | Fault `INPUT_STATIC_CARRY` (`machine_input.c:29`, `:62`, `:148`); case `run.input.isolation`; c-purity writable-data rule |
| T-04-35 | Tampering | low | mitigate | CLOSED | `machine_input.c:91-98` (pass 2 UNSUPPORTED from the profile controller count, before any write); refusal case with memcmp |
| T-04-36 | Spoofing | medium | mitigate | CLOSED | Case `run.input.guest-invisible` in `run.ids` |
| T-04-37 | Tampering | medium | mitigate | CLOSED | Controls `api-input-bit-duplicate`, `api-input-bit-unpinned` |
| T-04-38 | Repudiation | low | mitigate | CLOSED | C63-C67 present in `data/discrepancies.json`; discrepancies step passes (`discrepancies=68`) |
| T-04-39 | Tampering | medium | mitigate | CLOSED | Seeded run fuzz with non-vacuity in the `run` suite (sanitized pass) |
| T-04-40 | Information disclosure | high | mitigate | CLOSED | Images under `build/dev/images/` (for example `boot-stub.exec`); `build/` ignored; no image tracked; `files ... binaries-listed=0` |
| T-04-41 | Tampering | medium | mitigate | CLOSED | Path-from-reset test plus plan 04-15 listing step and `pack.words-*` (see T-04-43, T-04-45) |
| T-04-42 | Elevation of privilege | medium | mitigate | CLOSED | `docs/diagnostics.md:68` "Review rule: no routines, no services"; guards per T-04-44 |
| T-04-43 | Tampering | medium | mitigate | CLOSED | `listing` step; controls `listing-bits-mismatch`, `-header`, `-operand-count`, `-range`, `-row-mismatch`, `-unlisted`. `listing pass listings=2 listing-words=78`. Pack parser is bounded: `pack.c:103-122` (token cap 64), `:169-256` (line ≤1024, printable ASCII, 10-bit check, contiguity, window, ≤4096 words), `:413-417` (include ≤256 KiB) |
| T-04-44 | Elevation of privilege | medium | mitigate | CLOSED | Controls `bootstub-size`, `bootstub-row`, `bootstub-vector` |
| T-04-45 | Spoofing | medium | mitigate | CLOSED | Cases `pack.words-boot-stub`, `pack.words-diag-cpu-1` (`data/expected/tests/pack.ids:5-6`) |
| T-04-46 | Spoofing | medium | mitigate | CLOSED | `data/golden/diag-cpu-1.expect` hand derivation with page cites; shared-misreading risk stated in CL-0008 note (`data/claims.json:86`) |
| T-04-47 | Information disclosure | high | mitigate | CLOSED | `.ivsc` built only into `build/`; `.gitignore:16`; manifest lists only the listing; `files` and `manifests` pass |
| T-04-48 | Repudiation | medium | mitigate | CLOSED | CL-0008 `tier: original-diagnostic`, `standing: provisional`, scoped statement (`data/claims.json:80-86`) |
| T-04-49 | Tampering / Information disclosure | high | mitigate | CLOSED | `runner/scenario.c:160-191` (charset `[A-Za-z0-9._/-]`, ≤200, no leading `/`, no `..`, no empty segment); joined only under the content dir (`runner/main.c:74-78`, `hostio.c:70-82`); user images only from argv (`main.c:80-88`). Fault `RUNNER_DOTDOT_ACCEPTED` (`scenario.c:178`). Probe: all traversal forms refused |
| T-04-50 | Denial of service | medium | mitigate | CLOSED | `ivs_runner.h:82-88` caps; `scenario.c:797` (64 KiB), `:807` (512 lines), `:813-815` (256 bytes); steps and asserts caps at `:463`, `:557`. Fault `RUNNER_LINE_CAP_UNCHECKED`. Seeded fuzz (`tests/runner/runner_fuzz.c`) plus 600 auditor fuzz runs, clean |
| T-04-51 | Tampering | medium | mitigate | CLOSED | `scenario.c:810-829` refuses NUL, CR, non-ASCII and control bytes; echoed tokens are charset-restricted |
| T-04-52 | Information disclosure | medium | mitigate | CLOSED | Closed key table `runner/report.c` (`rp_label` `:76`, emit loop `:466-468`); case `runner.exec.no-paths`; probe report had no path text |
| T-04-53 | Repudiation | high | mitigate | CLOSED | Faults `RUNNER_ASSERT_ALWAYS_PASS` (`exec.c:374`), `RUNNER_STOP_EXPECT_IGNORED` (`exec.c:70`); pure outcome function `outcome.c` |
| T-04-54 | Spoofing | medium | mitigate | CLOSED | `runner/report.c:274-286`: label from `IVS_SCN_CONTENT_BOOT_STUB` kind only ("project boot stub, not an EXEC") |
| T-04-55 | Denial of service | medium | mitigate | CLOSED | `runner/hostio.c:84-96` (`open(O_NONBLOCK)`, `fstat`, `S_ISREG`), `:97-113` (read at most cap+1); wall cap `runner/exec.c:113-138`. Probe: FIFO, dir and `/dev/zero` refused, oversize gives OVERSIZED |
| T-04-56 | Tampering | medium | mitigate | CLOSED | `hostio.c:28-34` (handler sets one `volatile sig_atomic_t`), read between slices `exec.c:90-101`, `:113-138`. Fault `RUNNER_CANCEL_IGNORED` (`exec.c:92`) |
| T-04-57 | Repudiation | medium | mitigate | CLOSED | `exec.c:129-136` wall gives `TIMEOUT_WALL`; fault `RUNNER_WALL_AS_GUEST_FAIL` (`:131`); golden refuses `cancelled` and `wall` (`main.c:280-288`) |
| T-04-58 | Elevation of privilege | medium | mitigate | CLOSED | POSIX only in `hostio.c` (runner-scan); `gate-linux.sh` run recorded in 04-25-SUMMARY (`:155`, GNU, `steps=33/33`) |
| T-04-59 | Repudiation | high | mitigate | CLOSED | `runner-selftest pass cases=8`; case `runner-process.outcomes-covered`; exact scenario inventory (`tests-missing=0`) |
| T-04-60 | Denial of service | medium | mitigate | CLOSED | `tests/runner-process-tap.sh:98-121` `run_one` watchdog (sleep only bounds a broken runner, `:15`); `run_limited` in `tools/check/steps/c-faults.sh`; cancel via `--self-signal-after-slices` hook (`main.c:97-105`) |
| T-04-61 | Information disclosure | medium | mitigate | CLOSED | No golden for `needs-user-exec` (`data/golden/` holds only `diag-cpu-1`, `diag-input-pass`, `timeout-budget`); control `golden-user-scenario-tracked` |
| T-04-62 | Repudiation | high | mitigate | CLOSED | Every `.digests` has `# tier: deterministic-repeatability` / `# oracle: self-recorded`; controls `golden-header-missing-tier`, `-missing-oracle`, `golden-self-recorded-wrong-tier` |
| T-04-63 | Spoofing | medium | mitigate | CLOSED | `# domains:` with `none` in goldens; `main.c:214-231` (count 0 or `-` gives none); control `golden-empty-domain-observed` |
| T-04-64 | Tampering | medium | mitigate | CLOSED | Only `ivs-run --update-golden` writes (`main.c:293`, operator argv path only, `:366-367`); golden step compares only; control `golden-mismatch` |
| T-04-65 | Information disclosure | medium | mitigate | CLOSED | `main.c:461-466` refuses user roles before the run (probe: no file created); control `golden-user-scenario-tracked` |
| T-04-66 | Information disclosure | high | mitigate | CLOSED | `pkg-install.sh` exact inventory (`prefix-files=10`); selftest `extra-file`; `pkg-selftest=10/10` |
| T-04-67 | Information disclosure | medium | mitigate | CLOSED | `core/CMakeLists.txt:69` (no absolute path in export); `pkg-path-leak`; selftest `baked-path` |
| T-04-68 | Tampering | medium | mitigate | CLOSED | Selftests `missing-symbol`, `ghost-decl`, `unprefixed` in `pkg-install.sh`; `public-symbols=14 defined=55` |
| T-04-69 | Spoofing | low | mitigate | CLOSED | `SameMinorVersion` (`core/CMakeLists.txt`); selftest `version-refused` (`pkg-install.sh` refuse probes) |
| T-04-70 | Spoofing | high | mitigate | CLOSED | `pkg-install.sh:69` (mktemp outside repo), `:318` (unset path and package vars), `:565-566` (`NO_CMAKE_PACKAGE_REGISTRY` etc.), `ninja -t deps` presence (`:345`), `ivs_DIR` check (`:424`); selftest `leak-detector` |
| T-04-71 | Repudiation | medium | mitigate | CLOSED | Pinned consumer golden; selftest `golden-drift`; `consumer-lines=26` |
| T-04-72 | Information disclosure | medium | mitigate | CLOSED | `tests/installed-consumer/{main.c,main.cpp,CMakeLists.txt}` take images as argv; no `ivs_img_` or `IVS_IMG_` in the consumer |
| T-04-73 | Repudiation | high | mitigate | CLOSED | `important-gates pass gates=26 lines=102`; controls `important-gates-uncovered-gate`, `important-gates-unknown-id` |
| T-04-74 | Denial of service | low | mitigate | CLOSED | 04-24 gap fills by controls and selftests; measured in 04-25 (`CI-EFFICIENCY.md` close-out, 04-25-SUMMARY:14) |
| T-04-75 | Repudiation | medium | mitigate | CLOSED | 04-25-SUMMARY: fresh gate run (`:55`), evidence table (`:65`), honest caveats (`:118`), open items (`:144`) |
| T-04-76 | Tampering | low | mitigate | CLOSED | 04-25-SUMMARY `:87-100`: per-id script failed first on exactly C24, C25, D4, D6, D12 |
| T-04-77 | Elevation of privilege | low | transfer | CLOSED (transferred) | `.planning/AUTONOMY.md:47` "Branch protection on main (Phase 4 D-47, recommended, not applied by agents)" with ruleset and commands; 04-25-SUMMARY:15, `:42`, `:116` |
| T-04-78 | Denial of service | low | mitigate | CLOSED | D-45 evaluated on 23 hosted rows; re-baseline with triggers, no check dropped (04-25-SUMMARY:14, `:40`; gate counts unchanged `controls=276/276`, `faults=149/149`) |
| T-04-79 | Repudiation | medium | mitigate | CLOSED | Controls `golden-host-dependent-unmarked`, `golden-host-dependent-golden`, `golden-dev-release-diff`; `golden ... host-dependent=2`; `main.c:280-288` |
| T-04-SC | Tampering (supply chain) | low | accept | CLOSED (accepted) | Accepted in each plan's `<threat_model>` (for example 04-01:188, 04-04:140, 04-16:177). Confirmed: no `FetchContent`, `ExternalProject`, lockfile, manifest, vendored tree or third-party `find_package` added; workflow has one pinned action; libasm never wired in |

Summary: 79 numbered threats plus T-04-SC = 80 distinct ids; 80 closed, 0 open. The numbered threats are 21 high, 46 medium and 12 low. T-04-SC is low.

## Unregistered flags

| Flag | Classification | Detail |
|---|---|---|
| 04-21-SUMMARY `threat_flag: file-write` (`runner/main.c` `--update-golden`) | Informational, maps to T-04-64 and T-04-65 | `fopen(path, "w")` truncates an operator-chosen path from argv only, never from a scenario. It follows symlinks and has no `O_EXCL`, by design for an operator tool. It refuses user content and host-dependent outcomes |
| Auditor observation: symlinks in the content directory are followed | WARNING-level observation, non-blocking (no mapping) | `hostio.c:85` opens without `O_NOFOLLOW`. Probe: `c/lnk.ivsc -> /etc/passwd` was read and loaded (refused MALFORMED). A scenario cannot escape by path text (T-04-49 holds), but a symlink inside the operator's content dir (default: the scenario's own directory, `main.c:423-428`) can point the runner at any regular file the operator can read. Bytes go only to the load and digest, never echoed. Suggest documenting it in `docs/runner.md`, or `O_NOFOLLOW` on the final component |
| 04-LEVER-01 and 04-LEVER-02 summaries | Informational | No `## Threat Flags` section. Both are gate-performance refactors (batching, parallel presets) with no new input path. The gate and controls still pass |

All other SUMMARY `## Threat Flags` sections (04-01 to 04-25) read "None" or map to existing ids.

## Accepted risks log

| ID | Risk | Rationale | Revisit trigger |
|---|---|---|---|
| T-04-07 | Recorder output in `.planning/ci-runs.tsv` | Rows hold only date, run id, SHA, event, conclusion and numeric timings; no token, path or log text | Any new column taken from log text |
| T-04-SC | Supply chain via package installs | No dependency installed or vendored; one SHA-pinned action; libasm optional and never installed; the local-only `gate-linux.sh` container apt install stays under T-03-SC | Any dependency, FetchContent or vendored source; wiring gate-linux.sh into CI |
| T-04-77 (transfer) | Branch protection on `main` | Recorded as an owner/orchestrator action in `.planning/AUTONOMY.md:47`; agents do not apply repository settings | Owner applies or rejects the ruleset |

## Residual risks (non-blocking, below `block_on: high`)

1. **Content-dir symlinks are followed** (see the observation above). Operator trust boundary; low.
2. **`--update-golden` overwrites any operator-named file** (truncate, follows symlinks). Operator-only by design; low.
3. **Public-history scratch strings** in `c89aa48`, `6008847` and `345c48d` (listed under Hard constraints). They contain no username or home path and are absent from the tree; removing them needs a rewrite of public `main`, which is an owner decision. The branch adds none.
4. **`hygiene-history` scans `history-commits=1` locally.** This audit scanned every commit in `main..HEAD` and every commit on `main` independently.
5. **`gate-linux.sh` image pulled by tag, not digest** (carried over from Phase 3); Phase 4 now relies on it for T-04-58 hosted parity.
6. **`c-build-unlisted-source` has no control tree.** Only the emit site plus a quoted probe (as T-04-14 specifies); a regression in that loop would not be caught by `run-controls`.
7. **Two test skips** (`tests-skipped=2`). Accepted by `tests-inventory`, guarded by control `tests-inventory-unexpected-skip`.

**threats_open:** 0
