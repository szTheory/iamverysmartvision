# Phase 3: Memory Map, Content Loading and Refusal - Research

**Researched:** 2026-11-10
**Domain:** pure C17 memory-system and loader on top of the Phase 2 CPU stepper; CMake/ctest/sh gate extension (faults loop, purity, generators, effect table)
**Confidence:** MEDIUM-HIGH (gate mechanics read and probed HIGH; hosted-runner timing, one nm behaviour on Ubuntu PIE, and three internal inconsistencies of the locked decisions listed in Open Questions are MEDIUM)

Scope note: the design is settled by D-01..D-38 in `03-CONTEXT.md` and the five `03-DISCUSS-*.md` option files. This file adds only what reduces planning risk: which Phase 1/2 mechanisms must change and where (file:line), a plan decomposition, codebase-specific hazards, the validation map, and the data-file edits. No web research was done; no source of another emulator was opened. Probes were run this session on this host (Apple clang 21, CMake 4.4.3, Ninja 1.13.2) and in a `gcc:13` container (GCC 13.5.0) to close two named [UNK] items (Linux `nm` letters; libc calls the compilers emit).

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
D-01 through D-38 in `.planning/phases/03-memory-map-content-loading-and-refusal/03-CONTEXT.md` `<decisions>` are locked and incorporated by reference (a verbatim copy of ~4k words would drift; the planner reads that file first). One-line index:
- Lifecycle and API (API-01, API-06): D-01 caller-provided storage and opaque handle; D-02 one flat pointer-free instance struct; D-03 create options and profile registry; D-04 `struct_size` rule; D-05 ten public functions, internal step seam; D-06 headers in `core/include/ivs/`, static library only; D-07 thread-safety and the nm writable-data check; D-08 a refused call writes only its out parameters.
- Status and refusal (MEM-04, API-06): D-09 frozen status numbers (-1..-5, -16..-23, -24..-31 reserved), pinned by `data/expected/api-statuses.ids`; D-10 validate-then-commit; D-11 fixed precedence BAD_ARG, BAD_HANDLE, BAD_STATE, DUPLICATE_LOAD, OVERSIZED, MALFORMED, OUT_OF_RANGE, OVERLAP, DIGEST_MISMATCH; D-12 every refusal has effect `none`; D-13 loads and unloads only while OFF, order-free, one image per slot, no replace; D-14 MISSING_SYSTEM_IMAGE is refused by power-on reset; D-15 `data/api/api-effects.json` plus a cmake -P step, new `machine` suite.
- Memory map (MEM-01, MEM-02): D-16 descriptors are profile fields and a fail-closed generator; D-17 one width formula; D-18 region table; D-19 gates and stubs; D-20 one bus function, `cart_page[]` the only mutable decode state; D-21 literal map probed over 65,536 addresses; D-22 boundary probes from literal rows; D-23 C60, C61, C62, C21, C51, `profile-fields.ids`.
- Content (MEM-03): D-24 D5 user-supplied only plus a Phase 4 boot stub; D-25 `ivs_machine_load(m, kind, bytes, len, opts, &report)` copy-in and per-slot report; D-26 raw system-image container (big-endian words, EXEC 8192 bytes, GROM 2048 bytes); D-27 identity is a recorded fact, optional `expect_sha256`; D-28 in-repo SHA-256 with FIPS vectors, `cmake -E sha256sum` oracle, six faults.
- Cartridge (MEM-05): D-29 IVSC v1 in `docs/formats/ivsc-v1.md`; D-30 layout, big-endian; D-31 limits and refuse-everything-undefined; D-32 `required_features`; D-33 pure zero-copy parser and fixed check order; D-34 text fixtures, no binary committed.
- Digest and tests (API-06, SC3, SC5): D-35 per-domain SHA-256 state digest, big-endian; D-36 isolation tests; D-37 generalize the fault loop over `data/expected/faults/*.ids`; D-38 gate cost, budget re-baselined 30 s to 40 s hosted, revisit trigger 50 s.

### Claude's Discretion (verbatim)
Plan ordering, exact struct layouts, the names of internal helpers, the final fixture and fault id lists, TAP suite splitting, and which pure-sh vs cmake -P form each new gate step takes (prefer cmake -P for JSON).

### Deferred Ideas (OUT OF SCOPE, verbatim)
- Allocating create convenience; `ivs_machine_run`, peek/poke; export/visibility check for shared libs (Phase 4+).
- `.rom` / `.bin+.cfg` importers (later phase, own rights review); bundle atomic `load_set`.
- Real GROM/STIC/PSG device behaviour; D10 measurements resolving C20/C21/C60-C62.
- Opt-in masking flag if real dumps carry stray top bits (D11).
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| MEM-01 | Memory map generated from profile descriptors; decode dump matches the sourced literal map | Generator pattern `cmake/IvsVectorTable.cmake`; produce/consume split for the dump (Pitfall 6); c-purity cannot see build-tree `.inc` (Pitfall 1) |
| MEM-02 | 10-bit ROM and 8/16-bit RAM masking and truncation tested at each boundary | Rule-table probes from literal rows; ROM mask is unobservable through the loader (Pitfall 7), needs an internal raw-poke seam |
| MEM-03 | Typed content loaded as separate inputs; length and digest reported | Reported digest is over the whole supplied file, not the IVSC trailer (Pitfall 8); streaming SHA-256 copy trick |
| MEM-04 | Typed refusals, prior state unchanged (refusal matrix) | Precedence matrix, whole-storage memcmp oracle, validate-then-commit with `const ivs_machine *` |
| MEM-05 | One documented cartridge format with positive, boundary, malformed fixtures | IVSC v1 spec; segment alignment and header-size gaps (Open Questions OQ-1, OQ-2); manifest digest pinning (Pitfall 9) |
| API-01 | Create, reset, unload, destroy independent instances; isolation test | Flat instance (about 110 KB [INF]), storage alignment, interleaved and relocation tests, writable-data check with `-fno-pie` (Pitfall 3) |
| API-06 | Every error/refusal has a documented, tested state effect | `data/api/api-effects.json` and a static `api-effects` step; status pin; effect-table residual (OQ-5) |
</phase_requirements>

## Project Constraints (from CLAUDE.md / AGENTS.md / AUTONOMY.md)

- Prefix `ivs_`; never "intellivision" in package, product or repo names (hard stop). C17 + CMake (D1 delegated). No copying from MAME, jzIntv or libasm; behaviour agreement with another emulator is not evidence. "Another copy-paste is better than another dependency" (AUTONOMY section 2): SHA-256, the TAP harness, the JSON generators are all in-repo.
- Rights: no committed manuals, page images, ROMs or system images; `refs/` is gitignored; a hash is identity, not rights. Phase 3 tests use generated stand-in bytes only; IVSC fixtures are text JSON, no `.ivsc/.rom/.bin` is tracked (D-34).
- Shift-left: nothing in this phase is handed to the owner except D10 (hardware measurement) and D11 (real EXEC/GROM byte-order check), both non-blocking and already outside the milestone's automated exit.
- Gate is `sh tools/gate.sh` (the single local and CI command); measure before changing CI shape; poll CI from the orchestrator only (agents never call `gh`).
- Label observations vs inference; a green test is not a hardware claim: MEM-01 claim tier is `source-expectation`.
- Claims in `docs/**/*.md` and `README.md`: the words supports, compatible, passes, accurate, verified, runs need `[claim:CL-NNNN]` or `[nc]` on the same line (`tools/check/steps/text.sh:30-40`); `docs/formats/ivsc-v1.md` must obey it. The names on `tools/check/denylist.txt` (author names, `jzintv`, a BSD licence header string) must not appear in any tracked file outside `.planning/`, `.claude/`, `docs/provenance/`, `tools/check/`, `data/sources.json` (`tools/check/steps/denylist.sh:6-9`, `:41-43`).

## Summary

The Phase 2 gate is already a general mechanism with exactly four single-assumption seams that Phase 3 must open: the fault loop (`c-faults`, `faults-verdict`, `tests/faults.cmake` all hard-code `faults/cpu.ids` and the `cpu-vectors` executable), the purity step (it only understands `core/src/**` and a compile with `-I core/src`, so neither a public header under `core/include/` nor a build-tree generated `.inc` can pass it), the symbol check (no writable-data rule; on an Ubuntu default-PIE compile a const table with pointers is `d`, so a naive rule false-positives on the existing CPU decode table), and the `_good/tree` positive-control coupling (every new step and every new inventory file must land together with its fixtures in the toy tree, or `controls=` goes red). Everything else (new steps, ids inventories, TAP suites, cmake -P generators, controls) follows patterns that exist and are copy-able.

