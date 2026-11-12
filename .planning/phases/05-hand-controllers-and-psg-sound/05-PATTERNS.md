# Phase 5: Hand Controllers and PSG Sound - Pattern Map

**Mapped:** 2026-11-11
**Base commit:** 66a5dda (Phase 5 discuss context), Phase 4 landed and merged.
**Input:** `05-CONTEXT.md` (D-01..D-28), `05-DISCUSS-{ctrl,psg,audio,verify}.md`, `AGENTS.md`, `.planning/AUTONOMY.md`, `04-PATTERNS.md` (shape template) and the landed Phase 4 tree.
**Files analyzed:** about 100 new or modified files (grouped below).
**Analogs found:** about 88 exact or role-match of about 100; the rest partial or none (section "No Analog Found").

Every analog path below was checked against `git ls-files` (tracked source, no gitignored mirror, no `.gsd/capabilities` copy). Paths are repo-relative on purpose (the hygiene step rejects home and scratch paths in `.planning/` too). Line numbers are of the tracked file at the base commit.

**Reality check against 04-PATTERNS.md.** That file proposed placeholder names; these are the names that actually landed and that Phase 5 edits: core `machine_bound.c` (run ladder), `machine_inspect.c` (cpu_state, peek), `machine_input.c`, public header `core/include/ivs/ivs_run.h` (no `ivs_run.h` proposal left open), runner `runner/{scenario,exec,report,outcome,hostio,main}.c` + `runner/ivs_runner.h`, tools `tools/pack/pack.c`, suites `tests/run/*` and `tests/runner/*`, scenarios `data/scenarios/*.ivss`, goldens `data/golden/*.expect|*.digests`, steps `listing golden pkg-install important-gates runner-scan runner-selftest ci-log ci-summary`. Use these, not the Phase 4 placeholders.

## How to read this file

Conventions [S] [C] [I] [H] [T] [F] are the six of `04-PATTERNS.md` and still hold (step contract, control layout, inventory style, SPDX headers, TAP suite contract, named-fault contract). Deltas since then are folded in below. Five Phase 5 conventions are named once and referenced by letter:

- **[A] Append-only ABI growth** (A-02; the `ivs_machine_info` idiom, `core/src/machine/machine.c` L249-301). A struct starts `uint32_t struct_size`; the minimum accepted size is a PINNED v1 constant, never `sizeof` of the grown struct; the core writes exactly `min(struct_size, sizeof)` bytes and sets `struct_size` to that count; a larger in-struct needs a zero tail past the core's own size.
- **[D] Digest field row.** One table `ivs_digest_fields[]` in `machine_digest.c` covers every instance byte exactly once with `IVS_DF(name, member, width, count, domain, kind)`; the serializer never memcpy's a struct; `slot[]` stays the last member (static assert L37-42).
- **[G] Generator + independent literal.** A `cmake -P` generator reads committed data, fails closed with `message(FATAL_ERROR "<Gen>: <ctx>: <reason>")`, takes `-DROOT -DOUT` (+ optional `-DMUTATE`), writes a build-tree `.inc` that is never committed; an independence literal is written from sources only and a probe (never the profile or the generator) produces the test expectations.
- **[P] Pure reads.** CPU reads, ADAR strobes and `ivs_machine_peek` all go through `ivs_memsys_read(const ivs_memsys *, addr)`; any state a read depends on lives in `ivs_memsys` and is changed only by an internal setter.
- **[X] Cross-suite twin.** Phase 4 tests compare a run-driven instance with a seam-stepped twin (`ivs_ms_step`, `ivs_machine_step_unit_internal`) by whole-storage `memcmp`; every new instance byte must be equal in both paths.

## File Classification

### A. Core: PSG device, input byte, run and digest growth

| New/Modified File | Role | Data Flow | Closest Analog | Match | Plan |
|---|---|---|---|---|---|
| `core/src/psg/psg.h` (new internal header: device state struct, sink, API) | internal header | event-driven (lazy catch-up) | `core/src/cpu/cp1610.h` L24-140 (self-contained device header), `core/src/memmap/memsys.h` L83-96 (flat pointer-free struct) | role-match | 05-02/05 |
| `core/src/psg/psg_{regs,tone,noise,env,mix,sample,log}.c` (new, one small TU per rule family, name free) | service | event-driven + transform | `core/src/machine/machine_input.c` (small TU, statics only under a fault macro), digest sink `machine_digest.c` L110-146 (buffer-or-count sink) | role-match | 05-03..07 |
| `core/src/machine/machine_bus.c` (new, name free: machine-level bus function with tick-aware ctx) | service | request-response | `core/src/memmap/memsys.c` L280-304 (`ivs_memsys_bus`), `machine_run.c` L98-149 (port wiring) | role-match | 05-03/07 |
| `core/src/memmap/memsys.h`, `memsys.c` (PSG backing words, `port_pulldown[2]`, setter, `psg8` read branch) | service | CRUD (register file) | same files: `ivs_memsys_set_gate_internal` L362-367, `ivs_mm_load/store` L100-139, `ivs_mm_region_read` L155-179 | exact | 05-03/04 |
| `cmake/IvsMemmapTable.cmake` (device kind `psg8`, `psg` leaves the stub set, `MM_CAP_psg`) | generator | batch | same file L150-161, L262-275, L316-349, L447-465 | exact | 05-03 |
| `cmake/IvsCtrlTable.cmake` (new; `ivs_ctrl_lines.inc`) | generator | batch | `cmake/IvsMemmapTable.cmake` helpers L48-147, MUTATE L550-553; `tests/input-bits-inc.cmake` (whole file) | exact (copy) | 05-04 |
| `core/src/machine/machine_input.c` (derive pull-down, call setter) | service | event-driven | same file L44-65, L123-153 | exact | 05-04 |
| `core/src/machine/machine_bound.c` (psg_log stages, sink, catch-up, report v2, take-back re-derive) | service | request-response + loop | same file L216-257, L261-349, L390-501 | exact | 05-02/07 |
| `core/src/machine/machine_run.c` (power-on resets PSG and pull-down; step seam passes the machine ctx) | service | request-response | same file L42-65, L98-149 | exact | 05-03/07 |
| `core/src/machine/machine_digest.c` (PSG rows, audio and psglog rows, domain 19, composite v2) | transform | transform | same file L21-93, L206-253, L285-333 | exact | 05-02/05/07 |
| `core/src/machine/machine.h`, `machine.c` (layout, `IVS_MACHINE_BYTES`, obs rows, env, info v3) | internal header + service | request-response | same files: `machine.h` L12-92, L171-196; `machine.c` L249-314 | exact | 05-02 |
| `core/include/ivs/ivs_run.h`, `ivs_machine.h` (request v2, report v2, info v3, reasons 55+, domain 19, format id) | public header | request-response | same files: `ivs_run.h` L197-280, `ivs_machine.h` L60-67, L106, L128-145, L158-205 | exact | 05-02 |
| `core/CMakeLists.txt` (new TUs, second generator output) | config | build | same file L11-24, L26-46 | exact | 05-02/04 |
| `data/api/api-effects.json` (run row: psg_log precedence and refusals; info, digest notes) | data record | CRUD | same file, `ivs_machine_run` row L388-454 | exact | 05-02 |
| `tools/check/api-effects.cmake` (`DEFINE_CLASSES`: output formats, digest domains, psg log source) | checker | batch | same file L67-72 | exact | 05-02 |
| `data/expected/api-reasons.ids` (+55..57), new pin files `psg-log-source.ids` (+ optional formats/domains) | inventory | pin | `data/expected/api-reasons.ids`, `api-stop-reasons.ids` | exact | 05-02 |

### B. Profile, literals, rule inventory, records

| New/Modified File | Role | Data Flow | Closest Analog | Match | Plan |
|---|---|---|---|---|---|
| `data/profiles/intv-2609-ntsc-family-p1.json` (31 `input.control.*`, `input.line_model/idle/port_output_read`, ~20 `psg.*`, `memmap.device.psg8`, `psg_regs` retarget, `ticks_per_psg_clock` retag) | data record | CRUD | same file: `input.controllers` L508-518, `memmap.device.stub` L351-360, `memmap.region.psg_regs` L391-400, `ticks_per_psg_clock` L106-115 | exact | 05-02 |
| `data/expected/profile-fields.ids` | inventory | pin | itself (must equal the members of `fields`) | exact | 05-02 |
| `data/memmap/intv-2609-ntsc-family-p1.literal.txt` (psg rows stub -> real) | literal | batch | same file L1-33 header, L58, L69 rows | exact | 05-01/03 |
| `data/input/intv-2609-ntsc-family-p1.literal.txt` (new, 31 rows + header) | literal | batch | `data/memmap/intv-2609-ntsc-family-p1.literal.txt` L1-33 (tier, independence rule, row grammar) | exact (shape) | 05-01 |
| `data/expected/input-policies.ids` (new) | inventory | pin | `data/expected/cp1610-policies.ids` | exact | 05-01 |
| `data/psg/rules.json` (new, `PSG-R01..`) | data record | CRUD | `data/discrepancies.json` + `tools/check/schema/discrepancy.txt` + `kinds/discrepancy.cmake` | role-match | 05-01 |
| `data/psg/vectors/*.txt` (new, line-based) | literal vectors | batch | `data/sha256/vectors.txt` (id-per-line grammar, generator re-derivation), `data/memmap/*.literal.txt` header | role-match | 05-01 |
| `data/expected/psg-rules.ids`, `d10-measurements.ids`, `psg-log-source.ids` (new) | inventory | pin | `data/expected/discrepancies.ids`, `claims.ids`, `api-stop-reasons.ids` | exact | 05-01/02 |
| `data/discrepancies.json` (+ `.ids`): C69..C76 per PSG note, C77+ from one allocation table, updates to C11 C12 C14 C15 C16 C23 C63..C67 | data record | CRUD | C62 and C14/C15 entries; `tools/check/kinds/discrepancy.cmake` L1-50 | exact | 05-01 |
| `data/claims.json` (+ `.ids`): CL-0009..CL-0015 | data record | CRUD | CL-0008 (`diag-cpu-1`), CL-0006 (memmap literal) | exact | 05-09/10 |
| `data/sources.json` (+ `sources.ids`): tilton and consolemods pin pages, GI AY-3-8910 reprint | data record | CRUD | `SRC-gi-cp1600-um-1975` entry (rights `unknown`, `capture: hashed`, `local_path` under `refs/`) | exact | 05-01 |
| `docs/time-base.md` (PSG step correction), `data/expected/glossary-terms.ids` | doc + inventory | n/a | same file L61-66; glossary step `tools/check/steps/glossary.sh` L52-60 | exact | 05-01 |
| `docs/psg/reference-notes.md`, `docs/input/controller-notes.md` (new) | doc | n/a | `docs/cp1610/reference-notes.md`, `docs/cp1610/second-reader-protocol.md` | role-match | 05-01 |
| `docs/runner.md` (ivsr 2, options, capture, golden key list), `docs/diagnostics.md` (branch-target rule) | doc | n/a | same files | exact | 05-08/09 |

### C. Gate steps, checkers, controls

