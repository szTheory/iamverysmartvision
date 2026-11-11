# Phase 3 discuss, decision point C: content loading and system-image supply (D5)

Scope: MEM-03, the system-image part of MEM-04, API-06 for load/unload/reset, and D5.
Labels: [OBS] fetched or run this session, [INF] inference, [REC] recommendation, [UNK] unknown, [BG] background not re-fetched.
Page cites for the GI 1980 catalog (`SRC-gi-1980-catalog`) are "pdf N" of the file whose sha256 matches `data/sources.json` (re-downloaded and verified 2026-11-10, `70094d12...`; copy kept outside the repo).
Tier-4 community docs (jzIntv `doc/` tree, Intellicart manual) were read as documentation only, as leads; no emulator source was opened.

## 1. Facts established (what the options are judged against)

- [OBS] RO-3-9502 program ROM is "2048 x 10 bit words"; on a read it "will output the 10 bits of addressed data and also drive a logic zero on the top six bits of the bus" (GI 1980, pdf 194 = printed 5-13). So a ROM read yields bits 15..10 = 0. This settles the "what ROM upper bits read as" item left open by C51 for tier 1: the ROM read mask is 0x3FF.
- [OBS] RO-3-9504 is also 2048 x 10 (pdf 202, printed 5-21). RO-3-9503 graphics ROM is "2048 x 8 bit words", "higher 8 bits driven to zero during read operations" (pdf 197, printed 5-16).
- [OBS] The 9502 supplies the start vector and the interrupt vector over the bus during IAB; both are "programmable" on the chip (pdf 194). [INF] The vectors are chip programming, not array content, so an EXEC dump need not contain a reset vector. This matches `reset.r7` / `interrupt.iab_value` living in the profile and answered by the bus layer (C32, C42): the image slot carries code only, and a stand-in image needs no vector words.
- [OBS] Memory-map placement of the 2K ROMs is mask-programmable on 2K boundaries (pdf 194).
- [OBS, tier 2] BSR: the EXEC is "a 4K program" in "the Executive ROM chip (with an additional 344 bytes over in the GROM chip)"; physically two 2K chips (RO-3-9502 + RO-3-9504) in the 2609, one 4K RO-3-9506 in the Intellivision II. GROM is a "2K ROM" with 213 images; the intended 256 images x 8 bytes = 2048 bytes. [INF] 213 x 8 + 344 = 2048, so the GROM image carries EXEC code and a real EXEC does not boot without a real GROM; the pair is one unit.
- [OBS, tier 4] jzIntv `memory_map.txt`: EXEC at $1000-$1FFF, GROM at $3000-$37FF, "$5000-$6FFF first 8K of default cartridge map". The tier-4 EXEC boot sequence checks $7000, $4800, then reads a header at $5000-$5014. [INF] A stub must not imitate that header protocol; see 4.
- [OBS, tier 4] Intellicart manual: the `.rom` download format is 0xA8, segment count, its ones complement, then per segment start-high, stop-high, 2 bytes per word, CRC-16 (poly 0x1021, init 0xFFFF), then access/fine-range tables. Words are 16-bit. [UNK] Word byte order is not stated in the docs I read. [BG] Community `exec.bin` is 8192 bytes of big-endian 16-bit words and `grom.bin` is 2048 bytes. Not verified, and no real image is available here.
- [OBS] The project's own table: decle 0x000 is HLT (`data/cp1610/opcode-families.json`). A zero-filled unused stand-in area therefore halts deterministically instead of running garbage.
- [OBS] `core/src/common/ivs_prelude.h` allows `<limits.h> <stdbool.h> <stddef.h> <stdint.h> <string.h>`; the symbol allowlist permits `memcpy memmove memset memcmp`. A core SHA-256 and copy-in loader need nothing else. Status codes today: `IVS_OK`, `IVS_E_BAD_ARG`, `IVS_E_BAD_STATE`.
- [OBS] hashlib reproduces the FIPS 180-4 example digests: "" `e3b0c442...b855`, "abc" `ba7816bf...15ad`, 56-byte `248d6a61...06c1`, 112-byte `cf5b16a7...e9d1`, 10^6 x "a" `cdc76e5c...2cd0`.
- [OBS] Profile `system_images.exec_grom` is currently one free-text field "required EXEC/GROM slot; digest bound at run time; a different digest is a different profile identity; supply policy pending D5". It has no machine-readable size, width, placement or required flag.

