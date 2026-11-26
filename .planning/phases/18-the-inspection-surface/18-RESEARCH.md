# Phase 18: The Inspection Surface — Research

**Researched:** 2026-11-26
**Domain:** build-only C17 inspection tool over existing core seams; CMake target wiring; shell-suite test seams; rights-ledger surfaces
**Confidence:** HIGH on every seam that exists today (read this session, line-anchored below); MEDIUM on the disassembly emitter's row coverage (the corpus is small and its rows are enumerated); one genuine ambiguity in the frozen criterion, recorded as **O1** with a recommendation.

## Summary

Phase 18 adds one build-only executable under `tools/inspect/` with four subcommands, and nothing else
that a user can see. Everything it needs already exists and is reachable: the runner's parser and report
writer are in a `main`-less static library (`ivs_runner_lib`), the per-unit access trace is an internal
seam the `psg` suite already drives, the state-digest call the report uses is **public**, and the
"refuse an output inside the work tree with `refs/` excepted" rule is already implemented once
(`tools/flatwrap/flatwrap.c`) with a suite case shape that can be copied verbatim.

The one thing that does **not** exist and shapes the whole plan: `ivs_runner_execute` steps the machine
through `ivs_machine_run` (`runner/exec.c:1188`), which carries no access log, and the executor's state
struct is `static` inside `exec.c`. **The report and the trace therefore cannot come from one call.** The
tool must produce the report by calling the library (so it is not a second report writer) and produce the
trace by driving its own instance through the same per-unit seam the core's run loop uses. That two-pass
shape is what the phase's liveness guard has to make honest, and it can: `ivs_machine_state_digest` is
public (`core/include/ivs/ivs_machine.h:374`), so the tool can compute the traced instance's nine digests
and require them to equal the nine the report printed. A tracing build that pokes a register then fails by
name, which is exactly criterion 2's third stand-in.

**Primary recommendation:** one target `ivs_inspect` (executable `ivs-inspect`) in a new
`tools/inspect/CMakeLists.txt` added to the root `CMakeLists.txt` beside `tools/flatwrap`, linking
`ivs_runner_lib ivs_core ivs_core_internal ivs_warnings ivs_san` with
`IVS_INSPECT_TREE_ROOT="${PROJECT_SOURCE_DIR}"`; report via `ivs_runner_execute` +
`ivs_runner_report_write`; trace via a per-unit loop over `ivs_machine_step_unit_trace_internal` +
`ivs_input_apply_due`; reconciliation via the nine public digests **and** the last trace tick against
`step.<k>.now_ticks`; 8 ids in `tests/runner-process-tap.sh` (9th argument) and 6 in `tests/asm-tap.sh`
(4th argument); 5 in-case stand-ins; 1 new control dir; 1 rights row. Nothing else moves.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|---|---|---|---|
| scenario parsing, content loading policy, report text | `ivs_runner_lib` (linked, not re-implemented) | — | a second report writer is a second truth; the library already owns the closed key table |
| per-unit bus access observation | `ivs_core_internal` seam (`machine/machine.h:279`) | — | the `psg`, `stic` and `machine` suites already drive it; the core gains no sink |
| run termination ladder (halt / fault / target) and input floor | the tool's own loop, mirroring `machine_bound.c:633-700` | — | unavoidable: the public run call exposes no access log |
| machine state identity (the two passes agree) | public `ivs_machine_state_digest` (`ivs_machine.h:374`) | — | public, so the tool needs no new core surface to check itself |
| read-only guest observation for `step` | public `ivs_machine_cpu_state` (`ivs_run.h:66`) and `ivs_machine_peek` (`ivs_run.h:124`) | — | side-effect-free by construction; no new core function |
| output-path policy (work tree refused, `refs/` allowed, no overwrite) | the tool (`tools/inspect`), root as a compile-time define | — | same shape as `tools/flatwrap`; the core never sees a path |
| disassembly text | the committed listings themselves (`data/expected/listings.ids`) | `ivs_asm` as the re-assembler oracle | a second opcode table is refused by CONTEXT's Deferred section |
| rights posture and refusal-by-name | `data/rights/`, `data/expected/rights.ids`, `docs/RIGHTS.md`, `tools/check/binary-ext.txt`, `.gitignore` | `files` step + one control | the five-place row shape already exists (`save-state` is the closest analogue) |

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| TOOL-02 | A build-only, scripted, headless inspection surface: disassembly of the project's own listings, breakpoints with an asserted hit and non-hit count, a cycle-stamped access trace whose records are non-empty, cycle-monotonic, end at the report's own step tick and leave every digest byte-identical to the same run untraced. Never installed, never in the eleven-path prefix, adds no gate step. | §1 (the linkable seam and the two-pass shape), §2 (the target with no install rule), §3 (the ids and where they live), §5 (absence from the prefix and from an archive) |
| TOOL-09 | Trace and disassembly output is an operator-local, rights-bearing artifact: one question-shaped `data/rights/` row, an output path inside the work tree refused by name except under `refs/`, `ivst` refused by name in the binary-extension list and `.gitignore`, nothing ever committed. | §4 (exact JSON key set, the two refusal tokens, the `ivst` surfaces, the control shape, the `refs/` positive problem) |

## User Constraints

Pointer, not a restatement — the frozen text is `18-CONTEXT.md`, which the planner reads in full. Three
things in it bind this research and are not re-litigated here: the document is frozen alone in wave 1 with
an ancestry proof and wins over plan prose (W5); one binary with four subcommands reusing the runner's own
sources, so a second report writer is refused; `18-03` is built, with the drop's reversal condition already
recorded. The `## Claude's Discretion` block grants exactly the record field names and order after the tick,
the flag spellings, and the internal module split — §1.6 and §3.6 below use that grant and say so.
</phase_requirements>

---

## 1. What the tool can link, and whether the `digest.*` equality is achievable today

### 1.1 Which runner sources are a library, and which are the executable's

`[VERIFIED: runner/CMakeLists.txt:14-37]` — verbatim:

```
set(IVS_RUNNER_SOURCES
  exec.c
  frame_export.c
  ivsl.c
  outcome.c
  report.c
  runner_chain.c
  runner_sha256.c
  runner_vchain.c
  scenario.c
)

add_library(ivs_runner_lib STATIC ${IVS_RUNNER_SOURCES})
...
target_link_libraries(ivs_runner_lib PUBLIC ivs_core PRIVATE ivs_refpcm ivs_warnings ivs_san)

set(IVS_RUN_SOURCES
  hostio.c
  main.c
)
add_executable(ivs_run ${IVS_RUN_SOURCES})
```