Three more findings change what the planner writes. (1) The locked decisions contain three small internal inconsistencies that must be resolved in the first plan that touches them, not discovered at execution: a 1-word IVSC segment versus the 256-word cartridge alignment (OQ-1), the "24-byte header plus padding to 24" wording (OQ-2), and the IVSC trailer digest versus the D-25 "SHA-256 over the exact supplied container bytes" report (OQ-3). (2) The MEM-01 dump cannot be both behavioural and complete: behaviour cannot tell `impl=stub` from `undriven`, and loaded ROM can never hold a wide word, so a ROM mask fault is invisible through the loader; both need explicit seams (OQ-4, Pitfall 7). (3) Gate cost: baseline measured today is 46 s local wall (load about 9), 24 steps, 192 controls, 67 fault runs; the CONTEXT estimate of +2 to 4 s hosted is optimistic for about 40 more fault executables each running a full suite, so plan 03-01 should land the parallel fault run lever and measure after 03-02.

**Primary recommendation:** ten plans in nine waves. Wave 1 is gate infrastructure only (fault loop, purity, header move, controls) with no behaviour change; then SHA-256 as the first real second fault file; then memory system (MEM-01) in parallel with the IVSC parser (MEM-05); then width and boundary probes (MEM-02); then instance lifecycle and digest; then typed load and the refusal matrix; then reset, step seam, isolation and the writable-data rule; then the effect table and robustness sweeps; then close-out. The four choke points are 03-01, 03-03, 03-06 and 03-07.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Region table, width formula, decode, stubs, gates | Core `core/src/memmap/` | Generator output in build tree | Pure function of (profile constants, instance state); no host (D-19, D-20) |
| Profile registry, descriptors, IAB vectors, power-on fill | `data/profiles/*.json` | `cmake/IvsMemmapTable.cmake` | Single source of truth with per-field tag and cite (D-16) |
| Literal map and decode dump comparison | `data/memmap/*.literal.txt` plus gate step `memmap` | Test suite `memmap` writes the dump | Independence rule: literal never reads the profile (D-21) |
| SHA-256 and canonical state digest | Core `core/src/common/ivs_sha256.c`, `core/src/machine/machine_digest.c` | `cmake -E sha256sum` and `string(SHA256)` as oracle | Core reports digests; oracle must be a different implementation (D-28, D-35) |
| IVSC parse (layer 1) | Core `core/src/content/ivsc.c` (pure, zero-copy) | Fixtures in `data/cart/` | Malformed-input refusal belongs in the refusal matrix (D-33) |
| Window check, validate, commit (layer 2) | Core `core/src/machine/machine_load.c` | Profile `memmap.cart_window` | Profile-dependent; atomic copy-in (D-10, D-25) |
| Public API surface | `core/include/ivs/*.h` | `data/api/api-effects.json`, step `api-effects` | Effects are data, checked by a step (D-15) |
| File reading, allocation, wall time | Host (Phase 4 runner, tests) | - | Core takes bytes only (D-01, D-25) |

## Standard Stack

No external package is installed by this phase (copy-paste over dependency). Packages: none, so the Package Legitimacy Audit is not applicable.

| Tool | Local (this host) | Hosted ubuntu-24.04 | Note |
|------|-------------------|---------------------|------|
| CMake | 4.4.3 [VERIFIED: `cmake --version`] | 3.31.6 [CITED: `02-RESEARCH.md` Standard Stack, runner-images Readme] | Floor 3.25; `string(JSON)`, `string(SHA256)`, `file(SHA256)` are available in the whole range |
| Ninja | 1.13.2 [VERIFIED: `ninja --version`] | 1.13.2 [CITED: same] | |
| C compiler | Apple clang 21.0.0 [VERIFIED] | gcc 13 [CITED: same] | GCC 13.5.0 probed here via the `gcc:13` image (not the Ubuntu build; see A1) |
| nm / ar | llvm-nm | binutils 2.42 [CITED] | Letters differ; probed below |
| sh / awk | macOS sh, dash present | dash 0.5.12, mawk [CITED/ASSUMED] | New scripts must be POSIX-only; run once under `dash` |
| docker | present locally [VERIFIED: `docker images` lists `gcc:13`, `ubuntu:24.04`] | n/a | Local-only aid for Linux probes; never a gate dependency |

**Independent SHA-256 oracles available at zero dependency cost:** `cmake -E sha256sum` (already used by `tools/check/steps/tests-inventory.sh:151-168`), CMake `file(SHA256)`, CMake `string(SHA256)`. Recommended use: a committed literal table (`data/sha256/vectors.txt`, authoring-time values) that a generator re-derives with `string(SHA256)` on every build and fails closed on a mismatch, so the oracle is exercised every run rather than once at authoring [INF].

## Architecture Patterns

### System flow (conceptual)

```
profile JSON --(cmake -P IvsMemmapTable)--> build/<preset>/core/gen/ivs_memmap.inc
   (descriptors, IAB vectors,                 (const regions, rd_region[4096], wr_region[4096],
    fill, features, slots)                     profile registry, instance sizes)
                                                      |
host bytes --> ivs_machine_load(m, kind, bytes, len) -+-> validate (const m, stack plan) -> commit (void memcpy)
   EXEC 8192 B / GROM 2048 B / IVSC container --------+        |
                                                      |        v
 ivs_machine_create(storage) -> flat instance { magic, profile idx, phase OFF|POWERED, time, cpu[32B],
   RAM/GRAM/scratch, STIC/PSG stub state, slot records, EXEC[4096] GROM[2048] cart pool[49152], cart_page[] }
                                                      |
 ivs_machine_reset(POWER_ON): required slots present? -+-no-> MISSING_SYSTEM_IMAGE (effect none)
                                                      +-yes-> volatile fill, ivs_cp1610_reset, OFF->POWERED
 ivs_machine_step_unit_internal -> ivs_cp1610_step(port=machine bus) -> bus fn: rd_region/wr_region -> device switch
 ivs_machine_state_digest -> SHA-256 per domain (CPU, MEM, IMAGES, INSTANCE) -> composite
 tests/gate: memmap suite writes dump -> step `memmap` compares with literal; api-effects step checks table
```

### Recommended structure
```
core/include/ivs/{ivs.h, ivs_status.h, ivs_machine.h}      public (standard headers only, extern "C", IVS_API)
core/src/common/{ivs_prelude.h, ivs_sha256.h, ivs_sha256.c}
core/src/content/{ivsc.h, ivsc.c}                          layer 1 parser
core/src/memmap/{memsys.h, memsys.c}                       decode, width formula, stubs, gates, install primitives
core/src/machine/{machine.h, machine_create.c, machine_load.c, machine_reset.c, machine_digest.c, machine_status.c}
cmake/{IvsMemmapTable.cmake, IvsMemmapProbes.cmake, IvsCartTable.cmake, IvsShaVectors.cmake}
data/{memmap/*.literal.txt, cart/*.json, api/api-effects.json, sha256/vectors.txt}
data/expected/{api-statuses.ids, api-calls.ids, cart-fixture-files.ids, faults/{sha256,memmap,ivsc,machine}.ids,
               tests/{machine,memmap,ivsc}.ids, gate-steps/machine.ids, controls/{memmap,api-effects}.ids}
docs/formats/ivsc-v1.md ; fixtures/FX-ivsc-vectors/manifest.json
tests/{machine,memmap,ivsc}/ ; tools/check/{steps/memmap.sh, steps/api-effects.sh, memmap.cmake, api-effects.cmake, core-generated.txt}
```
Core `.c` stems must stay unique across directories because `core/CMakeLists.txt` names objects by `NAME_WE` (`ivs_core_obj_<stem>`, lines 13-17) and `tests/faults.cmake` names plain copies the same way (lines 24-29); keep each core TU small so a per-fault owner recompile stays cheap [INF].

### Pattern 1: validate-then-commit
`validate(const ivs_machine *, request, plan *)` is pure and fills a small stack plan (kinds, lengths, indices; never copies of image bytes, because the instance is about 110 KB [INF] and the plan lives on the stack). `commit(ivs_machine *, plan *, bytes)` returns `void`, has no early return and no loop bounded by unchecked input. The 03-03 plan writes the commit primitives (`install_exec/grom/cart_segment`) first, as internal functions the probe suite uses, so 03-07 only adds the validate half and the API.

### Pattern 2: produce/consume split for gate evidence
Producers are ctest suites that write files into `$IVS_TEST_OUT_DIR` (set per preset by `c-tests.sh`); consumers are pure text steps that read only `build/gate/` and `data/expected/` so the control runner can drive them with overlays. This is exactly `c-faults` to `faults-verdict` and `c-tests` to `tests-inventory`. Do the same for the decode dump (`memmap-dump.txt` with a `# nonce=` header) and keep both new steps pure.

