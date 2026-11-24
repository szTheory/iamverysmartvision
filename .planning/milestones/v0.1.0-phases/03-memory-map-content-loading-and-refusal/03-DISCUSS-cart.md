# Phase 3 discuss, decision point D: cartridge import format (MEM-05, open question 4)

Labels: [OBS] fetched or run this session, [INF] inference, [REC] recommendation, [UNK] unresolved, [BG] background knowledge, not re-checked.
Inputs read: AGENTS.md, AUTONOMY.md, PROJECT.md Decision Register, docs/RIGHTS.md, docs/fixture-manifest.md, the D3 profile, ivs_status.h, cp1610.h, the Phase 2 fixture/test/gate patterns (IvsVectorTable.cmake, tests/CMakeLists.txt, cpu_fuzz.c, binary-ext.txt, manifests.cmake).
No emulator source was opened. One tier-4 community *document* was read (below).

## 1. Facts gathered

- [OBS, tier 4] `doc/rom_fmt/id_tag.txt` in the GPL FujiNet jzintv fork (spec "Intellivision ROM Metadata Tag Format" v0.13, 2019) describes the community `.ROM` file. First byte 0xA8, second byte segment count, third byte its complement. Per segment a low-page byte and a high-page-minus-one byte (256-word pages, 512 bytes each), then a CRC-16. Then a fixed 50-byte "enable tables" block. Then optional linked-list metadata tags that "tools may ignore". The document carries no licence notice. The `rom_fmt` listing also holds an Intellicart manual PDF; I did not open it. The enable-table semantics are not in what I read. [UNK] their meaning and whether the "16-bit word" storage is big-endian (BG says big-endian, unchecked).
- [INF] `.ROM` is Intellicart-hardware-shaped (enable tables, tags, CRC-16, 256-word granularity, no version field, no total length, ignorable tags). Implementing it needs the enable-table semantics, which come from GPL-hosted docs. Facts of a format may be implemented independently, but the expression may not be copied, and the owner's no-source rule bites.
- [OBS] RIGHTS.md and the hygiene step: `rom`, `bin`, `cfg` are binary-by-name and may be tracked only as committed outputs of an effective-yes manifest. Other extensions are classified by content (a NUL byte fails). The `ids`, `txt`, `json`, `md`, `sh`, `cmake` extensions are text by name.
- [OBS] Phase 2 precedent: tests data are text (JSON) under `data/`, turned into a C table at build time by a `cmake -P` generator (`cmake/IvsVectorTable.cmake`, build tree only). Inventories live in `data/expected/*.ids` and `data/expected/tests/<suite>.ids`. Fuzzing is a seeded xorshift ctest suite (`cpu_fuzz.c`). Named-fault controls live in `data/expected/faults/`.
- [OBS] The profile already says `cartridge.slots` = one slot, ROM-only 10-bit decles, and `cartridge.address_windows` = "from the per-content descriptor, not the profile". [INF] That second field needs to change, see 4.5.
- [OBS] `cmake -E sha256sum` and `shasum -a 256` agree on a hand-built 34-byte buffer here, so an independent digest oracle exists at zero dependency cost.

## 2. Options (all lenses)

| Option | Pros | Cons / footguns |
|---|---|---|
| A. Project-defined container (versioned header, segment table, SHA-256 trailer), parser in core, importers in tools | Specified wholly in project words; one canonical encoding, so identical content gives identical bytes and digest; refuse-unknown policy is expressible; fuzzable pure function; importers are a later 150-line tool each; no rights question for the format | New format nobody else emits; importer work is deferred, so real-world ROMs need that tool before anyone can run them |
| B. Community `.rom` as the core format | Interop on day one (large existing library of homebrew and dumps) | Format rights/semantics unchecked, derived from GPL-hosted docs; hardware-shaped enable tables; ignorable tags conflict with refuse-unknown; weak CRC-16; no version; core inherits every quirk and a parser far larger than the phase needs; this blocks MEM-05's "project's own words" |
| C. `.bin` + `.cfg` pair | Widely used | Two inputs make one cartridge (breaks "one typed input"); the `.cfg` is a text grammar (large parse surface, ambiguity); flat `.bin` word packing is not self-describing; same unchecked-rights/semantics problem |
| D. Raw binary + profile-side metadata | Trivial parser | Content is not self-describing; load addresses live outside the file; no identity, no integrity, no version; couples fixtures to profile edits; every host re-invents the metadata |
| E. Core takes only typed segments (no container), container parsed in tools | Smallest core | Malformed-input refusal (MEM-04 SC3) moves out of the core's refusal matrix; each host re-implements validation and drifts |

