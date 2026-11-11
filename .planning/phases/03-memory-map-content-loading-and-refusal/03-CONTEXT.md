# Phase 3: Memory Map, Content Loading and Refusal - Context

**Gathered:** 2026-11-11
**Status:** Ready for planning
**Mode:** `--auto` with the fan-out protocol (`.planning/AUTONOMY.md` §2). Five parallel research agents covered: public API and instance lifecycle (`03-DISCUSS-api.md`), memory map and width masking (`03-DISCUSS-memmap.md`), content loading and D5 (`03-DISCUSS-content.md`), cartridge import format (`03-DISCUSS-cart.md`), refusal matrix, atomicity and state digest (`03-DISCUSS-refusal.md`). The orchestrator reconciled their conflicts (section "Reconciliation"). Full option analysis lives in those files and in `03-DISCUSSION-LOG.md`; they are audit trail, not planning input.

<domain>
## Phase Boundary

A host can create isolated machine instances, load typed system images (EXEC, GROM) and a cartridge against the profile's memory map, and every bad input gets a typed refusal that leaves prior machine state unchanged.

In scope: MEM-01..05, API-01, API-06.
- Profile-driven memory map with a literal-map cross-check, width masking, undriven/blocked reads.
- First public headers (`core/include/ivs/`), caller-provided instance storage, create/reset/unload/destroy/load/info/state-digest.
- In-repo SHA-256, canonical state digest.
- Project cartridge container IVSC v1 and its fixtures.
- Refusal matrix, isolation test, machine-readable API effect table and the gate steps for them.

Not in scope: bounded run API and slicing (API-02, Phase 4), controller input (API-03), video/audio buffers (API-04), headless runner, installed consumer, boot stub authoring (Phase 4), STIC/PSG/GROM device behaviour (Phase 5+; Phase 3 has stub devices with explicit policies), `.rom`/`.bin+.cfg` importers, snapshots.
</domain>

<decisions>
## Implementation Decisions

### Instance lifecycle and public API (API-01, API-06)

- **D-01:** Caller-provided storage, opaque handle. `ivs_machine_storage_requirements(profile_id)` -> size/alignment; `ivs_machine_create(opts, storage, size, &m)`; `ivs_machine_destroy(m)` zero-scrubs and invalidates. No core allocation and no allocator callback in Phase 3 (an allocating convenience can be added later without a break).
- **D-02:** The instance is one flat pointer-free struct: magic word, profile index (u32), run phase (OFF / POWERED), machine time, the embedded 32-byte `ivs_cp1610`, RAM/GRAM/STIC/PSG state, slot records, worst-case content storage. `create` zero-fills the storage; misaligned or undersized storage is refused. Instance size is a compile-time constant per profile (about 150 KB estimated [INF]).
- **D-03:** `ivs_create_options { u32 struct_size; u32 flags (0); const char *profile_id; }`. No clock, seed, path or allocator in options. The profile registry is a const table generated from `data/profiles`; an unknown id is a registry miss (UNKNOWN_PROFILE).
- **D-04:** Every API struct starts with `uint32_t struct_size`. Below the minimum: BAD_ARG; unknown nonzero tail or flag: UNSUPPORTED; reserved fields must be 0. `IVS_API_VERSION_{MAJOR,MINOR,PATCH}` and `ivs_api_version()` exist.
- **D-05:** Phase 3 public surface is 10 functions: `ivs_api_version`, `ivs_status_name`, `ivs_machine_storage_requirements`, `ivs_machine_create`, `ivs_machine_destroy`, `ivs_machine_reset`, `ivs_machine_load`, `ivs_machine_unload`, `ivs_machine_info`, `ivs_machine_state_digest`. No public run or peek. The isolation test steps through an internal `ivs_machine_step_unit_internal` seam; Phase 4 builds `ivs_machine_run` on it.
- **D-06:** Headers: `core/include/ivs/{ivs.h, ivs_status.h (moved from core/src/common), ivs_machine.h}`, standard headers only, `extern "C"`, `#define` constants, an `IVS_API` macro. Static library only in Phase 3. A header self-containment test and an `IVS_API` symbol list-equality check stand in for a deferred export check. Phase 4's runner and installed consumer include only `ivs/ivs.h` and link `ivs::core`.
- **D-07:** Thread-safety contract: distinct instances are independent; the same instance is host-serialized. No globals, threads, locks or host callbacks in the public API. Enforced by an `nm` writable-data-symbol check on the core archive (known risk: ELF `.data.rel.ro` false positives; behavioural isolation tests remain the primary proof), an interleaved-vs-sequential isolation test and a memcpy-relocation test.
- **D-08:** A refused call writes only its out parameters. No last-error, counter or timestamp lives in the instance.