### Pattern 3: fail-closed generator
Copy the structure of `cmake/IvsVectorTable.cmake`: `vt_fail` is `message(FATAL_ERROR ...)` with a context prefix, `vt_keys` rejects unknown keys, `vt_hex` validates before `math(EXPR)`, `-DMUTATE` for named-fault controls (lines 11-14, 471-476, 690).

### Anti-patterns
- Hand-copying the region table into C and "verifying" it against itself (D-16 forbids it).
- A generated file read by a core TU that no gate step can compile (Pitfall 1).
- A `memcmp`-style identity for refusals that skips padding (use whole-storage `memcmp`, D-12).
- `strlen`, `strcmp` or an unbounded scan loop in the core: the compiler turns them into libc imports (Pitfall 4).

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| JSON in the build | A C JSON parser | `cmake -P` generators with `string(JSON ...)` | D-14 precedent; no parser in the core |
| Independent SHA-256 expected values | A second C SHA-256 | `cmake -E sha256sum`, `string(SHA256)`, `file(SHA256)` | Different implementation, zero dependency |
| Test runner, inventory, nonce, stale detection | A new harness | `tests/harness/ivs_test.[ch]` and `tests-inventory` | Exact id sets, plan and trailer counts exist (`tests-inventory.sh:34-148`) |
| Negative-control runner | New control mechanism | `tools/check/run-controls.sh` overlays on `_good/tree` | 4-lane runner, inventory-exact (`run-controls.sh:231-240`) |
| Hex/endian packing | Struct casts of byte buffers | Shifts on `uint8_t` after widening: `(uint32_t)b[0] << 24` | `uint8_t` promotes to `int`, so `b[0] << 24` is signed-overflow UB; casts of `const uint8_t *` to `uint16_t *` break alignment and aliasing |
| Fixture binaries | Committed `.ivsc` files | Text JSON plus build-tree table (D-34) | `files` and `hygiene` steps ban unlisted binaries |

**Key insight:** every recurring failure class here (a stale inventory, a mutation before refusal, a drifted table, a forgotten digest field) already has a mechanical detector pattern in the repo; Phase 3's job is to instantiate it, not invent one.

## Section A: existing mechanisms Phase 3 must extend (requested item 1)

| # | Mechanism | Where (file:line) | Today | Phase 3 change |
|---|-----------|-------------------|-------|----------------|
| A1 | Step registration | `tools/gate.sh:65-80` | Step scripts in `tools/check/steps/*.sh` must equal the union of `data/expected/gate-steps/*.ids`; a missing or extra name is `gate-step-missing` or `gate-step-extra` | Add `memmap` and `api-effects` as `tools/check/steps/<name>.sh` plus `data/expected/gate-steps/machine.ids` in the same plan. Steps run in sorted order, so `memmap` (m) runs after `c-tests` (c), good for a results consumer; `api-effects` (a) runs before `c-build`, so keep it a static check (data, headers, ids only) |
| A2 | Pass-line contract | `tools/gate.sh` (`-- <name> pass <stats>`) | A step without its pass line is `gate-step-no-pass-line` | Each new step prints `-- memmap pass ...` and `-- api-effects pass ...` |
| A3 | Test suites and inventories | `tests/CMakeLists.txt:38-73`, `tests-inventory.sh:34-148` | One ctest test per suite, name equals `data/expected/tests/<suite>.ids` stem and TAP file stem; `ctest -N` must equal the suite set (`tests-ctest-drift`) | New suites `machine`, `memmap`, `ivsc`: each `add_executable`, `add_test(NAME <suite>)`, `SKIP_RETURN_CODE 77`, plus the ids file, in one plan. The `machine` suite aggregates several case tables (sha256, refuse, iso, digest, sweep) in one executable per D-15 |
| A4 | Core source list | `core/CMakeLists.txt:7-10` | Explicit list, no glob; `c-build` fails on a tracked `core/**/*.c` not compiled (`c-build.sh:50-56`) | Append `common/ivs_sha256.c`, `content/ivsc.c`, `memmap/memsys.c`, `machine/*.c`; every new TU also reaches `ivs_core` and the fault plain copies automatically (`faults.cmake:24-29`) |
| A5 | Include path | `core/CMakeLists.txt:17,25` (`core/src` PRIVATE per object, PUBLIC on `ivs_core`) | Tests and fault exes reach `cpu/cp1610.h` through the PUBLIC dir | Add `core/include` to every object, to `ivs_core` PUBLIC, and a separate internal interface (`core/src`) for tests so Phase 4's runner sees only `ivs/ivs.h`. Four places in `faults.cmake` list include dirs explicitly (lines 28, 33-34, 44, 54-56) and each needs `core/include` and the generated dir |
| A6 | Status header | `core/src/common/ivs_status.h:7-11` (`typedef int ivs_status;`, `#define IVS_OK 0`, `#define IVS_E_BAD_ARG (-1)`, `#define IVS_E_BAD_STATE (-2)`); sole includer `core/src/cpu/cp1610.h:17` (`#include "common/ivs_status.h"`) | Internal header | Move to `core/include/ivs/ivs_status.h`, edit the one include, append D-09 codes. 26 test lines use `IVS_E_*` through `cp1610.h` and need no edit |
| A7 | Purity step, source layer | `tools/check/steps/c-purity.sh:46-47` (only `^core/src/.*[.]c$` and `.h$`), `:96` (quoted include must be a FILELIST header at `core/src/<h>` or beside the includer), `:159` (compile with `-I"$ROOT/core/src"`) | A public header under `core/include` is neither scanned nor resolvable; `#include "ivs/ivs_status.h"` fails `c-purity-include` | Scan `^core/(src\|include)/.*[.]h$`; resolve quoted includes also at `core/include/<h>`; compile with `-I"$ROOT/core/include"`. A nonexistent `-I` dir is silently ignored, so the `_good/tree` (no `core/include`) still passes |
| A8 | Purity vs generated include | same `:96`, `:159` | The step compiles tracked sources only | A core TU that includes the build-tree `ivs_memmap.inc` fails stage 1 (not a tracked header) and, if bypassed, stage 2 (file not found). Fix: `tools/check/core-generated.txt` (reviewed basenames) accepted by the include rule; when `$ROOT/cmake/IvsMemmapTable.cmake` exists the step runs the generator into `$tmp/gen`, adds `-I$tmp/gen`, and also runs the source-layer scan on the generated file. The `_good/tree` has no generator, so the step skips that part there |
| A9 | Symbol allowlist | `tools/check/core-symbols-allow.txt` lines 7-25: `memcpy`, `memmove`, `memset`, `memcmp`, `__memcpy_chk`, `__memmove_chk`, `__memset_chk`, `__stack_chk_fail`, `__stack_chk_guard`, `bzero` | New TUs may import only these | SHA-256, copy-in, zero-fill and the digest compare need nothing else; `strlen`, `strcmp`, `bcmp`, `memchr` are not allowed (Pitfall 4) |
| A10 | Writable-data rule | `tools/check/core-symbols.sh:6-12`, `:72-75` (uses `nm -P -g --defined-only`) | Only imports are checked; D-21 of Phase 2 deferred mutable-static detection to Phase 3 | Add opt-in `--forbid-writable-data` (types `b B d D c C`, not `s`), used only by `c-purity` stage 2 on its own non-instrumented archives compiled with `-fno-pie`; token `core-symbols-writable`; not applied to the ctest `core-symbols` archive (ASan globals, PIE) |
| A11 | Fault loop, runner | `tools/check/steps/c-faults.sh:22` (`ids=.../faults/cpu.ids`), `:27` and `:29` (single suite header), `:62` (exe `build/dev/tests/ivs_fault_$id`), `:68` (result `faults/$id/$suite.txt`) | One ids file | Loop over `data/expected/faults/*.ids`, suite per file; ids must be unique across files because the exe name is `ivs_fault_<ID>`; new token `c-faults-run: duplicate id` or a verdict-side `fault-duplicate-id` |
| A12 | Fault loop, verdict | `tools/check/steps/faults-verdict.sh:32-47` (single file and suite), `:61-113` (per-fault checks against `$tmp/suite.ids`), `:119-133` (site scan over `core/src/**` against the one id list) | Single list | Per file: its suite and `data/expected/tests/<suite>.ids`; the orphan-site and missing-site checks must use the **union** of every file's non-GEN ids (a site named in `sha256.ids` is not an orphan of `cpu.ids`). Keep the awk single-pass if possible: this step is already 4 s at 67 faults (measured) |
| A13 | Fault loop, build | `tests/faults.cmake:13` (one ids file), `:53` (exe is always `cpu/cpu_vectors.c` + `ivs_vectors.inc`), `:72-77` (only `GEN_EXPECT_MUTATION` accepted), `:81-92` (owner TU found by scanning `IVS_CORE_SOURCES` for `IVS_FAULT_<ID>`) | One suite | Per-suite descriptor (sources, generated includes, extra include dirs) registered from `tests/CMakeLists.txt` and read here, so each fault exe recompiles the suite's own test sources with plain flags (no `ivs_san`, per the header comment lines 8-10); a dispatch table for `GEN_` ids (`GEN_EXPECT_MUTATION`, `GEN_MAP_MUTATION`); owner detection stays `.c`-only, so sites live in core `.c` files, never headers |
| A14 | `_good/tree` coupling | `tools/check/run-controls.sh:242-256` (every distinct CHECK step must pass on the pristine good tree); `tools/check/controls/_good/tree/data/expected/faults/cpu.ids`, `.../tests/toy.ids`, `.../build/gate/faults/GOOD_FAULT/toy.txt` | Toy suite `toy`, toy fault `GOOD_FAULT`, toy site in `core/src/cpu/good.c` | For the loop generalization add a **second faults file with the same toy suite** (`GOOD_FAULT2`, a second site in `good.c`, `build/gate/faults/GOOD_FAULT2/toy.txt`): no second suite means no change to the 12 `tests-inventory` controls. For `memmap` and `api-effects` add toy inputs (literal map, dump, JSON, headers, ids) so `_good/<step>` passes |
| A15 | Controls inventory | `run-controls.sh:231-240` | Each control dir must be listed in a `data/expected/controls/*.ids`; orphan or unlisted fails | New files `memmap.ids`, `api-effects.ids`; append to `faults-verdict.ids`, `core-purity.ids`; one plan adds dirs and ids together |
| A16 | Profile record | `tools/check/schema/profile.txt` (`keys value tag cite conflict resolving_evidence revisit_trigger decision note`; value `STRING\|NUMBER\|OBJECT\|ARRAY`; `unknown-default-chosen` needs non-blank resolver and trigger), `data/expected/profile-fields.ids` (exact set, order-independent: `records.cmake:313`) | 28 fields | About 25 new fields (Section E); region and device values are OBJECT; the generator, not the schema, validates their inner shape |
| A17 | Claims | `data/claims.json`, `data/expected/claims.ids` (CL-0001..CL-0005), `tools/check/kinds/claim.cmake` | `depends_on` names profile fields; a field tagged tier4, inference or unknown-default-chosen makes the claim provisional | Add CL-0006 for MEM-01 (tier `source-expectation`, standing `provisional`) and a claim for the IVSC fixtures; ids file in the same plan |
| A18 | Manifests | `docs/fixture-manifest.md` rules, `data/expected/fixtures.ids` (only `FX-sample-rights-unknown` today), `tools/check/manifests.cmake` | Committed output needs effective `yes`: licence evidence url plus text digest, rights review `author-attested`, sha256 and size of every listed output | `FX-ivsc-vectors` lists every `data/cart/*.json`; `_good/tree/fixtures/FX-good/manifest.json` is the shape to copy. Every later edit of a cart JSON must refresh its sha256 and size (Pitfall 9) |
| A19 | Text and SPDX rules | `tools/check/integrity.cmake` (ASCII only under `data/ fixtures/ tools/ core/ tests/ cmake/`; SPDX line in `core/tests/cmake/tools` `.c .h .sh .cmake`) | | Every new `.c/.h/.sh/.cmake` has `SPDX-License-Identifier: MIT`; `data/**/*.txt` and `.json` do not need it; no non-ASCII anywhere |

