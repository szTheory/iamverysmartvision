---
phase: 03-memory-map-content-loading-and-refusal
plan: 03
subsystem: gate
tags: [purity, include-split, generated-includes, writable-data, D-06, D-07, A-06, H1, H2, H5]

requires:
  - phase: 03-memory-map-content-loading-and-refusal
    provides: "plan 03-02: core SHA-256 TU (third core TU), suite machine, 196 controls, 73 faults"
provides:
  - "core/include/ivs/ivs_status.h: the public status header, moved (git mv) from core/src/common, codes unchanged"
  - "include split: core objects see core/include and core/src PRIVATE; ivs_core exports only core/include; interface target ivs_core_internal (core/src) linked by the harness and every test executable"
  - "c-purity: scans and resolves core/include/**.h with the core/src rules; compiles with -I core/include -I core/src -I <temp>/gen -fno-pie"
  - "data/expected/core-generated.ids (empty, #allow-empty until 03-04) and the generated-include hook in c-purity: listed generators run into the step's temp dir, their output is scanned and compiled against; token c-purity-generated"
  - "core-symbols.sh --forbid-writable-data: token core-symbols-writable for nm types b B d D c C, used only by c-purity on its own -fno-pie archives"
  - "controls core-purity-include-outside, core-purity-generated-unlisted, core-purity-generated-scan, core-purity-generated-missing, core-purity-strlen, core-purity-static-mutable"
affects: [03-04, 03-05, 03-08, 03-09, 03-12, phase-4-installed-consumer]

actuals:
  tokens: 9200
  tasks: 3
  commits: 3
plan_head_before: 00d7b02118cd0eaf225d5990d975613661fddbe4
plan_head_after: 4b6ab4e97f02ed2888c4017a3ea58d8f4849ffd0

tech-stack:
  added: []
  patterns:
    - "A gate input that a control must be able to replace lives under data/expected/ and is read from ROOT (the control's copied tree), not from CHECK_DIR"
    - "A build-tree generated file reaches a core TU only through a reviewed `<basename> <generator>` line; the purity step runs the generator itself, so the text it checks is the text it compiles"
    - "Structural no-shared-state proof: writable data symbols are failed on the purity step's own non-instrumented -fno-pie archives, never on the sanitized ctest archive"

key-files:
  created:
    - core/include/ivs/ivs_status.h
    - data/expected/core-generated.ids
    - tools/check/controls/core-purity-include-outside/
    - tools/check/controls/core-purity-generated-unlisted/
    - tools/check/controls/core-purity-generated-scan/
    - tools/check/controls/core-purity-generated-missing/
    - tools/check/controls/core-purity-strlen/
    - tools/check/controls/core-purity-static-mutable/
  modified:
    - core/src/cpu/cp1610.h
    - core/CMakeLists.txt
    - tests/CMakeLists.txt
    - tests/faults.cmake
    - tools/check/steps/c-purity.sh
    - tools/check/core-symbols.sh
    - data/expected/controls/core-purity.ids
  deleted:
    - core/src/common/ivs_status.h (moved to core/include/ivs/)

key-decisions:
  - "A generated file gets the header-level scan (include, asm, float literal). The float/double type-word rule stays .c-only as before; the prelude's poison pragma still stops those words at compile time"
  - "Inventory lines are checked strictly: a malformed line, a basename with a slash or a leading dot, a generator path that is absolute or contains `..`, and a duplicate basename each fail with c-purity-generated (<reason>)"
  - "On Darwin every symbol whose name starts with l or L is dropped before the writable rule. C names carry a leading underscore there, so this covers ltmpN, l_.str, lCPI*, lJTI* and L* labels without hiding a C static"
  - "A writable finding is printed once per archive (`in core-O0.a`, `in core-O2.a`). It is not merged across the two variants the way imports are, so the reader sees which optimization level holds the symbol"
  - "API-01 and API-06 are not marked complete: this plan opens gate seams only (same practice as 03-01 and 03-02)"