### Status codes and refusal semantics (MEM-04, API-06)

- **D-09:** Status is `int`; negative values are refusals; numbers are frozen, append-only, pinned by `data/expected/api-statuses.ids`. Keep `-1 BAD_ARG`, `-2 BAD_STATE`. Add `-3 BAD_HANDLE`, `-4 UNSUPPORTED`, `-5 BUFFER_TOO_SMALL`, and content codes `-16 UNKNOWN_PROFILE`, `-17 MALFORMED`, `-18 OVERSIZED`, `-19 OVERLAP`, `-20 OUT_OF_RANGE`, `-21 MISSING_SYSTEM_IMAGE`, `-22 DUPLICATE_LOAD`, `-23 DIGEST_MISMATCH`; `-24..-31` reserved. Detail, segment index and byte offset travel in an out report struct, not the status. `ivs_status_name()` maps codes to stable names.
- **D-10:** Atomicity is validate-then-commit. Validation takes `const ivs_machine *` and builds a stack plan; commit returns void and is copy-only. Staging in a copy, undo logs and replace-on-duplicate are rejected.
- **D-11:** Load precedence, first failing class wins: BAD_ARG, BAD_HANDLE, BAD_STATE (machine is POWERED), DUPLICATE_LOAD (slot occupied), OVERSIZED (u64-widened length checks), MALFORMED (zero length and out-of-width words included), OUT_OF_RANGE, OVERLAP, DIGEST_MISMATCH, then commit. A `prec.*` matrix row proves each adjacent pair.
- **D-12:** Every Phase 3 refusal has state effect `none`: the instance storage is byte-identical (whole-storage `memcmp`) and the state digest is equal. Each matrix row runs against several prior states (fresh, system images loaded, full, powered).
- **D-13:** Loads and unloads are legal only while the machine is OFF. Loads are order-free (EXEC, GROM, cartridge in any order). A slot holds one image: loading an occupied slot is DUPLICATE_LOAD (unload first). `unload(kind)` on an empty slot is OK. After unloading everything the state digest equals a fresh instance.
- **D-14:** MISSING_SYSTEM_IMAGE is refused by `ivs_machine_reset(IVS_RESET_POWER_ON)` when a required slot (EXEC, GROM for the D3 profile) is empty, with effect `none`. MEM-04's "missing-system-image load" is read as "a start without a required image" (reset is the first operation that needs it); the refusal matrix row is `reset` with an empty EXEC and another with an empty GROM. A successful reset moves OFF -> POWERED, sets volatile state to power-on fill, CPU to its reset entry, time 0.
- **D-15:** API effect table: `data/api/api-effects.json` (call x status x effect x test id), checked by a new cmake -P gate step against the status pin, the headers, and `data/expected/tests/*.ids` (every error code has at least one test and a documented effect). A new `machine` test suite and executable holds refusal, isolation, digest and sha256 ids.

### Memory map and width masking (MEM-01, MEM-02)

