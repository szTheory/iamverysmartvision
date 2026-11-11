---
phase: 03-memory-map-content-loading-and-refusal
plan: 06
subsystem: memmap
tags: [memmap, literal-map, decode-dump, boundary-probes, named-faults, claims, D-17, D-19, D-21, D-22, H6, MEM-01, MEM-02]

requires:
  - phase: 03-memory-map-content-loading-and-refusal
    provides: "plan 03-04: profile-driven memory system, raw-poke and describe seams (A-05); plan 03-05: the sourced literal map and the memmap gate step"
provides:
  - "cmake/IvsMemmapProbes.cmake: probe table generated from data/memmap/*.literal.txt only (rows, structural projection lines, literal SHA256 as suite sources)"
  - "Suite memmap case memmap.dump.matches-literal: all 65,536 addresses of both views probed through the bus function, merged, compared with the literal and written to <results>/<preset>/memmap-dump.txt with a nonce header"
  - "memmap gate step: consumer half comparing the dev and release dumps with the literal (memmap-dump-missing, memmap-dump-stale, memmap-dump-diff; stat dump-lines)"
  - "28 cases memmap.row.<view>-<LO>-<HI> and 2 cases memmap.row-gate.<gate>-closed; expected values from a rule table keyed on the literal's device and width"
  - "Faults MEM_ROM_NO_MASK, MEM_RAM8_NO_TRUNC, MEM_RAM16_TRUNC, MEM_ROM_WRITABLE, MEM_GATE_IGNORED (faults=84/84)"
  - "Controls memmap-dump-diff, memmap-dump-stale, memmap-dump-missing (controls=215/215)"
  - "Claim CL-0006 (MEM-01, source-expectation, provisional)"
affects: [03-07, 03-11, 03-13, 03-14]

actuals:
  tokens: 18650
  tasks: 3
  commits: 3
plan_head_before: bed1fe8be869bf605fea2c09e18dc10bbe302472
plan_head_after: b70d9b70df9830171fcbde8b3d7ec7583650d1fc

tech-stack:
  added: []
  patterns:
    - "Produce/consume split for a binary-derived check (H6): a test case writes a nonce-headed file into IVS_TEST_OUT_DIR, and a pure gate step compares it with a source file; the control runner drives the step with text overlays of build/gate"
    - "Expected values derived from the sourced literal (generated table) plus a test-side rule table keyed on device and width; never from the descriptors under test"
    - "A kept store is checked twice: read back through the bus, and as exactly one changed 16-bit word of the machine state holding value & rmask (catches a store kept wider than the read mask)"
    - "Dropped-store rows are checked by a state snapshot and memcmp after writing every address of the row"

key-files:
  created:
    - cmake/IvsMemmapProbes.cmake
    - tests/memmap/memmap_rows.c
    - tests/memmap/memmap_rows.h
    - tools/check/controls/_good/tree/build/gate/dev/memmap-dump.txt
    - tools/check/controls/_good/tree/build/gate/release/memmap-dump.txt
    - tools/check/controls/memmap-dump-diff/
    - tools/check/controls/memmap-dump-stale/
    - tools/check/controls/memmap-dump-missing/
  modified:
    - tests/memmap/memmap_main.c
    - tests/CMakeLists.txt
    - core/src/memmap/memsys.c
    - core/src/memmap/memsys.h
    - data/profiles/intv-2609-ntsc-family-p1.json
    - tools/check/memmap.cmake
    - tools/check/steps/memmap.sh
    - data/expected/tests/memmap.ids
    - data/expected/faults/memmap.ids
    - data/expected/controls/memmap.ids
    - data/claims.json
    - data/expected/claims.ids