## Section B: proposed plan decomposition (requested item 2)

Waves are sequential unless marked parallel. Every plan ends gate-green (`sh tools/gate.sh` PASS with exact inventories) and adds its controls and ids in the same plan. Plans that share `core/CMakeLists.txt`, `tests/CMakeLists.txt` or `.planning/` bookkeeping can run in parallel worktrees only if the executor merges those two files by hand; the default recommendation is sequential.

| Plan | Wave | Title and vertical outcome | Requirements | Depends on |
|------|------|----------------------------|--------------|-----------|
| 03-01 | 1 | **Gate infrastructure, no behaviour change.** (a) Generalize `c-faults`, `faults-verdict`, `faults.cmake` over `data/expected/faults/*.ids` with per-file suite and a suite descriptor registry; bounded parallel fault execution (lanes like `run-controls.sh`). (b) `c-purity`: `core/include` scan and include resolution, `core-generated.txt`, `-fno-pie`, generator hook, optional `--forbid-writable-data` flag in `core-symbols.sh` (rule off until 03-08). (c) Move `ivs_status.h` to `core/include/ivs/`, add `core/include` and the internal include interface. (d) Controls: second-file orphan and missing-site, per-file suite header missing, duplicate id across files, `core-purity-public-include`, `core-purity-generated-unlisted`; `_good/tree` gains `faults/second.ids` + `GOOD_FAULT2`. | API-06 (infra), RUN-03-style gate | Phase 2 |
| 03-02 | 2 | **SHA-256 and the `machine` suite.** `core/src/common/ivs_sha256.{h,c}` (streaming, context copyable), `data/sha256/vectors.txt` + `cmake/IvsShaVectors.cmake` (re-derives every expected digest with `string(SHA256)`, fails closed), `tests/machine/` skeleton with `machine.sha256.*` ids (FIPS vectors, lengths 0,1,55,56,57,63,64,65,119,120,127,128,129,2048,8192, split-at-every-offset, 1,000,000 x "a"), `data/expected/tests/machine.ids`, `data/expected/faults/sha256.ids` with six faults (wrong K, wrong rotate, length little-endian, padding off by one at 55/56, final block dropped, init does not reset). First real use of the generalized loop. | MEM-03, API-06 (digest primitive) | 03-01 |
| 03-03 | 3 | **Memory system and MEM-01.** Profile fields (Section E), `cmake/IvsMemmapTable.cmake` (+`-DMUTATE`), `core/src/memmap/memsys.[ch]` (device switch, width formula, stubs, gates, `cart_page[]`, install primitives), literal map `data/memmap/<profile>.literal.txt` authored from sources, `tests/memmap/` suite writing `memmap-dump.txt` (65,536 addresses x 2 views through the real bus functions), step `memmap` (`tools/check/memmap.cmake`: grammar, tiling, tag and source checks, generator run, dump comparison), controls (13 listed in `03-DISCUSS-memmap.md` section 4 plus dump-stale and dump-missing), faults `GEN_MAP_MUTATION` and the decode-shape faults (`MEM_HI_INCLUSIVE`, `MEM_MIRROR_MASK`, `MEM_WO_READS_DEVICE`, `MEM_UNDRIVEN_ZERO`), discrepancies C60/C61/C62/C21/C51, CL-0006. Choke point: profile schema, c-purity generated path, instance-size constants. | MEM-01 | 03-01 |
| 03-04 | 3 (parallel-capable with 03-03; collides only on `core/CMakeLists.txt`, `tests/CMakeLists.txt`) | **IVSC v1: spec, parser, fixtures.** `docs/formats/ivsc-v1.md` (obeys the claim-word tripwire), `core/src/content/ivsc.[ch]` (pure, zero-copy, u64 arithmetic, check order of D-33), `data/cart/*.json` + `cmake/IvsCartTable.cmake` (build-tree table, `reseal`, `patch`, `repeat` forms), `data/expected/cart-fixture-files.ids`, `fixtures/FX-ivsc-vectors/manifest.json` + `data/expected/fixtures.ids`, `tools/cart/` helper that prints manifest output entries, suite `ivsc` (P/B/N ids, canonical round trip, seeded fuzz with half resealed), `data/expected/faults/ivsc.ids` (six faults), claim for the fixtures. Layer 2 (`L*`, `X01`) waits for 03-07. | MEM-05, MEM-04 (parse rows) | 03-01, 03-02 |
| 03-05 | 4 | **Width masking and boundary probes (MEM-02).** `cmake/IvsMemmapProbes.cmake` expands literal rows to probe tuples (`lo-1, lo, hi, hi+1`, mirror wrap), expected values from a device-and-width rule table in the test, never from descriptors; gate open/closed probes through the internal setter; internal raw-poke seam so a wide stored ROM word proves the read mask; stub policy tests; `memmap.ids` rows keyed by literal row; `data/expected/faults/memmap.ids` (`MEM_ROM_NO_MASK`, `MEM_RAM8_NO_TRUNC`, `MEM_RAM16_TRUNC`, `MEM_ROM_WRITABLE`, `MEM_GATE_IGNORED`, plus those left from 03-03). `MEM_LOAD_TRUNCATES` moves to 03-07. | MEM-02 | 03-03 |
| 03-06 | 5 | **Instance lifecycle and canonical digest.** Public headers `ivs.h`, `ivs_machine.h`, `ivs_status.h` (full D-09 codes), `IVS_API`, `ivs_api_version`, `ivs_status_name`, `ivs_machine_storage_requirements`, `ivs_machine_create`, `ivs_machine_destroy`, `ivs_machine_info` (empty slots), `ivs_machine_state_digest`; flat instance struct (`_Alignas(16)`, size and alignment `_Static_assert`s); field table serializer and domain framing (D-35); `data/expected/api-statuses.ids` pin; tests `machine.create.*`, `machine.digest.*` (literal byte stream through a capture sink, sensitivity sweep, padding-flip no-change), header self-containment ctest (each public header alone, `-x c` C17 pedantic, a C++ compile only where a C++ compiler exists and a visible SKIP otherwise); faults `DIGEST_*`, `CREATE_MAGIC_BEFORE_PROFILE_CHECK`, `INIT_SKIPS_MEMSET`, `DESTROY_NO_SCRUB`. Choke point: instance layout. | API-01, API-06 | 03-03, 03-05 |
| 03-07 | 6 | **Typed load, unload, reports, refusal matrix.** `ivs_machine_load` and `ivs_machine_unload` for EXEC, GROM, CARTRIDGE: validate (D-11 order), layer 2 window and feature checks, `expect_sha256`, reports (length, whole-file SHA-256, units, base, loaded) and `ivs_machine_info`; rows `machine.refuse.*`, `machine.prec.*` (each adjacent pair), layer-2 fixtures `L*`, `X01` (window drift), `X02`; priors fresh, system loaded, full; every row asserts status, whole-storage `memcmp`, digest equality, zeroed out struct; faults `LOAD_*_BEFORE_*`, `DUPLICATE_REPLACES`, `CHECK_ORDER_RANGE_FIRST`, `CODE_OVERLAP_AS_RANGE`, `STATE_CHECK_OMITTED`, `LEN_TRUNCATED_32`, `ROM_DECLE_MASKED_NOT_REFUSED` (= `MEM_LOAD_TRUNCATES`), `UNLOAD_KEEPS_CART`; `data/expected/faults/machine.ids`. Choke point: status registry and the matrix. | MEM-03, MEM-04, MEM-05 | 03-04, 03-06 |
| 03-08 | 7 | **Reset, step seam, isolation.** `ivs_machine_reset(POWER_ON)` with MISSING_SYSTEM_IMAGE (empty EXEC, empty GROM, neither), OFF to POWERED, volatile fill, `ivs_cp1610_reset`, time 0; `ivs_machine_step_unit_internal` driving `ivs_cp1610_step` through the machine bus (IAB answers `reset.r7` and `interrupt.iab_value`); isolation rows `machine.iso.*` (D-36: interleaved vs sequential, poisoned storage byte-identical, memcpy relocation, destroy mid-run, cross-instance load); turn on `--forbid-writable-data` in `c-purity` with controls `core-purity-static-mutable` and a Linux-shape control; fault `STATIC_CARRY`. Hand-assembled decle programs from literals (no assembler, D4). | API-01, MEM-04, SC5 | 03-07 |
| 03-09 | 8 | **Effect table and robustness.** `data/api/api-effects.json`, `data/expected/api-calls.ids`, `tools/check/api-effects.cmake` + `steps/api-effects.sh` (rules in Section D), controls `api-*` (about 8), `machine.sweep.*` (every prefix and single-byte change of each cart fixture; refusal implies unchanged storage, OK implies unload equals fresh digest), optional non-gate libFuzzer target recipe under `tests/fuzz/`. | API-06, MEM-04 | 03-08 |
| 03-10 | 9 | **Close-out.** Final discrepancy and claim reconciliation, `docs/formats` cross-links, `.planning/CI-EFFICIENCY.md` row and re-baseline (D-38), measurement vs the 40 s hosted budget, verification traceability SC1 to SC5, `STATE.md`/`ROADMAP.md` bookkeeping. | all | 03-09 |

