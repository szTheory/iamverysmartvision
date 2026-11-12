# Phase 5: Hand Controllers and PSG Sound - Research

**Researched:** 2026-11-12
**Domain:** growing a C17 deterministic core (run API, digests, memory map, profile generators) with guest-visible controller input, a PSG device, raw cycle-stamped audio and a write log; extending the runner, goldens, installed consumer and gate to carry them
**Confidence:** MEDIUM-HIGH (code facts, derived constants and gate mechanics read or executed this session: HIGH; hosted timing, gcc-only behaviour of code that does not exist yet, and the 8910 page contents: MEDIUM or LOW, tagged)

Scope note: the design is locked by D-01..D-28 and the Reconciliation in `05-CONTEXT.md`; the four `05-DISCUSS-*.md` files are the audit trail. No hardware or chip question was re-researched and no web search was made. This file grounds the decisions in the CURRENT code: which file and mechanism changes for each decision (file:line), whether the CONTEXT assumptions about the code are true, a decomposition check, ranked pitfalls and the validation map. Probes run this session on this host (Apple clang 21, CMake 4.4.3, Ninja 1.13.2, dash, Docker 29 with `ubuntu:24.04` cached): an exhaustive brute-force of `ivs_cp1610_step` over all 1024 decles with SDBD off and on and wait 0, 1 and 35, and file reads of every source cited below. No gate run was made (no source file was edited).

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
D-01 through D-28 and the Reconciliation section of `.planning/phases/05-hand-controllers-and-psg-sound/05-CONTEXT.md` `<decisions>` are locked and incorporated by reference (a verbatim copy would drift; the planner reads that file first; Phase 4 research did the same). One-line index:
- Controller input (INP-01..03, API-03): D-01 Phase 4 logical model stands, `run.input.guest-invisible` is inverted; D-02 31 `input.control.<name>` profile fields (`{"lines": "$XX"}`, tier4, C16) plus `input.line_model`, `input.line_idle`, `input.port_output_read`, generator `cmake/IvsCtrlTable.cmake`, independence literal `data/input/intv-2609-ntsc-family-p1.literal.txt`; D-03 `pins = 0xFF XOR OR(line masks)`, `data/expected/input-policies.ids`; D-04 `$01FE` = R14 = port 0, `$01FF` = R15 = port 1, device kind `psg8` fill (C64), `port_pulldown[2]` in `ivs_memsys`; D-05 API-03 proof (fuzz B and C, C67 granularity test, runner slices 1 and 7, installed replay); D-06 evidence tiers.
- PSG rules (AUD-01): D-07 `data/psg/rules.json`; D-08 register file at `$01F0..$01FF` per `psg.regmap`; D-09 one PSG step = 8 chip clocks = 16 master ticks = 4 CPU cycles (corrects `docs/time-base.md`); D-10 tone/noise/envelope/mixer/amplitude/reset; D-11 `psg.dac.levels` never read by the core; D-12 discrepancy ids C69..C76, next C77, ONE allocation table in plan 05-01.
- Raw audio and log (AUD-02): D-13 16-byte write-log record; D-14 third buffer `psg_log`, reasons `RUN_PSGLOG_*`; D-15 4-byte sample `level_A, level_B, level_C, 0`, `k * 16` stamps; D-16 lazy slicing-invariant generation with a per-call sink; D-17 audio capacity with slack `s`, NULL plus capacity 0 means "not requested"; D-18 digests (domain 17 live, domain 19 new, composite v2); D-19 `ivs_run_request` v2, `ivs_run_report` v2, `ivs_info` v3, no new function; D-20 runner `ivsr 2`, capture and assert forms, operator-only options; D-21 (D7) reference converter outside the core; D-22 operator WAV.
- Verification: D-23 oracle-first plan 05-01 without C; D-24 fixture `ivs-diag-ctrl-psg-1`; D-25 suites `input` and `psg`; D-26 claims CL-0009..CL-0015; D-27 plan shape (re-cut allowed); D-28 hard stops (D10, D11, datasheets stay in the gitignored cache).

### Claude's Discretion
CONTEXT has no separate section; D-27 states: "The planner may re-cut if it finds file-set collisions or a plan over the caps, but must not exceed Phase 4's discipline." Everything else the planner may choose: exact struct layouts and field names (appended per the A-02 rule), final fault and control id lists, whether the reference converter lives in `tools/pcm` or `runner/` (see Open Question 3), task splits.

### Deferred Ideas (OUT OF SCOPE, verbatim from CONTEXT)
- Reads in the write log (domain-19 v2); a tick-tight audio capacity formula; an installed `ivs::refpcm` target; an analytic fast path for silent channels; back-pressure on the log buffer; ECS keyboard scan and output-mode contention modelling; the 8914 differences beyond the documented unknowns; a project mini-assembler (trigger: a third diagnostic or labels/macros, expected Phase 7).
- Phase 4 review deferrals IN-04, IN-06, IN-09, IN-10, IN-11 stay open.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| INP-01 | 12 keys, 16 disc directions, side actions each produce the documented read through the PSG ports, per player | Generator and literal (Plan 05-05), read path through `ivs_mm_region_read` at memsys.c:155-179, apply wiring at machine_input.c:123 and machine_bound.c:444, suite `input` (`input.single.*`, two read paths), step `input` |
| INP-02 | Simultaneous inputs give a documented, tested result | Union rule `0xFF XOR OR(masks)`, `input-policies.ids`, `input.combo.*` and pairwise loops in suite `input`, policy rows excluded from evidence |
| INP-03 | Core input is a machine-level representation, no host mapping | Phase 4 input model unchanged (ivs_run.h:119-146), `c-purity` and `core-symbols` already ban host names; profile fields carry no host naming |
| API-03 | Input applied at an explicit machine-time boundary; identical results regardless of host timing | Unit-start apply (machine_bound.c:444), fuzz B and C, C67 test, `input.fuzz.seed*`, runner slices 1 and 7, installed replay |
| AUD-01 | PSG register file, tone, noise, envelope follow documented map with per-rule vectors; unknowns are profile data | `psg.regmap` and rule fields via a new generator, rule inventory, `data/psg/vectors/*.txt`, steps `psg-vectors`, suite `psg`, perturbation tests |
| AUD-02 | Cycle-stamped write log and integer chip-rate samples, separate from host PCM (D7) | Bus wrapper with unit start and offsets (Pattern 1), per-call sink, domains 17 and 19, third buffer, `ivs_run_*` v2, reference converter outside the core |
</phase_requirements>

## Project Constraints (from CLAUDE.md / AGENTS.md / AUTONOMY.md)

- Prefix `ivs_`; never "intellivision" in a package, product or repo name (hard stop). C17 + CMake (D1). No copying from MAME, jzIntv, libasm or any assembler; agreement with another emulator is not evidence. "Another copy-paste is better than another dependency" (AUTONOMY section 2): no external package is installed by this phase.
- Rights: datasheet PDFs, wiki images and fetched pages stay in the gitignored `refs/` cache; `data/sources.json` entries carry `rights: unknown`, `capture: hashed` (needs `sha256`, `size_bytes`, `local_path`); the literal files hold derived hexadecimal facts and short paraphrases only. No committed `.ivsc`, `.bin`, `.rom`, `.wav` (`tools/check/binary-ext.txt` lists `int ivsc bin rom itv cfg cc3 pdf png jpg jpeg gif zip 7z gz tar bz2 xz dat bmp webp svg ico a26`; `.wav` is not on it, so a stray WAV would be caught only by review: the operator WAV is written outside the repo).
- Shift-left: zero human UAT. D10 (hardware) and D11 (owner EXEC/GROM) are not performed; evidence that needs them is recorded missing and handed off (D-26, D-28). Executors never call `gh`; the orchestrator runs `tools/ci-record.sh`.
- `sh tools/gate.sh` is the single local and CI command; it must stay green under `sh`, `dash tools/gate.sh` and `sh tools/gate-linux.sh` at every plan end. Measure before changing CI shape.
- Docs rule: README and `docs/**/*.md` are scanned (tools/check/steps/text.sh and glossary.sh read only `README.md` and `docs/`): claim words (supports, compatible, passes, accurate, verified, runs) need `[claim:CL-NNNN]` or `[nc]` on the same line; the bare word clock/clocks fails unless the line says input clock, STIC clock, PSG chip clock or `[nc]`. `.planning/` is not scanned by either.
- Profile-field names: a last segment containing a whole token window/offset/hold/duration/delay/period/latency/interval/timeout must end `_ticks` or `_cpu_cycles` (tools/check/steps/glossary.sh:76-90); `psg.sample_period_ticks` and `psg.write_strobe_ticks` comply, `psg.period_zero.tone` complies because the LAST segment is `tone`.
- Hygiene (tools/hygiene.sh `HOME_ALL`): no home path, scratch or temp-directory path, tilde-relative path or secret shape in any tracked file; use `<repo-root>` placeholders and `mktemp -d` in scripts.

## Summary

Phase 5 is the first phase that changes what the guest can observe, so every Phase 4 mechanism it touches is touched at its seams: the bus port context (the memsys has no clock), the instance layout and digest table, the run ladder (a third buffer and grown structs), the profile registry generator (a PSG device and per-register masks), the memmap literal and its behavioural probe, the runner report and golden key lists, and the goldens themselves. The design is sound against the code; the plan risk is the number of places one change must be registered at once and the number of times the same goldens must be re-recorded.

