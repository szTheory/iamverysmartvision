---
phase: 03-memory-map-content-loading-and-refusal
plan: 09
subsystem: api
tags: [public-api, instance, caller-storage, refusal, status-pin, state-digest, sha256, named-faults, D-01, D-02, D-03, D-04, D-06, D-08, D-09, D-35, D-37, API-01, API-06, MEM-04]

requires:
  - phase: 03-memory-map-content-loading-and-refusal
    provides: "plan 03-02: core SHA-256 (ivs_sha256_*) and the fault suite machine; plan 03-03: core/include/ivs and ivs_status.h; plan 03-04: ivs_memsys, ivs_memsys_init and the profile registry (ivs_profile_count/ivs_profile_get)"
provides:
  - "Public headers core/include/ivs/{ivs.h, ivs_machine.h} and IVS_API plus extern \"C\" in ivs_status.h; seven of the ten D-05 functions (ivs_api_version, ivs_status_name, ivs_machine_storage_requirements, ivs_machine_create, ivs_machine_destroy, ivs_machine_info, ivs_machine_state_digest)"
  - "Flat pointer-free instance struct ivs_machine (core/src/machine/machine.h): 113,456 bytes, 16-byte aligned, three slot records"
  - "Canonical state digest (core/src/machine/machine_digest.c): per-domain SHA-256 (CPU, MEM, IMAGES, INSTANCE) and the IVSSTATE composite from one field table, plus the internal seams ivs_machine_digest_field* and ivs_machine_digest_preimage"
  - "Status pin data/expected/api-statuses.ids (NAME VALUE, 14 codes)"
  - "Suite machine families create (45 cases) and digest (19 cases)"
  - "Eight machine named faults (data/expected/faults/machine.ids); three SHA faults widened to machine.digest."
affects: [03-10, 03-11, 03-12, 03-13, 03-14]

actuals:
  tokens: 20805
  tasks: 3
  commits: 3
plan_head_before: 30c1729a5b4dc8dc0a4f0b29d5e54e05ab22a765
plan_head_after: bed72d78ac3680ee3370ce96434a388760ea57c0

tech-stack:
  added: []
  patterns:
    - "Validate-before-write create: every refusal class is checked before the first byte of host storage is written, and a refusal row compares the whole poisoned storage (plus a 32-byte margin) byte for byte"
    - "Field-table serializer: one const table drives the state digest; rows are UINT (big-endian by shifts, u32 count before arrays), PROFILE (registry index written as the id) or declared MAGIC/PAD bytes; a coverage test proves each instance byte is in exactly one row"
    - "Preimage seam without a function pointer: a stage buffer feeds the hash and, for tests, a caller buffer"

key-files:
  created:
    - core/include/ivs/ivs.h
    - core/include/ivs/ivs_machine.h
    - core/src/machine/machine.h
    - core/src/machine/machine.c
    - core/src/machine/machine_digest.c
    - tests/machine/machine_create.c
    - tests/machine/machine_digest.c
    - data/expected/api-statuses.ids
    - data/expected/faults/machine.ids
  modified:
    - core/include/ivs/ivs_status.h
    - core/CMakeLists.txt
    - tests/CMakeLists.txt
    - tests/machine/machine_cases.h
    - tests/machine/machine_main.c
    - data/expected/tests/machine.ids
    - data/expected/faults/sha256.ids