key-decisions:
  - "Profile corrected to match the literal (D-18, orchestrator decision 1): the six write-only aliases now carry the home gate (STIC aliases at 4000/8000/C000 gate stic-regs, GRAM aliases at 7800/B800/F800 gate gmem), tagged inference with the reasoning in the cite, conflict C61 kept. Effect: closing gmem or stic-regs also drops writes through the aliases"
  - "ivs_memsys_describe reports the shared backing or register file as the mirror of an alias region (another region of the same profile reaches the same physical device from a lower lo), matching the literal header's mirror definition; before, the STIC aliases (extent 64 = size 64) reported no mirror"
  - "Write-view row class comes from the literal: rows with a width are measured by write-and-read-back at the literal's read-side address; width-0 rows by snapshot and memcmp. Either kind of disagreement with the runtime shows up (diff line or failed case)"
  - "The dump carries one `# memmap-dump nonce=<n> profile=<id>` header per profile; the checker compares each section with the literal of a profile whose literal has no finding (one defect, one token) and does not compare a stale dump"
  - "Row probes judge lo-1 and hi+1 by the neighbouring literal row; a dropped row's own case also sweeps every address of the row (this is what lets MEM_ROM_WRITABLE fail the write row covering $1000)"
  - "memmap-gen-* generator controls not added here (open item for 03-14)"
  - "MEM-01 and MEM-02 marked complete: the decode dump equals the literal in both presets by gate evidence, and width masking and store truncation are probed at every literal row boundary with five faults; the MEM-01 claim stays provisional (source-expectation)"

patterns-established:
  - "data/memmap/<profile>.literal.txt drives both the static literal rules (03-05) and the generated probes and dump comparison (03-06)"

requirements-completed: [MEM-01, MEM-02]

coverage:
  - id: D1
    description: "Decode dump of every address in both views equals the literal on the structural columns (D-21, T-03-18)"
    requirement: MEM-01
    verification:
      - kind: integration
        ref: "sh and dash tools/gate.sh: -- memmap pass literal-rows=28 views=2 dump-lines=56"
        status: pass
      - kind: unit
        ref: "build/dev/tests/ivs_tests_memmap --only memmap.dump. (ok 1 memmap.dump.matches-literal)"
        status: pass
    human_judgment: false
  - id: D2
    description: "Width masking and store truncation probed at every literal row boundary; five width and gate faults each fail a named row (D-17, D-19, D-22, T-03-19)"
    requirement: MEM-02
    verification:
      - kind: unit
        ref: "ivs_tests_memmap --only memmap.row (30 cases) in dev; ctest --preset release -R '^memmap$'"
        status: pass
      - kind: other
        ref: "gate faults=84/84 (faults-verdict over the eleven memmap faults)"
        status: pass
    human_judgment: false
  - id: D3
    description: "Dump comparison fails for a diff, a stale nonce and a missing dump (T-03-18); CL-0006 states its columns (T-03-20)"
    requirement: MEM-01
    verification:
      - kind: other
        ref: "sh tools/check/run-controls.sh: control ok memmap-dump-{diff,stale,missing}, _good/memmap; controls=215/215; gate claims=6 provisional=5"
        status: pass
    human_judgment: false

duration: 12min
completed: 2026-11-11
status: complete
---

# Phase 3 Plan 06: Decode dump against the literal map, boundary probes and width faults Summary

**The runtime decode of all 65,536 addresses in both views now equals the sourced literal map on the structural columns, checked twice: inside suite `memmap` and by the `memmap` gate step from nonce-bound dev and release dumps. 28 generated row cases and 2 gate cases probe every literal boundary with expectations from a device-and-width rule table, five new faults each turn a named row red, and claim CL-0006 records MEM-01 as provisional. Final gate under sh and dash: `gate: PASS steps=25/25 controls=215/215 ... claims=6 provisional=5 ... faults=84/84 ... literal-rows=28 views=2 dump-lines=56 ... tests-expected=1268 tests-executed=1268 tests-skipped=0 tests-missing=0`.**

## Performance

