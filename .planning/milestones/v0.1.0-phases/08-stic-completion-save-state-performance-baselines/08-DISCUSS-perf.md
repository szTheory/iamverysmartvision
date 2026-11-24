# Phase 8 discuss, decision S-3: PRF-01 benchmark runner (research agent C)

Labels: [OBS] read or measured this session, [INF] inference, [REC] recommendation, [UNK] unknown,
[BG] background knowledge not re-verified. Read-only on the repo except this file. Measurements use a
scratch build under a scratch directory (release `-O2 -DNDEBUG -g`, and dev `-O1` ASan+UBSan) of the current
tree; nothing in the repo was built or changed.

## 1. Recommendation on one screen

- S3-1 [REC] Build **`ivs-bench`**, a new host executable in `tools/bench/` (about 400 lines of C,
  two TUs: `bench.c` ISO C, `bench_host.c` the only POSIX file). It links `ivs_runner_lib` and calls
  `ivs_runner_execute` in-process. Do **not** add timing flags to `ivs-run` and do not put a clock
  anywhere in the core or in `runner/` (the runner report and golden key list stay clock-free).
- S3-2 [REC] **Digest before throughput, structurally.** Iteration 0 of every workload is an untimed
  verification run: the report is captured in memory, filtered with the public
  `ivs_runner_golden_keep`, and compared line by line with the committed
  `data/golden/<workload>.digests`. Only on exact equality does the tool enter warmup and the timed
  loop; each timed iteration re-checks the composite digest line (cheap). On any mismatch it prints
  `bench-refuse <workload> digest-mismatch <key>` and exits non-zero with **no timing fields at all**.
- S3-3 [REC] Workloads are **ordinary IVSS scenarios** named `bench-*` registered in
  `data/expected/runner-scenarios.ids` with a `.digests` golden each. The existing `runner-process`
  suite (both presets) and the `golden` step then verify them in the gate for free, including the
  dev-vs-release judge. No new gate step, no new CI job.
- S3-4 [REC] Named workloads (all from the Lamp Tone fixture, so no new rights-bearing material):
  `bench-lamp-idle` (full machine, 120 frames), `bench-lamp-input` (same, 64 input events),
  `bench-lamp-cold` = existing `demo-lamp-tone-pass` (latency-dominated, 13 frames, carries the
  independently derived asserts), and `bench-lamp-snapshot` (idle + save/restore every 30 frames,
  digest must equal `bench-lamp-idle`) only after S-2 lands. One optional new original fixture
  `bench-cpu` (display off, tight ALU/memory loop) only if the planner has a spare plan. Do not label
  a Lamp Tone workload "cpu-bound": it is not (section 3).
- S3-5 [REC] Output is **TSV, one row per workload**, with three field groups that never merge:
  steady-state throughput (wall and CPU time), end-to-end latency (iteration 0 cold, and median), and
  a pointer-only `ci_time` that is NOT measured by the tool (CI time stays in `.planning/ci-runs.tsv`
  via `tools/ci-record.sh`, unchanged). Every row carries `build_id`, `optimized`, `sanitizers`,
  `iterations`, `warmup`, host `ncpu`.
- S3-6 [REC] **Never gate on absolute throughput.** The gate proves correctness (digests, refusal
  control). Throughput is a local record (`build/bench/*.tsv`, gitignored) plus a committed baseline
  file updated by a person. Optional local-only `--compare <baseline.tsv> --max-regress <pct>` on CPU
  time, never wired into `tools/gate.sh`.
- S3-7 [REC] **Negative control** in the existing control framework: a mutated core (an existing
  named-fault style mutation) or a mutated golden makes `ivs-bench` exit non-zero and print no timing.
  The control asserts the absence of any `ns` field in output.
- S3-8 [REC] **Do not optimize in Phase 8.** Profile shows ~43% of time in per-frame SHA-256 that is
  correctness machinery, and ~10% in CP-1610 decode; neither is a free win (section 5). The baseline
  record is the deliverable; any later optimization is justified by a regression against it.
- S3-9 [REC] No hyperfine, no Google Benchmark, no new dependency. The in-repo harness is smaller than
  the integration of an external one, and hyperfine cannot assert digests before timing anyway.
- S3-10 [REC] Cost: 2 plans (tool plus workloads plus negative control, then docs plus baseline
  record), about +1 to 2 s on a hosted gate that is already 96 to 131 s; no new CI step or job.

