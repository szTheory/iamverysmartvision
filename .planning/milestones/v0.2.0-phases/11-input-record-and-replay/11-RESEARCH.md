# Phase 11: Input Record and Replay - Research

**Researched:** 2026-11-25
**Domain:** runner-side input logging (`ivsl` v1), replay hook, gate registration
**Confidence:** HIGH for the code and count facts (every one measured on this tree);
MEDIUM for the design recommendation (it is inference from those facts).

**Provenance note.** No external research provider is enabled (`.planning/config.json`:
`brave_search`, `exa_search`, `firecrawl`, `tavily_search`, `ref_search`, `perplexity`,
`jina` all `false`), and this session has no web tool. **Every claim below is read from this
tree or produced by a command run on it**, so tags are `[OBS]` (observed: path + line, or
command + output) and `[INF]` (inference from observations). There is no `[CITED]` tag in
this document and there is no external `[VERIFIED]`. Anything I could not read is marked
`[UNK]`.

---

## Summary

The phase is smaller than its frozen criteria read, and **one of the five criteria is
unsatisfiable as written** (the "all nine domains observed" clause — §"Frozen criteria that
cannot be met as written", A-11-01) while a second is satisfiable only by an interpretation
that must be recorded (the seven refusal classes cannot express the grammar's real refusal
surface — A-11-02). Both were found by reading the code, not by planning.

The seam already exists and is well understood: `runner/exec.c` has one slice loop, one
options validator and one hook (`ex_roundtrip_after_slice`, exec.c:264) that plan 08-16 built
and then **had to repair** because a cut body that did nothing matched every report
(08-16-SUMMARY deviation 2, commit `336b3cb`). The repair is the template for this phase, and
the lesson generalises in a way the context does not yet say: the round-trip hook's vacuity was
*"the body could do nothing"*; this phase's vacuity is *"the replay could ignore the log"*, and
that one is **not** caught by comparing two reports that both come from the same scenario. The
test must replay the log into a scenario whose own `input` lines are gone.

Two hard architectural facts decide the plan, and neither is in the context:

1. **The two named faults cannot have sites in `runner/main.c` or `runner/hostio.c`.**
   `tests/faults.cmake` refuses an owner TU that is not in a module of the suite
   (`FATAL_ERROR "fault ${id}: owner ${owner} is not in a module of suite ${suite}"`,
   faults.cmake:155), and the runner suite's modules are `core;runner` where `runner` means
   `IVS_RUNNER_SOURCES` (runner/CMakeLists.txt:9-18) — which excludes `main.c` and `hostio.c`
   (runner/CMakeLists.txt:34-37). So the record/replay logic must live in a **library** TU and
   operate on **memory**, and `main.c` keeps the file I/O. That is also exactly what
   GUARDRAILS 2.7 demands.
2. **A new flag bit at `2u` would break an existing test.** `runner.exec.roundtrip-option-range`
   uses bit 2 as its "unknown flag" witness in three rows
   (runner_exec.c:2935-2937: `{2u, 0u, 0, 0u}`, `{2u, 5000u, 0, 0u}`,
   `{IVS_RUNNER_OPT_ROUNDTRIP | 2u, 5000u, 0, 0u}`). The log options should carry **no new flag
   bits at all** (presence of a non-NULL buffer is the switch), or bits above the tested ones.

**Primary recommendation:** put the format in a new pure library TU (`runner/ivsl.c` +
`runner/ivsl.h`, added to `IVS_RUNNER_SOURCES`), carry the log through two appended
`ivs_runner_options` fields and one appended `ivs_runner_result` field, do all file I/O in
`runner/main.c` through the existing `ivs_hostio_out_create`/`ivs_hostio_read`, make the
recorder write only the *applied prefix* the executor already counts, and make the equality
case run **A = scenario with input (record)** against **B = the same scenario with its `input`
lines removed (replay)** so that a replay that ignores the log reports `input.events 0` and
fails the guard.

---

## User Constraints (from CONTEXT.md)

> Copied verbatim from `.planning/phases/11-input-record-and-replay/11-CONTEXT.md`.
> The phase's own research findings amend four of these; the amendments are named in
> §"Frozen criteria that cannot be met as written" and are the planner's to record as
> deviations, never to edit into the context file (GUARDRAILS 2.10, lesson 21).

### Locked Decisions

- **`docs/formats/ivsl-v1.md` is committed before any code**, and the ancestry is
  proved with `git merge-base --is-ancestor`. Its grammar has **version, scenario id,
  profile id, a `content project|user` line, and `at_tick <n> c0=<HEX>
  c1=<HEX>` records** — and no field able to hold a content digest, an image name, a
  length, a frame byte, an audio byte or a path. An unknown directive is refused
  with a typed token. — **Reversibility:** costly — the format is a published
  contract; a later change means a version bump and a reader that accepts both.
- **Identity binding is not in the format.** When an operator wants it, they put
  `expect-sha256` in their own untracked scenario. A digest field in the log would
  make the log derived material under D11 and would be the one thing the format
  exists to avoid.
- **`--record-input` (exclusive create) and `--replay-input`** live in
  `runner/hostio.c`'s territory: operator files never touch the core. Exclusive
  create means an existing path is refused, not overwritten.
- **The log is text, is not in `tools/check/binary-ext.txt`, and is not
  gitignored.** A produced log is operator output, never a tracked artifact.
- `runner.record.replay-equals-straight` and `runner.record.format-roundtrip` in
  process; the refusal family (unknown directive, non-monotonic tick, tick beyond
  bound, event cap, undefined controller bit, wrong version, trailing garbage)
  each with a typed token and no state change; and **exactly one** process case
  for the command line (argument parsing, `--content-dir`, exit codes).
- **Two named faults**, `IVS_FAULT_RECORD_DROPS_LAST_EVENT` and
  `IVS_FAULT_REPLAY_APPLIES_LATE` (one tick), on digest domain 18, registered
  under TOOL-01 in the same commit as the ids that detect them.
- The equality case asserts **at least one event**, `input.consumed ==
  input.events`, an `at_tick` list equal to the recorded one, and **all nine
  domains observed**. Without those, a hook that does nothing matches every
  report — the exact 08-16 failure class.
- **Two judge self-controls**, each shown able to fail: `replay-differs` (one
  digest nibble changed → rejected) and `replay-inert` (a fake pair with zero
  applied events → rejected **by the guard**, not by the equality).
- `runner.record.no-content-digest` greps a produced log for a run of 32 or more
  hex digits and requires **zero hits**. The log holds nothing the images
  produced, so under D11 it is not derived material.

### Claude's Discretion

The plan chooses the exact token spellings, whether the process case is one id or
two, and where the `at_tick` bound comes from (a measured log-size probe sizes the
event cap).

### Deferred Ideas (OUT OF SCOPE)

- **A content-identity field in the log** — D15; use `expect-sha256` instead.
- **Promoting the process cases from in-process to real-executable** — the D15
  reversal trigger is a defect found only at the process boundary.
- **Snapshot file options** — deferred from Phase 8, still deferred.
- **A new gate step** — forbidden this milestone.

---

## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| TOOL-01 | A run can be recorded as an input-only log and replayed at machine-time boundaries, and the replay reproduces the recorded run's digest chain and applied-event chain, guarded against a vacuous match. | §1–§3 give the seam, the two chains and the input model; §6 gives the two must-fail faults with their forced TU; §7–§8 give the rights check and the cap; §9 gives the exact count delta. |

---

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|---|---|---|---|
| `ivsl` v1 grammar, writer, parser, refusal tokens | **runner library** (`runner/ivsl.c`) | — | The two named faults need owner TUs in `IVS_RUNNER_SOURCES` (faults.cmake:155); a pure byte-in/byte-out TU is the only place both the in-process suite and the fault executables can reach. |
| Which events were applied | **runner library** (`runner/exec.c`) | — | The applied count is `sum(rep.input_consumed)`, which only the executor sees (exec.c:1098-1100). |
| The event schedule used by a run | **runner library** (`ex_run_steps`) | — | The schedule is built at exec.c:968-988; a replay substitutes the log's array there. |
| Reading/writing the log file, exclusive create | **runner executable** (`runner/main.c` → `runner/hostio.c`) | — | GUARDRAILS 2.7; `O_EXCL` exists only in `hostio.c` (hostio.c:150) and `runner-scan` confines the POSIX feature macros there (runner-scan.sh:20-26). |
| The `input.events` report value under replay | **runner library** (`runner/report.c`) | — | It prints `s->n_inputs` (report.c:340-341); under replay the effective count is the log's. |
| The format document | **docs** (`docs/formats/ivsl-v1.md`) | — | GUARDRAILS 2.9: oracle first, ancestry proved. |
| Registration, ids, faults | **data + tests** | — | R9/R10. Zero new gate steps (GUARDRAILS 2.1). |
| Any machine behaviour | **core** — *no change* | — | The core already exposes everything needed: `ivs_run_report.input_consumed` (ivs_run.h:345), `ivs_machine_state_digest` domain 18 (ivs_machine.h:68). |

---

## 1. The 08-16 round-trip hook, read in full

### Where the seam is

| Thing | Location | What it does |
|---|---|---|
| The hook's option pair | `runner/ivs_runner.h:468-469` — `#define IVS_RUNNER_OPT_ROUNDTRIP 1u`, `IVS_RUNNER_ROUNDTRIP_MAX_CUTS 64u` | One flag bit; 64 cuts |
| The options struct | `runner/ivs_runner.h:473-486` | `flags`, `wall_timeout_ms`, `slice_ticks`, `max_ticks`, `audio_out_max_bytes`, `wav_rate`, `reserved` (must be 0), `video_out_max_bytes`, `roundtrip_ticks`. **No `struct_size`** — the header is runner-internal and fields are appended |
| The result struct | `runner/ivs_runner.h:497-506` | `outcome`, `exit_code`, `host_calls`, `report_lines`, `report_write_failed`, `reserved`, `roundtrip_cuts` |
| The validator | `runner/exec.c:204-213` (`ex_roundtrip_options_valid`) | Only the one flag bit may be set; the flag requires `roundtrip_ticks` in `1..IVS_RUN_TARGET_MAX`; flag clear requires 0 |
| The cut body | `runner/exec.c:232-259` (`ex_roundtrip_cut`) | `ivs_machine_snapshot_save` → `ivs_machine_reset(IVS_RESET_POWER_ON)` → `ivs_machine_snapshot_restore`, each its own statement with its status read; then two `ivs_machine_info` time reads |
| The invocation point | `runner/exec.c:1175` inside the slice loop — `if (!ex_roundtrip_after_slice(x, more)) return;` | Called after the slice's streams are captured and after the harness-fail check, before `if (!more) break` |
| The slice shortening | `runner/exec.c:1072-1074` | `if (x->rt_ticks != 0u && x->rt_next > x->now && q.target_ticks > x->rt_next) q.target_ticks = x->rt_next;` — the slice that would cross a multiple ends on it |
| The multiple bookkeeping | `runner/exec.c:264-276` (`ex_roundtrip_after_slice`) | Advances `rt_next` past machine time; makes the cut only when `more` (the slice loop goes on) |
| `rt_t0` | `runner/exec.c:1440` — `x.rt_t0 = info.time_ticks;` after the power-on reset that starts the run | The power-on machine time, used as the post-reset witness |

`[OBS]` All of the above read at those lines. `[INF]` The seam is directly reusable: the log
hook is a *third* caller of the same "after the slice, if the loop goes on" point, and needs no
change to the slicing or the stop accounting.

### How a hook is invoked

`[OBS]` The slice loop is one `for` over the scenario's steps; each iteration calls
`ivs_machine_run` with `target = min(limit, now + slice_ticks)` (exec.c:1071-1073), captures the
streams, computes `more = stop == IVS_STOP_TARGET && x->now < limit` (exec.c:1173), and then
calls the hook (exec.c:1175). A hook therefore runs **only between slices of a run-to that is
still going**; it never runs at the end of a run-to, at a step boundary, or after the last
slice. `[OBS]` That rule is stated at ivs_runner.h:459-461 and is why plan 08-16's five process
cases all cut inside a run-to.

`[INF]` For record/replay this means the *applied prefix* must be maintained incrementally in
the loop, not computed at the end from the scenario — an event applied in slice 3 is visible to
slices 4..n and the recorder must not re-derive it.

### What `host.calls` counts

`[OBS]` Exactly four call sites increment `x->rec->host_calls`:

| Line | Call | Increment |
|---|---|---|
| `runner/exec.c:160` | `host->cancel_requested` (in `ex_cancelled`) | +1 per slice |
| `runner/exec.c:170` | `host->now_ms` (in `ex_now_ms`) | +1 per slice, only when the host has a clock |
| `runner/exec.c:292`, `:296`, `:300`, `:307` | `host->read_file` / `host->read_user`, once per content role read | +1 per role loaded |

`[OBS]` `ex_between_slices` (exec.c:175-197) calls `ex_cancelled` then `ex_now_ms` before every
slice, which is why `check_roundtrip` asserts `host.calls(hook) - host.calls(plain) >= 2 * cuts`
(runner-process-tap.sh:367-370) and why the in-process case asserts
`rb.host_calls == ra.host_calls + rb.roundtrip_cuts` (runner_exec.c:2877-2878 — the fake host
has a cancel hook and no clock, so a cut costs exactly one).

**Consequence for this phase:** the log file must **not** be read or written through the host
vtable. If it were, `host.calls` would differ between the recorded and the replayed run and the
equality would have to exclude a second key. Read and written in `main.c` via `ivs_hostio_read`
and `ivs_hostio_out_*`, the two reports are equal **including** `host.calls` — a strictly
stronger comparison than 08-16's.

### Why the 08-16 body could do nothing and still match

`[OBS]` The oracle is `rx_roundtrip_compare` (runner_exec.c:2846-2889): both runs must end
`pass`, the two reports must be **byte-equal apart from the `host.calls` line**
(`rx_without_host_calls`, runner_exec.c:2802-2826), the plain run must make 0 cuts, the hook run
at least `min_cuts`, and the host-call arithmetic above must hold.

`[OBS]` A cut that performs no save, no reset and no restore leaves the machine exactly as it
was, so the report *is* the plain report — the comparison passes by construction. The two
liveness conditions that existed at the first landing (cuts ≥ 3, host-call arithmetic) are both
satisfied by a body that only increments `x->rt_cuts`, because the extra host calls come from
the **shortened slices**, not from the cut body. `[OBS]` 08-16-SUMMARY deviation 2 states this
in one sentence: *"A first probe showed that a body that did nothing at all passes every
equality test: an uncut run has the plain report, and the cut counter and `host.calls` only see
the shortened slices."*

`[OBS]` The repair (commit `336b3cb`, exec.c:232-259) reads machine time after the reset and
must equal `rt_t0`, and after the restore and must equal `x->now`; otherwise the run ends
harness-fail with detail `roundtrip-reset` / `roundtrip-restore`. The probe table in the SUMMARY
shows all three in-process ids failing on the no-op body.

`[INF]` The generalisation is *not* "add more counters". It is: **the guard must observe
something that only a working body can produce.** A counter incremented by the body is not that
(the body increments it whether or not it works); a *state read after the body ran* is.

### What the record/replay hook must assert that the round-trip hook did not

| # | Assertion | Why 08-16 has no analogue | What it catches |
|---|---|---|---|
| 1 | The log holds **≥ 1 event** | 08-16's analogue (cuts ≥ 3) had no second source of truth to be confused with | A recorder that writes a header only |
| 2 | The replayed run's `input.events` equals the **log's** event count, and `input.consumed == input.events` | 08-16 had one input source | **A replay that ignores the log** — the phase's real vacuity class |
| 3 | The log's `at_tick` list equals the list the recorded run **applied** (established by `input.consumed` and, for Lamp Tone, by the scenario's own six ticks) | 08-16's chain had no discrete list | A recorder that drops the last event, or records an unapplied one |
| 4 | The replay run is executed on a scenario **whose own `input` lines are absent** | 08-16 had nothing to substitute | The whole class: with the scenario's own inputs present, a replay that ignores the log produces the identical report |
| 5 | All nine `digest.*` lines are present and equal (non-`-`) | 08-16 compared the whole report, which already covers presence | A run that failed before the final observation |