- **D-16:** Descriptors are profile fields with per-field tag/cite/conflict: `memmap.region.<name> {view, lo, hi, device, size_words, gate, impl}`, `memmap.device.<kind> {bits, rmask, fill, store}`, `memmap.undriven_read`, `memmap.cart_window`, `memmap.cart_align_words`. `cmake/IvsMemmapTable.cmake` (fail-closed, build tree only, MUTATE option for controls) emits a const region array, `rd_region[4096]` and `wr_region[4096]` (index `addr>>4`) and the profile registry.
- **D-17:** One width formula: `read = (stored & rmask) | (fill & ~rmask)`. ROM decle 10 bits (`03FF`, fill 0; GI 1980 pdf 194/197, tier 1), GROM 8 bits (`00FF`), system RAM 16 bits, scratchpad 8 bits (`00FF`, upper bits `unknown-default-chosen`), undriven reads `FFFF` (C5, default-chosen). Stores truncate to `rmask`. Loaders refuse an out-of-width word, never truncate.
- **D-18:** Region table (views are separate for read and write): STIC `0000-003F` RW stub plus `0040-007F` read alias; scratchpad `0100-01EF` ram8; PSG `01F0-01FF` stub; system RAM `0200-035F` ram16; EXEC `1000-1FFF` rom10 read-only; GROM `3000-37FF`; GRAM `3800-3FFF` mirror 512; write-only STIC/GRAM aliases; cartridge in `4000-FFFF` as per-segment rom10 windows (`memmap.cart_window`, alignment 256 words; real windows are tier 4 and recorded as discrepancy C62). The literal map overrides any number in this list where a source differs.
- **D-19:** Gates (`stic-regs`, `gmem`) are open by default; an internal setter closes them; a closed gate reads the undriven fill and drops writes (C5). An unloaded EXEC, GROM or cartridge region reads undriven, not zero (decle 0 is HLT). Devices that do not exist yet (STIC, PSG, GROM engine) are stubs with explicit undriven policies; no heap, no per-device function pointers; dispatch is a switch on a device-kind enum.
- **D-20:** One bus function serves the CPU port: FETCH/READ/ADAR read, WRITE/DW write, IAB returns the profile vector, INTAK data 0, wait 0. Mutable decode state is only `cart_page[]`.
- **D-21:** Literal map `data/memmap/<profile>.literal.txt`, one row per line (view, range, device, width, impl, mirror, gate, `src=SRC-id@locator`, tag, conf). It is authored from sources only (tier 5 refused), must tile `0000-FFFF` per view with explicit undriven rows. The `memmap` gate step dumps the runtime decode by probing all 65,536 addresses through the real bus functions and requires a byte-equal match on the structural columns. Claim tier: source-expectation.
- **D-22:** Boundary probes (`lo-1, lo, hi, hi+1`, mirror wrap) are generated from the literal rows; expected values come from a rule table keyed on device and width, never from the descriptors under test.
- **D-23:** New discrepancies: C60 (upper bits of 8-bit RAM, PSG and STIC reads), C61 (write-only alias vs cartridge ROM), C62 (cartridge window). Retarget C21's resolver to D10; mark C51 partial; update `profile-fields.ids`.

### Content loading, system images and D5 (MEM-03)