## 2. Existing assets (what can be a workload)

### 2.1 Scenarios and goldens [OBS]

| Scenario (data/scenarios) | Halts? | Machine time | Frames | Notes |
|---|---|---|---|---|
| `diag-ctrl-psg-pass` | yes | 16,752 ticks | 0 | process total 17 ms (floor of a shell spawn, see 5.1) |
| `diag-stic-fgbg-pass` | yes | 88,476 ticks | 1 | halts at about 1.5 frames |
| `diag-boot-pass`, `diag-input-pass` | yes | under 20,000 ticks | 0 | |
| `demo-lamp-tone-pass` | **no** (idles forever) | 806,568 ticks | 13 | 6 input events, 55 asserts, independent derivation |
| `demo-lamp-tone-idle/-shift/-swap` | no | 806,568 ticks | 13 | metamorphic variants |

- [OBS] Only the Lamp Tone program never halts, so it is the only fixture that can be run for an
  arbitrary length. Every diagnostic halts within 1.5 frames of machine time.
- [OBS] Goldens: `data/golden/*.digests` hold the report filtered to the golden key list (outcome,
  slot sha256s, step lines, nine `digest.*` lines, output counts, `video.chain.check`). A golden is
  tier `deterministic-repeatability`, oracle `self-recorded`: it proves a later run repeats the
  recorded one, not that the run is right (`docs/runner.md` section Goldens).
- [OBS] `tools/check/steps/golden.sh` accepts a `.digests` golden for any scenario listed in
  `data/expected/runner-scenarios.ids` (class deterministic, no `content ... user`), with no
  `.expect` required (precedent: `timeout-budget.digests`). It compares the golden to the dev report
  and a dev-vs-release judge compares presets.
- [OBS] `ivs_runner_execute(scn, host, opts, result)` is a library function
  (`runner/ivs_runner.h`); the host vtable has `write_report`, `read_file`, `now_ms`,
  `cancel_requested`, host-owned storage and buffers. `ivs_runner_golden_keep(line, n, live)` is public
  in the same header. So an in-process bench needs no new library API.
- [OBS] The report has no wall time or throughput field; `host.calls` is the only host-dependent key
  and is excluded from the golden. `ivs_hostio_now_ms` (runner/hostio.c) is millisecond resolution,
  used only for the wall cap.

### 2.2 Installed consumer [OBS]

`tests/installed-consumer/main.c` replays four scenarios through the installed headers alone and
compares digests with the goldens. [REC] Keep it a correctness proof; do not make it a benchmark
(it is the package-contents proof; adding timing there couples packaging to host timing). It already
shows the right pattern for a no-runner host: sized buffers from `ivs_info`, digests per domain.
[INF] If REL-01 wants "the benchmark runs against the installed package", that is a later option
(`ivs-bench` could be built against the install prefix like the consumer); not needed for PRF-01.

### 2.3 Proposed named workloads

| Name | Purpose (what it stresses) | Source | Frames | Status |
|---|---|---|---|---|
| `bench-lamp-idle` | whole machine: CPU idle loop, STIC frame render and per-frame hash chain, PSG tone/mix, audio sample emit | Lamp Tone, no input | 120 | needs scenario + golden |
| `bench-lamp-input` | input scheduling path and PSG write bursts | Lamp Tone, 64 input events (the scenario cap), alternating the six documented words | 120 | needs scenario + golden |
| `bench-lamp-cold` | start-up and load latency, cold caches, short run | existing `demo-lamp-tone-pass` | 13 | exists; its golden and independent asserts already in the gate |
| `bench-lamp-snapshot` | snapshot/restore every 30 frames, digest equals `bench-lamp-idle` | S-2 API | 120 | blocked on S-2; list as `pending` in the inventory so it is visible |
| `bench-cpu` (optional) | CPU-bound: display off, tight loop, no PSG | new original fixture via `ivs_asm` | 120 | only if a plan slot exists; needs fixture manifest, listing, rights row |

- [INF] "audio-bound" and "video-frame-bound" are not separable with the current fixtures: audio
  samples (one per 16 ticks, 2.24 M samples per 600 frames) and frame rendering run on every frame
  of the same program. Honest names (`full machine idle`, `input`, `cold`, `snapshot`) beat claiming
  bound-ness that the profile does not show.