- **Duration:** about 12 min of execution (2026-11-11T06:08:58Z to 06:20:41Z), not counting the reading before the recorded start
- **Tasks:** 3 of 3
- **Files:** 29 in 3 commits (diff +1377 / -45)
- **Gate time (local, warm build):** `gate-time total=20` (sh) and `21` to `22` (dash), against `20` before the plan. `step-time memmap 0`, `c-tests 1`, `c-faults 1`. Suite `memmap` runs in about 0.13 s in dev (ASan, 86 cases, dump included). The probe generator runs in under 0.01 s; rebuilding `memmap_rows.c` and the fault executables takes about 0.2 s each. The cold hosted build cost of the one new test TU and five fault executables was not measured here; the next hosted run shows it against the 44 s figure.

## Accomplishments

- **Probe generator (`cmake/IvsMemmapProbes.cmake`, prefix `mp_`).** Reads only `data/memmap/*.literal.txt`. It re-validates the row grammar, device and gate vocabularies, width, mirror and per-view tiling, and fails closed. It emits one record per row (profile, view, LO, HI, w, mirror, device, impl, gate, projection line; R rows then W rows by LO) and `ivs_mp_sources[]` with the literal's SHA256. The SHA256 appears as a `# source` line in the suite's TAP, which `tests-inventory` re-hashes. It is registered in `tests/CMakeLists.txt` with `DEPENDS` on the literal files and the generator, and the `memmap` fault suite now compiles `memmap_rows.c` against it.
- **Dump case `memmap.dump.matches-literal` (D-21, A-05, H6).**
  - Setup: stand-in EXEC `(37*i+11) & 3FF`, GROM `(53*i+7) & FF`, and cartridge words `(29*i+5) & 3FF` in one segment per literal `cart` read row (here `4000-FFFF`, 49,152 words).
  - Read-view width: popcount of the reads after raw-poking FFFF and 0000.
  - Write-view width, rows with a width: write FFFF, then 0000, and read each back at the literal's read-side address.
  - Write-view rows with width 0: snapshot, write every address, then memcmp with the snapshot.
  - Mirror of rows with a width: the smallest N where a change at lo is seen at lo+N. This is behavioural.
  - Device, impl, gate, and the mirror of stub and undriven rows come from `ivs_memsys_describe` (declarative).
  - The case merges adjacent equal tuples, compares the lines with the literal projection (its failure message names the first differing line) and writes `memmap-dump.txt`.
- **Consumer (`tools/check/memmap.cmake`, `steps/memmap.sh`).** The wrapper passes `-DGATE_DIR="${IVS_GATE_DIR:-$ROOT/build/gate}"`. For `dev` and `release` the checker reports:
  - `memmap-dump-missing: <preset> build/gate/<preset>/memmap-dump.txt`;
  - `memmap-dump-stale: <preset> ...: nonce '<n>' is not build/gate/NONCE ('<NONCE>')`, also for a dump with no header;
  - `memmap-dump-diff: <preset> dump line <k> of <id> is '<got>', literal has '<want>'`, plus a section of an unknown profile or a line before any header.

  It compares only profiles whose literal has no finding, so a literal-rule control still reports one token. New stat: `dump-lines=<lines over both presets>` (56).
- **Row probes (D-22, MEM-02).** `memmap.row.<view>-<LO>-<HI>` for all 28 literal rows. Each case probes lo-1 and hi+1 (judged by the neighbouring row), lo and hi, and, for rows with a mirror inside them, lo+mirror-1 and lo+mirror. The rule table (`exec`/`cart` 10 bits, `grom` 8, `scratchpad`/`gram` 8 kept, `sysram` 16 kept, `stic`/`psg`/`undriven` read FFFF) is keyed on the literal's device and width.
  - A kept store of 1234 and then of FFFF must read back as value & rmask and must change exactly one 16-bit state word to that value.
  - A dropped store must leave the whole state unchanged; a dropped row's own case also sweeps every address of the row.
- **Gate cases.** `memmap.row-gate.stic-regs-closed` and `memmap.row-gate.gmem-closed` run over every literal row of the gate. With the gate closed, reads give FFFF and stores are dropped. After reopening, the stored data reads back.
- **Faults (one site each in `core/src/memmap/memsys.c`).**