Lessons from other ecosystems (reasoning from published designs only): PNG's signature (high-bit byte plus CR LF SUB LF) detects text-mode mangling [BG]; ELF/WAV-style size fields catch truncation; formats that "ignore unknown chunks" accumulate parser disagreement (differential-parsing bugs), which is the concern for a hash-bound identity; checksum-protected formats starve naive fuzzers, so the harness must re-seal after mutation (standard libFuzzer fix-up pattern); a loader that validates while committing leaves half-loaded state, which is exactly what the refusal matrix forbids.

## 3. Recommendation (A, with a two-layer API)

Name: **IVSC** ("IVS cartridge image"), version 1. Extension `.ivsc`. No third-party trademark in the name. The core never sees a file name.

### 3.1 Byte layout (little-endian everywhere; every multi-byte field is assembled byte by byte, never by struct cast)

| Offset | Size | Field | Rule |
|---|---|---|---|
| 0 | 8 | magic | `89 49 56 53 43 0D 0A 1A` (0x89, "IVSC", CR LF SUB) |
| 8 | 2 | format_version | must be 1, else VERSION |
| 10 | 2 | header_flags | must be 0 |
| 12 | 4 | required_features | bit mask; 0 in v1 files. Bit names are reserved (0 banked mapping, 1 cartridge RAM, 2 expansion peripheral, 3 voice) with no semantics in v1 |
| 16 | 4 | total_bytes | exact size of the whole image including trailer |
| 20 | 2 | segment_count | 1..16 |
| 22 | 2 | reserved | must be 0 |
| 24 | 8*n | segment table | entries in this order: `load_address` u16, `length_words` u16 (1..65535), `width_bits` u8 (must be 10), `attr` u8 (0 = read-only ROM), `reserved` u16 (0) |
| 24+8n | 2*W | data | segments' words concatenated in table order, no padding, W = sum of lengths. Each word u16, value 0..0x03FF |
| total-32 | 32 | digest | SHA-256 (FIPS 180-4) of bytes 0..total-33 |

- Offsets are implied, never stored: no data-offset field exists to forge or overlap.
- `total_bytes` must equal `24 + 8n + 2W + 32` and the caller's length; MAX_SEGMENTS 16; MAX_BYTES = 24 + 16*8 + 2*65536 + 32 = 131256.
- Words: 10-bit values stored in 16-bit units (not bit-packed: tiny files, trivial importer, no unpack bugs). A value above 0x3FF is refused, not masked, so the loader never has to decide what the upper 6 bits of a ROM read return ([UNK], Phase 3 research flag). That decision stays with the memory map.
- No metadata (title, year, notes) and no profile hint in v1. Titles are a host/library concern; an unverifiable "intended profile" field would invite unbound claims (Core Value). A later version may add them as a counted, refused-if-unknown section.
- Worked example, P01 (66 bytes, one segment, one word 0x3FF at $5000): `89 49 56 53 43 0D 0A 1A | 01 00 | 00 00 | 00 00 00 00 | 42 00 00 00 | 01 00 | 00 00 | 00 50 01 00 0A 00 00 00 | FF 03 |` then the 32-byte digest `8408a46fa203f3e7b60fc9e1fd8518e384a7ccdbb3b0557de72a7e880c5c8331` [OBS: computed with `cmake -E sha256sum` and `shasum -a 256` on the 34 body bytes].

### 3.2 Parse rules and refusal classes

Parsing is a pure function `ivsc_parse(const uint8_t*, size_t, ivsc_view*, ivsc_report*)`. It does no allocation and no copy (view segments point into the caller buffer), has at most 16 loop iterations, and does all size arithmetic in `uint64_t` with a bound check before each use. Checks run in this fixed order and the first failure wins:
1. length: len < 66 TRUNCATED; len > MAX_BYTES OVERSIZE (no byte is read).
2. magic MAGIC; version VERSION.
3. header_flags, reserved nonzero MALFORMED; total_bytes > MAX_BYTES OVERSIZE; total_bytes > len TRUNCATED; total_bytes < len MALFORMED (trailing bytes).
4. segment_count 0 or >16 MALFORMED; total_bytes disagrees with the table-implied size MALFORMED.
5. per segment: length 0 MALFORMED; segment reserved nonzero MALFORMED; width != 10 or attr != 0 UNSUPPORTED; load_address + length > 0x10000 RANGE (overflow-safe, "wraps the address space").
6. ordering: start < previous start and no intersection ORDER; any intersection OVERLAP; adjacency (start == previous end) is allowed. v1 requires ascending order, which makes the encoding canonical.
7. digest: mismatch DIGEST.
8. data: first word above 0x03FF WORD_RANGE (report carries segment and word index).
The digest check sits before the data check so a flipped data bit is a DIGEST error. The checks before the digest are structural and cheap and do not depend on data. A fuzz or test build reseals after mutation (4.4).

