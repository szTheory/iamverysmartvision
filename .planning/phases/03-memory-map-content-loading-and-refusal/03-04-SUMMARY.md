---
phase: 03-memory-map-content-loading-and-refusal
plan: 04
subsystem: core
tags: [memmap, profile, generator, D-16, D-17, D-18, D-19, D-20, D-23, A-01, A-02, A-05, A-08, H2, H14]

requires:
  - phase: 03-memory-map-content-loading-and-refusal
    provides: "plan 03-03: core/include split, reviewed generated includes in c-purity (core-generated.ids), writable-data rule"
provides:
  - "Profile fields memmap.undriven_read, memmap.device.{rom10,grom8,ram8,ram16,stub}, 14 memmap.region.<name>, memmap.cart_span, cartridge.features_supported, system_images.{exec,grom}; system_images.exec_grom rewritten to D-24; cartridge.address_windows removed"
  - "Discrepancies C60, C61, C62; C21 resolver retargeted to D10; C51 partial with ROM upper-bit evidence"
  - "cmake/IvsMemmapTable.cmake: fail-closed profile-to-C generator (-DROOT -DOUT [-DMUTATE=<region>]); emits ivs_profiles.inc (regions, device records, rd_region/wr_region[profile][4096], profile registry, _Static_asserts)"
  - "core/src/memmap/memsys.{h,c}: the only includer of ivs_profiles.inc; one width formula, switch dispatch, gates, stubs, bounded 16-segment cartridge scan, install primitives, slot clear, gate setter, ivs_memsys_poke_raw_internal, ivs_memsys_describe, ivs_profile_count/ivs_profile_get, bus function ivs_memsys_bus"
  - "core/CMakeLists.txt: generated include rule (gen/ivs_profiles.inc, target ivs_core_gen, IVS_CORE_GEN_DIR); tests/faults.cmake: gen dir on every plain and owner core copy; ivs_fault_gen CONSUMER core:<src>"
  - "suite memmap (55 cases, both presets) and data/expected/faults/memmap.ids (6 faults)"
affects: [03-05, 03-06, 03-07, 03-08, 03-09, 03-10, 03-11, 03-12, phase-4-run-api]

actuals:
  tokens: 27200
  tasks: 3
  commits: 3
plan_head_before: a662229e0196e761c276c85601fdaa7dba255c86
plan_head_after: ece054516eaf6da96e7c731aaf7d8c381e62ecfd

tech-stack:
  added: []
  patterns:
    - "A core TU consumes a build-tree table: the generator rule and its target live in core/CMakeLists.txt, the directory and target are read by tests/faults.cmake through get_directory_property, and the fault copies wait on the target"
    - "Numeric ids are written twice on purpose (memsys.h macros, the generator's fixed lists) and the generated file asserts they agree (_Static_assert)"
    - "A generator-path fault on a core table: ivs_fault_gen(... CONSUMER core:<src>) rebuilds that TU as the owner copy with the fault directory first on its include path"

key-files:
  created:
    - cmake/IvsMemmapTable.cmake
    - core/src/memmap/memsys.h
    - core/src/memmap/memsys.c
    - tests/memmap/memmap_main.c
    - data/expected/tests/memmap.ids
    - data/expected/faults/memmap.ids
  modified:
    - data/profiles/intv-2609-ntsc-family-p1.json
    - data/expected/profile-fields.ids
    - data/discrepancies.json
    - data/expected/discrepancies.ids
    - data/expected/core-generated.ids
    - core/CMakeLists.txt
    - tests/CMakeLists.txt
    - tests/faults.cmake

