---
phase: 02-cp-1610-cpu-core
plan: 07
subsystem: core-purity
tags: [c17, api-05, nm, allowlist, gate-step, negative-controls, ctest]

requires:
  - phase: 02-cp-1610-cpu-core
    plan: 06
    provides: the real core sources (cp1610.c, cp1610_decode.c) with the mandatory prelude, the gate C steps and the control runner conventions
provides:
  - tools/check/core-symbols.sh (nm allowlist check; fails closed on missing, bitcode, LTO, nm failure, zero definitions)
  - tools/check/core-symbols-allow.txt (10 reviewed names) and core-symbols-deny.txt (46 names, 7 categories)
  - gate step c-purity (source layer before any compile, fresh -O0/-O2 symbol layer, purity self-test 5/5)
  - 13 controls core-purity-{clock,env,fs,thread,io,rand,alloc,header,asm,float,float-literal,hollow,prelude}
  - ctest suite core-symbols on the shipped ivs_core archive (strict release, sanitizer prefixes only in dev)
affects: [02-08, 02-13, 02-14, 02-15, 02-16, 02-17, 02-18, phase-03-api-isolation, phase-04-installed-consumer]

actuals:
  tokens: 6886
  tasks: 3
  commits: 3
plan_head_before: d8095cf4b250f6e4ea27a011082f57c94ae4cead
plan_head_after: 03c45443f1632bb9c17abadc876ccb5c3af36dc8

tech-stack:
  added: []
  patterns:
    - "Purity is an allowlist on undefined external symbols of freshly compiled archives (and of the shipped archive), not a denylist; the deny list only labels categories"
    - "Source-layer findings stop a step before its compile stage so a violation is reported by its own token, proven by a control with a FORBID line for the compile token"
    - "Classes an overlay cannot produce (LTO, bitcode, missing input, no input) are covered by an in-step self-test stage on synthetic temp-dir inputs, reported as a stat"
    - "Instrumented-build allowances come from the build option (IVS_SYMBOL_PREFIX_ARGS), never from the check script or the allowlist"

key-files:
  created:
    - tools/check/core-symbols.sh
    - tools/check/core-symbols-allow.txt
    - tools/check/core-symbols-deny.txt
    - tools/check/steps/c-purity.sh
    - data/expected/controls/core-purity.ids
    - tools/check/controls/core-purity-*/ (13 controls)
    - tests/core-symbols-tap.sh
    - data/expected/tests/core-symbols.ids
  modified:
    - data/expected/gate-steps/cpu.ids
    - tools/check/controls/_good/tree/core/src/cpu/good.c
    - tests/CMakeLists.txt

key-decisions:
  - "core-symbols external= counts imports not defined inside the input (undefined minus defined, union over all inputs); c-purity reports the -O2 archive's count as core-imports (2 today: __stack_chk_fail, __stack_chk_guard)"
  - "The source layer is a C preprocessing-number tokenizer (comments, strings and character literals removed) rather than a regex after hex stripping: it flags 1e5, .5 and hex floats such as 0x1.8p3, and never flags digits inside identifiers or integer hex constants"
  - "The float-literal rule also scans core headers; float/double words stay .c-only because the prelude names them in its poison pragma"
  - "Bitcode inside an archive is found by extracting the members (AR, default ar) into the temp dir and reading each member's first four bytes; GCC LTO by the .gnu.lto_ section name in the file bytes or nm output"
  - "core-purity-lto is a self-test case, not an overlay control: the step forces -fno-lto, so an overlay cannot produce an LTO object (D-21 allows dropping a control when the class is still covered)"
  - "The deny list is optional (D-21); when present it must be disjoint from the allowlist"

patterns-established:
  - "A one-file stand-in core overlay per host-dependency class with a locally declared prototype (no header), so only the symbol layer can catch it"
  - "A helper script reads its lists from CHECK_DIR (or its own directory) so neither a control tree nor a test can substitute them"

requirements-completed: [API-05]

