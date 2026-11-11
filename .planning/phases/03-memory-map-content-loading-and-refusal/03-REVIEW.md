---
phase: 03-memory-map-content-loading-and-refusal
reviewed: 2026-11-11T09:33:57Z
depth: standard
files_reviewed: 41
files_reviewed_list:
  - core/include/ivs/ivs.h
  - core/include/ivs/ivs_status.h
  - core/include/ivs/ivs_machine.h
  - core/src/machine/machine.h
  - core/src/machine/machine.c
  - core/src/machine/machine_digest.c
  - core/src/machine/machine_load.c
  - core/src/machine/machine_run.c
  - core/src/content/ivsc.h
  - core/src/content/ivsc.c
  - core/src/common/ivs_sha256.h
  - core/src/common/ivs_sha256.c
  - core/src/memmap/memsys.h
  - core/src/memmap/memsys.c
  - core/src/cpu/cp1610.h
  - core/CMakeLists.txt
  - cmake/IvsMemmapTable.cmake
  - cmake/IvsShaVectors.cmake
  - docs/formats/ivsc-v1.md
  - tests/CMakeLists.txt
  - tests/faults.cmake
  - tests/headers-tap.sh
  - tests/machine/machine_main.c
  - tests/machine/machine_cases.h
  - tests/machine/machine_create.c
  - tests/machine/machine_digest.c
  - tests/machine/machine_refuse.c
  - tests/machine/machine_reset.c
  - tests/machine/machine_iso.c
  - tests/machine/machine_standin.c
  - tests/memmap/memmap_rows.c
  - tests/ivsc/ivsc_main.c
  - tools/check/memmap.cmake
  - tools/check/api-effects.cmake
  - tools/check/core-symbols.sh
  - tools/check/steps/memmap.sh
  - tools/check/steps/api-effects.sh
  - tools/check/steps/c-faults.sh
  - tools/check/steps/c-purity.sh
  - tools/check/steps/faults-verdict.sh
  - tools/gate-linux.sh
findings:
  critical: 0
  warning: 7
  info: 10
  total: 17
  necessity: 1
status: issues_found
---

# Phase 3: Code Review Report

**Reviewed:** 2026-11-11T09:33:57Z
**Depth:** standard
**Files Reviewed:** 41
**Status:** issues_found

## Summary

Scope: `git diff main...HEAD` on `gsd/phase-3-memory-map-content-loading`. I covered the public headers, the
machine, content, SHA-256 and memsys sources, the memory-map generator, the new gate steps and the core
parts of the test harness. I read all of these in full except the test files. Some test files were
sampled, not read line by line: `machine_load.c`, `machine_sweep.c`, `ivsc_fuzz.c`,
`ivsc_reencode.c`, `memmap_main.c`, `cmake/IvsCartTable.cmake` and `cmake/IvsMemmapProbes.cmake`.

Executed probes (scratchpad, Apple clang 21, `-fsanitize=address,undefined`):
- `ivs_sha256` was checked against Python `hashlib` for every length from 0 to 300 bytes, with split
  points every 7 bytes and a copied context. There were 0 mismatches. The FIPS padding, length field and
  context-copy logic are correct.
- An oversized `opts->struct_size` makes `ivs_machine_load` read past the caller's struct. ASan reported
  `stack-buffer-overflow READ ... in ivs_machine_load machine_load.c:340` (WR-02).
- A RAM store of `FFFF` followed by `0000` leaves `ivs_memsys` byte-identical to its snapshot. This
  confirms the vacuous dropped-write check (WR-05).
- `tools/check/steps/c-purity.sh` was run on a synthetic tree whose core TU includes `<unistd.h>` and
  `<stdio.h>` and reads the ARM virtual counter. The result was `-- c-purity pass core-imports=0
  purity-selftest=7/7` (WR-04).

Overall assessment: the parser, the SHA-256, the validate-then-commit ladder, the `uintptr_t` overlap
arithmetic, the digest field table and the memsys width formula are sound. I found no UB from integer
promotion or signed overflow, and no `size_t`/`u64` truncation in core code. Every length is widened
before comparison, and the 32-bit `size_t` path is handled, including the `RF_SKIP32` rows. The
defects are at the edges:
- a refusal status that depends on where the host's buffer is;
- an unbounded tail scan on `struct_size`;
- inconsistent trust of the stored profile index;
- one header contract that the implementation does not keep;
- two gate checks that can pass when they should fail: the c-purity include and intrinsic rule, and the
  memmap dropped-write probe.