key-decisions:
  - "D-20 reading: the only mutable decode state is a committed segment table of at most 16 {addr, len_words} entries scanned by a bounded loop, not a per-256-word page table, because A-01 allows a segment (even one word) to start at any address in the span (A-08 c)"
  - "Every backing array holds 16-bit words, also for 8-bit devices: install primitives and the raw-poke seam store whole words and the width formula masks on read, so a wide stored value is observable (A-05a)"
  - "The physical device is the region name up to its first `_`; regions that share one (gram and its three write-only aliases) share one backing and must declare the same size_words and device kind, or the generator fails"
  - "Write-only alias rows carry conflict C61 (coexistence with cartridge ROM) rather than C21; C21 stays on stic_regs, stic_alias_read and gram"
  - "The registry record holds the profile id as a char[64] array, not a pointer, so the registry stays pointer-free like the instance"
  - "An IAB access with neither reason flag returns the undriven fill (no device answers); the CPU always sets one"
  - "The generator also requires every device kind record, checks rmask == 2^bits - 1, requires impl stub exactly for device stub on a stub physical device (stic, psg), and refuses a region name with a glossary unit token (H4)"
  - "MEM-01 and MEM-02 are not marked complete: the literal-map dump (MEM-01) and the D-22 boundary probes generated from the literal rows (MEM-02) land in later plans (same practice as 03-01..03-03)"

patterns-established:
  - "Later plans reach the memory system through memsys.h only: the instance embeds ivs_memsys, loaders validate and then call the void install primitives, the run seam passes ivs_memsys_bus as the CPU port"

requirements-completed: []

coverage:
  - id: D1
    description: "Profile descriptors to generated decode in one core TU (D-16, H2 a, b)"
    requirement: MEM-01
    verification:
      - kind: unit
        ref: "build/dev/tests/ivs_tests_memmap --only memmap.anchor.exec-lo (ok); suite memmap 55/55 in dev, ctest --preset release -R '^memmap$' passed"
        status: pass
      - kind: integration
        ref: "sh and dash tools/gate.sh: c-purity pass core-imports=4 (generator listed in core-generated.ids, output scanned and compiled)"
        status: pass
    human_judgment: false
  - id: D2
    description: "Fail-closed generator (T-03-11)"
    requirement: MEM-01
    verification:
      - kind: other
        ref: "scratch working-tree copy: overlap, misaligned lo, unknown device each stop the generator with an IvsMemmapTable: message; generator renamed gives c-purity-generated (see Refusal probes)"
        status: pass
    human_judgment: false
  - id: D3
    description: "Width formula, gates, stubs, unloaded fill, mirrors, cartridge scan and bus kinds (D-17, D-19, D-20)"
    requirement: MEM-02
    verification:
      - kind: unit
        ref: "suite memmap families width, anchor (lo and hi of 14 regions), mirror, gate, unloaded, cart, bus, describe, registry: 55/55"
        status: pass
      - kind: integration
        ref: "faults-verdict pass faults=79/79 (6 memmap faults detected by their intended cases)"
        status: pass
    human_judgment: false

duration: 15min
completed: 2026-11-11
status: complete
---

# Phase 3 Plan 04: Profile-driven memory system Summary

**The memory map now lives only in the profile. `cmake/IvsMemmapTable.cmake` checks every descriptor fail-closed and turns them into `ivs_profiles.inc` in the build tree. `core/src/memmap/memsys.c` is the one core TU that includes it. Every read goes through `read = (stored & rmask) | (fill & ~rmask)`; gates, stubs, mirrors, unloaded content and a bounded 16-segment cartridge scan are all handled there, and one bus function serves the CPU port. Discrepancies C60 to C62 record the new defaults. Suite `memmap` has 55 cases and there are six named faults, one of them a generator mutation that reaches the core TU. Final gate under sh and dash: `gate: PASS steps=24/24 controls=202/202 ... fault-runs=79 ... faults=79/79 ... tests-expected=1206 tests-executed=1206 tests-skipped=0 tests-missing=0`.**

## Performance

- **Duration:** about 15 min of execution (2026-11-11T05:33:13Z to 05:47:55Z), not counting the reading before the recorded start
- **Tasks:** 3 of 3
- **Files:** 14 in 3 task commits (diff +2234 / -25)
- **Gate time:** `gate-time total=19` to `24` (sh) and `19` to `21` (dash) on this host; the baseline before the plan was 20

## Accomplishments

