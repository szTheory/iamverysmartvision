# The benchmark tool (`ivs-bench`)

`ivs-bench` (`tools/bench`, PRF-01) executes named workloads on the machine core through the headless runner library and reports how long they take, in three separate groups: throughput, latency and CI time.
It is a development tool: it is built in both presets, never installed and never shipped.
A figure it prints is a measurement of this implementation on one host with one build. It is never evidence and never a claim [nc], and the gate never reads one (sections "What a figure is not" and "The gate").

## Definitions

- **Workload**: one row of `data/bench/workloads.tsv`: an id, the scenario it executes, the fixed warmup and iteration counts, the round-trip cadence (0 for none), the final machine time `ticks` the run must end at, and a note.
  The scenario is an ordinary IVSS scenario of `data/scenarios`, listed in `data/expected/runner-scenarios.ids`, with a repeatability golden `data/golden/<scenario>.digests`.
- **Verification iteration** (iteration 0): one untimed execution whose report is filtered with the runner's golden filter and compared with the committed golden, and whose final machine time is compared with the `ticks` column. No timer is read before it has matched.
- **Warmup iteration**: an untimed execution after the verification iteration. Its outcome, composite digest and step end ticks are compared with those of iteration 0.
- **Timed iteration**: an execution bracketed by two reads of the monotonic time source around the call that executes the scenario, and nothing else. One iteration covers creating the instance, loading the images, the reset, the run and the report. The comparison with iteration 0 happens after the second read.
- **Fixed counts**: the numbers of warmup and timed iterations are read from the table and never from a time budget, so the number of executions never depends on the host.
- **Median**: the median of the per-iteration wall times (the mean of the two middle values when the count is even), never the mean.

## The output

Standard output is TSV: the line `bench-format 1`, one header line, one row per workload in table order, 22 tab-separated columns. The groups never merge.

| Column | Group | Unit | How it is computed |
|---|---|---|---|
| `workload` | provenance and static | | the id of the table row |
| `status` | provenance and static | | `verified` (a `--verify-only` run) [nc], `measured` (a timed run) or `refused` |
| `scenario` | provenance and static | | the scenario id of the row; its golden is `data/golden/<scenario>.digests` |
| `tier` | provenance and static | | always `repeatability-only` (see "What a figure is not") |
| `build_id` | provenance and static | | compiler id and version, build type and 12 hex digits of a digest of the compile flags of that build type, from `tools/bench/CMakeLists.txt` |
| `optimized` | provenance and static | 0 or 1 | 1 only for the build types Release, RelWithDebInfo and MinSizeRel |
| `sanitizers` | provenance and static | `on` or `off` | `on` when the preset builds with AddressSanitizer and UndefinedBehaviorSanitizer |
| `ncpu` | provenance and static | count | the online processors of the host (a property of the host, used in no figure) |
| `warmup`, `iterations` | provenance and static | count | the fixed counts of the table row |
| `ticks_per_iter` | provenance and static | master ticks | the final machine time of the run, equal to the `ticks` column and to the last `step.<k>.now_ticks` of the golden |
| `cpu_cycles_per_iter` | provenance and static | CPU cycles | `ticks_per_iter` divided by the ticks per CPU cycle of the profile, read from the core |
| `frames_per_iter` | provenance and static | frames | `output.video.frames` of the report |
| `thr_cycles_per_s_wall` | throughput | CPU cycles per second | `cpu_cycles_per_iter` over the median wall time |
| `thr_cycles_per_s_cpu` | throughput | CPU cycles per second | `cpu_cycles_per_iter` over the process CPU time of the whole timed loop divided by `iterations` |
| `thr_frames_per_s` | throughput | frames per second | `frames_per_iter` over the median wall time |
| `realtime_x` | throughput | ratio | the guest time of the run (`ticks_per_iter` over the crystal frequency of the profile) over the median wall time |
| `ns_per_guest_cycle` | throughput | nanoseconds | the median wall time in nanoseconds over `cpu_cycles_per_iter` |
| `lat_first_ms` | latency | milliseconds | the wall time of the first timed iteration |
| `lat_median_ms` | latency | milliseconds | the median wall time |
| `lat_max_ms` | latency | milliseconds | the largest wall time |
| `ci_time` | CI time | | the pointer text `see-ci-runs.tsv` [nc]; CI time is never measured here |