coverage:
  - id: D1
    description: "c-purity compiles core/src/**/*.c fresh at -O0 and -O2 and fails any undefined external symbol outside the reviewed allowlist; the real core passes (core-imports=2)"
    requirement: API-05
    verification:
      - kind: integration
        ref: "sh tools/gate.sh (-- c-purity pass core-imports=2 purity-selftest=5/5)"
        status: pass
    human_judgment: false
  - id: D2
    description: "One deliberate-violation control per host-dependency class (clock, env, fs, thread, io, rand, alloc) fails with core-symbols-forbidden: <name> (ROADMAP SC5)"
    requirement: API-05
    verification:
      - kind: integration
        ref: "sh tools/check/run-controls.sh (control ok core-purity-{clock,env,fs,thread,io,rand,alloc})"
        status: pass
    human_judgment: false
  - id: D3
    description: "Source layer: forbidden header, inline assembly, float type, float literal and missing prelude fail with their own tokens before any compile; hex constants and page cites are not flagged"
    requirement: API-05
    verification:
      - kind: integration
        ref: "sh tools/check/run-controls.sh (control ok core-purity-{header,asm,float,float-literal,prelude}, control ok _good/c-purity)"
        status: pass
    human_judgment: false
  - id: D4
    description: "core-symbols.sh rejects LTO objects, bitcode, zero-definition inputs, missing files and no input (purity-selftest=5/5); a hollow core fails core-symbols-no-definitions"
    requirement: API-05
    verification:
      - kind: integration
        ref: "dash tools/gate.sh (purity-selftest=5/5); control ok core-purity-hollow"
        status: pass
    human_judgment: false
  - id: D5
    description: "ctest suite core-symbols checks the exact shipped ivs_core archive of each preset: strict in release, sanitizer prefixes only in dev via IVS_SYMBOL_PREFIX_ARGS"
    requirement: API-05
    verification:
      - kind: integration
        ref: "cmake --build --preset release && ctest --preset release -R core-symbols; build/gate/{dev,release}/core-symbols.txt ok 1 core-symbols.ivs-core-archive"
        status: pass
    human_judgment: false

duration: 14min
completed: 2026-11-10
status: complete
---

# Phase 2 Plan 07: Core Purity (API-05) Summary

**API-05 is machine-enforced: an nm allowlist check on fresh -O0/-O2 core archives and on each preset's shipped ivs_core archive, a comment-aware source scan with the mandatory prelude, a 5-case self-test for LTO/bitcode/hollow/missing/no-input, and 13 deliberate-violation controls.**

## Performance

- **Duration:** about 14 min
- **Started:** 2026-11-10T06:50:12Z
- **Completed:** 2026-11-10T07:03:51Z
- **Tasks:** 3 of 3
- **Files modified:** 50 (697 insertions, 2 deletions)

## Accomplishments

- `tools/check/core-symbols.sh [--allow-prefix P]... FILE...`: undefined (`$NM -P -u`) minus defined (`$NM -P -g --defined-only`) minus allowlist minus prefixes, over the union of the inputs. Names lose `@ver` and, on Darwin, one leading underscore. It fails closed on a missing file, bitcode (raw, wrapper, or as an archive member), a GCC LTO object, an nm failure and an input with zero external definitions. Pass line: `-- core-symbols pass external=N defined=M`.
- The allowlist holds 10 names, each with a one-line reason. The deny list holds 46 names in 7 categories (clock, fs, env, thread, io, rand, alloc). It only labels a failure, as in `-- FAIL core-symbols-forbidden: fopen (fs)`, and fails `core-symbols-deny-in-allow` if a name appears in both lists.
- Gate step `c-purity` runs in three stages:
  1. A source layer that runs before any compile and stops the step on a finding.
  2. A fresh compile of the core at -O0 and -O2, one archive per variant, each checked by core-symbols.sh.
  3. The purity self-test.

  Pass line: `-- c-purity pass core-imports=2 purity-selftest=5/5`.