key-decisions:
  - "create zeroes exactly sizeof(struct ivs_machine) bytes of the storage (the bytes destroy scrubs); bytes past the instance in a larger storage belong to the host and are never touched (case machine.create.larger-storage)"
  - "Precedence for calls that take a handle: argument refusals (BAD_ARG) first, then BAD_HANDLE (A-08 b order); a NULL handle is BAD_HANDLE, not BAD_ARG"
  - "A refused info, requirements or digest call writes nothing; a refused create writes only *out = NULL"
  - "Digest: every row with count > 1 (fixed arrays included) is preceded by its u32 count; the profile index is serialized as the profile id (u32 length + ASCII), an out-of-registry index as u32 0xFFFFFFFF + the index; mem.profile goes to MEM, the memsys loaded flags, cartridge count and segment table to IMAGES; the magic, the reserved word, the CPU reserved bytes and the 4 tail bytes are declared and never serialized"
  - "Each slot record stores its kind (1, 2, 3) so IMAGES serializes a real field, not a constant"
  - "ivs_image_report fields are all u32 (length_bytes included: the largest image is 131,256 bytes)"
  - "API version packed as major << 16 | minor << 8 | patch, version 0.1.0"
  - "SHA faults K_WRONG, ROTATE_WRONG and LENGTH_LE allow machine.digest. (their rule breaks the digest literals); PAD_BOUNDARY, FINAL_BLOCK_DROPPED and INIT_NO_RESET leave every digest case green and stay machine.sha256. only"
  - "No requirement is marked complete: API-01 needs the isolation tests (03-12), API-06 the effect table (03-13), MEM-04 the load and reset refusal rows (03-10..03-12)"

patterns-established:
  - "Refusal row helper: poison the storage, call, compare every byte, check *out NULL; precedence pairs are ordinary rows with two violations"
  - "Literal digest anchors are typed from `cmake -E sha256sum` over hand-written byte files; repeatability-only literals are labelled as such in the test"

requirements-completed: []

coverage:
  - id: D1
    description: "Public headers, caller-provided storage, create/destroy/info (D-01..D-04, D-06)"
    requirement: API-01
    verification:
      - kind: unit
        ref: "build/dev/tests/ivs_tests_machine --only machine.create. ; ctest --preset release -R '^machine$'"
        status: pass
    human_judgment: false
  - id: D2
    description: "Lifecycle refusals with whole-storage comparison and the status pin (D-08, D-09, T-03-29..T-03-31)"
    requirement: MEM-04
    verification:
      - kind: unit
        ref: "build/dev/tests/ivs_tests_machine --only machine.refuse. (41 refusal cases, ASan+UBSan)"
        status: pass
    human_judgment: false
  - id: D3
    description: "Canonical state digest with literal streams, coverage and sensitivity (D-35, T-03-32)"
    requirement: API-06
    verification:
      - kind: unit
        ref: "build/dev/tests/ivs_tests_machine --only machine.digest. (12 cases, 0.12 s dev)"
        status: pass
    human_judgment: false
  - id: D4
    description: "Eight machine named faults; SHA faults re-judged (D-37)"
    requirement: API-06
    verification:
      - kind: integration
        ref: "sh and dash tools/gate.sh: faults=98/98 fault-runs=98"
        status: pass
    human_judgment: false

duration: 16min
completed: 2026-11-11
status: complete
---

# Phase 3 Plan 09: Public machine API, caller storage, status pin and canonical state digest Summary

**A host can now size, create, inspect, digest and destroy machine instances in its own storage through `ivs/ivs.h` only. The instance is one flat, pointer-free, 113,456-byte struct. Every create refusal is checked before the first write and leaves poisoned storage byte-identical. `ivs_status_name` and `data/expected/api-statuses.ids` pin the 14 status codes. The state digest is SHA-256 per domain plus an `IVSSTATE` composite, serialized big-endian from one field table, with literal streams typed from `cmake -E sha256sum`, a byte-exact coverage proof and a per-domain sensitivity sweep. Eight named faults each turn a named case red. Final gate under sh and dash: `gate: PASS steps=25/25 controls=215/215 ... fault-runs=98 ... faults=98/98 ... tests-expected=1516 tests-executed=1516 tests-skipped=0 tests-missing=0`.**

## Performance

- **Duration:** about 16 min (2026-11-11T07:06:07Z to 07:22Z).
- **Tasks:** 3 of 3.
- **Files:** 16 in 3 commits (diff +2276 / -8).
- **Suite machine wall time:** dev (ASan+UBSan) 0.12 s for the whole suite, up from 0.06 s; the digest family alone is 0.12 s of `real` including start-up. Release 0.03 s. One machine fault executable runs the suite in 0.03 s.
- **Gate time (local, warm build):** `gate-time total` 21 to 26 s (sh 25, 21, 26; dash 21, 21), against 20 to 23 s before the plan. New build cost: two core TUs, two test TUs per preset, eight owner copies for the faults. No gate step or control was added. The hosted figure is still to be read (budget 40 s, revisit trigger 50 s).