- **Profile (D-16, D-18, A-01, A-02, D-24, D-26, D-32).** Each new field is a full 8-key record with tag, cite and conflict.
  - `memmap.undriven_read` `"$FFFF"`: unknown-default-chosen, C5.
  - Device records:
    - `rom10` `{10, $03FF, $0000, drop}`: tier 1, pdf 194 and 202.
    - `grom8` `{8, $00FF, $0000, drop}`: tier 1, pdf 197.
    - `ram16` `{16, $FFFF, $0000, keep}`: tier 1, pdf 199.
    - `ram8` `{8, $00FF, $0000, keep}` and `stub` `{0, $0000, $FFFF, drop}`: unknown-default-chosen, C60.
  - 14 region records: `stic_regs`, `stic_alias_read`, `scratchpad`, `psg_regs`, `sysram`, `exec`, `grom`, `gram`, three `stic_alias_w{4,8,c}` and three `gram_alias_w{7,b,f}`. Their values follow the context table, with `size_words` being the power-of-two backing size (scratchpad 256, sysram 512, every GRAM row 512).
  - `memmap.cart_span` `{$4000, $FFFF}`: C62. There is no alignment field (A-01).
  - `cartridge.features_supported` is `0` (delegated decision, D-32).
  - `system_images.exec` is `{4096, 10, $1000, 1}` and `system_images.grom` is `{2048, 8, $3000, 1}` (delegated decision, D-24/D-26).
  - `system_images.exec_grom` now states the D-24 policy, including the label "project boot stub, not an EXEC".
  - `cartridge.address_windows` was removed. `grep -c address_windows data/claims.json` printed 0, so no claim depended on it.
- **Discrepancies (D-23).**
  - C60 (partial): upper bits of 8-bit RAM, PSG and STIC reads.
  - C61 (open): write-only aliases coexisting with cartridge ROM.
  - C62 (open): the cartridge span. The 256-word alignment appears only as a tier-4 note.
  - C21: resolver changed to `measurement: D10 ...`; sources now include the GI pdf 197 window and the `$3A00-$3FFF` write disagreement.
  - C51: now `partial`, with ROM upper-bit evidence (pdf 194, 202).
- **Generator.** It takes `-DROOT` and `-DOUT`, plus an optional `-DMUTATE=<region>`. It reads every `data/profiles/*.json` in sorted order and fails with `IvsMemmapTable: <profile>: <field>: <reason>` on any of:
  - `hi < lo`, or a range off the 16-word granule
  - an overlap within a view
  - a `size_words` that is not a power of two
  - an extent that is neither at most `size_words` nor a whole multiple of it
  - an unknown device, gate, impl, view, physical device or memmap key
  - a missing or unknown key inside a value object
  - a missing required field
  - an `rmask` that does not match `bits`
  - an impl that does not fit the device
  - a read-driving region inside the span
  - a non-zero power-on fill
  - a glossary unit token in a region name

  The output holds `static const` data only, plus the id and capacity `_Static_assert`s (29 for this profile). It carries no host path and no time.
- **Core TU (D-17, D-19, D-20).**
  - Dispatch is a switch on device and physical ids. There are no function pointers; the acceptance grep printed 0.
  - Stub devices hold no state.
  - A closed gate reads the undriven fill and drops writes. Unloaded EXEC, GROM and cartridge regions also read the undriven fill.
  - In the read view, the cartridge span scans at most 16 committed segments in a bounded loop.
  - IAB answers by reason flag; INTAK and the strobes return data 0; every reply has wait 0.
  - All state is in the pointer-free `ivs_memsys`: a 76-byte header plus 113,152 bytes of backing, with no implicit padding.
- **Build wiring (H2 a, b, e).**
  - The `add_custom_command` in `core/CMakeLists.txt` depends on every profile JSON (a `CONFIGURE_DEPENDS` glob) and on the generator.
  - The core objects get `gen/` on their include path and depend on `ivs_core_gen`.
  - Every plain and owner fault copy gets the same directory and dependency.
  - `ivs_profiles.inc cmake/IvsMemmapTable.cmake` is the first non-comment line of `data/expected/core-generated.ids`. The `#allow-empty` line was removed in the same commit as `memsys.c`.