- 13 controls. 7 symbol-class controls (clock, env, fs, thread, io, rand, alloc) each declare a host function locally so that only the symbol layer can catch it. 5 source-layer controls cover header, asm, float, float-literal and prelude. The hollow control fails with `core-symbols-no-definitions`. The good tree now holds `0x3E4u` and a `S3.2.6, pdf 49` comment and passes.
- ctest suite `core-symbols` runs `tests/core-symbols-tap.sh` on `$<TARGET_FILE:ivs_core>`. Release runs the strict check. Dev passes `--allow-prefix __asan_ __ubsan_ __sanitizer_` through `IVS_SYMBOL_PREFIX_ARGS`, which is set only when `IVS_SAN` is ON. `NM` and `AR` come from `CMAKE_NM` and `CMAKE_AR`. tests-expected goes from 94 to 96.

## Task Commits

1. **Task 1: Tracer: core-symbols.sh, allowlist, c-purity symbol layer and the clock control** - `667daaf` (feat). Tracer gate in auto mode: the gate re-ran green (`steps=24/24 controls=170/170 core-imports=2`), so the next tasks went ahead.
2. **Task 2: Source layer, deny-list categories, purity self-test and the remaining 12 controls** - `e7bc7b5` (feat)
3. **Task 3: core-symbols ctest suite on the shipped archive** - `03c4544` (test)

**Plan metadata:** recorded in the docs(02-07) completion commit.

## Files Created/Modified

- `tools/check/core-symbols.sh`: nm allowlist check, the helper for the step and the ctest suite
- `tools/check/core-symbols-allow.txt`: reviewed allowlist (memcpy, memmove, memset, memcmp, the three `_chk` forms, `__stack_chk_fail`, `__stack_chk_guard`, bzero)
- `tools/check/core-symbols-deny.txt`: category labels for known host names
- `tools/check/steps/c-purity.sh`: step (source layer, symbol layer, self-test)
- `tools/check/controls/core-purity-*/`: 13 controls (CHECK, EXPECT, overlay `core/src/cpu/good.c`; `core-purity-float` adds FORBID `FAIL c-purity-compile:`)
- `tools/check/controls/_good/tree/core/src/cpu/good.c`: hex constant and page cite added
- `data/expected/gate-steps/cpu.ids`: `c-purity`
- `data/expected/controls/core-purity.ids`: inventory of the 13 controls
- `tests/core-symbols-tap.sh`: TAP wrapper for the ctest suite
- `tests/CMakeLists.txt`: `add_test(core-symbols ...)`, `IVS_SYMBOL_PREFIX_ARGS`, ENVIRONMENT `NM` and `AR`
- `data/expected/tests/core-symbols.ids`: `core-symbols.ivs-core-archive`

## Decisions Made

- **What `external=` counts.** It counts the input's real imports: undefined names minus names defined anywhere in the inputs. The -O2 count is reported as `core-imports`; today it is 2 (`__stack_chk_fail`, `__stack_chk_guard`). At -O0 the count is 3, because Apple clang also emits `__memset_chk`.
- **Why core-purity-lto is a self-test case and not an overlay control.** c-purity forces `-fno-lto`, so a source overlay cannot produce an LTO object. The class is still covered by self-test case `lto`, which builds a `-flto` object and an archive of it and requires `core-symbols-lto` or `core-symbols-bitcode`. Apple clang emits the wrapper magic `DE C0 17 0B`. Bitcode files get the same coverage from `bitcode-magic`. D-21 allows dropping a control when its class is still covered.
- **The deny list is optional (D-21).** It has no `-list-missing` token. When it is present, it must be non-empty and disjoint from the allowlist.

## Measured cost (budget input for D-22)