**Schema and dependency choke points.** 03-01 (every later plan inherits the fault loop and purity changes), 03-03 (profile fields and generator output feed 03-05, 03-06, 03-08), 03-06 (the instance layout and public header freeze what 03-07 and 03-08 build on), 03-07 (status registry and the matrix are the largest single test inventory). If 03-03 grows too large, the cleanest split is 03-03a (fields, generator, decode, dump, literal, step) and 03-03b (discrepancies, claims, decode-shape faults).

## Section C: risks specific to this codebase (requested item 3)

1. **Gate wall time.** [VERIFIED: ran `sh tools/gate.sh` this session] PASS, 24 steps, 192 controls, 67 fault runs, `gate-time total=46`, load about 9. Step times: `controls` 25 (overlapped, `controls-wait 0`), `c-build` 13, `hygiene-history` 6, `cp1610-vectors` 6, `faults-verdict` 4, `c-faults` 3, `c-purity` 3. CONTEXT D-38 estimates +2 to 4 s hosted. Adding about 40 fault executables, each running the whole suite sequentially, plus about 35 controls, is more likely +6 to 10 s hosted [INF]: hosted baseline 31 s (CONTEXT, run 42942719332) would land at 37 to 41 s, inside the new 40 s budget only barely. Levers in order: lane the fault runs (03-01), single-pass awk in `faults-verdict`, keep owner TUs small, keep `machine` under about 0.25 s per run in a plain build, drop the second-preset run of `machine.sweep.*`. Measure at 03-02 (first real second file) and 03-07, and record in `CI-EFFICIENCY.md`.
2. **c-faults/faults-verdict generalization.** Four single-id-file assumptions (A11-A13). Fault ids must be unique across files; result files must list exactly the suite's id set, so a fault exe must run the full suite including skips. The orphan-site check on the union is the subtle part: without it every site of the second file reports `fault-orphan-site`.
3. **Id inventories and digest pinning.** Test ids, fault ids, control ids, `api-calls.ids`, `api-statuses.ids`, `cart-fixture-files.ids`, `fixtures.ids` and the manifest's sha256/size values are all exact sets: a one-line edit anywhere needs its pair edited in the same commit. For the literal-map-derived probe ids, derive the expected id set from the literal rows inside the `memmap` step and compare it with `memmap.ids` (token `memmap-probe-ids`), so a literal edit cannot leave stale ids.
4. **cmake -P generator fail-closed pattern.** A `FATAL_ERROR` inside an `add_custom_command` surfaces as a build failure that `c-build` prints (`c-build.sh:25-33`); keep messages prefixed with the generator name and field path. Dependencies: list the profile files, the generator and every ids file in `DEPENDS` and `CMAKE_CONFIGURE_DEPENDS` (pattern at `tests/CMakeLists.txt:18-29`), or incremental builds keep a stale table. Accept only `zero` for `power_on_fill.*` in Phase 3 (the profile schema also allows `pattern(v)` and `seeded(...)`, see `data/profiles/...` note) and fail closed otherwise.
5. **ROM-width tests at region boundaries.** Loaded ROM can never hold a word above `03FF` (loader refuses), so a no-mask read fault is invisible to every loader-based probe. Test the pure width function directly and add an internal raw-poke seam that stores a wide word; boundary probes then read it back masked at `lo`, `hi`, and just outside.
6. **SHA-256 independent oracle.** `cmake -E sha256sum` is a file hasher and the repo already shells out to it (`tests-inventory.sh:158`). The always-on form is `string(SHA256)` in a generator over printable-ASCII pattern messages; bytes at or above 0x80 and NUL cannot be CMake strings, so cover those through the IVSC fixtures (hex literals typed from an authoring-time `cmake -E sha256sum`) and the 1,000,000 x "a" vector.
7. **memcpy/memset availability under the purity allowlist.** Allowed: `memcpy memmove memset memcmp bzero` and the `__*_chk` forms (A9). Probed: at `-O2` clang lowers a byte-zeroing loop to `bzero` (allowed) and gcc to `memset` (allowed); `memcmp(..)==0` stayed `memcmp` on both (also on a variable length). Not allowed and emitted: explicit `strcmp`, `strlen`, and an unbounded `while (s[n]) n++` loop (both compilers turn it into `strlen`). A bounded `for (i < 64)` identity loop stayed clean on both. Rule: profile-id matching is a hand-written bounded loop; no `<string.h>` string functions in the core.
8. **C17 alignment of caller storage.** `_Alignas(16)` and `_Alignof` are language keywords (no `<stdalign.h>`, which the purity include list would reject); `max_align_t` is in `<stddef.h>`, allowed. `ivs_machine_storage_requirements` returns size and alignment; `create` refuses a pointer with `(uintptr_t)p % align != 0` (`<stdint.h>`) as BAD_ARG and a short size as BUFFER_TOO_SMALL. Static `unsigned char[]` storage that the host also touches as bytes is a strict-aliasing hazard no tool can detect; tests use `malloc` memory (exact size, so ASan checks bounds) and one `_Alignas(16)` static buffer in the release `-O2` run. Never place an instance on the stack (about 110 KB [INF]).
9. **Input overlapping instance storage.** D-11 omits it; a `memcpy` commit from a caller buffer that overlaps the instance is undefined. Refuse as BAD_ARG with u64-widened address-range comparison (API discuss D-A3), as an extra row of the BAD_ARG class, and add it to the precedence matrix.
10. **Hosted-only compile contact.** `-Wconversion -Wsign-conversion -Werror` on gcc 13 is first seen by hosted CI for new code; SHA-256 rotates and big-endian assembly are the likely hits (`uint8_t` operands promote to `int`). Dev preset runs UBSan with `-fno-sanitize-recover=all`, which turns the `b[0] << 24` class into a test failure locally.
11. **Fault executables never link a sanitizer** (`faults.cmake:8-10`). Out-of-bounds reads are covered by the dev-preset ASan run with exact-size heap buffers, a documented exemption from fault-testability (`03-DISCUSS-refusal.md` section 7).
12. **Instance size and test hygiene.** About 110 KB per instance [INF: EXEC 8192 B + GROM 2048 B + cart pool 49,152 words = 98,304 B + GRAM/scratch/system RAM about 1.5 KB + `cart_page[192]` + slots]; CONTEXT's 150 KB is an upper estimate. A refusal row run against four prior states allocates and compares several instances; reuse two buffers and re-create, do not allocate per row.