- **D-24 (D5, delegated decision):** System images are user-supplied only. No built-in, fetched or high-level-emulated image. A project-authored boot stub loads through the same EXEC slot, ships as a text decle list with a manifest, and is labelled "project boot stub, not an EXEC" (authored in Phase 4; Phase 3 tests generate stand-ins). Reverse if: the owner wants plug-and-play commercial play (a separately labelled HLE) or permission to redistribute an image appears.
- **D-25:** `ivs_machine_load(m, kind, bytes, len, opts, &report)` with kinds `EXEC`, `GROM`, `CARTRIDGE`; one typed image per call. The core copies the span into instance storage during the call; no pointer is retained and nothing is allocated. Report per slot: `length_bytes`, SHA-256 over the exact supplied container bytes (equals `sha256sum file`), `length_units`, `unit_bits`, `base_addr`, `loaded`. A never-loaded slot reports `loaded=0`, status OK. `ivs_machine_info` returns the same per slot.
- **D-26:** Raw system-image container: 16-bit words, **high byte first**, bits 15..10 must be 0. EXEC is exactly 8192 bytes (4096 decles); GROM is exactly 2048 raw bytes. No padding tolerance. Wrong size: OVERSIZED or MALFORMED per D-11; out-of-width word: MALFORMED with the offset of the first bad word. The byte order rests on background knowledge [BG] of existing dumps and is checked only on a real file under D11 (a wrong guess surfaces as a refusal, never a silent miscompute).
- **D-27:** Identity is a recorded fact; the core checks shape only. Optional `expect_sha256` per load gives DIGEST_MISMATCH. No hash database. At most one owner-attested hash per role, only under D11.
- **D-28:** SHA-256 is implemented in-repo from FIPS 180-4 in `core/src/common/ivs_sha256.{h,c}`: streaming, integer-only, no libc beyond what the purity allowlist already permits. Tests: FIPS vectors, padding-boundary lengths checked against `cmake -E sha256sum` as an independent implementation, split-at-every-offset invariance, six named faults.

### Cartridge import format IVSC v1 (MEM-05)