API classes (what the host sees): `IVS_E_CART_MALFORMED` (TRUNCATED, MAGIC, VERSION, MALFORMED, WORD_RANGE, DIGEST as `reason` values), `IVS_E_CART_OVERSIZE`, `IVS_E_CART_OVERLAP` (OVERLAP, ORDER), `IVS_E_CART_RANGE`, `IVS_E_CART_UNSUPPORTED`. These 5 class codes extend `ivs_status.h` (negative values; exact numbers belong to the API discuss point). A caller-filled `ivs_load_report` gives `reason`, `segment_index`, `byte_offset`, and `unsupported_features`, so a fixture asserts class plus reason plus index, and the host gets a diagnostic without a log. Classes stay few so API-06 can document and test each state effect.

### 3.3 Where it lives and the two layers

- `core/src/content/ivsc.c` and `.h` (pure parse, validate, canonical encode used by tests). Added to `IVS_CORE_SOURCES`, so the existing purity symbol check covers it. SHA-256 comes from the core digest unit that MEM-03 needs anyway ([INF] decided in the digest/typed-image discuss point; if it is not in core, this format needs it there too).
- Layer 1 `ivs_cart_parse` (container to view, profile-independent). Layer 2 `ivs_machine_load_cart(machine, view)` (profile-dependent). A host with another source of segments (a future importer in-process, a test) builds a view directly and gets the same layer-2 refusals. No file, stdio or format-name concept enters the core.
- Importers (`.rom`, `.bin`+`.cfg`) are `tools/` programs or host code, produce IVSC bytes, and each needs its own rights review before being written. [REC] Not in Phase 3; Phase 4 or a host phase. Same content from any importer then yields the same canonical bytes and digest (sorted, one attr, no metadata), except for the split/adjacency choice, which the importer spec must fix (merge adjacent segments).

### 3.4 Layer 2: segment to memory-range mapping

- The memory-map descriptors (MEM-01) gain a device class `cart-rom` with window ranges. IVSC carries no window authority: a segment is accepted only if `[load_address, load_address+length)` lies wholly inside one single `cart-rom` window. Otherwise RANGE: this covers outside any window, straddling a window end, spanning two windows across a gap, and touching EXEC, GROM, RAM, STIC or PSG ranges. Intra-image overlap was already refused by the parser.
- Features: `required_features & ~profile.cartridge_features_supported != 0` is UNSUPPORTED (the report carries the missing bits). New profile field `cartridge.features_supported` = 0 for the D3 profile (one slot, ROM only); the profile fields list and its test inventory grow by one.
- Candidate windows [UNK, tier 4]: the fixtures assume $5000-$5FFF-style cartridge addresses ($5000-$6FFF, $D000-$DFFF, $F000-$FFFF are widely cited community ranges, unverified). The memory-map discuss point owns the real windows (C21 and C20 neighbours). Fixture addresses therefore must be asserted against the profile by one test (X01 below), so a window change fails loudly instead of silently.
- Words not covered by a segment inside a window follow the profile's undriven-read policy; a window need not be filled.
- Commit is atomic: validate every segment against the map first, then copy into pre-sized instance storage (no fallible step after the first byte is written). A second load while a cartridge is present is BAD_STATE (unload first); load into an instance in a state the API discuss point forbids is BAD_STATE. Every refusal leaves the instance byte-identical, proven by a state digest before and after, for every refusal fixture.
- Missing EXEC/GROM is not a cartridge-load refusal; it is raised at reset or first run (interaction with D5, to be confirmed by that discuss point).
- Reported length/digest for MEM-03: length = total_bytes, digest = the trailer (equal to SHA-256 of the image, recomputed by the core and compared, so reporting is not trusted from the file).

### 3.5 Fixtures: text only, no committed binary