`[INF]` Assertion 4 is the load-bearing one and it is **not** in the context's list. Without it,
criteria 2 and 3 are both satisfiable by a `--replay-input` that validates the file and then
ignores it.

---

## 2. The report and the two chains

### The nine digest lines

`[OBS]` Produced by the runner, not by a report writer of their own: `ex_final`
(runner/exec.c:1289-1316) calls `ivs_machine_state_digest(x->m, ex_domains[i], x->rec->digest[i])`
for `i = 0..8` (exec.c:1301), and `runner/report.c:120-129` prints them in the fixed key order.
The domain table is `ex_domains` (exec.c:103-106):

| Row | Report key | `IVS_DIGEST_*` | Value |
|---|---|---|---|
| 0 | `digest.cpu` | `IVS_DIGEST_CPU` = 1 | cpu state |
| 1 | `digest.mem` | `IVS_DIGEST_MEM` = 2 | memory |
| 2 | `digest.images` | `IVS_DIGEST_IMAGES` = 3 | loaded images |
| 3 | `digest.instance` | `IVS_DIGEST_INSTANCE` = 4 | instance rows |
| 4 | `digest.composite` | `IVS_DIGEST_COMPOSITE` = 0 | composite of 1..4 |
| 5 | `digest.video` | `IVS_DIGEST_VIDEO` = 16 | frame chain |
| 6 | `digest.audio` | `IVS_DIGEST_AUDIO` = 17 | level-change chain |
| 7 | `digest.input` | `IVS_DIGEST_INPUT` = **18** | applied-event chain |
| 8 | `digest.psglog` | `IVS_DIGEST_PSGLOG` = 19 | write-log chain |

`[OBS]` The nine values for the phase's target scenario are committed in
`data/golden/demo-lamp-tone-pass.digests` — `digest.input` is
`832906ce775d32a98ae607386ce4bbcdd4d08017e3b914b6089e50a5364c7761`. **The oracle for the
applied-event chain already exists and is frozen**; no new golden is needed (and R11 does not
fire).

### The applied-event chain

`[OBS]` Defined in `core/include/ivs/ivs_run.h:221-228`:

```
h0 = SHA-256("IVS-INPUT-LOG-V1"), h(n+1) = SHA-256(h(n) || be64(at_tick) || be32(controller[0..3]))
```

over the events **actually applied**, with their absolute `at_tick`. So `digest.input` *is* the
applied-event chain, cryptographically. The human-readable chain is the log's `at_tick` list.

`[OBS]` Application rule (ivs_run.h:216-221): *"An event takes effect at the first CPU-unit
boundary whose start tick is at or after its `at_tick` … Nothing is applied when a call ends.
The report returns `input_consumed` … `input_floor_tick` … `controller_now`."*

`[INF]` Therefore the recorded `at_tick` is the **declared** tick, not the boundary tick. Writing
the declared tick is lossless: the replay submits the same declared tick into a fresh machine
whose floor is 0, and the core applies it at the same boundary.

### Can the public API report an applied-event count? — measured

`[OBS]` **For an instance: no.** `ivs_info` (core/include/ivs/ivs_machine.h:183-215) has
`struct_size`, `phase`, `time_ticks`, `profile_index`, `reserved`, `slots[3]`,
`ticks_per_cpu_cycle`, `cpu_cycles_per_scanline`, `scanlines_per_frame`, `video_format`,
`nominal_frame_ticks`, `video_bytes_per_frame`, `audio_format`, `audio_bytes_per_frame`,
`reserved2`, `audio_sample_ticks`, `audio_sample_bytes`, `psg_log_record_bytes`,
`psg_log_records_per_frame`, `crystal_hz_num`, `crystal_hz_den`, `frame_phase_ticks`,
`video_width`, `video_height`, `video_slot_bytes`, `video_header_bytes`, `video_slack_frames`,
`video_row_lines`. **No input field at all.** This is the Phase 8 gap recorded in
`08-ACCEPTED-DEBT.md` §6 ("The applied-event count of an instance has no public getter").

`[OBS]` **For a run: yes.** `ivs_run_report.input_consumed` (ivs_run.h:345),
`input_floor_tick` (ivs_run.h:347), `controller_now[4]` (ivs_run.h:348). The runner already
accumulates it: `consumed += rep.input_consumed; x->rec->input_consumed = consumed;`
(exec.c:1098-1100) and prints it as `input.consumed` (report.c:343-344).

**Conclusion: this phase needs no API addition and therefore no R1 retarget.** `[INF]` The
Phase 8 gap only bites a *restored* instance (a snapshot does not report its count), which is
exactly the case 08-16 open item 1 describes; Phase 11 never restores. **R1 does not fire:**
`data/api/api-effects.json`, `core/include/ivs/ivs_status.h`, `data/expected/api-*.ids`,
`data/expected/pkg-consumer-calls.ids` and `tools/check/core-symbols-allow.txt` are all
untouched, because no `IVS_API` call and no public header changes.

`[OBS]` Supporting that: `ivs_runner_lib` is "built in both presets and never installed (D-24)"
(runner/CMakeLists.txt:19-21), `data/expected/pkg-install-files.ids` lists only
`include/ivs/*.h` and `lib/libivs_core.a`, and `tools/check/api-effects.cmake:174` globs
`core/include/ivs/*.h` only. A change to `runner/ivs_runner.h` is invisible to R1, R7 and
`pinned-defines`.

### The `input.events` key is the one report value replay changes

`[OBS]` `runner/report.c:340-341`:

```c
  case RF_INPUT_EVENTS:
    rp_u64(v, cap, s->n_inputs);
```

`s` is `rec->scn` — the **scenario's** count, not the run's. `[INF]` Under replay the effective
schedule is the log's, so this key must print the effective count. The minimal change is one
field on `ivs_runner_record` (e.g. `input_events_effective`) defaulted to `s->n_inputs` and
printed here; for every existing run the value is unchanged, so **no golden moves and no report
key changes**.

`[INF]` This also *is* the guard: with the effective count printed, a replay that ignores the
log reports `input.events 0` for a no-input scenario, and criterion 3's
`input.consumed == input.events` check fails on `0 == 0`… **no** — it *passes* `0 == 0`. So the
guard needs both `input.events == <log count>` (a positive number) **and**
`input.consumed == input.events`. Assertion 1 (≥ 1 event) is what stops `0 == 0` being read as
success. All three clauses are needed; any two are satisfiable vacuously.

---

## 3. The scenario grammar and the input model

### The `input` directive

`[OBS]` `dir_input` (runner/scenario.c:883-944) requires **exactly four tokens**:

```
input at_tick=<decimal> c0=<value> c1=<value>
```

- token 1: `at_tick=` (decimal), `at_cycle=` (decimal), or `at_frame=` → refused
  `IVS_SCN_R_AT_FRAME` (scenario.c:905-910)
