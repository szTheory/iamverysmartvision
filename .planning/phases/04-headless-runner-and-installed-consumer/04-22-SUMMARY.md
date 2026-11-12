---
phase: 04-headless-runner-and-installed-consumer
plan: 22
subsystem: core CMake, cmake, tools/check, tests, data/expected
tags: [packaging, install, export, find_package, relocation, symbols, selftest, PKG-01, D-33, D-34, D-35, D-37, D-38, D-40, A-07]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-21: gate at steps=31/31 controls=273/273 faults=149/149; c-build builds the dev and release presets; tests/headers-tap.sh; tools/check/core-symbols.sh (norm, allowlist with _GLOBAL_OFFSET_TABLE_); data/api/api-effects.json call list"
provides:
  - "Installable static package: ivs::core (EXPORT_NAME core, in-tree ALIAS), include dirs $<BUILD_INTERFACE:core/include> / $<INSTALL_INTERFACE:include>, PIC core objects, hand-written cmake/ivsConfig.cmake, SameMinorVersion ARCH_INDEPENDENT version file, LICENSE in share/doc/ivs; a sanitized tree refuses to install"
  - "Version parsed from the IVS_API_VERSION_* macros of ivs.h before project() (project VERSION 0.1.0)"
  - "CMAKE_INSTALL_LIBDIR=lib in the base preset"
  - "Gate step pkg-install (tools/check/steps/pkg-install.sh): install build/release, move into a path with a space, delete the original, then inventory, content, path-leak, symbol, header-alone checks and eight in-step selftests"
  - "data/expected/pkg-install-files.ids (10 paths), data/expected/pkg-selftests.ids (8 cases), data/expected/gate-steps/pkg.ids"
  - "tests/headers-tap.sh takes an optional include-dir argument (ctest registration and ids unchanged)"
affects: [04-23, 04-24, 04-25]

actuals:
  tokens: 7600
  tasks: 3
  commits: 4
plan_head_before: a8835a0
plan_head_after: 738105a6b97b1e30fad68cf929fc9e78d365d322

tech-stack:
  added: []
  patterns:
    - "Step check functions print their own FAIL lines and return a status, so the main path and the in-step selftests call the same code (check_inventory, check_paths, check_symbols)"
    - "Each selftest that expects a failure also has a positive half where one is meaningful (core/src header compiles with core/src added; the built major.minor is found under the moved prefix with ivs_DIR inside it)"
    - "nm runs on a relative archive name and member headers are skipped by whole line, because GNU nm prints `<archive>[<member>]:`, which splits into fields under a path with a space"

key-files:
  created:
    - cmake/ivsConfig.cmake
    - tools/check/steps/pkg-install.sh
    - data/expected/gate-steps/pkg.ids
    - data/expected/pkg-install-files.ids
    - data/expected/pkg-selftests.ids
  modified:
    - CMakeLists.txt
    - core/CMakeLists.txt
    - CMakePresets.json
    - tests/headers-tap.sh

key-decisions:
  - "Install destinations use GNUInstallDirs variables (LIBDIR pinned to lib by the preset, INCLUDEDIR include, DOCDIR share/doc/ivs); the INSTALL_INTERFACE include dir is ${CMAKE_INSTALL_INCLUDEDIR}"
  - "Archive equality: whole-file SHA-256 on Linux (container: archive=sha256). On Darwin, CMake re-runs ranlib on an installed static library and restamps the index member header, so a hash difference falls back to a member-wise proof (same ar t list, every member byte-equal, the index included); the pass line says which held (archive=sha256|members)"
  - "version-refused also probes an older minor (0.0 for 0.1.0) when one exists: AnyNewerVersion would accept it, SameMinorVersion must not; the two D-40 refusals (0.2 and 1) are derived from the parsed version"
  - "Probes are LANGUAGES NONE Ninja configures with find_package(... CONFIG REQUIRED) and every default search path off except CMAKE_PREFIX_PATH, path env vars unset in a subshell"
  - "PKG-01 stays Pending: the structure is proved here, the independent consumer (load, refusal, run, digest) is plan 04-23"

duration: about 15 min of edits plus gates (first task commit 20:03, last 20:16), plus two container gates
completed: 2026-11-11
---

# Phase 4 Plan 22: install/export and the pkg-install step Summary

**The core now installs as a static `ivs::core` CMake package, with its version read from `ivs.h`. A new gate step `pkg-install` installs the release tree that `c-build` already built, moves the prefix into a path with a space and deletes the original. It then checks the moved prefix: the file list matches the 10-path inventory exactly, headers and archive are byte-equal to the build, no repo path appears in any file, the symbol rules hold, and each installed header compiles alone as C17 and C++17. Eight in-step selftests show that each check fails for its intended reason.**