patterns-established:
  - "Plan 03-04 adds its first generated core include by appending `ivs_profiles.inc cmake/IvsMemmapTable.cmake` (or its real name) to data/expected/core-generated.ids and removing the #allow-empty line, in the same commit as the TU that includes it"

requirements-completed: []

coverage:
  - id: D1
    description: "Status header moved to the public tree; ivs_core exports only core/include; tests reach internals through ivs_core_internal (D-06, H1)"
    requirement: API-01
    verification:
      - kind: integration
        ref: "cmake --build --preset dev && cmake --build --preset release && ctest --preset dev -R '^(cpu-api|machine)$' (2/2 passed); gate faults=73/73"
        status: pass
    human_judgment: false
  - id: D2
    description: "Reviewed generated includes in c-purity (A-06, H2 c-d)"
    requirement: API-06
    verification:
      - kind: integration
        ref: "sh tools/check/run-controls.sh: control ok for core-purity-include-outside, -generated-unlisted, -generated-scan, -generated-missing"
        status: pass
      - kind: other
        ref: "scratch tree: toy generator without the float literal gives `-- c-purity pass core-imports=0 purity-selftest=5/5` (the hook generates, scans and compiles)"
        status: pass
    human_judgment: false
  - id: D3
    description: "Writable-data rule on -fno-pie archives (D-07, H5)"
    requirement: API-01
    verification:
      - kind: integration
        ref: "controls core-purity-static-mutable and core-purity-strlen ok; sh and dash gate: controls=202/202"
        status: pass
      - kind: other
        ref: "docker gcc:13 (GCC 13.5.0, dash, mawk): c-purity on the real core passes; both new controls fail with their own token"
        status: pass
    human_judgment: false

duration: 8min
completed: 2026-11-11
status: complete
---

# Phase 3 Plan 03: Public include tree, generated includes and the writable-data rule Summary

**The status header now lives at `core/include/ivs/ivs_status.h` and is the first public header. `ivs_core` exports only `core/include`; tests reach `core/src` through a new interface target, `ivs_core_internal`. `c-purity` now treats `core/include` like `core/src`. It accepts a generated include only when `data/expected/core-generated.ids` lists it, and in that case it runs the generator, scans the output and compiles against it. Its own archives are now built with `-fno-pie` and fail on any writable data symbol. Six new controls cover these paths. Final gate under sh and dash: `gate: PASS steps=24/24 controls=202/202 ... faults=73/73 ... purity-selftest=5/5 ... tests-missing=0`.**

## Performance

- **Duration:** about 8 min of execution (2026-11-11T05:15:28Z to 05:23:33Z), not counting the reading before the recorded start
- **Tasks:** 3 of 3
- **Files:** 31 in 3 task commits (diff +312 / -46)

## Accomplishments

- **Header move and include split (D-06, H1).**
  - `git mv core/src/common/ivs_status.h core/include/ivs/ivs_status.h`. The guard and the codes `IVS_OK 0`, `IVS_E_BAD_ARG (-1)` and `IVS_E_BAD_STATE (-2)` are unchanged. The header comment now calls it the public status header, frozen and append-only (D-09).
  - `cp1610.h` includes `"ivs/ivs_status.h"`. Its stale comment ("the public header directory stays empty") was corrected.
  - In `core/CMakeLists.txt`, every core object gets PRIVATE `core/include` and `core/src`. `ivs_core` gets PUBLIC `core/include` only. `ivs_core_internal` is an INTERFACE target that carries `core/src`.
  - `ivs_test_harness` links `ivs_core_internal` PUBLIC, and so does every test executable.
  - `tests/faults.cmake` adds `core/include` before `core/src` on all five include lines: plain copies, harness copy, suite objects, owner copies and GEN_ executables.
- **c-purity, source layer.**
  - Headers are now collected from `^core/(src|include)/.*[.]h$`.
  - A quoted include resolves as `core/src/<h>`, as `core/include/<h>`, relative to the including file, or as a listed generated basename.
  - Every other rule is unchanged: the angle-include list, computed includes, the prelude, asm and float.
