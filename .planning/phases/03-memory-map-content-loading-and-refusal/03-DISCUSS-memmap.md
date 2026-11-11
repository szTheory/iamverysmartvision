# Phase 3 discuss: Decision Point B, profile-driven memory map and width masking (MEM-01, MEM-02)

**Date:** 2026-11-10. **Scope:** descriptor model, decode-table verification, the 2609 memory-map facts, ROM upper bits, GRAM/STIC decode (C21), store truncation, bus-port consumption, stub devices, boundary tests and faults.
**Labels:** [OBS] I fetched or ran it this session; [INF] inference; [REC] recommendation; [UNK] unknown. Tiers follow `data/tiers.json`.
**Rights:** the GI 1980 catalog was downloaded to the session scratch directory only (not `refs/`, not committed) and text-extracted with `pdftotext`. No emulator source file was opened. Wiki and Blue Sky pages are tier-4/tier-2 leads and are labelled as such. Page cites below use the catalog's *printed* page numbers; the profile already maps printed 5-10 to pdf 191, so I expect pdf = printed + 181 [INF, verify when cited].

## 1. Facts gathered

### 1.1 Tier-1 primary (GI Microelectronics Data Catalog 1980, SRC-gi-1980-catalog) [OBS]

| Fact | Where | Consequence |
|---|---|---|
| RO-3-9502 (EXEC part) is 2048 x 10. On a read it "will output the 10 bits of addressed data and also drive a logic zero on the top six bits of the bus". | printed 5-13 | ROM bits 15..10 read 0 for this part. Resolves the "upper bits of ROM" question for GI mask ROMs at tier 1. |
| RO-3-9504 (cartridge/EXEC-half part) is 2048 x 10, "16 bit tri-state bus with higher 6 bits driven to zero during read operations". | 5-21 | Same, for the GI cartridge part. A non-GI cartridge ROM is not covered [UNK]. |
| RO-3-9502 and RO-3-9504 each hold a "programmable memory map location for its own 2K page"; a 2K page sits on a 2K boundary. | 5-13, 5-21 | EXEC placement is a mask option of each chip. That is the real root of C20 (which half sits where). |
| RO-3-9502 answers the IAB access itself with a 16-bit start vector, then "any further IAB codes output the second interrupt vector". | 5-13 | The reset and interrupt vectors are chip-supplied, not ROM words. Matches Phase 2: the bus layer answers IAB from profile fields `reset.r7` and `interrupt.iab_value`. The map must not treat them as addressable ROM. |
| RO-3-9503 (GROM) is 2048 x 8, "8 bit tri-state bus with higher 8 bits driven to zero during read operations". After an SR1 pulse it sits at 12288-14335 (`$3000-$37FF`) and "the external memory is addressed at 14336 to 16383" (`$3800-$3FFF`). After BUSAK it sits at 0-2047 with external memory at 2048-4095. | 5-16 | GROM reads `$00xx` (tier 1). The CPU-time view puts GROM at `$3000-$37FF` and gives a 2K external window at `$3800-$3FFF`, so GRAM is decoded across the whole 2K window [INF]. The BUSAK view is the STIC's graphics-bus view, outside Phase 3. |
| RA-3-9600 (system RAM) is 352 x 16, "contiguous words from address 512-863" (`$0200-$035F`); graphics use the low 14 bits and "the two most significant bits [are] available for user storage". | 5-18 | System RAM stores and reads all 16 bits. Tier-1 backing for `$0200-$035F`. |
| The STIC section (5-10 to 5-12) gives no CPU-visible register addresses. | 5-10..5-12 | STIC register range, scratchpad range and PSG range have no tier-1 address source in this catalog [OBS]. They stay tier 4. |

### 1.2 Tier-2 (Blue Sky Rangers 2609 page, fetched) [OBS]
Scratchpad is one 256 x 8 chip (147 bytes left for games). GRAM is 64 8x8 images in two 256-byte RAMs. GROM is 2K with 213 images. System RAM is 240 BACKTAB words plus 112 stack words. No address ranges on that page.