## Section D: Validation Architecture (requested item 4)

### Test Framework
| Property | Value |
|----------|-------|
| Framework | In-repo TAP harness `tests/harness/ivs_test.[ch]` (D-20), ctest registration per suite; no third-party library |
| Config file | `CMakePresets.json` presets `dev` (Debug -O1, ASan+UBSan, Werror, fault controls) and `release` (-O2, Werror) |
| Quick run command | `ctest --preset dev -R '^(machine\|memmap\|ivsc)$'` (after `cmake --build --preset dev`); one id family: `build/dev/tests/ivs_tests_machine --only machine.refuse.` |
| Full suite command | `sh tools/gate.sh` (the single local and CI command) |

### Suites and inventories (names are recommendations; Discretion)
| Suite (ctest, exe, ids file) | Content | Expected size |
|------------------------------|---------|---------------|
| `machine`, `ivs_tests_machine`, `data/expected/tests/machine.ids` | `machine.sha256.*`, `machine.create.*`, `machine.refuse.*`, `machine.prec.*`, `machine.iso.*`, `machine.digest.*`, `machine.sweep.*` | about 150 ids, under 0.25 s per run |
| `memmap`, `ivs_tests_memmap`, `memmap.ids` | one id per literal row (probes), `memmap.dump`, gate and stub rows, width-function direct tests | about 50 ids |
| `ivsc`, `ivs_tests_ivsc`, `ivsc.ids` | `ivsc.P*`, `ivsc.B*`, `ivsc.N*`, `ivsc.L*`, `ivsc.X*`, `ivsc.fuzz.seed-*` (research file lists 70 fixture ids as the starting inventory; finalize in 03-04 and 03-07) | about 80 ids |
| `core-symbols`, existing | shipped archive vs allowlist | 1 |

### Phase Requirements to Test Map
| Req | Behaviour | Test type | Automated command | File exists |
|-----|-----------|-----------|-------------------|-------------|
| MEM-01 | Decode dump equals the literal map; generator refuses malformed descriptors; literal tiles `0000-FFFF` per view | step + generator controls | `sh tools/gate.sh` (step `memmap`), `ctest --preset dev -R memmap` | Wave 3 |
| MEM-02 | Probes at `lo-1, lo, hi, hi+1`, mirror wrap, width rules for 10/8/16-bit, store truncation, gates, stubs | unit | `build/dev/tests/ivs_tests_memmap --only memmap.row.` | Wave 4 |
| MEM-03 | Load EXEC, GROM, cartridge as separate inputs; report length, unit count, whole-file SHA-256; `expect_sha256` | unit with literal digests | `--only machine.load.` | Wave 6 |
| MEM-04 | Each refusal class returns its own code; whole-storage `memcmp` and digest equal before and after; precedence pairs; reset refuses missing images | table-driven unit | `--only machine.refuse.` and `machine.prec.` | Wave 6 and 7 |
| MEM-05 | IVSC positive, boundary, malformed fixtures; canonical round trip; fixtures recorded in the manifest | unit + manifests step | `ctest -R ivsc`, step `manifests` | Wave 3 and 6 |
| API-01 | create/reset/unload/destroy; two instances independent (interleaved equals sequential; poisoned storage byte-identical; memcpy relocation; no writable data symbols) | unit + purity step | `--only machine.iso.`, step `c-purity` | Wave 5 and 7 |
| API-06 | Every status has a documented effect and a test; effect is `none` and verified by `memcmp` | static step + unit | step `api-effects` | Wave 8 |

### Success criteria to checks (zero human UAT)
| SC | Automated check |
|----|-----------------|
| SC1 decode dump matches literal; widths at boundaries | Step `memmap` (dump written by suite `memmap`, nonce-bound, compared to literal) + `memmap.row.*` probes; faults `GEN_MAP_MUTATION`, `MEM_*` each turn a named row red |
| SC2 separate typed loads, length and digest, stand-ins only | `machine.load.*` with typed-in literal digests (authoring-time `cmake -E sha256sum`); stand-in word `(37*i + 11) & 0x3FF` built in test code; `files` and `hygiene` steps already forbid committed binaries |
| SC3 each refusal row its own typed error; state digest equal before and after | `machine.refuse.*`, `machine.prec.*` assert status, `memcmp` of whole storage and digest equality across priors fresh, system loaded, full, powered; reset rows for missing EXEC and GROM |
| SC4 format specified, fixtures pass and recorded in the manifest | `docs/formats/ivsc-v1.md` (text step covers claim words), `ivsc.*` ids, `manifests` step on `FX-ivsc-vectors` (digest and size of every `data/cart/*.json`) |
| SC5 isolation; every API error documented and tested | `machine.iso.*`; step `api-effects` (status pin, header function list, JSON rows, test ids) |

### Sampling Rate
- Per task commit: the suite `--only <prefix>` of the touched family plus the touched step script.
- Per wave merge: `ctest --preset dev` and `--preset release`.
- Phase gate: `sh tools/gate.sh` PASS, inventories exact, before `/gsd-verify-work`.

### Wave 0 Gaps
None outside the plans: 03-01 creates the infrastructure every later plan uses; no framework install is needed.

### `api-effects` step rules (D-15), all static
(1) every `#define IVS_E_*` and `IVS_OK` in `core/include/ivs/ivs_status.h` equals a line of `data/expected/api-statuses.ids` (`NAME VALUE`) and vice versa; (2) every `IVS_API` declaration in `core/include/ivs/*.h` is in `data/expected/api-calls.ids` and in the JSON `calls`, and vice versa; (3) each `(call, status)` row names a test id present in `data/expected/tests/machine.ids`; (4) every effect is `none` or registered; (5) every non-OK status appears in at least one row; (6) every status in `-24..-31` stays absent. Tokens: `api-status-renumbered`, `api-status-unpinned`, `api-call-undeclared`, `api-call-undocumented`, `api-refusal-no-test`, `api-effect-unregistered`, `api-dead-status`. Residual: the step cannot prove a test actually returns the documented status; each cited test asserts the code itself, and `tests-inventory` proves it ran (OQ-5 offers the optional observed-pair log).

## Section E: discrepancy-log and profile updates (requested item 5)