- [REC] Length 120 frames (7,168,320 ticks, about 2 s of guest time). Measured: the dev ASan build
  takes about 1.3 to 1.8 s for 600 frames under load 65 [OBS], so 120 frames is about 0.3 s in the
  gate per preset, acceptable; in release about 50 ms per iteration, long enough for a ns timer.
- [REC] The input workload's first six events equal the six already independently derived in
  `demo-lamp-tone-pass` so its prefix behaviour is anchored; the extra 58 events are self-recorded
  only. Label the tier honestly in the workload header: correctness anchor = repeatability digest,
  plus the prefix anchor where it applies.

## 3. Runner design

### 3.1 Where it lives

| Option | Verdict |
|---|---|
| Shell wrapper around `ivs-run` | Rejected [INF]: POSIX sh has no sub-second monotonic clock without perl/python or `date +%N` (not on macOS); process spawn cost (about 17 to 25 ms measured, 5.1) swamps a 50 ms run |
| `--bench` flag on `ivs-run` | Rejected [REC]: puts timing next to the report that goldens read; one process runs one scenario by design (`docs/runner.md`); widens the usage text and the `runner-process` control surface |
| **`tools/bench/` executable, in-process via `ivs_runner_execute`** | Chosen [REC]: mirrors `tools/pack` and `tools/asm` (host tools, not shipped, built in both presets); the only POSIX file is `bench_host.c` |
| Run under the installed consumer | Rejected for PRF-01 [REC] (2.2) |

Constraints it must satisfy [OBS]: `tools/check/core-symbols-deny.txt` lists `clock_gettime`,
`mach_absolute_time`, etc. as forbidden imports **of the core**; the runner and tools may use them
([OBS] only `runner/hostio.c` does today). The runner library already links only the public core
(`target_link_libraries(ivs_runner_lib PUBLIC ivs_core ...)`).

### 3.2 Order of operations (the contract)

