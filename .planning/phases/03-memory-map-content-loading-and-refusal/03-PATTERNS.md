# Phase 3: Memory Map, Content Loading and Refusal - Pattern Map

**Mapped:** 2026-11-10
**Files analyzed:** 45 file classes (a glob such as `data/cart/*.json` or `tools/check/controls/<new>/` counts once)
**Analogs found:** 44 / 45 (29 exact, 11 role-match, 4 partial); 1 class has none (`digest.c`). Seven sub-parts of otherwise matched classes have no repo analog and are listed under "No Analog Found".

Tracked-source gate (#3645): every analog path below was checked with `git ls-files --error-unmatch` and is tracked. No gitignored mirror path, nothing under `refs/` and no `.planning/` path is named as an analog. Line numbers refer to the working tree on branch `gsd/phase-3-memory-map-content-loading` at HEAD `443ef5f` (Phase 2 merged). The gate was not run while mapping.

## Reading guide

Phase 3 is the first phase whose C code has to live in the gate scaffolding that Phase 2 built for a CPU-only tree. The CPU code needed no generated include, no public header directory, no second fault suite and no new gate step. Phase 3 needs all four. So the riskiest part of the phase is the contract with that scaffolding, not the C itself.

- Read **"Gate and build coupling hazards" (H1-H14)** first. Each one is a place where an existing step, script or inventory fails on the first Phase 3 file unless it is edited in the same plan.
- Then read **Shared Patterns S1-S9**; the per-file assignments refer to them by number.
- Phase 2 analogs are the best matches for C code, tests and fault controls; Phase 1/2 gate files are the best matches for steps, record kinds and controls.

## File Classification

| New/Modified File | Role | Data Flow | Closest Analog | Match Quality |
|-------------------|------|-----------|----------------|---------------|
| `core/include/ivs/ivs_status.h` (moved from `core/src/common/`) | public header | n/a | `core/src/common/ivs_status.h` (itself) | exact |
| `core/include/ivs/ivs.h` (umbrella, version) | public header | n/a | `core/src/cpu/cp1610.h` (style only) | role-match |
| `core/include/ivs/ivs_machine.h` | public header (opaque handle, versioned structs) | request-response | `core/src/cpu/cp1610.h` 62-72, 134-148 | role-match |
| `core/src/common/ivs_sha256.h`, `ivs_sha256.c` | core utility (streaming digest) | transform | `core/src/cpu/cp1610_decode.c` (const table + prelude); algorithm: none | partial |
| `core/src/content/ivsc.h`, `ivsc.c` | core parser (pure, bounded) | transform | `core/src/cpu/cp1610.c` 199-224 (validate) and 1356-1376 (refusal order) | role-match |
| `core/src/machine/machine.h` (internal instance struct, seams) | core header | n/a | `core/src/cpu/cp1610.h` | role-match |
| `core/src/machine/machine.c` (create, destroy, reset, load, unload, info) | core service | CRUD / request-response | `core/src/cpu/cp1610.c` 1356-1416 (`step`: refuse, copy, commit) | role-match |
| `core/src/machine/memmap.c` (bus function, decode, includes generated `ivs_profiles.inc`) | core bus | request-response | `tests/harness/ivs_testbus.c` 52-86 (switch on access kind); `tests/cpu/cpu_vectors.c` 16 (generated include) | role-match |
| `core/src/machine/digest.c` (canonical state digest) | core utility | transform | none (field-table serializer) | none |
| `core/CMakeLists.txt` (modify) | config | n/a | itself (lines 7-25) | exact |
| `cmake/IvsMemmapTable.cmake` | build-time generator | transform (JSON to C initializer) | `cmake/IvsVectorTable.cmake` | exact |
| `cmake/IvsCartTable.cmake` (name discretionary) | build-time generator | transform | `cmake/IvsVectorTable.cmake` | exact |
| `data/profiles/intv-2609-ntsc-family-p1.json` (modify: `memmap.*`, slot descriptors, `cartridge.features_supported`) | data record | n/a | fields `reset.r7`, `crystal_hz` (OBJECT value), `cartridge.address_windows` | exact |
| `data/expected/profile-fields.ids` (modify) | inventory | n/a | itself | exact |
| `data/memmap/intv-2609-ntsc-family-p1.literal.txt` | data (literal map, line table) | n/a | `data/expected/faults/cpu.ids` (columns, `#` header); `data/expected/cp1610-skips.ids` | partial |
| `data/api/api-effects.json` | data record | n/a | `data/claims.json` | role-match |
| `data/cart/*.json` | data (fixtures) | n/a | `data/cp1610/vectors/*.json` | role-match |
| `data/expected/api-statuses.ids`, `api-calls.ids` | inventory (`NAME VALUE`) | n/a | `data/expected/cp1610-skips.ids` (two-column), `data/expected/claims.ids` | role-match |
| `data/expected/tests/{machine,memmap,ivsc-parse,ivsc-load,ivsc-fuzz}.ids` | inventory (first token + `skip-ok`) | n/a | `data/expected/tests/cpu-api.ids`, `cpu-sweep.ids`, `cpu-vectors.ids` | exact |
| `data/expected/faults/{machine,memmap,sha256,ivsc}.ids` | inventory (4 columns, `# suite=`) | n/a | `data/expected/faults/cpu.ids` | exact |
| `data/expected/gate-steps/machine.ids` | inventory | n/a | `data/expected/gate-steps/cpu.ids` | exact |
| `data/expected/controls/{memmap,api-effects}.ids`, extend `faults-verdict.ids`, `core-purity.ids` | inventory | n/a | `data/expected/controls/faults-verdict.ids`, `tests-inventory.ids` | exact |
| `data/expected/{fixtures,discrepancies,claims}.ids` (modify) | inventory | n/a | themselves | exact |
| `data/discrepancies.json` (C60-C62; edit C21, C51) | data record | n/a | entries C57-C59 | exact |
| `data/claims.json` (new claim for MEM-01) | data record | n/a | `CL-0003` (provisional, profile-field dependent) | exact |
| `fixtures/FX-ivsc-vectors/manifest.json` | fixture manifest | n/a | `tools/check/controls/_good/tree/fixtures/FX-good/manifest.json` | exact |
| `docs/formats/ivsc-v1.md` | doc (format spec) | n/a | `docs/cp1610/stepper-conventions.md`, `docs/time-base.md` | role-match |
| `tests/machine/machine_*.c` (refuse, prec, iso, digest, sha256, sweep; one exe, suite `machine`) | test | request-response | `tests/cpu/cpu_api.c` | exact |
| `tests/machine/memmap_rows.c` (suite `memmap`, one case per literal row) | test | batch (generated table) | `tests/cpu/cpu_sweep.c` | exact |
| `tests/machine/ivsc_fixtures.c` (suites `ivsc-parse`, `ivsc-load`) | test | batch (generated table) | `tests/cpu/cpu_vectors.c` | exact |
| `tests/machine/ivsc_fuzz.c` (seeded) | test | event-driven (PRNG) | `tests/cpu/cpu_fuzz.c` | exact |
| public-header self-containment and `IVS_API` list test | test | batch | `tests/core-symbols-tap.sh` | partial |
| `tests/CMakeLists.txt` (modify) | config | n/a | itself (lines 23-61) | exact |
| `tests/faults.cmake` (modify: loop over files, per-suite test sources) | config | n/a | itself (lines 13-100) | exact |
| `tests/harness/ivs_test.{h,c}`, `harness_selftest.c`, `data/expected/tests/harness.ids` (modify only if a second output stream is added) | test harness | event-driven | themselves | exact |
| `tools/check/steps/memmap.sh` | gate step (wrapper) | request-response | `tools/check/steps/cp1610-vectors.sh` | exact |
| `tools/check/memmap.cmake` | checker (cmake -P) | batch | `tools/check/vectors.cmake` | role-match |
| `tools/check/steps/api-effects.sh`, `schema/api-effects.txt`, `kinds/api-effects.cmake` | gate step + record kind | CRUD-validate | `steps/claims.sh`, `schema/claim.txt`, `kinds/claim.cmake` | exact |
| `tools/check/steps/c-faults.sh`, `faults-verdict.sh` (modify: loop over `faults/*.ids`) | gate steps | batch | themselves | exact |
| `tools/check/steps/c-purity.sh` (modify: `core/include`, generated include) | gate step | batch | itself (46-47, 96, 159) | exact |
| `tools/check/core-symbols.sh` (modify: writable-data rule) | helper script | transform (nm text to verdict) | itself (nm calls near 111-112) | exact |
| `tools/check/controls/<new>/{CHECK,EXPECT,FORBID,MAKE,overlay/}` and `_good/tree/**` additions | negative control | request-response | `controls/faults-not-detected`, `core-purity-alloc`, `claims-inventory-empty`, `tests-inventory-unlisted` | exact |
| `tools/check/binary-ext.txt`, `.gitignore` (optional: add `ivsc`) | config list | n/a | itself | exact |
| `tools/cart/seal.sh` (optional dev helper, not in the gate) | dev script | transform | `tests/core-symbols-tap.sh` (sh style) | partial |
| `.planning/CI-EFFICIENCY.md` (append a dated section), `README.md` (one line, optional) | doc | n/a | itself | exact |

## Pattern Assignments

### `core/include/ivs/ivs_status.h` (public header, move) and the status registry

**Analog:** `core/src/common/ivs_status.h` (whole file, 13 lines).

```c
/* SPDX-License-Identifier: MIT */
/* Status codes of the core API (D-06). 0 is success; a negative value is a caller refusal,
 * and a refused call never mutates caller state. Guest faults are not refusals (D-12). */
#ifndef IVS_STATUS_H
#define IVS_STATUS_H

typedef int ivs_status;

#define IVS_OK 0
#define IVS_E_BAD_ARG (-1)
#define IVS_E_BAD_STATE (-2)

#endif
```

Conventions to keep: `IVS_E_<NAME>` with a parenthesized negative literal, `ivs_status` is `int`, include guard `IVS_STATUS_H` (rename not needed). Add D-09's codes in the same style (`IVS_E_BAD_HANDLE (-3)` ... `IVS_E_DIGEST_MISMATCH (-23)`), and add `ivs_status_name()` as a plain `switch` returning string literals (no libc; `static const` strings only).

The one consumer that must be edited by the move: `core/src/cpu/cp1610.h:17` `#include "common/ivs_status.h"` becomes `#include "ivs/ivs_status.h"`. `grep` shows no other includer in `core/`, `tests/`, `tools/` or `docs/`. See H1 for the include-path consequences.

Pin file `data/expected/api-statuses.ids`: first token NAME, second VALUE (e.g. `IVS_E_BAD_ARG -1`). Two-column parse to copy: `cmake/IvsVectorTable.cmake` 231-236:

```cmake
  if(NOT l MATCHES "^([^ \t]+)[ \t]+(C[0-9]+)$")
    set(VT_CTX "${SKIPS}")
    vt_fail("skip line '${l}' is not '<id> <C-id>'")
  endif()
```

### `core/include/ivs/ivs.h`, `core/include/ivs/ivs_machine.h` (public headers)

**Analog:** `core/src/cpu/cp1610.h` for layout; no existing public header (see "No Analog Found" for the `IVS_API`/`extern "C"` parts).

Copy from `cp1610.h`: header comment that states what the header is and is not (lines 1-12), `#define`d constants with `u` suffix (no enums, no `bool`), plain structs with explicit `reserved` bytes, and `_Static_assert` on every size that the ABI depends on (lines 134-148):

```c
typedef struct ivs_cp1610 {
  uint16_t r[8];
  uint8_t status;
  ...
  uint8_t reserved[8];
} ivs_cp1610;

_Static_assert(sizeof(ivs_cp1610) == 32u, "ivs_cp1610 must be 32 bytes (D-03)");
```

D-04 adds `uint32_t struct_size` first in every request/report struct and D-06 limits public headers to standard headers; the handle is `typedef struct ivs_machine ivs_machine;` (incomplete type; the definition lives in `core/src/machine/machine.h`).

### `core/src/common/ivs_sha256.{h,c}` (core utility)

**Analog:** `core/src/cpu/cp1610_decode.c` for file shape; nothing for the algorithm.

File shape (`cp1610_decode.c` 1-31): license line, a block comment citing the source (here FIPS 180-4, no emulator source), then `#include "common/ivs_prelude.h"` FIRST, then the module header, then `static const` tables. Copy the `_Static_assert` on the table size (line 130) for the 64-entry K table. Use the existing narrowing macro for 16-bit stores; for 32-bit words write explicit `uint32_t` rotates (no `float`/`double`: `ivs_prelude.h` poisons them).

Test shape for the digest: literal expected digests typed in from `cmake -E sha256sum` (independent implementation), not computed by the code under test. Fault sites (S7) belong in `ivs_sha256.c`, one `#ifdef IVS_FAULT_<ID>` each.

### `core/src/content/ivsc.{h,c}` (pure parser)

**Analog:** `core/src/cpu/cp1610.c` 199-224 (`ivs_cp1610_state_validate`: pure, `const` input, ordered rejection, no writes).

```c
ivs_status ivs_cp1610_state_validate(const ivs_cp1610 *s)
{
  size_t i;
  if (s == NULL) {
    return IVS_E_BAD_ARG;
  }
  if (((uint32_t)s->status & ~(uint32_t)IVS_CP1610_STATUS_MASK) != 0u) {
    return IVS_E_BAD_STATE;
  }
  ...
  for (i = 0u; i < sizeof s->reserved; i++) {
    if (s->reserved[i] != 0u) {
      return IVS_E_BAD_STATE;
    }
  }
  ...
  return IVS_OK;
}
```

Copy: `const` view of the input, return a status, first failing check wins in a fixed documented order (D-33: length, magic, version, header, table, order/overlap, digest, word range), explicit loop bound over a count already checked, widen with `uint64_t` before any size arithmetic. The report struct (reason, segment index, byte offset, missing features) is the only thing written. No allocation, no libc beyond `memcpy`/`memset`/`memcmp` (S1; see H12 for `bcmp`).

### `core/src/machine/machine.{h,c}` (instance, lifecycle, load)

**Analog:** `core/src/cpu/cp1610.c` 1356-1416 (`ivs_cp1610_step`): argument refusal first, then state validation, then work on a stack copy, then one commit.

```c
  if (s == NULL || cfg == NULL || lines == NULL || port == NULL || port->bus == NULL || out == NULL) {
    return IVS_E_BAD_ARG;
  }
  ...
  if (ivs_cp1610_state_validate(s) != IVS_OK) {
    return IVS_E_BAD_STATE;
  }
  n = *s;
  ...
  r.n_accesses = (uint8_t)u.n;
  *s = n;
  *out = r;
  return IVS_OK;
```

For the machine, D-10 chooses validate-then-commit: the validators take `const ivs_machine *` and build a stack plan, and the commit function returns `void` and is copy-only (no early return). The CPU's copy-and-swap is the right shape only for the 32-byte CPU state; do not stage the whole instance. D-11's precedence list is the order of the `if` ladder in `ivs_machine_load` (BAD_ARG, BAD_HANDLE, BAD_STATE, DUPLICATE_LOAD, OVERSIZED, MALFORMED, OUT_OF_RANGE, OVERLAP, DIGEST_MISMATCH, then commit), one `if` per class so a `prec.*` row proves each adjacent pair.

Reuse the embedded CPU as-is: `ivs_cp1610` (32 bytes, pointer-free, `reserved[8]` zero) goes into the instance by value; reset goes through `ivs_cp1610_reset` (`cp1610.c` 241-250, which `memset`s and sets `RS_RESET_PENDING`). The CPU config defaults are currently hard-coded in `ivs_cp1610_config_default` (231-235: `0`, `2`, `35`); the machine layer should take them from the generated profile registry (`cpu.interrupt_accept_clears_intff`, `bus.release_latency_cpu_cycles`, `bus.wait_limit_cpu_cycles`), and the IAB answers from `reset.r7` and `interrupt.iab_value`, as the profile notes already promise (profile line 172, `interrupt.iab_value` note: "The core holds no vector constant. The machine bus answers the IAB access with this value").

Storage rules to follow (D-01, D-02, D-08): `create` zero-fills all storage first, checks size and alignment, writes only out-parameters on refusal, and `destroy` zero-scrubs. `-Wcast-qual` (in `cmake/IvsWarnings.cmake`) will flag casting `const` away in a validator, which is the intended guard.

### `core/src/machine/memmap.c` (bus function; generated tables)

**Analog (access-kind switch):** `tests/harness/ivs_testbus.c` 52-86.

```c
  switch (a->kind) {
  case IVS_ACC_FETCH:
  case IVS_ACC_READ:
  case IVS_ACC_ADAR:
    reply.data = ivs_testbus_read(b, a->addr);
    break;
  case IVS_ACC_WRITE:
  case IVS_ACC_DW:
    ivs_testbus_poke(b, a->addr, a->data);
    break;
  case IVS_ACC_IAB:
    ...
```

The machine bus has the same kinds and the same signature (`ivs_bus_fn`, `cp1610.h` 62: `typedef ivs_bus_reply (*ivs_bus_fn)(void *ctx, const ivs_cp1610_access *a);`, port struct 66-70). Differences to implement per D-20: IAB returns the profile vector chosen by the reason flag (`IVS_ACC_F_RESET` 0x01 / `IVS_ACC_F_INTERRUPT` 0x02, `cp1610.h` 40-41), INTAK and strobes return data 0, wait is 0. Per D-19 dispatch is a `switch` on a device-kind enum (no function pointers).

**Analog (generated include):** `tests/cpu/cpu_vectors.c` line 16 `#include "ivs_vectors.inc"` and the generator's output form: `static const` arrays inside the including TU (`cmake/IvsVectorTable.cmake` 697-701). `-Wmissing-variable-declarations` (clang, `IvsWarnings.cmake` 15-17) rejects a non-static const table, so `ivs_profiles.inc` must be `static const` and included by exactly one TU. This is the part of the phase with no core precedent: see H2.

Width formula (D-17) as one small function, keyed by device: `read = (stored & rmask) | (fill & ~rmask)`; use `IVS_U16` (`cp1610.h` 23) for the final narrowing.

### `core/src/machine/digest.c` (canonical state digest)

No analog. See "No Analog Found". Build it from one field table (D-35), serialize big-endian by shifts, never `memcpy` a struct. Use the CPU's own validator `ivs_cp1610_state_validate` to decide which CPU bytes are meaningful (the 8 `reserved` bytes are validated zero and are not serialized).

### `core/CMakeLists.txt` (modify)

**Analog:** itself, lines 7-25.

```cmake
set(IVS_CORE_SOURCES
  src/cpu/cp1610.c
  src/cpu/cp1610_decode.c
)
...
  set(obj "ivs_core_obj_${stem}")
  add_library(${obj} OBJECT "${src}")
  target_include_directories(${obj} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
  target_link_libraries(${obj} PRIVATE ivs_warnings ivs_san)
...
target_include_directories(ivs_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
```

Add each new `.c` to `IVS_CORE_SOURCES` (explicit list, no `file(GLOB)`; the `c-build` step fails on a tracked core `.c` that is not listed). `stem` is `NAME_WE`, so every stem must be unique across directories (`ivsc`, `ivs_sha256`, `machine`, `memmap`, `digest`), because both `ivs_core_obj_${stem}` and `tests/faults.cmake`'s `ivs_fault_plain_${stem}` use it. Add `core/include` to the `PRIVATE` object include lines and as the `PUBLIC` include of `ivs_core` (Phase 4's runner is to get only `core/include`, so decide here whether `src` stays `PUBLIC`; tests currently reach `cpu/cp1610.h` through it). The generated-include rule goes here (see H2).