| Item | Edit | Details |
|------|------|---------|
| C60 (new) | `data/discrepancies.json`, `data/expected/discrepancies.ids` | Upper bits of 8-bit RAM, PSG and STIC register reads. Status `partial` (GI mask ROM and GROM drive zeros: tier 1, pdf 194, 197). `default_policy` must begin `unknown-default-chosen:` (zero for scratchpad; stubs read the undriven fill). Resolver `measurement: D10 read-back` |
| C61 (new) | same | Write-only STIC and GRAM aliases coexist with cartridge ROM at `4000-403F` and similar (tier 4, one community lineage). Status `open`; default: reads return cartridge ROM when loaded, writes reach the alias device (here dropped) |
| C62 (new) | same | Cartridge window `4000-FFFF` with 256-word alignment is narrower than the community usable list; tier 4. Status `open` |
| C21 | edit resolver | Currently `test: Phase 3 memory-map decode diagnostic`; change to `measurement: D10 ...` and extend `sources` with the GI window (pdf 197, printed 5-16) and the `3A00-3FFF` write disagreement. A Phase 3 test can only pin the default (a green probe is consistency, not resolution) |
| C51 | `status: partial` | ROM upper bits read 0 for GI parts (pdf 194, 202); operand words on the flat CPU test bus still keep 16 bits; `evidence` field non-blank |
| C5 | optional text update | Blocked and undriven reads are the profile field `memmap.undriven_read` (`FFFF`) |
| C32, C42 | optional text hygiene | `reset.r7` `resolving_evidence` still says "pending: Phase 2 boot-stub decision (D5)"; D5 is decided (D-24), update to "pending: D10 read-back or the Phase 4 boot stub" |
| New profile fields (about 25; ids file exact set) | `data/profiles/intv-2609-ntsc-family-p1.json`, `data/expected/profile-fields.ids` | `memmap.undriven_read`; `memmap.device.{rom10,grom8,ram8,ram16,stub}` each `{bits, rmask, fill, store}`; `memmap.region.<name>` objects `{view, lo, hi, device, size_words, gate, impl}` for stic-regs, stic-alias, scratchpad, psg, sysram, exec, grom, gram, three STIC write-only aliases, three GRAM write-only aliases (one range per region); `memmap.cart_window`; `memmap.cart_align_words`; `cartridge.features_supported` = 0; `system_images.exec` and `system_images.grom` `{units, unit_bits, base, required}`. Rewrite `cartridge.address_windows` to point to `memmap.cart_window` (or remove from both file and ids) and update `system_images.exec_grom` to the D-24 policy |
| Claims | `data/claims.json`, `data/expected/claims.ids` | CL-0006 (decode matches the sourced literal map, tier `source-expectation`, `depends_on` the `memmap.*` fields, standing `provisional`); CL-0007 (IVSC fixtures); neither claim may use a stronger tier |
| Sources | `data/sources.json` (optional) | Register the community ROM-format metadata document read for D-29 as a tier-4 lead; do not name its host project in tracked non-planning files (denylist) |

## Common Pitfalls

### Pitfall 1: generated include invisible to c-purity
**What goes wrong:** `c-purity` stage 1 rejects `#include "ivs_memmap.inc"` (`c-purity-include`) and stage 2 cannot compile it. **Why:** A7/A8. **Avoid:** 03-01 adds the reviewed-basename list and the generator hook before any core TU includes a generated file. **Warning:** `c-purity-compile: <file>` on the first plan that includes a table.

### Pitfall 2: `_good/tree` drift
**What goes wrong:** a new step without a toy input fails `control-good-failed`; an extra id in the real tree is invisible to the toy tree. **Avoid:** every plan that adds a step or an inventory file adds its toy counterpart in the same commit; prefer the same-suite second faults file (A14).

### Pitfall 3: writable-data rule false positives
**Probed:** `gcc:13` `-O2`: the existing `ivs_cp1610_rows` const-with-pointers table is `r` by default and with `-fno-pie`, and `d` with `-fPIE`; a writable static is `b` in all three modes [VERIFIED: probe this session]. Ubuntu's gcc is believed to default to PIE [ASSUMED, A1], so a rule that fails on `d` would flag the existing table on the hosted runner. Apple clang 21: const tables are `s` regardless of `-fPIE/-fno-pie`; writable statics `b`/`d`/`C`; compiler labels `ltmpN`, `l_*` must be filtered [VERIFIED: probe]. **Avoid:** compile the rule's archive with `-fno-pie` in `c-purity`, apply only there, types `b B d D c C`.

### Pitfall 4: libc calls the compiler adds
See Section C item 7. Add a control `core-purity-strlen` (overlay a core file with an unbounded string loop) so the next person to write `strcmp` sees the token.

### Pitfall 5: `-fvisibility=hidden` hides nothing from nm
`cmake/IvsWarnings.cmake` sets hidden visibility on `ivs_warnings`, but the existing archive lists `T` for every external function [VERIFIED: `nm -P -g --defined-only build/release/core/libivs_core.a` shows `_ivs_cp1610_step T`]. The D-06 "IVS_API symbol list-equality" can therefore only be header-vs-table, not archive-vs-table; the export check stays deferred to a shared library (Phase 4).

### Pitfall 6: the dump cannot be purely behavioural
Behaviour (read after write, loaded stand-ins, mirror writes) reveals width, writability and mirror extent, but a stub and an undriven region both read the fill and drop writes. Split the dump: behaviour-derived columns (view, range, width, read/write, mirror) come from real bus probes; label columns (device, impl, gate) come from a decode introspection function and are declarative, labelled so in the claim (OQ-4). A probing instance loads stand-in EXEC, GROM and one cartridge segment filling the whole window `4000-FFFF` (49,152 words, within the 65,535-word segment limit).

### Pitfall 7: ROM mask not observable via the loader
See Section C item 5. **Avoid:** direct tests of the width function with wide stored values plus a raw-poke seam.

### Pitfall 8: trailer digest is not the reported digest
IVSC's trailer is SHA-256 of bytes `0..total-33` (D-30), while D-25 reports SHA-256 over the **whole** supplied file (equals `sha256sum file`). They differ. Compute both in one pass: hash the prefix, copy the context, finalize the copy to compare with the trailer, then update the original with the 32 trailer bytes and finalize for the report. The SHA context must therefore be a plain copyable struct (03-02).