- the unit must be uniform across the file (`IVS_SCN_R_MIXED_INPUT_UNIT`, scenario.c:920-923)
- ticks must be nondecreasing (`IVS_SCN_R_INPUT_UNSORTED`, scenario.c:924-926)
- `in.at > IVS_SCN_TIME_MAX` → `IVS_SCN_R_OUT_OF_RANGE` (scenario.c:917-919)
- token 2 must start `c0=`, token 3 `c1=` (scenario.c:928-930) — **there is no c2/c3 in the
  grammar**
- `parse_controller` (scenario.c:266-…) accepts either `0x` + 2..10 hex digits, or
  `NAME+NAME+…` symbolic names, and refuses bit 31 (`IVS_CTRL_RESERVED_MASK`) with
  `IVS_SCN_R_INPUT_RESERVED_BIT` (scenario.c:281-283)
- cap: `IVS_SCN_MAX_INPUTS` = 64 (ivs_runner.h:110), exceeded → `IVS_SCN_R_TOO_MANY_INPUTS`
- the scenario struct stores only `uint32_t controller[2]` (ivs_runner.h:283-287)

`[OBS]` `data/scenarios/demo-lamp-tone-pass.ivss` has six `input` lines at 100000, 220000,
340000, 460000, 580000, 700000 with symbolic names; the report's final
`input.controller.0 00000101` / `input.controller.1 20000000` match the last line
(`KEY_1+KEY_9` = bit 0 | bit 8, `SIDE_LOWER_LEFT` = bit 29).

### The bit layout and where it is pinned

`[OBS]` `core/include/ivs/ivs_run.h:237-269`: keypad bits 0..11, disc positions bits 12..27
(sixteen 22.5° steps clockwise from north), side codes bits 28..30 (`IVS_CTRL_SIDE_TOP` is the
two top buttons as **one** code), bit 31 = `IVS_CTRL_RESERVED_MASK` = `0x80000000u`. The header
comment states the pin: *"The bit indices are pinned by `data/expected/input-bits.ids`
(append-only): a moved bit would change every recorded input."*

`[OBS]` `data/expected/input-bits.ids` lists 31 rows `IVS_CTRL_<NAME> <bit>` 0..30, checked by
the `api-effects` step. The step's stat is `pinned-defines=120` over
`core/include/ivs/*.h` only (api-effects.cmake:174, :636).

### The tick/unit relationship

`[OBS]` `ex_run_steps` (exec.c:966-988) converts the scenario's events to master ticks once,
before the slice loop: `input_mult = s->input_unit == IVS_SCN_INPUT_AT_CYCLE ?
info->ticks_per_cpu_cycle : 1u` (exec.c:971), and each event's `at_tick` must be
`< IVS_RUN_TARGET_MAX` (`0x8000000000000000`, ivs_run.h:146) or the run is harness-fail
`input-out-of-range` (exec.c:981-985). The comment at exec.c:976 says it plainly:
*"words 2 and 3 stay 0."*

`[OBS]` The profile has `fields.input.controllers.value = 2`
(`data/profiles/intv-2609-ntsc-family-p1.json`), and a nonzero word at or above that index is
refused `UNSUPPORTED` / `INPUT_EXTRA_CONTROLLER` (ivs_run.h:207-212).

`[INF]` **This is what makes `at_tick <n> c0=<HEX> c1=<HEX>` lossless.** The core's chain
hashes four words; the log carries two; the other two are provably zero on this profile, so the
chain is reproduced exactly. The property is **profile-scoped** and must be written into
`docs/formats/ivsl-v1.md` as such: a profile with 3 or 4 connected controllers needs `ivsl` v2
with `c2=`/`c3=`. That is a format-design constraint, not an implementation detail.

### Expressibility and round-trip loss

| Quantity | Scenario grammar | `ivsl` v1 | Lossless? |
|---|---|---|---|
| event tick | `at_tick=<decimal>` | `at_tick <decimal>` | yes (same bound `IVS_SCN_TIME_MAX` = 2^62) |
| controller 0/1 | `c0=<hex or names>` | `c0=<8 upper hex>` | yes (bit 31 refused on both sides) |
| controller 2/3 | not expressible | not expressible | yes (both are provably 0 on this profile) |
| event order | nondecreasing enforced | nondecreasing enforced | yes |
| count | ≤ 64 | ≤ 64 | yes |
| scenario/profile identity | `id`, `profile` slugs `[a-z0-9-]` | same two slugs | yes |

`[OBS]` The slug charset is `copy_slug` (scenario.c:143-158): `is_lower(c) || is_digit(c) ||
c == '-'`, max `IVS_SCN_MAX_ID` = 48 / `IVS_SCN_MAX_PROFILE` = 64 (ivs_runner.h:117-118). **No
`/`, no `.`, no `:`, no `=`** — so no path is expressible in the header either.

---

## 4. The option parser and `hostio.c`

### The `strcmp` chain

`[OBS]` `runner/main.c:545-607` is one `if/else if` chain of `strcmp(a, "--option") == 0`, each
arm setting either a `const char **slot` (a path, "given twice" refused) or an `int *seen` plus
`lo`/`hi` (a number, "given twice" refused, `main_number` range-checked at main.c:265-288).
`--roundtrip-at-ticks` is the model (main.c:587-589):

```c
    } else if (strcmp(a, "--roundtrip-at-ticks") == 0) {
      seen = &have_roundtrip;
      lo = 1u;
      hi = IVS_RUN_TARGET_MAX;
```

and its value arm (main.c:641-643):

```c
    } else if (seen == &have_roundtrip) {
      opts.flags |= IVS_RUNNER_OPT_ROUNDTRIP;
      opts.roundtrip_ticks = v;
```

`[OBS]` The two new options are **path options**, not numeric ones: they take the
`slot = &log_path` form (like `--audio-raw`, main.c:553-554), with no `lo`/`hi` and no
`main_number`. `[OBS]` An unknown `--` word is refused `unknown option` (main.c:597-598), a
missing value `option without a value` (main.c:599-601), a repeat `option given twice`
(main.c:604-608); each prints one message line plus the usage text and exits 14
(`main_usage_error`, main.c:255-263).

`[OBS]` `--wav-rate needs --audio-wav` (main.c:661-663) is the precedent for a cross-option
requirement; the log options need `--record-input` and `--replay-input` to be **mutually
exclusive** (a usage error of the same shape).

### The usage text and the 4,095 limit

`[OBS]` The usage text is `static const char *const main_usage[]` with **two** elements
(main.c:61-123), printed in order by `main_usage_write` (main.c:124-129). Measured by summing
the escaped literals:

| Element | Chars | Limit |
|---|---|---|
| `main_usage[0]` (lines 62..86) | **1,876** | 4,095 |
| `main_usage[1]` (lines 87..123) | **2,647** | 4,095 |
| total | 4,523 | — |

`[INF]` Two options at roughly six usage lines each (~400 chars) fit in either element with
about 1,000 chars to spare. **No third literal is required** — but the plan must keep each
element under 4,095 or `-Woverlength-strings` fails under `-Werror` (the 08-16 deviation 1
precedent).

### Where operator files are opened, and with what modes

`[OBS]` `main_open_out` (main.c:291-303) wraps `ivs_hostio_out_create(o, path, mode)`; mode is
`IVS_HOSTIO_MODE_SHARED` = `0644u` for audio and the PSG log, `IVS_HOSTIO_MODE_PRIVATE` =
`0600u` for video and the PPM (hostio.h:47-48). The message on an existing path is
`ivs-run: <option>: output file exists, refusing to overwrite it` and the exit is 14
(main.c:295-298). Creation happens at main.c:773-820, **after** the scenario is read and parsed
(main.c:689-722) and after the user-content refusals (main.c:726-756), so a refused scenario or
a refused option creates no file.

`[OBS]` `main_discard` (main.c:325-336) closes and removes the file the run created, comparing
the recorded `(st_dev, st_ino)` so a replaced file is never removed
(`ivs_hostio_out_remove_if_same`, hostio.c:213-229). A harness-fail run removes its outputs
(main.c:898-925 for the close path, and the WR-01 rule stated at main.c:1114-1116).

### Exclusive create in C17 — what the project already does

`[OBS]` `O_EXCL` **is** POSIX and the project uses it, in exactly one file. `hostio.c:1-17`
declares `_POSIX_C_SOURCE 200809L` and `_DEFAULT_SOURCE` before its first include and says why;
`hostio.c:150` is the create:

```c
  fd = open(path, access | O_CREAT | O_EXCL | O_NOCTTY, (mode_t)mode);
```

`[OBS]` `tools/check/steps/runner-scan.sh:20-26` refuses any `#define`/`#undef` of
`_POSIX_C_SOURCE`, `_DEFAULT_SOURCE` etc. in **any runner file other than `hostio.c`**
(`runner-scan-posix-define`), and refuses a non-ISO header include outside `hostio.c`
(`runner-scan-header`).

`[INF]` **There is no portable C17 exclusive create**, and the project's answer is already
settled: the option's file is created in `hostio.c` through the existing
`ivs_hostio_out_create`, mode `0644` (a log is input, not derived art — the `0600` reason for
video does not apply). No new POSIX surface, no new file that may hold a feature macro.

`[OBS]` Reading the log back is `ivs_hostio_read` (hostio.c:74-134): `O_RDONLY | O_NOCTTY |
O_NONBLOCK`, `fstat` for `S_ISREG`, at most `cap + 1` bytes, and a refusal for a FIFO or a
directory. `[INF]` Reuse it with the log's own cap so a huge file is refused `TOO_LARGE` rather
than read.

---

## 5. The refusal family, each with a typed token

### Where the existing tokens live and how they are pinned

`[OBS]` There is **no** `data/expected/runner-refusals.ids` (measured: `ls data/expected` has
`runner-outcomes.ids` and `runner-scenarios.ids` and nothing else runner-shaped). The scenario
parser's 53 reasons live in `runner/ivs_runner.h:181-236` (`IVS_SCN_R_*`) with their token
strings in `ivs_scenario_reason_name` (scenario.c:1205-1290) and are pinned by **one test id per
token** in `data/expected/tests/runner.ids` (the `runner.parse.refuse.<token>` block, lines
44-84) plus `runner.parse.error-format` (runner.ids:42) which pins one full formatted line
(`"scenario-syntax line=4 reason=path-dotdot"`, runner_parse.c:982).

`[OBS]` So the token *spelling* is machine-pinned for exactly one token; every other spelling is
free but a rename would break the id that names it. The frozen document is the real oracle for
spellings.

**Therefore:** the log's tokens are pinned by (a) `docs/formats/ivsl-v1.md`'s refusal table,
frozen first, and (b) one `runner.record.refuse.<token>` id per token in
`data/expected/tests/runner.ids` (R9). No new ids file, no new gate step.

### The seven classes, with the token that fits this project's conventions

| # | Class | Rule | Recommended token | Existing analogue |
|---|---|---|---|---|
| 1 | unknown directive | a first token that is not `ivsl`, `scenario`, `profile`, `content`, `at_tick` | `unknown-directive` | `IVS_SCN_R_UNKNOWN_DIRECTIVE` → `unknown-directive` |
| 2 | non-monotonic tick | `at_tick` < the previous event's `at_tick` | `tick-order` | `IVS_SCN_R_INPUT_UNSORTED` → `input-unsorted` |
| 3 | tick beyond bound | `at_tick` > `IVS_SCN_TIME_MAX` (2^62 − 1 = 4,611,686,018,427,387,903) | `tick-bound` | `IVS_SCN_R_OUT_OF_RANGE` → `out-of-range` |
| 4 | event cap | more than 64 events | `event-cap` | `IVS_SCN_R_TOO_MANY_INPUTS` → `too-many-inputs` |
| 5 | undefined controller bit | bit 31 set in `c0`/`c1` | `controller-bit` | `IVS_SCN_R_INPUT_RESERVED_BIT` → `input-reserved-bit` |
| 6 | wrong version | the `ivsl` line's version is not `1` | `version` | `IVS_SCN_R_BAD_VERSION` → `bad-version` |
| 7 | trailing garbage | any byte after the last complete line, or a line that does not end in LF, or a NUL / non-ASCII / control byte | `trailing` | IVSC's `IVS_REASON_TRAILING` (ivs_machine.h:245) |