- Fixture form: JSON under `data/cart/` (the Phase 2 pattern), one file per group, each fixture `{id, cite, base?, patch?, reseal?, bytes?, expect:{class, reason, segment}}`. Positive fixtures give full literal bytes as hex text, with the digest typed in (computed with `cmake -E sha256sum` or `shasum`, a different implementation from the core's, so the first ctest run is the independent check). Malformed fixtures derive from a positive base by `patch` (offset/hex pairs, truncate, append) with `reseal: true|false`. Each negative then differs from a valid file in exactly one stated place. `reseal` uses the core SHA-256 test helper (core SHA-256 itself is tested against FIPS 180-4 vectors in MEM-03; that closes the loop).
- `cmake/IvsCartTable.cmake` (copy of the vector generator's helpers, no new dependency) emits a C table in the build tree only. No `.ivsc`, `.rom` or `.bin` file is ever created inside the repository, so the hygiene check needs no new exception and nothing rights-bearing can ride in. Large boundary images (max size) are built in the test by code from a recipe, since fixtures stay tiny.
- One manifest `fixtures/FX-ivsc-vectors/manifest.json`: kind `original-source`, `committed: true`, outputs = every `data/cart/*.json` with size and sha256, licence MIT, rights review author-attested, `redistributable: yes` (the licence-evidence url/digest point at the repo LICENSE). Add the id to `data/expected/fixtures.ids`. [UNK] the manifest checker looks like it accepts outputs outside the fixture directory (manifests.cmake line 149 only matches the fixtures prefix); confirm in plan.
- Dev helper `tools/cart/seal.sh` (sh, printf, `cmake -E sha256sum`): prints the digest trailer for a hex body. Not part of the gate.
- Spec doc `docs/formats/ivsc-v1.md` in the project's own words (3.1 and 3.2 are its core), recording that `.rom` was read only as tier-4 document context and is the target of a later importer, never an input of this spec. Register the id_tag document as a tier-4 source (`data/sources.json`, rights unknown) and add the format decision, including the `.rom` alternative, to the Decision Register as a delegated decision.

## 4. Fixture inventory (ids; expect = class/reason)

Suites and gate: `cart-parse` (X, P, B, N ids), `cart-load` (L ids), `cart-fuzz`, registered in `data/expected/tests/<suite>.ids`, plus `data/expected/cart-fixtures.ids`. The generator fails closed on an id in the JSON that is not listed and the reverse.

### 4.1 Positive (parse OK, load OK on the D3 profile unless stated)
- CART-P01-min: 1 segment, 1 word 0x3FF at $5000 (the 66-byte example).
- CART-P02-two-windows: 2 segments in two windows, ascending, different lengths.
- CART-P03-word-patterns: words 0x000, 0x3FF, 0x155, 0x2AA read back exactly.
- CART-P04-adjacent: segment 2 starts at segment 1 end (accepted).
- CART-P05-window-full: one segment exactly fills one whole window.
- CART-P06-gap-undriven: segment fills part of a window; the rest follows the undriven-read policy.

### 4.2 Boundary
- CART-B01-segs-16: 16 one-word segments accepted.
- CART-B02-segs-17: count 17, MALFORMED/SEG_COUNT (table of 17 supplied, so only the count rule fires).
- CART-B03-window-last-word: ends on the last word of a window, OK.
- CART-B04-window-plus-one: one word past the window end, RANGE.
- CART-B05-window-minus-one: starts one word below the window base, RANGE.
- CART-B06-addr-wrap: $FFFF length 2, RANGE/ADDR_WRAP at parse.
- CART-B07-max-file-ok (programmatic): two segments totalling 65536 words, 131144 bytes, parses OK (the load refuses it RANGE, which is the point of the boundary).
- CART-B08-len-over-max (programmatic): caller length MAX_BYTES+1, OVERSIZE, buffer never read (a guard page or poisoned buffer proves it).
- CART-B09-total-over-max: header total_bytes MAX_BYTES+1 with a short buffer, OVERSIZE.
- CART-B10-len-65535: segment length 65535 at address 0 plus 1 word at $FFFF parses OK (u16 length edge).
- CART-B11-word-3ff-vs-400: 0x03FF accepted (P03), 0x0400 refused (N26).

### 4.3 Malformed and refused
- Truncation and header: N01 empty (TRUNCATED), N02 length 65 (TRUNCATED), N03 one bit flipped in magic (MAGIC), N04 CR LF collapsed to LF in the magic (MAGIC), N05 version 0, N06 version 2 (VERSION), N07 header_flags nonzero, N08 header reserved nonzero (MALFORMED), N09 total_bytes 1 below length (trailing byte, MALFORMED), N10 total_bytes 1 above length (TRUNCATED), N11 table implies a different size than total_bytes with length agreeing (MALFORMED), N12 segment_count 0 (MALFORMED).
- Segment fields: N13 length 0, N14 width 8, N15 width 16, N16 width 0, N17 attr 1, N18 attr 0x80 (UNSUPPORTED for width and attr), N19 segment reserved nonzero (MALFORMED).
- Ordering: N20 overlap by one word, N21 identical duplicate segment, N22 inner segment inside the first (OVERLAP), N23 descending disjoint segments (ORDER).
- Data and digest: N24 word 0x0400 first word (WORD_RANGE, segment 0 word 0), N25 word 0xFFFF last word of last segment (proves a full scan, index asserted), N26 word 0x8000 with a valid reseal, N27 one flipped data bit with an unchanged trailer (DIGEST), N28 trailer flipped bit, N29 trailer all zero, N30 trailer cut to 31 bytes with total_bytes and length both reduced by one (MALFORMED/SIZE_MISMATCH at rule 4, since the table implies one more byte).
- Precedence: N31 bad digest plus a 0x0400 word (DIGEST wins), N32 bad magic on a file also 1 byte short of its total_bytes (MAGIC wins, rule 2 before rule 3), N33 version 2 plus bad flags (VERSION wins).

### 4.4 Load-level (need the profile; the state digest is compared before and after every one of these and every N fixture)
- CART-L01-below-window ($0100 start, in STIC/RAM space), L02-over-exec ($1000, EXEC range), L03-straddle-gap (starts in window A, ends in window B), L04-over-ram (the RAM range), all RANGE.
- CART-L05-feature-banked (required_features bit 0), L06-feature-unknown (bit 31): UNSUPPORTED with the report mask.
- CART-L07-second-load: while loaded, BAD_STATE, first cartridge unchanged.
- CART-L08-null-and-len: NULL machine, NULL buffer with len > 0, BAD_ARG; machine unchanged.
- CART-L09-unload-restores: after load then unload the instance state digest equals the never-loaded one.
- CART-L10-two-instances: a cartridge loaded into instance A is invisible to instance B (API-01 isolation).
- CART-X01-windows-match: every address window assumed by P/B/L fixtures is a window of the profile (map drift detector).
- CART-X02-canonical-roundtrip: for each accepted fixture, encode(view) equals the input bytes.

### 4.5 Fuzz and named faults
- `cart-fuzz` (seeded xorshift32, Phase 2 style, bounded run): mutate positive bases (bit flip, truncate, grow, field edit), half resealed. Invariants: defined status, no out-of-bounds (ASan/UBSan), determinism, a refusal leaves the view/state untouched, acceptance implies canonical round trip, every refusal reason reached at least once.
- A libFuzzer target `tests/fuzz/fuzz_ivsc.c` (re-seals the trailer in the harness) ships as an optional non-gate recipe; Apple clang cannot link libFuzzer, so it runs with Homebrew LLVM or a scheduled Linux job. [REC] Not in the PR gate.
- Named faults (`data/expected/faults/`, each must turn a named fixture red): FAULT_CART_SKIP_DIGEST (N27-N29), FAULT_CART_END_OFF_BY_ONE (B04, B06), FAULT_CART_ACCEPT_WORD_400 (N24), FAULT_CART_COUNT_17 (B02), FAULT_CART_COMMIT_BEFORE_VALIDATE (every refusal, state digest), FAULT_CART_TRUST_TOTAL_BYTES (N09-N11). Negative controls under `tools/check/controls/` for the new gate checks (inventory missing/extra id, manifest output digest drift, stray `.ivsc`/`.rom` tracked file is already covered by `files`).
- CI cost [INF]: parsing 66-byte buffers plus a 20000-iteration fuzz is milliseconds; no new CI job, no cache.

## 5. Decisions as numbered items (for CONTEXT)

1. Core cartridge format is IVSC v1 (project-defined, specified in `docs/formats/ivsc-v1.md`), not `.rom`, `.bin`+`.cfg` or raw binary; ext `.ivsc`; name carries no third-party mark.
2. Layout per 3.1: 8-byte magic, 24-byte header, 8-byte segment entries (u16 address, u16 length in words, u8 width = 10, u8 attr = 0, u16 reserved), little-endian, 10-bit words in 16-bit units (value above 0x3FF refused), trailing SHA-256 over all preceding bytes, `total_bytes` field.
3. Limits: 1..16 segments, 1..65535 words each, ascending, non-overlapping (adjacent allowed), whole image at most 131256 bytes; oversize is refused before reading.
4. Unknown-field policy: refuse everything undefined (version, flags, reserved, width, attr, required_features, no ignorable chunks, no metadata); evolution is by version bump.
5. Required features: u32 mask, 0 in v1; profile field `cartridge.features_supported` = 0 for the D3 profile; any unsupported bit is UNSUPPORTED with the mask reported.
6. Parser in core (`core/src/content/ivsc.c`, pure, zero-copy, bounded, uint64 arithmetic, fixed check order); importers for other formats live outside the core, are a later phase, and need a separate rights review.
7. Segments load only into `cart-rom` windows from the memory-map descriptors (one segment wholly inside one window); load is validate-all then atomic commit; second load is BAD_STATE; every refusal is state-unchanged, proven by a state digest; refusal classes `IVS_E_CART_{MALFORMED,OVERSIZE,OVERLAP,RANGE,UNSUPPORTED}` plus a report with reason, segment index, byte offset, missing features.
8. Fixtures are text JSON (`data/cart/`), hex bytes for positives, patch-from-base for negatives, built to C tables at build time in the build tree; one manifest `FX-ivsc-vectors`; max-size images are built by code; no binary file is committed; inventories in `data/expected/`; fuzz suite plus six named faults.

## 6. Adversarial pass

- "A new format means nobody can load a real cartridge." True for Phase 3's exit; the milestone's host is the test runner and diagnostics are project-authored, and `.rom` import is a small tool that does not touch the core. Reversal trigger: the owner wants third-party homebrew runnable in this milestone, then promote the `.rom` importer (after its rights review) to Phase 3/4, with the core unchanged.
- "Mandatory SHA-256 makes hand-authoring painful." IVSC is an interchange format, not an authoring format; `seal.sh` and every importer seal it. Reversal: if hand authoring dominates, add a flag-gated "no digest" variant in v2, not v1.
- "Parser in core grows the attack surface." It is one pure function over bytes, tested, fuzzed, under the purity check. The alternative (option E) leaves each host to re-validate and drift, which is the worse failure. Mitigation retained: layer 2 accepts views, so hosts may skip the container.
- "Refusing words above 0x3FF will reject real dumps." Likely only badly packed ones; the importer owns masking policy and says so. The core still never has to answer the unknown upper-bit read question.
- "Width, attr and features are YAGNI." They cost 4 bytes per segment and 4 in the header, and make unsupported content a typed refusal instead of silent mis-loading. If you want less, drop attr and rely on the version bump; the refusal behavior is the same.
- "Fixtures assume window addresses that are [UNK]." Mitigated by X01; if the map discuss picks different windows, only the literal addresses change and the digest typed into the positive fixtures must be recomputed with `seal.sh`.
- "Segment adjacency allows two encodings of one content." Accepted for v1 (identity is the byte image digest); the importer spec fixes merging so independent importers agree.

## 7. Top risks

1. The memory-map decision changes the cart windows or the MEM-01 device-class vocabulary; fixtures and layer 2 then need edits (bounded by X01 and the generator).
2. SHA-256 not landing in the core early enough, or sharing one implementation between seal and verify with no independent oracle for the trailer; mitigated by typed digests computed with cmake/shasum and the FIPS vectors.
3. Rights: the later `.rom` importer reading GPL-hosted docs; keep this spec independent and gate importer work on its own review. Also the manifest checker's treatment of outputs outside `fixtures/<id>/` (UNK).

## 8. What would reverse it

An owner instruction to run existing third-party cartridges in this milestone; a documented permissive licence plus a complete public `.rom` spec that makes it a safe core format; evidence that cartridge content needs semantics IVSC v1 cannot express before the first banked-ROM profile (then a v2 section, not a different format).

## 9. Blocking [UNK]

None blocks planning. Non-blocking: real cart window ranges (memory-map discuss, C21/C20 neighbours); whether SHA-256 lives in core (digest/typed-image discuss point); `.ROM` enable-table semantics and word endianness (importer phase); whether the manifest checker accepts outputs outside the fixture dir (check in plan 1).