### Pitfall 9: manifest digest pinning
`FX-ivsc-vectors` pins sha256 and size of every `data/cart/*.json`; any edit in any later plan fails `manifests` until refreshed. Provide `tools/cart/` helper that prints the `outputs` entries, have each plan that touches fixtures refresh them, and keep fixtures split into a few files so the diff is local. Also regenerate typed-in digests of positive fixtures after the big-endian change (the research file's example digest was computed for little-endian bytes and is void).

### Pitfall 10: stale or hand-edited ids
The `machine` suite aggregates several case tables; a duplicate id across tables is rejected before anything runs by the harness (`ivs_test_find_duplicate`), but a renamed id leaves `tests-missing` plus `tests-unlisted`. Generate a one-line `--list` check into the plan's verify step.

## Code Examples

### Streaming SHA-256 with a copyable context (sketch for 03-02; interface only)
```c
/* core/src/common/ivs_sha256.h: plain struct, no pointers, copy = snapshot. */
typedef struct ivs_sha256 {
  uint32_t h[8];
  uint64_t total_bytes;
  uint8_t block[64];
  uint32_t used;
} ivs_sha256;
void ivs_sha256_init(ivs_sha256 *c);
void ivs_sha256_update(ivs_sha256 *c, const uint8_t *p, size_t n);
void ivs_sha256_final(ivs_sha256 *c, uint8_t out[32]);
```

### Big-endian word assembly (no casts, no signed shift)
```c
static uint16_t ivs_be16(const uint8_t *p) { return (uint16_t)(((uint32_t)p[0] << 8) | (uint32_t)p[1]); }
static uint32_t ivs_be32(const uint8_t *p) {
  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}
```

### Width formula as one pure function (tested directly with wide stored values)
```c
/* D-17: read = (stored & rmask) | (fill & ~rmask). */
static uint16_t ivs_mem_read_value(uint16_t stored, uint16_t rmask, uint16_t fill) {
  return (uint16_t)((stored & rmask) | (fill & (uint16_t)~rmask));
}
```

### Faults file header for the generalized loop (proposed)
```
# suite=machine; columns: <FAULT_ID> <must-fail-id> <allowed-prefixes> <must-pass-id>
SHA_K_WRONG machine.sha256.fips-abc machine.sha256. machine.create.size-reported
```

## State of the Art

| Old approach (Phase 2) | Phase 3 approach | Impact |
|-----------------------|------------------|--------|
| One fault file, one suite, flat test bus | One file per area, suite per file, machine bus | Fault ids unique across files; owner TU scanning unchanged |
| Internal status header | Public `ivs/ivs_status.h` with frozen numbers | Pin file and a static step become the renumbering detector |
| Vector JSON compiled to a table | Same, plus profile JSON, cart JSON, literal text | Three generators share the fail-closed idiom |
| Imports-only symbol check | Plus writable-data rule on purity's own archive | Isolation has a structural proof next to the behavioural tests |

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | Ubuntu 24.04's packaged gcc 13 defaults to PIE, so const-with-pointers tables compile to `.data.rel.ro` (nm `d`) unless `-fno-pie` is passed | Pitfall 3 | If it does not default to PIE the `-fno-pie` flag is harmless; if the hosted runner behaves differently from the `gcc:13` image in another way, 03-08's first hosted run shows it (planned verification) |
| A2 | Hosted gate cost lands at 37 to 41 s after the phase, from measured local steps and the CONTEXT hosted baseline of 31 s | Section C item 1 | Budget breach; levers are listed; remeasure at 03-02 and 03-07 |
| A3 | The instance is about 110 KB | Section C item 12 | Only sizes test buffers; `ivs_machine_storage_requirements` reports the truth |
| A4 | Big-endian words for EXEC and GROM dumps (D-26 [BG]) | D-26 | A wrong guess surfaces as a refusal on a real file at D11, never a silent miscompute |
| A5 | mawk or another POSIX awk on the hosted image behaves like BWK awk for the new scripts | A11-A12 | Run new scripts under `dash` and keep awk POSIX-only |
| A6 | `ivs_core` objects compiled with `-fvisibility=hidden` and no `-fno-pie` remain linkable into a Phase 4 static consumer | A5 | Phase 4 owns the consumer; no Phase 3 impact |

## Open Questions

1. **OQ-1 (RESOLVED, planner decides in 03-03/03-04): segment alignment versus one-word fixtures.** D-18 sets `memmap.cart_align_words` 256 and a per-256-word-page `cart_page[]`; D-31 allows segments of 1..65535 words and the research fixtures use 1-word segments at `5000`. Recommendation: the container (layer 1) accepts any length; the profile (layer 2) requires segment start and length to be multiples of `cart_align_words` and refuses otherwise with OUT_OF_RANGE (new `L` fixtures). Parse-level fixtures stay tiny; load-level fixtures use a `repeat` form in `data/cart/*.json` (e.g. `{"word":"03FF","count":256}`), so no large hex literal is committed. Owner: planner; no owner input needed (D-18 stays literal).
2. **OQ-2 (RESOLVED): header size wording.** D-30 reads "24-byte header ... (+ padding to 24 per the spec)". The listed fields (version 2, flags 2, required_features 4, total_bytes 4, segment_count 2, reserved 2) sum to 16; with the 8-byte magic the prefix is exactly 24 and the table starts at offset 24 (the research file's worked layout and its 66-byte minimum, 24 + 8 + 2 + 32, confirm it). No padding. The spec doc states this explicitly.
3. **OQ-3 (RESOLVED): reported digest.** Follow D-25 (whole supplied file) and use the single-pass copy trick of Pitfall 8; the research file's "digest = the trailer" line is superseded.
4. **OQ-4 (RESOLVED, planner): dump columns.** "Structural columns" of D-21 are split as in Pitfall 6; the claim for MEM-01 says label columns are declarative. Owner: planner.
5. **OQ-5 (RESOLVED, optional hardening in 03-09): observed pairs.** The static `api-effects` step cannot prove a documented `(call, status)` pair is returned. Optional: a `REFUSE_CALL` macro that appends `# pair <fn> <CODE>` lines to a second per-suite output file the step cross-checks (needs a small harness addition and a results consumer, so the step would sort after `c-tests`, e.g. renamed `tests-api-effects`). Default: static only, each test asserts its code. Owner: planner.
6. **OQ-6 (RESOLVED): suite versus fault-file mapping.** D-15 puts sha256, refuse, isolation and digest ids in one `machine` suite; D-37 wants one faults file per area. Recommended: files `sha256.ids` and `machine.ids` both carry `# suite=machine`; `memmap.ids` suite `memmap`; `ivsc.ids` suite `ivsc`; `cpu.ids` unchanged.
7. **OQ-7 (open, owner-local, non-blocking): D10 and D11.** C20, C21, C60, C61, C62 (measurement) and the system-image byte order (a real EXEC/GROM) cannot be closed by any test here. Nothing in Phase 3 is gated on them; claims stay `provisional` and `source-expectation`.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| CMake | build, all generators | yes | 4.4.3 local, 3.31.6 hosted | none needed |
| Ninja | presets | yes | 1.13.2 | none needed |
| C compiler | build | yes | Apple clang 21; gcc 13 hosted | none needed |
| `nm`, `ar` | purity, symbol check | yes | llvm-nm local, binutils 2.42 hosted | none needed |
| Docker | optional Linux probes while planning | yes (local only) | 29.5.2 | skip; first hosted run is the check |
| C++ compiler | optional public-header C++ include check | clang++ local, g++ hosted | - | visible SKIP, never silent |

Missing dependencies with no fallback: none. No new tool is required by the phase.

## Security Domain

Applicable ASVS level 1 categories (C parsing library, no network, no auth):

| ASVS category | Applies | Standard control |
|---------------|---------|------------------|
| V5 Input validation | yes (all of MEM-04, MEM-05) | u64-widened length checks before any use, fixed check order, bounded loops (at most 16 segments), refuse-not-mask, zero-copy parser with no read past the supplied length (ASan on exact-size buffers) |
| V6 Cryptography | partial | SHA-256 for identity and digests only, implemented from FIPS 180-4 and cross-checked against a second implementation; no secret material, no keys |
| V12 Files and resources | partial | Core takes bytes only; no path, no file handle; no image bytes in reports or logs |
| V2/V3/V4 auth, session, access control | no | no principals in a library core |

| Threat pattern | STRIDE | Mitigation |
|----------------|--------|-----------|
| Integer overflow in `len`, `addr + len`, `total_bytes` | Tampering | u64 arithmetic, bound check before each use, `OVERSIZED` before any byte is read, fixtures B06, B08, B09 |
| Out-of-bounds read of a lying container | Info disclosure | Parser derives every offset from checked counts, never stores offsets; truncation sweeps under ASan |
| Mutation before refusal (partial state) | Tampering | validate-then-commit, `const` validate, whole-storage `memcmp` oracle, seven named mutation faults |
| Input buffer aliasing instance storage | Tampering | Overlap check as BAD_ARG (Section C item 9) |
| Stale or forged identity (digest confusion) | Spoofing | Report whole-file SHA-256 computed by the core, never trusted from the file; `expect_sha256` compare is a fixed-length loop |
| Rights leak (a real image enters the repo) | Information disclosure | Stand-ins generated in test code; text fixtures only; `files`, `hygiene`, `binscan` steps |

## Sources

### Primary (HIGH)
- Repository files read this session with line numbers: `tools/gate.sh`, `tools/check/steps/{c-build,c-faults,faults-verdict,c-tests,c-purity,tests-inventory,denylist,text,integrity,profile}.sh`, `tools/check/{run-controls.sh,core-symbols.sh,core-symbols-allow.txt,core-symbols-deny.txt,integrity.cmake,manifests.cmake,records.cmake,lib.cmake}`, `tools/check/controls/_good/tree/**`, `tests/CMakeLists.txt`, `tests/faults.cmake`, `tests/core-symbols-tap.sh`, `tests/harness/ivs_test.h`, `core/CMakeLists.txt`, `CMakeLists.txt`, `CMakePresets.json`, `cmake/{IvsVectorTable,IvsWarnings}.cmake`, `core/src/common/{ivs_status.h,ivs_prelude.h}`, `core/src/cpu/cp1610.h`, `data/profiles/intv-2609-ntsc-family-p1.json`, `data/discrepancies.json` (C5, C20, C21, C51), `data/claims.json`, `docs/fixture-manifest.md`, `.planning/CI-EFFICIENCY.md`.
- Probes run this session: full gate (`gate-time total=46`, steps/controls/faults counts above); Apple clang 21 and `gcc:13` (13.5.0) `nm -P` letters for writable and const tables under default, `-fPIE`, `-fno-pie`; `-O0/-O2` libc imports for `memcmp`, `strcmp`, unbounded and bounded string loops, zeroing loops.

### Secondary (MEDIUM)
- `03-CONTEXT.md` and `03-DISCUSS-{api,memmap,content,cart,refusal}.md` (dated option analysis; GI 1980 pdf 194, 197, 202 reads are theirs and are not repeated here).
- `02-RESEARCH.md` for hosted image versions (Ubuntu 24.04 runner Readme).

### Tertiary (LOW)
- Ubuntu's default-PIE gcc behaviour (A1) and hosted timing estimates (A2): not measured here.

## Metadata

**Confidence breakdown:**
- Existing mechanisms and change points: HIGH (read with line numbers).
- Plan decomposition and ordering: MEDIUM-HIGH (follows dependency edges; sizes are estimates).
- Gate cost projection: MEDIUM (baseline measured, increments inferred).
- Linux nm and libc behaviour: MEDIUM-HIGH (probed on GCC 13.5.0 container; Ubuntu package defaults assumed).

**Research date:** 2026-11-10
**Valid until:** 2026-12-10 (stable repository; refresh if `tools/check` or the fault loop changes before planning)