- c-purity step on the real repo: 0.93 s, 1.09 s and 1.17 s (wall; sh, sh, dash). Load average was 11 to 16 during the measurements.
- The 13 core-purity controls plus the positive control take about 11.7 s in an isolated runner. The 7 symbol-class controls and the hollow control cost about 1.0 s each. The 5 source-layer controls cost about 0.03 s each, because they stop before compiling.
- About 70 percent of a symbol-class run is the 6 core-symbols.sh runs: 2 real archives and 4 self-test inputs, at about 0.12 s each, mostly process spawns. Compiles cost about 0.03 s each.
- Full gate: 35.3 s (sh) and 41.4 s (dash) with `steps=24/24 controls=182/182`, against 35.4 s before Task 2 and 71 s for the baseline run under heavier load. The gate is not idle-measured. If the D-22 local budget is hit, the cheapest lever here is to run the self-test once per gate rather than once per control run.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] The source layer tokenizes numbers instead of applying the plan regex after hex stripping**
- **Found during:** Task 2
- **Issue:** Applied to identifiers, the plan regex `[0-9]+[eE][+-]?[0-9]+` flags digits inside a name such as `x1e5`. Stripping hex literals first and then matching would turn a hex float such as `0x1.8p3` into `.8p3`. That still flags it, but for the wrong reason, and the rule would depend on stripping order.
- **Fix:** One awk pass removes comments (with a block-comment state carried across lines), string literals and character literals. It then reads C preprocessing numbers and identifiers. A decimal number with `.` or `e/E` is a float literal, and so is a hex number with `.` or `p/P`. Integer hex constants and identifiers are never flagged.
- **Verification:** A scratch tree with 12 edge cases (`1e5`, `.5`, `0x1.8p3`, `x1e5`, `0x1E5`, `'.'`, `"2.5"`, a multi-line comment with `2.5 double asm`, `// 4.5`, `asmx`, `10u`, `1ULL`) produced exactly the expected findings. The good tree (`0x3E4u`, `pdf 49`) passes.
- **Committed in:** e7bc7b5

**2. [Rule 2 - Missing critical] The float-literal rule also scans core headers; the asm word list is extended**
- **Found during:** Task 2
- **Issue:** The plan applies only the include and asm rules to `.h` files. A literal such as `#define K 1.5` in a header expands into a `.c` file, where the `.c` scan cannot see it. The plan's asm list also omits the GNU spelling `__asm` and other cycle-counter intrinsics.
- **Fix:** Headers also get the float-literal rule. The float and double type words stay `.c`-only, because the prelude names them in its poison pragma. Added `__asm`, `__builtin_ia32_rdtsc` and `__rdtscp` to the asm words.
- **Verification:** The real core headers pass. A scratch header with `#define K 1.5` fails `c-purity-float-literal`.
- **Committed in:** e7bc7b5