| Fault | Site | Must fail | Failing ids observed |
|-------|------|-----------|----------------------|
| MEM_ROM_NO_MASK | rom10 region read uses rmask FFFF | `memmap.row.R-1000-1FFF` | width.rom10, anchor.exec-*, dump, row.R-0360-0FFF/1000-1FFF/2000-2FFF |
| MEM_RAM8_NO_TRUNC | ram8 store keeps 16 bits | `memmap.row.W-0100-01EF` | every write row next to or holding scratchpad or GRAM, row-gate.gmem |
| MEM_RAM16_TRUNC | ram16 store masked to 8 bits | `memmap.row.W-0200-035F` | width.ram16, gate.stic-regs-scope, bus.*-kinds, anchor.sysram-*, dump, three write rows |
| MEM_ROM_WRITABLE | write-view undriven store under EXEC lands in the EXEC backing | `memmap.row.W-0360-37FF` | anchor.exec-*, dump, row.W-0360-37FF |
| MEM_GATE_IGNORED | region read ignores a closed gate | `memmap.row-gate.gmem-closed` | gate.gmem-closed-blocks, row-gate.gmem-closed |

  The allowed prefixes of the six older memmap faults now also cover the new families where a run showed collateral failures: `GEN_MAP_MUTATION` (dump, row), `MEM_UNDRIVEN_ZERO` (row, row-gate) and `MEM_MIRROR_MASK` (dump). `MEM_MIRROR_MASK` is caught by the dump, which measures mirror 256, and by `memmap.mirror.gram-3a00`, not by the row probes. Its 256-word aliasing still maps lo+512 and lo+511 like the real mirror.
- **Controls.** `memmap-dump-diff` (release read line `w=16`), `memmap-dump-stale` (dev nonce `stale`) and `memmap-dump-missing` (MAKE deletes the dev dump). Each control forbids the other two tokens. The good tree gained the two dumps that `_good/memmap` now needs (nonce `gate.good0`, the toy literal's two undriven lines).
- **Claim CL-0006.** Tier `source-expectation`, standing `provisional`, artifact the literal map. It depends on all 21 `memmap.*` profile fields. The statement separates the behavioural columns from the declarative labels. The note says a pass shows transcription consistency, not hardware truth, and that a D10 probe would raise the claim.

## Profile corrections made to match the literal (D-18)

| Field | Before | After | Basis |
|-------|--------|-------|-------|
| `memmap.region.stic_alias_w4`, `_w8`, `_wc` | `gate: none`, tag tier4-community | `gate: stic-regs`, tag `inference` | Literal header [INF]: an alias reaches the same STIC registers as `$0000-$003F`, so it shares their access window. Cite extended with the reasoning; conflict C61 kept |
| `memmap.region.gram_alias_w7`, `_wb`, `_wf` | `gate: none`, tag tier4-community | `gate: gmem`, tag `inference` | The same reasoning for graphics RAM at `$3800-$3FFF`; C61 kept |

The dump comparison includes the gate column behaviourally through `memmap.row-gate.*-closed`: with `gmem` closed, a store through `7800`, `B800` or `F800` is dropped. A scratch check put `gram_alias_w7` back to `gate: none`. The dump case then failed with `dump line 22 is 'W 7800-7FFF gram w=8 impl=real mirror=512 gate=-', literal has '... gate=gmem'`.

## Task Commits

1. **Task 1 (tracer): decode dump compared with the literal by the memmap step.** `2fac07f` (feat). The tracer was verified end to end before expansion:
   - `build/dev/tests/ivs_tests_memmap --only memmap.dump.` passed.
   - `sh tools/gate.sh` and `dash tools/gate.sh` both printed `gate: PASS ... dump-lines=56`.
   - `head -1 build/gate/dev/memmap-dump.txt` carried the run's NONCE.
   - The dump and the literal both have 28 `R|W` lines.
2. **Task 2: boundary probes per literal row and five width and gate faults.** `e1e8d3f` (feat). `--only memmap.row` passed 30 cases in dev, `ctest --preset release -R '^memmap$'` passed, and the gate passed under sh and dash with `faults=84/84 tests-missing=0`.
3. **Task 3: dump controls and claim CL-0006.** `b70d9b7` (feat). The control runner ended `controls=215/215`, and the gate passed under sh and dash with `claims=6`.

**Plan metadata:** recorded in the docs(03-06) completion commit.

## Acceptance evidence

| Check | Result |
|-------|--------|
| `head -1 build/gate/dev/memmap-dump.txt` versus `cat build/gate/NONCE` | `# memmap-dump nonce=gate.6PoBRX ...` / `gate.6PoBRX` (that run) |
| `grep -c -E '^(R\|W) '` dump / literal | 28 / 28 |
| `grep -c '^memmap\.row\.' data/expected/tests/memmap.ids` | 28 (literal rows: 28) |
| `grep -v '^#' data/expected/faults/memmap.ids \| grep -c .` | 11 |
| `sh tools/check/run-controls.sh \| grep -c -E 'control ok +memmap-dump-(diff\|stale\|missing)$'` | 3 |
| run-controls last line | `controls=215/215` (plan text: 213, see Deviations) |
| `grep -c -F '"id": "CL-0006"' data/claims.json` | 1 |

## TDD note (Task 2)

The memory system already existed (plan 03-04), so the new row cases passed against it on their first run. The faults give the RED evidence: each of the five fault builds fails its named must-fail row (table above), and the plain build passes them.

## Decisions Made

See `key-decisions` in the frontmatter.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] describe gave no mirror for the STIC aliases**
- **Found during:** Task 1, while comparing the planned dump rules with the literal's mirror definition.
- **Issue:** `ivs_memsys_describe` reported `mirror_words` only when a region's extent exceeds its size. The literal (03-05 header) gives `mirror=64` to `R 0040-007F` and to the three write-only STIC aliases, whose extent equals the 64-word register file. The dump takes the mirror of stub rows from describe, so these four lines would have differed.
- **Fix:** A region is an alias when another region of the same profile reaches the same physical device from a lower lo, and an alias reports its size as the mirror (`ivs_mm_is_alias`, test-only path). The header comment says so.
- **Files:** `core/src/memmap/memsys.c`, `core/src/memmap/memsys.h`. **Commit:** `2fac07f`.