### 1.3 Tier-4 (intellivision.us wiki "Memory Map", oldid 14847, last edited 2010-12-04, fetched) [OBS]
`$0000-$003F` STIC (VBlank period 1); `$0040-$007F` read-only STIC-responsive alias; `$0100-$01EF` scratchpad; `$01F0-$01FF` PSG; `$0200-$035F` system RAM; `$1000-$1FFF` EXEC; `$3000-$37FF` GROM (read-only); `$3800-$39FF` GRAM and `$3A00-$3FFF` "more GRAM blocks, read/write"; write-only STIC aliases `$4000-$403F`, `$8000-$803F`, `$C000-$C03F`; write-only GRAM aliases `$7800-$7FFF`, `$B800-$BFFF`, `$F800-$FFFF`; reads from undriven space return `$FFFF` (pull-ups). Cartridge-usable lists are given with ECS/IntelliVoice conditions. `$0360-$03FF` garbage and `$0400-$04FF` EXEC are Intellivision II only. The wiki and the earlier research file disagree on whether `$3A00-$3FFF` is writable (research file: not writable) [OBS, feeds C21].
The wiki and the research file are one community lineage, so agreement between them is not an independent cross-check [INF].

### 1.4 Unknown after this pass [UNK]
What bits 15..8 of an 8-bit RAM (scratchpad), the PSG and the STIC registers read as on a 2609; the exact STIC decode extent; whether `$3A00-$3FFF` and `$0040-$007F` accept writes; read results of a blocked window on a 2609 (the one measurement is a STIC1A board, C5). None blocks planning because each becomes a tagged default plus a discrepancy entry.

## 2. Options

### 2.1 Where descriptors live
| Option | Pros | Cons |
|---|---|---|
| A. Fields in `data/profiles/*.json`, one record per region and per device kind, then a `cmake -P` generator writes a const C table into the build tree | Reuses the existing record schema: each region gets its own `tag`, `cite`, `conflict` (C21), `resolving_evidence`, `revisit_trigger`, so an unknown region is forced to say so. Same pattern as `cmake/IvsVectorTable.cmake` (fail closed, build tree only, no C JSON parser). Unknown-profile refusal becomes a lookup in a generated registry. | Profile grows by about 25 fields; `data/expected/profile-fields.ids` and the schema inventory change. Region value is an OBJECT, so cmake `string(JSON)` validation must be written (vector generator already does this). |
| B. Separate `data/memmap/<profile>.json` with its own kind schema | Keeps the profile file small. | A new kind (schema, hook, inventory, controls) and a second place that carries provenance. Provenance is the point of the profile record; splitting it loses the free `unknown-default-chosen` enforcement. |
| C. Hand-written C table | No generator. | Two sources of truth (C and profile). "Generated from descriptors" in MEM-01 would be false. Rejected. |
| D. Runtime JSON/blob parsed by the core | Profiles as host data. | Adds a parser to a core that must stay tiny and pure; unknown-profile handling becomes a parser problem. Rejected for now. |

**[REC] A.** Anti-pattern avoided: hand-copying a decode table into code and "verifying" it against itself.