A `verified` row [nc] fills the provenance and static group and leaves the throughput and latency columns empty. A `refused` row fills `workload` to `sanitizers` and leaves `ncpu` to `lat_max_ms` empty: an empty field cannot be mistaken for a measurement, a zero could be.

## Running it

From `<repo-root>`, in the release preset (the images the scenarios name are build products, so the whole tree is built once):

```
cmake --preset release
cmake --build --preset release
build/release/tools/bench/ivs-bench --root . --content-dir build/release/images
```

| Option | Meaning |
|---|---|
| `--content-dir <dir>` | required: the image directory of a build tree, `build/<preset>/images` |
| `--root <dir>` | the repository root, default the current directory; the table, the scenarios and the goldens resolve from it |
| `--workloads <file>` | another workload table, default `data/bench/workloads.tsv` under the root |
| `--golden-dir <dir>` | another directory of `.digests` goldens, default `data/golden` under the root |
| `--workload <id>` | execute only this workload; repeatable |
| `--verify-only` | the verification iteration only: no warmup, no timed iteration, no timer read |

Exit 0: every workload was verified [nc] or measured. Exit 14: a usage error or a harness failure (one line `ivs-bench: ...` on stderr; for a bad option or an unreadable input nothing at all on stdout). Exit 15: a workload was refused.

## Refusal

A workload whose run differs from its golden prints no figure. The tool writes `bench-refuse <workload> <reason> <detail>` on stderr, a `refused` row on stdout and exits 15:

- `digest-mismatch <key>`: the first key of the filtered report that differs from the golden (a changed core, scenario, image or golden);
- `ticks-mismatch table=<n> run=<m>`: the run ends at another machine time than the table says;
- `run-mismatch warmup=<i> <key>` or `run-mismatch iteration=<i> <key>`: a warmup or timed iteration differs from iteration 0 in its outcome, composite digest or step end ticks (the key names the first difference).

The refusal comes after the verification iteration and before any timer read, so no figure of a wrong run can reach the output. The suite `bench` shows it on a mutated golden and a wrong `ticks` column, and shows that the check which asserts the absence of a figure can fail.

## Reading a figure

- Use CPU time (`thr_cycles_per_s_cpu`) for a trend or a regression, and wall time (`lat_*`, `thr_cycles_per_s_wall`) for latency. On a loaded host wall time moved by a factor of four within minutes while CPU time did not move (measured once during the planning of this phase, so indicative only).
- `lat_first_ms` is the first TIMED iteration. The process has already executed the verification iteration and the warmup iterations when it is taken, so code and caches are warm: it is not a cold-start figure of a process, and the tool measures none. The `bench-lamp-cold` workload is a short run (13 frames, so start-up and load are a large share of it) taken in a warmed process.
- Every iteration is a verified run [nc]: the core hashes every frame (the video chain) and the runner hashes every frame again as an independent check, because the digests vouch for the code that was timed. About 43 percent of the time of such a run is SHA-256 (measured once on a loaded host during the planning of this phase, so indicative only). There is no switch that removes it, and none should be added for a benchmark. The figures are therefore not a pure CPU-emulation rate: they are the rate of the machine as implemented, for verified runs.
- A figure is comparable only with a figure of the same host class, the same `build_id`, the same workload row and the same counts. A figure from a build with `optimized` 0 or `sanitizers` on is never a baseline, and a host that was busy is not an idle host.
- Fifteen samples give a median and a spread. They are not a significance test.

## The workloads

No workload is labelled CPU-bound. The audio samples (one per 16 master ticks), the frame rendering and the per-frame chain run on every frame of the same program, so no fixture here separates them, and a name that claimed a bound would claim what the profile does not show. The names say what is executed.

| Workload | Scenario | What it executes |
|---|---|---|
| `bench-lamp-idle` | `bench-lamp-idle` | the Lamp Tone cartridge for 120 frames with no input: the whole machine, idle display |
| `bench-lamp-input` | `bench-lamp-input` | the same run with the 64 input events of the scenario cap |
| `bench-lamp-cold` | `demo-lamp-tone-pass` | the existing 13-frame scenario: start-up and load |
| `bench-lamp-snapshot` | `bench-lamp-idle` | the idle scenario with a save, a reset and a restore every 30 frames |