## Accomplishments

### Public headers (D-04, D-06)

- `ivs_status.h` gained `IVS_API` (empty unless a later shared build defines it), `extern "C"` guards and `ivs_status_name`.
- `ivs_machine.h` includes only `<stddef.h>`, `<stdint.h>` and `ivs/ivs_status.h`. It declares the incomplete `ivs_machine`, the kind, phase and digest-domain constants (16..18 reserved for RUN-05), the structs `ivs_storage_requirements`, `ivs_create_options`, `ivs_image_report` (64 bytes) and `ivs_info`, and five one-line `IVS_API` prototypes.
- `ivs.h` includes both, and defines `IVS_API_VERSION_{MAJOR,MINOR,PATCH}` (0.1.0), the packed `IVS_API_VERSION` and `ivs_api_version()`.
- No enums, bool or bitfields; every function prototype is one line starting `IVS_API `.

### Instance (D-01, D-02, D-08)

`struct ivs_machine` (`core/src/machine/machine.h`), offsets in bytes:

| Field | Offset | Size |
|---|---|---|
| `magic` (`_Alignas(16)`, `0x4956534D`) | 0 | 4 |
| `profile` (registry index) | 4 | 4 |
| `phase` (OFF / POWERED) | 8 | 4 |
| `reserved` | 12 | 4 |
| `time_ticks` (master ticks) | 16 | 8 |
| `cpu` (`ivs_cp1610`) | 24 | 32 |
| `mem` (`ivs_memsys`) | 56 | 113,228 |
| `slot[3]` (kind, loaded, length_bytes, length_units, unit_bits, base_addr, sha256) | 113,284 | 168 |
| tail padding | 113,452 | 4 |

- **`sizeof(struct ivs_machine)` = 113,456 bytes, alignment 16.** `_Static_assert`s pin both, and the size assert names the digest field table, so a new field forces a table review.
- The instance holds no last-error, counter or timestamp.

### Lifecycle and create precedence (as implemented)

`ivs_machine_create(opts, storage, storage_bytes, &m)` first sets `*out = NULL` when `out` is not NULL, then refuses in this order:

1. **BAD_ARG:** NULL `opts`, `storage` or `out`; `opts->struct_size` below `sizeof(ivs_create_options)`; profile id NULL or with no NUL in its first 64 bytes; storage not 16-byte aligned.
2. **UNSUPPORTED:** nonzero `flags`; a nonzero byte in `[sizeof(ivs_create_options), struct_size)` of a larger caller struct.
3. **UNKNOWN_PROFILE:** the id is not in the registry (exact, case-sensitive, hand-written loop bounded by 64 bytes, no string functions).
4. **BUFFER_TOO_SMALL:** `storage_bytes` below 113,456.

Only then does it zero the 113,456 instance bytes and set magic, profile index, phase OFF, time 0, `ivs_memsys_init` and the slot kinds.

- **`ivs_machine_destroy`:** BAD_HANDLE for NULL or a wrong magic, with no write; otherwise it zero-scrubs the instance, so a second destroy, info or digest is BAD_HANDLE.
- **`ivs_machine_info`, `ivs_machine_storage_requirements`:** BAD_ARG for a NULL or short out struct (nothing written), then BAD_HANDLE or UNKNOWN_PROFILE. A larger out struct receives only the known bytes, and `struct_size` is set to the written size.
- **`ivs_status_name`:** a `switch` over the 14 codes; anything else is `"IVS_E_UNKNOWN"`.

### Family create (45 cases)