- **Generated-include hook (A-06, H2 c-d).**
  - The step reads `$ROOT/data/expected/core-generated.ids`. A missing file counts as an empty list, so the good tree passes.
  - For each listed line it runs `cmake -DROOT=<tree> -DOUT=<temp>/gen/<basename> -P <tree>/<generator>`.
  - The step fails with `c-purity-generated: <basename> (<reason>)` on a malformed line, a bad basename, a bad generator path, a duplicate, a missing generator, a generator failure or missing output.
  - The generated text is scanned and reported as `gen/<basename>:<line>`. Stage 2 adds `-I<temp>/gen`.
- **Writable-data rule (D-07, H5).**
  - The new opt-in flag `--forbid-writable-data` runs `nm -P` without `-g`, so file-local statics are visible. It drops Darwin `l`/`L` labels and fails types `b B d D c C` with `-- FAIL core-symbols-writable: <symbol> (<type>) in <archive>`.
  - `s`/`S` and `r`/`R` are not failed.
  - `c-purity` compiles both variants with `-fno-pie` and passes the flag on both archives.
  - `tests/core-symbols-tap.sh` and the ctest `core-symbols` registration are untouched (imports only).

## Control behaviour (each run by hand on a copied good tree)

| Control | Overlay | Output |
|---------|---------|--------|
| core-purity-include-outside | `good.c` includes `"other/thing.h"`, present only as `tests/other/thing.h` | `-- FAIL c-purity-include: core/src/cpu/good.c:6` |
| core-purity-generated-unlisted | `good.c` includes `"ivs_unlisted.inc"`, no inventory | `-- FAIL c-purity-include: core/src/cpu/good.c:5` |
| core-purity-generated-scan | inventory `ivs_toy.inc cmake/IvsToyTable.cmake`, toy generator writing `(unsigned)(0.5 * 4)` | `-- FAIL c-purity-float-literal: gen/ivs_toy.inc:2` |
| core-purity-generated-missing | inventory `ivs_gone.inc cmake/IvsGone.cmake`, no generator | `-- FAIL c-purity-generated: ivs_gone.inc (generator cmake/IvsGone.cmake missing)` |
| core-purity-strlen | `while (s[n] != 0) n++;` over a `const char *` | `-- FAIL core-symbols-forbidden: strlen` |
| core-purity-static-mutable | `static unsigned ivs_good_count;` incremented | `-- FAIL core-symbols-writable: ivs_good_count (b) in core-O0.a` and the same `in core-O2.a` |

Every overlay `good.c` keeps both `GOOD_FAULT` and `GOOD_FAULT2` sites.

The toy generator was also run with the float literal replaced by `2u`. The step then printed `-- c-purity pass core-imports=0 purity-selftest=5/5`, so a listed generator's output really is generated, scanned and compiled. A scratch inventory with a duplicate line, a basename `bad/x.inc` and a one-field line produced the three expected `c-purity-generated` reasons.

**Strlen lowering:** Apple clang 21 at `-O2` lowers the loop to `strlen`, and so does gcc 13.5.0 (docker `gcc:13`). The explicit-`strlen` fallback in the plan was not needed.

## Linux probe (Task 3 step 4)

`docker images` listed `gcc:13`. It ran with `--network none` and the repo mounted read-only (GCC 13.5.0, `/bin/sh` is dash, mawk 1.3.4):

| Probe | Result |
|-------|--------|
| `cp1610.c`, `cp1610_decode.c`, `ivs_sha256.c` at `-O0 -fno-pie`, `nm -P` types | `10 T, 3 U, 3 r, 49 t`; no `b B d D c C` |
| same at `-O2 -fno-pie` | `10 T, 2 U, 2 r, 8 t`; no `b B d D c C` |
| `cp1610_decode.c -O2 -fPIE` (contrast) | `ivs_cp1610_rows d 0 6f0`, the PIE false positive the rule avoids |
| `tools/check/steps/c-purity.sh` on the real core | `-- c-purity pass core-imports=1 purity-selftest=5/5` |
| same step, control core-purity-static-mutable | `core-symbols-writable: ivs_good_count (b)` for both archives |
| same step, control core-purity-strlen | `core-symbols-forbidden: strlen` |

