# Phase 4: Headless Runner and Installed Consumer - Pattern Map

**Mapped:** 2026-11-11
**Branch:** gsd/phase-4-headless-runner-installed-consumer
**Input:** `04-CONTEXT.md` (D-01..D-47) plus the six `04-DISCUSS-*.md` audit files (used for file names only).
**Files analyzed:** 76 new or modified files (grouped below)
**Analogs found:** 66 exact or role-match / 76; 10 partial or none (section "No Analog Found")
**Cross-check:** `04-RESEARCH.md` (written in parallel, 22-row mechanism table) agrees with this map; deltas are folded in and cited by name.

Every analog path below was checked with `git ls-files` (tracked source, no gitignored mirror). Line numbers are
of the tracked file as of this commit. Paths are repo-relative on purpose (the hygiene step rejects home paths).

## How to read this file

Gate machinery is the main Phase 4 surface, so five repo-wide conventions are stated once here and referenced by letter.

- **[S] Step contract** (`tools/gate.sh` L13-14, L83-101; every file in `tools/check/steps/`). A step is `tools/check/steps/<name>.sh`,
  run as `sh <step>` with env `ROOT`, `CHECK_DIR`, `FILELIST` (tracked plus untracked-not-ignored, repo-relative, sorted, only
  `[A-Za-z0-9._/-]` paths that exist), `GATE_TMP`, `IVS_GATE_DIR` (= `build/gate`), `IVS_TEST_NONCE`. Findings print
  `-- FAIL <token>: <detail>`; success prints exactly one `-- <name> pass <k=v ...>` line (the k=v words are aggregated into the final
  `gate:` line; names containing `skipped|missing|pending|unknown` land in the D-42 "Skips and missing" table). Step names are the
  union of `data/expected/gate-steps/*.ids` (first token per non-`#` line); a script without a listed name or a name without a script
  is `gate-step-extra` / `gate-step-missing`. Steps run in sorted name order.
- **[C] Control layout** (`tools/check/run-controls.sh` L4-5, L133-200). `tools/check/controls/<name>/` holds `CHECK` (one step name),
  `EXPECT` (token lines, matched with `grep -F`; no blank line), optional `FORBID`, optional `overlay/` (copied over a fresh copy of
  `controls/_good/tree/`), optional `MAKE` (verbs `delete empty write rename crlf bom nonascii homepath secret password binary highbytes nul
  scratchpath credential email`). `<name>` must be listed in some `data/expected/controls/*.ids` (`control-not-in-inventory` /
  `control-inventory-orphan` otherwise). The step runs with `ROOT=<tmp>/tree`, `FILELIST=<find . -type f>`, `IVS_GATE_DIR=<tree>/build/gate`.
  Every distinct `CHECK` step must also PASS on the untouched `_good/tree` (positive control). The tree is text only; bad bytes are created by
  `MAKE` at run time. A step a control drives must therefore be a pure checker over files.
- **[I] Inventory style** (`data/expected/**.ids`). `#` comment lines, first whitespace token = id, extra columns are step-specific
  (`skip-ok` for tests; `<FAULT_ID> <must-fail> <allowed-prefixes|> <must-pass>` for faults; `NAME VALUE` for pins). Faults files need a
  `# suite=<name>` header. Test ids are dotted lower-case `suite.family.slug`. Inventories are exact sets, never subsets.
- **[H] File headers.** C/H: `/* SPDX-License-Identifier: MIT */` line 1. sh: `#!/bin/sh` then `# SPDX-License-Identifier: MIT`. cmake and yml:
  `# SPDX-License-Identifier: MIT` line 1. `.ids`, `.json`, `.md`, `.txt`, goldens carry none. `tools/check/integrity.cmake` L74-80 enforces SPDX only for
  `tools/**.sh|cmake`, `.github/workflows/*.yml`, `(core|include|tests)/**.c|h|sh|cmake`, `cmake/*.cmake`, root `CMakeLists.txt`,
  `(core|include|tests)/**/CMakeLists.txt`. `runner/` and `tools/pack/` are NOT in that regex (still add the header; see hazard 13).
- **[T] TAP suite contract** (`tests/harness/ivs_test.h`, `tests/headers-tap.sh` L72-85). A ctest test `<suite>` writes
  `<IVS_TEST_OUT_DIR>/<suite>.txt`: `# suite=<s> nonce=<IVS_TEST_NONCE|none>`, optional `# source <path> <sha256>`, `1..N`, `ok N <id>` /
  `not ok N <id>` / `ok N <id> # SKIP <reason>`, trailer `# suite=<s> executed=<n> skipped=<k> failed=<f>`. Exit 0 pass, 1 fail, 77 all skipped.
  `tests-inventory` requires ctest name == `data/expected/tests/<suite>.ids` stem == TAP header suite, in BOTH presets, every add_test with
  `SKIP_RETURN_CODE 77`.
- **[F] Named-fault contract** (`tests/faults.cmake` header L1-19, `faults-verdict.sh` header). One `IVS_FAULT_<ID>` site in exactly one TU; an
  `ivs_fault_<ID>` executable is the suite's test sources plus plain copies of every other TU plus that TU compiled with `-DIVS_FAULT_<ID>`;
  the id appears in exactly one line of exactly one `data/expected/faults/*.ids`; the faulted TAP must list exactly the suite's ids, fail the
  must-fail id, fail only ids with an allowed prefix, and pass the must-pass id.

## File Classification

### A. Public API, core internals, profile (CORE)

| New/Modified File | Role | Data Flow | Closest Analog | Match |
|---|---|---|---|---|
| `core/include/ivs/ivs_machine.h` (add run request/report, stop reasons, `ivs_input_event`, cpu_state, peek, digest domain names, `ivs_info` timebase/layout fields) | public header | request-response | same file: `ivs_load_options`/`ivs_load_report`/reasons (L121-183), `ivs_info` (L103-112) | exact |
| `core/include/ivs/ivs.h` (only if version moves; no change otherwise) | public header | n/a | itself L20-24 | exact |
| `core/src/machine/machine.h` (instance fields for rows 16-18 and input words, `IVS_MACHINE_BYTES`, `ivs_machine_run_env_internal` seam) | internal header | n/a | itself: step seam L117-128, overlap decls L100-115, layout L12-26 | exact |
| `core/src/machine/machine_run.c` (`ivs_machine_run`, env seam, `ivs_run_stop_name`) | service | request-response + loop | itself: `ivs_machine_step_unit_internal` L84-130; `machine_load.c` validate-then-commit L1-50, public fn L340-380 | exact |
| `core/src/machine/machine_observe.c` (new: `ivs_machine_cpu_state`, `ivs_machine_peek`) | service | request-response | `core/src/machine/machine.c` `ivs_machine_info` L240-265 | role-match |
| `core/src/machine/machine_input.c` (new, name free: event validation, apply, INPUT chain) | service | event-driven | `machine_load.c` validate/commit split (L209-313); `machine_digest.c` sink L89-147 | role-match |
| `core/src/machine/machine_digest.c` (rows 16-18, domain range, chain update) | service | transform | itself: field table L19-72, serializer L185-223, entry L255-302 | exact |
| `core/CMakeLists.txt` (new TUs, `ivs::core` alias/EXPORT_NAME, `BUILD_INTERFACE`/`INSTALL_INTERFACE`, PIC, install rules) | config | build | itself L11-72 | exact (install rules: none) |
| `CMakeLists.txt` (parse `IVS_API_VERSION_*` before `project()`, `include(GNUInstallDirs)`, `add_subdirectory(runner)`, tools/pack) | config | build | itself L1-20 | exact |
| `CMakePresets.json` (`CMAKE_INSTALL_LIBDIR=lib` in `base`) | config | build | itself L4-10 | exact |
| `cmake/ivsConfig.cmake` (hand-written package config) | config | build | none (see No Analog) | none |
| `core/include/ivs/ivs_run.h` (OPTIONAL new public header proposed by 04-RESEARCH.md; CONTEXT says additions to `ivs_machine.h`) | public header | request-response | `core/include/ivs/ivs_status.h` (small header, `#ifndef IVS_API`, `extern "C"`, one-line prototypes) | role-match |
| `cmake/IvsMemmapTable.cmake` + `core/src/memmap/memsys.h` (registry scalars `scanlines_per_frame`, `cpu_cycles_per_scanline`, optional `input.controllers`) | generator + internal header | batch | same file L240-262 (`ticks_per_cpu_cycle` scalar) | exact |
| `data/api/api-effects.json` (rows for run, cpu_state, peek, `ivs_run_stop_name`; digest row edit) | data record | CRUD | same file: `ivs_machine_create` L35-70, `ivs_machine_state_digest` L300-327 | exact |
| `tools/check/api-effects.cmake` (generic pin check for stop reasons and input bit names) | checker | batch | itself section 1 L63-143 | exact |
| `data/expected/api-stop-reasons.ids`, `data/expected/input-bits.ids` | inventory | pin | `data/expected/api-statuses.ids` | exact |
| `data/profiles/intv-2609-ntsc-family-p1.json` + `data/expected/profile-fields.ids` (`input.controllers`) | data record | CRUD | same file, field `cartridge.features_supported` (record shape tag/cite/conflict/decision/note) | exact |

### B. Core tests and faults

| New/Modified File | Role | Data Flow | Closest Analog | Match |
|---|---|---|---|---|
| `tests/machine/machine_runapi.c` (new family: run, slicing invariance, output-buffer matrix, refusals) | test | request-response | `tests/machine/machine_iso.c` (slice vs reference trace), `machine_refuse.c` (row matrix), `machine_reset.c` | exact |
| `tests/machine/machine_observe.c` (new family: cpu_state, peek, side-effect-free) | test | request-response | `tests/machine/machine_reset.c` | role-match |
| `tests/machine/machine_input.c` (new family: events, floor, chain, bit names) | test | event-driven | `tests/machine/machine_digest.c` (literal streams + oracle) | role-match |
| `tests/machine/machine_cases.h`, `machine_main.c`, `machine_digest.c` (domain-16 case), `machine_standin.c` (reuse) | test | n/a | themselves | exact |
| `tests/CMakeLists.txt` (executables, fault-suite registrations, runner suites, `headers` change) | config | build | itself L128-134, L169-181, L220-225, L266-276 | exact |
| `data/expected/tests/machine.ids` (+ any new suite ids) | inventory | pin | `data/expected/tests/machine.ids` | exact |
| `data/expected/faults/machine.ids` or new `faults/run.ids` | inventory | pin | `data/expected/faults/machine.ids` L21-60 | exact |
| `tests/faults.cmake`, `tools/check/steps/faults-verdict.sh`, `tools/check/steps/c-build.sh` (core-only to repo-relative, D-24) | config / step | batch | themselves | exact |