### The gap: seven tokens cannot cover the grammar

`[OBS]` A parser that reads `at_tick <n> c0=<HEX> c1=<HEX>` must also refuse: a line with the
wrong number of tokens, a decimal that is not a number, a hex field that is not eight hex
digits, a missing `ivsl` header, a missing `scenario`/`profile`/`content` line, a duplicated
header line, and (see §"Open Questions" O-3) a log whose recorded scenario id differs from the
scenario being replayed. The scenario parser has 53 reasons for a comparable grammar.

`[INF]` Two honest options, and the planner must pick one and record it as an amendment:

- **(a) Fold.** Define the grammar so classes 1 and 7 absorb the rest: any line that is not one
  of the five directives → `unknown-directive`; any malformed `at_tick` line (arity, bad
  decimal, bad hex, bad case, missing LF) → `trailing`. Literally seven tokens, at the cost of
  a `trailing` that means "almost everything".
- **(b) Extend to nine** by adding `bad-arity` and `bad-hex` (the two the grammar cannot avoid
  naming), which is **+2 ids** and moves `tests-expected` by +4 instead of +26→+30.

Recommendation: **(b)**, and record A-11-02. A token that means "something was wrong at the end
of the file, or maybe in the middle of a line" cannot be asserted as a distinct finding, and
GUARDRAILS 2.4 forbids a case that asserts only "it failed".

### "No state change"

`[OBS]` The scenario-refusal precedent is: exit 14, one stderr line, **no report on stdout**, and
nothing created (main.c:713-722 returns before main.c:773 creates any output). The log refusal
should land at the same point in the flow — read and parse the log right after
`ivs_scenario_parse` and before `ex_create` — so "no state change" is a property of the *order*,
not an assertion.

`[OBS]` For the in-process ids, "no state change" is asserted the way the state suite does it
(08-12/08-15): the output struct is poisoned (`0xA5`) before the call and must be untouched
after a refusal. `[INF]` The log parser's output should be an events array plus a header struct,
so the poisoning pattern transfers directly.

---

## 6. The two faults

### How a fault site is registered

`[OBS]` One line per fault in `data/expected/faults/runner.ids`, format
`<FAULT_ID> <must-fail> <allowed-prefixes> <must-pass>` (header at runner.ids:1). The site is a
`#ifdef IVS_FAULT_<ID>` in an owner TU. Three checks bind the two together:

1. `faults-verdict` scans every `#if* IVS_FAULT_[A-Za-z0-9_]*` in `core/src/**` and `runner/**`
   (faults-verdict.sh:169-183) and requires every site id to be listed
   (`fault-orphan-site`) and every listed non-`GEN_` id to have a site (`fault-missing-site`).
2. `tests/faults.cmake:155` refuses a fault whose owner TU is not in a module of the suite:
   `FATAL_ERROR "fault ${id}: owner ${owner} is not in a module of suite ${suite}"`.
3. The suite's fault executables are built from `IVS_FAULT_SUITE_<suite>_SOURCES` (the suite's
   test sources) plus **plain copies of every owner TU of the suite's modules**
   (faults.cmake:150-172). For suite `runner` the modules are `core runner`
   (tests/CMakeLists.txt:669-675), i.e. `IVS_RUNNER_SOURCES` — `exec.c frame_export.c outcome.c
   report.c runner_chain.c runner_sha256.c runner_vchain.c scenario.c` (runner/CMakeLists.txt:9-18).

`[OBS]` **The consequence, and the phase's hardest constraint:** a fault site in
`runner/main.c` or `runner/hostio.c` is a **configure error**, because those two files are in
`IVS_RUN_SOURCES` (runner/CMakeLists.txt:34-37), not `IVS_RUNNER_SOURCES`, and the CMake comment
says so: *"The executable ivs-run … adds main.c and hostio.c (IVS_RUN_SOURCES, never in the
fault framework: no in-process suite reaches them)"* (runner/CMakeLists.txt:13-15).

`[INF]` So both faults must sit in a library TU. This single fact decides the architecture: the
recorder and the replayer operate on **memory**, and `main.c` owns only the file.

### How `faults-verdict` judges

`[OBS]` For each fault: `build/gate/faults/<ID>/<suite>.txt` must exist, carry the gate nonce,
and list exactly the ids of `data/expected/tests/<suite>.ids`; the must-fail vector must be
`not ok`, every other failing vector must start with an allowed prefix, and the must-pass vector
must be `ok` (faults-verdict.sh:81-140). `c-faults` runs `build/dev/tests/ivs_fault_<ID>` once
per fault, in 4 lanes, with a watchdog (c-faults.sh:139-186).

### Which id each fault must fail

`[OBS]` The fault executables run the **in-process `runner` suite only** — `runner-process` is a
shell suite (`tests/runner-process-tap.sh`) and is never compiled into a fault executable. So a
process-level id can never be a must-fail, and the detecting id must be in
`data/expected/tests/runner.ids`.

| Fault | Site (proposed) | What it does | must-fail | must-pass |
|---|---|---|---|---|
| `IVS_FAULT_RECORD_DROPS_LAST_EVENT` | `runner/ivsl.c`, in the writer, after the applied count is known | writes `n-1` event lines instead of `n` | `runner.record.replay-equals-straight` | `runner.parse.diag-boot-pass` |
| `IVS_FAULT_REPLAY_APPLIES_LATE` | `runner/exec.c`, in `ex_run_steps` where the replayed schedule is built | `events[i].at_tick += 1` | `runner.record.replay-equals-straight` | `runner.parse.diag-boot-pass` |

`[INF]` Both are caught for the right reason and both are *only* caught by the equality case:

- The dropped event changes `digest.input` (one fewer chain step) and `input.consumed`, so the
  two reports differ and the guard's `input.consumed == input.events` also fails.
- The late event changes `at_tick` in the chain (`be64(at_tick)` is hashed), so `digest.input`
  differs even if the guest never reads a controller — which is why the **cheap boot diagnostic
  with synthetic events** is enough for the must-fail, and the 806k-tick Lamp Tone run can stay
  in the process case only (the 08-16 deviation 5 cost precedent).
- **`runner.record.format-roundtrip` does *not* catch the drop**: a writer that drops the last
  event consistently produces the same bytes both times. It is therefore a safe must-pass and
  must not be relied on as a detector. `[OBS]` This is a real trap: the two faults look like
  they should be caught by the round-trip id and are not.

### Is digest domain 18 "free"? — measured: no, and the criterion's wording is wrong

`[OBS]` Domain 18 **is** `IVS_DIGEST_INPUT` (core/include/ivs/ivs_machine.h:68) and it is live:
it is the applied-event chain (§2), it is in the golden, and it already carries **five** core
faults in `data/expected/faults/run.ids`:

| Fault | Line | must-fail |
|---|---|---|
| `INPUT_APPLY_AT_RUN_START` | run.ids:76 | `run.input.one-event` |
| `INPUT_LOG_SLICE_RELATIVE_TICK` | run.ids:79 | `run.input.overshoot-window` |
| `INPUT_NO_FLOOR_CHECK` | run.ids:81 | `run.input.refuse.below-floor` |
| `INPUT_STATIC_CARRY` | run.ids:84 | `run.input.isolation` |
| `INPUT_EXTRA_CONTROLLER_UNCHECKED` | run.ids:114 | `run.input.refuse.extra-controller-word` |

`[INF]` What is free is not the *domain* but the **runner-side record/replay path**: no existing
fault has a site in a runner TU on the recording or replaying of events. The two new faults are
correct as specified (they perturb what feeds domain 18), but the context's phrase "on digest
domain 18" must not be read as "a domain with no faults". Record as a note, not an amendment:
the criterion is satisfiable, the wording is ambiguous.

---

## 7. `runner.record.no-content-digest`

### Which suite

`[OBS]` Two suites are available and both are *existing*:
`data/expected/tests/runner.ids` (247 ids, in process, compiled into every runner fault
executable) and `data/expected/tests/runner-process.ids` (134 ids, driven by
`tests/runner-process-tap.sh`). The name in the context, `runner.record.no-content-digest`,
follows the in-process convention (`runner.parse.*`, `runner.exec.*`, `runner.judge.*`), so it
belongs to the **`runner`** suite.

### How it greps a produced log without writing into the tree

`[OBS]` **In process there is no file at all.** The recorder returns its bytes to the caller
(§"Architecture Patterns" P-2), so the case greps the buffer:

```c
/* shape, not a real case */
int hits = 0;
for (size_t i = 0; i + 32u <= log_len; i++) {
  size_t j = 0u;
  while (j < 32u && is_upper_hex(log[i + j])) j++;
  if (j == 32u) hits++;
}
IVS_CHECK(hits == 0);
```

`[OBS]` The process suite writes into a `mktemp -d` directory outside the work tree:
`tmp=$(mktemp -d "${TMPDIR:-/tmp}/runner-process-tap.XXXXXX")` (runner-process-tap.sh:127), and
every operator-output case writes there — the comment at runner-process-tap.sh:1206 says *"into
a mktemp directory (never the repository)"*, and the video case at :1338 repeats it. So the one
process case writes the real `.ivsl` into `$tmp/rec/` and greps **that** file with the same
pattern, never the tree.

### The check must be shown able to fail

`[INF]` By construction it cannot fire on a well-formed log (§"Provable input-only property"
below), so on its own it is exactly the "a check that cannot fail proves nothing" defect
(lesson 20). The same id must also run the identical grep over a **mutated copy** with
`digest.input 832906ce…` appended and require `hits >= 1`. One id, two assertions, a live
stand-in.

### The provable input-only property

`[OBS]` The grammar's only string fields are two slugs of `[a-z0-9-]` (≤ 48 and ≤ 64 chars,
scenario.c:143-158), its only decimal is a tick ≤ 2^62 − 1 (**19 digits**), and its only hex
fields are two 8-digit words. No 32-character run of `[0-9A-F]` can occur:

- a decimal run is at most 19 digits (2^62 − 1 = 4,611,686,018,427,387,903);
- every hex run is exactly 8 characters and is separated from its neighbour by ` c1=` / `\n`;
- the slugs contain no `A-F` and no 32-character unbroken run of `[a-z0-9-]` that is also
  `[0-9A-F]`-only is impossible because a slug with 32+ chars would have to be all digits —
  and `copy_slug` allows digits, so **check this**: a 32-digit scenario id is legal and *would*
  match. `[INF]` The pattern must therefore be `[0-9A-F]{32}` **with** a preceding-character
  guard, or — simpler and what I recommend — the header fields must be emitted with a prefix
  (`scenario `, `profile `) so a 32-digit slug is still preceded by a space and the check is run
  on the **event lines and header values** with the `scenario`/`profile` lines excluded and
  their lengths pinned instead (`id <= 48`, `profile <= 64`, both `[a-z0-9-]`).

This is the one place where the rights check has a real hole, and it is worth stating plainly:
`[OBS]` a slug may be all digits (scenario.c:151 permits `is_digit`), so a 32-digit id would
trip a naive `{32,}` grep. Measured: no current id is close (`demo-lamp-tone-pass` = 19 chars,
`intv-2609-ntsc-family-p1` = 24), but the check must not depend on that.

---

## 8. The event cap

### A realistic log, measured by arithmetic from the committed scenario