Apple clang 21 on this host: the real core archives at `-O0` and `-O2` with `-fno-pie` hold only `T U s t` symbols. `ivs_cp1610_rows` is `s`.

This is the `gcc:13` image, not Ubuntu's packaged default-PIE gcc. RESEARCH A1 still applies: the first hosted run is the check for that compiler.

## Task Commits

1. **Task 1 (tracer): status header moved, include split, c-purity scan scope.** `662c30c` (feat). Tracer verified end to end before expansion: both presets built, `ctest -R '^(cpu-api|machine)$'` gave 2/2, and `sh tools/gate.sh` gave `gate: PASS steps=24/24 controls=196/196 ... faults=73/73 ...` (`gate-time total=20`).
2. **Task 2: generated-include hook, inventory, four controls.** `96534a1` (feat). `run-controls.sh` ended `controls=200/200`, and `sh tools/gate.sh` ended `gate: PASS ... controls=200/200 ...`.
3. **Task 3: writable-data rule, `-fno-pie`, two controls.** `4b6ab4e` (feat). `controls=202/202`. Gate PASS under sh (`gate-time total=20`) and dash (`gate-time total=22`).

**Plan metadata:** recorded in the docs(03-03) completion commit.

## Acceptance evidence

| Check | Result |
|-------|--------|
| `test -f core/include/ivs/ivs_status.h && test ! -e core/src/common/ivs_status.h` | true |
| `grep -c -F '"ivs/ivs_status.h"' core/src/cpu/cp1610.h` | 1 |
| `grep -v '^#' tools/check/steps/c-purity.sh \| grep -c 'core/include'` | 2 (baseline 0) |
| `grep -c -F 'c-purity-generated' tools/check/steps/c-purity.sh` | 9 (baseline 0) |
| `grep -c -F 'forbid-writable-data' tools/check/core-symbols.sh` | 4 (baseline 0) |
| `grep -c -F 'fno-pie' tools/check/steps/c-purity.sh` | 3 (baseline 0) |
| `grep -c forbid tests/core-symbols-tap.sh` | 0 (ctest suite stays imports-only) |
| `control ok` lines for the four Task 2 controls | 4 |
| `control ok` lines for the two Task 3 controls | 2 |
| Gate `core-imports=` | 3, unchanged: `-fno-pie` added no import |

## Decisions Made

See `key-decisions` in the frontmatter.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Runner limit] `core-purity-generated-missing` uses an overlay, not `MAKE write`**
- **Found during:** Task 2, step 3.
- **Issue:** The `write` verb in `run-controls.sh` writes only its third word (`printf '%s\n' "${3:-x}"`). So `write data/expected/core-generated.ids ivs_gone.inc cmake/IvsGone.cmake` would write the line `ivs_gone.inc` without its generator path. That line is malformed, so the control would test the bad-line path instead of a missing generator.
- **Fix:** The control ships `overlay/data/expected/core-generated.ids` with the two-field line. Its EXPECT names the full reason: `FAIL c-purity-generated: ivs_gone.inc (generator cmake/IvsGone.cmake missing)`. The runner was not changed.
- **Committed in:** `96534a1`.

**2. [Rule 2 - Fail closed] Strict inventory grammar in the generated-include hook**
- **Issue:** The plan defines only the missing-generator and generator-failure reasons. Without more checks, a one-field line, a basename with a path, an absolute or `..` generator path, or a duplicate basename would be silently misread.
- **Fix:** Each of these cases fails with `c-purity-generated: <name> (<reason>)`. They were probed in a scratch tree (three reasons observed). No control was added for them, to keep the control count at the planned 202.
- **Committed in:** `96534a1`.