No BLOCKER-class defect was found in the shipped core paths.

## Narrative Findings (AI reviewer)

## Warnings

### WR-01: The out-parameter overlap test against a NULL handle makes the refusal status depend on the buffer address

**File:** `core/src/machine/machine.c:234-236`, `core/src/machine/machine_digest.c:263-265`, `core/src/machine/machine_load.c:311-319`
**Issue:** `ivs_machine_info`, `ivs_machine_state_digest` and `ivs_machine_load` test the out
parameter against `[m, m + sizeof(struct ivs_machine))` before they validate the handle, and they do this
even when `m` is NULL. With `m == NULL`, `ivs_machine_span_overlaps(out, n, NULL, 113456)` returns 1
for any out buffer whose address is below 113,456. So `info(NULL, &info)` returns BAD_ARG for an `info`
at a low address and BAD_HANDLE for the same call with `info` elsewhere. This is the class of
behaviour that A-08 a was written to rule out ("a refusal status is a pure function of argument values
and machine state, never of where the host allocated the buffer").
Failure scenario: a wasm32 host has linear memory from address 0, and its stack and data usually sit
inside the first 64 KB to 1 MB. When it calls `ivs_machine_info(NULL, &stack_info)`, it gets
`IVS_E_BAD_ARG` instead of the documented `IVS_E_BAD_HANDLE`. The `refuse.*.null-machine` rows pass
on macOS and Linux only because user addresses there are far above 113,456.
**Fix:** Test the overlap only when there is an instance to overlap:
```c
if (out == NULL || out->struct_size < (uint32_t)sizeof(ivs_info) ||
    (m != NULL && ivs_machine_out_overlaps(out, sizeof *out, m, sizeof(struct ivs_machine)) != 0u)) {
  return IVS_E_BAD_ARG;
}
```
Make the same change for `out_sha256` in `ivs_machine_state_digest` and for `report` in
`ivs_machine_load`. Then add a refuse row that passes a NULL handle with the out pointer cast to a
small address. The core only compares the address and never writes through it before the check.

### WR-02: The unbounded `struct_size` tail scan reads past the caller's struct

**File:** `core/src/machine/machine.c:180`, `core/src/machine/machine_load.c:217`
**Issue:** The D-04 "unknown tail must be zero" rule scans `[sizeof v1, struct_size)` of the caller's
struct, and nothing limits `struct_size`. A host that forgets to initialise an options struct on the
stack, or copies one from another struct, passes a garbage `struct_size` such as `0x7FFFFFF0`. The
core then reads up to 2 GB past the struct before it finds a nonzero byte, and may fault. Verified:
ASan `stack-buffer-overflow READ of size 1 ... in ivs_machine_load machine_load.c:340` with
`op.struct_size = 0x7FFFFFF0u` on a 40-byte stack `ivs_load_options`. The API is meant to refuse bad
arguments rather than crash. A small, documented ceiling turns this host bug into a refusal.
**Fix:** Add a public bound, for example `#define IVS_STRUCT_SIZE_MAX 4096u`, and refuse above it
before the scan:
```c
if (opts->struct_size > IVS_STRUCT_SIZE_MAX) {
  return ivs_load_refuse(d, IVS_E_BAD_ARG, IVS_REASON_ARG_STRUCT_SIZE, IVS_SEGMENT_NONE, 0u, 0u);
}
```
Do the same in `ivs_machine_create`. Record the bound in the D-04 struct rule in `ivs_machine.h`, and
add a refuse row for each call.

### WR-03: Load does not check the stored profile index, although reset and step treat a bad one as BAD_HANDLE

**File:** `core/src/machine/machine_load.c:211`, `:116`, `:161`; compare `core/src/machine/machine_run.c:65-68`, `:105-108`
**Issue:** The instance lives in host-owned storage, so `im->profile` crosses a trust boundary just as
`magic` does. `ivs_machine_reset` and `ivs_machine_step_unit_internal` treat
`ivs_profile_get(im->profile) == NULL` as BAD_HANDLE. `ivs_load_validate` assigns
`p = ivs_profile_get(im->profile)` and later dereferences it without a check: `&p->exec` and
`sd->units` for system images, `p->has_cart_span` and `p->cart_lo` for cartridges. The memsys paths
also index `ivs_mm_profiles[m->profile]` and `ivs_mm_rd_region[m->profile]` unchecked
(`memsys.c:83`, `:162`, `:215`, `:319`).
Failure scenario: storage holds a valid magic but a corrupted profile word, for example after a
partial `memcpy` relocation or a stray host write. Reset returns BAD_HANDLE, but load dereferences
NULL and crashes. The same handle gets two different outcomes depending on the call.
**Fix:** Do the check once, in the handle validation:
```c
const struct ivs_machine *ivs_machine_from_handle(const ivs_machine *m)
{
  if (m == NULL || m->magic != IVS_MACHINE_MAGIC || m->profile >= ivs_profile_count() ||
      m->mem.profile != m->profile) {
    return NULL;
  }
  return m;
}
```
Make the same change in the `_mut` version. Then remove the per-call `p == NULL` checks in
`machine_run.c`, which become redundant.

### WR-04: The c-purity stage-1 include and intrinsic rules can be bypassed (verified)

**File:** `tools/check/steps/c-purity.sh:140`, `:129`
**Issue:** The include rule only recognises lines that match `^[[:space:]]*#[[:space:]]*(include|import)`.
Two forms of a valid C17 directive slip past it:
1. A comment before `#`. Comments count as whitespace, so `/* x */ #include <unistd.h>` is a directive.
2. The digraph `%:`, as in `%:include <stdio.h>`.

The word list for host clocks and counters only names `__builtin_readcyclecounter`,
`__builtin_ia32_rdtsc`, `__rdtsc` and `__rdtscp`. It misses intrinsics that compile to inline
instructions and so never show up as an imported symbol in stage 2:
- `__builtin_arm_rsr64("cntvct_el0")`: the AArch64 virtual counter, which is this repository's primary
  host;
- `__builtin_readsteadycounter`;
- `__builtin_ia32_rdtscp`;
- `__builtin_ia32_rdrand*_step` and `__builtin_ia32_rdseed*_step` (hardware randomness).

Verified: a TU with `/* host */ #include <unistd.h>`, `%:include <stdio.h>` and a return value built from
`__builtin_arm_rsr64("cntvct_el0") + __builtin_readsteadycounter()` gives `-- c-purity pass
core-imports=0 purity-selftest=7/7`. The gate's purpose is to catch exactly this kind of determinism
break (API-05, D-21), and it passes.
**Fix:** Before matching, remove comments from the start of the line, and accept `%:` as well as `#`:
```awk
lead = line; gsub(/\/\*[^*]*\*+([^\/*][^*]*\*+)*\//, "", lead)
if (!incomment && lead ~ /^[[:space:]]*(#|%:)[[:space:]]*(include|import)/) { ... }
```
Also report `c-purity-include` for any `include` token that follows `#` or `%:` anywhere on a line.
Change the asm check to a deny prefix rule, so that any `__builtin_arm_*`, `__builtin_ia32_*` or
`__builtin_read*` word fails, and add a self-test case for each form.

### WR-05: The memmap dump's dropped-write probe cannot detect a kept store into RAM

**File:** `tests/memmap/memmap_rows.c:627-647` (used at `:662-670`, `:691-692`)
**Issue:** `mr_dropped_row` decides whether a literal `W ... w=0` row really drops stores. It writes
`FFFF` and then `0000` to every address and compares the whole `ivs_memsys` with a snapshot. Every RAM
backing (sysram, scratchpad, GRAM) is zero after `ivs_memsys_init`. A store that is kept therefore
ends at `0000` and leaves the state byte-identical to the snapshot. Verified: after
`ivs_memsys_write(0x0200, 0xFFFF); ivs_memsys_write(0x0200, 0x0000)` the memsys still equals the
snapshot.
The dump's `w` column for every literal-w=0 write row comes only from this probe. So a decode that
kept stores into a zero RAM backing somewhere inside a w=0 write row would still dump `w=0`. Two cases
escape it:
- a store bug that leaves `describe()` unchanged, such as a write-path `keep` or `phys` dispatch
  error;
- an address inside the row, which the per-row cases miss because they probe only `lo-1/lo/hi/hi+1`.

The comment says "every write must leave the state unchanged", but this check cannot fail for RAM.
**Fix:** Write a value different from the initial fill as the last write, and compare after each write
pair:
```c
mr_write((uint16_t)a, 0x0000u);
mr_write((uint16_t)a, 0xA5A5u); /* last write is nonzero: a kept store is visible */
if (memcmp(&mr_snap, &mr_m, sizeof mr_snap) != 0) { ... }
```
Alternatively, fill every RAM backing with a nonzero pattern through `ivs_memsys_poke_raw_internal`
before taking the snapshot.

### WR-06: `ivs_machine_create`'s header promises an untouched storage that the implementation does not keep

**File:** `core/include/ivs/ivs_machine.h:178-183`; `core/src/machine/machine.c:154-162`
**Issue:** The public contract says: "On refusal *out is NULL (when out is not NULL and not inside the
storage, where nothing is written) and the storage is untouched." The implementation protects only the
instance span, `span = min(storage_bytes, sizeof(struct ivs_machine))`. When `out` lies inside the
storage but after the instance bytes, every refusal still writes `*out = NULL` into the storage, for
example a nonzero `flags` (UNSUPPORTED) or an unknown profile. The internal comment
(`machine.c:14-16`) and the A-08-era test `machine.refuse.create.out-after-instance-ok` assume the
narrower rule, so the public text and the code disagree.
Failure scenario: a host keeps its own bookkeeping, including the handle slot, in a tail after the
instance inside one allocation. It relies on "storage untouched on refusal", but 8 bytes of the tail
are zeroed.
**Fix:** Either change the header to "not inside the instance bytes of the storage (the first
`sizeof` instance bytes)", which matches the code and the test, or protect the whole
`[storage, storage + storage_bytes)` range. The first option is the smaller change and keeps the
existing OK row valid.