### C. Runner (RUN-01, RUN-02)

| New/Modified File | Role | Data Flow | Closest Analog | Match |
|---|---|---|---|---|
| `runner/CMakeLists.txt` (lib `ivs_runner_lib`, exe `ivs_run` -> `ivs-run`, source list file) | config | build | `core/CMakeLists.txt` L11-72 | role-match |
| `runner/` parser (IVSS v1) | utility | transform | `core/src/content/ivsc.c` L60-140 (bounded bytes-in, reason-out) | role-match |
| `runner/` executor, outcome mapping, report writer | service | request-response | `core/src/machine/machine_load.c` (ladder + precedence) | partial |
| `runner/main.c` | entry | request-response | `tests/harness/ivs_test.c` L36-52 and L171-210 (argv parse, getenv, exit codes) | partial |
| `runner/hostio.c` (POSIX files, monotonic clock, `signal()`) | utility | file-I/O | none | none |
| `tests/runner/*.c` (in-process, fake host vtable, parser table, fuzz, slicing, faults) | test | request-response | `tests/ivsc/ivsc_main.c`, `tests/ivsc/ivsc_fuzz.c` | role-match |
| `tests/runner-process-tap.sh` (name free; real exe, scenarios, judge self-controls) | test | batch | `tests/headers-tap.sh`, `tests/core-symbols-tap.sh` | role-match |
| `data/expected/tests/runner.ids`, `runner-process.ids`, `runner-scenarios.ids`, `faults/runner.ids` | inventory | pin | `data/expected/tests/headers.ids`, `faults/memmap.ids` | exact |
| scenario `*.ivss` files (location not fixed by CONTEXT) | data | n/a | `data/cart/*.json` + `data/cp1610/vectors/*.json` (tracked text inputs) | partial |

### D. Fixtures, diagnostic, listing checker, pack tool (RUN-06)

| New/Modified File | Role | Data Flow | Closest Analog | Match |
|---|---|---|---|---|
| `fixtures/FX-boot-stub/{manifest.json, boot-stub listing}`, `fixtures/FX-diag-cpu-1/{manifest.json, listing}` | fixture | file-I/O | `fixtures/FX-ivsc-vectors/manifest.json`; listing lines: `data/cp1610/vectors/immediate.json` `program` strings | role-match |
| `data/expected/fixtures.ids` | inventory | pin | itself | exact |
| `tools/check/listing.cmake` + `tools/check/steps/listing.sh` + `data/expected/listings.ids` (+ `bootstub-rows.ids`) | checker + step | batch | `tools/check/vectors.cmake` L212-297, L323-363, L367-381, L562-620; `steps/cp1610-vectors.sh` | exact (copy) |
| `tools/check/controls/listing-*` + `data/expected/controls/listing.ids` + `_good/tree` toy listing | control | batch | `tools/check/controls/vectors-*` (40 dirs), `_good/tree/data/cp1610/{opcode-families.json,vectors/toy.json}` | exact |
| `tools/pack/*.c` + CMake (EXEC image + `.ivsc` into the build tree) | tool | file-I/O | `tests/machine/machine_standin.c` L38-101 (`ivs_ms_exec`, `ivs_ms_cart`), `tests/ivsc/ivsc_reencode.c` | role-match |
| `data/golden/*` (`*.golden` per task wording; D-11 says `*.expect` + `*.digests`) | golden | n/a | `data/memmap/*.literal.txt` header (tier, independence rule), `data/sha256/vectors.txt` | partial |
| golden header checker (step name unassigned) + controls | checker + step | batch | `tools/check/steps/memmap.sh` + `tools/check/memmap.cmake` (pure checker over data + results) | role-match |
| `tools/cart/manifest-outputs.sh` reuse/extend for new fixture manifests | tool | batch | itself | exact |

### E. Packaging and installed consumer (PKG-01)

| New/Modified File | Role | Data Flow | Closest Analog | Match |
|---|---|---|---|---|
| `tools/check/steps/pkg-install.sh` + `data/expected/gate-steps/pkg.ids` | step | batch | `steps/c-build.sh` (configure/build, logs only on failure), `steps/c-purity.sh` L283-296 (in-step `expect` self-tests), `tools/check/core-symbols.sh` (nm normalisation) | role-match |
| `tests/installed-consumer/{CMakeLists.txt, main.c}` | test (out-of-tree project) | request-response | `tests/machine/machine_reset.c` (API sequence); no CMake analog | partial |
| `data/expected/pkg-install-files.ids`, `pkg-consumer-calls.ids`, `pkg-consumer.golden` | inventory / golden | pin | `data/expected/api-statuses.ids`, `data/expected/tests/headers.ids` | role-match |
| `tests/headers-tap.sh` (optional include-dir argument) | test | batch | itself L17-31 | exact |

### F. CI, summary, measurement, parity (PKG-03, PKG-04, RUN-04)

| New/Modified File | Role | Data Flow | Closest Analog | Match |
|---|---|---|---|---|
| `tools/gate.sh` (EXIT-trap summary, `gate-env`, `gate-cpu`, fails/times capture) | script | batch | itself L11, L17-20, L128-129 | exact |
| `tools/check/ci-summary.sh` (helper, not a step) | script | transform | `tools/check/binscan.sh`, `tools/check/core-symbols.sh` (helpers in CHECK_DIR that are not steps) | partial |
| `tools/ci-record.sh` (orchestrator-only, uses `gh`) | script | batch | `tools/gate-linux.sh` header/Usage/Exit style (local helper, not a step) | partial |
| `.planning/ci-runs.tsv` + `tools/check/steps/ci-log.sh` + controls | data + step | batch | `steps/denylist.sh`, `steps/manifests.sh` (pure awk/cmake checker over tracked data) | role-match |
| `data/expected/important-gates.ids` + `steps/important-gates.sh` + controls | inventory + step | batch | `steps/faults-verdict.sh` (id cross-reference between inventories) | role-match |
| `tools/check/steps/ci-parity.sh` (action allowlist, `secrets.`, `strategy:`, `runs-on`) + controls | step | batch | itself L21-125 | exact |
| `.github/workflows/gate.yml` (`timeout-minutes` 10 -> 5) | config | n/a | itself | exact |
| `data/expected/faults/*` + `tools/check/controls/*` for RUN-04 gates | control | n/a | see sections above | exact |

### G. Docs and records

| New/Modified File | Role | Data Flow | Closest Analog | Match |
|---|---|---|---|---|
| `docs/diagnostics.md`, `docs/formats/ivss-v1.md`, `docs/formats/ivsr-v1.md` (report), `docs/runner.md` (optional) | doc | n/a | `docs/formats/ivsc-v1.md` | role-match |
| `data/claims.json` (+ `data/expected/claims.ids`): CL-0008 and any later | data record | CRUD | CL-0007 entry | exact |
| `data/discrepancies.json` (+ `data/expected/discrepancies.ids`): C14 status, C15, new entries from D-16 and D-31 | data record | CRUD | C62 entry, C14/C15 entries | exact |
| `.planning/AUTONOMY.md`, `.planning/CI-EFFICIENCY.md` | planning doc | n/a | themselves | exact |

### H. Gate plumbing (checker steps, inventories, hygiene rules)

| New/Modified File | Role | Data Flow | Closest Analog | Match |
|---|---|---|---|---|
| `data/expected/gate-steps/<new>.ids` (one file per owning plan; lists new step names) | inventory | pin | `data/expected/gate-steps/machine.ids` (`memmap`, `api-effects`) | exact |
| `tools/check/steps/runner-scan.sh` (runner includes only `ivs/*.h`, libc, POSIX; proposed by DISCUSS-runner and 04-RESEARCH.md) | step | batch | `steps/c-purity.sh` stage 1 include scan (header L5-30) | role-match |
| `tools/check/steps/ci-summary.sh`-selftest step (name free) | step | batch | `steps/runner-selftest.sh` | exact |
| `tools/check/integrity.cmake` (extend SPDX/ASCII directory patterns to `runner/` and C files under `tools/`) + controls | checker | batch | itself L60-80, controls `integrity-missing-spdx` | exact |
| `data/expected/controls/<group>.ids` per new control group | inventory | pin | `data/expected/controls/api-effects.ids`, `ci.ids` | exact |
| `tools/check/controls/_good/tree/**` toy inputs for every new pure step | control tree | n/a | `_good/tree/data/cp1610/vectors/toy.json`, `_good/tree/core/src/cpu/good.c`, `_good/tree/data/expected/faults/cpu.ids` | exact |

## Pattern Assignments

### A1. `core/include/ivs/ivs_machine.h` (public header, request-response)

**Analog:** the same file, `ivs_load_options` / `ivs_load_report` and the reason list (L121-183).

**Struct rule to copy** (header L7-14): every struct starts `uint32_t struct_size`; in-structs below the v1 size or above
`IVS_STRUCT_SIZE_MAX` (4096) are BAD_ARG, larger sizes need a zero tail else UNSUPPORTED, nonzero `flags` UNSUPPORTED; out-structs below v1 size
BAD_ARG with nothing written. Constants are `#define`s with `u` suffix, no enums/bool/bitfields. Prototypes are ONE line starting `IVS_API`
(`api-effects.cmake` L155 matches `^[ \t]*IVS_API([ \t]|$)` and takes the identifier before the first `(`):

```c
IVS_API ivs_status ivs_machine_load(ivs_machine *m, uint32_t kind, const uint8_t *bytes, size_t len, const ivs_load_options *opts, ivs_load_report *report);
```

**Reason numbers** (L132-167): append-only, `IVS_REASON_*`, new run/input reasons start at 34 (after `IVS_REASON_SLOT_OCCUPIED 33u`), each with a
trailing `/* STATUS: why */` comment. Phase 3 pins the parser-shared ones with `_Static_assert` in `machine_load.c` L40-65; Phase 4 stop reasons and
input bits have no pin check yet (see B5/A7).