- **19 create refusals** (`machine.refuse.create.*`), each against storage poisoned 0xA5 with a 32-byte margin, comparing every byte and checking `*out == NULL`:
  - null-opts, null-storage, null-out, size-0, size-4, id-null, id-unterminated (64 bytes, exact-size heap buffer), misaligned-1, misaligned-8 (BAD_ARG);
  - flags-1, tail-nonzero (UNSUPPORTED);
  - unknown-profile, near-miss (`...-p2`), trailing-space, id-63-unknown (the longest legal id is a miss, not BAD_ARG) (UNKNOWN_PROFILE);
  - short-storage (BUFFER_TOO_SMALL);
  - precedence pairs misaligned-over-flags (BAD_ARG), flags-over-unknown-profile (UNSUPPORTED), unknown-over-short-storage (UNKNOWN_PROFILE).
- **Positives:** ok-info-destroy, requirements, tail-zero-ok, poisoned-storage (0xA5 and 0x5A storage end byte-identical), larger-storage (bytes past the instance untouched), static-storage (a `_Alignas(16)` static buffer, so the release -O2 run covers static storage), info-larger-out, requirements-larger-out.
- **Destroy:** `machine.destroy.scrubs`; refusals null, wrong-magic (no write), twice.
- **Info refusals:** null-out, null-both (BAD_ARG wins), size-0, short, null-handle, destroyed; each also checks the out struct and the instance are unchanged.
- **Requirements refusals:** null-out, size-0, short, null-id, id-unterminated, unknown-profile.
- **Status and version:** `machine.status.names` covers all 14 codes (names stringized from the header macros) and the unknowns -6, -15, -24, -31, 1 and -1000. `machine.api.version` checks the macros against `ivs_api_version()`.

### Status pin (D-09)

`data/expected/api-statuses.ids`: `IVS_OK 0`, `IVS_E_BAD_ARG -1` ... `IVS_E_BUFFER_TOO_SMALL -5`, `IVS_E_UNKNOWN_PROFILE -16` ... `IVS_E_DIGEST_MISMATCH -23`. That is 14 lines, equal to the 14 `#define`s in `ivs_status.h`. The `api-effects` step of plan 03-13 will check this equality in the gate.

### Canonical state digest (D-35)

**Framing.**

- Domain preimage: `"IVSD"`, then u8 domain, then u16 version 1, then the domain's rows in table order.
- Composite: SHA-256 of `"IVSSTATE"`, then u16 version 1, then, for each of domains 1 to 4, u8 id and the 32-byte domain digest.

**Field table.** There are 49 rows in instance offset order:

| Domain | Rows |
|---|---|
| CPU | `r[8]`, `status`, `intff`, `dbd`, `prev_interruptible`, `run_state`, `halt_pending`, `fault_reason`, `busak` |
| MEM | `mem.profile` (as id), `gate_open[4]`, `sysram`, `scratchpad`, `gram`, `exec`, `grom`, `cart` |
| IMAGES | `exec_loaded`, `grom_loaded`, `cart_count`, `cart_seg` (32 u16), and seven fields for each of the three slots |
| INSTANCE | `profile` (as id), `phase`, `time_ticks` |
| Declared, never serialized | `magic`, `reserved`, `cpu.reserved[8]`, `tail_padding[4]` |

- Values are read natively (one scalar `memcpy`) and written big-endian by shifts.
- **Preimage sizes of a fresh instance (bytes):** CPU 35, MEM 113,219, IMAGES 259, INSTANCE 47.
- **Refusals:**
  - BAD_ARG: a NULL out, or a domain other than 0 to 4 (5, 16 and 99 are tested). It is checked before the handle.
  - BAD_HANDLE: a NULL or destroyed handle.
  - Neither refusal writes the out buffer or the instance.

**Family digest (19 cases).**

- **Literal streams:**
  - `stream-instance`: 47 bytes; the SHA-256 is `203dc162...c269`.
  - `stream-cpu`: fresh, 35 bytes; `22eba126...dfcd`.
  - `stream-cpu-synthetic`: R0 to R7 = A000 to A777, the eight flag bytes C3, 11 ... 77, and the reserved bytes EE (not serialized); `9c077f62...e4b1`.
  - Each preimage was written out by hand. Its digest was computed with `cmake -E sha256sum` over a byte file built with `printf`, and the test compares both the preimage bytes and the public digest.