### 2.2 How the descriptor table is consumed at run time
| Option | Cost per access | State | Verdict |
|---|---|---|---|
| Binary search over about 25 sorted regions | 4 to 5 compares | none | Fine but slower, and every access pays. |
| Static const page table: `rd_region[4096]` and `wr_region[4096]` of `u8`, index `addr >> 4` (16-word granule, set by the PSG's 16-word span) | one load, then a `switch` on device kind | none (const, generated, in .rodata) | **[REC]** |
| Per-instance copy of the table | same | 8 KiB mutable state to hash and snapshot | Rejected: constants do not belong in state. |

The only mutable decode state is the cartridge: the static read table marks the cart window `CART`, and a per-instance `cart_page[window_words >> 8]` (one `u8` segment index per 256-word page, 0 = empty) resolves it. Unload clears it. Read and write views are separate tables because write-only aliases (STIC at `$4000`, GRAM at `$7800`) coexist with a ROM that reads there (wiki: "ROM is fine") [INF from tier 4].

### 2.3 What a stub device is
| Option | Verdict |
|---|---|
| Absent device (region undriven) | Hides that a decode exists; STIC and PSG writes would be indistinguishable from stray writes. Rejected. |
| Stub that stores and returns values | Invents register behavior with no source. Rejected. |
| **Stub-open [REC]:** decoded and named, reads return the undriven fill, writes are dropped, marked `impl=stub` in the dump | Makes the decode testable and the claim honest. Phase 5 (PSG, controllers) and Phase 6 (STIC) replace the device kind; the region rows do not change. |

## 3. Recommendation (decision-ready)

**D-B1 Descriptors are profile fields.** New profile fields: `memmap.undriven_read` (`FFFF`, C5); one `memmap.device.<kind>` per device kind (`rom10`, `grom8`, `ram8`, `ram16`, `stub`) holding `{bits, rmask, fill, store}`; one `memmap.region.<name>` per region holding `{view R|W|RW, lo, hi, device, size_words (backing/mirror, power of two), gate, impl}`; and `memmap.cart_window` (`4000-FFFF`) plus `memmap.cart_align_words` (256). Hex values are 4-digit uppercase hex strings, as in the vectors. Each record carries tag, cite and conflict like every other profile field. Source of truth for the region list is the profile; nothing else in the repo may restate a region.

**D-B2 Generator.** `cmake/IvsMemmapTable.cmake` (`cmake -P`, fail closed, build tree only) reads every `data/profiles/*.json` and writes `ivs_profiles.inc` with: the const region array, the const `rd_region[4096]` and `wr_region[4096]`, per-profile sizes, and a profile registry keyed by id string. It stops the build on: `hi < lo`, a range not aligned to 16 words, an overlap within one view, a non-power-of-two `size_words`, an unknown device kind or gate, a missing required key, an unknown key, an unresolved region reference, or a read-driving region inside the cart window. Optional `-DMUTATE=<region>` shifts that region's `lo` by one (named fault GEN_MAP_MUTATION).

**D-B3 One decode formula, no per-device special cases for width.** For a driven region: `read = (stored & rmask) | (fill & ~rmask)`; store keeps `value & rmask` (RAM) or is dropped (ROM, stub, write-only alias target with no storage). Undriven space: `rmask = 0`, `fill = undriven_read`. Values: `rom10` rmask `03FF` fill `0000` (tier 1, 5-13 and 5-21); `grom8` rmask `00FF` fill `0000` (tier 1, 5-16); `ram16` rmask `FFFF` (tier 1, 5-18); `ram8` rmask `00FF` fill `0000` (unknown-default-chosen, C60); `stub` rmask `0000` fill = undriven. Loaders **refuse** (not truncate) any image word above its device mask: truncation exists only for bus stores.

**D-B4 Region table for `intv-2609-ntsc-family-p1`.** R = read view, W = write view; `impl` is `real` or `stub` in Phase 3.
| Range | View | Device | Backing | impl | Tag |
|---|---|---|---|---|---|
| `0000-003F` | RW | STIC regs | 64 | stub | tier4 (C21) |
| `0040-007F` | R only | STIC regs alias | mirror 64 | stub | tier4 (C21) |
| `0100-01EF` | RW | scratchpad `ram8` | 240 | real | tier2 for size (BSR), tier4 for address |
| `01F0-01FF` | RW | PSG regs | 16 | stub | tier4 (C11) |
| `0200-035F` | RW | system RAM `ram16` | 352 | real | tier1 (5-18) |
| `1000-1FFF` | R only (writes dropped) | EXEC `rom10` | 4096 | real | tier4 for `$1000`, tier2 for 4K, C20 |
| `3000-37FF` | R only | GROM `grom8` | 2048 | real | tier1 (5-16) range, tier4 gating |
| `3800-3FFF` | RW | GRAM `ram8` | mirror 512 | real | tier1 window (5-16) [INF], tier4 RAM size, C21 |
| `4000-403F`, `8000-803F`, `C000-C03F` | W only | STIC alias | mirror 64 | stub | tier4 (C21) |
| `7800-7FFF`, `B800-BFFF`, `F800-FFFF` | W only | GRAM alias | mirror 512 | real | tier4 (C21) |
| `4000-FFFF` minus the above in the read view | R | cartridge `rom10`, per loaded segment | per segment | real | unknown-default-chosen |
| everything else, both views | | undriven | | none | tier4 (C5) |
A read of `$4000-$403F` or `$7800-$7FFF` with no cartridge segment there returns the undriven fill; a loaded segment there reads as ROM while writes still reach the alias device (wiki, C61). The `$4021`-style mode-select side effect is a STIC behavior and is Phase 6; Phase 3 only drops the write.
The cart window is `4000-FFFF` for this profile: ROM-only, single slot, 256-word segments. Wiki-listed holes (`$0500`, `$0D00`, `$2000`) are not opened; widening later is additive, narrowing would be a claim change [REC, reversal below].

**D-B5 Gates.** Region rows may name a gate (`stic-regs` for `$0000-$007F`, `gmem` for GROM and GRAM). The instance holds `gate_open[N]`, initialized all open, changed only by an internal setter that Phase 6 will drive. A closed gate makes the region read as `blocked_read`, which is `undriven_read` in this profile (C5: no value is claimed for a blocked read) and drops writes. Phase 3 tests close a gate through the internal setter. This exercises the "blocked-read" descriptor now without inventing window timing.

**D-B6 Bus port.** One machine-level function matches `ivs_bus_fn`: FETCH, READ and ADAR read through `rd_region`; WRITE and DW (interrupt-entry stack write) write through `wr_region`; IAB returns the profile vector for its reason flag; INTAK and the strobes return no data. Wait is 0 in Phase 3. Region rows get no wait field now; Phase 6 adds the STIC-imposed stall as a machine concern (the bus reply struct already carries `wait_cpu_cycles`). No heap, no clock, no function pointer per device: the dispatch is a `switch` on a small device-kind enum, so the Phase 2 purity symbol check stays clean.

**D-B7 Unloaded content.** An unloaded EXEC, GROM or cartridge region reads as undriven, not as zeros (opcode `0000` is HLT, so a zero fill would look like a program). Load refusal and run refusal for a missing system image belong to decision points C/D; the map only exposes `*_loaded` flags in instance state. A boot stub loaded under D5 is an ordinary typed EXEC image: the map has no stub special case.

**D-B8 The literal map and its independence rule.** File `data/memmap/<profile-id>.literal.txt`, one row per line, comments with `#`:
`<R|W> <lo>-<hi> <device> w=<bits|?> impl=<real|stub|none> [mirror=<words>] [gate=<name>] src=<SRC-id>@<locator> tag=<tag> [conf=C<n>]`
Rules: (1) authored from sources only; it may not mention profile field names; the tool that writes it must not read the profile. (2) Per view the rows must tile `0000-FFFF` exactly: gaps are written as explicit `undriven` rows (`src=none tag=unknown-default-chosen conf=C5`), so an omitted descriptor shows up as a diff. (3) `src` ids must exist in `data/sources.json`, the locator must be non-blank (printed page for GI, oldid for the wiki), `tag` must be a source-authority tag, a row tagged `tier5-emulator-hint` is refused, and a row with `unknown-default-chosen` or `tier4-community` must name `conf=` or a source locator. (4) `w=` and `impl=` are the claims under test; the cart window appears as one row `4000-FFFF cartridge w=10` with `impl=real`.
The check step dumps the runtime decode by probing **all 65,536 addresses in both views through the real bus read/write functions** (not by reading the tables), run-length merges them into the same column format, strips `src`, `tag`, `conf`, and requires byte equality with the literal projection. Probing the executed bus instead of the table is what keeps the generator and the check from sharing a misreading.
Honest label: MEM-01's claim tier is `source-expectation`. Most rows are tier 4 from one lineage, so a pass shows transcription consistency, not hardware truth. The claim ledger entry must say so and depend on the region fields.

**D-B9 Boundary and truncation tests (MEM-02), generated from the literal map.** `cmake` expands the literal rows into probe tuples: for each row and view, addresses `lo-1`, `lo`, `hi`, `hi+1`, and the mirror wrap points (`lo + size - 1`, `lo + size`). The expected value comes from a small rule table in the test keyed on the literal's device and width, not from descriptors: `w=10` read of a loaded `03FF` gives `03FF`, store `FFFF` is dropped; `w=8` RAM store `FFFF` reads `00FF`, store `1234` reads `0034`; `w=16` RAM store `FFFF` reads `FFFF`; stub and undriven read `FFFF` and drop writes; a write-only alias reads `FFFF`; a mirror store at `lo+size` reads back at `lo`. Plus direct tests of the refusal rule in D-B3 (image word `0400` for `rom10` refused) and of gates (D-B5). The ids are listed in `data/expected/tests/memmap-*.ids`.

**D-B10 Discrepancy and profile bookkeeping.** New entries (next free ids, orchestrator renumbers if sibling decision points also add): C60 upper bits of 8-bit RAM, PSG and STIC reads (default zero; tier-1 zero for the GI mask ROMs and GROM; status partial); C61 write-only aliases coexisting with cartridge ROM (tier 4, wiki only); C62 cartridge window narrower than the wiki's usable list. Edit C21 so its resolver is `measurement: D10` (see risk 3: a Phase 3 diagnostic can only pin defaults) and extend its sources with the GI 5-16 window and the `$3A00-$3FFF` write disagreement. Edit C51 to say Phase 3 resolved ROM upper bits to zero for GI parts (partial). Add the new profile field paths to `data/expected/profile-fields.ids`.

**D-B11 Gate integration and CI cost.** One new step `memmap` (cmake -P, pure string work, estimated well under a second): generator self-checks, literal grammar and tiling, and the dump comparison (the dump executable is built by the existing `c-build`). New inventories: `data/expected/gate-steps/memmap.ids`, `data/expected/tests/memmap-*.ids`, `data/expected/faults/memmap.ids`, `data/expected/controls/memmap.ids`. No new CI job, no new dependency, no new matrix leg.

## 4. Negative controls and named faults

Tooling controls under `tools/check/controls/` (each must fail for its intended token): `memmap-literal-hole` (a view that does not tile), `memmap-literal-overlap`, `memmap-literal-no-cite`, `memmap-literal-unknown-source`, `memmap-literal-tier5`, `memmap-literal-mismatch` (one row edited, expected token `memmap-dump-diff`), `memmap-gen-overlap`, `memmap-gen-misaligned`, `memmap-gen-nonpow2-mirror`, `memmap-gen-unknown-device`, `memmap-gen-unknown-key`, `memmap-gen-cart-overlap`, `memmap-profile-unknown-region-tag` (region with `unknown-default-chosen` but empty `revisit_trigger`).

Named-fault controls (same pattern as `tests/faults.cmake`: one `IVS_FAULT_<ID>` site in one core TU, a must-fail test and a must-pass test):
| Fault | Mutation | Must fail |
|---|---|---|
| GEN_MAP_MUTATION | generator shifts one region's `lo` by 1 | the literal dump comparison and the `lo` boundary probe |
| MEM_ROM_NO_MASK | `rom10` read skips `rmask` | ROM upper-bits probe |
| MEM_RAM8_NO_TRUNC | `ram8` store keeps 16 bits | scratchpad and GRAM truncation probe |
| MEM_RAM16_TRUNC | `ram16` store masks to 8 bits | system RAM full-width probe |
| MEM_HI_INCLUSIVE | region `hi` treated as exclusive | `hi` and `hi+1` probes |
| MEM_MIRROR_MASK | mirror mask off by one bit | mirror wrap probe |
| MEM_WO_READS_DEVICE | write-only alias returns device data | alias read probe |
| MEM_ROM_WRITABLE | ROM store lands | ROM write-drop probe |
| MEM_GATE_IGNORED | closed gate still returns data | gate probe |
| MEM_UNDRIVEN_ZERO | undriven read returns `0000` | gap probe |
| MEM_LOAD_TRUNCATES | loader masks an out-of-width word instead of refusing | D-B9 refusal probe |

## 5. Lenses, patterns, anti-patterns (condensed)

- **Memory-system architect.** Declarative address maps as data are the established pattern (the earlier research file records that MAME and jzIntv both express maps as data [OBS from `.planning/research/FEATURES.md`, not re-read]). Footgun: modelling a write-only alias as a mirror. It reads differently, so the separate read and write views are needed. Footgun: one `size` field doing three jobs (extent, backing size, mirror); here `hi-lo+1` is the extent and `size_words` is the backing, so a mirror is any region whose extent exceeds its backing.
- **Hardware/EE.** Tier 1 says ROM upper bits are *driven* to zero by the chip, so the formula's `fill` is a driven value, unlike the undriven pull-up `FFFF`. Keep those two policies separate fields; collapsing them would make a future 8-bit RAM measurement ambiguous.
- **QA.** Boundary probes derived from the literal rows, not the descriptors, break the shared-misreading loop between generator and check. The exhaustive 64K probe costs about 131k calls (milliseconds).
- **Security/rights.** The map and tests use project-authored stand-in bytes only. The literal map cites sources by id and locator and quotes none; it contains no ROM bytes. The gate's existing `files` and `binscan` steps cover it.
- **DevOps/CI.** One cmake-script step and one test executable. Measure the step with the existing `step-time` line before adding anything.
- **API/integrator.** Instance size is a compile-time constant per profile (fixed arrays: 4096 EXEC words, 2048 GROM bytes, 512 GRAM, 256 scratch, 352 system RAM, cart pool for `4000-FFFF` = 49,152 words), which fits a caller-provided buffer and the purity rule against allocation. An unknown profile id is a registry miss, so it is refused before any state exists.
- **Product/DX.** The dump is human-readable and diffable; a failing diff names the row. The `impl=stub` column gives the claim ledger a mechanical way to avoid claiming STIC or PSG behavior.

## 6. Adversarial pass against D-B1..D-B11

1. **"The literal map is a copy of the same wiki, so the check proves nothing."** True for most rows (one 2010 revision). It still catches generator, decode-expansion, mirror and width bugs, which are the failures Phase 3 can introduce. It is labelled `source-expectation` and the tier-1 rows (5-13, 5-16, 5-18, 5-21) are marked as such, so the claim says which rows are firm.
2. **"Read and write views double the work."** It is a second const table, produced by the same generator. The cost is lower than retrofitting write-only aliases later.
3. **"Open gates by default is a lie about the console."** It is a documented default: no STIC exists in Phase 3 to open or close a window. The blocked-read path is real and tested; Phase 6 replaces the gate driver. Risk: a Phase 4 guest passes on an always-open window and later fails. Mitigation: claims for Phase 3/4 never mention window timing, and `impl=stub` plus `gate=` appear in the dump.
4. **"Putting 25 fields in the profile will bloat it and slow review."** Per-region cite and conflict are the value; the alternative (option B) rebuilds that machinery.
5. **"ram8 upper bits zero is a guess."** Yes: C60, tagged `unknown-default-chosen`, with D10 read-back as the resolver. The one data point (TutorVision, different board) agrees, and GROM at tier 1 is zero. The cost of being wrong is one device-kind field plus one rule-table row.
6. **"A 256-word cart granule excludes real content."** Possible; the cartridge-format decision may use a different granule. The map requires only that `cart_align_words` divides the segment, so the profile field can drop to 16 without code change.
7. **"Per-instance `cart_page` plus static tables makes snapshots awkward."** The opposite: constants stay out of the snapshot, and the mutable decode state is under 200 bytes.

## 7. Top risks
1. **Evidence depth.** Nearly all addresses are tier 4 from a single community lineage; the PSG, STIC and alias rows could be wrong in ways no Phase 3 test can see. Mitigation: tags, C21/C60/C61/C62, `impl=stub`, claims bounded to `source-expectation`.
2. **Profile and generator growth.** Region objects inside profile records stretch the schema and the cmake JSON validation; a bug there is a build-time failure that looks like a data error. Mitigation: negative controls for every generator refusal, and the generator written in the style of `IvsVectorTable.cmake`.
3. **C21 cannot be closed by Phase 3.** Its recorded resolver (a Phase 3 decode diagnostic) can only pin the chosen default; the unknown is hardware behavior. Leaving it as written would let a green test read as resolution. Mitigation: D-B10 rewrites the resolver to D10 and states that the Phase 3 test checks consistency with the default only.

## 8. What would reverse this
- A tier-1 to tier-3 source (a 2609 schematic, an AY-3-8914 or STIC data sheet with addresses) or a D10 read-back that contradicts a region or fill: change that region or device field, re-run the literal map, no structural change.
- A requirement for host-supplied runtime profiles: revisit D-B1/D-B2 toward a runtime descriptor blob (option D).
- Generator cost or complexity in cmake becoming a recurring failure source: move generation to a small checked-in C tool, still reading the profile.
- Real cartridge content needing the wiki's lower holes or non-256 granules: widen `cart_window` and `cart_align_words`.

## 9. Blocking unknowns
None block planning. Items to settle at plan time rather than here: the exact printed-to-pdf page offsets for the four GI cites (verify against the page images before writing locators), and the EXEC and GROM image container layouts (decision points C/D; the map only needs 4096 decles of at most 10 bits and 2048 bytes).