## 2. Options and verdicts by sub-question

### 2a. API shape for typed inputs
| Option | For | Against |
|---|---|---|
| A. One function per input class, each validate-then-commit and atomic: `load_system_image(kind, span)`, `load_cartridge(span)` | Easiest C/FFI (Rust, Python, NIF) surface; one documented state effect per call; images are independent so per-call atomicity is enough; hosts load in any order | Cross-image checks cannot happen at load |
| B. One `load(request{exec, grom, cart})` bundle | Cross-validation in one place | Struct-versioning burden before the ABI exists; hosts that swap only a cartridge must pass absent fields; bigger refusal matrix per call |
| C. Stage everything, validate at `reset` | Order-free | Errors arrive far from the cause; weakens "refused load leaves prior state unchanged" into "load never refuses" |
- [REC] A. The one cross-slot rule (cartridge versus the fixed system ranges, "overlapping" in MEM-04) is checked against the profile's static map, not against loaded images, so A loses nothing. Reverse to B if a later content type needs joint validation (ECS plus IntelliVoice ROM sets).

### 2b. Ownership and lifetime (copy versus borrow)
- Borrow (keep the pointer until unload): saves 10 KiB or so of copy; but host buffer lifetime and mutation become part of machine determinism, mmap'd files can change under the core, the isolation test becomes a host-discipline test, and every API-06 line grows a lifetime clause.
- Copy (span valid only for the duration of the call; the core copies into instance storage): [REC]. Fixed-size arrays inside the instance (EXEC 4096 x u16, GROM 2048 x u8, cartridge up to the profile cap); no allocation in the core; instance stays a plain struct with zeroed padding, so "refused or unloaded equals fresh" is a `memcmp`. No interior pointers are ever returned (peek is by value), so a host cannot alias or mutate stored images.
- Snapshot consequence (Phase 8): snapshots bind images by sha256 and never embed their bytes, because bytes are rights-bearing; restore refuses when the loaded digests differ.
- Footgun avoided: copy-then-validate leaves half-written state on refusal. Order is validate against the borrowed span (pure), compute the digest into a local, then an infallible `memcpy` commit.

### 2c. Digest: what is reported, and the algorithm
- "Length and digest reported" [REC]: for each slot the core returns and later re-reports `ivs_image_report{kind, loaded, length_bytes, length_units, unit_bits, base_addr, sha256[32]}`. `length_bytes` and `sha256` are over the exact container bytes the host supplied, so they equal `sha256sum file`. `length_units` is decles (EXEC, cartridge) or bytes (GROM). Digest over post-mask memory contents is rejected: it would hide differences the loader already refused, and it would not match any published hash.
- Algorithm options: SHA-256 in-repo; CRC32 (community databases use it [BG], but trivial collisions and no identity value); SHA-1 (broken, no benefit); BLAKE3 or xxHash (a dependency or non-interoperable with `shasum`, manifests and `sources.json`); host-computed digest passed in (core would report an unverified claim); a vendored library (dependency, vs "another copy-paste is better than another dependency").
- [REC] SHA-256, written in-repo from FIPS 180-4 as `core/src/common/ivs_sha256.{h,c}` (about 100 lines, integer-only, `uint32_t` arithmetic with explicit rotates, big-endian length). Streaming `init/update/final` (not one-shot) so Phase 8 per-domain digests (RUN-05) reuse it. It lives in the core because the core reports digests; it needs no libc beyond optional `memcpy`, so `c-purity` passes unchanged. Add the file to `IVS_CORE_SOURCES`.
- Verification [REC]: literal FIPS vectors (empty, "abc", 448-bit, 896-bit, 10^6 x "a"); padding-boundary lengths 0, 1, 55, 56, 57, 63, 64, 65, 119, 120, 127, 128, 129, 2048, 8192 with expected digests produced by an independent implementation (hashlib or `cmake -E sha256sum`) and committed as literals with their provenance; split-at-every-offset invariance (update in two parts equals one part) over a 130-byte message. Tier label: source expectation plus independent implementation comparison.
- Named faults: wrong K constant, wrong rotate amount, length written little-endian, padding off by one at 55/56, final block dropped, state not reset by `init`. Each must fail a named vector.