Five findings change what the planner writes. (a) The CONTEXT says access timing "offsets" reach the PSG: they exist in the CPU access record (`offset_cpu_cycles`, cp1610.h:48-55) but the bus context is the bare memory system (`port.ctx = &im->mem`, machine_run.c:128), so neither the unit start tick nor `ticks_per_cpu_cycle` is available to a write; a machine-level bus wrapper with a per-call context is required (Pattern 1). (b) The seam `ivs_machine_step_unit_internal` is used by seam-stepped twin instances that tests compare byte for byte with run-call instances (tests/run/run_slice.c header: "whole instance storage equal a second seam-stepped twin"); with a PSG that is only true if the seam itself leaves the PSG caught up at every unit end, so catch-up at the end of every unit (not only of every call) is the safe form. (c) Flipping `$01F0-$01FF` from `impl=stub` to `impl=real` is not data-only: `cmake/IvsMemmapTable.cmake` rejects a real device on a stub physical device (lines 449 and 453), the `memmap` suite has a stub rule for `psg` (tests/memmap/memmap_rows.c:280) and a descriptor table row (tests/memmap/memmap_main.c:83), and the behavioural width probe will see per-register widths; the flip belongs in the plan that makes the device real, not in plan 05-01 (V-05 in the verify note puts it in 05-01; that would break the gate). (d) The derived constants differ from the DISCUSS illustrations: the minimum gap between two PSG writes is g = 9 CPU cycles = 36 ticks (executed over all decles), and the maximum unit is U = 12 CPU cycles = 48 ticks on the real machine bus (wait 0) or 117 cycles = 468 ticks if every wait-taking access waits the profile limit of 35; D-17's "maximal waits" must pick one (Open Question 1; recommendation: publish 468, derived from profile fields, so Phase 6 stalls cannot invalidate it). (e) D-19 rebases only the request minimum size; the report (v1 112 bytes, machine_bound.c:74 and the whole-struct copy at :499), the env seam and `ivs_info` need the same treatment.

Gate cost: baseline `steps=33/33 controls=277/277 faults=149/149 tests-expected=2816` (tests-expected counts ids x 2 presets, tests-inventory.sh:19, so about 365 new ids give about 3,546). The V-16 projection (+17..28 cpu-s) against a last-5 median of 175 against the 200 trigger stands; the binding risk is not the engine speed (about 223,000 steps per emulated second) but re-recording goldens in three behaviour plans and per-fault reruns of two new suites.