**3. [Rule 1 - Platform] Darwin label filter widened to every `l`/`L` name**
- **Issue:** The plan lists `ltmp*`, `l_*` and `L*`. At `-O2`, Apple clang also emits `lCPI*` and `lJTI*` labels. They are type `s` today, so they are harmless now, but a narrow prefix list would rot.
- **Fix:** On Darwin, every name starting with `l` or `L` is dropped. This cannot hide a C static, because C symbols on Darwin have a leading underscore. The filter is not applied on ELF, where a static keeps its plain name.
- **Committed in:** `4b6ab4e`.

**4. [Rule 1 - Stale comment] `cp1610.h` header comment**
- **Issue:** The comment said the public header directory stays empty until API-01..06. That stopped being true with the move.
- **Fix:** It now says the internal header lives in `core/src` (seen only by the core and through `ivs_core_internal`) and uses the public status header.
- **Committed in:** `662c30c`.

---

**Total deviations:** 4, all small. No change to any existing token, inventory id, fault or test count.

## Issues Encountered

- The first `git add` of Task 1 named the old path `core/src/common/ivs_status.h`, which `git mv` had already staged as a rename. Git refused the pathspec and nothing was committed. The same paths without the old name were then staged and committed.

## Open items

- **First generated core include (plan 03-04).** The inventory is empty, with `#allow-empty`. 03-04 must:
  - list its table generator
  - make it work when called with only `-DROOT` and `-DOUT` (any `MUTATE` must stay optional)
  - remove the `#allow-empty` line in the same commit
  - handle H2 (a), (b) and (e): the CMake rule, the generated include dir on the `tests/faults.cmake` plain and owner copies, and `GEN_MAP_MUTATION`. This plan settled only (c) and (d).
- **Unused inventory lines are not flagged.** A listed basename that no core TU includes still passes. Consider an `unused` reason once real entries exist.
- **The c-purity hook needs `cmake` on PATH** in the step environment. The gate already needs it for other steps, and an empty inventory never calls it.
- **Hosted Ubuntu gcc (RESEARCH A1).** The `-fno-pie` rule is confirmed locally on Apple clang 21 and on the `gcc:13` image. The packaged default-PIE gcc on the hosted runner is checked by the first hosted run of the phase PR.
- **SHA fault prefixes (from 03-02).** Not applicable here: this plan does not connect SHA-256 to the loader. The plan that does must widen the `sha256.ids` allowed prefixes in the same commit.
- **API-01 and API-06** stay open. This plan opens the include, generated-include and writable-data seams they need.
- **Phase 4 consumer.** `ivs_core` now exports only `core/include`. An installed consumer still needs the install/export rules (Phase 4).

## Known Stubs

None.

## Threat Flags

None. The generator run inside `c-purity` is the surface T-03-07 already covers: only reviewed inventory lines run, and three controls guard them.

## User Setup Required

None.

## Next Phase Readiness

- Plan 03-04 can add a core TU that includes a generated table once it adds the inventory line. `c-purity` will generate, scan and compile it with no further step change.
- Plan 03-09 can add `core/include/ivs/ivs.h` and `ivs_machine.h`. They are scanned with the core rules and resolve as `"ivs/<h>"`.
- From now on, any new core TU with a writable static fails `c-purity` (`core-symbols-writable`).
- **Final gate:**
  - `sh tools/gate.sh`: `gate: PASS steps=24/24 controls=202/202 cc=AppleClang c-presets=2 fault-runs=73 core-imports=3 purity-selftest=5/5 ... faults=73/73 ... tests-expected=1096 tests-executed=1096 tests-skipped=0 tests-missing=0 ...` (`gate-time total=20`)
  - `dash tools/gate.sh`: the same summary line (`gate-time total=22`).

## Self-Check: PASSED

- Files: `core/include/ivs/ivs_status.h`, `data/expected/core-generated.ids` and the six control directories (each with CHECK, EXPECT and an overlay) were found. `core/src/common/ivs_status.h` is gone (moved).
- Commits: `662c30c`, `96534a1` and `4b6ab4e` are ancestors of HEAD. The ledger count from `00d7b02` is 3.
- Hygiene: no absolute home path and no "Intellivision" in any new file. `.planning/config.json` and `.planning/state.json` are still ` M` and in no commit of this plan.