### 2d. Container for 10-bit words
| Option | Verdict |
|---|---|
| 16-bit words, high byte first, bits 15..10 must be 0 (EXEC 8192 bytes) | [REC]. Matches the de facto files [BG]; canonical, so container bytes and decles are one-to-one |
| Same, little-endian | Rejected: no source; would reject the files users actually have if [BG] holds |
| Packed 10-bit (5120 bytes) | Rejected: no tool or dump uses it that I found |
| Split low-8 / top-2 files (BSR mentions this for prototype EPROMs) | Rejected: two files for one input |
| Accept either order by guessing | Rejected: guessing hides errors |
- GROM: raw 2048 bytes, one byte per location.
- Strict, not masking [REC]: a word with any of bits 15..10 set is refused (`IVS_E_IMAGE_WORD`, detail = first offending offset). Silent masking would make two different byte strings produce one machine state with two digests. Byte-order mistakes are caught because a big-endian file read little-endian has a nonzero top-six field almost every time (low byte above 3), so the failure is loud, not silent. [UNK] Real dumps might carry stray top bits; if D11 shows that, add an explicit opt-in mask flag as an additive change.
- Sizes are exact, with no padding or alignment tolerance: EXEC = 8192 bytes (4096 decles), GROM = 2048 bytes. Oversized, short, empty and NULL-with-length all refuse with one `IVS_E_IMAGE_SIZE` carrying expected and actual in the report. The EXEC is one 4K slot: the two physical 2K chips are an internal detail and the profile asserts no split (C20).
- Placement (profile data, tier 4 plus C20 note): EXEC base $1000, GROM base $3000.

### 2e. D5 supply policy
| Option | Pros | Cons |
|---|---|---|
| 1. User-supplied only, plus a project-authored boot stub for CI [REC] | Zero rights exposure; CI never needs a real image; honest labels | Real-machine compatibility claims stay owner-local (D11, SCN-02) |
| 2. A core-built-in HLE or "replacement" EXEC | Plays commercial ROMs out of the box | Out of Scope per REQUIREMENTS; misrepresents the EXEC; its contract changes timing; big testing burden |
| 3. No images at all (CPU-only profile) | Simplest | No end-to-end boot path; RUN-06 needs reset through a boot path |
| 4. Download or fetch on first run | Convenient | Rights and "CI never caches" violations; host IO in the wrong layer |
- [REC] Option 1, with these rules: the core has no built-in image and no path or filename knowledge; the repo ships no real EXEC or GROM, ever (hygiene already enforces); the stub is just a project-authored image loaded through the normal EXEC slot, so there is exactly one load path. The stub is identified by its digest in scenario or report data, never by a core flag. Reports and docs label it "project boot stub, not an EXEC". It is for original fixtures only.