`[OBS]` `data/scenarios/demo-lamp-tone-pass.ivss` has six `input` lines; the golden reports
`input.events 6` and `input.consumed 6`. Simulating the writer's output byte for byte:

| Part | Bytes |
|---|---|
| `ivsl 1\n` | 7 |
| `scenario demo-lamp-tone-pass\n` | 29 |
| `profile intv-2609-ntsc-family-p1\n` | 34 |
| `content project\n` | 16 |
| **header total** | **85** |
| six event lines at 39 bytes each (`at_tick 100000 c0=00000001 c1=00000000\n`) | **234** |
| **whole log** | **319 bytes** |

`[OBS]` Measured with `wc -c` on the constructed header (85) and a `python3` sum over the six
lines (234) — the arithmetic, not a guess. The six controller words are
`00000001`, `00000101`, `00000000`, `10000000`, `00000000`, `00000101` (c0) and
`00000000`, `00000000`, `00010000`, `00010000`, `00000000`, `20000000` (c1), derived from the
symbolic names and `data/expected/input-bits.ids`.

### A synthetic long run

`[OBS]` Worst case per the grammar's own bounds:

| Part | Bytes |
|---|---|
| `ivsl 1\n` | 7 |
| `scenario ` + 48 + `\n` | 58 |
| `profile ` + 64 + `\n` | 73 |
| `content project\n` | 16 |
| **max header** | **154** |
| one event line at its longest (`at_tick ` + 19 digits + ` c0=` + 8 + ` c1=` + 8 + `\n`) | **52** |
| 64 events | **3,328** |
| **max log** | **3,482 bytes** |

### The defensible cap, with the arithmetic

`[OBS]` The scenario grammar caps events at `IVS_SCN_MAX_INPUTS` = 64 (ivs_runner.h:110); the
core accepts up to `IVS_INPUT_EVENTS_MAX` = 65,536 (ivs_run.h:230). The recorder can only ever
produce what a scenario supplied, so:

- **event cap = 64**, the same constant the scenario parser enforces. Not a new number.
- **file cap = 4,096 bytes** (`154 + 64 × 52 = 3,482`, leaving 614 bytes of slack for a
  directive the grammar may grow). Any byte past 4,096 is refused `file-too-large` before it is
  parsed, exactly as `IVS_SCN_MAX_BYTES` = 65,536 does for a scenario (ivs_runner.h:107) — but
  the log is 19× smaller, so its own smaller bound is honest and cheap to test.
- **line cap = 64 bytes** (`at_tick ` + 19 + ` c0=` + 8 + ` c1=` + 8 + `\n` = 52, plus 12 for
  slack), refusing a 65-byte line the way the scenario's 256-byte cap does.

`[INF]` If the planner prefers one number, `IVS_SCN_MAX_BYTES` is defensible too — but then the
"event cap" refusal has to be tested with a file that is *under* the byte cap, which needs 64
events anyway, so the 64-event cap is doing the work regardless.

---

## 9. The counts

### The current token, measured

`[OBS]` `sh tools/gate.sh` on `8682a36` (this tree, before any phase-11 edit) ends:

```
gate: PASS steps=38/38 controls=395/395 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=235 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=82 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=670 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=235/235 tracked=2633 binaries-listed=0 terms=9 doc-files=62 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2633 history-commits=1 gates=109 lines=333 input-rows=31 input-policies=11 input-profile-fields=31 files=2633 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=16 cases=13 sources=12 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 docs=63 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

(`gate-time total=89`, `gate-cpu user=153.82 sys=129.32`, `EXIT=0`.)

### The recommended id set, and why each id exists

`[INF]` **`runner` suite (12 ids):**

| Id | Kind |
|---|---|
| `runner.record.replay-equals-straight` | the equality case (criterion 2) |
| `runner.record.format-roundtrip` | write → parse → write byte-identical (criterion 2) |
| `runner.record.no-content-digest` | rights check + its stand-in (criterion 5) |
| `runner.judge.record-replay-differs` | judge self-control, one digest nibble changed (criterion 3) |
| `runner.judge.record-replay-inert` | judge self-control, zero applied events → rejected **by the guard** (criterion 3) |
| `runner.record.refuse.unknown-directive` | class 1 |
| `runner.record.refuse.tick-order` | class 2 |
| `runner.record.refuse.tick-bound` | class 3 |
| `runner.record.refuse.event-cap` | class 4 |
| `runner.record.refuse.controller-bit` | class 5 |
| `runner.record.refuse.version` | class 6 |
| `runner.record.refuse.trailing` | class 7 |

`[OBS]` The two self-controls **must** be named `runner.judge.<slug>`, not
`runner.record.judge.<slug>`: `important-gates.sh:142` resolves a `mutant` mechanism with
`$1 ~ /^[a-z0-9-]+[.](judge|mutant)[.][^ ]+$/`, i.e. `<suite>.judge.<slug>` with no dots in the
suite token. `runner.judge.refpcm-floor-rounding` (runner.ids:214) is the precedent.

**`runner-process` suite (1 id):** `runner-process.record.command-line` — argument parsing,
`--content-dir`, exit codes, the exclusive-create refusal, and the Lamp Tone replay against the
committed golden.

Total **13 ids** (or 15 if A-11-02 option (b) adds `bad-arity` and `bad-hex`).

### The predicted token

| Field | Now | Predicted | Why |
|---|---|---|---|
| `steps` | `38/38` | **`38/38`** | frozen; GUARDRAILS 2.1 |
| `controls` | `395/395` | **`395/395`** | no new control directory; the 08-16 precedent ("no fault, no control") |
| `fault-runs` | `235` | **`237`** | +2 faults, one run each |
| `faults` | `235/235` | **`237/237`** | +2 faults, both detected |
| `gates` | `109` | **`110`** | 4 new registry rows under **one** new gate name (2 faults + 2 judge mutants) |
| `lines` | `333` | **`337`** | `lines` counts registry rows (important-gates.sh:168, `lines++` per non-comment row) |
| `tests-expected` | `4958` | **`4984`** | 13 ids × 2 presets = +26 |
| `tests-executed` | `4954` | **`4980`** | `tests-expected − tests-skipped` |
| `tests-skipped` | `4` | **`4`** | no new skip |
| `runner-files` | `16` | **`18`** | `runner/ivsl.c` + `runner/ivsl.h` (runner-scan counts `runner/**/*.{c,h}`) |
| `docs` | `63` | **`64`** | README + CHANGELOG + `docs/**/*.md` (61 today) + the format doc |
| `doc-files` | `62` | **`63`** | glossary's README + `docs/**/*.md` (61) + the format doc |
| `tracked` | `2633` | **`2633 + N`** | `git ls-files | wc -l`; `N` = 3 (doc + 2 sources) + the phase's own PLAN/SUMMARY records |
| `files` | `2633` | **`2633 + N`** | same list |

`[OBS]` Every other field must **not** move, and here is why for the ones at risk:

| Field | Must stay | Reason |
|---|---|---|
| `claims` / `provisional` | `28` / `27` | no claim; `[nc]` is the waiver and adds no row |
| `goldens` | `29` | the digest oracle is the existing `data/golden/demo-lamp-tone-pass.digests` (R11 does not fire) |
| `reports` / `host-dependent` / `release-only` | `30` / `3` / `2` | no new scenario row in `runner-scenarios.ids` |
| `api-calls`, `api-statuses`, `api-rows`, `public-symbols`, `pinned-defines` | `19`, `15`, `61`, `19`, `120` | no core header, no `IVS_API` call (R1 does not fire) |
| `pkg-selftest`, `consumer-lines`, `prefix-files` | `19/19`, `164`, `11` | no installed-consumer change (R7 does not fire) |
| `state-rows`, `state-cuts` | `88`, `18` | no snapshot change |
| `input-rows`, `input-policies`, `input-profile-fields` | `31`, `11`, `31` | no profile or literal-table change |
| `c-presets`, `ctest-runs` | `2`, `2` | no preset change |
| `cases` | `13` | `runner-selftest`'s own case count; no new selftest case |
| `sources`, `refs-verified` | `12`, `3/4` | no new source record |
| `terms` | `9` | no new glossary term |
| `listings`, `manifests` | `7`, `11` | a `docs/` file needs no listing row |
| `mode` | `tree` | no control-tree change |

### The ordering constraint nobody has written down

`[OBS]` The accepted-requirement set is **derived** from the Traceability table's status
(important-gates.sh:91-97, `st ~ /^Complete/`) minus the exclusion ledger. `TOOL-01` is
`| TOOL-01 | Phase 11 | Pending |` (REQUIREMENTS.md:104). A registry row naming TOOL-01 while it
is `Pending` is `important-gates-unknown-id` (important-gates.sh:167-171: `!($4 in accepted)`);
a TOOL-01 marked `Complete` with no registry row is `important-gates-complete-uncovered`
*and* `important-gates-uncovered-gate`.

`[INF]` **The REQUIREMENTS flip, the 4 registry rows, the 10 toy-registry lines (R2) and the 2
toy requirement records must all land in one commit.** A tracer commit that flips TOOL-01 early
takes the gate red; so does a commit that adds the rows while TOOL-01 is Pending. This is the
same shape as Phase 10's "the three requirement flips land with their five ledger rows in one
commit" (STATE.md, plan 10-06).

`[OBS]` R2's toy-registry limb **does** apply here: the derived accepted set is the must-be-covered
set, so accepting TOOL-01 needs one toy line in each of the ten whole-file toy registries plus a
row in each of the two toy requirement records of `tools/check/controls/_good/tree` — unless
TOOL-01 takes an **exclusion-ledger row** instead, which is what Phase 10 did for PKG-05 and
TOOL-04. `[INF]` TOOL-01 must **not** be excluded: its mechanisms are real (two faults, two
judges), so it belongs in the registry, and the toy-registry work is due. The measured precedent
for the size: "the ten toy registries over `git diff --numstat` (17/0 each)" (STATE.md, Phase 9).

### Cost

`[OBS]` Measured on this tree, dev preset (sanitized, `-O1`), three alternated runs each:

| What | real (s) | user (s) |
|---|---|---|
| `build/dev/tests/ivs_tests_runner` (the whole `runner` suite) | 0.92 / 0.91 / 0.93 | 0.77 / 0.77 / 0.78 |
| `build/dev/tests/ivs_fault_RUNNER_SLICE_DROPS_REMAINDER` (one fault executable) | 0.40 / 0.38 / 0.38 | 0.39 / 0.37 / 0.37 |

Load average at record time: `6.66 8.49 10.28` on 18 cpus — **not an idle host** (lesson 23: keep
the reading and say so).

`[INF]` Estimate, to be replaced by measurement in the plan (GUARDRAILS 2.5):

| Item | Local cpu-s |
|---|---|
| 12 in-process ids on the 2,084-tick diagnostic (≈ 24 short runs) | ≈ 0.02 |
| the same 12 ids inside each of **21** runner fault executables (19 existing + 2 new) | ≈ 0.6 |
| 2 new fault executables at ≈ 0.40 each | ≈ 0.8 |
| the process case, ≈ 4 `ivs-run` launches (dev ≈ 0.11 each, release ≈ 0.01) | ≈ 0.45 |
| **total** | **≈ 1.9 local cpu-s** |

`[INF]` At the recorded hosted factor 1.675 and `build_s` ≈ 46 that is ≈ 3.2 hosted cpu-s and
`+0.07` in `r` — inside the milestone's ≈ 21 hosted cpu-s headroom (STATE.md Recorded
Constraints item 1).

`[INF]` **The in-process ids must stay on the cheap diagnostic.** If the equality case ran
`demo-lamp-tone-pass` (806,568 ticks, 13 frames, 30 write-log records) it would run 21 times in
the fault executables; at ≈ 0.08 s a run that is ≈ 3.4 s of the ≈ 1.9 above, i.e. the phase
would nearly double. This is exactly the 08-16 deviation 5 trade, and the same answer applies:
cheap scenario in process, real scenario in the process case.

---

## 10. Rights and text

| Check | Result |
|---|---|
| `ivsl` in `tools/check/binary-ext.txt`? | **No.** The list ends `… ppm vraw ivsn` (binary-ext.txt:26-27). Nothing to add. |
| `ivsl` in `.gitignore`? | **No.** The rights-by-name block lists `.int .bin .rom .itv .cfg .cc3 .ivsc .wav .ppm .vraw .ivsn .grom .img` and no `ivsl`. Nothing to add. |
| `integrity` ASCII rule on `docs/formats/ivsl-v1.md`? | **Not applicable.** The ASCII rule is scoped to `^(data|fixtures|tools|[.]github|core|include|tests|cmake|runner)/` or a top-level `CMakeLists.txt` (integrity.cmake:68-73). `docs/` is not in that set. |
| `integrity` CRLF / BOM / NUL / empty on the doc? | **Applies.** Those rules are unconditional for a non-binary file (integrity.cmake:55-66). |
| `integrity` SPDX on the doc? | **Not applicable.** The SPDX pattern (integrity.cmake:78-83) covers `tools/**.sh|cmake`, workflows, `core|include|tests` sources, `cmake/*.cmake`, CMakeLists, and `runner/**.c|h|sh|cmake` — not `docs/`. |
| `integrity` SPDX on a new `runner/ivsl.c` / `ivsl.h`? | **Applies.** `^runner/.*[.](c|h|sh|cmake)$` (integrity.cmake:82). Each needs `SPDX-License-Identifier: MIT` in the first lines. |
| `integrity` ASCII on the new C files? | **Applies.** `^runner/` is in the ASCII set; keep them pure ASCII. |
| `text` claim words on the doc? | **Applies, and will bite.** The step scans `docs/**/*.md` for `supports|compatible|passes|accurate|verified|runs|plays|playable|looks|sounds` (text.sh:22-23) and requires `[claim:CL-NNNN]` or `[nc]` on the line. `docs/formats/ivsc-v1.md` has **zero** `[nc]` because it happens to avoid all ten; a log format doc will want "runs" and "passes". Every such line needs `[nc]`. |
| `glossary` bare-clock on the doc? | **Applies.** The tripwire is the word `clock`/`clocks` over README and `docs/**/*.md`, exempt only for `input clock`, `STIC clock`, `PSG chip clock` or a line carrying `[nc]` (glossary.sh:57-62). |
| `files` step | Nothing to declare: the log is operator output and is never committed. A tracked `.ivsl` would be a *text* file with no rights gate, which is why criterion 5's "not gitignored" matters — an operator may keep one, and it must not be silently dropped. |

---

## Frozen criteria that cannot be met as written

Each is a **recorded amendment** for the planner, in the shape Phase 9 and Phase 10 used. None
of these edits a frozen file (GUARDRAILS 2.10, lesson 21).

### A-11-01 — "all nine domains observed" is unsatisfiable in process

**Criterion (ROADMAP Phase 11 #3, CONTEXT "The liveness guards are the point"):** *"the equality
case asserts at least one event, `input.consumed == input.events`, an `at_tick` list equal to the
recorded one and **all nine domains observed**."*

**Measurement.** A domain is "observed" when its delivered byte or event count is nonzero
(lesson 2; `main_count_observed`, main.c:393-409; the golden's `# domains:` line,
`data/golden/demo-lamp-tone-pass.digests:5`). The in-process suite's fake host carries exactly
two images — `boot-stub.exec` and `diag-cpu-1.ivsc` (`fh_init`, runner_exec.c:223-228) — and the
boot diagnostic produces **no video, no audio and no PSG writes**: the golden for the diagnostic
is `data/golden/diag-cpu-1.digests`, and the round-trip summary records it as "frame 0, no
display" with no audio and no log. So only **6 of 9** domains are observed in process: cpu, mem,
images, instance, composite, input. Video, audio and psglog are unobserved.