### WR-07: Already-defended-elsewhere: the duplicate profile-id length check in `ivs_machine_storage_requirements`

**File:** `core/src/machine/machine.c:131-133`
**Category:** necessity
**Issue:** `ivs_machine_storage_requirements` calls `ivs_machine_profile_id_length` and maps a failure to
BAD_ARG. It then calls `ivs_machine_profile_lookup`, whose first step (`machine.c:87-90`) is the same
`ivs_machine_profile_id_length` call, returning the same BAD_ARG. No check runs between the two calls,
so the pre-check cannot change the result: it repeats a refusal the next call already makes. In
`ivs_machine_create` the pre-check is needed, because BAD_ARG must come before the alignment and
UNSUPPORTED checks. In `storage_requirements` nothing sits between the two calls.
**Fix:** Delete lines 131-133 and the unused `len` local, and let `ivs_machine_profile_lookup` supply
BAD_ARG and UNKNOWN_PROFILE.

## Info

### IN-01: `has_cart_span` is always 1, so its false branches never run

**File:** `core/src/machine/machine_load.c:161`, `core/src/memmap/memsys.c:323`; `cmake/IvsMemmapTable.cmake:265-272`, `:355`
**Issue:** The generator fails when `memmap.cart_span` is missing (required list at `:265`) and sets
`has_span 1` unconditionally (`:355`). As a result, `p->has_cart_span != 0u ? ... : 0u`, which would
refuse every cartridge as OUT_OF_RANGE at segment 0, and the `has_cart_span` test in
`ivs_memsys_install_cart_segment` are dead code.
**Fix:** Drop the field and the branches, or make the span optional in the generator with a test for
a profile that has no cartridge port.