| New/Modified File | Role | Data Flow | Closest Analog | Match | Plan |
|---|---|---|---|---|---|
| `tools/check/steps/input.sh` + `tools/check/input.cmake` (new step `input`) | checker + step | batch | `tools/check/steps/memmap.sh` (whole), `tools/check/memmap.cmake` L1-60 | exact (copy) | 05-01 |
| `tools/check/steps/psg-vectors.sh` + `tools/check/psg-vectors.cmake` (new) | checker + step | batch | `tools/check/steps/cp1610-vectors.sh` L16-44, `tools/check/vectors.cmake` skeleton | role-match | 05-01 |
| `data/expected/gate-steps/input.ids`, `psg-vectors.ids` (new) | inventory | pin | `data/expected/gate-steps/listing.ids`, `golden.ids` | exact | 05-01 |
| `tools/check/listing.cmake` (branch-target rule) + 2 controls | checker | batch | same file L55-60 (tokens), L328-435 (per-line loop) | exact | 05-01 |
| `tools/check/kinds/discrepancy.cmake` (D10 inventory cross-check) + controls | checker | batch | same file (hook_record / hook_finish) | exact | 05-01 |
| `tools/check/steps/golden.sh`, `runner/main.c` golden writer, `docs/runner.md` (psglog domain, count keys, `.refpcm`) | step + tool | batch | `golden.sh` L3-40; `main.c` L241-270 | exact | 05-08 |
| `tools/check/steps/important-gates.sh` (REQ list adds INP-01 INP-02 API-03 AUD-01 AUD-02) + `data/expected/important-gates.ids` | step + inventory | batch | same file L14-15, L69 | exact | 05-10 |
| `tools/check/steps/runner-scan.sh` (allow the pcm header), `tools/check/core-symbols.sh` (+ deny list) | step | batch | `runner-scan.sh` L9-30; `core-symbols.sh` `--forbid-writable-data` L20-34 | role-match | 05-08 |
| `tools/check/integrity.cmake` (SPDX for `tools/pcm`), `tools/check/binary-ext.txt` (`wav`) | checker | batch | `integrity.cmake` L63-76 | exact | 05-08 |
| `tools/check/steps/pkg-install.sh`, `data/expected/pkg-install-files.ids`, `pkg-consumer.golden` | step | batch | same files (L303-340, L434-499) | exact | 05-09 |
| `tools/check/controls/**` + `data/expected/controls/{input,psg-vectors,...}.ids` + `_good/tree` toy data | control | batch | `controls/listing-*`, `controls/memmap-*`, `controls/api-input-bit-unpinned` | exact | each plan |

### D. Tests and named faults