**2. [Rule 1 - Profile vs literal] Alias gates (orchestrator decision 1)**
- See the table above. **Commit:** `2fac07f`.

**3. [Plan count] Controls 215, not 213**
- The plan text predates the two extra controls that 03-05 added. The orchestrator set the expected count to 215 (212 + 3). **Commit:** `b70d9b7`.

**4. [Small addition] `tests/memmap/memmap_rows.h`**
- **Issue:** `-Wmissing-prototypes` needs a declaration for the four functions `memmap_main.c` calls. The plan's file list has no header.
- **Fix:** a 20-line internal header. **Commit:** `2fac07f`.

**5. [Rule 2 - Missing check] Kept stores checked against the machine state**
- **Issue:** The width formula masks reads, so a ram8 store kept at 16 bits (`MEM_RAM8_NO_TRUNC`) reads back as `0x0034` and the plan's bus read-back alone cannot see it.
- **Fix:** A kept store must also change exactly one 16-bit word of the state, to value & rmask. This is behaviour of the state and does not use the descriptors.
- **Commit:** `e1e8d3f`.

**6. [Rule 2 - Missing check] Whole-row sweep for dropped write rows**
- **Issue:** The boundary probes of `W 0360-37FF` (035F, 0360, 37FF, 3800) never touch `$1000`, so `MEM_ROM_WRITABLE` could not fail its intended row.
- **Fix:** The case of a dropped write row also writes FFFF and 0000 to every address of the row and memcmps the state, as the dump does.
- **Commit:** `e1e8d3f`.

**7. [Clarity] Dump sections per profile, comparison skipped for a bad literal**
- **Issue:** The plan does not say how several profiles share one dump. A broken literal would also produce a diff token, which breaks the one-defect-one-token rule of the 03-05 controls.
- **Fix:** One header per profile, as in the dump format above. The checker compares only profiles whose literal has no finding, and it does not compare a stale dump.
- **Commit:** `2fac07f`.