- **`preimage-hash`:** a test-side SHA-256 of each domain preimage equals the public domain digest.
- **`composite-framing`:** the test rebuilds the composite from the four public domain digests.
- **`fresh-composite`:** the literal `ac794821...d137`, labelled repeatability only. At authoring time an independent Python rendering of the four preimages, hashed with another SHA-256 implementation, gave the same value. The script was a scratch file and is not committed.
- **`coverage`:** each of the 113,456 bytes is in exactly one row, the row total equals `sizeof`, and every domain has a serialized row.
- **`sensitivity-{cpu,mem,images,instance}`:** for every serialized row of the domain, the test flips the first and the last byte. The domain's public digest must change. The other three domains' preimages must stay byte-identical. The composite must change, checked on the first-byte flip of each row to bound the MEM hashing.
- **`padding-ignored`:** each padding byte flipped alone leaves its domain's preimage identical. With all padding flipped at once, the four domains and the composite are unchanged.
- **Refusals:** domain-5, domain-16, domain-99, null-out, null-handle, destroyed, domain-over-handle.

### Named faults (`data/expected/faults/machine.ids`, `# suite=machine`)

The verdict was observed by running each fault executable before the allowed prefixes were written. Each fault's failures are listed below.

| Fault | Site | Must-fail | Observed failures (all allowed) | Must-pass |
|---|---|---|---|---|
| `CREATE_MAGIC_BEFORE_PROFILE_CHECK` | `machine.c`, magic written before the registry lookup | `machine.refuse.create.unknown-profile` | unknown-profile, near-miss, trailing-space, id-63-unknown, short-storage, unknown-over-short-storage | `machine.create.ok-info-destroy` |
| `INIT_SKIPS_MEMSET` | `machine.c`, zeroing length 0 | `machine.create.poisoned-storage` | poisoned-storage, ok-info-destroy, `digest.fresh-composite`, `digest.stream-cpu` | `machine.refuse.create.null-opts` |
| `DESTROY_NO_SCRUB` | `machine.c`, scrub length 0 | `machine.destroy.scrubs` | scrubs, refuse.destroy.twice, refuse.info.destroyed, refuse.digest.destroyed, ok-info-destroy, static-storage | `machine.create.requirements` |
| `DIGEST_SKIPS_RAM` | `machine_digest.c`, sysram row skipped | `machine.digest.sensitivity-mem` | sensitivity-mem, fresh-composite | `machine.create.ok-info-destroy` |
| `DIGEST_SKIPS_IMAGE_TABLE` | `machine_digest.c`, slot records skipped | `machine.digest.sensitivity-images` | sensitivity-images, fresh-composite | `machine.create.ok-info-destroy` |
| `DIGEST_LITTLE_ENDIAN` | `machine_digest.c`, shift order reversed | `machine.digest.stream-cpu-synthetic` | the three streams, composite-framing, fresh-composite | `machine.create.ok-info-destroy` |
| `DIGEST_INCLUDES_PADDING` | `machine_digest.c`, PAD rows emitted | `machine.digest.padding-ignored` | padding-ignored, the three streams, fresh-composite | `machine.create.ok-info-destroy` |
| `DOMAIN_ID_DROPPED` | `machine_digest.c`, composite without the u8 ids | `machine.digest.composite-framing` | composite-framing, fresh-composite | `machine.create.ok-info-destroy` |

**SHA faults re-judged (03-02 open item).**

- `SHA_K_WRONG`, `SHA_ROTATE_WRONG` and `SHA_LENGTH_LE` now also fail the four digest literal cases (the three streams and fresh-composite), for their own rule. Their allowed prefixes became `machine.sha256.|machine.digest.`.
- Each still fails its own must-fail, and its must-pass stays green.
- `SHA_PAD_BOUNDARY`, `SHA_FINAL_BLOCK_DROPPED` and `SHA_INIT_NO_RESET` fail no digest case and are unchanged. The preimage lengths 35, 47, 259, 113,219 and 142 avoid the 56-to-63 padding window, and the digest code zeroes its context before init.
- **Gate result:** `faults=98/98`, `fault-runs=98`.