So: **nine TUs are a `main`-less static library** and only `main.c` + `hostio.c` belong to the
executable. `ivs_runner_lib` is `PUBLIC ivs_core` — it sees `core/include` only, never `core/src`
(`runner/CMakeLists.txt:3-4`: "It links only the public ivs_core, never ivs_core_internal, so a runner TU
cannot reach core/src"). That is a constraint on the *runner*, not on a consumer: a target that links
`ivs_runner_lib` **and** `ivs_core_internal` gets both include trees. `tools/flatwrap` is the precedent
(`tools/flatwrap/CMakeLists.txt:19`: `target_link_libraries(ivs_flatwrap PRIVATE ivs_core ivs_core_internal
ivs_warnings ivs_san)`).

**Consequence the plan must state:** the tool must not live under `runner/`. `tools/check/steps/runner-scan.sh:20-23`
refuses any runner `.c`/`.h` whose include operand contains `src/`, `machine/`, `memmap/`, `cpu/`,
`common/` or `..` (`runner-scan-internal-include`), so a file under `runner/` that includes
`"machine/machine.h"` fails the gate. `tools/inspect/*.c` is not scanned by `runner-scan`
(`runner-scan.sh:8`: scope is `runner/**`), and `core/src` arrives through `ivs_core_internal`.

### 1.2 Is `ivs_machine_step_unit_trace_internal` reachable, and does `tests/psg/psg_log.c:563` already do it?

`[VERIFIED: core/src/machine/machine.h:275-281]` — verbatim:

```
 * The same step that also records the unit's bus accesses (plan 05-08, D-13): trace is NULL (none) or an array
 * of at least IVS_CP1610_MAX_ACCESSES accesses that ivs_cp1610_step fills (result.n_accesses of them). The
 * machine state, the log and the samples do not depend on it; the public and default seams pass NULL. Lets a
 * test compare the write log with the CPU's own access trace. */
ivs_status ivs_machine_step_unit_trace_internal(ivs_machine *m, ivs_cp1610_result *out, ivs_psg_sink *sink,
                                                ivs_cp1610_access *trace, size_t trace_cap);
```

It is declared in `core/src/machine/machine.h` and **nowhere under `core/include`**, so it is reachable
from any target that adds `core/src` to its include path — `ivs_core_internal` does exactly that
(`[VERIFIED: core/CMakeLists.txt:138-139]`):

```
add_library(ivs_core_internal INTERFACE)
target_include_directories(ivs_core_internal INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src")
```

`[VERIFIED: tests/psg/psg_log.c:555-583]` is the exact loop the tool must mirror — verbatim (function
`lg_trace_units`):

```
static uint64_t lg_trace_units(ivs_ms_inst *x, uint64_t units, uint8_t *want, uint64_t n_want, ivs_psg_sink *sink)
{
  uint64_t u;
  for (u = 0u; u < units; u++) {
    ivs_cp1610_access trace[IVS_CP1610_MAX_ACCESSES];
    ivs_cp1610_result res;
    uint64_t start = pu_now(x->m);
    uint32_t i;
    memset(trace, 0, sizeof trace);
    if (ivs_machine_step_unit_trace_internal(x->m, &res, sink, trace, IVS_CP1610_MAX_ACCESSES) != IVS_OK) {
      return UINT64_MAX;
    }
    for (i = 0u; i < res.n_accesses && i < IVS_CP1610_MAX_ACCESSES; i++) {
      const ivs_cp1610_access *a = &trace[i];
```

`IVS_CP1610_MAX_ACCESSES` is `8u` (`[VERIFIED: core/src/cpu/cp1610.h:21]`), so the array is 8 × 12 bytes
on the stack per unit. `sink` must not be NULL (`machine_run.c:145-147` returns `IVS_E_BAD_ARG` otherwise);
a counting sink is `ivs_psg_sink_init(&sink, NULL, 0u)` (`[VERIFIED: core/src/psg/psg.h:158]`, struct at
`psg.h:125-135`).

### 1.3 Can the report emitter be driven from a second `main` without copying it? Exact names and signatures

Yes. Three functions in the library, none of them `static`:

| Function | Declared | Signature |
|---|---|---|
| `ivs_scenario_parse` | `runner/ivs_runner.h:318` | `int ivs_scenario_parse(const uint8_t *bytes, size_t len, ivs_scenario *out, ivs_scenario_error *err)` |
| `ivs_runner_execute` | `runner/ivs_runner.h:523` | `uint32_t ivs_runner_execute(const ivs_scenario *scn, const ivs_runner_host *host, const ivs_runner_options *opts, ivs_runner_result *result)` |
| `ivs_runner_report_write` | `runner/ivs_runner.h:623` | `int ivs_runner_report_write(const ivs_runner_record *rec, const ivs_runner_host *host, uint64_t *lines)` |

`[VERIFIED: runner/ivs_runner.h:619-625]` — verbatim:

```
/* Writes the report of rec through host->write_report, every key of the closed key table in
 * its fixed order: `ivsr 3` first, `report.end <n>` last (n = lines before it); only when rec asks for
 * an operator WAV (plan 05-10) refpcm.rate, refpcm.frames and refpcm.sha256 follow host.calls, right
 * before report.end. Returns 0, or
 * -1 on a write error; *lines gets the count of lines before report.end. */
int ivs_runner_report_write(const ivs_runner_record *rec, const ivs_runner_host *host, uint64_t *lines);
```

`ivs_runner_execute` fills `ivs_runner_record` (`ivs_runner.h:560-601`), including
`uint8_t digest[9][32]; /* cpu, mem, images, instance, composite, video, audio, input, psglog */`
(`ivs_runner.h:576`). `ivs-run`'s `main` builds the `ivs_runner_host` vtable (`runner/main.c:148-265`) and
prints through `mh_write` (`main.c:251`). The tool needs its own small host: `read_file`, `read_user`,
`now_ms`, `cancel_requested`, `write_report` are the five the executor requires; a `write_report` that
`fwrite`s to stdout is ~6 lines and is **not** a report writer (it writes bytes the library formatted).

### 1.4 The exact `step.<k>` key that carries the end tick, and what the last trace tick must be

`[VERIFIED: runner/report.c:101-104]` — verbatim:

```
static const rp_key rp_step[] = {
    {"target_ticks", RF_STEP_TARGET}, {"now_ticks", RF_STEP_NOW},           {"overshoot_ticks", RF_STEP_OVERSHOOT},
    {"stop_reason", RF_STEP_STOP},    {"expected_stop", RF_STEP_EXPECTED},
};
```

`[VERIFIED: runner/report.c:663-666]` — the key is `step.%u.%s` with `k + 1`, so the first step is
`step.1.now_ticks`. `[VERIFIED: runner/exec.c:1195-1198]` sets it from the run report:
`x->now = rep.now_ticks; ... sr->now_ticks = x->now;`.

Measured this session at the current tree (`build/dev/runner/ivs-run --content-dir build/dev/images
data/scenarios/diag-boot-pass.ivss`):

```
step.1.target_ticks 16000 from=4000 cpu-cycles
step.1.now_ticks 2084
step.1.overshoot_ticks 0
step.1.stop_reason halted
step.1.expected_stop halted
```

`2084` is the number the trace file's last tick must equal. Note what it means: the scenario asks for
16000 ticks and stops **halted** at 2084, i.e. the run ended at a unit boundary below the target, not at
it. A trace pass that stepped to the target would print 16000 and fail. **The trace loop must implement
the halt break, not just the target test.**

### 1.5 Is `ivs_cp1610_access` public or internal, and what does it carry?

Internal. `[VERIFIED: core/src/cpu/cp1610.h:44-55]` — verbatim:

```
/* One bus access (12 bytes). offset_cpu_cycles is the BAR (or address-phase) cycle relative
 * to the unit start, after earlier waits; wait_cpu_cycles is the wait the CPU applied. In the
 * log, data is the value transferred (read reply or written value); INTAK and strobe records,
 * which transfer none, carry 0. */
typedef struct ivs_cp1610_access {
  uint32_t offset_cpu_cycles;
  uint16_t addr;
  uint16_t data;
  uint16_t wait_cpu_cycles;
  uint8_t kind;
  uint8_t flags;
} ivs_cp1610_access;
```

`[VERIFIED: core/src/cpu/cp1610.h:30-38]` — the kind values, verbatim: `IVS_ACC_FETCH 1u`,
`IVS_ACC_READ 2u`, `IVS_ACC_WRITE 3u`, `IVS_ACC_ADAR 4u`, `IVS_ACC_INTAK 5u`, `IVS_ACC_IAB 6u`,
`IVS_ACC_DW 7u`, `IVS_ACC_STROBE_TCI 8u`, `IVS_ACC_STROBE_PCIT 9u`. Flags:
`IVS_ACC_F_RESET 0x01u`, `IVS_ACC_F_INTERRUPT 0x02u` (`cp1610.h:40-42`).

The record's tick is therefore `unit_start + offset_cpu_cycles * ticks_per_cpu_cycle`. The PSG log uses
exactly that arithmetic with a hard-coded `* 4u` in the test (`psg_log.c:570`), but the profile's
`ticks_per_cpu_cycle` is what the tool must use, read from `ivs_machine_info` — do **not** hard-code 4.

### 1.6 The finding that decides the plan's shape: two passes, and the second one is the tool's

`[VERIFIED: runner/exec.c:1188]` — `rs = ivs_machine_run(x->m, &q, &rep);` — the executor steps through
the **public** run call. `ivs_machine_run` steps one unit at a time through
`ivs_machine_step_unit_internal` (`core/src/machine/machine_bound.c:633-679`, the loop is
`while (im->time_ticks < plan.limit) { ... ivs_machine_step_unit_sink_internal(m, &unit, &sink) ... }`),
which passes `NULL` for the trace (`machine_run.c:128`:
`return ivs_machine_step_unit_trace_internal(m, out, sink, NULL, 0u);`). `grep -rn trace runner/*.c
runner/*.h` returns **nothing**. And `ex_state` is a file-scope `static` in `exec.c`
(`[VERIFIED: runner/exec.c:106-119]`, the `ex_*` statics), so the executor's live instance is not
reachable from a second `main`.

There is no third option. A trace hook inside `ivs_runner_options` would require rewriting the executor's
step loop, which moves every golden — refused by §2.1 and by every count invariant. So:

**Pass A (the report).** `ivs_scenario_parse` → `ivs_runner_execute` → `ivs_runner_report_write` through a
`write_report` host callback. The report is the library's, so `digest.*` is byte-identical to `ivs-run`'s
**by construction** for the same scenario and options.

**Pass B (the trace).** The tool creates its own instance from the same content with the public
`ivs_machine_storage_requirements` (`ivs_machine.h:329`), `ivs_machine_create` (`ivs_machine.h:340`),
`ivs_machine_load` (`ivs_machine.h:394`), `ivs_machine_reset` (`ivs_machine.h:409`), reads
`ticks_per_cpu_cycle` from `ivs_info` (`ivs_machine.h:191`, printed by `ivs_machine_info`), then reproduces
the run loop:

```
while (machine_time < limit) {
  if (halted && stop_on_halt) break;             /* machine_bound.c:648 */
  if (faulted) break;                            /* machine_bound.c:644 */
  consumed = ivs_input_apply_due(m, ev, n, consumed, before, entry);  /* machine_bound.c:670 */
  ivs_machine_step_unit_trace_internal(m, &unit, &sink, trace, IVS_CP1610_MAX_ACCESSES);
  emit the unit's records;
}
```

`ivs_input_apply_due` is internal and reachable: `[VERIFIED: core/src/machine/machine.h:531]` —
`uint32_t ivs_input_apply_due(struct ivs_machine *im, const ivs_input_event *ev, uint32_t count, uint32_t next, ...)`.
So the input floor is **the core's own rule**, not a second implementation of it — the one part of the
executor's ladder that could have become a second truth is a call, not a copy.

**The reconciliation that makes the two passes provably the same run.** `[VERIFIED: core/include/ivs/ivs_machine.h:374]`:
`IVS_API ivs_status ivs_machine_state_digest(const ivs_machine *m, uint32_t domain, uint8_t out_sha256[32]);`
— public, and `runner/exec.c:1412` uses it for exactly these nine domains
(`ex_domains`, `exec.c:114-116` = CPU, MEM, IMAGES, INSTANCE, COMPOSITE, VIDEO, AUDIO, INPUT, PSGLOG).
The tool computes all nine on its own instance at the end of pass B and requires them equal to
`rec.digest[0..8]`; a difference is a refusal. This is the limb the "tracing build that pokes a register"
stand-in breaks, and it is a **byte equality against the report's own numbers**, not a proxy.

Caveat the plan must respect: the observation domains (VIDEO, AUDIO, PSGLOG) only match when pass A ran
with **counting sinks**, i.e. no `--audio-raw`, `--psg-log-out`, `--audio-wav`, `--video-raw`,
`--frame-ppm`. The trace id therefore uses a plain run with no operator output option, and the tool
refuses `trace` combined with an operator output option (`inspect-output-combo`) rather than silently
comparing digests that cannot match.

`[VERIFIED: runner/exec.c:372]` — `ex_load_all` is `static`. The tool's own loading is a ~60-line
duplication of role→kind, cap and `expect-sha256` checks. It is **self-checking**: the record's
`slot[role].sha256` and `slot[role].length_bytes` are the oracle for the tool's own loads, and the digest
equality above fails if the two loaders disagree. The plan must say this out loud rather than present the
duplication as free.

### 1.7 The exact bytes the trace id compares

The nine `digest.*` lines of `diag-boot-pass` at the current tree, for the id's own reference (the id
compares two live runs, never these literals):