**8. [Bug] Nested `/*` in comments**
- **Issue:** A `data/memmap/*.literal.txt` path inside a C comment (and in the generated file's first line) tripped `-Wcomment`.
- **Fix:** The C comments say `<profile>.literal.txt`, and the generated header says "the literal maps in data/memmap".
- **Commit:** `2fac07f`.

---

**Total deviations:** 8. The profile changed only on the six alias gates (as decided) and their tags and cites. No literal row changed. No existing test id, token or control changed. The allowed prefixes of three older faults were widened to the new case families.

## Issues Encountered

None beyond the deviations.

## Open items

- **memmap-gen-* generator controls (03-04, 03-05; orchestrator decision 2): not added; recorded for the plan 03-14 close-out.** This does not fit here. A gate-level control of `cmake/IvsMemmapTable.cmake` needs three things that the `memmap` step does not have:
  - (a) a way for a pure step to run a generator that lives outside `tools/check`;
  - (b) a fixture convention for a dedicated toy memmap profile (for example `data/memmap-gen/<case>.json`) that the good-tree profile (H11) and the real profiles do not share;
  - (c) six or so new controls (`-overlap`, `-misaligned`, `-nonpow2-mirror`, `-unknown-device`, `-unknown-key`, `-cart-overlap`), which change the expected count.

  Until then the generator stays covered by its fail-closed build refusals and by `GEN_MAP_MUTATION`.
- **Mirror probing in the row cases** only checks the wrap at lo+mirror. A mirror mask halved to 256 is caught by the dump (it measures 256) and by `memmap.mirror.gram-3a00`, not by a row case. A probe at lo+mirror/2 would add it if wanted.
- **Multi-profile ids.** Case ids `memmap.row.<view>-<LO>-<HI>` carry no profile id. A second literal with an identical row would collide, and the harness would stop on the duplicate id. Revisit when a second profile is added.
- **Hosted timing.** The local gate is unchanged at 20 to 22 s. The cold hosted cost of the new test TU and five fault executables has to be read from the next hosted run (budget 40 s, revisit trigger 50 s).
- **Unused generated-include lines** (03-03) are still not flagged. Out of scope.

## Known Stubs

None. The `impl=stub` rows of the literal describe the D-19 stub devices (STIC, PSG). They are not placeholders in this plan's output.

## Threat Flags

None. The new surface is covered by the threat register:
- T-03-18: the dump carries a nonce header, both presets are compared, and the diff, stale and missing cases each have a control.
- T-03-19: probes come from the literal only, the rule table lives in the test, and five faults back it.
- T-03-20: CL-0006 names its behavioural and declarative columns and is provisional.

## User Setup Required

None.

## Next Phase Readiness

- MEM-01 and MEM-02 are complete by gate evidence. CL-0006 stays provisional until a D10 decode probe exists.
- Plans 03-07 and 03-11 can rely on the decode of the cartridge span (`R 4000-FFFF cart w=10`), which the dump checks, and on the alias gates now in the profile.
- **Final gate:**
  - `sh tools/gate.sh`: `gate: PASS steps=25/25 controls=215/215 cc=AppleClang c-presets=2 fault-runs=84 ... claims=6 provisional=5 ... faults=84/84 ... literal-rows=28 views=2 dump-lines=56 ... tests-expected=1268 tests-executed=1268 tests-skipped=0 tests-missing=0 ...` (`gate-time total=20`)
  - `dash tools/gate.sh`: the same summary line (`gate-time total=21`).

## Self-Check: PASSED

- Files: the generator, `memmap_rows.c`/`.h`, both good-tree dumps and the three control directories were found.
- Commits: `2fac07f`, `e1e8d3f` and `b70d9b7` are ancestors of HEAD. The ledger count from `bed1fe8` is 3, and each commit message ends with the Co-Authored-By line.
- Hygiene: no absolute home path and no "Intellivision" in any added line. `.planning/config.json` and `.planning/state.json` are still ` M` and in no commit of this plan.