## Task Commits

1. **Task 1 (tracer): public header, create/info/destroy end to end.** `4ab9ed3` (feat).
   - `cmake --build --preset dev && build/dev/tests/ivs_tests_machine --only machine.create.` passed (2 cases).
   - `sh` and `dash tools/gate.sh` ended `gate: PASS ... faults=90/90 ... tests-missing=0`.
   - Acceptance greps printed 4 `IVS_API` lines, 2 `extern "C"` and 3 `#include`.
   - The tracer feedback gate re-ran the automated verify, and it was green before expansion.
2. **Task 2: refusals, status names, version and the pin.** `71bec21` (feat).
   - RED: the build failed on the missing `ivs/ivs.h` before the implementation.
   - GREEN: dev `--only machine.` passed 69 cases, and `ctest --preset release -R '^machine$'` passed.
   - Pin lines 14 = header defines 14; `machine.refuse.create.*` count is 19 (at least 12 required).
   - `sh` and `dash` gates both PASS with identical summary lines.
3. **Task 3: state digest and eight faults.** `bed72d7` (feat).
   - `--only machine.digest.` passed.
   - `sh` and `dash tools/gate.sh` both ended `gate: PASS steps=25/25 controls=215/215 ... faults=98/98 ... tests-missing=0`, with identical summary lines.
   - `grep -c -F IVSSTATE core/src/machine/machine_digest.c` printed 2, and the faults file has 8 lines.
   - `nm -P -u build/release/core/libivs_core.a` imports only `memcpy`, `bzero`, `__stack_chk_fail` and `__stack_chk_guard` (all allowlisted, no `bcmp`).

**Plan metadata:** recorded in the docs(03-09) completion commit.

## Decisions Made

See `key-decisions` in the frontmatter.

## Deviations from Plan

### Auto-fixed Issues

**1. [Plan count] Controls 215, not 213**
- The plan text predates the controls of plans 03-05 and 03-06, and the orchestrator gave 215 as the current count. This plan adds no control, so the gate reads `controls=215/215`. **Commit:** none needed.

**2. [Reading] create zeroes the instance bytes, not every byte of a larger storage**
- **Plan text:** "memset the whole storage".
- **What was done:** create zeroes `sizeof(struct ivs_machine)` bytes, the same span destroy scrubs. With exact-size storage the two readings agree.
- **Why:** with larger storage, the bytes past the instance are the host's (D-01: the host owns the storage), and destroy cannot know `storage_bytes`.
- **Test:** `machine.create.larger-storage` pins it.
- **Commit:** `4ab9ed3`.

**3. [Small addition] Extra refusal and positive rows**
- **Precedence pairs:** three create pairs, plus `info.null-both` and `digest.domain-over-handle`, pin the order recorded above. Plan 03-13 will fold them into its effect table.
- **Other boundaries:** `id-63-unknown` (the id bound edge), `larger-storage` and `static-storage`.
- **Digest:** `preimage-hash` ties the test seam to the hash path. `stream-cpu-synthetic` adds nonzero, byte-distinct values, so a byte-order slip shows in more than the count prefixes.
- **Commits:** `71bec21`, `bed72d7`.

**4. [Design detail] Digest field-table choices not fixed by the plan**
- **Choices:**
  - u32 count before every row with count > 1 (fixed arrays included);
  - the PROFILE row kind (index written as the id);
  - where the memsys header fields go (`mem.profile` to MEM; loaded flags, count and segment table to IMAGES);
  - a stored kind in each slot record.
- **Why:** each keeps the stream unambiguous and every instance byte attributable. The coverage case enforces the table.
- **Commit:** `bed72d7`.

---

**Total deviations:** 4.
- One count update (1).
- One reading of ambiguous plan text (2).
- Two small additions or design details inside the plan's scope (3, 4).

No existing test id, control or step changed. Three SHA fault prefix strings were widened, as the plan and the 03-02 open item required.