```
digest.cpu 8c6a8aa37cf3293d3d88f1133f3cb218d89fb96a34e8bda49e6ea750cf4dbe55
digest.mem 6ff06f9c2861d40901606baef86ac7b5d2c7527f3f42f9a148636d8ff3cd7757
digest.images 69d3b715595bdb3435c984ad72e0042fdf385b72e2687f13a4d59e8b09084e45
digest.instance 082ff48343762750e151681d1d76e36b93eae02da6e738c9f2e5956220846312
digest.composite eb893b66dd893ff853a31b65421821a8d9b939f45ca2e5c2b6297abdde5f2a14
digest.video 7dd0bb64bef361e82d1848575b4397006e15e858ffda85fe29bd60072895dea7
digest.audio 6dc304749b42d364796e08346e078bf439f6befc8fd30f478a06a52fd3df31ab
digest.input 50a5dfeaaeef6fb39d4d402b2d234efab7a33147703f602f96bef209b76a40f8
digest.psglog 12dcb66208667a8b659efd3ac5d698adccf0702767ec02ad84048550734a44fa
```

The id's comparison is `sed -n 's/^digest\.//p'` on both reports, `cmp -s` of the two files — the same
two-line shape `check_repeat` already uses (`tests/runner-process-tap.sh:2833-2834`).

### 1.8 The record shape (Claude's Discretion; recommendation)

One ASCII line per access record, LF, decimal numbers, no locale, `tick` first, and one final `end` line
so the criterion's "the last record's tick equal to the report's own `step.<k>` end tick" is literally
true rather than approximately true:

```
tick=1234 kind=write addr=01F4 value=03FF wait=0 cycle=6 unit_start=1210 unit_end=1254
...
end tick=2084 records=842 units=521
```

- `kind` is the `IVS_ACC_*` name lower-cased (`fetch read write adar intak iab dw strobe-tci strobe-pcit`).
- `tick` = `unit_start + cycle * ticks_per_cpu_cycle` — the cycle stamp the criterion names.
- `unit_start` / `unit_end` are "the unit's own stamps"; `unit_end` is machine time after the unit.
- `records` counts access records only, so `records > 0` is a real test even though `end` is a line.
- The `end` line's `tick` is the last unit's `unit_end`, which is by definition `step.<k>.now_ticks`
  for the last step. `records` and `units` are the two counts the ids read.

**O1 is the one ambiguity to record.** Criterion 2 says "the last record's tick equal to the report's own
`step.<k>` end tick"; CONTEXT's field order says "tick first, then the access kind, address, value and the
unit's own stamps", which puts the unit stamps *after* the tick. The two are only compatible if the tick
is the unit-end — which would make "cycle-stamped" false — or if the file's last line is an `end` record.
The recommendation above takes the second reading. The alternative (make `tick` the unit end) satisfies
the criterion literally with one field fewer but loses the per-access cycle stamp TOOL-02 requires
("a cycle-stamped access trace"), so it is the weaker of the two. **The plan must pick one and record the
choice as a deviation sentence if it differs from the doc**, since the doc is frozen first.

### 1.9 The `break` and `step` limbs over the same seam

- `break --pc <HHHH>` reads the trace file pass B just wrote and prints, in one report, the hit count at
  `<HHHH>` and the non-hit count at a second address. A "hit" is an access at `addr == pc` whose `kind` is
  `fetch` or `iab` (the reset entry is an `IAB` with `flags` `F_RESET`, not a `FETCH` — `cp1610.h:40`), so
  the rule must include both or the reset entry reads as never executed.
- A breakpoint changes nothing: `break` reuses the trace file and never steps a machine, so
  non-interference is structural rather than asserted — but the id still asserts it, because a future
  implementation that re-runs the scenario would break it silently.
- `step --count <n>` steps `n` units and prints `ivs_machine_cpu_state` (`ivs_run.h:66`) plus
  `ivs_machine_peek` (`ivs_run.h:124`) words. Both are public and side-effect-free (`machine_inspect.c:1-20`),
  so `step` adds no seam at all.

**The pinned pair for the breakpoint id, from the project's own listing rather than from a guess.** In
`fixtures/FX-boot-stub/boot-stub.listing.txt` the reset entry `B` is at `1000` and its displacement
`000E` skips `1002`–`100F`; the listing itself annotates `1002`/`1003` as "fill (zero, unreachable)". So
`diag-boot-pass` executes `1000` exactly once and `1002` never — a hit/non-hit pair the committed listing
documents. The plan must **measure** both counts during execution and pin what it measured; the listing is
the reason to expect `(1, 0)`, not a substitute for measuring it.

### 1.10 The disassembly limb

`ivs_asm` reads `.asm` source and writes a listing; it has no disassemble mode
(`tools/asm/asm.c:26-31`: `Usage: ivs_asm <source.asm> <listing.txt> | --pairs <pairs.txt> | --version`).
So the round trip is:

```
listing  --(ivs-inspect dis)-->  .asm  --(ivs_asm)-->  listing'  --(ivs_pack --dump-words)-->  words'
words    --(ivs_pack --dump-words on the input listing)---------------------------------------->  words
require words' == words, words > 0
```

That is exactly the comparison `tests/asm-tap.sh:74-86` already implements (`dumps_agree`, `twin_words`),
including the empty-dump refusal — reuse it, do not write a second comparator.

**The corpus, and why it needs no new expected value.** `fixtures/FX-boot-stub/boot-stub.listing.txt` is
22 words, is pure code (no `DECLE` data table), and contains a two-word form (`B` at 1000 → `0200 000E`;
`MVII` at 1004 → `02B8 0001`) and an immediate (`MVII #$0001, R0`). `tests/asm/twins/boot-stub.asm` is the
**known-good source** for exactly that listing and `asm.twin-boot-stub` already proves it assembles to the
same words. The new id therefore has three oracles that must agree — the input listing's words, the twin's
words, and the words of `dis`'s own output — and it invents none.

Verified this session:

```
$ build/dev/tools/pack/ivs_pack --dump-words fixtures/FX-boot-stub/boot-stub.listing.txt | head -6
1000 0200
1001 000E
1002 0000
1003 0000
1004 02B8
1005 0001
```

**How `dis` gets its text without a second opcode table.** CONTEXT's Deferred section refuses "any second
decoder table for disassembly". The listings *are* the pinned table: each word line is
`ADDR HEX4 BITS ROW ASM` and the `ROW` column already holds the leaf row id, which is the mnemonic
(`B`, `HLT`, `MVII`, `MVO`, `TSTR`, `DIS`, `J` appear verbatim in the boot-stub listing). Three candidate
emitters, in ascending cost:

1. **ROW + BITS driven (recommended).** Mnemonic = the `ROW` column verbatim; operands = the following
   `-`-row words plus the register field taken from the `BITS` groups. No table, no prose parsing. The rows
   the corpus needs are seven; the tool refuses a row it does not know with `inspect-dis-row`, which is
   fail-closed and is itself falsifiable by the corpus growing.
2. **ASM-column transcription.** Copy the `ASM` column and strip the ` -> ADDR` annotation and the trailing
   note. Cheapest, but it parses prose and the boot stub's annotations (`; skips the vector area`) are not
   uniformly separable — it will break on the next listing.
3. **`ivs_asm --disassemble`.** Add the inverse mode to the file that owns the table. Cleanest
   intellectually and the largest change: `tools/asm/asm.c` is 3,000+ lines with its own token vocabulary,
   and `asm.neg-coverage` (`tests/asm-tap.sh:301-340`) would need the new mode's tokens pinned. Not
   recommended inside this phase's priced line.

The `dis` output must carry the header block (`# id:`, `# window: AAAA-BBBB`) because `ivs_asm` requires
`# window:` (`tools/asm/asm.c:18-19`), and the window is in the listing's own header lines — copy the
input listing's leading `#` lines verbatim. Do **not** expect the generated listing to equal the committed
one byte for byte: `asm.regen-*` compares listings, this id compares **words** (`ivs_pack --dump-words`),
and the generated listing will lack the hand-written comments, labels and `# sym:` lines.

**The two-word-form and immediate guards, exactly.** Over the emitted source and the reassembled dump:

```sh
# an immediate is present
grep -q -E 'MVII +#' "$tmp/dis.asm"
# a multi-word form is present: the word dump has more lines than the source has instruction lines
[ "$(grep -c . "$tmp/dis.words")" -gt "$(grep -c -E '^ +[A-Z][A-Z@]* ' "$tmp/dis.asm")" ]
```

Both must be asserted, or a `DECLE`-everything emitter (which round-trips perfectly and disassembles
nothing) passes. The constant-mnemonic stand-in is the third limb: it emits the same mnemonic on every
instruction line, reassembles, and fails `dumps_agree` with a diff — the `asm.judge.mutated-twin` shape
(`tests/asm-tap.sh:146-172`), which builds its stand-in at run time by `sed` from a committed twin.

---

## 2. The CMake wiring

### 2.1 Files that change, and nothing else

| File | Edit | Why |
|---|---|---|
| `tools/inspect/CMakeLists.txt` | **new** | the target, the source list, the tree-root define, the `ivs-inspect-sources.txt` writer |
| `tools/inspect/*.c` | **new** | the tool |
| `CMakeLists.txt` | one `add_subdirectory(tools/inspect)` after line 45 (`add_subdirectory(tools/flatwrap)`), with a comment naming the phase | the target exists in both presets |
| `tools/check/steps/c-build.sh:48-51` | one more `'^tools/inspect/.*[.]c$ ivs-inspect-sources.txt'` in the `for mod in` list | W4's module line |
| `tools/check/integrity.cmake:82` | add `inspect` to the alternation: `"^tools/(pack|cart|pcm|asm|bench|flatwrap|inspect)/.*[.](c|h)$"` | **otherwise the tool's sources are silently outside the SPDX/ASCII scan** — every prior tool was added here in the same commit (`data/expected/controls/runner-hygiene.ids:4-7` records the flatwrap, bench, asm and pcm precedents) |
| `tools/check/controls/_good/tree/CMakeLists.txt:16` | one more `file(WRITE "${CMAKE_BINARY_DIR}/ivs-inspect-sources.txt" "")` | the toy tree must write the eighth list or the control's c-build fails with `dev (no ivs-inspect-sources.txt)` |
| `tools/check/controls/_good/tree/tools/inspect/good.c` | **new**, one line | the positive control needs a source the toy list names |
| `tools/check/controls/c-build-unlisted-inspect-source/` | **new control dir** (`CHECK`=`c-build`, `EXPECT`=`FAIL c-build-unlisted-source: tools/inspect/orphan.c`, `overlay/tools/inspect/orphan.c`) + one line in `data/expected/controls/runner-hygiene.ids` | the unlisted-source control W4 requires; this is the +1 in `controls` |