**Digest domains** (L57-66): rename `IVS_DIGEST_RESERVED_VIDEO|AUDIO|INPUT` (16/17/18) to live names and update the comment (L57-58 "refused until they
exist"). `tests/machine/machine_digest.c` L529 (`md_refuse_domain_16`) and the `ivs_machine_state_digest` doc comment (L207-211) and
`api-effects.json` L304 ("domain other than 0..4 (the reserved 16..18 included)") change in the same plan.

**Out-struct growth** (`ivs_info`, L103-112, D-06): `machine.c` L245 refuses `out->struct_size < sizeof(ivs_info)` and L262 does `*out = r`. Appending
timebase and layout fields grows `sizeof(ivs_info)`, so the minimum accepted size moves with it. Decide explicitly (keep a v1 minimum, or accept the move
pre-1.0) and say it in the header comment.

**Overlap rule for every new out struct/buffer** (machine_load.c L344-354): report/video/audio pointers are tested against the instance bytes with
`ivs_machine_span_overlaps` (uintptr_t differences only), and only after the handle check:

```c
const struct ivs_machine *live = ivs_machine_from_handle(m);
uint32_t aliased = live != NULL ? ivs_machine_span_overlaps(report, sizeof *report, live, sizeof *live) : 0u;
#ifdef IVS_FAULT_REPORT_INSIDE_UNCHECKED
aliased = 0u;
#endif
```

### A2. `core/src/machine/machine_run.c` (service, request-response + loop)

**Analog:** same file, `ivs_machine_step_unit_internal` (L84-130) plus the public function layout of `machine_load.c` (L340-380).

**Header comment convention** (machine_run.c L1-25, machine_load.c L1-45): list the refusal ladder in order, one numbered stage per class, name the faults at
the end ("Named faults (plan 03-12): RESET_CLEARS_BEFORE_IMAGE_CHECK, ..."). D-05's order is the ladder to write.

**Imports** (machine_run.c L26-29): `#include "common/ivs_prelude.h"` FIRST (c-purity rule, `float`/`double` poisoned), then `"ivs/ivs.h"`, `"machine/machine.h"`.
No new libc names: the core allowlist (`tools/check/core-symbols-allow.txt`) permits only memcpy/memmove/memset/memcmp/bzero and the stack-protector names.

**Core loop pattern** to extend (L94-129): argument checks -> `ivs_machine_from_handle_mut` -> `phase != IVS_PHASE_POWERED` -> `BAD_STATE` -> per unit
`ivs_machine_step_unit_internal` -> `im->time_ticks` advances by `elapsed_cpu_cycles * ticks_per_cpu_cycle` (L127). Classify exit as fault > halt > target (D-04); a
no-progress unit (`UNIT_FAULT`, `elapsed_cpu_cycles == 0`) must end the loop as GUEST_FAULT, never spin (stepper-conventions: Phase 2 D-12).

**Fault-site style** (L31-35, L120-126): `#ifdef IVS_FAULT_<ID>` blocks inside the one owning TU; a file-scope `static` exists only under the fault macro (the
c-purity writable-data rule forbids real statics):

```c
#ifdef IVS_FAULT_STATIC_CARRY
static uint32_t ivs_run_carry_cycles;
#endif
```

**Env seam** (D-06): declare `ivs_machine_run_env_internal` in `machine.h` next to the step seam (machine.h L117-128: doc comment states what is checked,
what is written only on IVS_OK, "Never public: declared here and nowhere under core/include"). The public call wraps it with the real (zero-length) layout; tests pass a
synthetic layout.

**Where input events apply** (D-14): immediately before beginning each unit whose start tick is `>= at_tick`, never at call exit. 04-DISCUSS-runapi 4.5 ("at the top of the loop before the exit check") reads
differently; 04-RESEARCH.md summary (b) shows only the D-14 form keeps `input_floor_tick` (start of the last begun unit + 1) self-consistent across sliced calls.

**Slicing/overshoot state rule** (D-01, D-07): no static, no field outside the instance; everything resumable lives in `struct ivs_machine`, as the header says
(machine.h L8-11 "The instance holds no last-error, counter or timestamp").

### A3. `core/src/machine/machine_observe.c` (service, request-response)

**Analog:** `ivs_machine_info` (machine.c L240-265): out struct check first, then `ivs_machine_from_handle` -> BAD_HANDLE, then overlap -> BAD_ARG, then fill a local `r`
(`memset`, `r.struct_size = sizeof r`) and `*out = r` last. `peek` reads through `ivs_memsys_read(const ivs_memsys *m, uint16_t addr)` (memsys.h, takes a `const`
memsys: the non-mutating view D-08 asks for). Refuse BEFORE any write to `out`. Sites for `PEEK_ADVANCES_TIME` and `PEEK_GATE_IGNORED` go here (one TU each).

### A4. `core/src/machine/machine_digest.c` rows 16-18 (transform)

**Analog:** the field table (L19-72). Each new instance member gets rows with `IVS_DF(name, member, width, count, domain, kind)`; a `count > 1` UINT row is preceded by its
`u32` count in the preimage (L216-218), so `chain[32]` serializes as `u32 32` then 32 bytes, `count` as 8 bytes:

```c
IVS_DF("cpu.r", cpu.r, 2u, 8u, IVS_DIGEST_CPU, IVS_DIGEST_KIND_UINT),
```

Constraints that bite:
- `IVS_MACHINE_SLOTS_END` (L36) and the `tail_padding` row (L70-71) assume `slot[]` is the LAST member; add the new members before it or recompute.
- `machine.h` L50-52: `_Static_assert(sizeof(struct ivs_machine) == IVS_MACHINE_BYTES)` plus the layout comment L12 and `#define IVS_MACHINE_BYTES 113456u` change together; the
  assert message says "review the digest field table".
- `ivs_machine_digest_preimage` (L237) and `ivs_machine_state_digest` (L263: `domain > IVS_DIGEST_INSTANCE` -> BAD_ARG) hard-code the 1..4 range; the composite loop
  (L287-298) stays over domains 1..4 (D-10: composite version 1, so every Phase 3 literal stays; pin one first).
- Domain digests hash "IVSD" || domain || version || rows (L186-223); the video/audio chain RULE (`SHA256("IVSR" || ...)`) is a separate, update-time hash.
- Test coverage is automatic (`machine.digest.coverage` and the sensitivity sweeps iterate `ivs_machine_digest_field_count()`), but `machine_digest.c` hard-codes
  `d <= IVS_DIGEST_INSTANCE` loops (L165) and the domain-16 refusal (L529); update both.

### A5. Core CMake: `core/CMakeLists.txt`, `CMakeLists.txt`, `CMakePresets.json`

**Source list** (core/CMakeLists.txt L11-21): explicit, no GLOB, alphabetical; the same list drives `ivs-core-sources.txt` (L66-72, read by `c-build`) and
`tests/faults.cmake` (L26-30, plain copies). Add each new core TU here and nowhere else.

**Object loop** (L45-61) is where PIC goes (`set_target_properties(${obj} PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF)` neighbour):

```cmake
add_library(ivs_core STATIC ${ivs_core_objects})
set_target_properties(ivs_core PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF LINKER_LANGUAGE C)
target_include_directories(ivs_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")   # L61: absolute path, breaks install(EXPORT)
```

L61 must become `$<BUILD_INTERFACE:...>` / `$<INSTALL_INTERFACE:include>` (D-33). `ivs_warnings` and `ivs_san` are linked PRIVATE to the OBJECT libs (L54), which is why the
export carries no extra targets. `install(...)` rules do not exist anywhere yet (see No Analog Found). Root `CMakeLists.txt` L5 is `project(ivs LANGUAGES C)`: the version parse
goes before it (`ivs.h` L20-22 spells `0u`, a `u` suffix the regex must allow). Presets: `CMakePresets.json` `base` (L6-10) has no `cacheVariables`; add `CMAKE_INSTALL_LIBDIR=lib` there.

### B1. Core test families (`tests/machine/machine_runapi.c`, `machine_observe.c`, `machine_input.c`)

**Analog (case table + helpers):** `tests/machine/machine_reset.c` L1-35 and tail L405-419; helper API in `tests/machine/machine_cases.h` L61-97 (`ivs_ms_powered`, `ivs_ms_step`,
`ivs_ms_digests`, `ivs_ms_new/free/content`, stand-in EXEC programs A/B at `$1000`). Check macro to copy verbatim:

```c
#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)
```

Case table shape (machine_reset.c tail): `const ivs_test_case ivs_machine_reset_cases[] = {{"machine.reset.equivalence", mr_equivalence, NULL}, ...};` then
`const size_t ivs_machine_reset_count = sizeof ... / sizeof ...[0];`. Declare in `machine_cases.h` (extern table + count), concatenate in `machine_main.c` L72-82
(`ivs_machine_add(&n, ivs_machine_xxx_cases, ivs_machine_xxx_count)`), list the file in BOTH `add_executable(ivs_tests_machine ...)` (tests/CMakeLists.txt L128-130) AND
`ivs_fault_suite(machine SOURCES ...)` (L220-225). `machine_main.c` L12-13: `IVS_MACHINE_MAX_CASES 512u` and `IVS_MACHINE_MAX_SOURCES 8u` are hard caps (suite has 381 ids; the
refuse family is indexed separately).

**Slicing invariance** (D-07): copy `tests/machine/machine_iso.c` L1-60: a solo reference run with checkpoints every `MI_EVERY` units (`mi_solo`, digests per checkpoint via
`ivs_ms_digests`), then a chunked run compared at every checkpoint; extend with five slicing patterns, whole-storage `memcmp` (`memcpy(snap, x.storage, IVS_MACHINE_BYTES)` as
in machine_reset.c tail), and `ivs_machine_run(now + 1)` as the single-step idiom against `ivs_ms_step`.

**Refusal oracle** (D-05, D-06): copy the `rf_row` matrix idea of `tests/machine/machine_refuse.c` L1-60, L118, L960: one row = one call + a prior, oracle = status, whole-storage memcmp
(a heap buffer of exactly the instance size, `aligned_alloc(IVS_MACHINE_ALIGN, IVS_MACHINE_BYTES)`), digest equal before/after, poisoned (`0xA5`) out struct untouched except the listed
detail fields. Ids `machine.refuse.<op>.<slug>`. The run rows must ALSO appear in `data/api/api-effects.json` with those ids in `tests` (A6).

**Literal oracle for the INPUT chain** (D-16): copy the machine_digest.c L1-12 discipline: literal byte streams typed from `cmake -E sha256sum` at authoring time; the code under test
never supplies its own expectation. NOTE `cmake/IvsShaVectors.cmake` (string(SHA256) over text) cannot carry the chain's raw 32-byte `h(n)` or NUL bytes (D-30 says cmake -P cannot write NUL);
produce the literals with POSIX `printf '\ooo'` into a scratch file and `cmake -E sha256sum`, and record that recipe in the test file header (see hazard 11).

**Fuzz** (D-16 "slicing fuzz including events inside the overshoot window"): `tests/ivsc/ivsc_fuzz.c` L1-60: fixed 32-bit seeds, in-repo xorshift32, ids `<suite>.fuzz.seed-NN` plus a
non-vacuity case `<suite>.fuzz.reasons-covered`; nothing reads host time or randomness.

### B2. `tests/CMakeLists.txt` additions

**Suite registration** (L128-134, the machine suite) is the template for new suites:

```cmake
add_executable(ivs_tests_machine machine/machine_main.c ... "${IVS_SHA_INC}" "${IVS_CART_INC}")
target_include_directories(ivs_tests_machine PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(ivs_tests_machine PRIVATE ivs_test_harness ivs_core ivs_core_internal ivs_warnings ivs_san)
add_test(NAME machine COMMAND ivs_tests_machine)
set_tests_properties(machine PROPERTIES SKIP_RETURN_CODE 77)
```

**Shell-TAP suite registration** (L266-276; analogs for `runner-process`):

```cmake
add_test(NAME core-symbols
  COMMAND sh "${CMAKE_SOURCE_DIR}/tests/core-symbols-tap.sh" "$<TARGET_FILE:ivs_core>" ${IVS_SYMBOL_PREFIX_ARGS})
set_tests_properties(core-symbols PROPERTIES SKIP_RETURN_CODE 77 ENVIRONMENT "NM=${CMAKE_NM};AR=${CMAKE_AR}")
add_test(NAME headers COMMAND sh "${CMAKE_SOURCE_DIR}/tests/headers-tap.sh" "${CMAKE_SOURCE_DIR}")
set_tests_properties(headers PROPERTIES SKIP_RETURN_CODE 77 ENVIRONMENT "CC=${CMAKE_C_COMPILER}")
```

**Fault-suite registration** (L169-181 function, L220-225 use):

```cmake
ivs_fault_suite(machine
  SOURCES machine/machine_main.c ... machine/machine_sweep.c
  INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
  GENERATED "${IVS_SHA_INC}" "${IVS_CART_INC}")
```

Fault executables run the WHOLE suite once per fault (c-faults L96-142). A new smaller suite (`run`, as DISCUSS-ci suggests) bounds per-fault cost; faults inside the 381-id machine suite
are cheaper to register but slower to run. Either way: `data/expected/tests/<suite>.ids` must exist for the suite named in the `# suite=` header (faults-verdict L52-56).

### B3. Named faults for the core (D-07, D-08, D-16)

**Analog:** `data/expected/faults/machine.ids` L1-60. Line shape and an example:

```
# suite=machine; columns: <FAULT_ID> <must-fail> <allowed-prefixes> <must-pass>
STATIC_CARRY machine.iso.interleaved-equals-sequential machine.iso.|machine.reset. machine.reset.iab-reset-vector
RESET_KEEPS_RAM machine.reset.equivalence machine.reset.equivalence machine.reset.step-program
```

Rules that cost time: allowed-prefix lists are `|`-separated and must cover EVERY collateral failure (INIT_SKIPS_MEMSET line shows how long they get); comment blocks above the lines record the
reasoning per fault (copy that habit). Ids are `[A-Z0-9_]+`, unique across all files (existing: `STATIC_CARRY`, `DIGEST_*` ...; D-07/D-16 names `RUN_*`, `PEEK_*`, `INPUT_*` do not collide today).
`GEN_*` faults (generator mutation) have a different registry (`ivs_fault_gen`, tests/CMakeLists.txt L191-207).

### B4. Fault framework generalisation (D-24): `tests/faults.cmake`, `faults-verdict.sh`, `c-build.sh`

Everything below is core-only today and must move to repo-relative owner paths together:

- `tests/faults.cmake` L26-30 reads `IVS_CORE_SOURCES` (relative to `core/`); plain copies L45-54 are `ivs_fault_plain_<stem>`; owner detection L213-224 scans `core/${src}` text for
  `IVS_FAULT_<ID>[^A-Za-z0-9_]` and requires exactly one owner (`message(FATAL_ERROR "fault ${id}: IVS_FAULT_${id} must appear in exactly one core TU ...")`); `ivs_fault_exe` L77-117 builds
  harness + plain core objects + suite objects. A runner suite also needs plain copies of the runner library sources and an owner scan over `runner/`.
- `faults-verdict.sh` L129-137 site scan: `grep -E '^core/src/.*[.](c|h)$' "$FILELIST"` (extend to `^runner/.*[.](c|h)$`; keep the `^[[:space:]]*#[[:space:]]*if` + `IVS_FAULT_` rules).
- `c-build.sh` L38-43: `c-build-unlisted-source` only compares tracked `core/*.c` with `build/dev/ivs-core-sources.txt`; add the runner list (and `tools/pack`) or an unbuilt `runner/*.c` escapes.
- Plain copies and object libraries are named from `NAME_WE` of the file (`ivs_core_obj_<stem>` core/CMakeLists.txt L47-48, `ivs_fault_plain_<stem>` faults.cmake L46-48), so a `runner/` stem that equals a core stem
  (for example two `machine*.c`) collides: add a configure-time uniqueness check. Plain copies of runner TUs get only `core/include` (plus the runner dir), so a runner TU that reaches into `core/src` fails to compile in the fault build, which is the intended guard.
- `_good/tree` has `core/src/cpu/good.c` and `data/expected/faults/{cpu,second}.ids`; a runner-site control needs a toy `runner/good.c` there (controls `faults-orphan-site`, `faults-missing-site` are the analogs).

### B5. `tools/check/api-effects.cmake` + pins (`api-stop-reasons.ids`, `input-bits.ids`)

**Analog:** section 1 (L63-143): define-lines of a header versus `NAME VALUE` pin lines, with reserved-band, reused-number and unpinned/renumbered tokens:

```cmake
if(l MATCHES "^[ \t]*#[ \t]*define[ \t]+(IVS_OK|IVS_E_[A-Z0-9_]+)[ \t]+[(]?(-?[0-9]+)[)]?[ \t]*(/[*].*)?$")
```

Statuses are the only pinned class today; stop reasons (D-04), input bit names (D-16) and `IVS_REASON_*` have no checker. Extend this script (new tokens in `ATOKENS` L48-49, one control each) or add a small
second checker with the same shape; do not hide the check inside a TAP test (D-04 says the .ids "pins" the numbers; the existing pin is a gate-step check with controls `api-status-renumbered`, `api-status-unpinned`).
`api-statuses.ids` format: `# comment`, `IVS_E_BAD_ARG -1`.

### A6. `data/api/api-effects.json` rows for new calls (D-05, D-08)

**Analog:** `ivs_machine_create` (L35-70) and `ivs_machine_state_digest` (L300-327). Entry shape: `name`, `header` (must equal the header that declares it), `on_ok`, `precedence[]`
(status, checks, optional `pinned_by` test ids), `notes[]`, `refusals[]` (status, `effect` from the `effects` registry, `outs`, `tests[]`):

```json
{"status": "IVS_E_BAD_ARG", "checks": "out inside the instance bytes of storage (nothing written, checked before *out is cleared)", "pinned_by": ["machine.refuse.create.out-inside-storage", "machine.refuse.create.out-after-instance-ok"]},
{"status": "IVS_E_BAD_ARG", "checks": "opts, storage or out NULL; ...", "pinned_by": ["machine.refuse.create.misaligned-over-flags", "machine.refuse.create.size-garbage"]},
```

Checker facts (api-effects.cmake L335-410): a call may repeat a status in `precedence` (create does) but has exactly ONE refusal row per status (`two rows with this status` is a format error);
every row status must appear in the call's precedence and vice versa; every cited test id must be a first token in some `data/expected/tests/*.ids` and not all `skip-ok`;
`ivs_run_stop_name` (no refusal) follows `ivs_status_name` L22-27 with `"precedence": [], "refusals": []`. D-05's run ladder has four BAD_ARG classes: one row, four precedence entries.
Every pinned non-OK status needs at least one row somewhere (`api-dead-status`); D-05 adds no new status numbers, so none new to pin.

### A7. `data/profiles/intv-2609-ntsc-family-p1.json` `input.controllers` (D-15)

**Analog:** field `cartridge.features_supported` in the same file (record keys `value`, `tag`, `cite`, `conflict`, `resolving_evidence`, `revisit_trigger`, `decision`, `note`; tag from
`data/tiers.json` source_authority_tags, here `delegated-decision`). Add the path to `data/expected/profile-fields.ids` (must equal the members of `fields`). Gate pieces: `tools/check/kinds/profile.cmake`
(tag counts only), `steps/profile.sh`, and the weakest-field rule in `kinds/claim.cmake` L53-150 for any claim that `depends_on` it. C14 and C15 in `data/discrepancies.json` are the records to update.

**Generator and registry (D-06 needs C code to read the timebase).** `ivs_info` gains ticks per CPU cycle and nominal frame ticks (= `scanlines_per_frame` x `cpu_cycles_per_scanline` x
`ticks_per_cpu_cycle`, 262 x 57 x 4 = 59,736, docs/time-base.md L27-31), but `ivs_profile_desc` (core/src/memmap/memsys.h, the generated registry record) carries only `ticks_per_cpu_cycle` today. Copy the
registry-scalar idiom of `cmake/IvsMemmapTable.cmake` L258-259 (`set(MM_CTX "${rel}: ticks_per_cpu_cycle")` then `mm_uint(ticks_per_cpu "${doc}" "value" 65535 fields ticks_per_cpu_cycle value)`) for
`scanlines_per_frame` and `cpu_cycles_per_scanline`, add the members to `ivs_profile_desc`, emit them in the generated initialiser, and keep the generator's fail-closed grammar (header L1-33). The generator is listed in
`data/expected/core-generated.ids` with `ivs_profiles.inc`, so c-purity re-runs it, and `GEN_MAP_MUTATION` (tests/CMakeLists.txt L233-241) mutates its output: both stay valid only if the output name and the `-DROOT -DOUT` interface do not change.
`input.controllers` itself only needs generator support if C code reads it in Phase 4 (D-15: the real profile registers no consumer), but 04-RESEARCH.md mechanism 18 lists it for the registry too; decide once.

### C1. `runner/` sources and CMake (RUN-01, RUN-02)

**Analog (CMake):** `core/CMakeLists.txt` L11-72: explicit source list, repo-relative list written to the build dir for `c-build`, `target_link_libraries(... PRIVATE ivs_warnings ivs_san)`.
`ivs_runner_lib` and `ivs_run` link ONLY public `ivs_core` (not `ivs_core_internal`; tests may use internals, the runner may not). Executable output name `ivs-run`
(`set_target_properties(ivs_run PROPERTIES OUTPUT_NAME ivs-run)`). Root `CMakeLists.txt` adds `add_subdirectory(runner)` before `add_subdirectory(tests)` (tests link the library).

**Analog (parser):** `core/src/content/ivsc.c` L60-140: pure function, bytes in, struct + reason out; length checked first, every read bounds-proven first, first failing stage wins, a
refusal helper that fills a report (`ivsc_refuse(rep, status, reason, seg, off, features)`), reason numbers in one header, no allocation. Copy the discipline, not the code:

```c
if (n_bytes > IVS_IVSC_MAX_BYTES) {
  return ivsc_refuse(rep, IVS_E_OVERSIZED, IVS_IVSC_R_TOO_LARGE, IVS_IVSC_NO_SEGMENT, IVS_IVSC_MAX_BYTES, 0u);
}
```

IVSS parsing needs hand-written ASCII classification and integer parsing (no `strtol`, `sscanf`, `isdigit`; D-22/B2), caps from D-22 (64 KiB, 512 lines, 256 B/line, 64 asserts, 64 steps), line-numbered
reason tokens (`scenario-syntax line=7 reason=...`).

**Analog (argv/env/exit):** `tests/harness/ivs_test.c` L36-52, L171-210 (`ivs_test_parse_args`, `getenv`, `snprintf` into a bounded path, `fopen` failure -> diagnostic + exit 1). The runner's table is D-20:
pass 0, guest-fail 10, timeout 11, cancelled 12, missing-fixture 13, harness-fail 14 (avoids ASan's 1, ctest skip 77, shell 126+).

**Outcome mapping** is a pure function in the library so a fault site (`IVS_FAULT_RUNNER_*`) can sit in it (DISCUSS-ci table: `OUTCOME_GUEST_AS_HARNESS` ...). Precedence (CONTEXT D-20 "fixed precedence";
DISCUSS-runner section 5): harness-fail > missing-fixture > load-refusal-per-expect > cancelled > timeout(wall) > timeout(budget) > guest-fail > pass.

**Report writer:** flat `key value` lines, first `ivsr 1`, last `report.end <n>`, fixed key order, no wall time and no host paths (D-19). Print `uint64_t` with `PRIu64` or an
`unsigned long long` cast (`machine_reset.c` uses `(unsigned long long)`).

**POSIX confinement:** only `runner/hostio.c` defines `_POSIX_C_SOURCE 200809L` (before any include) and holds `clock_gettime`, `stat`, `signal()`; the rest is ISO C17 (`-std=c17`, extensions OFF, root
CMakeLists L7-9). No existing file includes POSIX headers (hazard 12).

### C2. `tests/runner/*.c` (in-process suite `runner`)

**Analog:** `tests/ivsc/ivsc_main.c` + `ivsc_fuzz.c` (table-driven parser suite plus seeded fuzz, ids `ivsc.<class><nn>-<slug>`); harness `tests/harness/ivs_test.h` L29-45 (`ivs_test_suite`,
`ivs_test_case`, `ivs_test_main`). The host vtable (`read_file`, `now_ms`, `cancel_requested`) gets fakes in the test, so timeout/cancel cases use a fake clock, never sleeps. Executable links
`ivs_test_harness ivs_core ivs_core_internal ivs_warnings ivs_san` plus `ivs_runner_lib` (template: tests/CMakeLists.txt L128-134). Faults: `data/expected/faults/runner.ids` with `# suite=runner`
(B4 generalisation first).

### C3. `tests/runner-process-tap.sh` (shell TAP, real executable)

**Analog:** `tests/headers-tap.sh` (whole file; L17-31 args/env, L72-85 TAP emit) and `tests/core-symbols-tap.sh` (L22-39, `1..1` shape with the script output as `#` lines). Copy:

```sh
LC_ALL=C; export LC_ALL
tmp=$(mktemp -d "${TMPDIR:-/tmp}/headers-tap.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
nonce=${IVS_TEST_NONCE:-none}
...
{
  printf '# suite=%s nonce=%s\n' "$suite" "$nonce"
  printf '1..%s\n' "$n"
  cat "$tmp/results"
  printf '# suite=%s executed=%s skipped=%s failed=%s\n' "$suite" "$executed" "$skipped" "$failed"
} > "$tmp/tap"
cat "$tmp/tap"
if [ -n "${IVS_TEST_OUT_DIR:-}" ]; then cp "$tmp/tap" "$IVS_TEST_OUT_DIR/$suite.txt" || exit 1; fi
[ "$failed" -eq 0 ] || exit 1
[ "$n" -gt 0 ] || exit 1
[ "$executed" -gt 0 ] || exit 77
```

A visible skip is `ok N <id> # SKIP <reason>` and the id must be `skip-ok` in `data/expected/tests/runner-process.ids` (headers.ids does this for the C++ cases). Hang guard: copy
`c-faults.sh` `run_limited` (L65-86: background sleep in a watchdog subshell, TERM trap). dash-safe: no `local`, `[[`, `echo -e`, `sed -i`, `timeout`. Judge self-controls (fake `ivs-run` shell scripts
returning a wrong exit code etc.) are TAP ids in the same suite (D-23). The "dev vs release byte-for-byte" check cannot see both trees from one ctest run in a developer `ctest --preset dev`
loop (release tree may be absent): either give that id a visible skip when the other tree is missing, or follow the `memmap-dump.txt` precedent (suite writes an artifact to `$IVS_TEST_OUT_DIR`, a
pure step compares both presets; `steps/memmap.sh` L18-36).

### C4. Scenarios, goldens, `runner-scenarios.ids`

No tracked-scenario precedent exists. Closest: `data/cart/*.json` + `data/expected/cart-fixture-files.ids` (tracked text inputs, listed in an exact inventory, rights-recorded by the manifest
`FX-ivsc-vectors` L15-26) and `data/cp1610/vectors/*.json`. Decide location (`data/scenarios/` needs no manifest unless you want one; `fixtures/<id>/` needs one and lists every file in `outputs`). Inventory
row shape for D-20: `<id> <exit> <outcome> [host-dependent] [skip-ok]` (first token = id, so the generic awk readers keep working; `data/expected/tests/headers.ids` is the `skip-ok` precedent).

**Golden header analog:** `data/memmap/intv-2609-ntsc-family-p1.literal.txt` L1-30: a `#` header block stating claim tier, independence rule and recipe, then rows. Copy that spirit into the
`# tier:` / `# oracle:` lines (D-11): tier must be an id of `data/expected/evidence-tiers.ids` (`original-diagnostic`, `deterministic-repeatability`); `self-recorded` forces
`deterministic-repeatability`; empty domains read `none`/`absent`, never a hash of nothing. The check is a pure cmake/sh checker driven by controls (name it; CONTEXT leaves the step name open):
analog `steps/memmap.sh` + `tools/check/memmap.cmake` (reads tree and results dir, writes nothing). Control ids proposed in DISCUSS-ci: `golden-header-missing-tier`, `golden-header-missing-oracle`, `golden-mismatch`.

### D1. Fixtures and listing files (`fixtures/FX-boot-stub/`, `fixtures/FX-diag-cpu-1/`)

**Manifest analog:** `fixtures/FX-ivsc-vectors/manifest.json` (all keys required, no extras; `kind: original-source`, `committed: true`, `path` listed in `outputs`, `recipe: null`, `licence: MIT`,
`licence_evidence` {url `LICENSE`, `text_sha256`, accessed}, `redistributable: yes`, `allowed_use: [local-test, ci, repo]`, `rights_review` author-attested, `note`). Rules (docs/fixture-manifest.md L56-62, "Rules";
`files.cmake` L13-16, L128-156): every file inside `fixtures/<dir>/` must be an `outputs` entry with current `sha256` and `size_bytes` (else `files-unlisted-fixture-file` /
`manifest-digest-mismatch`); a `fixtures/<dir>/` needs `manifest.json`; `data/expected/fixtures.ids` is the exact set. The helper `tools/cart/manifest-outputs.sh` prints those entries for data/cart
(its `--check` mode is the "outputs are current" idiom); extend or copy it for the new manifests, and rerun in the same commit as every listing edit. The built EXEC image and `.ivsc` are never
tracked (`.gitignore` ignores `*.bin *.rom *.ivsc *.cfg`; `.ivsc` is also `binary-ext.txt`); manifest note text carries the D-28 label "project boot stub, not an EXEC".

**Listing line analog (words):** `data/cp1610/vectors/immediate.json` `program` strings: `"02B9 1010 111 001"` and `"1234 0001 0010 0011 0100"` (hex4, then bit groups; the groups' binary value equals the hex).
D-26 lines are `<ADDR> <hex4> <bit groups> <ROW|-> <asm>`; operand words use row `-`. Keep listings `.txt` (text by name, `tools/check/text-ext.txt`). Remember `integrity` (ASCII-only in `fixtures/`, no CR, no empty file).

### D2. `tools/check/listing.cmake`, `steps/listing.sh`, controls (D-26, D-29)

**Analog (copy these three pieces):** `tools/check/vectors.cmake`:
- row table loader L212-297 (reads `data/cp1610/opcode-families.json`: pattern `[01.]{10}` -> `R_IDS/R_MASKS/R_VALS/R_EQS/R_REFS`),
- `decle_leaves` L323-363 (rows that match a decle minus rows another match refines),
- `program_word` L367-381: `^([0-9A-Fa-f]{4}) ([01][01 ]*[01])$` then numeric hex vs numeric bits (`vector-program-mismatch`),
- the row-label check L593-620: `if(NOT row IN_LIST leaves) vfail(vector-row-mismatch ...)`.
Skeleton to copy (header, inputs guard, `lib.cmake` helpers, capped tokens):

```cmake
cmake_minimum_required(VERSION 3.25)
foreach(required ROOT FILELIST CHECK_DIR)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "vectors.cmake is missing ${required}")
  endif()
endforeach()
include("${CMAKE_CURRENT_LIST_DIR}/lib.cmake")
set(CAP 20)
function(vfail token detail)
  gate_incr("tok_${token}")
  gate_count(n "tok_${token}")
  if(n LESS_EQUAL ${CAP})
    gate_fail(${token} "${detail}")
  endif()
endfunction()
```

End with `gate_stat(...)` and `gate_finish(listing)` (lib.cmake L35-44 prints `-- listing pass <stats>`). The vectors checker also bans emulator names in text fields
(`vnames` L181-190 over `tools/check/denylist-cite.txt` + `vector-emulator-names.txt`: substrings `jzintv`, `mame`): do the same for listing asm/comment text.

**Step wrapper analog:** `tools/check/steps/cp1610-vectors.sh` L16-44 (`cmake -DROOT -DFILELIST -DCHECK_DIR -P`, capture, print findings, re-print the pass line with stats) and the thin
`steps/api-effects.sh` (L12-18: "re-print the checker's pass line once, then exit non-zero without stats").

**Stub guards (D-29)** go in the same checker or a sibling: addresses within `$1000-$101F`, at most 16 non-zero words, row allowlist (`data/expected/bootstub-rows.ids` in DISCUSS: HLT DIS EIS MVII MVO TSTR B J),
words present at the profile's `reset.r7` and `interrupt.iab_value` (read from `data/profiles/*.json` field values `$1000`/`$1004`). Row ids to use are in `data/expected/cp1610-rows.ids` (HLT, TSTR, PSHR, MVII ...).

**Controls:** one dir per token under `tools/check/controls/` (layout [C]) plus `data/expected/controls/listing.ids` (analog `data/expected/controls/api-effects.ids`). The `_good/tree` must carry a TOY
listing and the toy row table it checks against (`_good/tree/data/cp1610/opcode-families.json` has only rows `ALL`, `ZERO` = `0000000000`, `ONES` = `1111111111`; `_good/tree/data/cp1610/vectors/toy.json` is the
precedent), plus its inventory file, or the positive control `control-good-failed` fires for the new step.

### D3. `tools/pack` (build-tree C tool)

No tool of this kind exists. Closest code to copy from: `tests/machine/machine_standin.c` L38-101 (`ivs_ms_exec`: 8192 bytes, word i high byte first, bits 15..10 zero; `ivs_ms_cart`: IVSC v1 magic bytes
`89 49 56 53 43 0D 0A 1A`, big-endian fields, trailer = SHA-256 of all preceding bytes via `ivs_sha256_digest`) and `tests/ivsc/ivsc_reencode.c` L1-45 (`re_put16/re_put32`, canonical encoder). Format source of truth:
`docs/formats/ivsc-v1.md` (Layout L21-35: prefix 24, entry 8 each, data, trailer 32; word <= 0x03FF; segment `addr + len <= 0x10000`) and `core/src/content/ivsc.h` L15-30 for constants.
The tool reads the listing text (same grammar D-26 as the cmake checker, so the two can disagree: add a gate case that packs the real listing and loads it through the public API), writes only under the build tree,
and its outputs are what `runner` (`boot-stub`) and the consumer take as file arguments (D-39). `-DNDEBUG`-safe, no host-dependent bytes (no time, no path in outputs).

### E1. `tools/check/steps/pkg-install.sh` (D-37, D-38, D-40)

**Analog 1 (shape):** `steps/c-build.sh` L1-35 (header with Tokens/Stats, `tmp=$(mktemp -d "${TMPDIR:-/tmp}/<name>.XXXXXX")`, `trap 'rm -rf "$tmp"' EXIT`, `bad() { printf -- '-- FAIL %s: %s\n' ...; rc=1; }`,
`cd "$ROOT"`, logs printed only on failure: `cat "$tmp/$preset.configure.log"; bad c-build-failed "$preset"`). The step installs from the existing `build/release` tree (no third build).
**Analog 2 (in-step self-tests, D-40):** `steps/c-purity.sh` L283-296: an `expect <case> <token> <args>` helper with `selftest_ok/selftest_total` counters, a nonzero exit AND the intended `-- FAIL <token>:` required, failures
reported as `purity-selftest-<case>`, and a pass line carrying `purity-selftest=<ok>/<total>`:

```sh
expect() {
  _case=$1; _tok=$2; shift 2
  selftest_total=$((selftest_total + 1))
  _st=0
  (cd "$st" && sh "$CHECK_DIR/core-symbols.sh" "$@") > "$st/out" 2>&1 || _st=$?
  if [ "$_st" -ne 0 ] && grep -q -E -e "^-- FAIL ($_tok): " "$st/out"; then
    selftest_ok=$((selftest_ok + 1))
  else
    bad "purity-selftest-$_case" "exit $_st, wanted $_tok"
    sed 's/^/# /' "$st/out"
  fi
}
```

Use `pkg-selftest-<case>` and `pkg-selftest=<ok>/<total>`; a case that cannot be built counts as a failed case, as c-purity does for lto.
**Analog 3 (symbols):** `tools/check/core-symbols.sh` L88-105: `norm()` (drop `@ver`, strip one Darwin underscore) and `run_nm` with `nm -P -g --defined-only` / `-P -u`; reuse by sourcing or calling that script,
do not re-implement. **Analog 4 (headers alone):** `tests/headers-tap.sh` L46-61 compile lines (`-std=c17 -pedantic -Wall -Wextra -Werror -fsyntax-only -x c -I`, C++ visible skip when no `$CXX`);
D-38 wants the include dir as an argument, today the script takes exactly one argument and hard-codes `$root/core/include` (L18-21).
Step must not depend on ctest ordering: the golden compare reads the tracked `data/expected/pkg-consumer.golden` and the tracked runner golden, never the runner's output. Pass line (DISCUSS-pkg 16):
`-- pkg-install pass prefix-files=<n> public-symbols=<n> defined=<n> consumer-lines=<n> hdr-alone=<n>/<n> pkg-selftest=<n>/<n>`. Register in `data/expected/gate-steps/pkg.ids` (analog: `data/expected/gate-steps/machine.ids`).
It is not control-drivable (needs a build); the in-step self-tests replace controls, and `important-gates` lists them as `mutant` entries (F3).

### E2. `tests/installed-consumer/{CMakeLists.txt, main.c}` (D-36, D-39)

No in-repo analog for an out-of-tree CMake project. It is NOT added via `add_subdirectory`; the step copies it outside the repo. It must carry SPDX (the integrity regex matches
`tests/**/CMakeLists.txt` and `tests/**/*.c`). Flags (D-36): `-std=c17 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`, `find_package(ivs 0.1 CONFIG REQUIRED)`, `NO_SYSTEM_FROM_IMPORTED ON`. For `main.c`
copy the API call sequence of `tests/machine/machine_reset.c` / `machine_standin.c` (`ivs_machine_storage_requirements` -> aligned storage -> create -> load EXEC+GROM+cartridge -> refusals with digest equal
before/after -> reset -> run -> per-domain digests -> second fresh instance equal). It embeds no content bytes; images arrive as argv paths. Its printed digests are compared with the runner golden for the same scenario.
`data/expected/pkg-consumer-calls.ids` = `nm -u` names of `main.c.o` matching `^ivs_`. The consumer prints the version tie (`ivs_VERSION`, `IVS_API_VERSION`, `ivs_api_version()`).

### E3. `data/expected/pkg-install-files.ids`, `pkg-consumer-calls.ids`, `pkg-consumer.golden`

[I] style; the install inventory is an exact path list (D-33 layout): `lib/libivs_core.a`, `include/ivs/{ivs,ivs_machine,ivs_status}.h`, `lib/cmake/ivs/{ivsConfig,ivsConfigVersion,ivsTargets,ivsTargets-release}.cmake`,
`share/doc/ivs/LICENSE` (GNUInstallDirs: DOCDIR = DATAROOTDIR/doc/PROJECT_NAME). `ivsTargets-release.cmake` ties the inventory to `CMAKE_BUILD_TYPE=Release` of the `release` preset. The golden header states tier and
oracle ("regression pin, emulator is oracle") like D-11.

### F1. `tools/gate.sh` (summary trap, `gate-env`, `gate-cpu`)

**Analog:** the file itself. Existing hooks: EXIT trap L11 (`rm -rf "$GATE_TMP"` after stopping the control runner); `bad()` L17 and `step_time()` L20 are the two places that see every finding and timing; final lines L128-129:

```sh
printf 'gate-time total=%s\n' $(($(date +%s) - gate_t0))
printf 'gate: %s steps=%s/%s controls=%s/%s %s\n' "$status" "$passed" "$want" "$ctl_ok" "$ctl_total" "$tokens"
```

D-42/D-44: append to two files under `$GATE_TMP` from `bad()`/`step_time()`, call `sh "$CHECK_DIR/ci-summary.sh"` in the EXIT trap BEFORE the `rm -rf`, only when `GITHUB_STEP_SUMMARY` is non-empty, `> /dev/null`-quiet on stdout, never altering
the exit status (`|| true`); add `gate-env ncpu=<n> cc=<id>` and `gate-cpu user=<s> sys=<s>` (POSIX `times`, two lines: shell then children) next to `gate-time`. Early `exit 1` paths (L22-24, L28-32, L47-50) also run the trap. The helper's own checks (backticks stripped, unwritable `GITHUB_STEP_SUMMARY` keeps the status, "none" row, 40-line FAIL cap) belong in a
selftest step with the shape of `tools/check/steps/runner-selftest.sh` L1-60 (builds a throwaway tree in `$tmp`, runs the real tool, counts `cases=<n>`, token `<name>-missing`), not in `controls/`.
Helper location analog: `tools/check/binscan.sh` and `tools/check/core-symbols.sh` are scripts in CHECK_DIR that are not steps (the step inventory only globs `steps/*.sh`, L67-70).

### F2. `tools/check/steps/ci-parity.sh` extensions (D-43) + `gate.yml`

**Analog:** itself (L21-125). The awk emits `<token> <file>:<line>`; the shell loop (L117-124) re-prints each as `-- FAIL <token>: <detail>`. The `uses:` branch (L94-101) currently only checks the 40-hex pin:

```awk
else if (key == "uses") {
  ref = ""
  q = index(val, "@")
  if (q > 0) { ref = substr(val, q + 1); sub(/[ \t].*$/, "", ref) }
  if (length(ref) == 40 && ref ~ /^[0-9a-f]+$/) uses++
  else emit("ci-unpinned-action", NR)
}
```

New tokens (DISCUSS-ci E4): `ci-parity-action-not-allowed` (name before `@` other than `actions/checkout`; the unpinned `actions/checkout@main` control must keep emitting ONLY `ci-unpinned-action`),
`ci-parity-secrets`, `ci-parity-matrix` (`strategy:` key), `ci-parity-runs-on` (value other than `ubuntu-24.04`). Each gets a control dir (overlay `.github/workflows/gate.yml`; analog
`tools/check/controls/ci-parity-unpinned-action/{CHECK,EXPECT,overlay/.github/workflows/gate.yml}`), an entry in `data/expected/controls/ci.ids`, and the header comment and `Tokens:` line updated.
`_good/tree/.github/workflows/gate.yml` and `tools/check/controls/ci-parity-*/overlay/` must still be consistent (they already use `runs-on: ubuntu-24.04`). `gate.yml` has `timeout-minutes: 10` today (L15); D-41 says 5, and no check enforces it.
The existing `run:` allowlist (L90-93) already limits workflow commands to `sh tools/gate.sh` / `cmake --version`.

### F3. `important-gates`, `ci-log`, `ci-runs.tsv`, `tools/ci-record.sh`

- `steps/important-gates.sh` + `data/expected/important-gates.ids` (`<gate> <fault|control|mutant> <id> <REQ>`): analog `steps/faults-verdict.sh` L129-144 for cross-referencing ids between inventories with `comm`/`grep -x -F`
  and printing sorted findings; controls `important-gates-unknown-id`, `important-gates-uncovered-gate`. Pure checker (control-drivable): `_good/tree` needs a toy `important-gates.ids` that resolves against the toy faults
  (`GOOD_FAULT` in `_good/tree/data/expected/faults/cpu.ids`) and a toy controls inventory.
- `steps/ci-log.sh` + `.planning/ci-runs.tsv`: pure awk checker like `steps/denylist.sh` (header, tokens, stats line); `_good/tree/.planning/` currently holds only `PROJECT.md`, add a toy TSV there. Tokens `ci-log-header`, `ci-log-fields`, `ci-log-duplicate`, `ci-log-order`. Exact column list: DISCUSS-ci E5.
- `tools/ci-record.sh <run-id>`: orchestrator-only (2 `gh` calls); style analog `tools/gate-linux.sh` L1-25 (header block with Usage/Env/Exit, `LC_ALL=C`, `here=$(cd "$(dirname "$0")" && pwd)`, `exit 2` on missing tool). It must carry SPDX (integrity regex `^tools/.*[.]sh`), print one TSV row, and never be called from the gate or by an executor.

### G1. Docs (`docs/diagnostics.md`, `docs/formats/ivss-v1.md`, report format)

**Analog:** `docs/formats/ivsc-v1.md`: opening paragraph saying what the format is and that it is project-authored, "Conventions" (big-endian, units), a layout table (Offset | Size | Field | Rule), "Limits", a reason table, a
"Fixtures" section, and a claim line `Claim [claim:CL-0007] records this evidence.` Three text gates read every `README.md` and `docs/**/*.md` (hazard 1): claim words need `[claim:CL-NNNN]` or `[nc]`, bare "clock" needs `[nc]` or a qualifier, `jzintv` needs `[cite-only]`.

### G2. Records (`data/claims.json`, `data/discrepancies.json`, inventories)

**Claims analog:** CL-0007 in `data/claims.json` (keys `id statement profile tier artifact depends_on standing note`; `tier` from `data/tiers.json`; `artifact` must be an existing tracked file; ids ascending; `established` needs non-empty
`depends_on` and no weak-tag dependency, `kinds/claim.cmake` L53-150). CL-0008 (D-27): tier `original-diagnostic`, `provisional`, artifact = the diagnostic listing, `depends_on` fields that exist in the profile
(`reset.r7`, `cartridge.slots`; check `data/expected/profile-fields.ids`). Add the id to `data/expected/claims.ids` (must equal the ids in the JSON).
**Discrepancy analog:** C62 (keys `id topic sources status default_policy resolver evidence note`; ids strictly ascending numbers, so new entries are C63 and up; status `open|partial|resolved-primary|withdrawn`; `open`/`partial` need
`default_policy` starting `unknown-default-chosen: `; `resolved-primary` needs `evidence` containing `pdf <n>`; resolver `test:|measurement:|pending:` + text). C14 and C15 exist as `open` with resolver `pending: D6 ...`:
changing C14 needs a status the schema allows (a delegated decision is not a page-cited resolution; `partial` fits). Add each new id to `data/expected/discrepancies.ids`.

## Shared Patterns

### SP1. Step script prologue and epilogue (every new step)
**Source:** `tools/check/steps/tests-inventory.sh` L20-30 and L168-175; `steps/c-build.sh` L10-19.
```sh
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/<name>.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
find="$tmp/findings"; : > "$find"
bad() { printf '%s %s\n' "$1" "$2" >> "$find"; }
...
if [ -s "$find" ]; then
  sort -u "$find" | while IFS= read -r l; do printf -- '-- FAIL %s: %s\n' "${l%% *}" "${l#* }"; done
  exit 1
fi
printf -- '-- tests-inventory pass tests-expected=%s tests-executed=%s tests-skipped=%s tests-missing=0\n' ...
```
Apply to: `listing`, golden checker, `ci-log`, `important-gates`, `pkg-install` (c-build style variant with immediate `bad()`). Header comment lists Tokens and Stats; no `.sh` step may print a pass line when any finding exists.

### SP2. Producer/judge split with nonce (anything the control runner must drive)
**Source:** `c-tests.sh` + `tests-inventory.sh`; `c-faults.sh` + `faults-verdict.sh`. The producer writes results under `$IVS_GATE_DIR` with the nonce from `<gate>/NONCE`; the pure judge reads only the results dir and
`$ROOT/data/expected/`, defaulting `gate=${IVS_GATE_DIR:-$ROOT/build/gate}` (the control runner points it at the control tree's own `build/gate`). Apply to: any "mutant" results (DISCUSS-ci E4: `mutant <name> failed-with <token>` lines under
`build/gate/<step>/` with the nonce), the dev/release runner report comparison, and the golden checker.

### SP3. Test-id and skip policy
**Source:** `data/expected/tests/headers.ids` (header comment, `skip-ok` second token), `tests-inventory.sh` L45 and L147. A skip is visible (`ok N id # SKIP reason`), counted (`tests-skipped=`), and only legal when the id is `skip-ok`.
`needs-user-exec` style scenarios (D-20) are the intended skip-ok users; `api-effects.cmake` L237 treats a row whose cited tests are ALL `skip-ok` as untested.

### SP4. Refusal oracle (API calls)
**Source:** `tests/machine/machine_refuse.c` L1-60 and `api-effects.json` effect `none` definition (L7): whole-storage memcmp equal, digest equal where the handle is valid, only listed out parameters written, nothing allocated. Apply to every refusal of
run, peek, cpu_state and the input validation; poison outs with `0xA5` and compare.

### SP5. No host dependence in core code
**Source:** `core/src/common/ivs_prelude.h` (only stdint/stddef/limits/stdbool/string; `#pragma GCC poison float double`), `tools/check/steps/c-purity.sh` header L1-60. The run/observe/input code and the digest chains add no clock, file, env, allocation, float, or static. Time exists only as
`im->time_ticks`.

### SP6. Text-only control trees
**Source:** `run-controls.sh` L73-131 (`apply_make` verbs) and L133-145 (`build_tree`). Bad bytes are produced at run time; nothing binary or NUL ever sits in `controls/`. New controls overlay text files only.

## No Analog Found

| File | Role | Data Flow | Reason / fallback |
|---|---|---|---|
| `cmake/ivsConfig.cmake` and all `install()`/`export` rules | config | build | No install rules exist. Use DISCUSS-pkg section 4.1: hand-written two-line config including `ivsTargets.cmake` via `CMAKE_CURRENT_LIST_DIR`, `write_basic_package_version_file(... SameMinorVersion ARCH_INDEPENDENT)`, no `export(PACKAGE)`, `install(CODE ... FATAL_ERROR)` under `IVS_SAN`. |
| `runner/hostio.c` | utility | file-I/O | First POSIX code in the repo. Only precedent for bounded file reads is `tests/harness/ivs_test.c` (stdio). `_POSIX_C_SOURCE` before includes; `signal()`, `clock_gettime(CLOCK_MONOTONIC)`, `stat`. |
| `tools/pack` as a CMake-built tool (outside core/tests) | tool | file-I/O | No tool target exists. Build as an executable linked to `ivs_core` + `ivs_core_internal` for `ivs_sha256` (as tests do), or copy the trailer hash; writes only to the build tree. |
| `tools/check/ci-summary.sh` | script | transform | No `$GITHUB_STEP_SUMMARY` writer or EXIT-trap helper exists. Keep it tokens-only, backticks stripped from paths, `|| true` (DISCUSS-ci E3). It is a helper, not a step, and `run-controls.sh` L170-173 refuses a control whose `CHECK` is not `steps/<name>.sh` (`control-unknown-step`), so controls cannot drive it directly: give it a thin selftest step modelled on `steps/runner-selftest.sh` (synthetic temp tree, run the real helper, require the intended tokens, `Stats: cases=<n>`), as 04-RESEARCH.md mechanism 4 also concludes. |
| `tools/ci-record.sh` (the `gh` calls) | script | batch | No script uses `gh`. Style from `tools/gate-linux.sh` only. |
| out-of-tree consumer CMake project | test | build | No `find_package` consumer exists. Minimal idiomatic project per DISCUSS-pkg 4.2. |
| `*.ivss` scenarios, `*.golden` files | data | n/a | No tracked scenario or golden files exist; header idea from `data/memmap/*.literal.txt`. Naming and location are open (hazard 14). |
| Slicing/INPUT-chain independent oracle | test | transform | `cmake/IvsShaVectors.cmake` oracles text messages only; a binary chain needs authoring-time `printf` + `cmake -E sha256sum` literals (hazard 11). |

## Hazards (ranked; each is a thing the planner can get wrong)

1. **Docs trip three text gates** (`tools/check/steps/text.sh` L36-40, `steps/glossary.sh` L52-60, `steps/denylist.sh`). In README.md and docs/**/*.md a line with `supports|compatible|passes|accurate|verified|runs` needs `[claim:CL-NNNN]` or `[nc]`;
   a bare word `clock`/`clocks` (also "wall clock", "monotonic clock", which runner docs will want) needs `input clock`, `STIC clock`, `PSG chip clock` or `[nc]`; `jzintv` needs `[cite-only]` on the line and `Zbiciak` is never allowed (the D-25 rationale for rejecting as1600
   cites that project). `.planning/` is exempt from `text` and `denylist` but not from `hygiene`.
2. **Fault framework is core-only in five places** (faults.cmake owner scan and plain copies, `faults-verdict` site scan, `c-faults` binary path assumption `build/dev/tests/ivs_fault_<ID>`, `c-build` unlisted-source, `_good/tree`). D-24 must land as its own plan before any runner fault is registered, with controls.
3. **`api-effects` coupling**: new `IVS_API` function without a JSON call, a JSON call without a header line, a cited test id absent from `data/expected/tests/*.ids` or skip-ok, two refusal rows with the same status in one call, and a precedence/row mismatch are all gate failures. Stop reasons, input bits and `IVS_REASON_*` have no checker today (B5).
4. **Digest domains going live break Phase 3 artifacts**: `machine.refuse.digest.domain-16` (machine_digest.c L529, machine.ids L113, api-effects.json L304 text), the header comment, `IVS_DIGEST_RESERVED_*` names, `IVS_MACHINE_BYTES`/layout comment/`_Static_assert`, `tail_padding` assumption, and the `d <= IVS_DIGEST_INSTANCE` loops. Composite must stay over 1..4 and a Phase 3 composite literal should be pinned FIRST (D-10): the existing `md_fresh_composite` literal covers a FRESH instance only (04-RESEARCH.md summary (a)); pin a stepped-state set
(loaded + reset + N units) from the unmodified Phase 3 code before touching the table. In `machine_digest.c` the domain loops and per-domain arrays are hard-coded to 1..4 (grep `IVS_DIGEST_INSTANCE`, `4u`, `per_domain`);
for domains 16..18 a flipped row must change only that domain and NOT the composite, so the sensitivity helper needs the inverse assertion.
5. **`ivs_info` growth versus the struct rule**: `machine.c` L245 refuses a caller struct smaller than the current `sizeof(ivs_info)`; header L7-14 says out structs below the "v1 size" are BAD_ARG. Appending (D-06) silently changes that minimum.
6. **Suite plumbing is triplicated**: ctest name, `data/expected/tests/<suite>.ids`, TAP header suite; every new family `.c` must be listed in BOTH `add_executable` and `ivs_fault_suite(... SOURCES ...)`; `machine_main.c` caps (512 cases, 8 sources); ids sorted by habit. `tests-inventory` fails `tests-ctest-drift` if `ctest -N` differs from the ids stems.
7. **Control machinery cost and shape**: every control copies `_good/tree` (tree growth multiplies); each new step with controls must PASS on `_good/tree` (toy listing, toy golden, toy TSV, toy important-gates, toy runner source); `pkg-install` cannot be control-driven (build) so it uses in-step self-tests (D-40) and `important-gates` lists them as `mutant`.
8. **`pkg-install` correctness traps**: absolute include path at `core/CMakeLists.txt` L61; `headers-tap.sh` takes exactly one argument and hard-codes `core/include`; a temp dir reached through a symlink (compare with `pwd -P`); `ivsTargets-release.cmake` inventory depends on the Release preset; `IVS_SAN` trees must refuse install; dash has no `local`/`[[`; `nm` formats differ (reuse `core-symbols.sh` `norm`); `ninja -t deps` must be asserted non-empty.
9. **dev-vs-release comparison inside ctest** may find no sibling tree (developer loop). Use skip-ok with a visible skip or a pure judge step over both presets' result artifacts (memmap-dump precedent).
10. **HLT/TSTR semantics** (D-29): HLT stops only after the next instruction and only at an interruptible boundary; zero fill is not a trap; CPU HALTED idles 1 cycle (4 ticks) per unit; a faulted CPU returns `elapsed_cpu_cycles = 0` forever (no-progress guard in the run loop).
11. **Binary chain oracle**: `string(SHA256)` and `file(WRITE)` cannot carry NUL or arbitrary bytes; the INPUT chain's `h(n)` is raw. D-16's "independent cmake-sha256 oracle" must be authoring-time literals produced with `printf` octal escapes + `cmake -E sha256sum` (as `machine_digest.c` does), or a text-safe preimage.
12. **Strict C17 hides POSIX names** (`CMAKE_C_EXTENSIONS OFF`, root L9): `_POSIX_C_SOURCE` before the first include in `hostio.c` only; hosted Ubuntu gcc 13 and Apple clang differ (two Phase 3 escapes were gcc/Linux-only; run `sh tools/gate-linux.sh` before the first runner push). Warnings are `-Werror` in both presets (`ivs_warnings`: `-Wconversion -Wsign-conversion -Wshadow -Wmissing-prototypes -Wcast-qual ...`), including for runner code.
13. **Integrity coverage gaps**: SPDX is enforced only for the directories in `integrity.cmake` L74-80, ASCII-only only under `data|fixtures|tools|.github|core|include|tests|cmake` and root `CMakeLists.txt`; `runner/` and `tools/pack/*.c` get neither. Add the header anyway, or extend the regex (a one-line change with a control). Also: no empty tracked file (`integrity-empty-file`), CRLF/BOM banned, path charset `[A-Za-z0-9._/-]`.
14. **Naming collisions to settle before planning** (04-RESEARCH.md "Recommended structure" proposes: scenarios `data/scenarios/*.ivss`, goldens `data/golden/*.{expect,digests}`, a NEW public header
    `core/include/ivs/ivs_run.h` included by `ivs.h`, core TUs `machine_bound.c`/`machine_input.c`/`machine_inspect.c`, runner `scenario.c exec.c report.c outcome.c hostio.c main.c`, suites `run` and `runner`, steps `listing golden pkg-install
    important-gates ci-log ci-summary runner-scan`; it is not final until the planner adopts it. A new public header adds ids `headers.c17-ivs-run` and `headers.cxx-ivs-run skip-ok` to `data/expected/tests/headers.ids`, a `header` value in api-effects.json,
    one more line in `pkg-install-files.ids`, and must keep one-line `IVS_API` prototypes. This file's names are placeholders where it says "name free"). Still open: goldens (`*.golden` per task and D-31 vs `*.expect` + `*.digests` per D-11 vs `data/golden/runner/<id>.report` in DISCUSS-runner); scenario file location; step names for the listing and golden checkers; peek signature (D-08 `uint16_t addr`, no `out_cap`, versus DISCUSS-diag `uint32_t addr, count, out, out_cap`; CONTEXT governs, but note `count` up to 65536 and `BUFFER_TOO_SMALL` has no source without `out_cap`); `ivs_cpu_state_report` (CONTEXT) vs `ivs_cpu_state` (DISCUSS); CONTEXT D-41 says `timeout-minutes` 5, the file says 10.
15. **Fixture rights guard**: every file in `fixtures/<dir>/` must be a manifest `outputs` entry with current hash/size; `data/expected/fixtures.ids` exact; listing edits require the manifest refresh in the same commit; `.ivsc`/`.bin` are gitignored and `binary-ext`, so a stub image can never be tracked by accident, but a golden or listing that embeds image bytes as text can; D-30 says only identity hashes appear.
16. **ci-parity is line-based awk**: new tokens must keep the `<token> <file>:<line>` emit form; the allowlist must not change the token of existing unpinned/extra-workflow controls (`ci-unpinned-action` only); controls overlay whole workflow files.
17. **Version source**: `ivs.h` spells `IVS_API_VERSION_MAJOR 0u`; the CMake parse must accept the `u`. `project(ivs VERSION ...)` must come after the parse; the consumer ties `ivs_VERSION`, `IVS_API_VERSION`, `ivs_api_version()`.
18. **Machine suite growth**: 381 ids today; each named fault runs the entire suite once (c-faults 4 lanes). Adding many core faults to the machine suite lengthens every fault run; a dedicated `run` suite keeps cost bounded (D-45 budget: hosted median 40 s).

## Metadata

**Analog search scope:** `core/`, `tests/`, `tools/`, `cmake/`, `data/`, `fixtures/`, `docs/`, `.github/`, `CMakeLists.txt`, `CMakePresets.json`, `.planning/{CI-EFFICIENCY,AUTONOMY}.md`, Phase 4 CONTEXT and DISCUSS files.
**Files read for excerpts:** about 55 (gate.sh, gate-linux.sh, gate.yml, all four C-pipeline steps, faults-verdict, tests-inventory, api-effects.{cmake,sh,json}, ci-parity, run-controls, headers-tap, core-symbols{,-tap}, vectors.cmake, lib.cmake,
integrity.cmake, files.cmake, manifests, machine.{c,h}, machine_run.c, machine_load.c, machine_digest.c, ivsc.{c,h}, memsys.h, machine test files, harness, profile JSON, claims/discrepancies, kinds hooks, docs).
**Not run:** no build, no gate, no `gh`, no network. Statements about gate behaviour are from reading the scripts at this commit.
**Pattern extraction date:** 2026-11-11