| New/Modified File | Role | Data Flow | Closest Analog | Match | Plan |
|---|---|---|---|---|---|
| `tests/input/*.c` (new suite `input`) | test | event-driven | `tests/run/run_main.c`, `run_cases.h`, `run_input.c` | exact | 05-04 |
| `tests/psg/*.c` (new suite `psg`) | test | transform | `tests/run/*`, `tests/cpu/cpu_vectors.c` (data-driven), `tests/memmap/memmap_rows.c` (literal probe) | role-match | 05-03..07 |
| `tests/pcm/*.c` (suite for `tools/pcm`, not in D-25's count) | test | transform | `tests/ivsc/ivsc_main.c` | role-match | 05-08 |
| `tests/CMakeLists.txt`, `tests/faults.cmake` | config | build | same files L128-168, L206-225, L255-340; `faults.cmake` L1-47 | exact | each |
| `data/expected/tests/{input,psg}.ids`, `faults/{input,psg}.ids` (new); additions to `run.ids`, `machine.ids`, `memmap.ids`, `runner.ids`, `runner-process.ids`, `pack.ids`, `headers.ids` (no change) | inventory | pin | `data/expected/tests/run.ids`, `faults/run.ids` | exact | each |
| `tests/run/run_input.c` (invert `guest-invisible`), `run_fuzz.c` (v1 pins, content B/C seeds), `run_info.c`, `run_buffers.c` (D-17), `run_observe.c` L567-577, `run_cases.h` snapshot, `tests/machine/machine_standin.c` (new programs) | test | request-response | same files | exact | 05-02..05 |
| `tests/machine/machine_digest.c` (domain 19, composite v2, `MD_DOMAIN_IDS`), `tests/memmap/memmap_main.c` L83, `memmap_rows.c` L263-292 | test | transform | same files | exact | 05-02/03 |
| `tests/runner/*.c`, `tests/runner-process-tap.sh` (ivsr 2, capture, new fixture mutants, slices) | test | batch | same files | exact | 05-08/09 |

### E. Runner and `tools/pcm`

| New/Modified File | Role | Data Flow | Closest Analog | Match | Plan |
|---|---|---|---|---|---|
| `runner/scenario.c`, `ivs_runner.h` (`capture`, `assert psglog|audio`, `assert digest psglog`, new reasons) | utility | transform | same file L514-535, L550-635, L695-777 | exact | 05-08 |
| `runner/exec.c` (three-way buffer split, capture, chain check, `ex_domains`) | service | request-response | same file L45-46, L447-590 | exact | 05-08 |
| `runner/report.c` (`ivsr 2` key table) | utility | transform | same file L60-110 | exact | 05-08 |
| `runner/main.c`, `hostio.c`, `hostio.h` (operator files, `--wav-rate`) | entry + utility | file-I/O | `main.c` L272-330 (`main_write_golden`), `hostio.c` L1-20, L85-100 | role-match | 05-08 |
| `tools/pcm/ivs_refpcm.{h,c}` + `tools/pcm/CMakeLists.txt` (new) | tool | transform | `core/src/common/ivs_sha256.{h,c}` (pure integer module), `tools/pack/CMakeLists.txt` | role-match | 05-08 |
| `tools/listen-demo.sh` (new, operator helper, not a step) | script | batch | `tools/gate-linux.sh` header/Usage style | partial | 05-08 |

### F. Fixture, scenarios, goldens, consumer

| New/Modified File | Role | Data Flow | Closest Analog | Match | Plan |
|---|---|---|---|---|---|
| `fixtures/FX-diag-ctrl-psg-1/{diag-ctrl-psg-1.listing.txt, manifest.json}` | fixture | file-I/O | `fixtures/FX-diag-cpu-1/*` | exact | 05-09 |
| `data/expected/fixtures.ids`, `listings.ids`, `data/expected/tests/pack.ids`, `diag-mutations.ids` | inventory | pin | same files | exact | 05-09 |
| `tools/pack/CMakeLists.txt` + `tests/CMakeLists.txt` `ivs_images.inc` rule (third image) | config | build | `tools/pack/CMakeLists.txt` L33-47; `tests/CMakeLists.txt` L146-153 | exact | 05-09 |
| `data/scenarios/diag-ctrl-psg-*.ivss` (4) + `runner-scenarios.ids` | data | n/a | `data/scenarios/diag-input-pass.ivss`, `data/expected/runner-scenarios.ids` | exact | 05-09 |
| `data/golden/diag-ctrl-psg-1.expect`, `.digests`; regenerated `diag-cpu-1.digests`, `diag-input-pass.digests`, `timeout-budget.digests` | golden | n/a | `data/golden/diag-cpu-1.expect` L1-30, `diag-input-pass.digests` L1-8 | exact | 05-01/09 |
| `tests/installed-consumer/main.c` (+ `CMakeLists.txt` unchanged), `data/expected/pkg-consumer.golden` | test | request-response | same files: `main.c` L45-55, L264-352, L394-415 | exact | 05-09 |
| `tools/ci-record.sh` runs, `.planning/CI-EFFICIENCY.md`, `.planning/PROJECT.md` Decision Register D7 | planning doc | n/a | themselves | exact | 05-10 |

## Pattern Assignments

### A1. `core/src/psg/*` (device module: tone, noise, envelope, mixer, sample, log)

**Analog:** `core/src/machine/machine_input.c` for the TU shape; `core/src/machine/machine_digest.c` L110-146 for the optional-buffer sink; `core/src/cpu/cp1610.h` for a fixed-width state struct.

**TU shape to copy** (`machine_input.c` L20-34): the prelude first (the c-purity rule poisons `float double`), then the tracked headers; a file-scope static exists ONLY inside its named-fault block, because `c-purity --forbid-writable-data` rejects any real static:

```c
#include "common/ivs_prelude.h"
...
#ifdef IVS_FAULT_INPUT_STATIC_CARRY
static uint32_t ivs_input_carry[IVS_INPUT_CONTROLLER_WORDS];
#endif
```

Rules inherited: integer-only state held in `uint32_t` with an explicit mask after every operation (`IVS_U16(x)` is `cp1610.h` L24); tables are `static const`; no clock, file, allocation, `float` token, floating literal (c-purity, `core-symbols-allow.txt` allows only memcpy/memmove/memset/memcmp and the stack-protector names). One named fault per TU: the framework requires `IVS_FAULT_<ID>` in exactly one owner TU (`tests/faults.cmake` header L12-17, owner/module check L119-138, ids parse loop L205-235, and the `faults-verdict.sh` site scan L170-181), so split by rule family (`psg_tone.c`, `psg_noise.c`, `psg_env.c`, `psg_mix.c`, `psg_sample.c`, `psg_log.c`, `psg_regs.c`) as V-15 says; a fault then recompiles one small file. File stems must stay unique across `core/` and `runner/` (`tests/faults.cmake` L54-60 configure check).

**Sink (D-16)** copy `ivs_dsink` (`machine_digest.c` L110-146): one struct that always counts and writes only when a buffer is present, so a NULL buffer runs the same code and never changes machine state:

```c
if (k->buf != NULL && k->flushed < k->cap) {
  size_t n = k->cap - k->flushed;
  if (n > k->used) { n = k->used; }
  memcpy(k->buf + k->flushed, k->stage, n);
}
k->flushed += k->used;
```

The per-call sink is a stack struct; the instance keeps O(1) PSG state (counters, LFSR, envelope, previous levels, sample phase) in a pointer-free struct placed inside `struct ivs_machine` before `obs` (see A6).

**Differs:** nothing time-driven exists yet. `psg_catch_up(to_tick, sink)` and `psg_write(stamp, ...)` are new; catch-up must be idempotent and slicing invariant (catch up to t1 then t2 equals catch up to t2). See hazard 1 for where the end-of-unit catch-up must live.

### A2. `core/src/machine/machine_bus.c` (machine-level bus ctx) and the step seam

**Analog:** `ivs_memsys_bus` (`memsys.c` L280-304) and the port wiring of `ivs_machine_step_unit_internal` (`machine_run.c` L119-146):

```c
port.bus = ivs_memsys_bus;
port.ebci = NULL;
port.ctx = &im->mem;
...
start = im->time_ticks;
st = ivs_cp1610_step(&im->cpu, &cfg, &lines, &port, NULL, 0u, &r);
...
im->obs.input.floor_tick = start + 1u;
im->time_ticks += (uint64_t)cycles * (uint64_t)p->ticks_per_cpu_cycle;
```

The access record carries `offset_cpu_cycles` (BAR cycle relative to the unit start, after earlier waits; `cp1610.h` L44-55) and `kind` (`IVS_ACC_WRITE`=3, `IVS_ACC_DW`=7). Time advances AFTER the step (L146), so inside the bus callback `im->time_ticks` still equals the unit start: stamp = `start + offset_cpu_cycles * ticks_per_cpu_cycle + psg.write_strobe_ticks` (D-13). New ctx struct `{ivs_machine *im, uint64_t start, sink *}` passed as `port.ctx`; the function forwards everything to `ivs_memsys_bus` and intercepts WRITE and DW to `$01F0-$01FF` (source 1 and 2 of `psg-log-source.ids`). Reads stay on `ivs_memsys_read` ([P]).

**Differs:** the seam today takes no sink; both the seam and `ivs_machine_run_env_internal` must construct the ctx (the seam with a counting sink) so a seam-stepped twin and a run-driven instance stay byte-equal ([X], hazard 1).

### A3. `core/src/memmap/memsys.{h,c}` (PSG register words, `port_pulldown`, `psg8`)

**Analog:** the same files.

**Setter idiom to copy** (`memsys.c` L362-367, internal, never public; Phase 6 uses the same style for the STIC):

```c
void ivs_memsys_set_gate_internal(ivs_memsys *m, uint32_t gate, uint8_t open)
{
  if (gate != IVS_MEMSYS_GATE_NONE && gate < IVS_MEMSYS_GATES) {
    m->gate_open[gate] = (uint8_t)(open != 0u);
  }
}
```

Add `ivs_memsys_set_port_pulldown_internal(m, port, mask)` with `port < 2u`. State goes into `ivs_memsys` (`memsys.h` L83-96), which also changes the layout comment L18-19 and `machine.h` L14 (mem 113,228 bytes) and `IVS_MACHINE_BYTES` L31.

**Read path.** `ivs_mm_load/store` are switches on `phys` (L100-139; `IVS_MEMSYS_PHYS_PSG` falls in `default` today: "stub devices hold no state"). Add the PSG case; add the PSG-specific read in `ivs_mm_region_read` (L155-179): low byte of R14/R15 is `0xFF ^ port_pulldown[p]` when R7 bit (6 for port 0, 7 for port 1) is 0, the stored latch when 1; every other register reads `reg & rmask` with bits 15..8 zero; the upper byte comes from the device record fill so it stays profile data. The uniform width formula (`ivs_mm_width`, L75-78) is the right place for the fill, not a new constant.

**Differs:** the device record carries one `rmask` per device kind, but D-08 needs per-register masks (R10 reads 0). The per-register `wmask/rmask` table is data from `psg.regmap`; see hazard 5 for the generator and literal consequences.

### A4. `cmake/IvsMemmapTable.cmake` (`psg8`) and `cmake/IvsCtrlTable.cmake` (new)

**psg8 edit points** (analog: the file itself): ids are the list order (L150-151), and the output asserts them against `memsys.h`:

```cmake
set(MM_DEVICES undriven rom10 grom8 ram8 ram16 stub)
set(MM_DEVICE_MACROS UNDRIVEN ROM10 GROM8 RAM8 RAM16 STUB)
...
set(MM_STUB_PHYS stic psg)
```

Append `psg8` / `PSG8` (id 6, `IVS_MEMSYS_DEVICE_KINDS` becomes 7, `memsys.h` L40-47), add `memmap.device.psg8` to the required-field list (L281-283), drop `psg` from `MM_STUB_PHYS` and add `set(MM_CAP_psg IVS_MEMSYS_PSG_WORDS)` (L160-165 style). The device loop (L321-348) already checks `rmask` holds exactly the low `bits` bits, which `{"bits": 8, "rmask": "$00FF", "fill": "$0000", "store": "keep"}` satisfies. The stub-versus-backed rule (L447-465: "a stub device needs impl stub and a stub physical device") is what forces `psg_regs` to `impl real` together with a backed physical device. `GEN_MAP_MUTATION` (`tests/CMakeLists.txt` L286-290) mutates this generator's output and stays valid only while `-DROOT -DOUT -DMUTATE=<region>` do not change.

**IvsCtrlTable.cmake** copy list: the header (usage, "Fail closed", optional MUTATE; L1-40), the helpers `mm_type`, `mm_keys`, `mm_uint`, `mm_str` (L54-127; rename the prefix `ct_`), and the MUTATE tail (L550-553):

```cmake
if(DEFINED MUTATE AND NOT MUTATE STREQUAL "" AND NOT MM_MUTATED)
  set(MM_CTX "data/profiles")
  mm_fail("MUTATE target '${MUTATE}' is not a region")
endif()
```

Do NOT reuse `mm_hex` (L84-91): it accepts only `$XXXX`, the line masks are `$XX` (01..FF). Name set must equal the pinned `IVS_CTRL_*` set: read `data/expected/input-bits.ids` the way `tests/input-bits-inc.cmake` does (whole file, 32 lines: strict line regex, `FATAL_ERROR` on any other line, error if no name). Emit `ivs_ctrl_lines[profile][31]` (u8, index = bit number) plus `_Static_assert`s tying 31 to the pin; refuse an unknown `input.line_model`, any value outside `01..FF`, and a profile with `input.controllers > 0` and no table.

Registration: a second custom command beside `core/CMakeLists.txt` L32-46 writing `${IVS_CORE_GEN_DIR}/ivs_ctrl_lines.inc`, and make `ivs_core_gen` depend on BOTH outputs (`add_custom_target(ivs_core_gen DEPENDS ...)` L45), since `tests/faults.cmake` L64-82 waits only on `IVS_CORE_GEN_TARGET`. List the file in `data/expected/core-generated.ids` as `ivs_ctrl_lines.inc cmake/IvsCtrlTable.cmake` (c-purity reruns the generator with `-DROOT -DOUT` only). Fault: `ivs_fault_gen(GEN_CTRL_MUTATION SUITE input OUTPUT ivs_ctrl_lines.inc GENERATOR ... CONSUMER core:src/machine/machine_input.c)`, copy of `GEN_MAP_MUTATION` (`tests/CMakeLists.txt` L286-290):

```cmake
ivs_fault_gen(GEN_MAP_MUTATION SUITE memmap OUTPUT ivs_profiles.inc
  GENERATOR "${IVS_MEMMAP_GENERATOR}"
  ARGS "-DROOT=${PROJECT_SOURCE_DIR}" "-DMUTATE=sysram"
  DEPENDS ${IVS_PROFILE_FILES} "${IVS_MEMMAP_GENERATOR}"
  CONSUMER core:src/memmap/memsys.c)
```

### A5. `core/src/machine/machine_input.c` and the run-loop take-back

**Analog:** the same file. `ivs_input_apply` (L44-65) is the only place the words change; it receives only the INPUT row, so its signature must widen to `struct ivs_machine *im` (its caller `ivs_input_apply_due`, L123-153, already has `im`). After `in->words[i] = w` the machine derives `pulldown = OR over set bits b of ivs_ctrl_lines[profile][b]` and calls the memsys setter for ports 0 and 1 (D-03, D-04); words at index >= `input.controllers` are already refused nonzero (L106-110), so ports >= controllers keep pull-down 0.

**Take-back is the trap** (`machine_bound.c` L426-454): the run loop snapshots `obs.input` before applying events and restores it when the seam refuses the unit; the pull-down now derives from the words and must be re-derived from the RESTORED words, or the take-back leaves guest-visible state behind:

```c
ivs_machine_obs_input input_before = im->obs.input; /* restored if the seam refuses (IN-01) */
...
if (ivs_machine_step_unit_internal(m, &unit) != IVS_OK) {
  im->obs.input = input_before;
  consumed = consumed_before;
  internal = 1u;
  break;
}
```

Put `INPUT_PINS_NOT_RESTORED_ON_TAKEBACK` in `machine_bound.c`, `INPUT_PINS_NOT_UPDATED` in `machine_input.c`, `INPUT_PINS_NOT_RESET` in `machine_run.c` `ivs_run_power_on` (L42-65, next to the `memset(im->mem.sysram ...)` block: zero `port_pulldown` and the PSG there; the observation memset L57-64 is the pattern for the new observation rows, including the `IVS_FAULT_OBS_NOT_CLEARED_ON_RESET` style). Faults needed by the input suite: `INPUT_PINS_NOT_UPDATED`, `CTRL_PORTS_SWAPPED`, `CTRL_UNION_AS_AND`, `CTRL_NO_INVERSION`, `CTRL_DIRECTION_SENSE_FLIPPED`, `INPUT_PINS_NOT_RESTORED_ON_TAKEBACK`, `INPUT_PINS_NOT_RESET`, `GEN_CTRL_MUTATION`, plus the deferred Phase 4 `INPUT_EXTRA_CONTROLLER_UNCHECKED` (the comparison is L106-110 of `machine_input.c`; wrap it in `#ifndef` exactly like `INPUT_NO_FLOOR_CHECK` L80-84).

### A6. `core/src/machine/machine_bound.c`, `machine.h`, `machine.c`: run ladder, report v2, layout

**Analog:** the same files.

**Ladder.** psg_log joins stages 4b and 4c (`machine_bound.c` L216-257), after video and audio, with new reasons appended after 54 (`RUN_PSGLOG_NULL`, `RUN_PSGLOG_OVERLAP` BAD_ARG; `RUN_PSGLOG_SMALL` BUFFER_TOO_SMALL). Copy the pairwise overlap style (every pair tested with `ivs_bound_spans_meet`, `ivs_bound_span_wraps` for a span past the top of the address space; L143-169, L216-231) and ADD the psg_log buffer to `ivs_bound_report_ok` (L197-213) and to the event-array overlap list (L245-250), or a refusal could write into the log buffer.

**Capacity (D-14, D-17).** `ivs_bound_fits` (L100-119) is `bytes_per_frame * (max(target - now, 0) / frame + 1)`; Phase 5 adds the slack term and a third channel (log record bytes x `psg_log_records_per_frame`), and the amendment "capacity 0 with a NULL buffer is not requested". Its signature must learn the pointer. `run_buffers.c` L527 documents a Phase 4 behavior to keep or change deliberately: a non-NULL buffer with capacity 0 "is accepted (it names no byte)".

**A-02 violations to fix first.** The validator uses `sizeof` of the GROWN structs as minimums: `machine_bound.c` L72-75 (`_Static_assert(sizeof(ivs_run_report) == 112u ...)`), L184-190 (`ivs_bound_req_sized`), L199-200, L269-281, L298 (tail scan from `sizeof(ivs_run_request)`), L382 (`rep->struct_size = sizeof`), and the final `*rep = r;` (L499) writes the whole struct. Introduce `IVS_RUN_REQUEST_V1_SIZE` (64) and a report v1 constant (112), accept `struct_size >= v1`, rebase the nonzero-tail rule on the core's own `sizeof`, read the new request fields only when `struct_size` covers them, and write the report exactly like `ivs_machine_info` does (`machine.c` L257-262, L301):

```c
n = out->struct_size < (uint32_t)sizeof r ? (size_t)out->struct_size : sizeof r;
...
r.struct_size = (uint32_t)n;
...
memcpy(out, &r, n);
```

`IVS_FAULT_INFO_TAIL_UNGUARDED` (`machine.c` L266-270) is the fault analog: add `RUN_TAIL_UNGUARDED` for the report. Info v3 appends at offset 256 (`ivs_machine.h` L128-145; v1 minimum `IVS_INFO_V1_SIZE` 216 in `machine.h` L36).

**Layout.** Adding the PSG device struct and the new observation rows changes, in one plan: the layout comment (`machine.h` L12-16), `IVS_MACHINE_BYTES` (L31, 113,600 today), `sizeof(ivs_machine_obs) == 144` (L87), the "no implicit hole" assert (L88-89), the instance-size assert (L91-92), and `IVS_MACHINE_SLOTS_END` (`machine_digest.c` L37-42, requires `slot[]` last). `ivs_run_env` (L177-183) gains the audio layout (sample bytes, `psg_log` record bytes and bound); `ivs_machine_profile_env` (`machine.c` L304-314) is the one function both info and the run use, so edit it once.

### A7. `core/src/machine/machine_digest.c` (rows, domain 19, composite v2)

**Analog:** the same file. Row macro and an example of a new observation row group (L86-89):

```c
IVS_DF("obs.input.words", obs.input.words, 4u, 4u, IVS_DIGEST_INPUT, IVS_DIGEST_KIND_UINT),
IVS_DF("obs.input.count", obs.input.count, 8u, 1u, IVS_DIGEST_INPUT, IVS_DIGEST_KIND_UINT),
IVS_DF("obs.input.floor_tick", obs.input.floor_tick, 8u, 1u, IVS_DIGEST_INPUT, IVS_DIGEST_KIND_UINT),
IVS_DF("obs.input.chain", obs.input.chain, 1u, 32u, IVS_DIGEST_INPUT, IVS_DIGEST_KIND_UINT),
```

A row with `count > 1` is preceded by its `u32` count in the preimage (L246-248). Edit points: the domain-known predicate (L206-213: add 19), the table (L53-93: AUDIO gains `prev_levels` and `emitted`, new PSGLOG rows, PSG device rows in the chosen state domain), the composite (L318-329 hard-codes `uint8_t d[4][32]` and `i < 4`; version define `IVS_DIGEST_COMPOSITE_VERSION` in `machine.h` L110 goes to 2 and the composite gains the PSG state domain), and the header comment L1-15 plus the public comment (`ivs_machine.h` L268-274).

**Chains (D-18).** Domains 17 and 19 are update-time chains, not field hashes: AUDIO over level-change records indexed by change count, PSGLOG over the 16-byte records. Copy the INPUT chain mechanics (`machine_input.c` L44-65: `pre[]` built big-endian by shifts, one `ivs_sha256_digest(pre, sizeof pre, chain)` per record, count++). Fresh at create and reset: the observation rows are zero except chains that start at an `h0` (`ivs_input_init`, L36-41; its call sites `machine.c` L226 and `machine_run.c` L61-64 must also init the new chains if they have an `h0`).

**Differs:** the INPUT chain hashes the script tick; the AUDIO chain must be call-independent: the Phase 4 per-record `index || len || record` framing from per-call data must not be reused for audio (verify note, D-18).

### A8. `core/include/ivs/ivs_run.h`, `ivs_machine.h`

**Struct rule** (header comments L7-18 of `ivs_run.h`): appended fields only; one-line `IVS_API` prototypes (the checker matches `^[ \t]*IVS_API([ \t]|$)`); constants are `#define ...u`, no enums or bitfields. Edits: request v2 appends `uint8_t *psg_log; uint64_t psg_log_capacity;` (offsets 64 and 72 on LP64, sizeof 80; pointer-free prefix pinned only through `offsetof(video) == 16`, `machine_bound.c` L72); report v2 appends `audio_first_index`, `psg_log_bytes_written`, `psg_log_first_index` after `controller_now` (offset 112, sizeof 136); info v3 appends after `reserved2` (offset 256). Invert the controller-word comment (`ivs_run.h` L119-126 "Nothing in the machine reads the words in Phase 4") and the buffer comment L108-117 (capacity 0 now "not requested"). Add `IVS_DIGEST_PSGLOG 19u` (`ivs_machine.h` L60-67 block and the doc comment L268-274), `IVS_OUTPUT_FORMAT_PSG_LEVELS_V1 1u` (after L106), reasons 55..57 (after `IVS_REASON_INPUT_EXTRA_CONTROLLER 54u`, `ivs_machine.h` L227; pinned by `data/expected/api-reasons.ids`).

### A9. `data/api/api-effects.json` and `tools/check/api-effects.cmake`

**Analog:** the `ivs_machine_run` row (L388-454). The checker allows ONE refusal row per status per call (a duplicate status is `api-effects-format`), so the three new reasons join the existing `IVS_E_BAD_ARG` and `IVS_E_BUFFER_TOO_SMALL` rows (text of `outs` and `precedence[].checks`, plus new test ids in `tests[]` that exist in `data/expected/tests/*.ids` and are not all `skip-ok`); every precedence status must appear in the refusals and vice versa. The `on_ok` text carries the Phase 4 sentence "Nothing in the machine reads the words in Phase 4 ... (run.input.guest-invisible, D-15)" that must be rewritten when the test is renamed.

New pinned define classes go into `DEFINE_CLASSES` (`api-effects.cmake` L67-72); each class needs its own pin file, an orphan/unpinned control and an inventory line in `data/expected/controls/api-effects.ids`:

```cmake
set(DEFINE_CLASSES
  "IVS_STOP_|data/expected/api-stop-reasons.ids|uint|"
  "IVS_REASON_|data/expected/api-reasons.ids|uint|"
  "IVS_CTRL_|data/expected/input-bits.ids|bit|IVS_CTRL_RESERVED_MASK")
```

Candidates: `IVS_PSGLOG_SRC_` -> `data/expected/psg-log-source.ids` (D-13), `IVS_OUTPUT_FORMAT_`, `IVS_DIGEST_`. There is NO digest-domain inventory today (the audio note assumed one); D-19's "digest-domain inventory" is a new file or a new class.

### B1. Profile fields, literals, `input` step

**Profile fields.** Record shape (`tools/check/schema/profile.txt`): keys `value tag cite conflict resolving_evidence revisit_trigger decision note`; `conflict` is ONE id matching `^C[0-9]+$` and must exist in `data/discrepancies.json` (put secondary conflicts in `note`); `tag` from `data/tiers.json`; `unknown-default-chosen` requires non-blank `resolving_evidence` and `revisit_trigger`. Analog entry (`memmap.device.stub`, L351-360, and `input.controllers`, L508-518). The 31 `input.control.<name>` values are objects `{"lines": "$XX"}`, tag `tier4-community`, conflict `C16`; `input.line_model` `ground-union` tag `inference`; `input.port_output_read` `latch` tag `unknown-default-chosen` conflict `C65`. Add all new paths to `data/expected/profile-fields.ids`. A field path whose last segment holds `window offset hold duration delay period latency interval timeout` must end `_ticks` or `_cpu_cycles` (`docs/time-base.md` L74-77), so `psg.sample_period_ticks`, `psg.write_strobe_ticks`, `psg.clock_phase_ticks` are valid and a bare `...period` is not.

**Memmap literal.** Flip the two `01F0-01FF psg` rows (`data/memmap/intv-2609-ntsc-family-p1.literal.txt` L58 read view, L69 write view) from `w=0 impl=stub` to the new width and `impl=real`, in plan 05-01 as data only, per V-05; the grammar is documented in the file header L11-33 and checked by `tools/check/memmap.cmake` ("stub has w=0, real has w>0"). See hazard 5.

**Input literal** `data/input/intv-2609-ntsc-family-p1.literal.txt`: copy the memmap literal's header discipline (claim tier, independence rule "written from the sources only ... never generated from the profile", row grammar, source notes) with the row grammar of the ctrl note: `<NAME> pins=<n,n> cleared=<HH> wiki=<HH> byte=<HH> src=<SRC-id>@<locator> tag=<tag> conf=C16`. Locators use SRC ids, never local paths.

**`input` step** (analog `tools/check/steps/memmap.sh` + `memmap.cmake`, pure checker over tree files, writes nothing): copy `memmap.sh`'s wrapper (run the cmake checker, re-print its pass line once as `-- input pass <stats>`, exit 1 on any finding) and the `memmap.cmake` skeleton (required `ROOT FILELIST CHECK_DIR`, `include(lib.cmake)`, `CAP 20` finding cap, token list). The probe rules (D-02): pins mapped through the header map equal `cleared` equal `wiki`; the profile field value equals `cleared`; name set equals `data/expected/input-bits.ids`; tags and sources checked against `data/tiers.json` and `data/sources.json` (the `memmap-literal-unknown-tag/-source` rules are the copy). Cost note (V-16): use `file(STRINGS)`/`string(REGEX)` for the literal; the profile read is one `string(JSON)` pass over 31 fields.

**`input-policies.ids`** and **`psg-log-source.ids`**: plain `NAME` or `NAME VALUE` lines with a `#` header, as `data/expected/cp1610-policies.ids` and `api-stop-reasons.ids`; a policy row that is only the model's definition stays a policy row, never evidence (D-03).

### B2. `data/psg/*`, `psg-vectors` step, `d10-measurements.ids`

**Rule inventory.** `data/psg/rules.json` is a JSON record file checked by the generic `records.cmake` driver: copy the three-file idiom of the discrepancy log: `tools/check/schema/discrepancy.txt` (file, topkeys, `array`, `idkey`, `inventory data/expected/...ids`, `keys`, `req`, `match`, `when`, `hook`), `tools/check/kinds/discrepancy.cmake` (hook_record/hook_finish), and `tools/check/steps/discrepancies.sh` (12 lines: run records.cmake with `-DKIND=...`, re-print the pass line under the step's name). Schema ids `PSG-R01..`, inventory `data/expected/psg-rules.ids`; the hook checks each rule has a profile field or an engine marker, a covering vector, and a source or an `unknown-default-chosen` tag with resolver and trigger (D-07).

**Vector files.** Line grammar (id, rule, kind source|policy, cite, expected, derivation) copied from `data/sha256/vectors.txt` (id-per-line, header documents provenance, no `;` `\` `[` `]` TAB or CR, ids `[A-Za-z0-9._-]`) and from the policy discipline of `data/cp1610/vectors` (`kind: source` carries tier `source-expectation`; `kind: policy` names an id from a policy inventory). Do NOT use `string(JSON)` for vectors (V-16: the Phase 4 vector step costs ~5 cpu-s). Emulator names stay out of the text (denylist, `vector-emulator-names.txt`).

**psg-vectors step** (name `psg-vectors`): skeleton of `tools/check/vectors.cmake` (inputs guard, `CAP`, `vfail` capped tokens via `gate_incr/gate_count/gate_fail`), wrapper of `steps/cp1610-vectors.sh` L16-44. Tokens from V-18: unknown rule, rule without vector, vector without cite or tier, unknown-default parameter without a profile tag, orphan vector id, bad grammar, unlisted file.

**d10-measurements.ids** (`<id> <discrepancy> <what the owner records> <procedure anchor>`, ~14 lines): checked inside `kinds/discrepancy.cmake` (every discrepancy with a `resolver` starting `measurement:` appears in the file, every line names an existing open discrepancy; 2 controls under `data/expected/controls/discrepancies.ids`). `records.cmake` already reads one inventory per kind (`inventory data/expected/discrepancies.ids`, L70 of `records.cmake`), so the second inventory is read inside `hook_finish`.

**Discrepancy and claim shapes.** Discrepancies: strictly ascending numeric ids (next free C69), status `open|partial|resolved-primary|withdrawn`, `open`/`partial` need `default_policy` starting `unknown-default-chosen: `, `resolved-primary` needs `evidence` containing a page. C23 is NOT `resolved-primary` (no primary source): use `partial` (audio note). Claims (analog CL-0008): keys `id statement profile tier artifact depends_on standing note`, `artifact` an existing tracked file, `depends_on` ids that exist in `profile-fields.ids`, `standing: provisional` while any dependency is weak (`kinds/claim.cmake`); CL-0006 depends on `memmap.region.psg_regs` and `memmap.device.stub`, so editing those fields touches a claim.

### C1. Gate-step plumbing (steps, controls, inventories)

Everything in `04-PATTERNS.md` SP1 (step prologue/epilogue), SP2 (producer/judge with nonce), SP6 (text-only control trees) applies unchanged to `input`, `psg-vectors` and the extended checkers. New facts from the landed tree:

- Step names are the union of `data/expected/gate-steps/*.ids` (one file per owning plan): add `input.ids` and `psg-vectors.ids` (content: the step name).
- Controls: one dir per token (`CHECK`, `EXPECT`, optional `FORBID`, `MAKE`, `overlay/`) plus an entry in some `data/expected/controls/*.ids`; every `CHECK` step must PASS on the untouched `tools/check/controls/_good/tree` (positive control), so each new pure step needs toy inputs there: a toy `data/input/*.literal.txt` and the toy profile `fields` it reads (`_good/tree/data/profiles/intv-2609-ntsc-family-p1.json`, 35 lines today), toy `data/psg/rules.json`, `data/psg/vectors/toy.txt`, `data/expected/psg-rules.ids`, a toy listing with a branch, and a toy `d10-measurements.ids`. Sample control layouts: `tools/check/controls/listing-bits-mismatch/{CHECK,EXPECT,overlay/data/listings/...}` (`CHECK` = `listing`, `EXPECT` = `FAIL listing-bits-mismatch:`), `controls/api-input-bit-unpinned` (with `FORBID`). 277 control dirs exist (plus `_good`); each control copies the whole tree, so keep toy data tiny.
- Budget (D-25, V-16): at most 25 controls and 8 faults per plan; prefer an in-suite `<suite>.judge.<slug>` self-control over a fault, and a control over a fault for checker steps; `important-gates.sh` counts `<suite>.judge.*`/`.mutant.*` ids as mutants.
- The branch-target rule (V-11) goes in `tools/check/listing.cmake`: tokens list L55-60, per-line loop L328-435 (operand-count handling L412-435, `words` from `R_WORDS` L428). New token `listing-branch-target`, 2 controls in `data/expected/controls/listing.ids`, and the retroactive check of `diag-cpu-1`'s one loop (the hand derivation is in the listing header L47-51: `BNZE` first decle `10 0010 1100`, S = 1 backward, `5020 + ~0003 = 501C`; the row list needing direction coverage is `data/expected/cp1610-branch-required.ids`). The line must carry the target in its ASM text so the checker can recompute it from the printed displacement word; add the grammar sentence to `docs/diagnostics.md` L11-30.

### D1. New suites `input` and `psg`: plumbing and test shapes

**Suite registration (copy):** `tests/CMakeLists.txt` L136-168 (suite `run`, its own executable so the machine suite's per-fault reruns stay bounded) is the template, and L206-225 the `ivs_fault_suite()` registration:

```cmake
add_executable(ivs_tests_run run/run_main.c run/run_observe.c ... machine/machine_standin.c "${IVS_IMAGES_INC}" "${IVS_DIAG_EXPECT_INC}")
target_include_directories(ivs_tests_run PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/machine" "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(ivs_tests_run PRIVATE ivs_test_harness ivs_core ivs_core_internal ivs_warnings ivs_san)
add_test(NAME run COMMAND ivs_tests_run)
set_tests_properties(run PROPERTIES SKIP_RETURN_CODE 77)
```

Every test `.c` goes in BOTH `add_executable` and `ivs_fault_suite(<suite> SOURCES ...)` (L271-274 for `run`). A suite is registered three times in lockstep (ctest name = `data/expected/tests/<suite>.ids` stem = TAP header suite; `tests-inventory` fails `tests-ctest-drift` otherwise), and `faults/<suite>.ids` needs `# suite=<suite>` as its first header line.

**Case-table plumbing (copy):** `tests/run/run_cases.h` (family declarations, snapshot oracle L61-91), `tests/run/run_main.c` (concatenation; hard cap `IVS_RUN_MAX_CASES 256u` L11, `run` has 135 ids; machine suite cap 512 with 388 ids). `IVS_CHECK` and the `RI_CHECK ... goto done` macros are copied verbatim per file (`run_input.c` L43-60).

**Data-driven suites:** the committed data a suite reads must be hash-bound: the suite exports `ivs_test_source` entries and the TAP header carries `# source <path> <sha256>` lines that `tests-inventory.sh` re-hashes (L95-166). Analog: `ivs_machine_sha256_sources` (`machine_cases.h` L16-17, `machine_sha256.c` L214) and `IvsShaVectors.cmake` L11-16 (embeds the SHA-256 of the source file). `machine_main.c` caps sources at `IVS_MACHINE_MAX_SOURCES 8u`.

**Literal probes (independence):** the `input.table.profile-vs-literal` expectation comes from the INPUT literal through a generated probe include, never from the profile or `ivs_ctrl_lines`: analog `cmake/IvsMemmapProbes.cmake` (reads `data/memmap/*.literal.txt` ONLY; `tests/CMakeLists.txt` L87-100 wires it) and the 32-line `tests/input-bits-inc.cmake` for strict line grammar. The expected byte for a combination is computed in test code as `0xFF ^ OR(literal masks)` (D-03) so the code under test never supplies its own oracle.

**Guest read tests.** Program C (`tests/run/run_input.c` L1254-1285: `ri_prog_ports` = `MVI $01FE,R1 / MVO R1,$0200 / MVI $01FF,R2 / MVO R2,$0201 / INCR R3 / B`) is the existing port-polling stand-in and `ri_powered_ports` builds it. Move it (and a data-dependent variant for fuzz content C) into `tests/machine/machine_standin.c` as new `IVS_MS_PROG_*` ids so both suites share it; `ivs_ms_exec` (L33-47 area) currently knows only programs A and B (`program == IVS_MS_PROG_B ? ... : a`). The C67 granularity test needs the unit start ticks observed by single-stepping (`run(now + 1)`), not predicted from the cycle table (V-07).

**Invert, do not delete, the Phase 4 case:** `run.input.guest-invisible` (`run_input.c` L1290-1370, table row L1397, `data/expected/tests/run.ids` L160, `api-effects.json` text) becomes `run.input.guest-visible`; the three digest EQUALITIES (L1350-1354: CPU, MEM, composite) become inequalities on the guest-visible words, and L1362-1364 (whole storage outside the INPUT row equal) must be rewritten because `mem.port_pulldown` now differs.

**Slicing fuzz (D-05):** copy `tests/run/run_fuzz.c` (header L1-34: fixed xorshift32 seeds, reference = one unit per `run(now + 1)` with the unconsumed suffix resubmitted, independent expectation from `b[]` and the schedule, `run.fuzz.cases-covered` non-vacuity case). New ids `run.fuzz.input-visible-seed-01..08` and `run.fuzz.input-visible-covered`; content B (constant-cycle poller) keeps the independent oracle, content C (data-dependent) uses the relational oracle at every reference boundary (consumed count, `controller_now`, floor, CPU, MEM and INPUT digests, then final storage byte for byte). Add the invariant `port_pulldown == derive(words)` after every call.

**Struct pins:** `run.struct.v1-sizes` (`run_fuzz.c` L394-405) uses `FZ_PIN(sizeof(ivs_run_report) == 112u)` and `sizeof(ivs_info) == 256u`; both become pins on the pinned v1 constants plus `offsetof` pins for every new v2/v3 field, with the LP64-only request pin kept behind its visible skip (`FZ_LP64_SKIP`).

**Chain oracle (binary):** the audio and psglog chain literals are made at authoring time with POSIX `printf` octal escapes and `cmake -E sha256sum`, because `string(SHA256)`/`file(WRITE)` cannot carry NUL or raw hash bytes; record the recipe in the test file header, copy `run_input.c` L19-35:

```
oct() { h=$1; while [ -n "$h" ]; do b=${h%"${h#??}"}; h=${h#??}; printf '\\%03o' "0x$b"; done; }
sha() { cmake -E sha256sum "$1" | cut -c1-64; }
printf 'IVS-INPUT-LOG-V1' > h0.in; H0=$(sha h0.in)
printf "$(oct "${H0}...")" > h1.in; H1=$(sha h1.in)
```

**Twin oracle ([X]).** `tests/run/run_slice.c`, `run_diag.c`, `machine_reset.c` and `run_fuzz.c` compare run-driven and seam-stepped instances by whole-storage `memcmp`; `IVS_RUN_SNAP_DIGESTS 8u` in `run_cases.h` L66-72 (composite + domains 1..4, 16..18) must grow with every new domain, and `tests/machine/machine_digest.c` `MD_DOMAIN_IDS 19u` (L59), `md_domains` (L340-395), the sensitivity table (L664-670) and `md_refuse_domain_19` (L781-783, id `machine.refuse.digest.domain-19`, retarget to the first value after the last live domain) move with it.

### D2. Named faults for the PSG, audio and run growth

**Analog:** `data/expected/faults/run.ids` (header `# suite=run; columns: <FAULT_ID> <must-fail> <allowed-prefixes> <must-pass>`, one comment block per fault, the allowed prefixes `|`-separated and covering EVERY collateral failure). The fault list is fixed by V-17 (26 faults, owner TU and must-fail id per row) plus the audio note's list; ids are `[A-Z0-9_]+`, unique across all files, exactly one `IVS_FAULT_<ID>` site in exactly one core or runner TU. Existing Phase 4 faults `INPUT_APPLY_AT_RUN_START`, `RUN_EVENT_AT_TARGET_ONLY`, `INPUT_NO_FLOOR_CHECK`, `INPUT_LOG_SLICE_RELATIVE_TICK`, `INPUT_STATIC_CARRY` keep their must-fail ids; check `fault-collateral` for the new guest-visible ids that now also fail under them. `PEEK_*` faults live in `machine_inspect.c` (`run.ids` L1-9).

### E1. Runner: parser, executor, report, options

**IVSS (`runner/scenario.c`, `runner/ivs_runner.h`).** Analog: the existing directive functions. `parse_digest_domain` (L514-535) is a plain `tok_is` ladder: add `psglog` -> `IVS_DIGEST_PSGLOG`. `dir_assert` (L550-635) is the template for `assert psglog ...` / `assert audio ...` subjects (new `IVS_SCN_SUBJ_*` after `IVS_SCN_SUBJ_DIGEST 5u`, `ivs_runner.h` L119-123); `parse_directive` (L695-777) dispatches (`assert`, `input`, ...): add `capture psglog|audio` as a header-or-anywhere directive per D-20 (cap 4096 entries, overflow is a harness failure, no directive names a file or a rate). New refusal reasons are appended after `IVS_SCN_R_NO_RUN_TO 44u` and `IVS_SCN_R_COUNT` moves (L127-178), each with its report token in `ivs_scenario_reason_name` (scenario.c L930) and a refuse case in `tests/runner/runner_parse.c` (`runner.parse.reasons-named` fails a reason without a case). Hand-written ASCII scanning only (no `strtol`, `sscanf`, `isdigit`); caps are constants in the header (L82-91); the parser stays a pure function of bytes with no I/O.

**Executor (`runner/exec.c`).** The slice loop builds one request per slice and today splits `host->buffers` into two halves only when a layout is nonzero (L500-516):

```c
if (info->video_bytes_per_frame != 0u || info->audio_bytes_per_frame != 0u) {
  size_t half = x->host->buffer_bytes > IVS_RUNNER_BUFFERS_MAX ? IVS_RUNNER_BUFFERS_MAX / 2u : x->host->buffer_bytes / 2u;
  if (x->host->buffers != NULL && half > 0u) {
    q.video = x->host->buffers; q.video_capacity = half;
    q.audio = x->host->buffers + half; q.audio_capacity = half;
  }
}
```

Phase 5 makes `audio_bytes_per_frame` nonzero for the REAL profile, so (INF, from this loop) every Phase 4 scenario now delivers audio and fills `output.audio.bytes`, which is one reason those goldens change. Replace the halving with a proportional three-way split (video, audio, psg_log; audio about 1.1 MiB and log about 2.3 MiB at a 2^22-tick slice per the audio note, inside `IVS_RUNNER_BUFFERS_MAX` 8 MiB, `ivs_runner.h` L283-284), accumulate `rep.audio_bytes_written` / log bytes (L535-536), and recompute the audio chain over the delivered bytes for `audio.chain.check`. Digest rows: `ex_domains[8]` (L45-46), `ex_final` loop `i < 8u` (L583), `ivs_runner_record.digest[8][32]` (`ivs_runner.h` L413): add `psglog` as the ninth. Capture arrays (4096 records x 16 B, 4096 changes x 12 B) must live in the host-provided storage or the record: the library allocates nothing and has no globals (L259-276).

**Report (`runner/report.c`).** Closed key table, fixed order; edit L60-110 (head table `{"ivsr", RF_IVSR}`) and the tail table: new keys after `output.audio.bytes` per D-20 (`output.audio.samples`, `output.audio.changes`, `output.psglog.records`, `output.psglog.bytes`, `digest.psglog` after `digest.input`, `audio.sample_ticks`, `audio.chain.check`, `refpcm.*` only with `--audio-wav`). `ivsr 1` literals to change together (grep result): `report.c` L4 and key table, `tests/runner/runner_exec.c` L251-258 and `ex_key_order` L871-880 (the independent copy of the key table), `runner/ivs_runner.h` L423, `docs/runner.md` L77, L83, L154, `tests/runner-process-tap.sh` L10 and L171 (`[ "$_first" = "ivsr 1" ]`), and five control files (`tools/check/controls/_good/tree/build/gate/{dev,release}/runner/toy*.report`, `golden-dev-release-diff/overlay/.../toy.report`).

**Golden writer.** The golden key list exists in three places that must agree: `tools/check/steps/golden.sh` (header L3-40 and its awk), `runner/main.c` `main_golden_keep` (L241-270, with `live[3]` and `live_count[3]` for video, audio, input) and `docs/runner.md` L160-172. Add the psglog live domain to all three; the `# domains:` header line is a fixed key list (golden.sh: `cpu=observed mem=observed images=observed instance=observed video=<v> audio=<v> input=<v>`) copied verbatim by five control files (`grep -l "^# domains:" tools/check/controls`); `.refpcm` is NOT an accepted golden extension (golden.sh: any file in `data/golden/` that is not `.expect` or `.digests` is `golden-mismatch`).

**Operator files (`--audio-raw`, `--psg-log-out`, `--audio-wav`, `--wav-rate`, `--audio-out-max-bytes`).** Option parsing analog: `runner/main.c` (usage text near L61-67, "every option once, numbers decimal digits only, a usage error is exit 14"). Writing analog: `main_write_golden` (`main.c` L274-330: `fopen(path, "w")`, one stderr line and exit 14 on a write error, golden refused for host-dependent outcomes). D-20 says the new files are exclusive-create and written only in `hostio.c`; `hostio.c` currently documents "never writes a file" (L12) and is the only POSIX file (`runner-scan`), so flip that comment and expose a write call through `hostio.h`; give the library a vtable hook next to `write_report` (`ivs_runner_host`, `ivs_runner.h` L307-318) so the in-process suite can fake it (no file system in `tests/runner`). Exclusive create is also expressible in ISO C11 (`fopen(path, "wx")`) without POSIX.

### E2. `tools/pcm/ivs_refpcm.{h,c}` (D7 reference converter)

**Analog (module shape):** `core/src/common/ivs_sha256.{h,c}` (pure integer module, no I/O, no allocation) and the build wiring of `tools/pack/CMakeLists.txt` (explicit source list `IVS_PACK_SOURCES`, a repo-relative list written to `build/<preset>/ivs-pack-sources.txt` for `c-build`, internal static target, no install rule). Rules from D-21: no I/O, allocation, floats or globals; caller-owned DC-block state; exact rational box filter in `uint64_t`; floor division through a helper (no right shift of a negative value); signed 16-bit mono. Linked ONLY into `ivs_run` (for `--audio-wav`) and its own test executable.

**Not in the fault framework yet.** `PCM_FLOOR_ROUNDING`, `PCM_DC_STATE_CARRY`, `PCM_RATE_DRIFT` need a third owner module; see hazard 4. `tools/pcm/*.c|h` are not matched by the SPDX regex (`integrity.cmake` L74-76 covers `tools/(pack|cart)/`): add `pcm` with a control, or the header goes unenforced. The "symbol deny list" of D-21 does not exist in the form assumed: `tools/check/core-symbols-deny.txt` only LABELS forbidden imports (categories `clock fs env thread io rand alloc`) and the allowlist decides; to prove the core archive defines no `refpcm|resample|wav|pcm` symbol, add a rule next to `--forbid-writable-data` (`core-symbols.sh` L20-34: lists defined symbols with `nm -P` and fails by type), token e.g. `core-symbols-pcm-defined`, with a control; `data/expected/pkg-install-files.ids` already proves the install prefix has no such file.

### F1. Fixture `FX-diag-ctrl-psg-1`, scenarios, goldens, mutants

**Fixture.** Analog `fixtures/FX-diag-cpu-1/{diag-cpu-1.listing.txt, manifest.json}`. Listing header keys (`id kind profile window label entry stack`, L1-7 of the analog) and the comment block that derives every encoding from cited manual pages; line grammar `ADDR HEX4 BITS ROW ASM` (the BNZE and displacement lines are L82-83, their hand derivation L47-51). About 100 words at `5000`, no SDBD (the checker fails closed on SDBD), every operand address below `0400` so each is one decle (V-06). Manifest: copy `FX-diag-cpu-1/manifest.json` (all keys required, `kind original-source`, `committed true`, `path` listed in `outputs` with current `sha256` and `size_bytes`, `redistributable yes`, `allowed_use [local-test, ci, repo]`, `rights_review` author-attested); refresh outputs in the same commit as every listing edit (`cmake -E sha256sum` + `wc -c`, helper idiom `tools/cart/manifest-outputs.sh`). Register: `data/expected/fixtures.ids` (exact set), `data/expected/listings.ids` (`<id> cart <path>`), `data/expected/tests/pack.ids` (`pack.words-<id>`, `pack.trailer-<id>`), and the build: a third image in `tools/pack/CMakeLists.txt` (L33-47 pattern: `add_custom_command` running `ivs_pack --ivsc <listing> <out>`, `ivs_images` target) and the `ivs_pack --c-include` list in `tests/CMakeLists.txt` L146-153 (`diag_cpu_1=${IVS_DIAG_CPU_1_IVSC}` pairs). Images are build-tree only (`.gitignore` ignores `*.ivsc`; `.ivsc` is in `binary-ext.txt`).

**Scenarios.** Analog `data/scenarios/diag-input-pass.ivss` (comment block stating outcome and exit, `ivss 1`, `id`, `profile`, `content exec boot-stub`, `content grom zero-grom`, `content cart <file>.ivsc`, `input at_tick=...`, `run-to ... ticks`, `assert ...`), and the inventory `data/expected/runner-scenarios.ids` (six columns `<id> <exit> <outcome> <class> <skip> <operator-args>`). Scenario ids from V-08: `diag-ctrl-psg-pass`, `diag-ctrl-psg-nokey` (timeout budget, exit 11), `diag-ctrl-psg-order`, `psg-log-assert-mismatch` (guest-fail, exit 10). `# expect-detail:` comment lines pin `outcome.detail` (judge in `runner-process-tap.sh` L9-14). No asserts name file paths or rates; port reads are observed by `assert mem 01FE == 00FB` (peek reads what the CPU read returns).

**Goldens.** `.expect`: copy the header of `data/golden/diag-cpu-1.expect` L1-30 (tier `original-diagnostic`, oracle `independent-derivation`, `# scenario:`, `# profile:`, `# program:`, a derivation block per assert, an "Excluded on purpose" paragraph, the shared-misreading caveat). The assert lines must equal the scenario's assert lines in order (`golden-expect-drift`); the derivation is committed BEFORE the program first runs (V-01). `.digests`: header of `diag-input-pass.digests` L1-8 (`# tier: deterministic-repeatability`, `# oracle: self-recorded`, `# domains:`), body written only by `ivs-run --update-golden`, never by the gate. Phase 4 goldens to regenerate: `diag-cpu-1.digests`, `diag-input-pass.digests`, `timeout-budget.digests`, `data/expected/pkg-consumer.golden` digest lines.

**Mutants.** `data/expected/diag-mutations.ids` is `<ID> <addr> <word> <clause>` for ONE diagnostic: `tests/runner-process-tap.sh` L311-337 hard-wires `fixtures/FX-diag-cpu-1/diag-cpu-1.listing.txt`, `ivs_pack --ivsc ... --patch`, and `diag-boot-pass.ivss`:

```sh
muts="$root/data/expected/diag-mutations.ids"
while read -r mid addr word clause; do
  ...
  elif ! "$pack" --ivsc "$listing" "$md/diag-cpu-1.ivsc" --patch "$addr=$word" > "$tmp/why.pack" 2>&1; then
  ...
    run_one "$tmp" "mut-$mid" "$ivs_run" --content-dir "$md" "$scen/diag-boot-pass.ivss"
    judge "mut-$mid" 10 guest-fail "assert=* line=* $clause got *" "" || ok=0
```

Add two columns (listing id, scenario id) or a second inventory; the content dir copy (L84-96) and the slices loop (L341: `for id in diag-boot-pass diag-input-pass`) also name the first diagnostic. The negative mutant (delay count changed, predicate must still pass) needs a third outcome in that loop. Test ids `runner-process.mutant.<ID>` are exact inventory lines in `data/expected/tests/runner-process.ids`.

### F2. Installed consumer and `pkg-install`

**Analog:** `tests/installed-consumer/main.c`. The consumer replays one scenario through the PUBLIC header only and prints a pinned text. Edits: (1) `v1_sizes()` (L394-415) asserts `sizeof(ivs_run_report) == 112u`, `sizeof(ivs_info) == 256u`, `sizeof(ivs_run_request) == 64u` against the grown installed headers and prints them; rewrite against the pinned v1 constants and print v1 and v2/v3 sizes (the printed line is pinned by `data/expected/pkg-consumer.golden` L10); (2) the domain tables `c_domain[]`, `c_domain_name[]` and `C_DOMAINS` (L45-55) add `psglog`; (3) `session()` (L264-352) gains an audio scenario (the three buffers through `ivs_run_request` v2) printing `digest.audio` and `digest.psglog` and the peeked `$01FE` after the first event (expect `0x007E` for KEY_1); (4) usage takes a third image argument. `tests/installed-consumer/CMakeLists.txt` needs no change (`find_package(ivs 0.1 CONFIG REQUIRED)`, strict flags). Keep the C rule in its header: "one call per statement when one argument reads state another argument's call changes".

**`pkg-install.sh`:** `check_runner_golden` (L434-458) compares the consumer's `digest.<domain>` lines and observed-domain set with `data/golden/diag-input-pass.digests` (hard-coded path `rgold`, L437); add a second comparison for the audio scenario's golden; `img_exec`/`img_cart` (L489-494) and the `pkg-consumer-images` token take the third image. `data/expected/pkg-consumer-calls.ids` stays unchanged (D-19: no new public function). The step cannot be control-driven (needs a build): new cases go to the in-step self-tests (`pkg-selftests.ids`), registered in `important-gates.ids` as `mutant`.

### G1. Records, docs and text gates

`docs/time-base.md` L61-66: the glossary entry "PSG step: 16 PSG chip clocks ... 32 master ticks" is wrong by D-09 (8 PSG chip clocks, 16 master ticks, 4 CPU cycles on NTSC); the correction lands before any PSG code and updates `data/expected/glossary-terms.ids` if the term changes. Text gates read every `README.md` and `docs/**/*.md`: a line with `supports|compatible|passes|accurate|verified|runs` needs `[claim:CL-NNNN]` or `[nc]` (`steps/text.sh` L36-40); a bare `clock` or `clocks` needs `input clock`, `STIC clock`, `PSG chip clock` or `[nc]` (`steps/glossary.sh` L52-60, so "PSG clock" alone fails, "monotonic clock" and "wall clock" fail); `jzintv` needs `[cite-only]` and `Zbiciak` is never allowed (`steps/denylist.sh`). `.planning/` is exempt from `text` and `denylist` but not from `hygiene`.

## Shared Patterns

### SP1. Fault registry (owner TU, inventory line, verdict)
**Source:** `tests/faults.cmake` header L1-33, L119-138, L205-235; `data/expected/faults/run.ids`; `tools/check/steps/faults-verdict.sh` L170-181.
One `IVS_FAULT_<ID>` site in exactly one TU of the suite's modules; `ivs_fault_<ID>` = the suite's test sources + plain copies of every other TU + that TU compiled with `-DIVS_FAULT_<ID>`; the id is in exactly one line of exactly one `data/expected/faults/*.ids` (`# suite=<name>` header, registered by `ivs_fault_suite` in `tests/CMakeLists.txt`); the faulted TAP must list exactly the suite's ids, fail the must-fail id, fail only ids with an allowed prefix, pass the must-pass id. Generator faults are `GEN_*` ids registered by `ivs_fault_gen` (no source site). A fault build runs the WHOLE suite once, so a fault in a 388-id suite costs more than in a small suite (the reason for suites `input` and `psg`, V-15).

### SP2. Exact inventories
**Source:** `data/expected/**.ids`, `tools/check/steps/tests-inventory.sh`, `tools/check/steps/important-gates.sh`.
`#` comment lines, first whitespace token = id, extra columns step-specific (`skip-ok`, fault columns, `NAME VALUE` pins). Inventories are exact sets, never subsets. A skip is visible (`ok N <id> # SKIP <reason>`), counted, and legal only for a `skip-ok` id (SP9 of Phase 4 still applies).

### SP3. Controls
**Source:** `tools/check/run-controls.sh` L130-200, `tools/check/controls/listing-bits-mismatch/`.
A control is a negative test of a CHECKER step: `CHECK` (step name), `EXPECT` (token lines matched as fixed strings, none blank), optional `FORBID`, `overlay/` copied over a fresh copy of `_good/tree`, optional `MAKE` verbs; the control runner executes `steps/<name>.sh` with `ROOT=<tree> FILELIST IVS_GATE_DIR=<tree>/build/gate`. Steps that need a build (`pkg-install`, `c-faults`) cannot be control-driven.

### SP4. TAP harness
**Source:** `tests/harness/ivs_test.h` L1-70, `tests/headers-tap.sh` L72-85.
`# suite=<s> nonce=<IVS_TEST_NONCE|none>`, optional `# source <path> <sha256>`, `1..N`, `ok N <id>` / `not ok N <id>` / `ok N <id> # SKIP <reason>`, trailer `# suite=<s> executed=<n> skipped=<k> failed=<f>`; exit 0 pass, 1 fail, 77 all skipped; every `add_test` has `SKIP_RETURN_CODE 77`; `ivs_test_diag` prints `#` lines to stdout only.

### SP5. Generator + independent literal probe
**Source:** `cmake/IvsMemmapTable.cmake` (generator), `cmake/IvsMemmapProbes.cmake` (probe from literal only), `cmake/IvsShaVectors.cmake` (generator re-derives every literal with a second implementation and stops the build on mismatch), `tests/input-bits-inc.cmake` (strict grammar, 32 lines).
Generated files are build output, never committed, listed in `data/expected/core-generated.ids` when a core TU includes them; the literal file header states tier, independence rule and recipe, and is NEVER derived from the profile.

### SP6. Listing checker
**Source:** `tools/check/listing.cmake` L1-60, `tools/check/steps/listing.sh`, `data/expected/listings.ids`, `docs/diagnostics.md` L11-30.
Grammar `ADDR HEX4 BITS ROW ASM`; bits value equals HEX4, ROW is a leaf row of `data/cp1610/opcode-families.json`, operand lines follow instructions of `words > 1`, contiguous addresses inside the window; `tools/pack/pack.c` is the only decoder to image bytes and `tests/pack-tap.sh` compares `ivs_pack --dump-words` with the checker's `-DDUMP=<id>` output byte for byte (two independent parsers).

### SP7. api-effects rows
**Source:** `data/api/api-effects.json` L388-454, `tools/check/api-effects.cmake` L56-80.
A new field or reason is a data edit plus test ids; one refusal row per status; every precedence status has a refusal and vice versa; every cited test id is a first token of some `data/expected/tests/*.ids` and not all `skip-ok`; refusal effects come from the `effects` registry (`none` = whole-storage memcmp equal, digest equal where the handle is valid, only listed out parameters written).

### SP8. Digest field tables
**Source:** `core/src/machine/machine_digest.c` L21-93, `tests/machine/machine_digest.c` (coverage and sensitivity families), `machine.h` L94-120.
Every new instance member gets rows in the same plan as the member; `machine.digest.coverage` and the sensitivity sweeps iterate `ivs_machine_digest_field_count()`; a flipped row in an observation domain (16..) must change ONLY that domain and not the composite (the inverse assertion in the sensitivity helper); `IVS_DIGEST_KIND_PAD` rows are declared but never serialized.

### SP9. Fail-closed generators and parsers
**Source:** `IvsMemmapTable.cmake` header L24-30, `core/src/content/ivsc.c` L60-140 (bounds-proven reads, first failing stage wins), `runner/scenario.c`.
An unknown key, a missing required field, a value outside its range and a count mismatch are errors with a context string; a pure function of its input; hand-written ASCII classification (no locale-dependent `<ctype.h>`), integer parsing without `strtol`/`sscanf`.

### SP10. Append-only ABI growth and refusal oracle
**Source:** [A] above; `tests/machine/machine_refuse.c` L1-60 (row matrix), `tests/run/run_cases.h` L61-91 (`ivs_run_snap_take/same`).
A refusal changes nothing: whole-storage memcmp equal, digests and machine time equal, poisoned (`0xA5`) out structs untouched except listed detail fields. Apply to the new request/report/info growth, `RUN_PSGLOG_*`, and any peek/cpu_state change.

### SP11. No host dependence in the core
**Source:** `core/src/common/ivs_prelude.h` (stdint/stddef/limits/stdbool/string only; `#pragma GCC poison float double`), `tools/check/steps/c-purity.sh` header L1-60.
No clock, file, env, allocation, float, or writable static in the PSG, input or digest code. Time exists only as `im->time_ticks`. Hosts, tools and the reference converter own every rate, filter and DAC table.

## No Analog Found

| File | Role | Data Flow | Reason / fallback |
|---|---|---|---|
| `core/src/psg/*` catch-up scheduler | service | event-driven | First time-driven device. Use the algebra in `05-DISCUSS-psg.md` P-4/P-5/P-13 and `05-DISCUSS-audio.md` AU-04; the sink and TU shape come from A1. |
| `tools/pcm/ivs_refpcm.c` (box filter, DC block) | tool | transform | Nothing resamples today. Algorithm shape fixed in D-21 / AU-12; module shape from `ivs_sha256.c`. |
| WAV and raw/log operator writers in `hostio.c` | utility | file-I/O | Only precedent for a runner file write is `main_write_golden` (stdio `fopen "w"`); exclusive create is new. |
| Per-register read masks inside a uniform device record | service | CRUD | Device records have one `rmask` (`memsys.c` L35-40). Use a `psg.regmap`-derived const table; see hazard 5. |
| `tools/listen-demo.sh` | script | batch | No operator listening helper exists; style only from `tools/gate-linux.sh` (Usage/Env/Exit header, `LC_ALL=C`, `exit 2` on a missing tool). It needs SPDX (integrity regex `^tools/.*[.]sh`) and must refuse `content ... user` scenarios (audio note AU-13). |
| Defined-symbol deny rule (`refpcm|resample|wav|pcm`) | checker | batch | Existing deny list labels imports only; add a rule beside `--forbid-writable-data`. |
| Digest-domain pin inventory | inventory | pin | Does not exist (audio note assumed it); add a `DEFINE_CLASSES` entry and a pin file, or drop the claim from the D-19 plan text. |
| Branch-target recomputation | checker | batch | Not in `listing.cmake`; formula in the `diag-cpu-1` listing header L47-51 and the CP-1600 manual branch table (pdf 54 per that header). |

## Hazards (ranked; each is a thing the planner can get wrong)

1. **Twin oracle and where the end-of-unit PSG catch-up lives.** D-16 says "at the end of every run call the PSG is caught up to `now`". But run.slice, run.fuzz, run.diag and machine_reset compare a run-driven instance with a seam-stepped twin by whole-storage `memcmp` (`run_cases.h` L61-91, `run_fuzz.c` header L25-33). If catch-up exists only at run exit, a seam-stepped instance never catches up and every such comparison fails on PSG state. Put the catch-up (to the unit end) in the step seam, or give the seam a sink and the same post-unit call; because catch-up is idempotent the result is identical for any slicing. Decide this in plan 05-02/05 before writing any PSG test.
2. **ABI size pins and A-02 minimums.** `machine_bound.c` uses `sizeof(ivs_run_request)` / `sizeof(ivs_run_report)` as MINIMUMS (L72-75, L184-190, L199-200, L269-281, L298, L382) and `*rep = r` (L499) writes the whole struct. Growing the structs without pinned v1 constants makes every caller built against the v1 header (a v1-sized request or report, which A-02 promises keeps working) BAD_ARG, and the whole-struct write overruns a v1-sized report; the runner and consumer rebuilt from the new header pass the larger size and would NOT notice, so only a dedicated v1-sized-caller test (extend `run.struct.*`, as `run.info.v1-size-caller` does for `ivs_info`) catches it. Also: `run.struct.v1-sizes` pins (`run_fuzz.c` L394-405), `ivs_machine_info` `IVS_INFO_V1_SIZE 216`/`sizeof 256`, consumer `v1_sizes()` and the pinned `pkg-consumer.golden` L10, LP64-only pointer pins with a visible skip elsewhere.
3. **Digest literal churn and the D-04 versus D-18 conflict.** D-04 puts the register file and `port_pulldown` in `ivs_memsys` "covered by the digest (MEM rows)", but D-18 and V-19 say only composite, instance and audio lines may change in the goldens. MEM-domain rows change `digest.mem` in EVERY golden (`diag-cpu-1`, `diag-input-pass`, `timeout-budget`, `pkg-consumer.golden`). Resolve explicitly (put the PSG state rows in a new state domain included in composite v2, or widen the allowed-diff set to include `digest.mem`) and record the classification. Also: `machine.digest.fresh-composite` (`machine_digest.c` test L254-269) and `machine.digest.stepped-literals` are the Phase 3 literal pins already in place; they change with the table. `machine.refuse.digest.domain-19` (L781) retargets to 20; if a state domain id 5..15 is chosen, `machine.refuse.digest.domain-5` (machine.ids L127) flips; `MD_DOMAIN_IDS` (L59), the composite array `d[4][32]` (`machine_digest.c` L289, L318), `IVS_RUN_SNAP_DIGESTS`, the runner `digest[8][32]`, consumer `C_DOMAINS` and the `# domains:` golden line all hard-code the domain list.
4. **The fault framework has two modules, `tools/pcm` would be a third.** `tests/faults.cmake` L36-47 lists `core runner`; `ivs_fault_suite(MODULES ...)` accepts only `core` or `core;runner` (`tests/CMakeLists.txt` L206-216); `faults-verdict.sh` L170 scans `^(core/src|runner)/`; `c-build.sh` L41-43 knows three lists (core, runner, tools/pack); `runner-scan.sh` forbids any include whose operand contains `..` or is not a header under `runner/` (a quoted `ivs_refpcm.h` is `runner-scan-header`). Either put the refpcm TUs in `runner/` (and `IVS_RUNNER_SOURCES`) or extend all five places with controls. PCM faults and tests are also outside D-25's 26-fault/365-id budget; say so in the plan.
5. **Uniform-width memmap machinery versus per-register PSG masks.** The literal grammar has one `w=` per row; the memmap probe merges adjacent equal tuples (`memmap_rows.c` header L24-27), its rule table has `{"psg", 0u, 0x0000u, 0xFFFFu, 0u}` (L280), `memmap_main.c` L83 declares `psg-regs` as a STUB row, `run_observe.c` L567-577 expects peek of `$01F0` to read the undriven fill, and the generator refuses a backed device on a "stub physical device" (`IvsMemmapTable.cmake` L447-465). Flipping the rows needs: the generator change (A4), a literal representation for widths 8/8/8/8/4/4/4/8/8/5/0/6/6/6/8/8 (or `w=8` plus a documented exception in `memmap.cmake` and the probe), the memmap rule table, the three tests above, CL-0006 (depends on `memmap.region.psg_regs`, `memmap.device.stub`), and the dev/release `memmap-dump.txt` compare. Decide in plan 05-01 (V-05) before 05-03.
6. **Gate-only hard-codes that must move together** (each is a silent breakage if missed): `important-gates.sh` L14-15 and L69 hard-code the Phase 4 REQ list (add INP-01 INP-02 API-03 AUD-01 AUD-02 and cover each, with controls); `golden.sh`/`main.c`/`docs/runner.md` golden key lists and the fixed `# domains:` line (5 control files); `ivsr 1` literals (10 places, A-E1); `runner-process-tap.sh` first-diagnostic hard-wiring (L84-96, L311-337, L341); `runner.parse.reasons-named`; `ex_key_order` in `runner_exec.c`; `IVS_SCN_R_COUNT`.
7. **gcc/hosted-only failure classes** (two Phase 3 and one Phase 4 escapes were Linux-only): argument evaluation order in test helpers and table loops (`rd(next(), next())` banned; the consumer header states the rule), GNU `nm` shapes, `-Wconversion -Wsign-conversion -Wshadow -Wcast-qual` under `-Werror` in both presets (all PSG arithmetic: 16-bit and 17-bit LFSR values promoted to `int`), `static const` only in the core, `%llu` casts for `uint64_t` in tests and runner. Run `sh tools/gate-linux.sh` before pushing plans 05-02, 05-05, 05-07, 05-08, 05-09, 05-10 (V-25). Use `uint32_t` internal state with explicit masks and `IVS_U16`.
8. **Pure reads must stay pure.** CPU reads, ADAR strobes and `ivs_machine_peek` share `ivs_memsys_read(const ...)`. The input read depends on `port_pulldown` and R7, both in `ivs_memsys`. A cached byte anywhere else desyncs peek from the guest (D-04, ctrl note P1). The invariant `port_pulldown == derive(words)` after every call and after the IN-01 take-back is a test, not a comment.
9. **Input take-back and reset.** `machine_bound.c` L426-454 restores `obs.input` on a refused unit; the pull-down must be re-derived from the restored words; `ivs_run_power_on` (`machine_run.c` L42-65) must zero the pull-down and the PSG, and the OBS memset at L57-64 does not reach `mem`. One fault per path (A5).
10. **Chain framing and the binary oracle.** The AUDIO chain is over LEVEL-CHANGE records indexed by change count and binds `prev_levels` and `emitted`; the Phase 4 per-call `index || len || record` framing breaks slicing invariance if reused (D-18). `string(SHA256)` and `file(WRITE)` cannot carry NUL: authoring-time literals via `printf` octal and `cmake -E sha256sum` (`run_input.c` L19-35). A slicing-invariance test of both chains is mandatory and the runner's `audio.chain.check` recomputes from delivered bytes.
11. **D-17 changes Phase 4 behavior.** Capacity 0 with a NULL buffer becomes "not requested" (no check). Tests that currently expect BUFFER_TOO_SMALL for capacity 0 against a nonzero synthetic layout flip: `run.prec.capacity-over-event-order` (`run_input.c` L917-935) and any `run_buffers.c` case that relies on capacity 0 (grep `NULL, 0u`); `ivs_bound_fits` (L100-119) needs the pointer. `ivs_run.h` L108-117 and the api-effects text change.
12. **Symbol-deny semantics (D-21).** The existing deny list only labels forbidden IMPORTS; nothing checks defined names. Without a new rule `pkg-install-files.ids` is the only proof that no PCM code ships. Add the rule with a control (E2).
13. **Hygiene.** `.planning/` is not exempt. Never write absolute home-directory paths, scratch or temp-directory paths, tilde paths, email addresses or credential-shaped assignments (use repo-relative paths and `<repo-root>`). `integrity` enforces ASCII-only and no CR or empty file under `data|fixtures|tools|core|include|tests|cmake|runner`; SPDX only for the regexes in `integrity.cmake` L74-76 (`runner/` yes, `tools/pack|cart` yes, `tools/pcm` no); add the header anyway. Add `wav` (and any raw extension a helper writes) to `tools/check/binary-ext.txt` so a committed render is refused. Datasheet PDFs, wiki images and fetched pages stay in the gitignored cache (`refs/`); `data/sources.json` entries carry `rights: unknown`, `capture: hashed`.
14. **Docs trip three text gates** (G1): claim words need `[claim:CL-NNNN]` or `[nc]`, a bare word `clock` needs a qualifier (PSG docs will write "PSG clock" and "monotonic clock"), emulator project names need `[cite-only]` or are banned. The D7 and AUD docs are the first likely offenders.
15. **Profile and discrepancy coupling.** One `conflict` id per field; the id must already exist in `data/discrepancies.json` (so allocate C69.. in plan 05-01, one allocation table, C77+ for the verification note's extras); `data/expected/discrepancies.ids` and `profile-fields.ids` are exact sets; ids are strictly ascending; `ticks_per_psg_clock` retag moves from `unknown-default-chosen` to `inference` (its `resolving_evidence` and `revisit_trigger` become optional); `data/discrepancies.json` C14, C15, C16 keep `open`/`partial` with the `unknown-default-chosen: ` prefix.
16. **Suite plumbing is triplicated and capped.** ctest name, `data/expected/tests/<suite>.ids`, TAP header suite; every test `.c` in both `add_executable` and `ivs_fault_suite`; `IVS_RUN_MAX_CASES 256` (135 ids used), `IVS_MACHINE_MAX_CASES 512` (388 ids used; composite v2 adds ids); `IVS_MACHINE_MAX_SOURCES 8`; `tests-inventory` fails `tests-ctest-drift` on a mismatch; new suites `input`, `psg` (and `pcm`) each need a `faults/<suite>.ids` with a `# suite=` header registered by `ivs_fault_suite`.
17. **Cost.** Last-5 cpu median is 175 against a trigger of 200 (CI-EFFICIENCY, D-25): each fault is ~0.15-0.25 cpu-s, each control ~0.04, each plain-run id ~0.003. Measure after 05-03 and 05-07 with `tools/ci-record.sh` (orchestrator only, never an executor), and use the lever order (V-16) before adding checks. Prefer `<suite>.judge.*` over faults, line formats over `string(JSON)`.
18. **Process, not a gate.** Plan 05-01 contains no C and writes the oracles; each later SUMMARY names the oracle commit that precedes the implementation commit and the verifier checks `git log` order. An expectation file generated from a run (instead of derived by hand and committed before the program first ran) defeats the whole evidence plan; the `.expect` assert lines must equal the scenario's (`golden-expect-drift`).
19. **IVSS and report growth are one-shot.** The `ivsr 2` bump is "not reversible without a goldens refresh; pick once" (audio note); decide the full new key list in one plan (05-08) with the parser reasons, the golden key list and the controls. IVSS keywords are append-only; no directive names a file or a rate (D-20).
20. **Listing risks.** The listing checker does not model SDBD (it fails closed), does not check branch targets, and requires a follower after `HLT` (`docs/diagnostics.md` "Endings need a follower"). The fixture must stay a state-only predicate (V-07: no timing assertion from the cycle table), at most 4 branches, about 100 words, at most 20,000 ticks reset to halt.

### Move-together table (counts and inventories that must change in the same plan)

| Change | Must move together |
|---|---|
| new core TU | `core/CMakeLists.txt` `IVS_CORE_SOURCES` only (fault copies, `c-build` list and symbol checks follow); stem unique across `core/` and `runner/` |
| new generated core include | generator, `data/expected/core-generated.ids`, `ivs_core_gen` dependency list, fault `GEN_*` registration, `-DROOT -DOUT` interface |
| new reason `IVS_REASON_*` | `ivs_machine.h`, `data/expected/api-reasons.ids`, `api-effects.json` row text and tests, runner report detail, test ids |
| new public struct field | header comment with offsets, pinned v1/v2 constants, `run.struct.*` pins, consumer `v1_sizes`, `pkg-consumer.golden` |
| new digest domain | `ivs_machine.h`, `ivs_digest_domain_known`, field table, tests (`MD_DOMAIN_IDS`, snapshot count, refusal retarget), runner record/report/golden, consumer, `docs/runner.md` |
| composite v2 | `machine.h` version define, composite loop, test literals, four golden digest files, `pkg-consumer.golden`, `docs/time-base.md` if it states the domains |
| new suite | `tests/CMakeLists.txt` (executable, `add_test`, `ivs_fault_suite`), `data/expected/tests/<suite>.ids`, `faults/<suite>.ids`, `important-gates.ids` rows |
| new named fault | one site, one `faults/*.ids` line, allowed prefixes covering collateral failures, important-gates row if it guards a requirement |
| new profile field | profile JSON, `profile-fields.ids`, discrepancy id (existing), time-unit suffix rule, `cite` source id in `sources.ids` |
| new discrepancy | `discrepancies.json` (ascending), `discrepancies.ids`, `d10-measurements.ids` when the resolver is `measurement:` |
| new fixture | `manifest.json` outputs hashes, `fixtures.ids`, `listings.ids`, `data/expected/tests/pack.ids`, `tools/pack/CMakeLists.txt`, `tests/CMakeLists.txt` image include, `runner-scenarios.ids`, goldens, claim |
| new IVSS reason or subject | `ivs_runner.h`, `scenario.c`, reason name table, `runner_parse.c` refuse case, `docs/runner.md` |
| `ivsr 2` | `report.c`, `runner_exec.c`, `ivs_runner.h` comment, `docs/runner.md`, `runner-process-tap.sh`, five control files, all goldens |
| new control group | `controls/<name>/`, `data/expected/controls/<group>.ids`, `_good/tree` toy data so the positive control passes |

## Metadata

**Analog search scope:** `core/`, `cmake/`, `runner/`, `tools/` (steps, check, controls, pack, cart), `tests/` (all suites, harness, installed-consumer, shell TAP), `data/` (profiles, memmap, api, expected, golden, scenarios, records), `fixtures/`, `docs/`, `.planning/{AUTONOMY,CI-EFFICIENCY}.md`, `AGENTS.md`, Phase 4 PATTERNS and Phase 5 CONTEXT and DISCUSS files.
**Files read for excerpts:** about 70 (core headers and TUs, generator, memsys, digest, runner headers and sources, report writer, golden step, listing checker, important-gates, core-symbols, faults framework, tests CMake, run/runner/machine/memmap test files, consumer, pkg-install, fixtures, goldens, scenarios, records).
**Not run:** no build, no gate, no `gh`, no network, no git writes. Statements about gate behavior come from reading the scripts at the base commit.
**Pattern extraction date:** 2026-11-11