1. Parse `bench-*.ivss`; read `data/golden/<id>.digests` (ignore `#` lines).
2. **Verification run** (untimed, iteration 0): execute with a `write_report` that appends to a memory
   buffer; keep lines passing `ivs_runner_golden_keep`; compare with the golden body.
   Mismatch: `bench-refuse`, exit 15 (new, distinct from the runner's 10 to 14), print **no**
   throughput or latency field, only the first differing key and both values.
3. Warmup: W untimed iterations (fresh instance each, same buffers).
4. Timed: N iterations, each: `t0 = mono_ns(); execute(); t1 = mono_ns();`, plus
   `getrusage(RUSAGE_SELF)` deltas for CPU time around the whole loop and per-iteration where the
   resolution allows. After each iteration compare `digest.composite` and `outcome` (a 64-hex compare;
   the timing excludes the compare).
5. Aggregate (median, min, max over iterations; never the mean) and print one TSV row.

- [REC] W and N are **fixed per workload in the workload table** (`data/bench/workloads.tsv`: id,
  warmup, iterations, optional note), never time-budgeted: a wall-budgeted loop makes the number of
  executed iterations (and so the output) host-dependent and was the classic source of
  non-reproducible reports. Suggested W=3, N=15 for the 120-frame workloads, W=2, N=30 for the 13-frame
  cold one. [INF] 15 samples is enough for a median and a min/max spread; it is not a significance test.
- [REC] Fresh instance per iteration (the library already creates one per execute): the measured
  time then includes create+load+reset+run+report, which is the honest "scenario end-to-end" cost.
  Steady-state throughput is computed from the run length and the median.
- [REC] Optimization barrier: the digest compare reads output of every iteration, and the run's
  output is hashed by the core itself (SHA-256 chain of all frames), so the compiler cannot drop work
  the digest depends on. See 6.2.

### 3.3 Metrics

| Field | Definition | Group |
|---|---|---|
| `ticks_per_iter`, `cpu_cycles_per_iter`, `frames_per_iter` | from the report (`step.k.now_ticks`, `ticks_per_cpu_cycle` = 4, nominal frame 59,736 ticks); deterministic | static |
| `thr_cycles_per_s_wall` | `cpu_cycles_per_iter / median_wall_s` | throughput |
| `thr_cycles_per_s_cpu` | `cpu_cycles_per_iter / median_cpu_s` | throughput |
| `thr_frames_per_s`, `realtime_x` | frames per second, and ratio to the 59.92 Hz profile rate [INF: 14,934 x 4 = 59,736 ticks at 3,579,545 Hz] | throughput |
| `ns_per_guest_cycle` | `median_wall_ns / cpu_cycles_per_iter` | throughput |
| `lat_first_ms` | wall time of the first timed iteration of a fresh process (cold) = create+load+reset+run+report | latency |
| `lat_median_ms`, `lat_max_ms` | per-iteration wall over N | latency |
| `proc_ms` (optional, `--spawn`) | process-start-to-result of the real `ivs-run`; only if the planner wants it (3.5) | latency |
| `ci_time` | column present, value `see .planning/ci-runs.tsv`; the tool never measures CI | CI |
| `build_id`, `optimized`, `sanitizers`, `ncpu`, `iterations`, `warmup` | provenance of the number | meta |

- [REC] Drop "ns per instruction". [OBS] The only instruction-ish counter is
  `ivs_run_report.units_executed` (per `ivs_machine_run` call); it is not exposed through
  `ivs_runner_execute` and [UNK] whether a "unit" equals one instruction (bus-steal chunks may be
  units). CPU cycles are the public deterministic work measure; use `ns_per_guest_cycle`. If the owner
  wants ns/instruction, it needs a documented instruction counter first (a core change, out of scope).
- [REC] Report **both** wall and CPU time. [OBS] Same binary, same workload, one minute apart on this
  laptop (load average 65 to 112 on 18 cores): wall 328 to 1,378 ms for 600 frames; user CPU time
  0.25 s in 5 of 5 runs. CPU time is about 4 to 5 times more load-stable than wall time here. Use CPU
  time for trend and regression decisions, wall time for latency.

### 3.4 Output format

[REC] TSV, header line, one row per workload, columns as in 3.3, plus a leading `bench-format 1`
line in the style of `ivsr 3`. TSV over JSON because every existing record (`ci-runs.tsv`, goldens,
`.ids`) is line-oriented and the gate toolchain is awk/sh; no JSON parser dependency.
[REC] Refusal rows use the same file but carry `status=refused` and empty numeric columns (not zero:
an empty field cannot be confused with a measurement).

### 3.5 End-to-end latency, defined

[OBS] Requirement text: "throughput, end-to-end latency and CI time are reported separately".
[INF] Three possible meanings; pick (a) and keep (b) as already covered:
a. **Wall latency of one scenario**, load to report (`lat_first_ms`, `lat_median_ms`). Chosen.
b. Machine-time input-to-output latency (guest frames between an input event and the frame that shows
   it). Deterministic, machine time; already asserted by the Lamp Tone derivation (events placed at
   least 16,800 ticks from frame boundaries). Not a benchmark; do not duplicate.
c. Process spawn to result (`ivs-run` exec, dyld, content read). [UNK] value to the owner; measured
   at 17 to 25 ms including a `sh -c` hop, indistinguishable from `/usr/bin/true` (21 ms) on a loaded
   host. [REC] defer; if wanted, an optional `--spawn <ivs-run>` using `posix_spawn` in `bench_host.c`.

## 4. Recording, noise, and the CI-cost arbiter

### 4.1 What the CI record looks like today [OBS]

- `.planning/ci-runs.tsv` columns: `date run_id sha event conclusion queue_s setup_s build_s test_s
  gate_s total_s ncpu cpu_s`, appended by the orchestrator via `sh tools/ci-record.sh <run-id>`
  (never by the gate or agents; the GitHub API rate limit is shared, AUTONOMY section 4).
- Recent hosted runs (2026-11-13/14): gate_s 94 to 154, build_s 36 to 60, test_s 51 to 86, total_s
  94 to 154 (4 CPUs). The gate is already the dominant cost; the earlier "16 s" era is gone.
- `tools/gate.sh` emits `step-time <name> <s>` for every step; `ci-record.sh` extracts only
  `c-build`; test_s = gate_s - build_s.

### 4.2 Recommendation

| Concern | Decision [REC] |
|---|---|
| Does the benchmark run in the gate? | **No.** Throughput measurement (about 15 iterations per workload, about 1 to 2 s per workload in release) is local/optional: `build/release/tools/bench/ivs-bench` run by a person or by the orchestrator at phase close |
| What runs in the gate? | Only what is already there: the new `bench-*` scenarios are executed and golden-compared by `runner-process` and `golden` in both presets (about +0.3 s per workload per preset, about +1 to 2 s total) |
| Does the tool itself need a gate test? | One CTest case `bench-selftest` in the existing `tests/CMakeLists.txt` pattern (like `runner-process`): runs `ivs-bench --verify-only` on every workload (iterations 0, warmup 0, so no timing) and the negative control (5.3). About 1 s in the dev build. Register in `data/expected/tests/*.ids` |
| Where do results go? | Local: `build/bench/<date>-<sha>.tsv` (build/ is gitignored). Committed: `data/bench/baseline-<host-class>.tsv` updated only by a person at milestone close with the host (`uname -m`, `ncpu`, build id) in the header; it is a record, never an assertion |
| Hosted trend | Optional, later: if the owner wants a hosted trend, add **one** non-gating line to `gate.sh` output (`bench-time ...`) only when a distinct recurring failure class justifies it. Not recommended now: hosted runners are shared VMs, absolute throughput there is noise (see 4.3) |
| CI time field | The tool prints `ci_time=see-ci-runs.tsv`. CI time remains the output of `ci-record.sh`; the bench never calls `gh`, never touches `.planning/ci-runs.tsv` or `CI-EFFICIENCY.md` |
| CI-efficiency review | Per AUTONOMY section 4 the next review should record the measured delta of adding the bench scenarios (expected +1 to 2 s on a 96 to 131 s gate); append to `CI-EFFICIENCY.md` by the orchestrator as usual |

### 4.3 Noise [OBS]

Measured on this laptop while other work loaded it (uptime load averages 62 to 112 on 18 cores):

| Run (release, 600-frame Lamp Tone idle) | Wall |
|---|---|
| first sample | min 327.7, median 459.8, max 539.5 ms |
| a minute later | min 1,378, median 1,472, max 2,829 ms |
| user CPU time, 5 runs | 0.25 s every time |
| `/usr/bin/true` via `sh -c`, 15 runs | min 20.9, median 25.7, max 64.6 ms |

[INF] A threshold on wall throughput would have failed spuriously by a factor of 4 on this host
within minutes. [REC] statistic rules: median of N, report min and max, compare CPU time not wall,
and compare only to a baseline from the same host class and build id; a regression threshold, if any,
is a loose local-only ratio (for example 25 percent on median CPU time) that prints a warning and
exits non-zero only under `--max-regress`.

## 5. Profiling findings (measured, release `-O2 -g`, macOS arm64, 6,000-frame Lamp Tone idle)

### 5.1 Throughput numbers (indicative only, host loaded) [OBS]

- 600 frames (35,841,632 ticks, 8,960,408 CPU cycles): user CPU 0.25 s, so about 2,400 frames/s,
  about 40x real time, about 36 M guest cycles per CPU-second, about 28 ns per guest cycle.
  Lowest wall observed 328 ms (27 M cycles/s).
- 6,000 frames: wall about 5.5 to 6 s under load 64 (about 1,000 frames/s). Not linear against the
  600-frame figure, i.e. wall time here is dominated by scheduler load, not by the workload.
- Short existing scenarios (13 frames or less) take 17 to 30 ms per process, equal to the cost of
  spawning `sh` and `true`. They cannot measure throughput; they measure process start. This is why
  the benchmark needs longer, fixed-length variants.
- Digest equality across builds [OBS]: the 600-frame run gave identical `digest.composite`
  (`fc0ef83e...`), `digest.video` and `digest.audio` in the dev ASan `-O1` build and the release `-O2`
  build, so one golden serves both presets; the existing dev-vs-release judge enforces this in the gate.

### 5.2 Hotspots (macOS `sample`, 463 samples at 1 ms) [OBS]

| Share of samples | Function | What it is |
|---|---|---|
| about 26% (120) | `ivs_rsha256_compress` | the runner's own SHA-256 (independent recompute of the video chain, `runner_vchain.c`) |
| about 17% (77) | `ivs_sha256_compress` via `ivs_stic_frame_deliver` (stic_frame.c:200) | the core's per-frame video chain hash |
| about 10% (46) | `ivs_cp1610_decode` | decode (cp1610_decode.c:136-147) |
| about 6% each | `ivs_psg_catch_up` (29), `ivs_machine_step_unit_trace_internal` (23) | |
| rest | `ivs_stic_lines`, `ivs_cp1610_step`, `ivs_step_running`, PSG mix and tone, memsys, bus | each 1 to 3% |

- [OBS] About 43% of the time is SHA-256 over every frame, twice (core chain plus the runner's
  independent chain check). That is the design of the video evidence path (`video.chain.check pass`,
  `digest.video`), not an accident.
- [INF] The throughput a host sees is "verified-run throughput". A host that does not need the chain
  would be about 1.7x faster, but there is no switch to turn the chain off and none should be added
  for a benchmark: the benchmark must measure the code the digests vouch for.
- [REC] Cheap optimization candidates, each **not** recommended in Phase 8:
  - Hash chain: could skip the runner recompute for a benchmark; that removes the independent check the
    digest assertion relies on. Reject.
  - Decode table: about 10% of samples; a table-driven decode could save a few percent. The decode is
    covered by an exhaustive CPU vector sweep; a rewrite risks regressions for a small gain. Defer.
  - Compiler: `-O2` today; `-O3` or LTO (`INTERPROCEDURAL_OPTIMIZATION OFF` in runner/CMakeLists.txt)
    not tried. Do not tune; flags are part of `build_id` in the baseline.
  Per the North Star the baseline record is what Phase 8 owes. Optimize later against it.

## 6. Adversarial pass

### 6.1 Ways the benchmark reports fast but wrong

| Failure | Mitigation [REC] |
|---|---|
| Throughput printed before or without the digest check | Structural order in 3.2; refusal prints no `ns` or rate field; negative control asserts that absence |
| Digest compared only on iteration 0, later iterations broken (nondeterminism, state leak) | Per-iteration composite and outcome compare; the golden covers all nine domains |
| Workload quietly ends early (timeout, guest halt, refusal) and finishes "fast" | Golden includes `outcome`, `step.k.now_ticks`, `cpu.run_state`, `output.video.frames`; frames and ticks per iteration are printed and the tool refuses if `ticks_per_iter` differs from the workload table |
| Golden regenerated from a broken core ("self-recorded" tautology) | Golden changes are a reviewed diff (existing rule, `docs/runner.md`); the prefix of `bench-lamp-input` is anchored to the hand-derived asserts; the tier is printed in the row as `repeatability-only`; no claim, no `important-gates` row |
| Dead-code elimination: the run result unused | The digest of every iteration is compared (a real data dependency), and the work is inside `ivs_machine_run` behind a library boundary in a different TU; the core hashes its own output |
| Workload drift (scenario or fixture edited) | `slot.*.sha256` and `step.*` lines are in the golden, so an edited cartridge or bound changes the golden and the gate fails until a person reviews the diff; the bench also prints the golden's file sha256 in the row |
| Debug vs release comparison | Row carries `build_id` (compiler id and version, flags hash from CMake), `optimized`, `sanitizers`; `--compare` refuses a baseline with a different `build_id` or `optimized`; by default the tool prints `warn=unoptimized` and does not write to the committed baseline file when `optimized=0` or `sanitizers=on`. [OBS] dev and release produce the same digests, so the digest cannot tell them apart; the provenance fields must |
| Iteration count depends on time | Fixed per workload (3.2); the count is in the row |
| Compiler or CPU differences move the number | Baselines are keyed by host class; cross-host comparison is not made; macOS arm64 dev laptop and hosted ubuntu x86_64 are never compared |
| First-iteration effects (page faults, cold caches) hidden in a median | `lat_first_ms` is reported separately from the median; warmup excluded from steady state, included in none |
| Timer/clock leaks into machine behaviour | The clock lives only in `bench_host.c`; the machine never sees it; `core-symbols` import check and the runner's "no clock in the report" rule are untouched. The bench also asserts that two runs differing only in timing produce identical digests (they must, by construction) |
| CPU-time resolution on a short iteration | Compute CPU time over the whole loop divided by N (getrusage resolution is microseconds), per-iteration CPU only informational |
| Throughput reported from machine ticks that include overshoot | Use the report's actual `now_ticks` (e.g. 35,841,632), not the nominal target |

### 6.2 Is a benchmark in the gate a regression risk to the evidence rules?

- [INF] No: the bench adds no clock to the core or runner, no new golden semantics, and no new claim.
  The throughput numbers carry no evidence tier (they are measurements of this implementation on one
  host, not hardware claims); state that in `docs/` and never add them to `data/claims.json`.
- [REC] A `bench-*` scenario must not carry `# tier:` text that implies independent oracle: tier
  `deterministic-repeatability`, oracle `self-recorded`, exactly as the existing `.digests` rule
  enforces.

### 6.3 Scope and plan count (for the planner)

| Plan | Content | Size |
|---|---|---|
| P-A | `tools/bench/` (bench.c, bench_host.c, CMake, listing in the sources inventory like `ivs-runner-sources.txt`), `data/bench/workloads.tsv`, `bench-lamp-idle`, `bench-lamp-input` scenarios + goldens (`--update-golden`), registration in `runner-scenarios.ids`, CTest `bench-selftest` with the negative control and its fault/control inventory rows | about 10 files, medium |
| P-B | `docs/bench.md` (definitions, field table, how to run, how to read, "not evidence" statement), baseline record `data/bench/baseline-*.tsv` for this host, `bench-lamp-cold` registration (reuses an existing scenario), `bench-lamp-snapshot` row once S-2 landed (or `pending` list entry), STATE/ROADMAP/REQUIREMENTS PRF-01 update; first-baseline measurement run on an idle host | small |
| Optional | `FX-bench-cpu` fixture and `bench-cpu` scenario | +1 small plan |

- [INF] Dependencies: P-A needs nothing from S-1 (STIC completion). `bench-lamp-snapshot` needs S-2's
  snapshot API; if S-2 slips, ship PRF-01 without it and list it as `pending` (visible, not silent).
  If S-1 changes STIC rendering, the goldens of all `bench-*` change by design: **land P-A after S-1
  and S-2's behaviour-affecting plans**, or regenerate goldens in one reviewed commit. Ordering [REC]:
  bench plans last but before the milestone-close plan.
- [INF] CI-cost impact: +1 to 2 s on the hosted gate (new scenarios x 2 presets), +1 s for
  `bench-selftest`; no new job, cache, matrix leg or path filter. Within the current CI-efficiency
  decisions; record the delta in `CI-EFFICIENCY.md` after the first hosted run.

## 7. Dependency taste: hyperfine and others

- [OBS] CLAUDE.md research (dated 2026-11-09) lists hyperfine v2.0.0 as an optional tool "for
  end-to-end runner timing; do not use as the correctness oracle"; [OBS] it is not installed here.
- [REC] No hyperfine. It times a command, so it cannot assert a digest before reporting (the PRF-01
  requirement), it adds process-spawn and shell noise (the 17 to 25 ms floor measured above), and it
  is a Rust binary outside the dependency policy. A developer who wants a quick ad-hoc comparison may
  still use it locally; nothing in the repo depends on it. This agrees with "another copy-paste is
  better than another dependency".
- [REC] No Google Benchmark (C++ only, already listed under "What NOT to Use"), no Unity timing.
- [REC] Copy-paste inventory: `bench_host.c` repeats the two POSIX calls of `runner/hostio.c`
  (`clock_gettime(CLOCK_MONOTONIC)` at ns resolution, `getrusage`); a few lines duplicated beats a
  new shared header between runner and tool.

## 8. Unknowns and what would change the recommendation

- [UNK] Whether the golden step lint (`tools/check/steps/golden.sh`) or other inventories (scenario
  pairs, `runner.ids` fault ids, `tests/*.ids`) impose extra rows on a scenario without an `.expect`.
  Precedent `timeout-budget` suggests not; the planner should read `golden.sh` and
  `data/expected/runner-scenarios.ids` before writing P-A.
- [UNK] Exact behaviour of `ivs_runner_execute` with a host storage that is reused across iterations
  (the executor creates the instance in host storage "aligned by hand"). [INF] it re-creates state each
  call; verify with the per-iteration digest compare, which is why that compare exists.
- [UNK] A true idle-host baseline: this session's host was at load 62 to 112, so the numbers in
  section 5 are indicative only. The first committed baseline must be taken on an idle host.
- [UNK] Whether the owner wants hosted-runner throughput trends at all. Reverse S3-6 only if a
  recurring perf regression is observed that CPU-time local runs missed.
- Would change the recommendation: if S-1/S-2 add per-frame or per-snapshot cost that makes 120-frame
  scenarios too slow for the dev ASan gate (measure first, shorten to 60 frames); if the owner wants
  the benchmark to run against the installed package (then build `ivs-bench` as a second installed
  consumer).