**The snapshot workload.** `bench-lamp-snapshot` is a row of the table and not a scenario of its own (D-39). Its `roundtrip_ticks` column is 1,792,080 (30 frames of 59,736 ticks), which the tool hands to every execution of the workload through the round-trip option of the runner: at the first unit boundary at or after each multiple of the cadence the executor saves a snapshot, resets the machine to its power-on state and restores the snapshot just written. A 120-frame run has three cuts (30, 60 and 90 frames; none is made at the end bound). A run in which the hook made no cut is a harness failure of the tool, so the row cannot silently measure no snapshot. Its digest precondition is the SAME golden as the idle workload: a run with the three round trips must produce the report of the uninterrupted run, so the verification iteration proves each restore equal before any figure is printed. The figures include the cost of the three round trips (a snapshot is a few kilobytes of memory, no file is written), and the difference from the `bench-lamp-idle` row is what they cost, to the precision of the noise and no better.

## What a figure is not

- A figure is not evidence and not a claim [nc]. The `tier` column says `repeatability-only`: it labels the row, it is not one of the evidence tiers, and no row of `data/claims.json` mentions a throughput. A figure says how long this implementation took to execute one workload on one host with one build. It says nothing about the hardware, about the accuracy of any behaviour, or about any other host.
- The goldens the digests are compared with are self-recorded repeatability goldens (tier `deterministic-repeatability`), so a matching digest shows that the run equals an earlier run of this implementation, not that the run is right.
- No figure is a result of a measurement on hardware. The decisions D10 (hardware measurement) and D11 (a run with an owner-supplied EXEC or GROM image) are not performed and are not touched by this tool.

## The gate

The gate never reads a timer for the benchmark and never compares a throughput, a latency or a baseline with a threshold. It executes the tool only with `--verify-only` (the suite `bench`, `tests/bench-tap.sh`), and executes the benchmark scenarios through the runner steps like any other scenario. The monotonic time source and the process CPU time are read in one file only, `tools/bench/bench_host.c`; the core, the runner library and `ivs-run` have none.

CI time is a third group, never measured here: the hosted run times are recorded by the orchestrator in `.planning/ci-runs.tsv` [nc] (and read in `.planning/CI-EFFICIENCY.md`), and the tool never calls any service and never writes under `.planning`.

## The baseline

A baseline is a PERSON-WRITTEN record of one run of the tool, taken on an idle host in the release preset with no sanitizers, over all the workloads of the table. It is kept as `data/bench/baseline-<os>-<arch>.tsv` (lower case, the host class of the record), so that a later change that claims to speed the machine up, or a suspicion of a slowdown, has an earlier figure of the same host class and build to be read against. Nothing in the gate compares a number with it: the suite checks only the FORMAT of a committed record (the format line, the exact header, one `measured` row per workload in table order, `optimized` 1, `sanitizers` off, the counts of the table, positive figures, the `ci_time` pointer and the provenance comment lines).

Procedure, from `<repo-root>`, by a person, when the host is idle (no other build, test or gate is executing; read the load first):

```
uptime
cmake --preset release
cmake --build --preset release
mkdir -p build/bench
{ printf '# host: <operating system, architecture, processor count>\n# idle: <what the load was>\n# date: <ISO date>\n'
  build/release/tools/bench/ivs-bench --root . --content-dir build/release/images; } > build/bench/baseline-input.tsv
```

The three comment lines name a host class and the load, never a machine name, a user, a path or a network address. A record with a `refused` row, `optimized` 0 or `sanitizers` on is not a baseline and is not committed. The record is added to the repository by hand and by a person's decision: a later record replaces it only by a reviewed change that says what changed on the host or in the build.

**The committed baseline.** No baseline record is committed yet (plan 08-19, Task 3 records the first one, from a file that the orchestrator takes on an idle host).

**An idea that is not implemented.** A local-only comparison of CPU-time throughput against the committed record of the same host class and `build_id`, which prints a warning past a loose ratio and exits non-zero only on request, would never run in the gate and is not part of this tool.