## Issues Encountered

- One hand-typed SHA literal (`stream-cpu-synthetic`) had a dropped byte on the first run. The test failed on it. The literals were then generated mechanically from the `cmake -E sha256sum` output and re-pasted.

## Open items

- **Requirements stay open:**
  - API-01 needs the isolation tests (plan 03-12).
  - API-06 needs the API effect table and its gate step (plan 03-13).
  - MEM-04 needs the load, unload and reset refusal rows (plans 03-10 to 03-12).
  - None was marked complete.
- **`api-statuses.ids` is not yet read by any gate step.** Plan 03-13's `api-effects` step compares it with the header, as does the `IVS_API` list equality. Today `machine.status.names` (stringized header names) and the acceptance grep (14 = 14) keep it honest.
- **Header self-containment and C++ include checks** (D-06) are still owed. Plan 03-13 per D-A11.
- **No digest-fields golden** (`data/expected/digest-fields.ids`, suggested by the refusal research file and not in this plan). The `IVS_MACHINE_BYTES` static assert, the coverage case and the domain version stand in. Plan 03-13 or a later plan may add it.
- **Hosted timing:** read from the next hosted run (budget 40 s, revisit trigger 50 s). Local warm gate is 21 to 26 s.
- **Load reports (plan 03-10)** embed the same `ivs_image_report` (A-08 f). Its `length_bytes` is u32.

## Known Stubs

None. The slot records hold zeros until plan 03-10 loads images. That is the documented state of a fresh instance (`loaded = 0`, status OK), not a stub.

## Threat Flags

None. The new public surface (create, destroy, info, requirements, digest) is the surface the threat register covers:
- **T-03-29:** validate before write, the poisoned-storage rows and `CREATE_MAGIC_BEFORE_PROFILE_CHECK`.
- **T-03-30:** the bounded 64-byte loop, exact-size id buffers under ASan, and no string imports.
- **T-03-31:** the zero-scrub, the magic check and `DESTROY_NO_SCRUB`.
- **T-03-32:** the coverage, sensitivity and literal streams, and four digest faults.

## User Setup Required

None.

## Next Phase Readiness

- **Plans 03-10 and 03-11** can write the slot records and `mem` through validate-then-commit. The IMAGES and MEM domains already serialize every slot field and every backing word.
- **Refusal rows** can use the poisoned whole-storage helper pattern of `machine_create.c` and compare `ivs_machine_state_digest` before and after.
- **Plan 03-12** adds `ivs_machine_reset` and the phase POWERED. The INSTANCE domain already carries the phase and `time_ticks`.
- **Final gate:**
  - `sh tools/gate.sh`: `gate: PASS steps=25/25 controls=215/215 cc=AppleClang c-presets=2 fault-runs=98 core-imports=4 purity-selftest=5/5 ... claims=7 provisional=6 ... faults=98/98 tracked=1096 ... tests-expected=1516 tests-executed=1516 tests-skipped=0 tests-missing=0 docs=12 claim-refs=6 tags=10 tiers=8` (`gate-time total=26`)
  - `dash tools/gate.sh`: the identical summary line (`gate-time total=21`).

## Self-Check: PASSED

- **Files found:** `core/include/ivs/ivs.h`, `core/include/ivs/ivs_machine.h`, `core/src/machine/machine.h`, `core/src/machine/machine.c`, `core/src/machine/machine_digest.c`, `tests/machine/machine_create.c`, `tests/machine/machine_digest.c`, `data/expected/api-statuses.ids` and `data/expected/faults/machine.ids`.
- **Commits:** `4ab9ed3`, `71bec21` and `bed72d7` are ancestors of HEAD. The ledger count from `30c1729` is 3, and each message ends with the Co-Authored-By line.
- **Hygiene:**
  - No absolute home path and no "Intellivision" in any added line; the profile id stays as registered.
  - No rights-bearing bytes; every literal is project-authored.
  - `.planning/config.json` and `.planning/state.json` are still ` M` and in no commit of this plan.