### `cmake/IvsMemmapTable.cmake` and the cartridge fixture generator

**Analog:** `cmake/IvsVectorTable.cmake` (exact; this is the project's stated pattern: "copy over deps", build tree only, fail closed). Copy, with a new prefix instead of `vt_` (the sweep generator did the same with `st_`):

Header contract (lines 1-20): usage line with `-DROOT -DOUT`, build-tree-only, required-variable guard:

```cmake
foreach(required ROOT FILES SKIPS OUT)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "IvsVectorTable: missing -D${required}")
  endif()
endforeach()
```

Helpers to copy by value (they are not shared, by design): `vt_fail` (22-25, context-prefixed `FATAL_ERROR`), `vt_type` (27-34), `vt_keys` (36-48, unknown key fails closed), `vt_hex` (50-59), `vt_uint` (61-72), `vt_cstr` (83-94, ASCII only, trigraph-safe), `vt_str` (96-107), `vt_name` (109-115, name to C constant from a fixed list).

Embed each source file's digest for the inventory step (lines 249-250, and the `ivs_test_source` array at 702-707):

```cmake
  file(SHA256 "${ROOT}/${rel}" digest)
  string(APPEND sources_c "  {\"${rel}\", \"${digest}\"},\n")
```

The test then lists these as `suite.sources`, the harness prints `# source <path> <sha256>` lines, and `tests-inventory` re-hashes every one (`tests-vectors-digest`). Do the same for the literal map and for every `data/cart/*.json`, so a stale generated table cannot pass. Source paths must match `[A-Za-z0-9._/-]`.

Mutation hook for a generator fault (lines 471-477 and 690-693): `-DMUTATE=<id>` changes one value, sets a flag, and the script fails if the target does not exist:

```cmake
  if(DEFINED MUTATE AND id STREQUAL MUTATE)
    if(NOT cycles_asserted)
      vt_fail("MUTATE target asserts no expect.cycles")
    endif()
    math(EXPR cycles "${cycles} + 1")
    set(VT_MUTATED 1)
  endif()
...
if(DEFINED MUTATE AND NOT VT_MUTATED)
  set(VT_CTX "${FILES}")
  vt_fail("MUTATE target '${MUTATE}' is not a vector id")
endif()
```

For `IvsMemmapTable.cmake` the mutation is "shift one region's `lo` by 1" (named fault `GEN_MAP_MUTATION`). Output banner (697-698): `/* Generated by cmake/IvsMemmapTable.cmake ... */ /* Build output only: never committed, never edited. */`. Write via `file(WRITE "${OUT}" "${out_c}")` (709).

Registration in CMake: copy `tests/CMakeLists.txt` 23-29 (`add_custom_command(OUTPUT ... COMMAND "${CMAKE_COMMAND}" -D... -P generator DEPENDS <data> <generator> ...)`) and `set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS <inventory>)` (line 21). Because the memmap table is consumed by a core TU, the command lives in `core/CMakeLists.txt` (H2).

Specifics for the memmap generator (from D-16/D-18 and `03-DISCUSS-memmap.md` B2): fail on `hi < lo`, a range not aligned to 16 words, overlap within one view, non-power-of-two `size_words`, unknown device kind or gate, missing or unknown key, unresolved region reference, a read-driving region inside the cart window. Profile hex convention: existing profile hex values are `"$1000"` strings (`reset.r7`, `interrupt.iab_value`), while the research proposed bare 4-digit hex; pick one and keep the generator's `hex` helper strict (decide in plan 1; `$XXXX` matches the file it edits).

### `data/profiles/intv-2609-ntsc-family-p1.json` and `data/expected/profile-fields.ids` (modify)

**Analog:** the existing field records. Every field is the same 8-key object; key set is fixed by `tools/check/schema/profile.txt`: `value tag cite conflict resolving_evidence revisit_trigger decision note`. Copy `reset.r7` (136-145) for a tier-4 field with a conflict id, and `crystal_hz` (77, value `{"num": 3579545, "den": 1}`) for an OBJECT value. An OBJECT or ARRAY value must be non-empty (`nonblank value`).

```json
    "reset.r7": {
      "value": "$1000",
      "tag": "tier4-community",
      "cite": "SRC-gi-cp1600-um-1975 SC.1, pdf 21 and pdf 27 ...; SRC-community-emu-docs for the $1000 default",
      "conflict": "C32",
      "resolving_evidence": "pending: Phase 2 boot-stub decision (D5)",
      "revisit_trigger": "at Phase 2 reset-vector design",
      "decision": null,
      "note": "..."
    },
```

Rules enforced by `profile.txt` and `kinds/profile.cmake`: `tag` must be an id of `data/tiers.json` `source_authority_tags`; `tag=unknown-default-chosen` requires non-blank `resolving_evidence` and `revisit_trigger`; `tier5-emulator-hint` fails (`profile-tier5-only`); `conflict` must match `^C[0-9]+$` AND exist in `data/discrepancies.json`, so C60-C62 must land in the same plan or before the profile fields that cite them. Every member of `fields` must be listed in `data/expected/profile-fields.ids` and vice versa (`profile-id-missing` / `profile-id-extra`).

Existing fields to rework in place (D-16, D-26, D-32): `cartridge.address_windows` (261-269, now "from the per-content descriptor", contradicts D-18; repoint to the `memmap.*` fields) and `system_images.exec_grom` (270-278, free text; D-14 needs machine-readable EXEC and GROM slot descriptors with size, width, base and `required`). Add `cartridge.features_supported` = 0 (D-32). New field-name hazard: see H4 (`memmap.cart_window` fails the glossary step).

### `data/memmap/intv-2609-ntsc-family-p1.literal.txt` (literal map)

**Closest analogs:** `data/expected/faults/cpu.ids` (a `#` header that documents the columns, then one row per line, read by `sed`/`awk`/`cmake` with a first-token rule) and `data/expected/cp1610-skips.ids`. No analog for the tiling and `src=SRC-id@locator` grammar. Conventions to keep (S2): ASCII, LF, `#` comments, one record per line, column header comment first, the record format documented in the checker's header comment. Content rules from D-21: authored from sources only (tier 5 refused; check against the same tag enum as the profile), must tile `0000-FFFF` per view with explicit undriven rows, `src` id must exist in `data/sources.json` (the registry ids are in `data/expected/sources.ids`), non-blank locator, `tag` from `source_authority_tags`, `conf=C<n>` an existing discrepancy id.

Provenance caution (H7): `tools/check/denylist.sh` scans this file. It may not contain the author names in `tools/check/denylist.txt`, and `jzintv` is allowed only on a line carrying `[cite-only]`. Cite community leads through `SRC-intv-wiki` / `SRC-community-emu-docs`, not by emulator name.

### `data/api/api-effects.json` and the `api-effects` gate step (record kind)

**Analog:** `data/claims.json` + `tools/check/schema/claim.txt` + `tools/check/kinds/claim.cmake` + `tools/check/steps/claims.sh` (exact: a JSON array of records, an `ids` inventory, a hook for cross-checks, a wrapper step).

Schema file to copy (whole of `schema/claim.txt`, adapted):

```
file data/claims.json
topkeys schema_version note claims
top schema_version NUMBER
top note STRING
top claims ARRAY
array claims
idkey id
inventory data/expected/claims.ids
keys id statement profile tier artifact depends_on standing note
req id STRING
...
hook claim
```

For api-effects: `file data/api/api-effects.json`, `array calls`, `idkey name`, `inventory data/expected/api-calls.ids`, `keys name header on_ok refusals`, `hook api-effects`. The generic engine gives, for free, tokens `api-effects-id-missing`, `-id-extra`, `-missing`, `-type`, `-blank`, `-duplicate-id`. The hook (copy the structure of `kinds/claim.cmake`: `hook_record(id rec)` at line 53 loops nested arrays with `json_type`/`string(JSON ...)`; `hook_finish()` at 147 emits stats with `gate_stat`) adds the nested checks from D-15: for each `refusals[]` row `{status, effect, test}`: status is a name in `data/expected/api-statuses.ids`; effect is `none` or a name in the registry; test is an id in `data/expected/tests/*.ids`; every non-OK status has at least one row; every `IVS_API` function declared in `core/include/ivs/*.h` is a `calls` entry. Read headers with `file(GLOB ... "${ROOT}/core/include/ivs/*.h")` so the control runner's copied tree is what is checked.

Wrapper step (`steps/claims.sh`, the whole 13-line file):

```sh
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
rc=0
out=$(cmake -DROOT="$ROOT" -DKIND=claim -P "$CHECK_DIR/records.cmake" 2>&1) || rc=1
printf '%s\n' "$out"
[ "$rc" -eq 0 ] || exit 1
stats=$(printf '%s\n' "$out" | sed -n 's/^-- claim pass records=[0-9][0-9]* //p')
[ -n "$stats" ] || exit 1
printf -- '-- claims pass %s\n' "$stats"
```

The step is a static checker (it reads headers, the status pin and the ids files, not test results), so its alphabetical position does not matter. If the planner prefers a standalone `tools/check/api-effects.cmake`, copy the skeleton of `tools/check/vectors.cmake` instead: required-variable guard with `-DROOT`, `include(lib.cmake)`, the capped `vfail` (76-83) and the exact-totals tail (1314-1328).

### `data/cart/*.json` (cartridge fixtures) and `FX-ivsc-vectors`

**Analog:** `data/cp1610/vectors/*.json` (vector files; each record has an `id` that is also the test id) and the generator pattern above. Shape from the control tree's toy file (`tools/check/controls/vectors-duplicate-id/overlay/data/cp1610/vectors/toy.json`): `schema_version`, `source`, `group` (equal to the file stem), `note`, then the array. Test-id regex in the vector generator is `[A-Za-z0-9.@_-]+` (`IvsVectorTable.cmake` 261); keep cart ids inside it. Use the id form `ivsc.<class><nn>-<slug>` (for example `ivsc.P01-min`, `ivsc.N24-word-400`) so the prefix names the suite family and the research inventory letters P, B, N, L, X survive.

Inventories (S2): the stems of `data/cart/*.json` go in a `data/expected/` list like `cp1610-vector-files.ids`; the generator fails closed on a file not listed and on a listed file that does not exist (`IvsVectorTable.cmake` 209-247). Ids in the JSON must equal the first tokens of `data/expected/tests/ivsc-parse.ids` / `ivsc-load.ids`; `tests-inventory` enforces this at the gate (`tests-missing`, `tests-unlisted`), so no extra step is required.

Hygiene: no `.ivsc`, `.rom` or `.bin` may be committed (D-34). `.gitignore` already lists `*.bin *.rom *.cfg` but not `.ivsc`, and `tools/check/binary-ext.txt` does not list `ivsc`; consider adding both so a stray container is caught by name and not only by the content scan (it would fail the `files` step as an unlisted binary either way).

Manifest `fixtures/FX-ivsc-vectors/manifest.json`: copy `tools/check/controls/_good/tree/fixtures/FX-good/manifest.json` (an effective-yes, committed, `original-source` entry with `licence_evidence` pointing at the repo `LICENSE`, `rights_review.status` `author-attested`, `redistributable: yes`, `allowed_use` `["local-test","ci","repo"]`). Resolved question from the research (cart UNK): the manifest checker accepts `outputs[].path` outside `fixtures/<id>/` (it only checks the path charset and that the file exists with the recorded size and sha256; `manifests.cmake` 286-318), so listing `data/cart/*.json` is valid. Cost to plan for: every edit to a cart JSON changes its sha256, so the manifest `outputs` must be refreshed in the same commit. Add `FX-ivsc-vectors` to `data/expected/fixtures.ids`.

### `data/discrepancies.json`, `data/claims.json` and their inventories (modify)

**Analog:** discrepancy entries C57-C59 (keys: `id topic sources status default_policy resolver evidence note`). Rules from `schema/discrepancy.txt` and `kinds/discrepancy.cmake`: ids ascend (`discrepancy-order`), `resolver` must match `^(test|measurement|pending):.+`, an `open` or `partial` entry's `default_policy` must start with `unknown-default-chosen: `, a `resolved-primary` entry needs non-blank `evidence`. C21's resolver changes to `measurement: D10 ...` (D-23); C51 becomes `status: partial`; new C60, C61, C62 are appended in order, and `data/expected/discrepancies.ids` gets three lines.

Claim for MEM-01: copy `CL-0003` (`profile`, `tier: source-expectation`, `artifact` an existing repo path, `depends_on` field paths that exist in the profile, `standing: provisional`). `kinds/claim.cmake` forces `provisional` when any depended-on field is tier 4, tier 5, `inference` or `unknown-default-chosen` (`claim-overstated`), and ids must ascend (`CL-0006`...). Add the id to `data/expected/claims.ids`. State the weak provenance in the claim note (D-21: transcription consistency, not hardware truth).

### `docs/formats/ivsc-v1.md` (format spec)

**Analog:** `docs/cp1610/stepper-conventions.md` (a note that opens by saying what it is for and what it excludes, one fact per sentence per line, `[OBS]`/`[INF]` labels, cite by `pdf N` / discrepancy id) and `docs/time-base.md`. Constraints enforced on every `docs/**/*.md` by gate steps (H8): the words `supports`, `compatible`, `passes`, `accurate`, `verified`, `runs` need a `[claim:CL-NNNN]` tag or `[nc]` on the same line (step `text`); the bare word `clock` fails unless the line contains `input clock`, `STIC clock`, `PSG chip clock` or `[nc]` (step `glossary`); `jzintv` only on a line carrying `[cite-only]`; ASCII, LF, no BOM.

### Test suite `machine` (`tests/machine/machine_*.c`, one executable)

**Analog:** `tests/cpu/cpu_api.c` (exact: plain-function cases, a per-file `IVS_CHECK` macro, fixtures built by small `static` helpers, an alphabetically ordered case table, `main` builds the suite).

Per-file macro (15-22), copy verbatim:

```c
#define IVS_CHECK(cond)                                                                                \
  do {                                                                                                 \
    if (!(cond)) {                                                                                     \
      ivs_test_diag("check failed at line %d: %s", __LINE__, #cond);                                   \
      return IVS_TEST_FAIL;                                                                            \
    }                                                                                                  \
  } while (0)
```

Case table and `main` (991-1023):

```c
static const ivs_test_case api_cases[] = {
    {"cpu-api.access-size-12", api_access_size_12, NULL},
    ...
};

int main(int argc, char **argv)
{
  ivs_test_suite suite;
  ivs_test_suite_from_cases(&suite, "cpu-api", api_cases, sizeof api_cases / sizeof api_cases[0]);
  return ivs_test_main(argc, argv, &suite);
}
```

Isolation cases copy `api_two_instances_interleaved` (the whole function ends the file's case list; it runs A and B alone, then interleaved A,B,A,B and compares results, logs and final state) and the fixture helper `api_setup` (220-232). The machine versions compare whole-instance `memcmp` and the state digest (D-36). Storage for the instances: file-scope `static` aligned arrays are fine in tests (the CPU tests use `static ivs_testbus api_bus_a`, line 200); the aliasing footgun in `03-DISCUSS-api.md` means also run one case with `malloc` storage.

Test ids: `<suite>.<slug>` lowercase with hyphens (`cpu-api.state-size-32`). For `machine`: `machine.refuse.<op>.<slug>`, `machine.prec.<higher>-over-<lower>`, `machine.iso.<slug>`, `machine.digest.<slug>`, `machine.sha256.<slug>`, `machine.sweep.<slug>` (D-15 puts refusal, isolation, digest and sha256 in one suite and executable). Every id must appear in `data/expected/tests/machine.ids` (header comment line `# Suite machine: ...` like `cpu-api.ids`), and the suite must be registered with `add_test` plus `SKIP_RETURN_CODE 77` (S6). Sweeps are the oracle-free pattern: every truncation and byte-change of a fixture is either OK or one allowed refusal code, and a refusal implies byte-identical instance storage.

### Test suite `memmap` (generated per-row cases)

**Analog:** `tests/cpu/cpu_sweep.c` (one case per generated row plus a few extras; ids built at startup; table generated from a data file; `sources` handed to the harness). Lines 308-340:

```c
#define SWEEP_CASES (IVS_SWEEP_ROWS + 1u)
static char sweep_ids[SWEEP_CASES][40];
...
  for (i = 0u; i < IVS_SWEEP_ROWS; i++) {
    (void)snprintf(sweep_ids[i], sizeof sweep_ids[i], "cpu-sweep.%s", ivs_sweep_row_id[i]);
  }
  ...
  suite.name = "cpu-sweep";
  suite.count = SWEEP_CASES;
  suite.id = sweep_id;
  suite.run = sweep_run_case;
  suite.sources = ivs_sweep_sources;
  suite.n_sources = sizeof ivs_sweep_sources / sizeof ivs_sweep_sources[0];
  return ivs_test_main(argc, argv, &suite);
```

Use one case per literal-map row (id `memmap.row.<view>-<lo>-<hi>`) with the boundary probes `lo-1, lo, hi, hi+1` and mirror wrap points inside the case (D-22), not one id per probe, to keep `data/expected/tests/memmap.ids` near the size of `cpu-sweep.ids` (77 lines) instead of `cpu-vectors.ids` (413). Expected values come from a small rule table in the test keyed on device and width, never from the profile descriptors under test.

The dump producer for the `memmap` gate step (probing all 65,536 addresses in both views through the real bus function) is a test-side writer; see H6 for how its output reaches a pure checker.

### Test suites `ivsc-parse`, `ivsc-load` (generated fixtures)

**Analog:** `tests/cpu/cpu_vectors.c` (exact for suite wiring: a generated table `ivs_vec_table`, `id`/`skip`/`run` callbacks over it, `sources` for re-hash). `main` (275-288):

```c
  suite.name = "cpu-vectors";
  suite.count = ivs_vec_count;
  suite.data = ivs_vec_table;
  suite.id = ivs_vec_id;
  suite.skip = ivs_vec_skip;
  suite.run = ivs_vec_run;
  suite.sources = ivs_vec_sources;
  suite.n_sources = ivs_vec_source_count;
```

Table types go in a harness header like `tests/harness/ivs_vec.h` (plain structs, an `ivs_vec_word`-style `{addr, word}`; see its top comment: "Nothing here is committed generated output: ivs_vectors.inc lives in the build tree only"). Patch-from-base negatives, resealing through the core SHA-256 test helper, and maximum-size images built in test code are D-34's rules; there is no existing analog for the patch/reseal step (see "No Analog Found").

### Test suite `ivsc-fuzz` (seeded)

**Analog:** `tests/cpu/cpu_fuzz.c` (exact). Each case is one fixed 32-bit seed through an in-repo xorshift32 (59-65), no host randomness or time:

```c
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return x;
```

Ids `ivsc-fuzz.seed-NN` in `data/expected/tests/ivsc-fuzz.ids` (see `cpu-fuzz.ids`). Invariants to copy from the header comment (1-30): defined status on every input, determinism (repeat from a copy), refusal leaves the state untouched, and a per-seed non-vacuity check (every refusal reason reached at least once). Half the mutations must be resealed or the digest check masks everything behind it.

### Public-header self-containment and `IVS_API` list test

**Analog (partial):** `tests/core-symbols-tap.sh` (a ctest suite that is a shell script and emits harness-format TAP itself) and the ctest registration at `tests/CMakeLists.txt` 84-95:

```cmake
add_test(NAME core-symbols
  COMMAND sh "${CMAKE_SOURCE_DIR}/tests/core-symbols-tap.sh" "$<TARGET_FILE:ivs_core>"
          ${IVS_SYMBOL_PREFIX_ARGS})
set_tests_properties(core-symbols PROPERTIES SKIP_RETURN_CODE 77
  ENVIRONMENT "NM=${CMAKE_NM};AR=${CMAKE_AR}")
```

`core-symbols-tap.sh` 16-39 shows the TAP shape a non-C suite must print: header `# suite=<s> nonce=<IVS_TEST_NONCE or none>`, plan `1..N`, `ok`/`not ok <id>`, trailer `# suite=<s> executed=<n> skipped=<k> failed=<f>`, and a copy to `$IVS_TEST_OUT_DIR/<suite>.txt`. A header-self-containment test can instead be a C executable that includes each public header alone as its first include (compiled in the normal build, so no new script); a C++ include check must skip visibly (exit 77) when no C++ compiler exists.

### `tests/CMakeLists.txt` (modify) and `tests/faults.cmake` (modify)

**Analog:** themselves. Per new suite copy the four-line block (58-61):

```cmake
add_executable(ivs_tests_cpu_api cpu/cpu_api.c)
target_link_libraries(ivs_tests_cpu_api PRIVATE ivs_test_harness ivs_core ivs_warnings ivs_san)
add_test(NAME cpu-api COMMAND ivs_tests_cpu_api)
set_tests_properties(cpu-api PROPERTIES SKIP_RETURN_CODE 77)
```

Executable name `ivs_tests_<suite with underscores>`; ctest name equals the suite name (this is what `tests-inventory` checks against `ctest -N`). Generated-table suites also copy the custom-command block (23-29, 41-56) and `target_include_directories(... PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")`.

`tests/faults.cmake` generalization (D-37): the file hard-codes `IVS_FAULT_IDS` to `faults/cpu.ids` (13), the test source `cpu/cpu_vectors.c` (53), the vector include (54-55), the single generator control name `GEN_EXPECT_MUTATION` (72-75) and the plain-object include dirs (28, 44, 55: only `core/src`). To loop over `data/expected/faults/*.ids` map each `# suite=<name>` header to its test sources (keep the map in `faults.cmake`, not in the ids files, which stay pure inventory), keep `ivs_fault_<ID>` target names (the `c-faults` step runs `build/dev/tests/ivs_fault_<ID>`), and keep the rule that each non-`GEN_` id appears in exactly one core TU (88-98). `GEN_MAP_MUTATION` differs from `GEN_EXPECT_MUTATION`: the mutated `ivs_profiles.inc` is consumed by a core TU, so the fault executable must recompile `memmap.c` with the mutated include directory placed before the real one (H2).

### Gate steps `c-faults.sh` and `faults-verdict.sh` (modify to loop over `faults/*.ids`)

**Analog:** themselves.

`c-faults.sh` 22-31 reads one file and one suite header; 50-67 runs each id once and requires a result file `<results>/faults/<ID>/<suite>.txt`:

```sh
ids="$ROOT/data/expected/faults/cpu.ids"
...
suite=$(sed -n 's/^#[[:space:]]*suite=\([a-z0-9][a-z0-9-]*\).*$/\1/p' "$ids" | head -n 1)
...
  exe="$ROOT/build/dev/tests/ivs_fault_$id"
  ...
  IVS_TEST_OUT_DIR="$out_root/$id" IVS_TEST_NONCE="$nonce" "$exe" > "$tmp/$id.log" 2>&1 || :
```

`faults-verdict.sh` 37-52 loads `data/expected/tests/$suite.ids` for the suite, 57-114 judges each id's TAP against the four columns, and 118-133 compares `#ifdef IVS_FAULT_<ID>` sites in `core/src/**/*.{c,h}` against the non-`GEN_` ids:

```sh
sort -u "$tmp/fault-ids" -o "$tmp/fault-ids"
awk '{ print $1 }' "$tmp/sites" | sort -u > "$tmp/site-ids"
...
for s in $(comm -23 "$tmp/fault-ids" "$tmp/site-ids"); do bad fault-missing-site "$s"; done
```

Loop body per file; the orphan/missing-site comparison must use the union of ids over all files, otherwise every machine fault is reported as `fault-orphan-site` (H3). The pass line stays `-- faults-verdict pass faults=<ok>/<total>` with totals summed. Also `tools/check/vectors.cmake` (1270-1273 and 1309) reads `data/expected/faults/cpu.ids` by path for `vector-policy-no-fault`; that stays CPU-only and needs no change.

Fault file form to copy for the four new files (`data/expected/faults/cpu.ids` lines 1-3):

```
# suite=cpu-vectors; columns: <FAULT_ID> <must-fail-vector-id> <allowed-prefixes> <must-pass-vector-id>
# Named-fault controls (D-18). Prefixes are |-separated vector-id prefixes; GEN_ ids are
# generator-path controls with no source site; every other id has one IVS_FAULT_<ID> site in core/src.
GEN_EXPECT_MUTATION cpu.CLRC.clears-c cpu.CLRC.clears-c cpu.SETC.sets-c
RESET_ENTRY cpu.UNIT-RESET.provider-ffff cpu.UNIT-RESET. cpu.SETC.sets-c
```

Each row: a must-fail test id, the allowed failing prefixes (collateral failures outside the prefixes are `fault-wrong-reason`), and a must-pass sensitivity id (a must-pass failing is `fault-collateral`). Start from the fault lists in `03-DISCUSS-refusal.md` section 7, `03-DISCUSS-memmap.md` section 4, `03-DISCUSS-content.md` 2c and `03-DISCUSS-cart.md` 4.5.

Site shape in C (copy `core/src/cpu/cp1610.c` 87-92 and 264-272): at most three lines, `#ifdef IVS_FAULT_<ID>` on its own line, one site per id in exactly one core TU, and the faulty statement placed after the good one so the file stays readable:

```c
  uint16_t v = ivs_access(u, nominal, IVS_ACC_IAB, flags, 0u, 0u);
#ifdef IVS_FAULT_IAB_HARDCODED_1004
  v = 0x1004u;
#endif
  return v;
```

Fault executables link `ivs_warnings` only, never `ivs_san` (so a sanitizer abort cannot pass for a detection); over-read faults cannot be tested this way and are covered by ASan in the normal dev run (`03-DISCUSS-refusal.md` 7).

### `tools/check/steps/memmap.sh` and `tools/check/memmap.cmake`

**Analog:** `tools/check/steps/cp1610-vectors.sh` (wrapper, exact) and `tools/check/vectors.cmake` (checker, role-match). Wrapper skeleton (`cp1610-vectors.sh` 14-23, 42-44):

```sh
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/vectors.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
out=$(cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -DCHECK_DIR="$CHECK_DIR" -P "$CHECK_DIR/vectors.cmake" 2>&1) || rc=1
printf '%s\n' "$out"
stats=$(printf '%s\n' "$out" | sed -n 's/^-- vectors pass //p')
...
[ "$rc" -eq 0 ] || exit 1
[ -n "$stats" ] || exit 1
printf -- '-- cp1610-vectors pass %s blind-files=%s second-read=%s/%s\n' ...
```

The step name must equal the script basename (`gate.sh` greps `-- <name> pass`), so the pass line is `-- memmap pass <stats>`; findings are `-- FAIL <token>: <detail>`; the header comment of the `.sh` lists every token and every stat (S3). `memmap.cmake` copies the `vectors.cmake` skeleton: required-variable guard, `include("${CMAKE_CURRENT_LIST_DIR}/lib.cmake")`, a capped `vfail` (76-83), `first_tokens`/`read_ids` helpers from `lib.cmake`, `gate_stat` and `gate_finish(<name>)` (1322-1328). Token set to implement (from `03-DISCUSS-memmap.md` section 4 and D-21): `memmap-literal-hole`, `memmap-literal-overlap`, `memmap-literal-no-cite`, `memmap-literal-unknown-source`, `memmap-literal-tier5`, `memmap-dump-diff`, plus the generator refusals `memmap-gen-overlap`, `-misaligned`, `-nonpow2-mirror`, `-unknown-device`, `-unknown-key`, `-cart-overlap`. Source-id and tier checks reuse `data/sources.json` ids and `data/tiers.json` tags exactly as `vectors.cmake` does (reference sets at 193-206).

The `memmap` step is sorted after `c-tests` (alphabetical), so a dump written by the machine tests is already in the results dir when it runs.

### `tools/check/steps/c-purity.sh` and `tools/check/core-symbols.sh` (modify)

**Analog:** themselves. Four edits (H1, H2, H5):

- 46-47 collect `core/src/**` `.c` and `.h` from FILELIST; headers under `core/include/**` must be in the header list too, so both the scan loop (144) and the quoted-include resolution see them.
- 96 resolves a quoted include only against `core/src/<h>` or the including file's directory: `if (!(("core/src/" h) in corehdr) && !((dir "/" h) in corehdr)) report("c-purity-include")`. `#include "ivs/ivs_status.h"` and a generated `#include "ivs_profiles.inc"` both fail here today.
- 159 compiles each core `.c` with `-I"$ROOT/core/src"` only; add `-I"$ROOT/core/include"` and, for the generated include, run the generator into the step's own temp dir and add that directory (the step "writes only to its temp dir").
- `core-symbols.sh` nm calls (`run_nm ... -P -u` and `-P -g --defined-only`, lines 115-116) see only external symbols (`-g`). The writable-data rule (D-07) needs `nm -P` without `-g` to see file-local statics (types `d D b B c C`; Darwin `s` is ambiguous). Token `core-symbols-writable`; run it on the plain `-O0`/`-O2` archives that `c-purity` builds, NOT on the sanitized dev archive checked by the `core-symbols` ctest (ASan/UBSan add writable symbols there).

New controls for both: copy `controls/core-purity-alloc/overlay/core/src/cpu/good.c` (a stand-in core file with the offending construct) for a writable static. The `_good` stand-in core must keep passing every step (`_good/tree/core/src/cpu/good.c`).

### Negative controls (`tools/check/controls/<name>/`) and `_good/tree` additions

**Analog:** the existing control directories (exact). Layout (S4): `CHECK` (the step name, one line), `EXPECT` (one or more lines, each a literal substring that must appear in the output; never blank), optional `FORBID` (substrings that must NOT appear, to prove the control fails for its own reason), optional `overlay/` (files copied over a fresh copy of `controls/_good/tree/`), optional `MAKE` (transform lines run on the copy: `delete`, `empty`, `write`, `rename`, `crlf`, `bom`, `nonascii`, `nul`, ... see `run-controls.sh` 74-133).

```
faults-not-detected/CHECK:   faults-verdict
faults-not-detected/EXPECT:  FAIL fault-not-detected:
faults-not-detected/overlay/build/gate/faults/GOOD_FAULT/toy.txt   (a TAP file in which the must-fail id passed)
claims-inventory-empty/MAKE: empty data/expected/claims.ids
manifests-digest-mismatch/MAKE: write fixtures/FX-good/payload.txt changed
faults-missing-site/FORBID:  fault-missing:
```

Naming: `<step-family>-<what>`; the token in `EXPECT` is the exact `FAIL <token>:` the step prints. Every control directory must be listed in a `data/expected/controls/<family>.ids` file (first token per non-`#` line), with a header comment naming the step and the plan (`# Negative controls for the faults-verdict step (plan 02-04).`); the runner reports `control-not-in-inventory` / `control-inventory-orphan` on a mismatch.

Positive control: the runner runs every distinct `CHECK` step once on the pristine `_good/tree` and fails with `control-good-failed` unless it prints `-- <step> pass`. So each new step (`memmap`, `api-effects`) and each changed step needs a toy input in `_good/tree` that passes it: a toy profile with `memmap.*` fields (or a toy literal map plus dump), a toy `data/api/api-effects.json` with its headers, status pin and ids, a toy second faults file with its own `# suite=` header (the existing toy is `_good/tree/data/expected/faults/cpu.ids`, suite `toy`), and toy dump files under `build/gate/<preset>/` shaped like the existing `build/gate/dev/toy.txt`. Controls for the faults-loop change (D-37): one with a site that belongs to a second ids file (must not be `fault-orphan-site`), one with an id listed in two files.

The control count drives gate time (the controls overlap the steps, `controls-wait` 0 so far; H13): prefer many one-line `MAKE` controls to many full-file overlays.

### `data/expected/**` inventories (S2)

- `data/expected/gate-steps/machine.ids`: header comment then one step name per line (`api-effects`, `memmap`); copy `gate-steps/cpu.ids`. `gate.sh` fails `gate-step-missing` / `gate-step-extra` if the step scripts and the union of these files differ.
- `data/expected/tests/<suite>.ids`: first token is the id; an optional second token `skip-ok` permits that id to be skipped; header comment names the suite and the plan. One file per ctest name. Sorted order is not required (`cpu-api.ids` is in declaration order).
- `data/expected/controls/<family>.ids`, `faults/<name>.ids`, `fixtures.ids` (`FX-` ids, one per manifest), `profile-fields.ids` (exact member set of `fields`), `discrepancies.ids`, `claims.ids`: exact-set inventories, ASCII, LF.

### `.planning/CI-EFFICIENCY.md` (append)

The file is an append-only log (its own header says so; latest entry "2026-11-11 hosted run 42942719332", budget text at line 169 and 176). D-38's re-baseline (30 s to 40 s, revisit trigger 40 s to 50 s) is a new dated section, not an edit of old rows. Agents do not call `gh`; the orchestrator records hosted figures.

## Shared Patterns

### S1. Core C source discipline
**Source:** `core/src/common/ivs_prelude.h`, `core/src/cpu/cp1610.c` 1-5, `tools/check/steps/c-purity.sh` header.
**Apply to:** every new `core/**/*.c` and `core/**/*.h`.

- First line `/* SPDX-License-Identifier: MIT */`; second a block comment saying what the file is, with the decision ids (`D-xx`) and `pdf N` cites.
- In a `.c` file the FIRST include is `#include "common/ivs_prelude.h"` (`c-purity-prelude`). The prelude allows only `limits.h stdbool.h stddef.h stdint.h string.h`, and poisons `float`/`double` (`#pragma GCC poison float double`); a floating literal also fails (`c-purity-float-literal`).
- Only those five standard headers may be included by angle brackets; quoted includes must name a header the step can resolve in the tracked tree (H1).
- Banned words in core: `asm`, `__asm`, `__rdtsc`, `syscall`, and any host service. Allowed libc imports are `memcpy memmove memset memcmp` (and the `_chk` forms, `bzero`, stack-protector names) in `tools/check/core-symbols-allow.txt`; anything else is `core-symbols-forbidden`.
- Naming: functions, types, static helpers `ivs_*`; macros `IVS_*`; hex constants `0x....u`; fixed-width types only; every 16-bit store through `IVS_U16`; booleans as `uint8_t` 0/1; no enums or bitfields in state or in headers.
- Compiler flags to satisfy (`cmake/IvsWarnings.cmake` 11-18, `-Werror` in both presets): `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wstrict-prototypes -Wmissing-prototypes -Wvla -Wundef -Wcast-qual -Wimplicit-fallthrough -Wdouble-promotion`, plus clang `-Wmissing-variable-declarations`. So: every non-static function has a prototype in an included header; every file-scope variable is `static`; `case` fall-through is explicit; no narrowing without a cast.
- No heap, no globals that are writable, no clock, no file I/O, no `printf` (D-07, API-05).

### S2. Inventory (`.ids`) files
**Source:** `tools/check/lib.cmake` `read_ids` (62-100), `data/expected/**`.
**Apply to:** every new list under `data/expected/`.

One id per line, `#` comment lines, blank lines ignored, first token is the id, ASCII, LF, no duplicates; an intentionally empty list needs `#allow-empty: <reason>`. Pure shell readers use `awk '/^#/ { next } NF { print $1 }'`; cmake readers use `read_ids` or `first_tokens` (`vectors.cmake`). A header comment states the suite, the columns and the owning plan. An inventory is an exact set: a missing and an extra id are both failures.

### S3. Gate step shape
**Source:** `tools/check/steps/cp1610-vectors.sh`, `claims.sh`, `tests-inventory.sh` header, `tools/gate.sh` 65-100.
**Apply to:** `memmap`, `api-effects`, and the edited `c-faults`, `faults-verdict`, `c-purity`.

- POSIX `sh`, `LC_ALL=C; export LC_ALL`, `: "${ROOT:?ROOT is not set}"` guards for `ROOT`, `CHECK_DIR` (and `FILELIST` when used), `tmp=$(mktemp -d ...)` with `trap 'rm -rf "$tmp"' EXIT` and `trap 'exit 1' HUP INT TERM`.
- Output contract: failures `-- FAIL <token>: <detail>` (sorted, capped per token where the count can be large), success a single `-- <step-name> pass <stat>=<n> ...` line that `gate.sh` re-reads; a step that exits 0 without that line is `gate-step-no-pass-line`.
- A header comment lists the step's rules, its tokens and its stats.
- Producer/judge split: a step that needs a built binary (`c-build`, `c-tests`, `c-faults`) writes results under `$IVS_GATE_DIR` (default `$ROOT/build/gate`), and a pure checker (`tests-inventory`, `faults-verdict`) reads only that results dir, `data/expected/` and FILELIST, so the control runner can drive it on a text-only tree. Every new check that depends on a binary must follow the split (H6).
- Steps run in alphabetical order of file name and never stop at the first failure.

### S4. Negative-control layout
**Source:** `tools/check/run-controls.sh` 1-12, 135-200; `tools/check/controls/*`.
**Apply to:** all new controls. See the Pattern Assignment above.

### S5. cmake -P generators and checkers
**Source:** `cmake/IvsVectorTable.cmake`, `cmake/IvsSweepTable.cmake` 22-33, `tools/check/vectors.cmake`, `tools/check/lib.cmake`.
**Apply to:** `IvsMemmapTable.cmake`, the cart table generator, `memmap.cmake`.

Required `-D` guard, `cmake_minimum_required(VERSION 3.25)`, helpers copied per file with their own prefix, `string(JSON ... ERROR_VARIABLE e)` through a `*_type` helper that returns `MISSING`, unknown key = failure, build-tree output only, `file(SHA256)` embedded for re-hash, `-DMUTATE` for the generator fault. Checkers (`tools/check/*.cmake`) use `gate_fail`, `gate_stat`, `gate_finish`; build-time generators (`cmake/*.cmake`) use `message(FATAL_ERROR ...)` with a context prefix.

### S6. Test wiring and TAP
**Source:** `tests/harness/ivs_test.h` 22-50, `tests/CMakeLists.txt` 58-61, `tests/cpu/cpu_api.c`.
**Apply to:** every new suite.

Suite = one executable `ivs_tests_<name>` linking `ivs_test_harness ivs_core ivs_warnings ivs_san`, registered with `add_test(NAME <suite> ...)` and `SKIP_RETURN_CODE 77`; exit codes 0 / 1 / 77 from the harness; TAP written to `$IVS_TEST_OUT_DIR/<suite>.txt` with the nonce; ids unique within the suite (the harness rejects a duplicate before running anything); an `--only <prefix>` filter selecting nothing exits 1. Ids are `<suite>.<slug>` or, for generated tables, the id in the data file. `tests-inventory` requires every `data/expected/tests/*.ids` stem to be a ctest name, and every suite to have a TAP file in BOTH `dev` and `release` presets (H9).

### S7. Named faults
**Source:** `tests/faults.cmake` header (1-12), `data/expected/faults/cpu.ids`, `core/src/cpu/cp1610.c` 87-92.
**Apply to:** machine, memmap, sha256 and ivsc faults.

One `#ifdef IVS_FAULT_<ID>` site (a `#if` line naming the id; at most three lines of code) in exactly one `core/src` TU per non-`GEN_` id; `GEN_<NAME>` ids are generator-path controls with no site; a fault executable is the owner TU compiled with `-DIVS_FAULT_<ID>`, plain (non-sanitized) copies of the other TUs, and no `ivs_san`; fault executables are `EXCLUDE_FROM_ALL`, never registered with ctest; `c-faults` runs them, `faults-verdict` judges them. A fault is only credited if its must-fail test fails, every failing id matches an allowed prefix, and its must-pass sensitivity id passes.

### S8. Record kinds and cross-file referential rules
**Source:** `tools/check/records.cmake`, `tools/check/schema/*.txt`, `tools/check/kinds/*.cmake`.
**Apply to:** profile, discrepancies, claims, `api-effects`.

The engine reads a `schema/<kind>.txt` (`file`, `topkeys`, `top`, `array|members`, `idkey`, `inventory`, `keys`, `req`, `nonblank`, `match`, `enumfrom`, `when`, `hook`) and an optional hook with `hook_record(id rec)` and `hook_finish()`. Cross-file references are by id: profile `conflict` to `discrepancies.json`, `tag` to `tiers.json`, claim `depends_on` to profile field paths, `cite` source ids to `sources.json`. A change in one file usually needs its inventory and its referent in the same plan.

### S9. Text hygiene on every tracked file
**Source:** `tools/check/integrity.cmake` 40-75, `steps/denylist.sh`, `steps/text.sh`, `steps/glossary.sh`, `steps/files.sh`.
**Apply to:** every new tracked file.

- `integrity`: ASCII only under `data|fixtures|tools|core|include|tests|cmake` and root `CMakeLists.txt`; LF only, no BOM, no empty file, no NUL; path charset `[A-Za-z0-9._/-]`; an `SPDX-License-Identifier: MIT` line in every `*.sh`, `*.cmake` under `tools/`, every `.c`/`.h`/`.sh`/`.cmake` under `core|include|tests`, `cmake/*.cmake`, `CMakeLists.txt` files (JSON, `.ids`, `.txt`, `.md` carry none).
- `files`: no tracked ROM-like or binary file unless it is a committed output of an effective-yes manifest; no file under `fixtures/<dir>/` that the manifest does not list; no `refs/` path.
- `denylist`: author names in `tools/check/denylist.txt` are never allowed; `jzintv` only with `[cite-only]` on the same line.
- `text` and `glossary` on `README.md` and `docs/**/*.md` (H8).

## Gate and build coupling hazards (read first)

**H1. Public headers break `c-purity` and the include paths.** Moving `ivs_status.h` to `core/include/ivs/` and adding `ivs.h`/`ivs_machine.h` means `#include "ivs/ivs_status.h"` in `cp1610.h` and in new TUs. Today `c-purity.sh` collects headers only under `core/src` (line 47), resolves a quoted include only as `core/src/<h>` or relative to the includer (line 96) and compiles with `-I$ROOT/core/src` only (line 159). Without edits: `c-purity-include` on the first new include, then `c-purity-compile`. Edit `c-purity.sh` as described, add `core/include` to `core/CMakeLists.txt` (objects PRIVATE, `ivs_core` PUBLIC) and to the three include lines of `tests/faults.cmake` (28, 44, 55), and add a control that a header outside both trees still fails.

**H2. A generated include inside a core TU has no precedent.** `ivs_vectors.inc` and `ivs_sweep.inc` are consumed by tests; D-16's `ivs_profiles.inc` is consumed by a core TU. Consequences: (a) the generator rule goes in `core/CMakeLists.txt` with the binary dir on the TU's include path; (b) `tests/faults.cmake` builds plain and owner copies of every core TU, so each needs the same generated include directory; (c) `c-purity.sh` Stage 2 compiles raw core sources with `cc` outside CMake, so it must run `cmake -P cmake/IvsMemmapTable.cmake` into its temp dir and add `-I`; (d) Stage 1 rejects `#include "ivs_profiles.inc"` (not a tracked header), so name the generated file as an explicit exception or generate a `.h` and special-case its name, and keep `core-purity-computed-include` and `core-purity-quoted-header` green; (e) `GEN_MAP_MUTATION` must rebuild the core TU that includes the table with the mutated directory first on the include path, which is more than `GEN_EXPECT_MUTATION` needed. Settle (a)-(e) in the first plan; it is the largest unknown in the phase.

**H3. The faults trio hard-codes `cpu.ids`.** `tests/faults.cmake` (13, 53, 72-75), `c-faults.sh` (22-31) and `faults-verdict.sh` (37-52, 118-133) each name `data/expected/faults/cpu.ids` and one suite. The orphan/missing-site comparison must use the union of ids across files, executable names `ivs_fault_<ID>` must be unique across files, and each ids file's `# suite=` must map to a built test source. Keep `controls/faults-*` green by leaving `_good/tree/data/expected/faults/cpu.ids` (suite `toy`) valid.

**H4. `memmap.cart_window` fails the glossary step.** `steps/glossary.sh` (d) fails a profile field whose last path segment contains `window`, `offset`, `hold`, `duration`, `delay`, `period`, `latency`, `interval` or `timeout` between underscores unless it ends in `_ticks` or `_cpu_cycles`. Checked with the same `case` logic: `cart_window` FAILS, `address_windows` (plural, already in the profile) is not matched, `cart_align_words` and `undriven_read` pass. D-16 names `memmap.cart_window`; rename it (for example `memmap.cart_range`) or amend the glossary rule in the same plan. Region names under `memmap.region.<name>` and device kinds under `memmap.device.<kind>` must avoid those tokens too.

**H5. `core-symbols` writable-data rule needs file-local symbols.** `core-symbols.sh` lists defined symbols with `-g`, which omits file-local statics, and D-07's rule is about statics. Run it on the plain c-purity archives (not the sanitized dev archive checked by the ctest suite `core-symbols`), expect Darwin and ELF to differ (`.data.rel.ro`, Darwin `s`), and keep behavioural isolation tests as the primary proof (D-07 already says so).

**H6. The `memmap` dump check must be a pure checker.** The control runner copies a text tree and runs a step on it, so `memmap` cannot build or run C. Follow the `c-faults` to `faults-verdict` split: a test executable writes the 65,536-address dump (both views, run-length merged into the literal map's structural columns) under `$IVS_TEST_OUT_DIR`, and the step compares it with the literal map's projection. `ivs_test_main` writes exactly one file, `<dir>/<suite>.txt` (`ivs_test.c` 171-210), so either add a second output file to the harness (then update `harness_selftest.c` and `data/expected/tests/harness.ids`) or carry the dump as `#` lines inside the suite's TAP the way `core-symbols-tap.sh` carries script output. Dump files exist per preset (`build/gate/dev`, `build/gate/release`); `memmap` sorts after `c-tests`.

**H7. Provenance scan on new data and docs.** See S9. The literal map, cart fixtures and `docs/formats/ivsc-v1.md` are scanned by `denylist`; the research files in `.planning/` are not.

**H8. Doc tripwires.** `text` (claim words need `[claim:CL-NNNN]` or `[nc]`), `glossary` (bare `clock`), and the evidence labels: write the spec with `[OBS]`/`[INF]` labels per `docs/evidence-tiers.md`.

**H9. `tests-inventory` is strict and runs both presets.** `presets="dev release"` is hard-coded (line 31). Every new suite must have an ids file, a ctest registration, and a TAP file in each preset in the SAME plan; a suite that exists only in one is `tests-ctest-drift` or `tests-suite-missing`. Dev runs under ASan+UBSan, so a 10^6-byte SHA-256 vector and the sweeps cost more there than in release.

**H10. `c-build-unlisted-source` and unique stems.** Every tracked `core/**/*.c` must appear in `IVS_CORE_SOURCES`; stems must be unique (`NAME_WE`) because object and fault-copy target names derive from them. The list is written to `build/dev/ivs-core-sources.txt` by `core/CMakeLists.txt` 28-33.

**H11. Existing profile controls overlay whole profile files.** About 12 `profile-*` controls ship their own copy of a toy profile. If the shared `_good/tree/data/expected/profile-fields.ids` gains `memmap.*` ids, those overlays lack the fields and print extra `profile-id-missing` lines; their `EXPECT` tokens still match but any `FORBID` of `profile-id-missing` would not. Either keep the `_good` profile small and give `memmap` controls their own overlay profile, or extend `_good` and re-run every `profile-*` control. Verify by running the control runner, not by reading.

**H12. `memcmp` equality may emit `bcmp`.** The core uses only `memset` today (no `memcmp`, `memcpy` or string functions in `core/src`). New code will emit `memcpy` for struct copies and may call `memcmp`; clang lowers `memcmp(a,b,n) == 0` to `bcmp`, which is not in `tools/check/core-symbols-allow.txt` (it lists `memcpy memmove memset memcmp`, the `_chk` forms, `__stack_chk_*`, `bzero`). Profile-id matching written as a bounded byte loop may be idiom-recognized into `strlen`/`strncmp`-like calls at `-O2`. Check the `-O2` archive in the first plan and add a reviewed allowlist line (with its one-line reason, `core-symbols-allow.txt` style) rather than rewriting code around the compiler.

**H13. Gate time and control count.** Estimated +2 to 4 s hosted (D-38) from roughly 40 fault executables (each links plain copies and runs the machine suite), a second preset run of the sweeps and about 25 new controls. The runner overlaps controls with steps. Measure after the first plan lands with the `step-time` lines `gate.sh` prints, and append the result to `CI-EFFICIENCY.md`.

**H14. `data/memmap` and `data/cart` JSON are data, not core input.** There is no C JSON parser by design; everything is turned into C initializers at build time, and `ivs_profiles.inc` must be fully determined by the profile files (no host path or time in the output; `-Werror=date-time` is already on).

## No Analog Found

Planner should use the research files (`03-DISCUSS-*.md`) and the cited standard, not a repo file.

| File | Role | Data Flow | Reason |
|------|------|-----------|--------|
| `core/include/ivs/*.h` `IVS_API` macro and `extern "C"` guards | public header | n/a | No public header exists; `core/include/` is empty today. Style comes from `cp1610.h`, the macro and C++ guards from `03-DISCUSS-api.md` D-A11 |
| `core/src/common/ivs_sha256.c` algorithm | utility | transform | No digest code in the repo. FIPS 180-4; expected digests from `cmake -E sha256sum`, not from the code under test |
| `core/src/machine/digest.c` field-table serializer | utility | transform | No serializer exists. D-35 and `03-DISCUSS-refusal.md` section 5: one field table, big-endian by shifts, a bitmap guard that every instance byte is covered by a table row or declared padding, a `_Static_assert(sizeof)` against the table total |
| Patch-from-base fixture expansion with reseal (`data/cart/*.json` negatives) | test data transform | transform | The vector generator merges `init` with an `expect` delta, but nothing patches bytes or recomputes a digest; the reseal uses the core SHA-256 at test run time, so it belongs in test code, not the cmake generator |
| `tools/check/core-symbols.sh` writable-symbol classification | helper | transform | Existing script classifies only imports and external definitions; letter sets per platform are an open spike (`03-DISCUSS-api.md` D-A10, `03-DISCUSS-refusal.md` section 6) |
| `IVS_API` list equality (headers vs `data/expected/api-calls.ids`) | check | transform | Closest is the `#if` site scan in `faults-verdict.sh` 118-133 (grep a source for a macro name); no header scanner exists |
| Optional libFuzzer target `tests/fuzz/fuzz_ivsc.c` | test | event-driven | No libFuzzer target; Apple clang cannot link the fuzzer runtime (`STACK.md`), so it stays an optional non-gate recipe |
| Harness second output stream (only if H6 chooses it) | test harness | event-driven | `ivs_test_main` has exactly one result file (`ivs_test.c` 171-210); extension follows the existing `ivs_emit(out, tap, ...)` two-sink pattern (`ivs_test.c` 73-93) |

## Metadata

**Analog search scope:** `core/`, `cmake/`, `tests/`, `tools/check/` (steps, kinds, schema, controls, lists), `tools/gate.sh`, `data/` (profile, discrepancies, claims, sources, tiers, expected inventories, cp1610 vectors), `fixtures/`, `docs/`, `.github/workflows/`, `.planning/CI-EFFICIENCY.md`, `.planning/phases/02-cp-1610-cpu-core/02-PATTERNS.md`, and the five `03-DISCUSS-*.md` research files (read for ids and fault lists only).
**Files scanned:** about 70 read in full or by range; every named analog path verified tracked.
**Pattern extraction date:** 2026-11-10