## What was built

- Task 1 (tracer, `5623b0b`): root version parse and `include(GNUInstallDirs)`; `core/CMakeLists.txt` PIC objects, `EXPORT_NAME core`, `ivs::core` ALIAS, generator-expression include dirs, install/export rules, version file, LICENSE, IVS_SAN refusal; `cmake/ivsConfig.cmake`; preset pin; step part 1 (install, relocate, delete, inventory); `pkg.ids`, `pkg-install-files.ids`.
- Task 2 (`3a0bb93`): header `cmp`, archive hash, path-leak grep (logical and `pwd -P` ROOT), symbol rules against `api-effects.json` (temp `cmake -P` script, `string(JSON)`, floor 3.25), `core-symbols.sh` on the installed archive, `headers-tap.sh` include-dir argument and the header-alone check.
- Task 3 (`1541e2c`): eight selftests and their inventory; the GNU nm member-header fix found by the container gate.
- `738105a`: comment reworded so the plan's `export(PACKAGE` grep counts 0 (no behaviour change).

| Selftest | Mutation (on a copy) | Must show |
|---|---|---|
| core-src-include | TU `#include "common/ivs_sha256.h"`, only the prefix include dir | compile fails naming `ivs_sha256.h`; compiles once `core/src` is added |
| missing-symbol | `ar d` `machine_load.c.o` | `pkg-symbol-missing: ivs_machine_load` |
| ghost-decl | extra `IVS_API int ivs_ghost(void);` in a copied ivs.h | `pkg-symbol-missing: ivs_ghost` |
| unprefixed | `sha256_init` object added with `ar r` | `pkg-symbol-namespace: sha256_init` |
| extra-file | `share/ivs/stray.txt` in a copied prefix | `pkg-inventory-extra: share/ivs/stray.txt` |
| baked-path | `# $ROOT/build/release` appended to a copied ivsTargets.cmake | `pkg-path-leak: lib/cmake/ivs/ivsTargets.cmake` |
| version-refused | find_package 0.2, 1 and 0.0 | all fail naming 0.1.0; 0.1 found, version 0.1.0, ivs_DIR under the moved prefix |
| old-prefix | CMAKE_PREFIX_PATH = the deleted prefix-a | configure fails with the not-found message |

Meta-check (scratch copies of the step, not committed): with the inventory-extra, path-grep, namespace and missing-symbol rules disabled, and with the old prefix recreated, the matching selftests fail (`inventory passed`, `path check passed`, `symbol rules passed`, `found a package under the deleted prefix`). An unknown name in the inventory gives `pkg-selftest-missing: bogus-case`. The sanitized dev tree refuses: `cmake --install build/dev` exits 1 with `ivs: refusing to install a sanitized (IVS_SAN) tree`.

## Performance

- Local (macOS arm64, loaded laptop): `step-time pkg-install 1`. A standalone run takes 0.95-1.01 s wall: install, 4 LANGUAGES NONE configures, 8 header compiles and 3 small cc/ar runs.
- Container (`tools/gate-linux.sh`, amd64 emulated on arm64): `step-time pkg-install 4`. Hosted native x86_64 should be close to the local figure. Record the hosted value after the first hosted run (PKG-04 / CI-EFFICIENCY).
- Gate total 25-27 s locally, as before this plan (the step overlaps nothing, so it adds about 1 s).

## Counts

Start: `steps=31/31 controls=273/273 faults=149/149 tests-expected=2776 tests-skipped=2`.
End: `steps=32/32 controls=273/273 faults=149/149 tests-expected=2776 tests-executed=2774 tests-skipped=2 tests-missing=0`. Step stats: `prefix-files=10 archive=members public-symbols=14 defined=55 hdr-alone=8/8 pkg-selftest=8/8` (Linux: `archive=sha256`, other values the same).

Acceptance checks: `grep -c CMAKE_INSTALL_LIBDIR CMakePresets.json` = 1; the `export(PACKAGE` count over both CMakeLists = 0; `grep -v '^#' data/expected/pkg-selftests.ids | grep -c .` = 8; `sh tests/headers-tap.sh "$PWD" "$PWD/core/include" | tail -1` = `# suite=headers executed=8 skipped=0 failed=0` (same as without the argument).

## Deviations from Plan

**1. [Rule 1 - Bug] Archive hash on Darwin.** The plan requires the installed archive's SHA-256 to equal the build tree's. On macOS, CMake's install script re-runs `/usr/bin/ranlib` on the installed archive. The member contents stay identical, the symbol index included, but the header timestamp changes. On Darwin only, the step therefore falls back to a member-wise proof, and the pass line shows `archive=members`. Linux keeps the whole-file hash (`archive=sha256` in the container).