### IN-02: Per-profile `system_images.*.units` and `unit_bits` are reported but never enforced

**File:** `core/src/machine/machine_load.c:83-93`, `:119`, `:127`, `:134-136`; `cmake/IvsMemmapTable.cmake:284-299`
**Issue:** The generator accepts any `units` from 1 to 65536 and any `unit_bits` from 1 to 16 per
profile. The loader ignores both:
- it validates against the fixed `IVS_MEMSYS_EXEC_BYTES`/`GROM_BYTES`;
- it applies the fixed 10-bit mask `0xFC`;
- `ivs_memsys_install_exec` always copies 4096 words.

The slot report still copies `sd->units` and `sd->unit_bits`. A profile with `units: 2048` would report
`length_units 2048` while 4096 words are installed and an 8192-byte image is required.
**Fix:** Pin `units`/`unit_bits` in the generator to the values the loader enforces, with a
`_Static_assert` against the `IVS_MEMSYS_*` macros, or derive the bound and the width mask from the
descriptor.

### IN-03: No compile-time link between the public reason numbers and the parser's reason numbers

**File:** `core/src/machine/machine_load.c:153`; `core/include/ivs/ivs_machine.h:123-159`; `core/src/content/ivsc.h:37-59`
**Issue:** `ivs_load_cart` copies `irep.reason` and `irep.segment_index` into the public report
unchanged. Nothing asserts that `IVS_REASON_TRUNCATED..WORD_RANGE` equal `IVS_IVSC_R_*`, or that
`IVS_SEGMENT_NONE == IVS_IVSC_NO_SEGMENT`. If the internal header is renumbered, the public,
append-only reason values change silently, and only the reasons the refuse matrix happens to cover
would catch it.
**Fix:** Add 19 `_Static_assert`s in `machine_load.c`, one per reason plus the no-segment value.