## Refusal probes (Task 1 acceptance)

The probes ran in a scratch working-tree copy: `git clone -q .` plus a tar overlay of `git ls-files -co --exclude-standard`. Each one edited the profile copy and ran `cmake -DROOT=<copy> -DOUT=<scratch> -P cmake/IvsMemmapTable.cmake`:

| Probe | Message |
|-------|---------|
| overlap: an added region `exec_overlap` `$1800-$18FF` | `IvsMemmapTable: data/profiles/intv-2609-ntsc-family-p1.json: memmap.region.exec_overlap: overlaps exec in the rd view` |
| misaligned lo: `exec.lo` = `$1008` | `IvsMemmapTable: data/profiles/intv-2609-ntsc-family-p1.json: memmap.region.exec: lo is not aligned to 16 words` |
| unknown device: `exec.device` = `rom12` | `IvsMemmapTable: data/profiles/intv-2609-ntsc-family-p1.json: memmap.region.exec: unknown device 'rom12'` |
| c-purity, generator renamed to `cmake/IvsMemmapTableRenamed.cmake` | `-- FAIL c-purity-generated: ivs_profiles.inc (generator cmake/IvsMemmapTable.cmake missing)` |

## Named faults (data/expected/faults/memmap.ids)

Each fault was observed failing only inside its allowed prefixes, and `faults-verdict` credited all six:

| Fault | Mutation | Must fail | Failing ids observed |
|-------|----------|-----------|----------------------|
| GEN_MAP_MUTATION | generator moves `sysram.lo` to `$0210` | `memmap.anchor.sysram-lo` | sysram-lo, describe.stub-vs-undriven |
| MEM_UNDRIVEN_ZERO | the undriven fill becomes `0000` | `memmap.unloaded.exec-reads-ffff` | 23 ids: unloaded, width.undriven, gate, cart and the write-only alias anchors |
| MEM_CART_END_INCLUSIVE | segment end compared inclusive | `memmap.cart.above-5001` | cart.above-5001, cart.sixteen-segments, cart.under-gram-alias |
| MEM_WO_READS_DEVICE | an unmatched cartridge-span read returns the write-view device | `memmap.anchor.gram-alias-w7-lo` | the GRAM alias anchors, unloaded.cart, two cart cases |
| MEM_MIRROR_MASK | backing index mask one bit short | `memmap.mirror.gram-3a00` | mirror.gram-3a00 only |
| MEM_IAB_VECTOR_SWAPPED | reset and interrupt answers swapped | `memmap.bus.iab-reset` | bus.iab-reset, bus.iab-interrupt |

The must-pass id is `memmap.bus.iab-reset` for five faults. For MEM_IAB_VECTOR_SWAPPED it is `memmap.anchor.exec-lo`.

## TDD evidence (Task 2)

RED: the Task 2 test file was built against the Task 1 tree (commit `de3506a`, scratch clone). It gave `# suite=memmap executed=54 skipped=0 failed=30`: every case that needs a Task 2 region, device, span or slot failed.

GREEN: with the Task 2 data and generator, the same 54 cases passed (`failed=0`).

The decode code itself landed with the tracer (see Deviations), so Task 2's RED/GREEN boundary is the data plus the generator's required-field change.

## Task Commits

1. **Task 1 (tracer): EXEC region from profile through generated decode to the bus.** `de3506a` (feat). Tracer verified end to end before expansion: `cmake --build --preset dev && build/dev/tests/ivs_tests_memmap --only memmap.anchor.exec-lo` printed ok, and the gate passed under sh (`gate-time total=24`) and dash (`19`) with `faults=73/73`.
2. **Task 2: every descriptor, C60-C62, anchor suite.** `521b6f4` (feat). 54/54 in dev; the release ctest passed. The gate passed under sh and dash with `discrepancies=62` and `profile-keys=51`.
3. **Task 3: core-consumed GEN path and six faults.** `ece0545` (feat). The gate passed under sh and dash with `fault-runs=79`, `faults=79/79` and `controls=202/202`.