`data/expected/gate-steps/c-build.ids` does **not** exist — the module list is inline in `c-build.sh`
(verified: `ls data/expected/gate-steps/` has no `c-build.ids`). So the module line is a shell edit, not an
inventory edit, and `gates`/`lines` do not move for it.

No new preset: `CMakePresets.json` has exactly `base`(hidden)/`dev`/`release` and both build presets build
every target in the tree. No install rule: nothing in `tools/inspect/CMakeLists.txt` may call `install(`,
and `core/CMakeLists.txt:159-168` holds every `install(...)` rule in the tree. No `ivs_san`
exemption: `tools/asm`, `tools/pack`, `tools/bench`, `tools/flatwrap` all link `ivs_warnings ivs_san`; the
dev preset builds the core archive instrumented, so a target that links the core without `ivs_san` fails to
link.

### 2.2 The exact `tools/inspect/CMakeLists.txt` shape

Copy `tools/flatwrap/CMakeLists.txt` and change four things: the source list, the target/executable names,
the link line, and the define name. The `IVS_*_SOURCES` → `ivs-*-sources.txt` idiom is load-bearing and
must be copied byte-for-byte in shape:

```
set(IVS_INSPECT_SOURCES
  inspect.c
  inspect_trace.c
  inspect_dis.c
)
add_executable(ivs_inspect ${IVS_INSPECT_SOURCES})
set_target_properties(ivs_inspect PROPERTIES OUTPUT_NAME ivs-inspect INTERPROCEDURAL_OPTIMIZATION OFF LINKER_LANGUAGE C)
target_link_libraries(ivs_inspect PRIVATE ivs_runner_lib ivs_core ivs_core_internal ivs_warnings ivs_san)
target_compile_definitions(ivs_inspect PRIVATE "IVS_INSPECT_TREE_ROOT=\"${PROJECT_SOURCE_DIR}\"")

set(listed "")
foreach(src IN LISTS IVS_INSPECT_SOURCES)
  string(APPEND listed "tools/inspect/${src}\n")
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/ivs-inspect-sources.txt.tmp" "${listed}")
file(COPY_FILE "${CMAKE_BINARY_DIR}/ivs-inspect-sources.txt.tmp" "${CMAKE_BINARY_DIR}/ivs-inspect-sources.txt"
  ONLY_IF_DIFFERENT)
```

Two traps, both measured elsewhere in this tree:

- **The target name is underscored, the executable hyphenated** — the convention `ivs_pack`/`ivs_pack`,
  `ivs_flatwrap`/`ivs_flatwrap`, `ivs_bench`/`ivs-bench` follow. `$<TARGET_FILE:ivs_inspect>` is what the
  suites receive, so the *target* name is what `tests/CMakeLists.txt` must name.
- **File stems must stay unique across `core/src` and `runner/`** — `runner/CMakeLists.txt:6-8` says
  `faults.cmake` checks this at configure time because object and plain-copy names use the stem. Naming a
  tool source `trace.c` is fine (no `core/src/trace.c`); naming one `machine.c` or `report.c` would collide.
  `inspect.c` / `inspect_trace.c` / `inspect_dis.c` collide with nothing.

`tools/inspect` is *not* in `faults.cmake`'s scan and cannot be: `tools/asm`, `tools/flatwrap` and the rest
are absent from the named-fault framework (`c-build.sh:3-4` lists the six modules; `IVS_FAULT_CONTROLS`
builds `ivs_faults` from `core/src` only). So `faults` stays 238 with no decision to make.

### 2.3 `runner/hostio.c` is not in the library — decide once

`hostio.c` is in `IVS_RUN_SOURCES` (`runner/CMakeLists.txt:33-35`), not `ivs_runner_lib`. The tool needs
(a) an exclusive-create writer for the output and (b) a non-blocking reader for the content files.
`tools/flatwrap/flatwrap.c:228` uses `fopen(out, "wbx")` and `flatwrap.c:292` uses
`open(O_RDONLY|O_NOCTTY|O_NONBLOCK)` directly, so a tool may use POSIX — `runner-scan` only polices
`runner/**` (`runner-scan.sh:8`). Two options, both acceptable:

- **Own POSIX in `tools/inspect/`** (flatwrap's choice). No extra TU, no cross-target compile.
- **Compile `runner/hostio.c` into the target** (the `host/CMakeLists.txt:20` precedent compiles
  `runner/frame_export.c` in). Then `runner/hostio.c` is compiled by two targets — harmless — but the tool
  must not add it to `IVS_INSPECT_SOURCES` (the list is the *tool's* files, and `c-build.sh:48-51` compares
  `^tools/inspect/.*[.]c$` against it, so a `runner/` entry would be wrong in the list and right in the
  build). **Recommendation: own POSIX in `tools/inspect/`**, because the tool's writer must refuse an
  existing path *and* report the tree refusal before creating anything, which is a different ladder from
  `ivs_hostio_out_create`'s.

---

## 3. The test seams

### 3.1 How a new binary reaches the two suites

`tests/CMakeLists.txt:754-757` — verbatim:

```
add_test(NAME runner-process
  COMMAND sh "${CMAKE_SOURCE_DIR}/tests/runner-process-tap.sh" "${CMAKE_SOURCE_DIR}"
          "$<TARGET_FILE:ivs_run>" "$<TARGET_FILE:ivs_pack>" "${IVS_IMAGES_DIR}" "$<CONFIG>"
          "$<TARGET_FILE:ivs_flatwrap>" "$<TARGET_FILE:ivs_retro_shim>" "$<TARGET_FILE:ivs_host_harness>")
```

The tool becomes the **ninth** argument: `"$<TARGET_FILE:ivs_inspect>"`. The suite's own guard is
`[ "$#" -ge 4 ] && [ "$#" -le 8 ]` (`runner-process-tap.sh:126`) and its usage string is at line 125 —
both must become `-le 9` / gain the argument, and the absent case must report `not ok` with a reason
("the ninth argument (ivs_inspect) is missing"), never a skip. That is exactly how the flatwrap/shim/harness
arguments are handled (`runner-process-tap.sh:1903-1908`).

`tests/CMakeLists.txt:718-719` — verbatim:

```
add_test(NAME asm COMMAND sh "${CMAKE_SOURCE_DIR}/tests/asm-tap.sh" "${CMAKE_SOURCE_DIR}" "$<TARGET_FILE:ivs_asm>"
  "$<TARGET_FILE:ivs_pack>")
```

The tool becomes the **fourth** argument, and `asm-tap.sh:39` changes from
`[ "$#" -eq 3 ] || { printf 'usage: sh asm-tap.sh <source root> <ivs_asm> <ivs_pack>\n' ...` to `-ge 3 -le 4`
with the usage line extended. Nothing else in `asm-tap.sh` reads `$4`.

**Both suites are registered unconditionally and every `add_test` gets `SKIP_RETURN_CODE 77`
(`tests/CMakeLists.txt:2-4`), so a missing binary is a `not ok`, not a skip — that is what keeps
`tests-skipped=4` from moving.**

### 3.2 The `tests-expected` delta, exactly

`tests/check`-side: `tools/check/steps/tests-inventory.sh:50-58` — `expected` is `ids × presets` over
`data/expected/tests/*.ids`, so **each new id costs 2**. At the current tree `tests-expected=5042`; the plan
must recompute it at its own start commit and state `5042 + 2 × (new ids)` (or the start commit's value plus
the same delta) **before** execution. Under the recommended inventory (§3.5: 8 + 6 = 14 ids, plus 5
stand-in cases) the delta is `2 × 19 = 38`.

New ids go in two inventories: `data/expected/tests/runner-process.ids` (currently 150 non-comment lines)
and `data/expected/tests/asm.ids` (50). `tests-inventory.sh:50-51` requires the result id **set** to equal
the file, so an id added to the script and not the file is `tests-unlisted`, and the reverse is
`tests-missing`.

### 3.3 What a new id costs in the other registries

- **`data/expected/important-gates.ids`** (§2.7's must-fail row): one `mutant` row per stand-in case, of the
  form `<gate> mutant <suite>.judge.<slug> <REQ>`. `runner-process.judge.flatwrap-shim` is the newest row of
  this shape and the one to copy. `gates` and `lines` move by the number of rows added — the plan states the
  number (5 rows ⇒ `gates=119`→`124`, `lines=364`→`369`, **measured at the start commit, not predicted from
  this file**).
- **W2's "the pending line deleted"** has nothing to delete: no `data/**/*pending*.ids` file names an
  inspection id. Verified: `grep -rn 'inspect' data/*pending*.ids data/psg/vectors-pending.ids
  data/state/tests-pending.ids data/expected/cp1610-coverage-pending.ids` returns nothing. The plan should
  record that sentence rather than leave it unaddressed.
- **The ten toy registries**: `data/expected/controls/*.ids` holds 37 inventories; a new control dir needs a
  line in the inventory of the step it drives (`data/expected/controls/c-build.ids` or whatever
  `run-controls.sh` reads — check `tools/check/run-controls.sh`'s `CONTROLS_INVENTORY_DIR` use and the
  control's `CHECK` file for the step name). The `_good/tree` overlay comparison is the
  `control-toy-drift` limb: an overlay that replaces `data/expected/important-gates.ids` must declare every
  difference in a `DEVIATION` file (`run-controls.sh:12-20`).

### 3.4 Where the stand-ins live: the `self_control` idiom, copied

`runner-process-tap.sh:2793-2825` is the exact machinery, verbatim in shape:

```
self_control_run <slug> <expect> <check...>: runs the check against a stand-in and sets _sc to `ok`, `vacuous`
or `wrong-reason`. <expect> is the COMPLETE set of finding tokens the stand-in is declared to produce
...
self_control <slug> <expect> <check...>: runs the check; ok only when it fails for exactly the declared set.
```

A control is healthy only when the **observed token set equals the declared one in both directions**, and
`runner-process.judge.host-two-limb` (`runner-process-tap.sh:2878-2896`) is the guard **on** the guard.
Phase 15's two host stand-ins are written into `$fakes` **by the suite** from committed inputs
(`runner-process-tap.sh:2841-2864`) — "They are cases, not scratch probes" (line 2838). That is the
precedent the phrase "committed stand-in" means in this tree: **a committed case id whose stand-in the suite
constructs from committed material**, not a committed binary.

The five stand-ins and their declared token sets:

| Case | Stand-in | Declared tokens |
|---|---|---|
| `runner-process.judge.trace-empty` | a `.ivst` with zero access records and an `end` line | `trace-records` |
| `runner-process.judge.trace-repeat-tick` | a `.ivst` whose access records all carry one tick | `trace-ticks` |
| `runner-process.judge.trace-pokes-register` | the untraced report with one `digest.*` line replaced by another domain's value | `trace-state` |
| `runner-process.judge.break-always-hits` | a `break` report with a nonzero non-hit count printed as 0 | `break-zero` |
| `asm.judge.dis-constant-mnemonic` | a source with every instruction line replaced by one mnemonic, reassembled | `dis-words` |

Each is one limb, so `self_control`'s exact-set rule holds. `asm-tap.sh` has no `self_control` helper — the
closest is `asm.judge.mutated-twin` (`tests/asm-tap.sh:146-172`), which asserts `judge-vacuous` /
`judge-wrong-reason` inline. **Recommendation:** copy the two-token discipline inline rather than
restructuring `asm-tap.sh`; it is 15 lines and keeps the diff to the suite small.

The `break` stand-in needs care: the real `break` reads a trace file, so its check is a pure function of a
file and the stand-in is a file — which is the cheapest stand-in in the phase. Say so; do not build a second
tool.

### 3.5 The recommended id inventory (14 process ids + 5 stand-ins)

`runner-process` (8):

1. `runner-process.inspect.trace-digests` — the tool's report vs `ivs-run`'s, nine `digest.*` lines byte-equal
2. `runner-process.inspect.trace-records` — `records > 0`, strictly increasing `tick`, `end tick == step.1.now_ticks` (2084 for `diag-boot-pass`)
3. `runner-process.inspect.trace-state` — the tool's own nine digests of the traced instance equal the report's
4. `runner-process.inspect.trace-noninterference` — traced report vs untraced report byte-equal on `digest.*` (the second seam, asserted separately)
5. `runner-process.inspect.break-pair` — hit 1 at `1000`, hit 0 at `1002`, both in one id
6. `runner-process.inspect.break-noninterference` — `digest.*` under a breakpoint equal the run with none
7. `runner-process.inspect.step` — `step --count N` prints the CPU state the report's `assert reg` lines agree with (a count and a witness pair, not `exit 0`)
8. `runner-process.inspect.surface` — `--help` names the four subcommands; an unknown subcommand and a missing `--pc` are usage errors; the token-coverage judge (every token the tool's source can print appears in `docs/inspection.md`'s `## Refusal tokens` block, and every token there has a case)

plus the refusal ids that criterion 4 requires — `runner-process.inspect.neg.inspect-output-tree` (with its
`refs/` positive) and `runner-process.inspect.neg.inspect-output-exists`. **If those are ids rather than
sub-limbs, the count is 10 and the plan must say so**; the priced line has room (14 was the ceiling, not a
floor) but `tests-expected` must match whatever is shipped.

`asm` (6): `asm.dis.roundtrip-boot-stub`, `asm.dis.two-word-form`, `asm.dis.immediate`,
`asm.dis.words-nonvacuous`, `asm.dis.deterministic` (two `dis` runs byte-identical — the `asm.repeat`
shape), and the droppable-limb marker case. If `18-03` is dropped, the last four do not land and
`docs/inspection.md`'s disassembly section reads as CONTEXT records.

### 3.6 The `refs/` positive, and the one thing that is awkward

`tools/flatwrap`'s tree rule is `fw_inside_tree` (`tools/flatwrap/flatwrap.c:163-199`): resolve
`IVS_FLATWRAP_TREE_ROOT` physically at run time, resolve the output's **parent** physically, compare, and
allow `refs` and `refs/...`. The three negative spellings its case uses — literal, `..`, and a symlink into
the tree — are at `tests/pack-tap.sh:325-343` and should be copied for `inspect-output-tree`.

The awkward part: `IVS_FLATWRAP_TREE_ROOT` is a **compile-time** define
(`tools/flatwrap/CMakeLists.txt:20`), so the owner-run cases' trick — build a scratch root that mimics the
tree and give it its own `refs/` (`runner-process-tap.sh:2062`, `or_img "$sr/refs/e.bin"`) — is not
available. The `refs/` positive must therefore write `$root/refs/inspect-probe.ivst` **in the real tree**,
which is gitignored (`/refs/`, `.gitignore:2`) and untracked, and the case must create the directory if a
fresh clone lacks it and remove the file unconditionally. Options, with the trade-off stated so the plan can
choose:

- **(a) Real-tree `refs/` positive (recommended).** One `mkdir -p "$root/refs"`, one write, one `rm -f`.
  Leaves an untracked empty directory on a fresh clone. Matches the criterion literally.
- **(b) A runtime env override** (`IVS_INSPECT_TREE_ROOT`). Refused: an operator could disable the refusal,
  which is the whole point of the rule.
- **(c) Outside-the-tree positive only, `refs/` proved by a unit assertion.** Cheaper, but the criterion
  says "beside its `refs/` positive" and the `files`-step's `files-tracked-refs` refusal is the other half
  of the same policy; proving it needs the write.

---

## 4. The rights and refusal surfaces

### 4.1 The exact JSON row schema

`[VERIFIED: tools/check/rights-lib.cmake:11-13]` — verbatim:

```
# A row is: id, item, disposition in {cleared, refused, owner-local-only}, source (a SRC- id or
# null), taken ("nothing" is legal), tokens (an array; an empty array is a recorded answer) and note.
```

Enforced at `rights-lib.cmake:171-174` — `"id:STRING" "item:STRING" "disposition:STRING"
"source:STRING|NULL" "taken:STRING" "tokens:ARRAY" "note:STRING"` — and **every other member is
`rights-unknown-key`** (`rights-lib.cmake:99-106`), so a misspelling fails closed. Further limbs:
the slug in the filename must equal `id` (`rights-id-mismatch`, line 183); `item`, `taken` and `note` must
be non-blank (line 197); each token must match `^[.A-Za-z0-9][A-Za-z0-9._-]*$` (line 214); a `cleared`
disposition needs a resolvable `source` (line 229) — an `owner-local-only` row does **not**.

`data/rights/save-state.json` is the closest analogue and the one to copy:

```
{
  "id": "save-state",
  "item": "a saved machine state",
  "disposition": "owner-local-only",
  "source": null,
  "taken": "nothing",
  "tokens": ["save-state", "savestate"],
  "note": "Owner-local only: ..."
}
```

Three places must move in one commit (`GUARDRAILS-v0.4` W3, `rights-lib.cmake:249-256` and `:266-282`):
the row, a line in `data/expected/rights.ids` (`rights-id-missing` / `rights-id-extra` in both
directions), and a `docs/RIGHTS.md` table row whose **Item** cell contains the row's `item` phrase
**verbatim** (`rights-doc-row`, `rights-lib.cmake:276-281`). `rights=10` → `11`, `rights-cleared=1`
unchanged (only a `cleared` row increments it).

### 4.2 `ivst` in the two name lists, and the control W3 requires

`tools/check/binary-ext.txt` is one list read by **seven** readers (its own header, lines 3-5:
`integrity.cmake`, `files.cmake`, `binscan.sh`, `release-assets.sh`, `steps/release-assets.sh`,
`tools/release-readiness.sh`, `tools/hygiene.sh`). `ivst` joins the body, one extension per line, **below
the comment block** — the header is explicit (`binary-ext.txt:15-16`): "Do not put a comment inside the list
body: a trailing comment on a name line would become part of the name."

`.gitignore` gains `*.ivst` in the "rights-bearing or derivative binary classes" block (lines 4-14), with a
one-line comment naming the phase and why — the `*.ivsn` and `*.vraw` entries are the pattern.

**The control, and whether one extension needs one.** `binary-ext.txt:13-15` states the precedent verbatim:

```
# Controls files-unlisted-binary-so and
# files-unlisted-binary-dylib plant a tracked .so and a tracked .dylib; ... dll and bundle are listed with no control: the four
# extensions are one line of code in files.cmake, and a control per extension would prove it
# again for no new evidence (the tools/bench precedent).
```

So the precedent is *per refusal surface*, not per extension. But W3 names "one `files-unlisted-*`
control" for a trace artifact, and CONTEXT fixes `controls +1` — which the c-build unlisted-source control
already spends. **Read them together and the plan must choose:** either the +1 is the c-build control and
`ivst` gets no control (the `dll`/`bundle` precedent, defensible), or the +1 is the `ivst` control and the
c-build control is the one W4 requires *plus* one more. Recommendation: **`files-unlisted-binary-ivst` is
the one new control**, because W3 names it explicitly for a trace artifact and the c-build unlisted-source
limb is already covered by four existing per-tool controls (`c-build-unlisted-pcm-source`,
`-asm-source`, `-bench-source`, `-flatwrap-source`, `c-build.sh:8-11`) — a fifth is the same line of code
again. If the plan takes that reading it must record the deviation from CONTEXT's parenthetical
("the unlisted-source control W4 requires") in the SUMMARY.

The control itself is three files, copied from `files-unlisted-binary-ivsn`:

```
tools/check/controls/files-unlisted-binary-ivst/CHECK   -> files
tools/check/controls/files-unlisted-binary-ivst/EXPECT  -> FAIL files-unlisted-binary: docs/run.ivst
tools/check/controls/files-unlisted-binary-ivst/MAKE    -> write docs/run.ivst x
```

plus one line in **`data/expected/controls/rights.ids`** (`files-unlisted-binary-ivsn` is at line 20 of that
file, which is the inventory the `files` controls live in). `controls` 404 → 405. The inventory is
per-*group*, not per-step (`run-controls.sh:290-298`: every `*.ids` under `data/expected/controls/` is
concatenated, and a control dir with no line is `control-not-in-inventory` while a line with no dir is
`control-inventory-orphan`), so the plan must add the line to `rights.ids` and the c-build control's to
`runner-hygiene.ids` — two different files, both easy to miss.

### 4.3 The two refusal tokens and their suite cases

| Token | Condition | Positive beside it |
|---|---|---|
| `inspect-output-tree` | the output's physical parent is inside the work tree and not under `refs/` | the same write under `refs/` succeeds |
| `inspect-output-exists` | the path exists (any kind, a dangling symlink included) | a new path succeeds and the refused run leaves the existing file byte-identical |

The exact case shape is `tests/pack-tap.sh:197-217` (`fw_refused`) — exit 1, **nothing on stdout**, exactly
one stderr line `ivs_flatwrap: <token>: ...`, and no output file left. For the tool the prefix is
`ivs-inspect: `. Copy the three spellings for `-tree` (literal, `..`, symlink) and the byte-identical
check for `-exists` (`pack-tap.sh:344-360`).

`data/expected/flat-refusals.ids` is **append-only and is the flatwrap tool's**, not a general token
registry (`data/expected/flat-refusals.ids:2-9`). The inspection tool's tokens must **not** go there. Two
homes, and the plan must pick one:

- **`docs/inspection.md`'s `## Refusal tokens` block (recommended).** The doc is already the frozen oracle
  (W5) and criterion 1 already requires it to state "the refusal tokens". A coverage id reads the block
  between two headings with `sed -n '/^## Refusal tokens/,/^## /p'` and compares it with the token literals
  `grep -o '"inspect-[a-z-]*"' tools/inspect/*.c | tr -d '"' | sort -u` — the `asm.neg-coverage` recipe
  (`tests/asm-tap.sh:323-328`) exactly. No new inventory file.
- **A new `data/expected/inspect-refusals.ids`.** Cleaner as data, but it is a new inventory file that
  `GUARDRAILS-v0.4` §2.2's "may enter" list does not name, and it would need its own staleness limb.

Recommendation: the doc block. It also makes the doc authoritative in a machine-checked way, which is
W5's stated intent.

### 4.4 Nothing is ever committed

`git ls-files | grep -E '\.ivst$'` must be empty at every commit — and the `files` step refuses it
independently once `ivst` is on the binary list, so two mechanisms guard one rule. The `integrity` step
skips text rules for a binary extension (`files.cmake` header, `binscan.sh`), so no second rule is needed.

---

## 5. The build-only proofs

### 5.1 `pkg-install` selftest — `inspect-absent`

`[VERIFIED: tools/check/steps/pkg-install.sh:670-679]` is the template, verbatim:

```
    host-so-absent)
      # The build-only host object (plan 15-02, TOOL-08, D-49) is a NON-member: it has no install rule and the
      # eleven-path inventory is what makes its absence a check rather than a statement. A future
      # install(TARGETS ivs_retro_shim ...) fails here first, by name, before it can reach a release.
      cp -R "$pb" "$d/prefix" || { detail="copy"; return 1; }
      mkdir -p "$d/prefix/lib" && printf 'x\n' > "$d/prefix/lib/libivs_retro.so"
      if check_inventory "$d/prefix" > "$d/out"; then detail="inventory passed"; return 1; fi
      grep -q -x -F -e '-- FAIL pkg-inventory-extra: lib/libivs_retro.so' "$d/out" \
        || { detail="no pkg-inventory-extra: lib/libivs_retro.so"; return 1; }
      ;;
```

The inspection case is the same three lines with `bin/ivs-inspect` and
`-- FAIL pkg-inventory-extra: bin/ivs-inspect`. The case name joins `data/expected/pkg-selftests.ids`
(20 → 21 lines) **and** the `case ... in` ladder; a name in the file that the script does not know is
`pkg-selftest-missing` (`pkg-install.sh:66-69`). `pkg-selftest=20/20` → `21/21`. `prefix-files=11`
**unchanged** — that is the whole point.

### 5.2 `release-assets` case — which member is actually refusable

The member rule is **extension-based only**: `tools/check/release-assets.sh` (header, "release-assets-binary-member
the last extension of a member file name (lower case, after the last dot) is a line of binary-ext.txt,
whatever the list says") and the awk at `tools/check/release-assets.sh:189-195`
(`k = split(base, part, "."); if (k >= 2 && (tolower(part[k]) in bin)) emit(...)`). **A member named
`bin/ivs-inspect` has no dot, so no rule refuses it.** The refusable member is the trace artifact:

```
inspect-ivst-member)
  mkdir -p "$tmp/$c" && mkarch "$c" a.tar.gz ivs-x include/ivs/ivs.h lib/libivs_core.a docs/run.ivst
  runlib "$c" install a.tar.gz ivs-x include/ivs/ivs.h lib/libivs_core.a docs/run.ivst
  wantfail "$c" 'release-assets-binary-member: docs/run.ivst' || return 1
  grep -q -e 'release-assets-extra-member' -e 'release-assets-missing-member' "$tmp/$c/lib.out" && fail
```

— the `so-member` shape at `tools/check/steps/release-assets.sh:182-195` with the extension swapped. The plan must record the
reading: criterion 5 says "one `release-assets` case refuses it as an archive member"; the only member the
current rule can refuse is the `.ivst`, so **"it" is the trace artifact, not the binary**. Making the
binary refusable would need a new name-based rule in `release-assets.sh`, which is a behaviour change
needing its own positive and negative control — out of this phase's line. `release-selftest=12/12` → `13/13`.

### 5.3 The four invariant counts, and where they come from

| Token | Value | Source |
|---|---|---|
| `steps` | `38/38` | `ls tools/check/steps/*.sh \| wc -l` (verified: 38) |
| `controls` | `404/404` → `405/405` | `ls -d tools/check/controls/*/ \| grep -v _good \| wc -l` (verified: 404); `run-controls.sh:375` |
| `faults` | `238/238` | `tools/check/steps/c-faults.sh`; `tools/inspect` is outside the named-fault framework |
| `goldens` | `29` | `tools/check/steps/golden.sh`; no trace or disassembly is ever a golden |
| `claims` / `provisional` | `29` / `28` | this phase mints no claim (`GUARDRAILS-v0.4` §3) |
| `prefix-files` | `11` | `data/expected/pkg-install-files.ids`, unchanged |
| `docs` / `doc-files` | `64` → `65` / `63` → `64` | `steps/text.sh:31` (README + CHANGELOG + `docs/**/*.md`); `steps/glossary.sh:46` (README + `docs/**/*.md`) |

`docs/inspection.md` is a new `docs/*.md`, so **both** doc tokens move by exactly one in the wave-1 commit —
predict it, do not be surprised by it.

---

## 6. The cost, priced locally

`[CITED: .planning/research/v0.4.0-scoping-2-gate-cost.md §2.3]` — verbatim:

```
| 18 | 14 process-launch ids (8 trace/breakpoint/command surface, 6 disassembly and round trip) + 5 controls (liveness + stand-in, non-vacuity + stand-in, rights refusal, not-installed) + 1 rights row | 14 x 0.10 = 1.40 | 5 x 0.31 = 1.55 | 0.005 | **2.96** |
```

and `§2.3` also records "Dropping Phase 18's disassembly limb returns 1.22 (6 ids, 2 controls)". The unit
prices are frozen upstream: in-process id 0.065, process-launch id 0.10, control 0.31, in-step selftest
0.05, registry row 0.005 (from Phase 15, cited by `17-01-SUMMARY.md`'s requires block).

**The plan states its line before execution, and the arithmetic is:**

| item | unit (hosted cpu-s) | count | subtotal |
|---|---|---|---|
| process-launch id, `runner-process` | 0.10 | 8–10 (see §3.5) | 0.80–1.00 |
| process-launch id, `asm` | 0.10 | 6 | 0.60 |
| in-case stand-in case (`self_control` / `judge`) | 0.31 | 5 | 1.55 |
| control dir (`files-unlisted-binary-ivst`) | 0.31 | 1 | 0.31 |
| registry row (`important-gates.ids`) | 0.005 | 5 | 0.025 |
| rights row | 0.005 | 1 | 0.005 |
| **priced addition** | | | **3.29** |

That is **over** the 2.96 roadmap line by 11 percent — inside §2.4's 25 percent repricing threshold, and it
is over only because the control is charged twice in the table above (once as a control dir, once among the
five in-case stand-ins). If the plan keeps the control count at 5 *total* (4 in-case + 1 dir) the line is
`0.10 × 14 + 0.31 × 5 + 0.005 × 6 = 1.40 + 1.55 + 0.03 = **2.98**`, two percent over the 2.96 line. **State
the arithmetic the plan actually ships; do not restate 2.96 as if it were a measurement.** A phase that
ships fewer than 14 ids is under the line and must say so rather than pad.

The measurement the plan owes (§2.4: "The priced unit is the **locally measured** addition (alternated
before and after, multiplied by 1.675 for the hosted scale)"):

```sh
# alternated, three rounds, on an otherwise quiet tree
sh tools/gate.sh >/dev/null 2>&1   # warm
/usr/bin/time -p sh tools/gate.sh >/dev/null 2>/tmp/before.txt   # twice
/usr/bin/time -p sh tools/gate.sh >/dev/null 2>/tmp/after.txt    # twice, after the ids land
# the addition is the median of (after_user + after_sys) - (before_user + before_sys), x 1.675
```

`gate-time total=<n>` is printed by the gate itself; either figure is acceptable as long as the plan names
which one it used. Do **not** price from `.planning/ci-runs.tsv` — §2.1 records why the hosted median
cannot resolve a 1–3 cpu-s addition (221 cpu-s span over a 572 median).

---

## 7. Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---|---|---|---|
| scenario parsing | a second `.ivss` parser | `ivs_scenario_parse` (`ivs_runner.h:318`) | 1,336 lines of grammar, caps and refusal ladder; a second parser is a second grammar |
| report text | a `printf` per key | `ivs_runner_report_write` (`ivs_runner.h:623`) | the closed key table, the `report.end` count and the WAV tail are already pinned; a second writer makes the digest equality unfalsifiable |
| the nine digests | a hash of a `memcpy` | `ivs_machine_state_digest` (`ivs_machine.h:374`) | the serializer is a field table with a version in the preimage; a copy diverges silently on the next field |
| the input floor | a "next event at or before this unit" rule | `ivs_input_apply_due` (`machine.h:531`) | the floor is `unit_start + 1` and the take-back on a refused step is subtle (`machine_bound.c:670`, `:679-691`) |
| opcode decoding | a new mnemonic table | the listings' `ROW` + `BITS` columns, and `ivs_asm` as the re-assembler | CONTEXT's Deferred section refuses a second table; `asm.twin-boot-stub` already proves the corpus |
| word extraction from a listing | a second listing parser | `ivs_pack --dump-words` | two independent parsers already exist (`pack` suite), and this would be a third |
| "output inside the work tree" | a new `..`-and-symlink normaliser | `fw_inside_tree`'s technique (`flatwrap.c:163-199`), copied | `realpath` on the **parent**, compare, allow `refs` — the owner-run and flatwrap rules agree on it |
| exclusive create | `fopen(path, "w")` | `"wbx"` (flatwrap) or `O_CREAT|O_EXCL` (hostio) | a truncating open destroys the file `inspect-output-exists` exists to protect |

---

## 8. Common Pitfalls

### Pitfall 1: the trace loop that steps to the target instead of to the stop
**What goes wrong:** the tool prints a last tick of `16000` for `diag-boot-pass` and the id fails on a
scenario that is not broken.
**Why it happens:** `run-to 4000 cpu-cycles stop halted` ends at 2084 because the guest halts; the target is
never reached.
**How to avoid:** mirror `machine_bound.c:633-700` exactly — faulted break, halted break under
`IVS_RUN_F_STOP_ON_HALT`, input apply, step — and treat a refused step as the end of the loop, not an error.
**Warning signs:** `step.1.now_ticks` from the report is much smaller than `step.1.target_ticks`, and the
trace's `end tick` equals the target.

### Pitfall 2: `ticks_per_cpu_cycle` hard-coded as 4
**What goes wrong:** the record's `tick` is wrong on any profile whose timebase differs, and the equality
with `now_ticks` is only accidentally right.
**Why it happens:** `tests/psg/psg_log.c:570` writes `* 4u` literally, because that suite's profile is the
NTSC one.
**How to avoid:** read it from `ivs_machine_info` (the `ivs_info` timebase fields) and multiply.
**Warning signs:** the same trace file passes on one profile and fails on another.

### Pitfall 3: the observation digests compared against a run that requested a buffer
**What goes wrong:** `trace` on a scenario run with `--audio-raw` compares `digest.audio` from a
counting-sink pass against one from a real-buffer pass and refuses a correct run.
**Why it happens:** `digest.video/audio/psglog` carry chain and count values that only match when both
passes use the same sink shape.
**How to avoid:** refuse the combination with a token, and use a plain run in the id.
**Warning signs:** the refusal fires only for scenarios with an output option.

### Pitfall 4: `docs/inspection.md` trips the claim-word tripwire
**What goes wrong:** `text-claim-untagged` fails the gate on the frozen document, in wave 1, alone.
**Why it happens:** `tools/check/steps/text.sh:44-49` scans `docs/**/*.md` case-insensitively for
`supports|compatible|passes|accurate|verified|runs|plays|playable|looks|sounds` and a line needs a
`[claim:CL-nnnn]` tag or a literal `[nc]` on the same line. An inspection document will want "runs",
"passes" and "verified" repeatedly.
**How to avoid:** every such line carries `[nc]`; do not invent a claim id (the phase mints none).
**Warning signs:** `docs=65` in the token line but a `-- FAIL text-claim-untagged: docs/inspection.md:<n>`.

### Pitfall 5: the bare-clock tripwire, the same way
**What goes wrong:** `glossary-bare-clock`.
**Why it happens:** `steps/glossary.sh:54-57` refuses the word `clock`/`clocks` in `docs/**/*.md` unless the
line also holds `input clock`, `STIC clock`, `PSG chip clock` or `[nc]`.
**How to avoid:** the tool has no clock; if the doc needs the word, tag the line.

### Pitfall 6: a comment line inside `tools/check/binary-ext.txt`'s body
**What goes wrong:** the extension list acquires an entry named after the comment's first word.
**Why it happens:** the header says so in as many words (`binary-ext.txt:15-16`), and seven readers each treat
every non-`#` line as one extension.
**How to avoid:** put the `ivst` note in the comment block above the body.

### Pitfall 7: `asm-tap.sh`'s arity assertion
**What goes wrong:** the whole suite exits 1 with a usage line, in both presets, and every asm id reads
`tests-suite-missing`.
**Why it happens:** `tests/asm-tap.sh:39` is `[ "$#" -eq 3 ]`.
**How to avoid:** change the arity test and the usage string in the same edit that adds the fourth
argument to `tests/CMakeLists.txt:718`.

### Pitfall 8: `dis` emitting `DECLE` for everything
**What goes wrong:** the round trip passes and the disassembler disassembles nothing.
**Why it happens:** `DECLE` round-trips any word, so byte equality alone is vacuous.
**How to avoid:** the two-word-form and immediate assertions of §1.10, plus the constant-mnemonic stand-in.
**Warning signs:** the emitted source has no mnemonic at all and the word dump equals the source line count.

### Pitfall 9: `ivst` refused by the tree rule before the tree rule is reached
**What goes wrong:** the `inspect-output-tree` case fails with `inspect-output-exists` or a file error.
**Why it happens:** the check order matters — the tree rule must run **before** any create, and a leftover
file from a previous run turns the second case into an "exists" refusal.
**How to avoid:** order the ladder as flatwrap does (`flatwrap.c:341-355`: tree, then exists) and `rm -f`
the probe path at the start of every case.

### Pitfall 10: `tests-expected` predicted instead of recomputed
**What goes wrong:** the plan predicts `5080` and the gate prints something else, and the executor treats an
unexpected token as a stop.
**Why it happens:** the start commit's `tests-expected` may not be 5042, and `expected = ids × presets` over
`data/expected/tests/*.ids` (`tests-inventory.sh:50-58`) counts every line, including `skip-ok` ids.
**How to avoid:** recompute at the plan's start commit and state `start + 2 × (new ids)`.

---

## 9. Validation Architecture

### Test Framework

| Property | Value |
|---|---|
| Framework | the project's own TAP shell suites, one `ctest` test per suite (`tests/CMakeLists.txt:2-4`) |
| Config file | `CMakePresets.json` (`dev`, `release`); `tests/CMakeLists.txt` registers the suites |
| Quick run command | `ctest --preset dev -R 'runner-process\|asm' --output-on-failure` |
| Full suite command | `sh tools/gate.sh` (and `dash tools/gate.sh` for the §4 definition of done) |

### Phase Requirements → Test Map

| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|---|---|---|---|---|
| TOOL-02 | trace records non-empty, cycle-monotonic, end at `step.<k>.now_ticks` | integration (process) | `ctest --preset dev -R runner-process` | ❌ Wave 0 (new ids) |
| TOOL-02 | every `digest.*` byte-identical to the untraced run | integration (process) | same | ❌ Wave 0 |
| TOOL-02 | breakpoint hit/non-hit pair `(1, 0)` | integration (process) | same | ❌ Wave 0 |
| TOOL-02 | disassembly round-trips, two-word form + immediate present | integration (process) | `ctest --preset dev -R '^asm$'` | ❌ Wave 0 |
| TOOL-02 | absent from the eleven-path prefix | in-step selftest | `sh tools/check/steps/pkg-install.sh` via the gate | ✅ ladder exists, case new |
| TOOL-09 | output inside the tree refused, `refs/` allowed, existing refused | integration (process) | `ctest --preset dev -R runner-process` | ❌ Wave 0 |
| TOOL-09 | `ivst` refused by name in the tree and in an archive | control + in-step selftest | the gate's `files` and `release-assets` steps | ✅ mechanisms exist, control and case new |
| TOOL-09 | nothing tracked with `.ivst` | gate step | `git ls-files \| grep -E '\.ivst$'` (empty) | ✅ |

### Sampling Rate

- **Per task commit:** `ctest --preset dev -R 'runner-process|asm' --output-on-failure`
- **Per wave merge:** `sh tools/gate.sh` and `dash tools/gate.sh`, both ending `gate: PASS` with the plan's
  predicted token string byte for byte (`GUARDRAILS-v0.4` §4)
- **Phase gate:** the full gate plus one `sh tools/gate-linux.sh` (copy `build/gate-linux.log` aside first —
  lesson 18), then `/gsd:verify-work`

### Wave 0 Gaps

- [ ] `docs/inspection.md` — wave 1, alone, with the ancestry proof (W5); the `## Refusal tokens` block
      pinned here is what the coverage id reads
- [ ] `tools/inspect/` + `tools/inspect/CMakeLists.txt` — wave 2
- [ ] `data/expected/tests/runner-process.ids`, `data/expected/tests/asm.ids` — the new ids
- [ ] `data/rights/inspection-output.json`, `data/expected/rights.ids`, `docs/RIGHTS.md` row — one commit
- [ ] `tools/check/binary-ext.txt`, `.gitignore` — the same commit
- [ ] `tools/check/controls/files-unlisted-binary-ivst/` + its line in `data/expected/controls/rights.ids`
- [ ] `tools/check/controls/c-build-unlisted-inspect-source/` + its line in `data/expected/controls/runner-hygiene.ids`
- [ ] `tools/check/integrity.cmake:82` — `inspect` joins the tools alternation (SPDX/ASCII coverage)
- [ ] `data/expected/pkg-selftests.ids` + `pkg-install.sh`'s ladder; `data/expected/release-assets-selftests.ids` + `release-assets.sh`'s ladder
- [ ] `data/expected/important-gates.ids` — one `mutant` row per stand-in case

*(No framework install: the harness is the tree's own.)*

---

## 10. Security Domain

`security_enforcement` is absent from `.planning/config.json`, so this section is required.

### Applicable ASVS Categories

| ASVS Category | Applies | Standard Control |
|---|---|---|
| V2 Authentication | no | no identity, no network, no credential |
| V3 Session Management | no | one process, one run, no session |
| V4 Access Control | yes (weak) | the tool is build-only and never installed, so the "who may run it" answer is "whoever built the tree"; the rights row is the *artifact* control, not an identity control |
| V5 Input Validation | **yes** | the scenario is untrusted inert data parsed by `ivs_scenario_parse` (caps: 64 KiB, 512 lines, 64 steps, 4096 capture entries — `runner/ivs_runner.h:104-115`); the tool's own arguments are the new surface: an address, a count and two paths, each with a closed grammar |
| V6 Cryptography | yes, by reuse | `ivs_machine_state_digest` and `ivs_sha256` only; the tool must not implement or wrap a hash |
| V12 File Resources | **yes** | the exclusive create, the work-tree refusal and the `refs/` exception are the whole file policy; no file is written before every refusal has been decided |

### Known Threat Patterns for this stack

| Pattern | STRIDE | Standard Mitigation |
|---|---|---|
| writing a rights-bearing artifact into the tree | Information disclosure | `inspect-output-tree` before any create; `ivst` on the binary list and in `.gitignore`; the `files` control |
| clobbering an existing operator file | Tampering | `inspect-output-exists` via `O_CREAT\|O_EXCL` / `"wbx"`; the case asserts the refused file is byte-identical |
| a path that escapes the rule through `..` or a symlink | Elevation of privilege | `realpath` the **parent** and compare physically (`flatwrap.c:163-199`); three spellings in the case |
| a scenario that reads a file outside `--content-dir` | Tampering | the parser refuses `..`, a leading `/` and an empty segment (`runner/ivs_runner.h:62-64`); the tool adds no reader of its own beyond `--content-dir` |
| an unbounded trace file | Denial of service | the trace's length is bounded by `max_ticks` (the runner's own default 2^40) and by the scenario's caps; the plan should state the bound rather than discover it on a 40-minute run |
| a trace file that silently truncates | Repudiation | the writer must fail on a short write and remove what it created, as `flatwrap` does (`flatwrap.c:232-245`) |

---

## 11. Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | The traced instance's nine digests equal the record's nine for a plain (no operator output) run | §1.6 | the reconciliation limb does not exist and `trace-state` cannot be asserted; fallback is the tick-only reconciliation, which is weaker but still a domain assertion |
| A2 | `1000` is fetched exactly once and `1002` never in `diag-boot-pass` | §1.9 | the pinned pair is wrong and `break-pair` fails; the plan measures before pinning, so this is a prediction, not a commitment |
| A3 | `dis` can be implemented from `ROW` + `BITS` for the seven rows the boot stub needs | §1.10 | `18-03` cannot round-trip inside the line and the pre-authorized drop is taken (CONTEXT records the reversal condition) |
| A4 | One new control dir satisfies both W3's `files-unlisted-*` and W4's unlisted-source limb | §4.2 | a second control is needed and `controls` is 406, not 405 — a one-line correction to the predicted token line |
| A5 | The `release-assets` case refuses a `.ivst` member, not the extensionless binary | §5.2 | the case cannot be written as criterion 5's words read; needs an owner decision |
| A6 | `docs/inspection.md` can be written without a claim word on an untagged line | §8 Pitfall 4 | the wave-1 commit fails the gate and the freeze is delayed; fixable with `[nc]` |

**If A1 fails**, the phase still meets TOOL-02's words ("end at the report's own step tick") but loses the
state-equality limb; the poke stand-in would then have to break the tick instead, and the plan must say so.

## 12. Open Questions (RESOLVED)

All three are resolved by the plans; each entry names where the resolution lands, so no question is left for the
executor to settle in the dark.

1. **O1 — what "the last record's tick" means. (RESOLVED — `18-01-PLAN.md` Task 1, `docs/inspection.md`'s
   `## The trace file` paragraph.)** The page is frozen with an `end` record as the last line, its `tick` equal to
   the last step record's `end` and to the report's own `step.<k>.now_ticks`, while an access record keeps its own
   cycle stamp. The alternative (every record's `tick` = unit end) is refused in the page's own words because it
   loses the per-access cycle stamp the requirement names. Criterion 2 says it equals `step.<k>`'s end tick; CONTEXT's
   field order puts the unit stamps after the tick. Recommendation: an `end` record as the last line, access
   records carrying their own cycle stamp (§1.8). Alternative: `tick` = unit end. **The plan decides and, if
   the decision differs from the frozen doc's wording, records it as a deviation** — this is precisely the
   case W5's "the document wins" rule was written for, and the doc is frozen first, so the doc's own sentence
   is the one that binds.
2. **O2 — how many ids the phase actually ships. (RESOLVED — `18-02-PLAN.md` and `18-03-PLAN.md`, stated in each
   plan's delta table before execution.)** Nineteen: fourteen non-stand-in ids (four in `18-02` Task 1, five in
   `18-02` Task 2, five in `18-03`) and five stand-ins (four in `18-02` Task 2, one in `18-03`) — the priced line's
   14 + 5 exactly, so the phase is neither under nor over it. `tests-expected` moves 5044 → 5052 → 5070 → 5082 and
   `tests-executed` 5040 → 5048 → 5066 → 5078, both counted across two presets.
3. **O3 — where the disassembly ids live when `18-03` is dropped. (RESOLVED — `18-03-PLAN.md` Task 1, steps (c)
   and (e).)** The six `asm` ids, their six inventory lines and their one registry row land in the same commit as
   the emitter, and the drop is taken in that same commit: the six inventory lines are removed with it, so
   `data/expected/tests/asm.ids` never names an id the suite cannot mint. The drop also appends the note
   `docs/inspection.md`'s `## Disassembly` section authorises and records `TOOL-02` exiting with the reduced
   surface named.

## 13. Sources

### Primary (HIGH confidence — read this session, line-anchored above)
- `core/src/machine/machine.h` (267, 275-281, 531, 136-198), `core/src/machine/machine_run.c:128-132`,
  `core/src/machine/machine_bound.c:633-700`, `core/src/machine/machine_inspect.c:1-20`
- `core/include/ivs/ivs_run.h:66,124,331-357`, `core/include/ivs/ivs_machine.h:329,340,374`
- `core/src/cpu/cp1610.h:21,30-55`
- `runner/ivs_runner.h:318,523,560-601,619-625`; `runner/CMakeLists.txt:14-45`;
  `runner/report.c:101-104,663-676`; `runner/exec.c:106-119,372,1188-1198,1412`
- `core/CMakeLists.txt:138-139,159-168`; `CMakeLists.txt:33-51`; `CMakePresets.json`
- `tools/flatwrap/CMakeLists.txt:19-20`, `tools/flatwrap/flatwrap.c:163-199,228-245,341-355`
- `tools/asm/asm.c:18-31`; `tests/asm-tap.sh:39,74-86,146-172,301-340`; `tests/pack-tap.sh:197-217,325-360`
- `tests/runner-process-tap.sh:127-129,250-340,1453-1454,1895-1990,2755-2900`; `tests/CMakeLists.txt:718-719,746-757`
- `tools/check/steps/c-build.sh:48-59`; `tools/check/steps/pkg-install.sh:670-679` (the `host-so-absent` case);
  `tools/check/steps/release-assets.sh:182-195` (the `so-member` case); `tools/check/release-assets.sh:186-196`
  (the member rule's awk);
  `tools/check/steps/text.sh:44-49`; `tools/check/steps/glossary.sh:40-65` (bare-clock rule at 52-57);
  `tools/check/steps/tests-inventory.sh:50-58`; `tools/check/rights-lib.cmake:11-13,99-106,176-229,246-282`;
  `tools/check/files.cmake` (binary-by-name rule); `tools/check/integrity.cmake:82`;
  `tools/check/run-controls.sh:1-30,290-298,375`; `data/expected/controls/rights.ids:20`;
  `data/expected/controls/runner-hygiene.ids:1-25`
- `tools/check/binary-ext.txt` (header 3-22), `.gitignore:1-40`
- `data/rights/save-state.json`, `data/rights/rendered-frame.json`, `data/expected/rights.ids`,
  `data/expected/pkg-install-files.ids`, `data/expected/pkg-selftests.ids`,
  `data/expected/release-assets-selftests.ids`, `data/expected/listings.ids`,
  `data/expected/flat-refusals.ids`, `data/expected/important-gates.ids`
- `docs/RIGHTS.md:34-78`, `docs/packaging.md:38,122-123`
- `fixtures/FX-boot-stub/boot-stub.listing.txt`, `tests/asm/twins/boot-stub.asm`, `data/scenarios/diag-boot-pass.ivss`
- `core/include/ivs/ivs_machine.h:374` for the digest call; `core/include/ivs/ivs_run.h:124` for peek

### Secondary (MEDIUM confidence)
- `.planning/research/v0.4.0-scoping-2-gate-cost.md §2.1-2.4` (the priced line and the unit prices)
- `.planning/ops/GUARDRAILS-v0.4.md` W1-W10, §2.1-2.14, §4
- `.planning/phases/17-.../17-01-SUMMARY.md` (the recomputed window, `cpu_s=572 build_s=49 gate_s=164` at `21d023c`)

### Tertiary (LOW confidence)
- None. Every claim above is either read from a file this session or cited from a frozen upstream record;
  the four predictions that are not (A1, A2, A3, A5) are marked `[ASSUMED]` in the Assumptions Log.

## Metadata

**Confidence breakdown:**
- Standard stack: **HIGH** — every target, header, function and signature was read this session with a line
  anchor; the two-pass shape is forced by `exec.c:1188` and the absence of any trace in `runner/`.
- Architecture: **HIGH** — the tool's tier assignment is the flatwrap precedent plus the runner library's
  own `main`-less split; the responsibility map has no contested row.
- Pitfalls: **MEDIUM** — the gate-side tripwires (`text`, `glossary`, `binary-ext`, `asm-tap` arity) are
  read and certain; the runtime pitfalls (Pitfall 1, 2, 3) are derived from the code and not yet executed.
- Cost: **MEDIUM** — the unit prices are frozen upstream; the count of shipped ids is the plan's to fix.

**Research date:** 2026-11-26
**Valid until:** 2026-12-26 (30 days; the tree's seams are frozen by the milestone, but `runner/` and the
suites move every phase, so re-read §1.3 and §3.1 if a later plan changes `runner/CMakeLists.txt`,
`tests/CMakeLists.txt` or `runner-process-tap.sh`'s argument list)

## RESEARCH COMPLETE