### IN-04: c-faults runs each fault binary with no time limit

**File:** `tools/check/steps/c-faults.sh:97-98`
**Issue:** The fault executables are not ctest tests, so no ctest timeout applies. A fault that turns
a bounded loop into an unbounded one stalls the whole gate, which then hangs until the CI job's own
timeout.
**Fix:** Wrap the call in a portable watchdog, for example a background `kill` after N seconds, and
report `c-faults-run: <ID> (timeout)`.

### IN-05: The faults-verdict site scan misses some site forms

**File:** `tools/check/steps/faults-verdict.sh:133-135`
**Issue:** The greedy `sed 's/^.*IVS_FAULT_\(...\)'` captures only the last `IVS_FAULT_` token on a
line, so `#if defined(IVS_FAULT_A) || defined(IVS_FAULT_B)` registers only B. Lines that start with
`#elif` are not scanned at all. Both cases fail closed today (fault-missing-site), but they will
surprise whoever first writes such a site.
**Fix:** Extract every `IVS_FAULT_[A-Za-z0-9_]*` token with `grep -o`, and include `elif` in the
directive pattern.

### IN-06: api-effects only sees `IVS_API` at column 0

**File:** `tools/check/api-effects.cmake:153`
**Issue:** The D-06 symbol-list equality comes from lines that match `^IVS_API[ \t]`. Two kinds of
public function are invisible to the check:
- one declared without the marker, which still links and appears in `ivs/*.h`;
- one whose declaration is indented, for example inside an `#ifdef`.
**Fix:** Also flag any prototype-shaped line in `core/include/ivs/*.h` that has no `IVS_API`, and allow
leading whitespace.

### IN-07: `ivs_info.profile_index` exposes an unstable registry index

**File:** `core/src/machine/machine.c:246`; `cmake/IvsMemmapTable.cmake:164-165`
**Issue:** The index is a position in the sorted `data/profiles/*.json` list. Adding a profile whose
name sorts first renumbers every existing profile. The digest correctly uses the profile id, but the
public info struct uses the index.
**Fix:** Document the index as valid only within one library build, or report the id, for example by
copying it into a fixed `char[64]`.

### IN-08: Tests assume `malloc` returns 16-byte-aligned storage

**File:** `tests/machine/machine_standin.c:110`, `tests/machine/machine_refuse.c:1405`, `:1458`
**Issue:** On targets where `malloc` aligns to 8 bytes, such as 32-bit Windows and some embedded C
libraries, `ivs_machine_create` correctly refuses BAD_ARG. The suites then fail as harness errors
rather than show the behaviour under test. `machine_create.c` already uses `aligned_alloc`.
**Fix:** Use `aligned_alloc(16, ...)` with a size rounded up to a multiple of 16, or reuse
`mc_store_new`, in the shared helpers.

### IN-09: Memmap row test ids have no profile component

**File:** `tests/memmap/memmap_rows.c:833`
**Issue:** The id is `memmap.row.<view>-<LO>-<HI>`. A second profile with any row on the same range
produces a duplicate id. The harness then refuses the whole suite, which fails closed but blocks a
second profile.
**Fix:** Add the profile id, or a short profile key, to the id.

### IN-10: Instance access through a struct lvalue on host storage that may have a declared type

**File:** `core/src/machine/machine.c:196` (and every `ivs_machine_from_handle*` use)
**Issue:** If the host passes storage that is a declared `unsigned char` or `uint8_t` array, for example
`static _Alignas(16) unsigned char buf[IVS_MACHINE_BYTES]`, then reading and writing it through
`struct ivs_machine *` is outside the C17 6.5p7 effective-type rules. Today this is safe in practice:
the core and the host are separate TUs, IPO is off and the static library is not built with LTO. A
future LTO or shared build could expose it.
**Fix:** State in `ivs_machine.h` that storage must be allocated memory, or a union or object without a
conflicting declared type. Alternatively, build the core with `-fno-strict-aliasing` as a documented
safeguard.

---

_Reviewed: 2026-11-11T09:33:57Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