**2. [Rule 1 - Bug] GNU nm member headers under a path with a space.** The first container gate failed with `pkg-symbol-namespace: /tmp/pkg-install.../reloc`. GNU `nm -P` prints `<archive>[<member>]:` member headers, and the space in `reloc pfx` split that header into fields. The step now runs nm (and `core-symbols.sh`) on a relative archive name and skips any line that ends in `:`. `core-symbols.sh` was not changed: it has the same latent split, but no caller passes it a path with a space.

**3. [Rule 2] Extra probe in version-refused.** An older minor (0.0) must also be refused, so the case tells SameMinorVersion apart from AnyNewerVersion. A positive find_package of 0.1 must also succeed with ivs_DIR under the moved prefix, so the refusals cannot pass for an unrelated reason.

**4. Temp dir canonicalised** (`T=$(cd "$T" && pwd)`): a TMPDIR ending in `/` gave `//`, which CMake normalises, so the ivs_DIR comparison failed until the path was canonical.

**5. No `cmake --build --preset release` no-op inside the step** (DISCUSS 4.3 item 1). The gate runs steps in sorted order, so `c-build` always builds the release tree first. A standalone run of the step against a stale tree would install stale objects. In the gate this cannot happen.

## Open Items

- Plan 04-23 (consumer): the moved prefix is `"$T/reloc pfx/prefix-b"` and the deleted original is `$T/stage/prefix-a`. Add the consumer build, the `ninja -t deps` and link-line leak checks (D-37 d, e), the direct C++ link (D-38), the version tie (`ivs_VERSION`, `IVS_API_VERSION`, `ivs_api_version()`), and the two remaining D-40 selftests (leak-detector, golden-drift). Extend the pass line with `consumer-lines=` and append both cases to `data/expected/pkg-selftests.ids`. Keep only `build/gate/pkg-consumer.txt`. Pin the v1 struct sizes from the installed headers (04-13 item).
- `docs/packaging.md` (if added in 04-23) must follow the text, glossary and denylist rules. It should state what the step does not prove: same toolchain and machine, no shared build, 3.25 floor untested.
- PKG-01 stays Pending until the consumer passes. The hosted `step-time pkg-install` goes to CI-EFFICIENCY.md after the first hosted run.
- Carried: C14 and C63-C68 revisited in Phase 5/6/7; `API-02`, `PKG-01`, `PKG-03`, `PKG-04`, RUN-04 Pending; `pending-resolvers=17`, `unknown-fields=16`. `sh tools/hygiene.sh --history` still fails on earlier planning commits (this plan adds no home paths). The orchestrator records hosted runs with `sh tools/ci-record.sh <run-id>`. Important-gates (04-24) may cite the pkg selftests and register the seven runner faults of 04-18/04-19.

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=32/32 controls=273/273 ... cc=AppleClang c-presets=2 fault-runs=149 core-imports=4 purity-selftest=7/7 ... faults=149/149 ... prefix-files=10 archive=members public-symbols=14 defined=55 hdr-alone=8/8 pkg-selftest=8/8 ... tests-expected=2776 tests-executed=2774 tests-skipped=2 tests-missing=0 ...`, `gate-time total=25`.
- `dash tools/gate.sh`: the same line, `gate: PASS`, `gate-time total=25`.
- `sh tools/gate-linux.sh` (Ubuntu 24.04 container, x86_64, gcc 13.3.0, cmake 3.28.3): `gate: PASS steps=32/32 controls=273/273 ... cc=GNU c-presets=2 fault-runs=149 core-imports=3 ... faults=149/149 ... prefix-files=10 archive=sha256 public-symbols=14 defined=55 hdr-alone=8/8 pkg-selftest=8/8 ... tests-expected=2776 tests-executed=2774 tests-skipped=2 tests-missing=0`, `gate-time total=106`, `gate-linux: exit 0`.

## Threat Flags

None. The step writes only under its mktemp dir and opens no network path. The installed prefix is fixed by the inventory.

Mitigations from the plan's threat model:
- T-04-66: exact inventory; `extra-file` selftest.
- T-04-67: generator-expression include dirs; `pkg-path-leak`; `baked-path` selftest.
- T-04-68: symbol rules against the headers and the effect table; `missing-symbol`, `ghost-decl`, `unprefixed` selftests.
- T-04-69: `SameMinorVersion`; `version-refused` selftest (next minor, next major, older minor).
- T-04-SC: no package installs.

## Self-Check: PASSED

- Files exist: cmake/ivsConfig.cmake, tools/check/steps/pkg-install.sh, data/expected/gate-steps/pkg.ids, data/expected/pkg-install-files.ids, data/expected/pkg-selftests.ids.
- Commits on HEAD: 5623b0b, 3a0bb93, 1541e2c, 738105a.