### 2f. Boot path without a real EXEC (feeds RUN-06, built in Phase 4)
- Stub image = 4096 decles, committed as a text decle list (hex words) with a manifest (`kind original-source`, redistributable yes, MIT), not as a binary, so `files`/`integrity` need no binary exception. A small test/runner helper expands it to the 8192-byte container. Everything not written is 0x000 = HLT.
- Contract [REC, details are Phase 4's]: code at $1000 (reset) and $1004 (interrupt entry), the profile's answers to IAB. Reset code does DIS, sets R6, then jumps to a project-defined cartridge entry. It never reads the $5000 EXEC header; a commercial cartridge's header would run as code, which is the intended "not an EXEC" failure mode. The interrupt entry returns through the stack convention of C17. No timing claim is made for the stub.
- GROM for stub runs: a project-authored 2048-byte stand-in (all zero is enough until Phase 6). The profile keeps `grom.required = 1`, because hardware has a GROM and a real EXEC needs it; a scenario supplies the stand-in explicitly and the report names its digest.
- Phase 3 itself needs no stub file. Tests use generated stand-ins (word i = (37*i + 11) & 0x3FF, so every address has a distinct value) built in test code, never committed as bytes.

### 2g. Identity, known-image representation, proof
- [REC] The core accepts shape as proof, never identity: exact size, canonical words. Identity is a recorded fact. A caller may pass an optional `expect_sha256` (32 bytes) with a load; the core compares and refuses with `IVS_E_DIGEST_MISMATCH`, no effect. This gives "a different digest is a different profile identity" an enforcement point without any database in the core.
- No bulk hash database anywhere (RIGHTS). Run records carry (profile id, per-slot sha256, length). A scenario file may assert expected digests. The only third-party hash that may ever be recorded is a single owner-attested identity per role, as a fact, under D11 and SCN-02; it is [UNK] today and not needed for Phase 3.
- A matching hash grants nothing: docs state "identity, not rights". Reports and error text carry digests, offsets and lengths, never image bytes.

### 2h. Missing system image, `requires`, and running without images
- [REC] The profile gets machine-readable slot descriptors (generated header, same pipeline as MEM-01): `system_images.exec` {words 4096, word_bits 10, base $1000, required 1}, `system_images.grom` {bytes 2048, bits 8, base $3000, required 1}, with tags and `C20`/`C32` cites. `required` is data, so a later profile can differ. `ivs_profile_slot_info(profile, slot, out)` lets a host ask what to supply.
- Policy: in p1 the machine does not start without both slots. Loads are accepted in any order. `ivs_machine_reset` (power-on) refuses `IVS_E_MISSING_SYSTEM_IMAGE` when a required slot is empty, with no state effect, and `ivs_machine_missing(m)` returns the slot bitmask. Stepping before a successful reset stays `BAD_STATE` / NOT_RESET (Phase 2). Debug peeks work regardless of readiness.
- Rationale against refusing the cartridge load itself when EXEC is absent: host order-independence and simpler atomicity. [INF] REQUIREMENTS MEM-04 says "missing-system-image loads"; read it as "a start without a required image", and the plan should word the test that way. Escalate to strict-at-load only if the synthesizer reads the requirement literally; it costs one check in the cartridge call.
- No CPU-only "machine" mode: Phase 2 vectors keep using the flat test bus.

### 2i. Lifecycle and refusal semantics (what 03-CONTEXT should state)
- Loads and unloads are legal only while the machine is OFF (before the first reset, or after unload). A load while powered returns `IVS_E_BAD_STATE`, no effect. Replace-by-load is allowed in OFF; a refused replacement keeps the old image.
- `ivs_machine_unload(m, slot_mask)` clears the chosen slots to zero and returns the machine to OFF. Test: create, load, unload equals a fresh instance under `memcmp`.
- Validation order, fixed and documented, first failure wins: BAD_ARG (null handle, unknown kind, NULL with nonzero length) then BAD_STATE then IMAGE_SIZE then IMAGE_WORD then DIGEST_MISMATCH. Every refusal happens before the first write to the instance.
- New status codes (stable negative values appended to `ivs_status.h`): `UNKNOWN_PROFILE` (at create; no instance, `*out = NULL`), `IMAGE_SIZE`, `IMAGE_WORD`, `DIGEST_MISMATCH`, `MISSING_SYSTEM_IMAGE`, `UNSUPPORTED` (known kind not in this profile). Cartridge codes (`MALFORMED`, `OVERLAP`, `RANGE`) belong to the cartridge-format decision and should come from one shared table. Details (expected, actual, offset) travel in the report, never in the status.
- State effect of every refusal: "none: no byte of the instance changes (padding included); the caller's report is written with `loaded = 0` and detail fields". API-06 matrix test: for each refusal, `memcmp` of the whole instance before and after.

## 3. Lenses
- Architect: one load path, copy-in, plain-struct instance, digests and descriptors as data; the stub is not special in the core.
- Hardware/EE: ROM reads drive zeros on the top six bits (pdf 194, 197), vectors come from the chip, and 10-bit versus 8-bit slot widths follow the parts. The two-chip EXEC split is not modelled (C20).
- QA: every refusal class, the precedence order, the copy proof, fresh-equals-unloaded and SHA vectors are automatable with literal expected values; nothing for human UAT.
- Security/rights: no image bytes in repo, logs, reports or CI cache; hash-only identity; stub labelled; NULL/length/overflow checks come first, and exact-size checks make `len` arithmetic overflow-free.
- DevOps/CI: SHA and loader tests are microseconds (million-"a" under ASan is tens of ms); a new `mem`/`sha256` suite plus `data/expected/faults/mem.ids`; no new gate step beyond the existing inventory and fault steps, no new dependency.
- API/integrator: per-class calls, copy semantics, no interior pointers, status plus report struct, descriptor query. FFI-friendly.
- Product/DX: a wrong-endian or wrong-size file gets a specific refusal with offset and sizes; `missing()` mask tells the host what is still needed; no magic detection.

## 4. Adversarial pass against the recommendation
1. The boot stub becomes a de facto HLE EXEC and users blame the emulator for commercial games. Counter: no core flag, no name, digest-labelled, header protocol deliberately not imitated, never offered in releases as a way to play commercial content. Residual risk stays; it is a documentation and reporting discipline.
2. Strict word validation rejects real dumps with stray bits. Counter: loud, diagnosable, additive fix; evidence only at D11 ([UNK]).
3. Byte order wrong ([BG] only). Counter: top-six check makes a wrong guess a refusal, not a miscompute. Stand-in tests use the chosen order, so they cannot detect a wrong choice; only a real-file check at D11 can. Accepting the other order later is additive.
4. In-repo SHA-256 bug silently corrupts identity. Counter: FIPS plus independent-implementation vectors at every padding boundary, split invariance, named faults.
5. Cartridge digest meaning is not yet settled: for a `.rom` container, whole-file digest (file identity, what users hash) versus per-segment payload digest. [REC] report the whole-blob digest plus per-segment lengths; revisit with the cartridge-format decision. A parsed container is not canonical, so one memory image can have several valid digests; that is acceptable for identity of a file.
6. Copy-in makes the instance big (cartridge cap decides; at most 128 KiB for 64K words of u16). Counter: fixed arrays, no allocation, snapshot excludes them.
7. "Required GROM" forces CPU-only fixtures to carry a stand-in. Counter: one line in the scenario; honest, and the stand-in digest appears in every report.

## 5. Recommended decisions (copy into CONTEXT as D-xx)
1. Typed per-class calls (`load_system_image`, `load_cartridge`, `unload`, `image_report`), each validate-then-commit and atomic; a refusal changes no byte of the instance.
2. The core copies the span during the call into fixed instance storage; no pointer is retained, no allocation, no interior pointer returned.
3. Reported per slot: `length_bytes` and SHA-256 over the supplied container bytes, plus `length_units`, `unit_bits`, `base_addr`, `loaded`; queryable at any time; a never-loaded slot reports `loaded = 0` with status OK.
4. SHA-256 implemented in-repo from FIPS 180-4 as `core/src/common/ivs_sha256.{h,c}`, streaming, integer-only, with FIPS plus boundary-length vectors and six named faults.
5. Container: 16-bit words, high byte first, bits 15..10 zero (EXEC exactly 8192 bytes = 4096 decles); GROM exactly 2048 raw bytes; no padding tolerance; strict refusal, never masking.
6. D5: user-supplied EXEC/GROM only; no built-in or fetched image; a project-authored boot stub loaded through the same EXEC slot, committed as a text decle list with a manifest, labelled "project boot stub, not an EXEC", built in Phase 4 (Phase 3 tests use generated stand-ins).
7. Boot-stub contract for Phase 4: code at $1000 and $1004, DIS plus stack init plus jump to a project-defined cartridge entry, no EXEC-header protocol, unused words 0x000 (HLT); GROM stand-in is project-authored, zero-filled for now.
8. Identity is a recorded fact: core accepts shape only; optional `expect_sha256` per load gives `IVS_E_DIGEST_MISMATCH`; no hash database; at most one owner-attested third-party identity per role, only under D11.
9. Profile gets machine-readable slot descriptors (words/bytes, bits, base $1000/$3000, `required` = 1 for both) with tags and a `ivs_profile_slot_info` query; ROM reads mask to 10 bits and GROM to 8 (GI 1980 pdf 194 and 197).
10. `reset` refuses `IVS_E_MISSING_SYSTEM_IMAGE` when a required slot is empty (no effect); loads are order-free; loads and unloads only in OFF; unload returns to OFF and equals a fresh instance.
11. Fixed validation precedence (BAD_ARG, BAD_STATE, SIZE, WORD, DIGEST) with new stable status codes; API-06 matrix test `memcmp`s the whole instance before and after every refusal.

## 6. Risks, reversal, blockers
Top risks: (1) byte order and strict top-bit rule are [BG]-based and only checkable on a real file at D11; (2) stub labelling and use discipline; (3) cartridge digest meaning and shared status table depend on the cartridge-format decision.
Would reverse: real dumps with stray bits (add opt-in mask flag); a needed joint validation across several images (move to a bundle call); an owner demand for plug-and-play commercial play (revisit option 2 as a separate, labelled, tested HLE, which is currently Out of Scope).
[UNK] blocking planning: none. Needed alongside, not blocking: the instance-memory model (caller-provided versus init-time storage) and lifecycle state names from the lifecycle decision, which this note assumes (an OFF state, `reset` as power-on).