**Options.** (a) Move the nine-domain clause to the process case on `demo-lamp-tone-pass`, whose
golden observes all nine (`video=observed audio=observed input=observed psglog=observed`,
digests:5) — and whose replay can be compared against the committed golden. (b) Run Lamp Tone in
process, at ≈ 3.4 s of extra cost across the 21 fault executables (§9).

**Recommendation: (a).** The in-process equality asserts ≥ 1 event, `input.consumed ==
input.events`, the `at_tick` list, all nine `digest.*` lines present and equal (non-`-`), and
whole-report byte equality; the process case asserts the four live counts are nonzero on both
runs and compares `digest.input` with the committed golden. Criterion 3 is met **as a phase**,
not by one id.

### A-11-02 — seven refusal tokens cannot express the grammar's refusal surface

**Criterion (#4, and CONTEXT "one per class, seven of them"):** seven classes, one token each.

**Measurement.** §5. A four-token line grammar with a header, a decimal and two hex fields needs
at least arity, number and hex refusals in addition to the seven, or a `trailing` token that means
"something, somewhere".

**Recommendation:** nine tokens (add `bad-arity`, `bad-hex`), **+2 ids**, `tests-expected`
`+4`. Alternative if the planner prefers literal compliance: fold into `trailing` and record that
`trailing` carries two meanings, which GUARDRAILS 2.4 makes uncomfortable.

### A-11-03 — "the replay could ignore the log" is a vacuity class the context does not name

**Criterion (#2, #3):** the equality case and its guards.

**Measurement.** §1, "What the record/replay hook must assert". With the scenario's own `input`
lines present in the replay run, a `--replay-input` that parses the file and then ignores it
produces a **byte-identical** report, satisfies `input.consumed == input.events`, satisfies
"≥ 1 event" (the scenario has six), satisfies the `at_tick` list, and observes the same domains.
Every guard in the frozen list passes.

**Recommendation:** the replay run must use a scenario with **no `input` lines** — the process
case generates one in `$tmp` with `sed '/^input /d'` from the real scenario (the `id` line is
kept, so `scenario.id` still matches), and the in-process case builds its scenario text with the
existing `rx_replace` helper. Add assertion 2 of §1 as an explicit clause:
`input.events == <log count>` **and** `input.consumed == input.events`. Record A-11-03 as the
phase's restatement of lesson 20.

### A-11-04 — the two faults cannot have sites where the context implies

**Criterion (#4, MILESTONE-CONTEXT item 4):** the two named faults, "on digest domain 18".

**Measurement.** §6: `tests/faults.cmake:155` refuses a site in `runner/main.c` or
`runner/hostio.c`, and domain 18 is not fault-free (five core faults already sit on it).

**Recommendation:** sites in a new `runner/ivsl.c` (the writer) and `runner/exec.c` (the replay
schedule), i.e. `IVS_RUNNER_SOURCES` TUs; the "domain 18" phrase is read as "the faults are
detected through the input chain", which they are. No criterion is lost; the plan's
`files_modified` and the site placement are what change.

### A-11-05 — `runner.record.no-content-digest` cannot fail without a stand-in

**Criterion (#5):** "finds zero runs of 32 or more hex digits in a produced log".

**Measurement.** §7: by construction no well-formed log can contain a 32-character `[0-9A-F]`
run (a decimal is ≤ 19 digits; a hex field is 8), so the check passes on every log the writer can
produce — the lesson-20 defect. It also has a hole in the other direction: a slug may be all
digits, so a 32-digit scenario id would match a naive `{32,}` grep.

**Recommendation:** one id, two assertions — zero hits on the produced log, and ≥ 1 hit on a
mutated copy with a `digest.input <64 hex>` line appended — plus pin the header fields' shape
(`[a-z0-9-]{1,48}`, `[a-z0-9-]{1,64}`) instead of relying on the grep for them.

---

## Standard Stack

No new dependency. `[OBS]` The phase uses what is already there:

| Component | Version | Purpose | Why this and not something else |
|---|---|---|---|
| C17 + CMake | `CMakePresets.json` dev/release | the new TU and the ids | D1; nothing in the phase needs a library |
| `ivs_core` public API | in tree | `ivs_machine_state_digest`, `ivs_machine_info` | the digest chain and the time reads already exist |
| `runner/hostio.c` | in tree | exclusive create, bounded read | the only file allowed POSIX (runner-scan) |
| the harness (`tests/harness/ivs_test.c`) | in tree | TAP result files | every suite uses it |

**Installation:** none.

**Version verification:** not applicable — no external package is added, so the Package
Legitimacy Audit is empty (see below).

## Package Legitimacy Audit

**Not applicable.** This phase installs no external package, in any ecosystem. `[OBS]` The
build's dependencies are the C standard library, CMake ≥ 3.25 (integrity.cmake:16) and the
project's own targets; `CMakeLists.txt` adds no `find_package` for the phase's work and the
runner links only `ivs_core`, `ivs_refpcm`, `ivs_warnings`, `ivs_san`
(runner/CMakeLists.txt:22-31). No `npm`/`pip`/`cargo` invocation exists anywhere in this phase's
plan surface. The `package-legitimacy` seam is therefore not run.

## Architecture Patterns

### System Architecture Diagram

```
                       operator command line
                               |
                    runner/main.c  (ISO C17, no POSIX)
                    |        |             |
     ivs_hostio_read  |        |  ivs_hostio_out_create (O_EXCL)
     (the .ivsl in)   |        |  (the .ivsl out)          \
                      v        v                            v
             +----------------------------------+      [ log bytes in memory ]
             |  ivs_runner_execute              |            ^   |
             |  (runner/exec.c)                 |            |   |
             |                                  |            |   |
             |  ex_run_steps:                   |            |   |
             |    schedule = scenario inputs    |            |   |
             |             OR log events  ------+------------+   |
             |    slice loop:                   |                |
             |      ivs_machine_run(...)        |                |
             |      consumed += input_consumed  |                |
             |      [round-trip hook]           |                |
             |    at the end:                   |                |
             |      applied prefix ------------>+--> ivs_runner_log_build()
             |      ex_final: 9 digests         |        (runner/ivsl.c)
             +----------------------------------+
                      |                  ^
                      v                  |
             runner/report.c        ivs_runner_log_parse()
             (input.events =        (runner/ivsl.c)
              effective count)             ^
                      |                    |
                      v                    |
                 report on stdout     [typed refusal: no report,
                                       exit 14, one stderr line]
```

Entry points: the command line (`main.c`). Processing stages: parse the scenario → parse the log
(if replaying) → create the machine → run in slices, substituting the log's schedule and
accumulating the applied prefix → build the log bytes (if recording) → write the report → write
or remove the log file. Decision points: replay vs record vs neither; a log refusal (before any
file or instance exists); a harness failure (removes the log file). External boundaries: the
filesystem, through `hostio.c` only.

### Recommended Project Structure

```
runner/
├── ivsl.c          # NEW: ivsl v1 writer + parser + refusal tokens (pure, no I/O)
├── ivsl.h          # NEW: the format constants, the events struct, the refusal reasons
├── exec.c          # the schedule substitution, the applied prefix, the replay fault site
├── report.c        # input.events prints the effective count
├── main.c          # --record-input / --replay-input, the file, the refusals
└── hostio.c        # unchanged; the exclusive create already exists
docs/formats/
└── ivsl-v1.md      # NEW: the frozen oracle, committed first
data/expected/
├── tests/runner.ids          # +12 ids
├── tests/runner-process.ids  # +1 id
├── faults/runner.ids         # +2 faults
└── important-gates.ids       # +4 rows
```

### P-1: The log is a byte array in the library, a file only in `main.c`

**What:** `ivs_runner_log_build(header, events, n, buf, cap, &written)` and
`ivs_runner_log_parse(bytes, len, &out, &err)` live in `runner/ivsl.c`, a pure TU.
`ivs_runner_options` gains `{const uint8_t *log_in; uint64_t log_in_bytes;}` and
`{uint8_t *log_out; uint64_t log_out_capacity;}`; `ivs_runner_result` gains
`uint64_t log_out_bytes`.

**When to use:** always — it is the only placement the fault framework permits (§6).

**Why:** `tests/faults.cmake:155` + GUARDRAILS 2.7 + the fact that the in-process suite has
no file system.

### P-2: The recorder writes the applied prefix, not the scenario

**What:** `ex_run_steps` already accumulates `consumed += rep.input_consumed`
(exec.c:1098-1100). At the end of the run the recorder writes `events[0..consumed)` — the prefix
the core actually applied.

**Why:** an event beyond the run's end is never applied, and writing it would make the log claim
an event the recorded run did not have. The core's application rule (ivs_run.h:216-221) makes
"applied" exactly "consumed", and the prefix is exact because the array is nondecreasing and the
core applies in array order.

### P-3: `input.events` prints the effective count

**What:** one field on `ivs_runner_record`, defaulted to `s->n_inputs`, printed at
report.c:340-341.

**Why:** under replay the schedule is the log's, so `s->n_inputs` is a lie; and the effective
count is what makes "a replay that ignores the log" observable. Zero golden movement: for every
existing run the two values are identical.

### P-4: The equality case runs A-with-input against B-without-input

**What:** A = the scenario with its `input` lines, `--record-input`; B = the **same scenario with
the `input` lines deleted**, `--replay-input`. Reports must be byte-identical.

**Why:** with the scenario's own inputs present in B, a replay that ignores the log is
indistinguishable from one that works (A-11-03).

### Anti-patterns to avoid

- **A new flag bit at `2u`.** Three rows of `runner.exec.roundtrip-option-range` assert that bit
  2 is refused (runner_exec.c:2935-2937). Use no flag at all, or a bit above the tested ones.
- **Parsing the log inside `ivs_runner_execute`.** The refusal would then have to become a
  harness-fail report, changing the outcome vocabulary; and "no state change" would become a
  claim about a machine that already exists. Parse before `ex_create`, in `main.c`.
- **Recording from `scn->inputs` at the end.** It re-derives what the core did; it cannot see a
  run that ended early and it cannot be faulted at the point the log is written.
- **A `trailing` token that covers everything.** §5 A-11-02.
- **Comparing reports with `host.calls` excluded.** It is unnecessary here (§1) and excluding it
  is exactly the weakness `rx_without_host_calls` had to work around.

## Don't Hand-Roll

| Problem | Don't build | Use Instead | Why |
|---|---|---|---|
| exclusive create | a `stat`-then-`fopen` dance | `ivs_hostio_out_create` (hostio.c:150) | TOCTOU; a dangling symlink defeats `stat`; `O_EXCL` refuses it (hostio.h:53-56) |
| bounded file read | `fopen`/`fseek` | `ivs_hostio_read` (hostio.c:74) | refuses a FIFO, a directory and an oversize file; no blocking open |
| the applied-event count | a second bookkeeping pass | `sum(rep.input_consumed)` (exec.c:1098) | it is the core's own number |
| the applied-event chain | a second hash | `digest.input` / `ivs_machine_state_digest(…, IVS_DIGEST_INPUT, …)` | already defined and already in the golden |
| hex/controller parsing rules | a fresh table | `data/expected/input-bits.ids` + `IVS_CTRL_*` | the bits are pinned; a second table would drift |
| the log's removal on failure | `unlink(path)` | `ivs_hostio_out_remove_if_same` (hostio.c:213) | never removes a file that was replaced |
| scenario construction in a test | string assembly | `rx_scn` / `rx_replace` (runner_exec.c:243-256, and its users) | already the suite's idiom, already bounded |

**Key insight:** every one of these already exists *because a previous phase was burned by not
having it* — the exclusive create by T-05-23, the identity-checked removal by review WR-01, the
bounded read by the FIFO pitfall. Re-deriving any of them re-opens a closed finding.

## Common Pitfalls

### Pitfall 1: the vacuous replay (the 08-16 class, one level up)
**What goes wrong:** the equality case passes while `--replay-input` ignores its file.
**Why:** the recorded run and the straight run are the same scenario, so their reports are equal
by construction unless the replay run's scenario differs.
**How to avoid:** replay into a scenario with no `input` lines; assert `input.events == log count`
and `input.consumed == input.events` and `input.events >= 1`.
**Warning signs:** the equality case passes on a `--replay-input` that only checks the file's
existence; the guard's three clauses pass with `input.events 0`.

### Pitfall 2: a fault site in `main.c`
**What goes wrong:** `cmake` fails at configure with `fault … is not in a module of suite runner`.
**Why:** `IVS_RUN_SOURCES` is outside the fault framework.
**How to avoid:** put both sites in `IVS_RUNNER_SOURCES` TUs.
**Warning signs:** the fault executable builds but the site's `#ifdef` has no effect — which
cannot happen here, because the build refuses it.

### Pitfall 3: `input.events` unchanged under replay
**What goes wrong:** a replay of a 6-event log into a no-input scenario reports `input.events 0`,
so the equality fails for a reason that looks like the guard working.
**Why:** report.c:341 prints the scenario's count.
**How to avoid:** P-3.

### Pitfall 4: the log's event cap tested with a file that is also over the byte cap
**What goes wrong:** the `event-cap` case fires `file-too-large` instead.
**Why:** 65 events × 52 bytes = 3,380 < 4,096, so it is fine at 65 — but 128 events is not.
**How to avoid:** build exactly 65 event lines (3,380 + 154 = 3,534 bytes) for `event-cap`, and a
separate 4,097-byte file for `file-too-large`.

### Pitfall 5: a claim word in the format document
**What goes wrong:** `text-claim-untagged: docs/formats/ivsl-v1.md:<line>` takes the gate red.
**Why:** "runs", "passes", "verified" are claim words (text.sh:22-23).
**How to avoid:** `[nc]` on the line, or avoid the word.

### Pitfall 6: the TOOL-01 flip landing in its own commit
**What goes wrong:** `important-gates-complete-uncovered` + `important-gates-uncovered-gate`.
**Why:** the accepted set is derived from the status (§9).
**How to avoid:** one commit for the flip, the 4 registry rows, the 10 toy lines and the 2 toy
requirement records.

### Pitfall 7: the round-trip id mistaken for a fault detector
**What goes wrong:** `IVS_FAULT_RECORD_DROPS_LAST_EVENT` is given
`runner.record.format-roundtrip` as its must-fail and `fault-not-detected` fires.
**Why:** a consistent drop round-trips to identical bytes.
**How to avoid:** must-fail is `runner.record.replay-equals-straight`.

## Code Examples

### The replay substitution in `ex_run_steps`

```c
/* shape, not a real case: exec.c:966-988 with the replay path added */
static void ex_run_steps(ex_state *x, const ivs_info *info)
{
  const ivs_scenario *s = x->scn;
  ivs_input_event events[IVS_SCN_MAX_INPUTS];
  uint32_t n_events = s->n_inputs;
  uint32_t consumed = 0u;
  ...
  memset(events, 0, sizeof events);
  if (x->log_n != 0u) {
    /* the log's schedule replaces the scenario's (words 2 and 3 stay 0) */
    n_events = x->log_n;
    for (uint32_t i = 0u; i < n_events; i++) {
      events[i].at_tick = x->log_events[i].at_tick;
      events[i].controller[0] = x->log_events[i].controller[0];
      events[i].controller[1] = x->log_events[i].controller[1];
#ifdef IVS_FAULT_REPLAY_APPLIES_LATE
      /* Fault (plan 11-xx): every replayed event is one tick late. */
      events[i].at_tick += 1u;
#endif
    }
  } else {
    for (uint32_t i = 0u; i < s->n_inputs && i < IVS_SCN_MAX_INPUTS; i++) {
      /* unchanged: exec.c:980-988 */
    }
  }
  x->rec->input_events_effective = n_events; /* NEW: report.c prints this */
  ...
}
```

### The rights check, with its stand-in

```c
/* shape, not a real case */
static int rx_hex32_runs(const uint8_t *b, size_t n)
{
  int hits = 0;
  for (size_t i = 0u; i + 32u <= n; i++) {
    size_t j = 0u;
    while (j < 32u && ((b[i + j] >= '0' && b[i + j] <= '9') || (b[i + j] >= 'A' && b[i + j] <= 'F'))) j++;
    if (j == 32u) hits++;
  }
  return hits;
}
/* the produced log: zero. The stand-in: one appended digest line must be found. */
IVS_CHECK(rx_hex32_runs(log, log_len) == 0);
memcpy(mutated, log, log_len);
memcpy(mutated + log_len, "digest.input 832906ce775d32a98ae607386ce4bbcdd4d08017e3b914b6089e50a5364c7761\n", 78u);
IVS_CHECK(rx_hex32_runs(mutated, log_len + 78u) >= 1);
```

### The judge, and the two self-controls

```sh
# shape, not a real case: the process-case judge
# check_record <reportA> <reportB> <logA> <logB>
#   record-no-log      : a log is missing or empty
#   record-no-events   : the log has no event line            <- the guard
#   record-not-consumed: input.consumed != input.events
#   record-tick-list   : the at_tick lists differ
#   record-differs     : the reports differ                   <- the equality
# replay-differs feeds one changed digest nibble  -> record-differs
# replay-inert   feeds a zero-event pair          -> record-no-events (NOT record-differs)
```

## State of the Art

| Old approach | Current approach | When changed | Impact |
|---|---|---|---|
| a hook whose body could be a no-op and still pass | machine time read after the reset and the restore | plan 08-16, commit `336b3cb` | the cut body is now provable |
| `host.calls` excluded from the comparison | excluded only because a cut adds a slice | plan 08-16 | this phase does not need the exclusion |
| process-level ids as fault detectors | in-process ids only | plan 04-03 (`ivs_fault_suite`) | forces the memory-side design |
| a hand-written accepted-requirement set | derived from the Traceability status | plan 09-05 (EV-04, A-01) | the TOOL-01 flip and its registry rows are one commit |

**Deprecated/outdated:** nothing in this phase's path. `[OBS]` `data/expected/important-gates.ids`'s
`nreq` split string named by GUARDRAILS R2 no longer exists (the R2 note, GUARDRAILS-v0.2.md
after §1); the plan must retarget the header comment and the token prose only.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|---|---|---|
| A1 | The 12 in-process ids cost ≈ 0.02 local cpu-s and the whole phase ≈ 1.9 | §9 | The cost rule fires late and the phase needs a trim commit. Mitigation: the plan measures three alternated runs before landing (GUARDRAILS 2.5). |
| A2 | `demo-lamp-tone-pass` replayed into a copy with its `input` lines removed still passes its 55 asserts | §1, A-11-03 | If any assert depends on the *absence* of an event it would still pass (the replay supplies the same events); if an assert depends on slice boundaries it could differ. Low risk, but the process case must check it on the first run. |
| A3 | The recorded and replayed reports are equal **including** `host.calls` | §1 | If the replay changes the slice count (e.g. because `event_count` differs mid-run) the comparison needs the 08-16 exclusion. The guard would still bite; only the comparison weakens. |
| A4 | Nine refusal tokens (A-11-02 option b) is the right count | §5 | Two extra ids (+4 `tests-expected`). Reversible in the plan. |
| A5 | A 4,096-byte file cap and a 64-byte line cap are the right bounds | §8 | If a future directive needs a longer line, the format needs v2 anyway (the format is a published contract). |
| A6 | The `content project|user` line is informational and not checked on replay | §"Open Questions" O-3 | If the owner wants it checked, one more refusal token. |

## Open Questions

1. **O-1 — Should the replay refuse a log whose recorded scenario id differs from the scenario's?**
   What we know: the log carries `scenario <id>` and the report prints `scenario.id`; a mismatch
   means the operator replayed the wrong log.
   What's unclear: whether that is a refusal or a recorded note.
   Recommendation: **refuse**, token `scenario-mismatch` (a tenth token, or fold into
   `unknown-directive` — do not). It is the cheapest operator-error guard in the format and it is
   what makes the process case's "copy with the input lines removed, id kept" pattern meaningful.

2. **O-2 — Is the profile id checked the same way?**
   Recommendation: **yes, same token family** (`profile-mismatch`). The events are profile-scoped
   (two controllers); replaying a 2-controller log into a 4-controller profile is exactly the v2
   case.

3. **O-3 — Does `--record-input` write a log for a run that applied zero events?**
   Recommendation: **yes** — a header-only log is a valid log, and the *test* is what requires
   ≥ 1 event. Refusing would make `demo-lamp-tone-idle` unrecordable for no reason.

4. **O-4 — Is the log written on a `guest-fail` / `timeout` / `cancelled` run?**
   Recommendation: **yes** (the events were applied and are worth replaying), and **removed** on
   `harness-fail`, matching the operator-output rule (`main_discard`, main.c:325-336). The plan
   must state it because it is not in the criteria.

5. **O-5 — Does the process case need a second `--content-dir` scenario?**
   What we know: the generated no-input copy lives in `$tmp`, so the default content directory
   (the scenario's own directory) is wrong and `--content-dir` must be passed — which is one of
   the three things the criterion asks the process case to cover.
   Recommendation: no extra scenario; the generated copy *is* the `--content-dir` test.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|---|---|---|---|---|
| C17 compiler | the new TU | ✓ | `AppleClang-21.0.0.21000101` (`cc=AppleClang` in the gate token) | — |
| CMake | the new TU and the presets | ✓ | ≥ 3.25 required (integrity.cmake:16) | — |
| `sh` and `dash` | the gate | ✓ | both green on this tree (`gate: PASS` under `sh`; the container gate is the third leg) | — |
| docker | `tools/gate-linux.sh` | `[UNK]` for this session — not probed | — | A visible `not run` line, never a pass (lesson 24) |
| `mktemp`, `sed`, `cmp`, `awk` | the process case | ✓ | POSIX userland, already used by `runner-process-tap.sh:127` | — |

**Missing dependencies with no fallback:** none.
**Missing dependencies with fallback:** the container gate, if docker is unavailable — the plan's
verify block must then write the `not run` line (GUARDRAILS §3, lesson 24).

## Validation Architecture

`workflow.nyquist_validation` is `true` (`.planning/config.json`), so this section is required.

### Test Framework

| Property | Value |
|---|---|
| Framework | the project's own harness (`tests/harness/ivs_test.c`) registered with ctest, plus the shell suites; no external framework |
| Config file | `tests/CMakeLists.txt` (suite registration), `CMakePresets.json` (dev/release), `tools/gate.sh` (the step driver) |
| Quick run command | `ctest --preset dev -R '^runner$'` (the in-process suite, 247 ids, 0.92 s measured) |
| Full suite command | `sh tools/gate.sh` (89 s measured on this tree) |

### Phase Requirements → Test Map

| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|---|---|---|---|---|
| TOOL-01 (record) | a run writes an input-only log whose event lines are the applied prefix | unit | `ctest --preset dev -R '^runner$'` → `runner.record.format-roundtrip` | ❌ Wave 0 |
| TOOL-01 (replay) | the replayed report is byte-identical to the recorded one, from a log-only schedule | unit | same → `runner.record.replay-equals-straight` | ❌ Wave 0 |
| TOOL-01 (guards) | a zero-event pair is rejected by the guard, a one-nibble change by the equality | unit | same → `runner.judge.record-replay-differs`, `.record-replay-inert` | ❌ Wave 0 |
| TOOL-01 (refusals) | 7 (+2) malformed logs refused with a typed token and an untouched output | unit | same → `runner.record.refuse.*` | ❌ Wave 0 |
| TOOL-01 (rights) | zero 32-hex runs in a produced log, and ≥ 1 in the stand-in | unit | same → `runner.record.no-content-digest` | ❌ Wave 0 |
| TOOL-01 (process) | the command line, `--content-dir`, exit codes, the exclusive create, and the Lamp Tone replay against the committed golden | integration | `ctest --preset release -R '^runner-process$'` → `runner-process.record.command-line` | ❌ Wave 0 |
| TOOL-01 (faults) | each fault fails exactly its must-fail id | fault | `sh tools/check/steps/c-faults.sh` + `faults-verdict.sh` via the gate | ❌ Wave 0 |

### Sampling Rate

- **Per task commit:** `ctest --preset dev -R '^runner$'` (0.92 s) plus
  `sh tools/check/steps/runner-scan.sh` (the new files' header/feature-macro rules).
- **Per wave merge:** `sh tools/gate.sh` (89 s baseline; the plan predicts the exact token).
- **Phase gate:** the full token equal to the plan's prediction, then
  `sh tools/gate-linux.sh` → `gate: PASS`.

### Wave 0 Gaps

- [ ] `runner/ivsl.h` + `runner/ivsl.c` — the format constants, the writer, the parser, the tokens.
- [ ] `docs/formats/ivsl-v1.md` — **committed first**, ancestry proved with
      `git merge-base --is-ancestor` (GUARDRAILS 2.9). No test file can precede it.
- [ ] `data/expected/tests/runner.ids` +12 ids; `data/expected/tests/runner-process.ids` +1 id (R9).
      `data/state/tests-pending.ids` has **5 lines and no runner id** (measured), so no pending
      line is due — R9's delete limb is a no-op here.
- [ ] `data/expected/faults/runner.ids` +2 faults (R10), same commit as their sites.
- [ ] `data/expected/important-gates.ids` +4 rows, the ten toy registries +1 line each, the two
      toy requirement records +1 row each, and the `TOOL-01` flip — **one commit** (§9).
- [ ] No framework install: the harness and ctest are already there.

## Security Domain

`security_enforcement` is `true`, `security_asvs_level` is `1`.

### Applicable ASVS Categories

| ASVS Category | Applies | Standard Control |
|---|---|---|
| V2 Authentication | no | no principal exists |
| V3 Session Management | no | no session |
| V4 Access Control | **yes, narrowly** | the log is written only to the operator's explicit path, created `O_EXCL`; the log never names a path, so a log cannot redirect a read or a write |
| V5 Input Validation | **yes** | the parser is the control: a closed directive set, a bounded file (4,096 B), a bounded line (64 B), a bounded event count (64), a decimal tick bound (2^62), a hex charset and the reserved bit; every refusal is a typed token with the output untouched |
| V6 Cryptography | no | the phase computes no hash of its own; `digest.input` is the core's |

### Known Threat Patterns for this stack

| Pattern | STRIDE | Standard Mitigation |
|---|---|---|
| a log that is a symlink or a FIFO used to write outside the operator's intent | Tampering | `O_CREAT|O_EXCL` refuses an existing path **of any kind, a dangling symlink included** (hostio.h:53-56) |
| a log that is a device or a directory used to hang the read | Denial of service | `ivs_hostio_read`'s `O_NONBLOCK` + `fstat` `S_ISREG` (hostio.c:100-113) |
| a crafted log that overflows the event array | Tampering | the 64-event cap is checked before any event is written into the array |
| a log that makes the runner read outside the content dir | Information disclosure | the log has no path field at all (§3) |
| a log that smuggles a digest and turns the log into derived material under D11 | Repudiation / rights | the grammar cannot express 32 hex digits (§7) plus `runner.record.no-content-digest` with its stand-in |
| a fault site left in the tree | Tampering | `faults-verdict`'s two-directional site check (`fault-orphan-site`, `fault-missing-site`) |

## Sources

### Primary (HIGH confidence)

Every claim is from this tree; the load-bearing ones:

- `runner/exec.c:103-106, 149-197, 204-276, 966-988, 1060-1180, 1289-1316, 1440`
- `runner/ivs_runner.h:107-118, 181-236, 283-287, 435-511`
- `runner/report.c:15-30, 120-129, 340-344`
- `runner/main.c:55-129, 255-336, 545-663, 689-820`
- `runner/hostio.c:1-30, 74-134, 145-165, 213-229`; `runner/hostio.h:40-80`
- `runner/scenario.c:143-158, 266-320, 883-944, 1205-1290`
- `core/include/ivs/ivs_run.h:143-160, 200-230, 277-290, 340-355`
- `core/include/ivs/ivs_machine.h:61-69, 183-215`
- `tests/faults.cmake:80-180`; `tests/CMakeLists.txt:490-540, 669-680`
- `tools/check/steps/faults-verdict.sh:1-40, 88-140, 220-240`
- `tools/check/steps/important-gates.sh:20-66, 80-200`
- `tools/check/steps/c-faults.sh:1-30, 130-190`
- `tools/check/steps/runner-scan.sh:20-26`
- `tools/check/integrity.cmake:30-90`; `tools/check/steps/text.sh:1-50`;
  `tools/check/steps/glossary.sh:40-62`
- `tools/check/binary-ext.txt`; `.gitignore`
- `data/golden/demo-lamp-tone-pass.digests`; `data/scenarios/demo-lamp-tone-pass.ivss`
- `data/expected/{input-bits,input-policies,profile-fields,pkg-install-files,important-gates,important-gates-excluded}.ids`
- `data/expected/faults/{runner,run}.ids`
- `data/expected/tests/{runner,runner-process}.ids`
- `.planning/milestones/v0.1.0-phases/08-…/08-16-SUMMARY.md`;
  `.planning/milestones/v0.1.0-phases/08-…/08-ACCEPTED-DEBT.md` §6
- `.planning/ops/GUARDRAILS-v0.2.md`; `.planning/ops/PLAN-LESSONS.md` lessons 4, 20, 21
- `.planning/STATE.md` (Recorded Constraints, Deferred Items)
- `.planning/MILESTONE-CONTEXT.md` §2 Phase 11, §3 D15

### Secondary (MEDIUM confidence)

- none: no web source was consulted.

### Tertiary (LOW confidence)

- none.

## Metadata

**Confidence breakdown:**

- Standard stack: **HIGH** — there is no stack decision; the phase adds no dependency and reuses
  the in-tree pieces, each read at its line.
- Architecture: **HIGH for the constraints** (faults.cmake, `IVS_RUNNER_SOURCES`, `host.calls`,
  report.c:341 — all read), **MEDIUM for the shape** (the memory-side design is my inference from
  those constraints, not a recorded decision).
- Pitfalls: **HIGH** — every pitfall is a measured property of the tree or a recorded defect from
  a previous phase.
- Counts: **HIGH** for the baseline (a gate run on this tree) and for the arithmetic;
  **MEDIUM** for `tracked`/`files`, which depend on how many record files the phase writes.

**Research date:** 2026-11-25
**Valid until:** the next commit that touches `runner/`, `tests/faults.cmake`,
`tools/check/steps/important-gates.sh` or `data/expected/tests/` — i.e. treat as valid for this
phase's planning only, and re-measure the token before landing (GUARDRAILS §3: an unexpected
token is a stop, not a re-bless).

## RESEARCH COMPLETE