**3. [Rule 2 - Missing critical] An empty or malformed `--allow-prefix` is a usage error**
- **Found during:** Task 1
- **Issue:** An empty prefix would allow every name and silently turn the check off.
- **Fix:** `--allow-prefix` needs a non-empty value made of `[A-Za-z0-9_.$]` (the `$` permits D-21's `section$`). Anything else exits 2.
- **Verification:** `sh tools/check/core-symbols.sh --allow-prefix "" x` exits 2.
- **Committed in:** 667daaf

**4. [Rule 3 - Blocking, portability] An nm "no symbols" complaint reads as empty output**
- **Found during:** Task 2
- **Issue:** An object with nothing external, such as the hollow control or the self-test case, makes some nm builds print "no symbols". If such a build also exits nonzero, the input would fail as `core-symbols-nm-failed` instead of `core-symbols-no-definitions`, and the hollow control would fail for the wrong reason. GNU nm behaviour is [ASSUMED] until the first hosted run.
- **Fix:** `run_nm` treats a nonzero nm exit as a failure unless every stderr line is a "no symbols" line. The input still fails closed, as `core-symbols-no-definitions`.
- **Verification:** control ok core-purity-hollow; self-test `no-definitions` passes.
- **Committed in:** e7bc7b5

**5. [Rule 2 - Missing critical] Archive members are extracted to find bitcode inside an archive**
- **Found during:** Task 1
- **Issue:** llvm-nm reads LLVM bitcode members of an archive without complaint. The archive case that D-21 calls for therefore needs a byte check on each member.
- **Fix:** Archives (`!<arch>\n`) are extracted with `$AR x` (default `ar`) into a temp subdirectory, and each member's first four bytes are checked. ctest passes `AR=${CMAKE_AR}` next to `NM=${CMAKE_NM}`.
- **Verification:** Self-test `lto` requires a bitcode or LTO token for both `lto.o` and `lto.a`.
- **Committed in:** 667daaf

**Smaller choices, no behaviour risk:**
- The deny label also applies to a Darwin `$SUFFIX` variant: `fopen$DARWIN_EXTSN` is labelled `(fs)`.
- On a pass, the TAP wrapper also writes the script output as `#` lines (`# -- core-symbols pass external=2 defined=6`).
- `core-symbols` gets `SKIP_RETURN_CODE 77`, per the tests/CMakeLists.txt header rule.
- The EXPECT lines of the header, asm, float-literal and prelude controls include `core/src/cpu/good.c:`. This is stricter than the plan's bare tokens.

---

**Total deviations:** 5 auto-fixed (1 bug, 3 missing critical, 1 blocking), plus 4 smaller choices.
**Impact on plan:** Every token, stat, file and count in the plan holds: `steps=24/24`, `controls=182/182`, `purity-selftest=5/5`, `faults=6/6`, and the core-symbols suite is ok in both presets. The deviations only make the checks stricter or more portable.

## Uncontrolled-token exceptions

- `c-purity-no-sources`: an environment failure (no `core/src/*.c` in the file list), shown by the real gate. Every overlay control keeps the good tree's core source, so an overlay cannot remove it without breaking the premise of the other controls.
- `c-purity-no-cc`: an environment failure (no C compiler on PATH), which an overlay tree cannot arrange; shown by the real gate.

Other tokens with no runner control (for the verifier):
- `core-symbols-deny-in-allow`: checked once by hand with a scratch CHECK_DIR whose allowlist held `malloc`. Output: `-- FAIL core-symbols-deny-in-allow: malloc`, exit 1.
- `core-symbols-list-missing` and `core-symbols-list-empty`: CHECK_DIR is the real `tools/check`, so a control tree cannot reach these lists.
- `core-symbols-nm-failed`: no control.
- `c-purity-compile`: no positive control. The core-purity-float control uses it as a FORBID line.
- `purity-selftest-<case>`: no control. These cases are the self-test's own failure outputs.

## Issues Encountered

- GNU nm's archive header lines and undefined-line shapes have not run yet ([ASSUMED] per RESEARCH Pitfall 2). The parser keys on `$1`, and skips lines with fewer than two fields or whose first field ends in `:`. The same applies to the hosted gcc 13 sanitizer imports: if the dev archive needs another runtime prefix on Linux, add it to `IVS_SYMBOL_PREFIX_ARGS` with a one-line reason, never to the allowlist.
- Ubuntu gcc enables `_FORTIFY_SOURCE` and `-fstack-protector-strong` by default. The `_chk` and `__stack_chk_*` allowlist entries cover both.

## Known Stubs

None.

## User Setup Required

None. No external service configuration is required.

## Next Phase Readiness

- Plan 02-08 can rely on step `c-purity`, its stat keys `core-imports=` and `purity-selftest=`, and the `core-symbols` suite in both presets (tests-expected=96).
- Every later core source must include `common/ivs_prelude.h` first and must not use a floating literal anywhere, including headers. Page cites in comments are fine.
- Deferred per D-21: detecting mutable static data (Phase 3, API-01), MSVC/COFF allowlist entries, and an exported-symbol prefix check.

---
*Phase: 02-cp-1610-cpu-core*
*Completed: 2026-11-10*

## Self-Check: PASSED

All 7 created files and 13 control directories exist; commits 667daaf, e7bc7b5 and 03c4544 are ancestors of HEAD; task acceptance criteria re-run green (allowlist count 10, deny count 46, no-arg exit 2, core-symbols.txt ok in both presets, strict release CTestTestfile, prefixed dev); final gate `dash tools/gate.sh`: gate: PASS steps=24/24 controls=182/182 (41.4 s).