**Primary recommendation:** 12 plans in 11 waves (CONTEXT's ten with 05-02 and 05-08 each split, composite v2 moved to the plan that first adds PSG state rows); oracle plan first and C-free; every plan ends gate-green under sh, dash and (marked plans) `tools/gate-linux.sh`; PSG device code takes an explicit parameter struct so perturbation tests need no registry seam; the machine bus wrapper and an eager end-of-unit catch-up are the two architectural choices that make the lazy-sink design testable.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Control-to-byte table, line model, port pull-down state | Profile data (`data/profiles`) plus generator `cmake/IvsCtrlTable.cmake` -> build-tree table read by `core/src/machine/` | `core/src/memmap/` (pure read of the stored pull-down) | Table is tier-4 data (D-02); the memsys read must stay pure so `ivs_machine_peek` equals a CPU read (D-04) |
| PSG register file and port reads | `ivs_memsys` (registers, `port_pulldown`, per-register masks from profile) | PSG device module (engine) | Reads must stay pure and const (`ivs_memsys_read(const ivs_memsys *, ...)`, memsys.h:147) |
| PSG engine (tone, noise, envelope, mixer, samples, log, chain) | New `core/src/psg/` TUs owned by the machine, parameters passed explicitly | Machine bus wrapper (stamps) | Needs unit start tick and offsets that only the machine layer has (Pattern 1) |
| Input application | `core/src/machine/machine_input.c` + run loop | `ivs_memsys` setter for `port_pulldown` | Unit-start apply stays in the run loop (machine_bound.c:444) |
| Output delivery (audio, log) | Caller-owned buffers through `ivs_machine_run` | Runner (allocates, accumulates, re-hashes) | D-14, D-16: per-call stack sink, core keeps no pointer |
| Reference PCM, WAV | `tools/pcm` or `runner/` static code, operator-side | none | D7: nothing in `ivs_core` or an installed artifact |
| Report, IVSS grammar, goldens | `runner/` library over the public API only | `tools/check/steps/golden.sh` (second copy of the key list) | Runner is the first consumer; key lists exist twice (main.c:243-262 and golden.sh header) |
| Rule/vector/literal consistency | Pure gate steps `input`, `psg-vectors` over text files | in-process suites | Controls can drive pure steps on a toy tree (run-controls.sh) |
| Installed-package proof | Gate step `pkg-install` + `tests/installed-consumer/` | runner golden agreement | A-02 growth can only be wrong in the installed header |

## Standard Stack

No external package is installed by this phase; the Package Legitimacy Audit is not applicable.

| Tool | Local (this host) | Hosted ubuntu-24.04 | Note |
|------|-------------------|---------------------|------|
| CMake | 4.4.3 [VERIFIED: `cmake --version`] | 3.28.3 [CITED: .planning/CI-EFFICIENCY.md Phase 3 section] | Floor 3.25 (CMakePresets.json `"minor": 25`); new generators use `file(STRINGS)`/`string(JSON)` only inside `cmake -P` generators, not in new gate steps |
| Ninja | 1.13.2 [VERIFIED: `ninja --version`] | 1.13.2 [CITED: 04-RESEARCH.md] | |
| C compiler | Apple clang 21 (`cc` and `gcc` are both Apple clang here) [VERIFIED: `gcc --version`] | gcc 13.3 [CITED: CI-EFFICIENCY.md] | No native gcc locally: gcc behaviour only through the container |
| sh / dash | macOS sh (bash 3.2 mode) and `/bin/dash` [VERIFIED: probes] | dash [ASSUMED same as Phase 4] | New shell must be dash-safe, no `[[`, no `pushd` |
| docker | daemon up, `ubuntu:24.04` image present [VERIFIED: `docker image ls`, `docker ps`] | n/a | `sh tools/gate-linux.sh`, about 3 to 5 minutes under amd64 emulation [CITED: CI-EFFICIENCY.md] |
| PDF tools | `pdftotext`, `pdftoppm`, `tesseract`, `qpdf` present; `mutool` absent [VERIFIED: `which`] | n/a | For plan 05-01 reading of the GI pages |
| Network | bitsavers answers 200 to a request with a browser user agent [VERIFIED: `curl -sI -A ...` on the 1980 catalog URL] | n/a | `refs/` holds ONLY `SRC-gi-cp1600-um-1975.pdf` and `pages/` today [VERIFIED: `ls refs`]; the 1980 and 1982 catalogs registered in `data/sources.json` (sha256 `70094d12...` and `b609963d...`) are NOT cached, so 05-01 must fetch and hash-verify them (about 42 MB and 37 MB) |

**Version verification:** compiler, CMake and tool rows are from commands run this session or recorded hosted runs; no registry package is involved.

## Architecture Patterns

### System flow (conceptual)

```
IVSS scenario (inert text: input lines, run-to, assert, capture) --> runner exec (slices of <= 1<<22 ticks)
   |  per slice: ivs_machine_run(target, events suffix, video NULL/0, audio buf, psg_log buf)
   v
ladder (stages 1..4b/4c(+psg_log)..9, refuse before any state change)
   v
loop while time < target:                                   [machine_bound.c:423-476]
   exit tests -> apply due input events (words, count, chain, port_pulldown)   [machine_input.c:123]
   -> step one unit through the machine bus wrapper (NEW): ctx = {machine, unit_start, ticks_per_cycle, sink}
        CPU access (offset_cpu_cycles) -> wrapper:
            READ/FETCH/ADAR -> ivs_memsys_read (pure: registers, pull-down, direction, masks)
            WRITE/DW to $01F0-$01FF -> psg_catch_up(stamp) ; store register ; psg_log(stamp,...)
            other -> ivs_memsys_bus
   -> unit end: psg_catch_up(now) into the sink (eager, so a seam twin equals a run-call instance)
   v
report: stop reason, now, overshoot, audio_first_index, bytes written, psg_log_bytes_written, ...
   v
runner: re-hashes the audio change chain from received bytes (audio.chain.check), prints ivsr 2 report
   v
gate: runner-process (both presets) -> golden step (key list x2) ; installed consumer replay ; steps input and psg-vectors
```

### Mechanisms Phase 5 must extend (read this session; the planner edits exactly these)

| # | Mechanism | Where (file:line) | Change required | Plan |
|---|-----------|-------------------|-----------------|------|
| 1 | Bus port context is the memsys only: `port.bus = ivs_memsys_bus;` ... `port.ctx = &im->mem;` and the step is called with no access log: `ivs_cp1610_step(&im->cpu, &cfg, &lines, &port, NULL, 0u, &r)` | machine_run.c:126-131 | New wrapper `ivs_machine_bus` with a per-step context struct (machine, `start`, `ticks_per_cpu_cycle`, sink); stamp = `start + offset_cpu_cycles * ticks_per_cpu_cycle + psg.write_strobe_ticks`; keep `ivs_machine_step_unit_internal(m, out)` and add a sink variant; unit-end catch-up inside the seam | 05-03/05-04, 05-06, 05-08 |
| 2 | Access record already carries the offset: `typedef struct ivs_cp1610_access { uint32_t offset_cpu_cycles; uint16_t addr; uint16_t data; uint16_t wait_cpu_cycles; uint8_t kind; uint8_t flags; }`; the bus sees it at the moment of the access: `a.offset_cpu_cycles = nominal + u->shift;` then `reply = u->port->bus(u->port->ctx, &a);` | cp1610.h:48-55, cp1610.c:116 and :122 | None (consumer only). Writes are WRITE at offset 4 (MVO@ and PSHR, cp1610.c:529 and :972, `IVS_DATA_BAR_OFFSET`), 6 (direct MVO, :1005 `IVS_DIRECT_ADAR_OFFSET`) and DW at offset 2 (interrupt entry, :1226); at most one write per unit [VERIFIED: probe, `max_writes_per_unit=1`] | n/a |
| 3 | Memsys bus returns wait 0 for every kind: `reply.wait_cpu_cycles = 0u;` | memsys.c:285 | A PSG wait would change U and g (Open Question 1); keep 0 and assert it in a test | 05-02/05-04 |
| 4 | Read path: region index -> `ivs_mm_load(m, r->phys, idx)` switch with `default: return 0u; /* stub devices hold no state */`; one width formula `read = (stored & rmask) | (fill & ~rmask)`; PSG phys falls into the default today | memsys.c:100-116 and :155-179 | Add `IVS_MEMSYS_PHYS_PSG` case: registers from `m->psg_reg[idx]`, port registers 14 and 15 from direction (R7 bits) plus `port_pulldown` or latch; per-register `rmask` from the profile table; upper byte from the `psg8` record fill | 05-04, 05-05 |
| 5 | Write path stores `value & d->rmask` when `d->keep` and the gate is open; PSG is `store: drop` stub today | memsys.c:222-258 | `psg8` with `keep` and per-register `wmask`; the machine wrapper (not memsys) calls the PSG engine first, so a write to the PSG region is never applied by `ivs_memsys_bus` directly | 05-04 |
| 6 | Device kinds and generator: `#define IVS_MEMSYS_DEVICE_KINDS 6u`; `set(MM_DEVICES undriven rom10 grom8 ram8 ram16 stub)`; `set(MM_STUB_PHYS stic psg)`; fail text `"a backed device needs impl real and a backed physical device"` (an `impl real` psg region is refused), `MM_CAP_<phys>` for backing asserts | memsys.h:47; IvsMemmapTable.cmake:150, :159, :161-165, :449-462 | Add kind `psg8` (new id 6, KINDS 7), make `psg` a backed physical device with `IVS_MEMSYS_PSG_WORDS 16`, new `MM_CAP_psg`, `MM_STUB_PHYS` = `stic` only; update `ids_c` asserts | 05-04 |
| 7 | Profile descriptor lacks the fields Phase 5 needs: it has `ticks_per_cpu_cycle`, `scanlines_per_frame`, `cpu_cycles_per_scanline`, `input_controllers` and nothing for `ticks_per_psg_clock`, `crystal_hz` or any `psg.*` or `input.control.*` | memsys.h:107-126; IvsMemmapTable.cmake:544-547 (registry row) | Extend `ivs_profile_desc` (psg sub-struct: clocks per step, sample period, period-zero fields, LFSR object, envelope and mixer bit fields, DAC (never read by core), `psg_log_records_per_frame`, `max_unit_ticks`, crystal num/den passthrough). Needs a NEW generator for the `psg.*` objects (CONTEXT lists only `IvsCtrlTable.cmake`) | 05-02/05-04, 05-06, 05-07 |
| 8 | Generated includes reviewed by `c-purity`: `ivs_profiles.inc cmake/IvsMemmapTable.cmake` (generator accepts only `-DROOT=<tree> -DOUT=<file>`); `ivs_core_gen` depends on the single `IVS_PROFILES_INC` custom command; fault copies wait for that target | data/expected/core-generated.ids; core/CMakeLists.txt:34-48; tests/faults.cmake (gen dir and target) | Add each new generated include (`ivs_ctrl_lines.inc`, psg params) to core-generated.ids AND to the `DEPENDS` and outputs of `ivs_core_gen`, else fault executables and `c-purity` miss them; `-DMUTATE` stays an extra optional arg registered through `ivs_fault_gen(... CONSUMER core:src/...)` | 05-04, 05-05 |
| 9 | Instance layout: `#define IVS_MACHINE_BYTES 113600u`; `uint32_t obs_pad;` and `ivs_machine_obs obs;` between `mem` and `slot[]`; asserts `offsetof(struct ivs_machine, obs) == offsetof(struct ivs_machine, obs_pad) + 4u`, `sizeof(struct ivs_machine) == IVS_MACHINE_BYTES` | machine.h:31, :81-92 | `ivs_memsys` grows (registers 32 bytes + pull-down) so `mem` no longer ends at 4 mod 8 (113,228 + 36 = 113,264, divisible by 8 [INF: arithmetic]); re-derive pad words, rows, `IVS_MACHINE_BYTES`; PSG device struct and wider `obs.audio` go BEFORE `slot[]` (machine_digest.c:233-235 skips every row at or after `slot` for a named fault) | 05-03 |
| 10 | Digest table: rows in instance order; domains known 1..4 and 16..18; composite `ivs_dsink_uint(&k, IVS_DIGEST_COMPOSITE_VERSION, 2u)` over domains 1..4 | machine_digest.c:53-93, :207-213, :317-330; machine.h:110 `#define IVS_DIGEST_COMPOSITE_VERSION 1u` | PSG rows tagged INSTANCE (4) so domains 1..3 digests stay byte-identical (CONTEXT D-18 expects only composite, instance and audio lines to differ); version to 2; add domain 19 (`IVS_DIGEST_PSGLOG`, ivs_machine.h:60-67 list) to `ivs_digest_domain_known`; audio rows gain `prev_levels` and `emitted` | 05-03, 05-06, 05-08 |
| 11 | Digest tests pin literals: fresh composite `md_fresh_composite`, stepped literals `md_stepped[5][32]` ("Content A loaded, power-on reset, 100 units through the step seam"), hand-framed domain-4 preimage stream (machine_digest.c:95-110), per-domain `in_composite` flags | tests/machine/machine_digest.c:95-110, :251-259, :288-309 | Record v1 values once more from the unmodified tree FIRST (they exist; do not rely on memory), extend the hand-framed INSTANCE stream by the new rows, mark domain 19 in the domain list, retarget `machine.refuse.digest.domain-19` (machine.ids:126, machine_digest.c:832, api-effects.json:317) to a still-invalid domain (20) | 05-03 |
| 12 | Input apply touches only `obs.input`; the refused-step take-back restores only it | machine_input.c:44-65, :123-153; machine_bound.c:426 `ivs_machine_obs_input input_before = im->obs.input;`, :444, :450 `im->obs.input = input_before;` | `ivs_input_apply` gets the instance, derives `port_pulldown[p]` from the words and the generated line table through a memsys setter; the take-back must restore the pull-down too (it lives in `mem`); fault candidate `PULLDOWN_NOT_RESTORED` | 05-05 |
| 13 | Run ladder, request: `req->struct_size < (uint32_t)sizeof(ivs_run_request)` refused; tail scan starts at `sizeof(ivs_run_request)`; capacity check `ivs_bound_capacity_ok(req, env, ...)`; buffers stage `ivs_bound_buffers`; env struct carries only video and audio layout | machine_bound.c:278, :298, :330, :216-231; machine.h:177-183 | Pin `IVS_RUN_REQUEST_V1_SIZE` 64 and a request v2 (adds `psg_log`, `psg_log_capacity`); read appended fields only when `struct_size` covers them; stage 4b gains psg_log NULL/overlap (pairs with instance, report, video, audio, events); `ivs_run_env` gains `psg_log_bytes_per_frame` and the audio/log slack inputs | 05-02 |
| 14 | Run ladder, report: `_Static_assert(sizeof(ivs_run_report) == 112u, ...)`; size check `rep->struct_size < (uint32_t)sizeof(ivs_run_report)`; full copy `*rep = r;` | machine_bound.c:74, :269-271, :499 | Same A-02 treatment (not in D-19): `IVS_RUN_REPORT_V1_SIZE` 112, write `min(struct_size, own size)` bytes, refusal writes stay inside the v1 prefix; add `audio_first_index`, `psg_log_bytes_written`, `psg_log_first_index` | 05-02 |
| 15 | Info growth: `IVS_INFO_V1_SIZE 216u` (machine.h:36), `_Static_assert(sizeof(ivs_info) == 256u, ...)` (machine.c:32), minimum check `out->struct_size < IVS_INFO_V1_SIZE`, write `n = min(caller size, sizeof r)` | machine.c:30-32, :257, :265, :300-301; ivs_machine.h:128-145 | Append at offset 256: `audio_sample_ticks`, `audio_sample_bytes`, `psg_log_record_bytes`, `psg_log_records_per_frame` (u32 each) then `crystal_hz_num` and `crystal_hz_den` (u64, 8-aligned at 272 and 280 [INF], sizeof 288); `ivs_machine_profile_env` (machine.c:304-314) fills real audio and log layout; `IVS_OUTPUT_FORMAT_PSG_LEVELS_V1` next to `IVS_OUTPUT_FORMAT_NONE 0u` | 05-02, 05-06 |
| 16 | Pinned define classes: `IVS_STOP_`, `IVS_REASON_`, `IVS_CTRL_` (bit form, `IVS_CTRL_RESERVED_MASK` excluded) with pin files; last reason is `IVS_REASON_INPUT_EXTRA_CONTROLLER 54` | tools/check/api-effects.cmake:65-72; data/expected/api-reasons.ids:60 | New reasons 55..57 (`RUN_PSGLOG_NULL`, `_OVERLAP`, `_SMALL`) appended to the header AND `api-reasons.ids`; the log-source defines (CPU write 1, interrupt-entry write 2) need a new class `IVS_PSGLOG_SRC_` with `psg-log-source.ids` and one control; any new `IVS_REASON_`/`IVS_STOP_` define without a pin line fails `api-define-unpinned` | 05-02, 05-08 |
| 17 | Effect table: 14 calls, `ivs_machine_run` has 6 rows; every row needs a test id that is not skip-ok; `ivs_machine_state_digest` row says `"out NULL, or a domain other than 0..4 and 16..18 (19 included)"` | data/api/api-effects.json:305, :317 | Rows for the three new reasons and the NULL-plus-0 rule; digest row updated (19 valid, 20 refused) | 05-02, 05-08 |
| 18 | Buffers: exec splits `host->buffers` into halves only when video or audio bytes per frame are nonzero, and `ivs-run` never sets `host.buffers` | runner/exec.c:504-511; runner/main.c:507-519 (`memset(&host...)`, `host.storage` only) | With D-17's NULL plus 0 rule this still runs (channel not requested); real capture needs `main.c` to allocate buffers sized from `ivs_info` (about 3 MB per default slice, derived below) and exec to split three ways | 05-09 |
| 19 | Report and golden key lists exist twice | runner/report.c:60-122 (key tables, `ivsr` header), runner/main.c:243-262 (`main_golden_keep`), tools/check/steps/golden.sh header (key list and `# domains:` grammar), tests/runner/runner_exec.c:257 and :871, tests/runner-process-tap.sh:171 (`ivsr 1`), `_good/tree` toy reports | `ivsr 2` and new keys in all of them in ONE plan; golden `# domains:` line is a fixed grammar (`cpu= mem= images= instance= video= audio= input=`), a `psglog` domain needs the grammar and its controls extended | 05-09 |
| 20 | IVSS grammar and caps | runner/ivs_runner.h:55-91 (grammar, `assert digest <composite|cpu|...|input>`, caps 64 asserts and 64 inputs) | `capture psglog|audio`, `assert psglog ...`, `assert audio ...`, `assert digest audio=|psglog=`; parser table rows, fuzz extension, new reasons appended after `IVS_SCN_R_NO_RUN_TO` (44; `IVS_SCN_R_COUNT 45u`) | 05-09 |
| 21 | Fault framework: owner TU must be in exactly one core or runner TU; stems unique across modules; suites registered once; plain copies per suite | tests/faults.cmake:34-57; tests/CMakeLists.txt:271 (`run`), :335 (`runner`); `ivs_fault_suite` with `MODULES core` or `core runner` only | Register suites `input` and `psg` with their sources; every new core TU is listed in `IVS_CORE_SOURCES` (core/CMakeLists.txt:12-24); a TU outside `core/` and `runner/` (a `tools/pcm` converter) cannot own a fault | 05-04 on |
| 22 | memmap literal and behavioural probe | data/memmap/intv-2609-ntsc-family-p1.literal.txt rows `R 01F0-01FF psg w=0 impl=stub` and the W twin; tests/memmap/memmap_rows.c:280 `{"psg", 0u, 0x0000u, 0xFFFFu, 0u}`; memmap_main.c:83; cmake/IvsMemmapProbes.cmake:24; width probe described memmap_rows.c:10-17 | Split the psg rows by per-register read width (the probe merges adjacent equal tuples), add psg rules, handle the two port registers and the read-masked shape register; ids `memmap.row.R-01F0-01FF` change names (data/expected/tests/memmap.ids) | 05-04 |
| 23 | Existing faults whose must-pass or collateral ids will change | data/expected/faults/run.ids:54 (`RUN_CAPACITY_AFTER_STEP ... run.buffers.real-layout-zero`), tests/run/run_info.c:93 (`audio_format == IVS_OUTPUT_FORMAT_NONE && audio_bytes_per_frame == 0u`), `run.input.guest-invisible` (run.ids:160; run_input.c:1397) | Update fault lines and tests in the plan that flips the layout (05-06) and the input visibility (05-05); `faults-verdict` demands the must-pass vector exist | 05-05, 05-06 |
| 24 | Gate registries | tools/gate.sh:66-81 (step inventory = union of `data/expected/gate-steps/*.ids`), run-controls toy tree, controls inventories `data/expected/controls/*.ids`, important-gates | New steps `input`, `psg-vectors` need a gate-steps line, toy fixtures in `tools/check/controls/_good/tree/` and a controls inventory line per control; every distinct `CHECK` step must pass on the pristine toy tree | 05-01 on |
| 25 | Docs | `docs/time-base.md:64` ("**PSG step**: 16 PSG chip clocks [INF] ... so 32 master ticks"), `docs/runner.md` key table, `docs/diagnostics.md` | Correct the PSG step (8 chip clocks, 16 ticks, 4 CPU cycles) BEFORE any PSG code; glossary step reads the term; runner and diagnostics docs follow their plans | 05-01, 05-09, 05-11 |

### Derived constants (executed and computed this session)

| Quantity | Value | Basis |
|----------|-------|-------|
| `ticks_per_cpu_cycle`, frame | 4; F = 262 x 57 x 4 = 59,736 ticks | profile fields `ticks_per_cpu_cycle` 4, `scanlines_per_frame` 262, `cpu_cycles_per_scanline` 57 [VERIFIED: data/profiles/intv-2609-ntsc-family-p1.json] |
| PSG step P | 16 ticks (8 chip clocks x `ticks_per_psg_clock` 2) | D-09; profile `ticks_per_psg_clock` is `"value": 2` tagged `unknown-default-chosen` today [VERIFIED: profile] |
| Samples per frame | 3,733.5, `ceil(F/P)` = 3,734 | arithmetic |
| g (minimum ticks between two PSG writes) | 9 CPU cycles = 36 ticks | [VERIFIED: probe over 1024 decles x SDBD 0/1; 60 writing decles; best pair is MVO direct (write at offset 6 of an 11-cycle unit) then MVO@ (offset 4): 11 - 6 + 4 = 9; MVO@ then MVO@ also 9; interrupt-entry DW at offset 2 then an instruction write at 4 gives 11; waits only lengthen it] |
| U, real machine bus (wait 0) | 12 CPU cycles = 48 ticks (decle `004`, the jump group) | [VERIFIED: probe `max_elapsed=12`] |
| U, every wait-taking access waits the limit 35 | 117 CPU cycles = 468 ticks (12 + 3 x 35) | [VERIFIED: probe `max_elapsed=117`, profile `bus.wait_limit_cpu_cycles` 35]; a wait-limit fault unit ends at `offset + 1 + limit + 1` (cp1610.c:137) so it is shorter than the maximum |
| Audio slack s = `ceil(U/P) + 1` | 4 (U = 48) or 31 (U = 468) | D-17 formula |
| `audio_bytes_per_frame` = 4 x (3,734 + s) | 14,952 or 15,060 bytes | arithmetic |
| `W_log` = `ceil(F/g) + ceil(U/g) + 1` | 1,660 + 2 + 1 = 1,663 (U = 48) or 1,660 + 13 + 1 = 1,674 (U = 468); 26,608 or 26,784 bytes per frame slot | arithmetic (the DISCUSS-audio illustration "g = 28, U = 608" does not match the code) |
| Default runner slice 1 << 22 = 4,194,304 ticks | frames = floor(4,194,304 / 59,736) + 1 = 71; audio capacity 1.06-1.07 MB, log capacity 1.89-1.90 MB; actual samples 262,144 = 1,048,576 bytes | arithmetic; sum about 3 MB against `IVS_RUNNER_BUFFERS_MAX` 8 MiB (ivs_runner.h:284) |
| Hash work | at most one SHA-256 block per audio level change (at most one per sample, 223,721 per emulated second) and one per logged write (at most 1 per 36 ticks, about 99,500 per emulated second) | arithmetic; cap fuzz lengths (Pitfall 8) |

### CONTEXT assumption check (against the code)

| CONTEXT statement | Verdict | Evidence |
|-------------------|---------|----------|
| Composite is v1 over domains 1..4 | TRUE | machine.h:110 `#define IVS_DIGEST_COMPOSITE_VERSION 1u`; machine_digest.c:317-318 comment "Composite version 1: domains 1..4 only" |
| `psg_regs` is a stub with fill $FFFF | TRUE | profile `memmap.region.psg_regs` value `{"view": "RW", "lo": "$01F0", "hi": "$01FF", "device": "stub", ..., "impl": "stub"}`; `memmap.device.stub` value `{"bits": 0, "rmask": "$0000", "fill": "$FFFF", "store": "drop"}` |
| Access timing offsets are available to the bus for the write-log stamp | PARTIAL | offsets are in the access record, but unit start and ticks per cycle are not in the bus context (`port.ctx = &im->mem`); needs the wrapper (mechanism 1) |
| Input words are applied at the first unit boundary with start >= at_tick | TRUE | machine_bound.c:444 `consumed = ivs_input_apply_due(...)` before the seam call at :447; machine_input.c:136 `if (e.at_tick > start) { break; }` |
| Refused-step take-back restores the input row | TRUE for `obs.input`; INCOMPLETE once `port_pulldown` lives in memsys | machine_bound.c:426 and :450 |
| One PSG step is 16 chip clocks in the glossary (needs correction) | TRUE (it is wrong) | docs/time-base.md:64 |
| D-17: U "asserted by a property test over every opcode with maximal waits" | UNDER-SPECIFIED | real bus U = 48 ticks, synthetic maximal waits U = 468 ticks; the DISCUSS illustrations (g = 28, U = 608) are wrong for this code |
| D-19 rebases the request minimum size | INCOMPLETE | the report (112) and env seam need the same rebase (mechanisms 13 and 14) |
| D-18 "only composite, instance and audio lines may differ" in goldens | TRUE only if the new rows are tagged INSTANCE (not MEM) and the golden key list is not extended in the same plan | digests 1..3 unchanged requires rows outside domains 1..3; the golden key list (main.c:243-262) hides new keys |
| D-21 `tools/pcm` is an internal static target with the symbol deny list extended | PARTIAL | `core-symbols-deny.txt` only labels (its header: "the allowlist decides what fails"); the real guards are the exact install inventory and the pkg-install public-symbol rule; a `tools/pcm` TU is outside `runner-scan`, `c-build`'s unlisted-source guard (c-build.sh:42 covers `core`, `runner`, `tools/pack`), integrity (tools/check/integrity.cmake:73 `tools/(pack|cart)/`) and the fault modules (core, runner) |
| 05-01 may flip the memmap literal (data only) | FALSE | see mechanism 22; the memmap step compares literal rows with the runtime dump |
| 05-01 may commit `data/golden/diag-ctrl-psg-1.expect` | FALSE for a gate-green 05-01 | golden.sh requires a `# scenario:` naming a tracked scenario in `runner-scenarios.ids` and an existing dev report |
| Phase 4 had no `ivs-run` buffers | TRUE and harmless only because of D-17 | runner/main.c never sets `host.buffers` (mechanism 18) |

### Recommended structure (additions)

```
core/src/psg/psg_params.h       explicit ivs_psg_params struct (one per profile) passed to every function
core/src/psg/psg_regs.c psg_tone.c psg_noise.c psg_env.c psg_mix.c psg_sample.c psg_log.c   small TUs: a fault recompiles one file
core/src/machine/machine_bus.c  wrapper ctx, stamps, unit-end catch-up (new TU; stems unique across core and runner)
cmake/IvsCtrlTable.cmake        control lines (D-02), name set == IVS_CTRL_* set, -DMUTATE=<control>
cmake/IvsPsgTable.cmake         psg.* objects -> ivs_psg_params[profile] (not named in CONTEXT; needed)
data/input/intv-2609-ntsc-family-p1.literal.txt  data/psg/{rules.json,vectors/*.txt}
data/expected/{input-policies,psg-rules,psg-log-source,d10-measurements}.ids  tests/{input,psg}/*.c
tools/check/steps/{input,psg-vectors}.sh   docs/{input/controller-notes.md,psg/reference-notes.md,input/d10-procedure.md}
fixtures/FX-diag-ctrl-psg-1/{manifest.json,diag-ctrl-psg-1.listing.txt}  data/scenarios/diag-ctrl-psg-*.ivss
tools/pcm/ivs_refpcm.[ch] or runner/refpcm.[ch]   (Open Question 3)
```

### Pattern 1: machine bus wrapper (the stamp seam)

**What:** the CPU calls `bus(ctx, &access)` at the moment of each access with `access.offset_cpu_cycles` relative to the unit start. Make `ctx` a stack struct built per unit: `{ ivs_machine *m; uint64_t start; uint32_t ticks_per_cycle; psg_sink *sink; }`. The wrapper delegates everything to `ivs_memsys_bus` except a WRITE or DW whose address lies in the PSG region: for those it first runs `psg_catch_up(stamp, sink)`, then stores the register (memsys setter), then appends the log record and advances the log chain. Reads never go through the PSG engine (pure). Time never goes through a global.
**Why:** it is the only place that has both the access offset and the unit start; it keeps `ivs_memsys` free of time (its digest rows and `peek` stay pure); the sink travels in the context, so no static state exists (core-purity bans mutable statics).
**Ordering rule (D-16):** "step first, then write": a sample at `k * P` observes writes stamped `< k * P`; every stamp is `>= unit start >= previous unit end >= last catch-up`, so `psg_catch_up(stamp)` never moves backwards.

### Pattern 2: eager unit-end catch-up inside the seam

**What:** after `im->time_ticks += ...` (machine_run.c:146) the seam calls `psg_catch_up(now, sink)`; the run call passes its call sink, tests that use the seam alone get a counting sink. Samples for `k * P <= now` are produced exactly once, so the stream is identical to catching up only at call end, and every instance is caught up at every unit boundary.
**Why:** tests compare run-call instances with seam-stepped twins by whole-storage `memcmp` (tests/run/run_slice.c header, run_fuzz.c, run_stub.c, machine reset and iso suites). With catch-up only at call end a seam-only twin would differ in PSG counters and the digest would need a "caught-up tick" row to even describe it.

### Pattern 3: explicit parameters for every PSG function

`psg_step(state *, const ivs_psg_params *, sink *)`: the engine reads parameters only from the struct it is given. The registry hands out the struct generated from the profile; perturbation tests (V-04, `psg.profile.*`) build a perturbed copy on the stack. This replaces a registry seam that does not exist: `ivs_profile_get()` returns `const` static generated data (memsys.h:140-141), and the only existing mutation path is the generator's `-DMUTATE` (a build-time fault, tests/faults.cmake `ivs_fault_gen`).

### Pattern 4: validate-then-commit stays; capacity is computed from the layout

Extend the Phase 4 ladder in place (machine_bound.c:261-349): one more buffer in stage 4b (video, audio, psg_log order), the capacity stage 7 checks three channels with `ivs_bound_fits` (machine_bound.c:100-119; keep the `frames == UINT64_MAX` guard and the `UINT64_MAX / bytes_per_frame` overflow guard). D-17: a channel is "not requested" only when BOTH the pointer is NULL and the capacity is 0.

### Pattern 5: produce/consume split for new gate steps

`input` and `psg-vectors` are pure `cmake -P` or awk steps over text files (no `string(JSON)` in new steps: the Phase 4 cost profile names JSON re-parsing the largest pure-step cost). Cross-checks against generated tables belong to suites (`input.table.matches-literal`), not to pure steps, because pure steps cannot read build outputs under `build/` (04-RESEARCH anti-pattern).

### Anti-patterns

- Putting PSG registers or the pull-down in domain MEM (changes `digest.mem` of every golden); tag them INSTANCE.
- Deriving the expected byte table from the generated profile table or from `core/src` headers in a test (circular; the expected values are the literal file).
- A PSG model in cmake or shell to produce expected samples (rejected in V-03; use closed-form bounds).
- Catching the PSG up to the call TARGET (D-16: only to `now`, the unit boundary).
- Hashing a struct by memory (padding); frame every digest and log byte explicitly (be64, be32).
- Using the Phase 4 per-record `index || len || record` framing for the audio chain (breaks slicing invariance; D-18 already forbids it).
- Putting the log record's byte order in the host's hands: write the 16-byte record and the sample byte by byte in a pinned order (recommendation: big-endian, matching the digest and IVSC framing).

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| SHA-256 (chains) | a second implementation | `core/src/common/ivs_sha256.c`; independent oracle `cmake -E sha256sum` over a `printf`-framed byte string (tests/run/run_input.c:1-35 recipe) | already FIPS-vector tested; oracle is a different implementation |
| Expected-value generators for steps | `string(JSON)` loops in new steps | line-based text formats copying `data/memmap/*.literal.txt` and the listing grammar | cost and one rule set per format |
| Profile to C tables | a C JSON parser | `cmake -P` generators in the `IvsMemmapTable.cmake` style (`mm_keys`, `mm_hex`, fail-closed, `-DMUTATE`) | no JSON parser may enter the core (H14) |
| Named faults | a new loop | `data/expected/faults/<suite>.ids` + `ivs_fault_suite` + `faults-verdict` | per-fault intended-reason check exists |
| Negative controls | a new mechanism | `tools/check/run-controls.sh` overlays on `_good/tree` | exact inventories exist |
| Slicing oracle | a second run implementation | `run(now + 1)` per unit as the reference plus relational checks (D-05 content C) | the reference goes through the same code; independent oracles only where the program is schedule-computable (content B) |
| Reference PCM | anything in `core/` | the pure integer converter of D-21 | rate, filter and DAC table are not chip truth |

**Key insight:** the new risk is registration breadth (profile fields, generator, inventories, toy trees, both key lists), not algorithms; the engine is about 400 lines of integer code.

## Plan Decomposition (re-cut check of D-27)

File-set collisions found in the CONTEXT outline (shared files: `ivs_run.h`, `ivs_machine.h`, `machine.h`, `machine_bound.c`, `machine_digest.c`, profile JSON, `profile-fields.ids`, `api-effects.json`, `tests/CMakeLists.txt`, goldens): 05-02 as outlined carries ABI growth, profile fields, the composite bump and PSG stubs (over 5 tasks and a choke point); 05-03 carries a register file, the memmap flip with suite changes, the digest rows and suite creation; 05-08 carries runner grammar, report keys, goldens and (D-21) the converter and WAV path. 05-04 and 05-05 collide on profile JSON, `memsys.c`, `machine_run.c`. Recommended cut (12 plans, 11 waves; every plan ends gate-green under sh, dash and, where marked, gate-linux):

| # | Plan | Reqs | Depends on | Core deliverables (summary) | Faults / controls (est.) | Tasks | Gate before push |
|---|------|------|------------|-----------------------------|--------------------------|-------|------------------|
| 05-01 | Sources and oracles (no C) | all | none | page reading and `refs/` fetch (hash-verified), `docs/psg/reference-notes.md`, `docs/input/controller-notes.md`, controller literal, `data/psg/{rules.json,vectors}`, `psg-rules.ids`, ONE discrepancy allocation (C69..C77+) and `claims` reservations FIRST (profile fields cite them: `conflict` must exist in `discrepancies.json`), steps `input` and `psg-vectors` (self-consistency only: pins to bits to wiki value, grammar, rule coverage), `d10-measurements.ids` + discrepancies step check, listing branch-target rule (`-> ADDR` annotations added to both existing listings, manifest hashes updated), `docs/time-base.md` PSG-step correction, derivation of the diagnostic as a doc (not yet `data/golden/*.expect`), second-read requests | 0 / about 17 | 5 | sh, dash |
| 05-02 | ABI growth, behaviour-neutral | API-03 (support), AUD-02 | 05-01 | `IVS_RUN_REQUEST_V1_SIZE` 64 and `IVS_RUN_REPORT_V1_SIZE` 112 rebases, request v2 and report v2 fields, ladder stage for `psg_log`, reasons 55..57 + pins + effect rows, `ivs_info` v3 fields (zero), env seam fields, D-17 NULL plus 0 rule with the existing run.buffers.* tests updated, new define class pin for log source, `run.struct.*` and headers ids | about 3 / 3 | 4 | sh, dash, linux |
| 05-03 | State and digest foundation | AUD-02 | 05-02 | FIRST: re-record v1 composite and stepped literals; then instance layout (PSG device struct, memsys register and pull-down rows tagged INSTANCE, wider audio row, domain 19 rows), `IVS_MACHINE_BYTES`, composite v2, `ivs_digest_domain_known`, hand-extended domain-4 literal stream, retarget the domain-19 refusal to 20, power-on reset of the new rows, golden re-bless #1 (classified), `pkg-consumer.golden` | 3 / 0 | 4 | sh, dash, linux |
| 05-04 | PSG register file, memmap flip, suite `psg` | AUD-01 | 05-03 | `psg8` device kind and generator changes, `IvsPsgTable.cmake` (regmap and params), per-register masks, writes through the bus wrapper (no engine yet), memmap literal split + rule table + descriptor row, `ivs_ctrl` untouched, suite `psg` created (`psg.reg.*`, `psg.port.*`), measurement checkpoint 1 | 3 / 2 | 5 | sh, dash |
| 05-05 | Input byte and port reads | INP-01, INP-02, INP-03, API-03 | 05-04 | `IvsCtrlTable.cmake` + 34 profile fields, pull-down setter and apply wiring and take-back, peek and guest read, `psg8` upper byte, flip `run.input.guest-invisible`, fuzz B and C, C67 test, `INPUT_EXTRA_CONTROLLER_UNCHECKED`, suite `input`, `input` step cross-check extension | 7 / 3 | 5 | sh, dash |
| 05-06 | Tone, mixer, amplitude, sampling, audio live | AUD-01, AUD-02 | 05-05 | engine core (tone, mixer, amp, step, sample record), wrapper catch-up, eager seam catch-up, per-call sink and counting sink, audio buffer capacity and `audio_first_index`, real `ivs_info` layout (flips `run.buffers.real-layout-zero`, `run.info` audio asserts and fault line run.ids:54), domain 17 chain, golden re-bless #2 | 6 / 0 | 5 | sh, dash, linux |
| 05-07 | Noise and envelope | AUD-01 | 05-06 | LFSR, envelope, profile perturbation tests, property guards, DAC table as data, golden re-bless #3 | 6 / 0 | 4 | sh, dash |
| 05-08 | Write log | AUD-02 | 05-07 | log record, third buffer wiring, `W_log` and property test for g and U, replay equivalence, bus-trace cross-check, domain 19 chain, checkpoint 2 | 6 / 1 | 5 | sh, dash, linux |
| 05-09 | Runner and scenarios | AUD-02, API-03 | 05-08 | buffers in `main.c`, three-way split in `exec.c`, `ivsr 2`, new keys, golden key list in both places, `# domains:` grammar, IVSS `capture`/`assert psglog|audio`, `--audio-raw`, `--psg-log-out`, `--audio-out-max-bytes`, parser table and fuzz rows, `psg-log-assert-mismatch` scenario, golden re-bless #4 (key-list extension only) | 3 / 4 | 5 | sh, dash, linux |
| 05-10 | Reference PCM converter and WAV helper | AUD-02 (D7) | 05-09 (wiring) or 05-07 (library only) | `ivs_refpcm` pure integer code and its tests in suite `runner`, `--audio-wav`/`--wav-rate` wiring, symbol and inventory proofs, pitch within 1 percent vs `crystal/(32*TP)` | 2 / 3 | 4 | sh, dash, linux |
| 05-11 | Fixture and end to end | INP-01..03, API-03, AUD-01, AUD-02 | 05-09 | `ivs-diag-ctrl-psg-1` listing, manifest, `.expect` (derivation committed in 05-01 moves into the header here), four scenarios, five program mutants, installed consumer replay and golden, claims CL-0014/0015 | 0 / 0 | 4 | sh, dash, linux |
| 05-12 | Close-out | all | all | claims CL-0009..0013, discrepancy updates, `important-gates.ids` lines, D-45 evaluation, VERIFICATION three tables | 0 / 3 | 3 | sh, dash, linux |
| 05-L1 | Conditional lever | | trigger only | V-16 lever order | | | |

Wave suggestion: W1 05-01; W2 05-02; W3 05-03; W4 05-04; W5 05-05; W6 05-06; W7 05-07; W8 05-08 (05-10's pure library part may run in parallel: disjoint files except `tests/CMakeLists.txt` lines); W9 05-09; W10 05-11; W11 05-12. Keeping ten plans instead requires 6-task plans; the caps (5 tasks, 8 faults, 25 controls) make the split cheaper than a mid-plan overrun.

**Sequencing constraints (hard):** C-id and claim allocation (05-01) before any profile field cites them; the v1 composite re-record is the FIRST task of 05-03; 05-02 before any plan that writes a grown struct field; 05-04 before 05-05 (device kind and generator); 05-06 before 05-08 (log shares the wrapper and sink); 05-08 before 05-09 (`capture psglog`); the `input` and `psg-vectors` steps get their profile cross-checks in 05-05 and 05-04/05-07 with extra controls; `gate-linux` before pushing 05-02, 05-03, 05-06, 05-08, 05-09, 05-10, 05-11, 05-12.

## Common Pitfalls (ranked)

### Pitfall 1: golden and literal churn across behaviour plans (determinism evidence)
**What goes wrong:** with PSG state in the digest, tone counters, the noise register and the envelope step counter evolve with time even for guests that never touch the PSG, so `digest.instance` and `digest.composite` of every golden change in each plan that changes engine state (05-03 layout, 05-06 tone/sampling, 05-07 noise/envelope) and `pkg-consumer.golden` follows. The golden step compares the dev report with the golden body and fails otherwise (golden.sh header). Also pinned: `md_fresh_composite`, `md_stepped`, the hand-framed domain-4 stream (machine_digest.c:95-110).
**How to avoid:** each such plan ends with one task: re-run `ivs-run --update-golden` for the three `.digests` goldens and refresh `pkg-consumer.golden`, then classify the diff in the SUMMARY (only `digest.instance`, `digest.composite`, later `output.audio.bytes`/`digest.audio` lines may change; `digest.cpu`, `digest.mem`, `digest.images`, every `step.*`, `slot.*`, `cpu.*` line byte-identical). Rows tagged INSTANCE keep domains 1..3 stable. Never edit a golden by hand.

### Pitfall 2: layout and alignment of the grown instance
**What goes wrong:** `mem` is 113,228 bytes and ends at 4 mod 8 (machine.h comment), which is why `obs_pad` exists; adding 36 bytes to `ivs_memsys` makes `obs_pad` wrong; any `uint64_t` in the PSG struct or widened audio row opens a hole that `md_coverage` reports ("byte N covered 0 times").
**How to avoid:** derive the layout on paper first, keep explicit PAD rows for every implicit byte, put everything before `slot[]`, update the three static asserts and `IVS_MACHINE_BYTES`; PSG internal state in `uint32_t`.

### Pitfall 3: lazy catch-up versus seam-stepped twins
**What goes wrong:** catching up only at call end leaves seam-only instances behind; whole-storage comparisons (run.slice, run.fuzz, run.stub, machine reset and iso families) then fail in a way that looks like a PSG bug.
**How to avoid:** Pattern 2 (eager unit-end catch-up in the seam); a test `psg.slice.seam-equals-run` compares a seam-stepped instance with a run-call instance at the same unit count, byte for byte.

### Pitfall 4: ABI growth done once, for one struct
**What goes wrong:** appending request fields while leaving `sizeof`-based minimums rejects v1-sized callers (the installed consumer is built against the installed header, and a Phase 4 caller would break); the report is copied whole (`*rep = r`, machine_bound.c:499) and the refusal path writes `struct_size` as `sizeof(ivs_run_report)`; `ivs_info` min size is already pinned at 216. On ILP32 the request's `sizeof` differs (header says 64 bytes with 64-bit pointers, ivs_run.h:197), so a pinned v1 constant of 64 is an LP64 statement: keep the existing `run.struct.v1-sizes-lp64` skip-ok pattern.
**How to avoid:** pin `IVS_RUN_REQUEST_V1_SIZE`, `IVS_RUN_REPORT_V1_SIZE`, `IVS_INFO_V1_SIZE`; read an appended request field only when `struct_size` covers it; write `min` bytes of the report; tests with v1-sized, exactly-v2-sized and larger-with-nonzero-tail callers; a fault in the style of `INFO_TAIL_UNGUARDED`.

### Pitfall 5: capacity formulas rest on U and g
**What goes wrong:** U and g are properties of the CPU model plus the bus; Phase 6 will make waits real. With U = 48 baked in, a Phase 6 stall silently makes the audio and log capacity too small and the "never expected" INTERNAL stop becomes reachable.
**How to avoid:** derive U from profile fields (`(max base cycles 12 + 3 x bus.wait_limit_cpu_cycles) x ticks_per_cpu_cycle` = 468) or pin 468 with a property test through the CPU model with a synthetic bus at the wait limit AND assert that the real memsys bus never waits; g = 36 holds with waits. Record in the SUMMARY which one was published (Open Question 1). Note the D-17 NULL plus 0 amendment changes Phase 4 tests that expected refusal for NULL/0 with a nonzero layout (`run.buffers.*`).

### Pitfall 6: the memmap probe sees per-register widths
**What goes wrong:** `memmap.dump.matches-literal` probes every address with raw pokes and bus writes and merges adjacent equal tuples; PSG registers have masks `$00FF`, `$000F`, `$001F`, `$003F`, a read-masked shape register and port registers whose reads depend on direction and pull-down (not on the backing). A single `psg w=8` literal row fails the dump comparison.
**How to avoid:** split the literal rows by width in the same plan as the device, give the rule table psg rows, decide how ports and the shape register are probed (they may need a dedicated row and a declared exception in the test), keep `impl=real` and the `psg8` upper byte; update `memmap.row.*` ids.

### Pitfall 7: gcc versus clang in code that does not exist yet
**What goes wrong:** `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion ... -Werror` (IvsWarnings.cmake) is on in both presets; narrowing of `uint8_t`/`uint16_t` expressions, `int` promotion of 16-bit counters, and unspecified argument evaluation order have each cost hosted round trips (CI-EFFICIENCY.md "hosted-only failures": argument order in `machine_refuse.c`, `_GLOBAL_OFFSET_TABLE_`; Phase 4: glibc `signal()`).
**How to avoid:** all PSG state and arithmetic in `uint32_t` with an explicit narrowing helper like `IVS_U16` (cp1610.h:24); counters up to `2 x 65,536` = 131,072 do not fit 16 bits; one call per statement when arguments read state another argument changes; no struct hashing; run `sh tools/gate-linux.sh` at the marked plans; `#pragma GCC poison float double` already bans floats.

### Pitfall 8: gate cost is dominated by reruns, not by the engine
**What goes wrong:** 26 faults each rebuild one small TU and rerun a whole suite; sanitizer builds are 3 to 10 times slower; the last-5 median `cpu_s` is 175 against a trigger of 200 (CI-EFFICIENCY.md Phase 4 close-out, rows in `.planning/ci-runs.tsv`). A noisy channel makes every sample a level change and every change one SHA-256 block (about 223,000 per emulated second).
**How to avoid:** keep suite plain-run time under 0.3 s release and 1.5 s dev; fuzz schedules at most 100,000 ticks (6 emulated milliseconds) per seed and avoid noise period 1 in fuzz; text-format vectors; judge self-controls and controls before faults; measure at checkpoints 05-04 and 05-08 (V-16); lever order is pre-committed.

### Pitfall 9: unit-start granularity and what a poll sees
**What goes wrong:** an event applies before the unit that begins at or after its tick, but a poll instruction reads at offset 4 or later inside that unit; the byte it reads is decided by the event relative to the START of the poll unit, not the read.
**How to avoid:** the C67 test places events at `s - 1`, `s`, `s + 1` of an OBSERVED unit start (single-step with `run(now + 1)`), never predicted from the cycle table.

### Pitfall 10: hygiene and rights in the oracle plan
**What goes wrong:** datasheet text pasted into a vector file or note; fetched PDFs placed in a tracked path; a scratch path in a script; `.planning` text with a home path (hygiene `HOME_ALL`).
**How to avoid:** vectors and notes are paraphrase plus page locators; PDFs only under `refs/` (ignored); `data/sources.json` entries with sha256, size, `local_path`, `rights: unknown`; scripts use `mktemp -d`; the oracle commit precedes the implementation commit (D-23, checked by the verifier through `git log`).

### Pitfall 11: the golden step and `.expect` ordering
**What goes wrong:** a hand-derived `.expect` committed before its scenario fails `golden` (no scenario, no report); the `.expect` assert lines must equal the scenario's assert lines in order; the `# domains:` grammar is fixed and a new `psglog` domain is not in it.
**How to avoid:** derivation as a doc in 05-01, `.expect` in 05-11 with its scenario in the same plan; extend the golden grammar and controls when `psglog` becomes a reported domain (05-09).

### Pitfall 12: test reference vs a schedule-dependent guest
**What goes wrong:** Phase 4's fuzz computes boundaries from a seam twin without events because input was invisible (run_fuzz.c header: "input is guest-invisible in Phase 4"). With a polling guest the boundaries depend on the schedule.
**How to avoid:** content A (a forever loop that never reads ports) keeps the Phase 4 oracle; fuzz B (constant-cycle poller) computes its oracle from the schedule; fuzz C (data-dependent loop) uses the one-unit-per-call reference as a relational oracle at every boundary (D-05).

## Code Examples

### Wrapper sketch (Pattern 1; written for this project, no third-party code)
```c
/* Source: derived from cp1610.h:48-55, cp1610.c:116-122, machine_run.c:126-131. */
typedef struct ivs_bus_ctx {
  struct ivs_machine *m;
  uint64_t start;            /* unit start tick */
  uint32_t ticks_per_cycle;  /* profile ticks_per_cpu_cycle */
  ivs_psg_sink *sink;        /* per-call stack sink */
} ivs_bus_ctx;

static ivs_bus_reply ivs_machine_bus(void *ctx, const ivs_cp1610_access *a)
{
  ivs_bus_ctx *c = (ivs_bus_ctx *)ctx;
  if ((a->kind == IVS_ACC_WRITE || a->kind == IVS_ACC_DW) && ivs_psg_addr(a->addr)) {
    uint64_t stamp = c->start + (uint64_t)a->offset_cpu_cycles * c->ticks_per_cycle;  /* + write_strobe_ticks */
    ivs_psg_write(c->m, stamp, a, c->sink);   /* catch up to stamp, store masked register, log, chain */
    return (ivs_bus_reply){0u, 0u};
  }
  return ivs_memsys_bus(&c->m->mem, a);
}
```
Compound-literal return is C17; keep one call per statement in tests.

### Capacity requirement (D-14, D-17) as the executable property
```
required_audio = audio_bytes_per_frame * (max(target - now, 0) / F + 1)
audio_bytes_per_frame = 4 * (ceil(F / P) + ceil(U / P) + 1)
required_log   = 16 * W_log * (max(target - now, 0) / F + 1),  W_log = ceil(F / g) + ceil(U / g) + 1
property test: for every opcode x SDBD x wait in {0, limit}, a run of T ticks emits at most floor((T + U) / P) + 1 samples
and at most (T + U) / g + 1 writes; a tight MVO loop reaches the log bound within 1 record.
```

### Independent oracle for the table (pins to bits; literal file, not the generator)
```
pin map: pin 4=b0, 3=b1, 2=b2, 1=b3, 9=b4, 8=b5, 7=b6, 6=b7
KEY_1 pins 4,6 -> b0 + b7 = $81 -> port byte $FF xor $81 = $7E   (wiki anchor: key 1 reads $7E)
KEY_1 + KEY_9 (pins 4,6 and 2,8) = $81 or $24 -> $A5 -> $FF xor $A5 = $5A   (wiki anchor)
```
These two values are quoted in 05-DISCUSS-ctrl.md P5-C1 and D-02; the planner reproduces them in the literal file from the sources, never from the generator [VERIFIED by hand arithmetic this session for KEY_1, KEY_9 = pins 2,8 = b2 + b5 = $24].

## State of the Art

| Phase 4 | Phase 5 | Impact |
|---------|---------|--------|
| Input recorded, digested, invisible to the guest | Input guest-visible through the PSG ports | `run.input.guest-invisible` becomes a positive read test; fuzz oracles change |
| Composite v1 over 1..4, no device state | Composite v2 including PSG state | all goldens change in a classified way |
| `ivs-run` report `ivsr 1`, audio bytes 0 | `ivsr 2`, real raw audio and log | key lists in four places |
| Output layout NONE | audio and log layouts real, video still NONE | `BUFFER_TOO_SMALL` reachable through the public API for audio (closes the Phase 4 verification warning 3) |
| PSG region stub with fill `$FFFF` | `psg8` device, upper byte `$00` (C64) | memmap literal and probe change |

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| CMake, Ninja, cc | everything | yes | 4.4.3, 1.13.2, Apple clang 21 | none needed |
| gcc (native) | gcc-only behaviour | no (Apple clang under both names) | n/a | `sh tools/gate-linux.sh` |
| dash | POSIX-sh parity | yes | `/bin/dash` | none needed |
| docker + `ubuntu:24.04` | gate-linux | yes (daemon up, image cached) | Docker 29.5.2 | skip only for record-only plans |
| `pdftotext`, `pdftoppm`, `tesseract`, `qpdf` | plan 05-01 page reading | yes | n/a | none needed |
| bitsavers (network) | plan 05-01 hash-verified fetch of the 1980 and 1982 catalogs (not in `refs/`) | yes, with a browser user agent | 200 | if unreachable, record "pages not re-read" and keep the C12 defaults `unknown-default-chosen` (D-23 fallback) |
| `gh` | `tools/ci-record.sh` | orchestrator only | n/a | executors never call it |
| Owner hardware, EXEC/GROM | D10, D11 | not available | n/a | record missing, hand off |

**Missing dependencies with no fallback:** none.

## Validation Architecture

Full map, ids and sampling: `05-VALIDATION.md`. Summary:

- Framework: in-repo harness (`tests/harness/ivs_test.[ch]`) under ctest, new suites `input` and `psg` (own executables and fault files, V-15), shell TAP (`runner-process`), pure steps (`input`, `psg-vectors`, extended `listing`, `discrepancies`, `golden`), named faults, negative controls.
- Quick command per task: `cmake --build --preset dev && build/dev/tests/ivs_tests_<suite> --only <prefix>`; per plan: `sh tools/gate.sh` and `dash tools/gate.sh`; marked plans: `sh tools/gate-linux.sh`.
- Independent oracles: wiki anchors `$FF`, `$7E`, `$5A`; pins-to-bits arithmetic in the literal; AY-8910 page equations by hand (rate bounds only, if the pages print them); closed-form rate bounds; slicing invariance; log replay equivalence; log vs CPU bus trace; `cmake -E sha256sum` over framed bytes; the hand-derived diagnostic. Self-recorded goldens are repeatability only.

## Data File Updates (discrepancy log, profile fields, claims, inventories)

| File | Edit | Plan |
|------|------|------|
| `data/discrepancies.json` + `data/expected/discrepancies.ids` | next free id is C69 [VERIFIED: ids end at C68]; allocate C69..C76 per `05-DISCUSS-psg.md` section 5.3 and C77 onward for the verification note's extras in ONE table; update C11, C12, C14, C15, C16, C23, C63..C67 (resolvers match `^(test|measurement|pending):.+`); every open entry says `unknown-default-chosen:`; the profile checker requires `conflict` to exist before a field cites it | 05-01 |
| `data/profiles/intv-2609-ntsc-family-p1.json` + `data/expected/profile-fields.ids` | add `input.control.<31 names>`, `input.line_model`, `input.line_idle`, `input.port_output_read`, `psg.regmap`, `psg.mirrors`, `psg.clocks_per_step`, `psg.sample_period_ticks`, `psg.write_strobe_ticks`, rule fields, `psg.dac.levels`, `psg.log_records_per_frame` (or derived), `psg.max_unit_ticks`; retag `ticks_per_psg_clock` to `inference`; change `memmap.region.psg_regs` to `psg8`, `impl real`; add `memmap.device.psg8`; one conflict id per field (profile schema `match conflict ^C[0-9]+$`); fields are added by the plan whose generator consumes them, never earlier (unused fields would be unverified) | 05-04..05-07 |
| `data/memmap/intv-2609-ntsc-family-p1.literal.txt` | split the `01F0-01FF` psg rows by width, `impl=real`; CL-0006 depends on this file | 05-04 |
| `data/claims.json` + `claims.ids` | next free CL-0009 [VERIFIED: ledger ends CL-0008]; CL-0009..CL-0015 per D-26, weakest-field rule, `standing: provisional` | 05-11, 05-12 |
| `data/sources.json` + `sources.ids` | entries for the AY-8910 reprint, the pin tables and the wiki revision: `capture: hashed` (sha256, size, pages, `local_path` under `refs/`) or `reference-only`, `rights: unknown` | 05-01 |
| `data/expected/` | new: `input-policies.ids`, `psg-rules.ids`, `psg-log-source.ids`, `d10-measurements.ids`, `tests/{input,psg}.ids`, `faults/{input,psg}.ids`, `gate-steps/<new>.ids`, `controls/<new>.ids`, `runner-scenarios.ids` (+4), `diag-mutations.ids` (+5), `listings.ids` (+1), `fixtures.ids` (+1), `important-gates.ids`, `pkg-consumer.golden`; changed: `api-reasons.ids` (55..57), `core-generated.ids`, `tests/{run,memmap,machine,runner,runner-process}.ids`, `faults/{run,machine,runner}.ids`, `profile-fields.ids` | all |
| `tools/check/controls/_good/tree/` | toy literal, toy vectors, toy profile fields for `input` and `psg-vectors`; toy reports gain nothing until `ivsr 2` (05-09) | 05-01, 05-09 |
| `.planning/PROJECT.md` Decision Register | D7 (delegated, D-21), D6 note, D3 note; `.planning/CI-EFFICIENCY.md` checkpoint entries | 05-12 |

## Security Domain

| ASVS category | Applies | Standard control |
|---------------|---------|------------------|
| V5 Input validation | yes | scenario parser caps and charset (IVSS stays inert text, no directive names a file or a rate), whole-array event validation before any write, request/report struct-size rules, buffer overlap checks with `uintptr_t` differences |
| V12 Files and resources | yes | operator-only output options written only in `hostio.c`, exclusive create, byte cap `--audio-out-max-bytes`; reads bounded as in Phase 4 |
| V6 Cryptography | partly | SHA-256 only as identity and framing digest; never hand-rolled beyond the existing implementation |
| V2, V3, V4 | no | no network service or users |
| V14 Configuration | yes | workflow unchanged (no cache, no artifact, no new action; ci-parity bans stay) |

| Threat | STRIDE | Mitigation |
|--------|--------|------------|
| Output buffer aliasing the instance, report, events or another buffer | Tampering | stage 4b overlap pairs for the third buffer, nothing written on refusal, poison-pattern tests |
| Undersized log or audio buffer silently truncates the cause stream | Repudiation | capacity checked before the first unit; INTERNAL stop as safety net; no partial record |
| Huge target or capacity wraps a product | Tampering | 64-bit guards of `ivs_bound_fits` retained; target above 2^63 refused before capacity |
| WAV or raw output path chosen by a scenario | Elevation | scenarios cannot name output files; operator flags only |
| Rights-bearing audio from a user cartridge leaving the machine | Information disclosure | operator-local artifact, never committed, never CI-uploaded; WAV extension not on the binary-by-name list so the review rule is the guard |
| Home or scratch path in a tracked file | Information disclosure | `hygiene` step; `<repo-root>` placeholders |

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | Hosted gcc 13 `-Wconversion` behaviour on new narrowing patterns matches past findings (classes only, no new code exists yet) | Pitfall 7 | an extra hosted round trip; mitigated by `gate-linux.sh` |
| A2 | `psg_catch_up` cost is about 40-80 ns per step under the dev preset (no PSG code exists to measure) | Pitfall 8 | suites slower than 1.5 s; plan SUMMARYs record measured plain-run times, checkpoint after 05-04 and 05-08 |
| A3 | `sizeof(ivs_info)` v3 of 288 bytes with `crystal_hz_*` as u64 at 272 and 280 | mechanism 15 | none to correctness (a static assert fixes the real value) |
| A4 | Moving the composite bump from 05-02 to 05-03 saves one golden re-bless | Plan Decomposition | equal if the planner keeps CONTEXT's order; an extra re-bless |
| A5 | The 1980 catalog pages 204-210 (printed 5-23..5-29) carry the AY-8910 equations (read by the discuss agents, which cite "pdf 205" and "pdf 207"; plan 05-01 must re-read, D-23) | Summary, Environment | SC4 falls back to `unknown-default-chosen` defaults with structural properties only (D-23 fallback) |
| A6 | Hosted `dash` and `g++` as in Phase 4 | Environment | hosted-only failure class, caught by gate-linux |
| A7 | `fopen` mode `"wx"` (C11 exclusive create) is accepted by glibc and Apple libc under `-std=c17`; otherwise `open(O_EXCL)` in `hostio.c`, which already holds POSIX | D-20 | `hostio.c` change only |

## Open Questions (recommendations)

1. **U for the capacity formula (D-17).** Real bus: 48 ticks. Synthetic maximal waits: 468 ticks. Recommendation: publish 468 (profile-derived from `bus.wait_limit_cpu_cycles`), property-test the CPU at the wait limit, and add a test that the memsys bus returns wait 0 so a Phase 6 change fails loudly; g = 36 ticks regardless.
2. **Seam catch-up.** Recommendation: eager at every unit end (Pattern 2); the digest then needs no caught-up-tick row, but a derived `next_sample_index` row is cheap insurance and detects a missed catch-up.
3. **Where does the reference converter live?** D-21 says `tools/pcm`. Recommendation: honor it and extend, in the same plan with one control each: the integrity anchor (`tools/(pack|cart|pcm)/`, integrity.cmake:73), the `c-build` unlisted-source guard (c-build.sh:42), and a runner-scan allowance for `pcm/ivs_refpcm.h`; faults for it are impossible outside core and runner modules, so use judge self-controls (`runner.judge.*`) instead. Alternative: `runner/refpcm.[ch]` gets all four guards free; nothing in D-21's substance changes (outside the core, not installed). Planner's choice; either is gate-checkable.
4. **Log byte order.** Recommendation: big-endian, written byte by byte, pinned by a hand-framed test and used by the domain-19 chain.
5. **Which domain tags the PSG rows.** Recommendation: INSTANCE (4); a new domain 5 would collide with `machine.refuse.digest.domain-5` (api-effects.json:317) and change the composite framing arity.
6. **`diag-ctrl-psg-1` expectations before the scenario exists.** Recommendation: derivation doc in 05-01 (`docs/` or `data/` text outside `data/golden/`), `.expect` in 05-11; the orchestrator verifies commit order.
7. **Control-to-byte table fields count against `unknown-fields`.** The summary tables count `unknown-default-chosen` fields; the three input policy fields add 3, psg rule fields about 10; nothing blocks.

## Sources

### Primary (HIGH confidence, read or executed this session)
- Repository files with line cites above: `core/include/ivs/{ivs_run,ivs_machine,ivs}.h`, `core/src/machine/{machine.h,machine.c,machine_bound.c,machine_digest.c,machine_input.c,machine_run.c,machine_inspect.c}`, `core/src/memmap/{memsys.h,memsys.c}`, `core/src/cpu/{cp1610.h,cp1610.c,cp1610_decode.c}`, `core/CMakeLists.txt`, `cmake/{IvsMemmapTable,IvsMemmapProbes,IvsWarnings}.cmake`, `data/profiles/intv-2609-ntsc-family-p1.json`, `data/memmap/intv-2609-ntsc-family-p1.literal.txt`, `data/expected/**`, `data/api/api-effects.json`, `data/scenarios/*.ivss`, `data/golden/*`, `data/sources.json`, `tools/gate.sh`, `tools/gate-linux.sh`, `tools/hygiene.sh`, `tools/check/{api-effects.cmake,integrity.cmake,run-controls.sh}`, `tools/check/steps/{golden,runner-scan,c-build,c-tests,tests-inventory,glossary,text,profile,memmap}.sh`, `tools/check/kinds/profile.cmake`, `tools/check/schema/{profile,source}.txt`, `tests/{CMakeLists.txt,faults.cmake}`, `tests/run/*.c`, `tests/machine/{machine_digest.c,machine_standin.c}`, `tests/memmap/{memmap_rows.c,memmap_main.c}`, `runner/{ivs_runner.h,exec.c,report.c,main.c,CMakeLists.txt}`, `docs/time-base.md`, `.planning/{AGENTS.md via root,AUTONOMY.md,CI-EFFICIENCY.md,ci-runs.tsv,ROADMAP.md,REQUIREMENTS.md}`, Phase 4 `04-RESEARCH.md`, `04-VALIDATION.md`, `04-VERIFICATION.md`, `04-REVIEW.md`, `05-CONTEXT.md`, `05-DISCUSS-{ctrl,psg,audio,verify}.md`.
- Executed: a probe over all 1024 decles with SDBD off and on, wait 0, 1 and 35 through `ivs_cp1610_step` (max unit, write offsets, minimum write gap, writes per unit); tool presence checks; `curl -I` to the catalog host.

### Secondary (MEDIUM confidence)
- `.planning/CI-EFFICIENCY.md` hosted figures (gate_s median 59, cpu_s last-5 median 175; budget median gate_s 75, cpu_s 200).

### Tertiary (LOW confidence, tagged `[ASSUMED]` above)
- engine speed, gcc warnings for unwritten code, hosted tool versions.

## Metadata

**Confidence breakdown:**
- Code mechanisms and file:line cites: HIGH (every cite read this session).
- Derived constants (g, U): HIGH (executed over the full opcode space); capacities: HIGH (arithmetic).
- Plan decomposition: MEDIUM-HIGH (dependencies read from code; task sizes inferred).
- Pitfalls: MEDIUM-HIGH (1 to 6, 9, 11, 12 from source; 7, 8 from recorded history).
- Gate cost: MEDIUM (projection from Phase 4 profiles; checkpoints replace it).

**Research date:** 2026-11-12
**Valid until:** 2026-12-12, or until plan 05-02 edits `ivs_run.h` and `machine_bound.c` (re-read the line numbers then).