- **D-29:** The core cartridge format is **IVSC v1**, specified in the project's words in `docs/formats/ivsc-v1.md` (no third-party name; extension `.ivsc`). It is not `.rom`, `.bin+.cfg` or raw binary. Importers for those formats live outside core, arrive in a later phase and each needs its own rights review (the jzIntv-hosted `.ROM` docs were read as tier-4 leads only; no emulator source was opened).
- **D-30:** Layout: 8-byte magic `89 'I' 'V' 'S' 'C' 0D 0A 1A`; 24-byte header `version u16 = 1, flags u16 = 0, required_features u32 = 0, total_bytes u32, segment_count u16, reserved u16 = 0` (+ padding to 24 per the spec); per segment an 8-byte entry `addr u16, len_words u16, width u8 = 10, attr u8 = 0, reserved u16 = 0`; then the data words (no stored offsets), then a 32-byte SHA-256 trailer over every preceding byte. **Byte order is big-endian throughout** (reconciled with D-26 so the project has one wire byte order; this overrides the research file's little-endian proposal). A decle is a u16 with value at most `0x3FF`; larger values are refused, not masked.
- **D-31:** Limits: 1..16 segments, 1..65535 words each, ascending and non-overlapping (adjacent allowed), at most 131,256 bytes total. An oversize length is refused before any byte is read. Every undefined version, flag, reserved field, width, attr or feature bit is refused; there are no ignorable chunks and no metadata. The format evolves by version bump.
- **D-32:** `required_features` is a u32 mask, 0 in v1; the profile field `cartridge.features_supported = 0` covers the D3 profile. An unsupported bit gives UNSUPPORTED (BAD_ARG-class codes are not reused) and the report carries the missing bits.
- **D-33:** The parser is a pure, zero-copy, bounded function in `core/src/content/ivsc.c`, check order: length, magic, version, header, table, order/overlap, digest, word range. Layer 2 (`ivs_machine_load` for CARTRIDGE) takes the validated view. A segment must lie wholly inside the profile's cartridge window (D-18), otherwise OUT_OF_RANGE. Refusal reasons map: bad structure MALFORMED, size OVERSIZED, overlap OVERLAP, window OUT_OF_RANGE, feature UNSUPPORTED, trailer DIGEST_MISMATCH. The report carries reason, segment index, byte offset, missing features.
- **D-34:** Fixtures are text JSON under `data/cart/`. Positives are full hex with a digest typed in independently from `cmake -E sha256sum`; negatives are patches of a positive base; tables are built in the build tree; one manifest `FX-ivsc-vectors` lists the files; no `.ivsc/.rom/.bin` is ever committed; maximum-size images are built in test code. Fixture addresses are generated from the literal map, never hard-coded (test X01 detects window drift). A seeded cartridge fuzz suite and an optional non-gate libFuzzer target accompany them. The research file's exact id list (70 ids across P/B/N/L/X) is the starting inventory and is finalized in planning.

### State digest and test architecture (API-06, success criteria 3 and 5)

- **D-35:** The state digest is SHA-256, computed per domain (CPU, MEM, IMAGES, INSTANCE); the composite hashes domain ids and digests under a version. Serialization is **big-endian** (project-wide rule, D-26), built field by field from one field table with u32 counts on lists. It excludes pointers, padding, magic and counters. RUN-05 video/audio/input domains reuse the framing as reserved ids.
- **D-36:** Isolation tests: interleaved vs sequential equality of two instances with different content, differently poisoned storage ending byte-identical, memcpy relocation, destroy-mid-run, cross-instance load isolation. No threaded test in the gate.
- **D-37:** Named-fault controls: generalize `c-faults`, `faults-verdict` and `faults.cmake` to loop over `data/expected/faults/*.ids` (CPU, memmap, machine, sha256, ivsc). Planning finalizes the list; the research files give about 17 machine faults, 11 memmap faults, 6 sha256 faults and 6 cartridge faults.
- **D-38:** Gate cost: new steps are `memmap` and `api-effects` plus the generalized faults loop. Estimated +2 to 4 s hosted. The D-22 hosted budget is re-baselined from 30 s to 40 s with the same 40 s revisit trigger moved to 50 s; measure after the first plan lands and again at phase end, and keep levers (overlap, owning-TU rebuild) in `CI-EFFICIENCY.md`.

### Claude's Discretion

Plan ordering, exact struct layouts, the names of internal helpers, the final fixture and fault id lists, TAP suite splitting, and which pure-sh vs cmake -P form each new gate step takes (prefer cmake -P for JSON).

### Reconciliation (orchestrator, where research files disagreed)

- Status numbering: the API file's banded registry won over the refusal file's contiguous one; DUPLICATE_LOAD and DIGEST_MISMATCH added from the content/refusal files.
- Load model: single `ivs_machine_load` with a kind (API file) over per-class calls (content file); loads only while OFF and no replace (content, refusal files) over "load is a power event, replace if occupied" (API file).
- Byte order: one project-wide big-endian rule for system images, IVSC and digest serialization.
- Missing system image is owned by `reset` (content, refusal), with the MEM-04 reading recorded in D-14.
- API-effects table is JSON checked by cmake -P (project norm) rather than TSV plus awk.


### Amendments from research and pattern mapping (supersede conflicting text above)

- **A-01 (cart alignment):** There is no alignment rule in Phase 3. A segment must lie wholly inside the profile cartridge span and must not overlap another segment; nothing else constrains its start. `memmap.cart_align_words` is dropped from D-16; the 256-word alignment seen in community documents is recorded only as a tier-4 note under C62. One-word fixtures are therefore legal.
- **A-02 (field name):** The profile field `memmap.cart_window` is named `memmap.cart_span` (the glossary step requires a unit suffix on any field ending in `window`). D-18/D-33 read "cartridge span".
- **A-03 (IVSC prefix):** The fixed prefix is 24 bytes including the 8-byte magic and has no padding (magic 8, version 2, flags 2, required_features 4, total_bytes 4, segment_count 2, reserved 2); the minimum valid file is 66 bytes (prefix 24 + one 8-byte entry + one 2-byte word + 32-byte trailer). D-30's "24-byte header + padding" is withdrawn.
- **A-04 (two digests):** The IVSC trailer SHA-256 (over all bytes before the trailer) and the reported whole-file SHA-256 (D-25) are different values; the loader computes both in one pass with a copyable SHA-256 context. The report carries the whole-file digest.
- **A-05 (observability seams):** Planning adds (a) an internal raw-poke seam so ROM read masks and stub devices can be tested although the loader refuses wide words, and (b) a device-kind column in the memory-map dump so a stub region is distinguishable from an undriven region. The raw-poke seam is internal only (never in `core/include`).
- **A-06 (gate hygiene):** `c-purity` must scan `core/include`, hook generated includes through a reviewed list, and compile with `-fno-pie` for the writable-data rule; profile-id matching uses a bounded loop (no `strlen`/`strcmp`; `memcpy`/`memset`/`memcmp` stay allowed); input buffers overlapping instance storage are refused BAD_ARG with a matrix row.
- **A-07 (budget):** D-38's estimate is optimistic; the research projects +6 to 10 s hosted. The re-baseline to 40 s stands with a revisit trigger at 50 s; measure at plan 03-02 and again at 03-07, and apply levers from `CI-EFFICIENCY.md` before accepting more.

- **A-08 (load check order, recorded after plan check):** (a) The claimed length bound (OVERSIZED) is checked before any buffer-overlap test, and the overlap test (a caller buffer overlapping instance storage, BAD_ARG) runs only on spans that already pass the length bound, using `uintptr_t` arithmetic that cannot wrap; so a refusal status is a pure function of argument values and machine state, never of where the host allocated the buffer. A `machine.prec.*` row pins it. (b) The staged order (arguments, handle, options/UNSUPPORTED after BAD_HANDLE, state, duplicate, then content with the layer-1 parser in D-33 order, then layer 2 span/feature checks, then `expect_sha256`) refines D-11 for cartridges: overlapping segments that also leave the span report OVERLAP, and a bad trailer with a wide word reports DIGEST_MISMATCH; this is intended, not a D-11 violation. (c) The bounded scan of at most 16 committed segments replaces `cart_page[]` in D-20 (A-01 makes a page table impossible); it is the only mutable decode state. (d) Reset from POWERED is a power cycle; nothing returns POWERED to OFF except `destroy`, so load/unload after the first reset are BAD_STATE until a new instance (the effect table says so). (e) Unknown enumerants are BAD_ARG in every call that takes an enum (load kind, reset kind); UNSUPPORTED is reserved for unknown nonzero option flags and struct tails (D-04). (f) `ivs_machine_info` returns an embedded `ivs_image_report`, the same struct the load report embeds.

</decisions>

<canonical_refs>
## Canonical References

- `.planning/ROADMAP.md` Phase 3; `.planning/REQUIREMENTS.md` MEM-01..05, API-01, API-06.
- `.planning/AUTONOMY.md`; `docs/RIGHTS.md`; `docs/time-base.md`; `docs/fixture-manifest.md`.
- `data/profiles/intv-2609-ntsc-family-p1.json`; `data/discrepancies.json` (C5, C20, C21, C51).
- Phase 2: `02-CONTEXT.md`, `core/src/cpu/cp1610.h`, `docs/cp1610/stepper-conventions.md`.
- Sources: GI 1980 data catalog pdf 191-202 (parts and widths); Blue Sky Rangers 2609 page; FIPS 180-4; community docs only as tier-4 leads.
- Research files in this directory: `03-DISCUSS-{api,memmap,content,cart,refusal}.md`.
</canonical_refs>

<deferred>
## Deferred Ideas

- Allocating create convenience; `ivs_machine_run`, peek/poke; export/visibility check for shared libs (Phase 4+).
- `.rom` / `.bin+.cfg` importers (later phase, own rights review); bundle atomic `load_set`.
- Real GROM/STIC/PSG device behaviour; D10 measurements resolving C20/C21/C60-C62.
- Opt-in masking flag if real dumps carry stray top bits (D11).
</deferred>