**Plan metadata:** recorded in the docs(03-04) completion commit.

## Acceptance evidence

| Check | Result |
|-------|--------|
| `grep -c -E '^memmap\.(undriven_read\|device\.rom10\|region\.exec)$' data/expected/profile-fields.ids` | 3 |
| `grep -c -F 'ivs_profiles.inc cmake/IvsMemmapTable.cmake' data/expected/core-generated.ids` | 1 |
| `grep -c -E '^C6[012]$' data/expected/discrepancies.ids` | 3 |
| `grep -c '^memmap\.' data/expected/profile-fields.ids` | 21 |
| `grep -c 'cart_window\|cart_align' data/expected/profile-fields.ids` | 0 |
| `--list` memmap ids vs `data/expected/tests/memmap.ids` | 55 = 55 |
| function-pointer grep on `memsys.c` | 0 |
| `grep -v '^#' data/expected/faults/memmap.ids \| grep -c .` | 6 |
| `CONSUMER core:` count over `tests/CMakeLists.txt tests/faults.cmake` | 4 |

## Decisions Made

See `key-decisions` in the frontmatter. The D-20 reading, as the plan requires: the committed cartridge segment table is the only mutable decode state. It holds at most 16 `{addr, len_words}` entries and is scanned by a bounded loop. It replaces D-20's per-256-word `cart_page[]`, because A-01 allows a one-word segment at any address, and a page table cannot represent that (A-08 c).

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Task 1 fault sites deferred to Task 3**
- **Found during:** Task 1 gate.
- **Issue:** The full decode, written in Task 1, carried the five `#ifdef IVS_FAULT_MEM_*` sites. Until their ids file existed, `faults-verdict` failed with `fault-orphan-site`.
- **Fix:** The sites were taken out for the Task 1 and Task 2 commits and restored unchanged in Task 3 together with `data/expected/faults/memmap.ids`, following the rule that faults land with what they guard.
- **Committed in:** `ece0545`.

**2. [Rule 3 - Blocking] Generated-file comments**
- **Issue:** `data/profiles/*.json` inside a C block comment raises `-Wcomment` (`/*` within a comment), which `-Werror` turns into an error.
- **Fix:** The generated banner and the `memsys.c` header comment were reworded.
- **Committed in:** `de3506a`.

**3. [Rule 3 - Runner contract] `-DMUTATE` source for a CONSUMER GEN fault**
- **Issue:** `faults.cmake` always appended `-DMUTATE=<must-fail id>`. The memory-map generator takes a region name (`sysram`), not a test id.
- **Fix:** An `-DMUTATE=` already present in ARGS now replaces the default. `GEN_EXPECT_MUTATION` is unchanged.
- **Committed in:** `ece0545`.

**4. [Rule 2 - Fault sensitivity] Case `memmap.mirror.gram-3a00` added**
- **Issue:** Under MEM_MIRROR_MASK no existing case failed. Each anchor writes and reads the same address, and `$3800` and `$3A00` agree even under a 0xFF mask.
- **Fix:** A case that writes `$3800` and `$3900` and reads them at `$3A00`, `$3B00` and `$3E00` (plus `$3FFF` back to `$39FF`).
- **Committed in:** `ece0545`.

**5. [Plan shape] The decode landed in Task 1**
- **Issue:** The plan puts "complete the decode" in Task 2. Task 1's must-haves (every validation rule, install primitives, seams, bus function) already needed the whole TU.
- **Fix:** Task 2 added the data, the required-field change in the generator, and the tests. The RED evidence above shows the Task 2 cases failing on the Task 1 tree.
- **Committed in:** `de3506a`, `521b6f4`.

**6. [Plan wording] The tracer case is the generic anchor**
- **Issue:** The table-driven anchors generate `memmap.anchor.exec-lo`, so the separate tracer function would have duplicated the id, which the harness rejects.
- **Fix:** The separate function was removed. The generic anchor installs the stand-in EXEC (word 0 = `0xFFFF`), raw-pokes `0xFFFF` at `$1000` and expects `0x03FF` from the rule table.
- **Committed in:** `521b6f4`.

---

**Total deviations:** 6, all small. No existing token, test id, fault or control changed. Suites stay at 8, and controls stay at 202.

## Issues Encountered

None beyond the deviations.

## Open items

- **Gate-level generator controls.** The DISCUSS file lists `memmap-gen-*` negative controls (overlap, misaligned, non-power-of-two, unknown device, unknown key, cart overlap). This plan quotes scratch probes only and keeps `controls=202/202` as the plan expects. A later plan (the `memmap` gate step, 03-05/03-06) should add them.
- **Boundary probes (MEM-02).** The anchors test `lo` and `hi` of each region. The `lo-1`/`hi+1` and mirror-wrap probes generated from the literal map (D-22) are 03-05/03-06 work. MEM-01 and MEM-02 stay open until then.
- **Unused generated inventory lines** (from 03-03) are still not flagged. This is now moot for `ivs_profiles.inc`, which `memsys.c` includes.
- **SHA fault prefixes (from 03-02):** not applicable. This plan does not connect SHA-256 to the loader.
- **Hosted Ubuntu gcc:** the generated `ivs_profiles.inc` and `memsys.c` were built with Apple clang 21 only. The first hosted run checks gcc (`-Wtype-limits` would flag `idx < IVS_MM_REGION_COUNT` only for a profile with zero regions, which the data never has).

## Known Stubs

- STIC (`stic_regs`, `stic_alias_read`, `stic_alias_w4/w8/wc`) and PSG (`psg_regs`) regions are stub devices on purpose (D-19, `impl stub`). They read the undriven value and drop writes. Phases 5 and 6 replace the device kind; the region rows stay. This is the plan's goal, not a gap.

## Threat Flags

None. The generator, the bus-address-to-backing indexing and the gated or unloaded reads are the surfaces T-03-11 to T-03-14 already cover. Mitigations applied:
- T-03-11: fail-closed validation, probes and `GEN_MAP_MUTATION`.
- T-03-12: power-of-two backings with a masked index, capacity `_Static_assert`s, and ASan/UBSan in dev.
- T-03-13: the undriven fill and `MEM_UNDRIVEN_ZERO`.
- T-03-14: tag, cite and conflict on every field, plus C60 to C62.

## User Setup Required

None.

## Next Phase Readiness

- Plan 03-05 can write the literal map and compare it with a dump. The dump can probe all 65,536 addresses through `ivs_memsys_read` and `ivs_memsys_write` and use `ivs_memsys_describe` for the device-kind column (A-05b). The physical names it uses are stic, scratchpad, psg, sysram, exec, grom, gram, cart and undriven.
- The loader and instance plans can embed `ivs_memsys`, call the void install primitives after validation, and read slot shapes and the span from `ivs_profile_get`.
- **Final gate:**
  - `sh tools/gate.sh`: `gate: PASS steps=24/24 controls=202/202 cc=AppleClang c-presets=2 fault-runs=79 core-imports=4 purity-selftest=5/5 ... discrepancies=62 ... faults=79/79 ... profile-keys=51 ... tests-expected=1206 tests-executed=1206 tests-skipped=0 tests-missing=0 ...` (`gate-time total=19`)
  - `dash tools/gate.sh`: the same summary line (`gate-time total=20`).

## Self-Check: PASSED

- Files: `cmake/IvsMemmapTable.cmake`, `core/src/memmap/memsys.h`, `core/src/memmap/memsys.c`, `tests/memmap/memmap_main.c`, `data/expected/tests/memmap.ids` and `data/expected/faults/memmap.ids` were found.
- Commits: `de3506a`, `521b6f4` and `ece0545` are ancestors of HEAD. The ledger count from `a662229` is 3.
- Hygiene: no absolute home path and no "Intellivision" in any added line. `.planning/config.json` and `.planning/state.json` are still ` M` and in no commit of this plan.
