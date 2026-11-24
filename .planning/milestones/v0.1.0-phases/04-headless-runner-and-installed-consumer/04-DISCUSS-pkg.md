# Phase 4 discuss, decision point D: packaging and the installed consumer (PKG-01, PKG-03 inputs)

Labels: [OBS] fetched or run in this session, [INF] inference, [REC] recommendation, [UNK] unknown.
Method note: all experiments ran on a COPY of the tree in the scratchpad (macOS arm64, Apple clang, CMake 4.4.3, Ninja 1.13;
and ubuntu:24.04 arm64 in docker: gcc 13.3, CMake 3.28.3, dash, GNU binutils). No repo file was changed. Sources: CMake docs
(install, GNUInstallDirs, CMakePackageConfigHelpers, imported targets, `SYSTEM` target property added in 3.25), repo files only
otherwise. No emulator source was opened.

## 1. Facts about the current tree [OBS]

- `core/CMakeLists.txt`: STATIC `ivs_core` built from per-source OBJECT libs via `$<TARGET_OBJECTS:...>`; PUBLIC include dir is the
  absolute source path (`core/include`), which would leak into an export. No `install()` rules exist. Warnings target adds
  `-fvisibility=hidden`; `IVS_API` is empty (`ivs_status.h`).
- Exporting `ivs_core` with `install(TARGETS ... EXPORT)` works as is: the generated `ivsTargets.cmake` mentions only
  `INTERFACE_INCLUDE_DIRECTORIES` (no `ivs_warnings`/`ivs_san`, because those are linked PRIVATE to the OBJECT libs).
- Archive symbols: 46 external definitions on BOTH macOS and Linux (hidden visibility, global binding). 10 are the `IVS_API`
  functions (8 `ivs_machine_*`, `ivs_status_name`, `ivs_api_version`, parsed from the headers); 36 are internals
  (`ivs_sha256_*`, `ivs_cp1610_*`, `ivs_memsys_*`, `ivs_machine_step_unit_internal`...), all `ivs_`-prefixed. All 46 are ELF HIDDEN.
  A static archive cannot keep internals unlinkable; hidden only matters once a shared object is made.
- Linking the same archive into `cc -shared` exports ZERO symbols (nm -D) while `IVS_API` is empty and `-fvisibility=hidden` is on
  [OBS, Linux]. This is the footgun that makes "static only now" the right call and `IVS_API` an unfinished macro.
- Installed prefix contains no build or source path: `grep -a` of the archive and of every installed file for the build root
  finds nothing (release has no `-g`); Targets file computes `_IMPORT_PREFIX` from its own location.
- Relocation spike [OBS, both OSes]: install to prefix A with `cmake --install build/release --prefix A` (0.04 s), `mv A B`,
  build a separate consumer (`find_package(ivs 0.1 CONFIG REQUIRED)` + `target_link_libraries(consumer PRIVATE ivs::core)`)
  from a directory outside the repo with `-DCMAKE_PREFIX_PATH=B`: configure+build+run 0.86 s wall on macOS (compiler detection
  dominates). `ninja -t deps` lists exactly `prefixB/include/ivs/{ivs,ivs_machine,ivs_status}.h` plus system headers.
  `ver=100` printed (0x000100 = 0.1.0).
- PIC [OBS, Linux]: `POSITION_INDEPENDENT_CODE ON` on the objects gives -fPIC, `core-symbols.sh` still passes (external=4
  defined=46), and `cc -shared -Wl,--whole-archive` links.
- The gate already has: in-step selftests (`purity-selftest=n/n`), exact inventories in `data/expected/`, `headers-tap.sh`
  (each public header alone, C17 and C++17), `core-symbols.sh`, `api-effects` (source headers == api-effects.json), a pin-free
  single-job workflow that just runs `sh tools/gate.sh`. Hosted gate budget 40 s, trigger 50 s.

## 2. Lens summary

- Emulator architect: static core, host-independent; the install tree is the product boundary. No libc surface added.
- API/integrator: integrators write `find_package(ivs CONFIG)` + `ivs::core`; libretro-style hosts link a static core into their own
  shared object (needs PIC). Header warnings must be visible to them (see footguns).
- QA: the check must not pass vacuously; each assertion needs a demonstrated failing case (RUN-04 spirit).
- Security/rights: the install tree is the redistributable artifact. An exact file inventory is the rights guard (no data/, refs/,
  fixtures/, system image or manual can ride along); LICENSE travels with the binary (MIT notice requirement).
- DevOps/CI: cost in low seconds, no cache, no artifact upload, POSIX sh/dash, gcc 13 and Apple clang.
- DX: one command (`sh tools/gate.sh`), the consumer is also the copy-paste integration example.
- Product: PKG-01 is "a clean independent consumer"; honest labelling matters more than ceremony.

## 3. Options

### 3.1 How to prove "installed, relocated, independent"
- A. Build the consumer with `-DCMAKE_PREFIX_PATH` only and trust it. Cons: passes vacuously if the compiler finds
  headers elsewhere (`-I core/include` in env CPATH, an in-tree alias, a build-tree registry entry).
- B. Env scrubbing only (`env -i`). Cons: input-side proof, easy to misconfigure, macOS toolchain needs some env.
- C. Output-side proof (REC): after the consumer builds, read what it actually used (`ninja -t deps`, `ninja -t commands`,
  `CMakeCache.txt`, installed-file `grep`) and assert every header and the archive resolve under the moved prefix, nothing under
  `$ROOT/core` or `$ROOT/build`, `ivs_DIR` under the prefix. Plus light env scrubbing as defence in depth. Prove each assertion
  can fail with in-step selftests (section 5). Pros: sound regardless of env, runs on both OSes [OBS ninja -t deps works on both].
  Cons: ninja-specific (gate already requires ninja).
- D. Second container/VM with only the prefix tarball. Strongest; seconds to minutes; use at release time, not per PR.

### 3.2 Static vs shared
- Static only now (REC): PKG-01/RUN-06 need nothing else; CI-EFFICIENCY revisit marker "dylib, rpath" is then moot (no macOS leg).
  Prepare, do not build, shared: keep `IVS_API` macro, PIC on, `STATIC` explicit.
- Static plus shared now: needs `IVS_API` visibility/dllexport macros, a `nm -D` equality check, rpath/install_name handling, a
  macOS leg: more cost than any current consumer justifies.

### 3.3 Symbol check
- Exact inventory of the 36 internals in `data/expected/`: tripwire, but every internal rename edits a list; internals are linkable
  from a static archive anyway, so the list guards nothing a consumer can rely on. Rejected for now.
- REC: (a) installed headers byte-equal to `core/include/ivs` (so the existing `api-effects` step, which ties source headers to
  `data/api/api-effects.json`, transitively covers the installed ones); (b) every `IVS_API` name parsed from the installed headers is
  DEFINED in the installed archive (a header promising a missing function fails); (c) every external definition matches `^ivs_`
  (no namespace pollution; known-bad names already denied by `core-symbols.sh`); (d) the parsed public set equals the call names
  in api-effects.json (belt and braces: the sed parse and the cmake parse must agree). When shared lands, (b)-(d) become exact
  equality against `nm -D`.

## 4. Recommendation (decisions)

### 4.1 CMake install/export (core/CMakeLists.txt, root CMakeLists.txt)
1. Export one target `ivs::core`: `set_target_properties(ivs_core PROPERTIES EXPORT_NAME core)`, `add_library(ivs::core ALIAS ivs_core)` in-tree
   so FetchContent/add_subdirectory users write the same line; internal name `ivs_core` and `ivs_core_internal` unchanged.
2. Include dirs: `$<BUILD_INTERFACE:.../core/include>` and `$<INSTALL_INTERFACE:include>` (never an absolute path in the export;
   do not also use `INCLUDES DESTINATION`: the spike shows a duplicated include entry).
3. Install rules with GNUInstallDirs: `lib/libivs_core.a`, `include/ivs/*.h` (the three public headers only), `lib/cmake/ivs/{ivsConfig,
   ivsConfigVersion,ivsTargets,ivsTargets-<config>}.cmake`, `${CMAKE_INSTALL_DOCDIR}/LICENSE`. Pin `CMAKE_INSTALL_LIBDIR=lib` in the
   base preset so the gate's inventory is layout-exact (Debian/Fedora set lib64 or multiarch only for /usr; a downstream
   packager may override, the consumer is layout-agnostic).
4. Package config: hand-written two-line `ivsConfig.cmake` (`include(CMakeFindDependencyMacro)` is not needed: no dependencies; include
   `ivsTargets.cmake` relative to `CMAKE_CURRENT_LIST_DIR`). Do not use `configure_package_config_file` (no paths to resolve).
   `write_basic_package_version_file(... COMPATIBILITY SameMinorVersion ARCH_INDEPENDENT)`: pre-1.0, so 0.1.x satisfies `0.1`, `0.2` and `1`
   are refused. Never call `export(PACKAGE)` or `export(EXPORT)` (user package registry would let a consumer find the BUILD tree).
5. Version single source of truth: `IVS_API_VERSION_{MAJOR,MINOR,PATCH}` in `ivs.h`; root CMake parses them with `file(STRINGS)`
   before `project(ivs VERSION ... LANGUAGES C)`. The consumer ties three values: `ivs_VERSION` (version file), `IVS_API_VERSION` (header)
   and `ivs_api_version()` (library); any mismatch fails.
6. Relocatability: nothing prefix-dependent at configure time; the gate installs with `cmake --install --prefix` (install-time
   prefix), so `CMAKE_INSTALL_PREFIX` never enters the build tree. `_IMPORT_PREFIX` is derived from the config file location [OBS].
7. Sanitized trees refuse to install (`if(IVS_SAN) install(CODE "message(FATAL_ERROR ...)")`): an ASan archive would need the
   consumer to link `-fsanitize`. The gate installs from the `release` preset only (no extra compile: c-build already built it).
8. `POSITION_INDEPENDENT_CODE ON` on the core objects (hosts that link the archive into a shared plugin, e.g. libretro-style).
   Cost nil [OBS gcc 13 passes core-symbols]. Confirm Apple clang `core-symbols` (the `--forbid-writable-data` caveat applies only to
   c-purity's own `-fno-pie` archives) in plan verification.
9. Small DX options: `IVS_BUILD_TESTS` (default `PROJECT_IS_TOP_LEVEL`) so an `add_subdirectory` consumer does not build tests; `IVS_WERROR`
   already defaults OFF downstream. Optional, one line each.
10. Static only now; `STATIC` stays explicit (BUILD_SHARED_LIBS ignored). Shared reversal trigger: a non-C host that must dlopen
    (NIF, ctypes, plugin ABI). Then define `IVS_API` as visibility default / dllexport under `IVS_BUILDING_SHARED`, add the
    `nm -D` exact-equality mode, macOS leg for install_name/rpath.
11. Not in the install: the runner (`tools/runner`), `ivs_core_internal`, tests, fixtures, docs. Reconsider the runner as an
    `EXCLUDE_FROM_ALL` install COMPONENT `runner` at first release; installing it now creates a CLI contract and an untested path.
    No `.pc` file now (reverse: a non-CMake integrator asks). Do not install `docs/formats/ivsc-v1.md` yet (optional later; header
    comments carry the contract).

### 4.2 The independent consumer
12. Location `tests/installed-consumer/{CMakeLists.txt,main.c}`; NOT reachable from the root CMake (no `add_subdirectory`), so it can
    only build by `find_package`. Its CMake is minimal and idiomatic (it is the integrator example): `cmake_minimum_required(3.25)`,
    `project(... LANGUAGES C)`, `find_package(ivs 0.1 CONFIG REQUIRED)`, link `ivs::core`, `-std=c17 -Wall -Wextra -Wpedantic
    -Wconversion -Wshadow -Werror`, and `NO_SYSTEM_FROM_IMPORTED ON`. [OBS-by-doc] Imported-target include dirs are SYSTEM by default, which
    silences warnings in OUR headers for consumers; the strict flags are meaningless without this property.
13. The step copies the consumer directory to a temp dir OUTSIDE the repo (so `../../core` includes cannot resolve), configures with
    `-G Ninja -DCMAKE_PREFIX_PATH=<moved prefix> -DCMAKE_FIND_USE_PACKAGE_REGISTRY=OFF -DCMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY=OFF`
    after unsetting `CPATH C_INCLUDE_PATH LIBRARY_PATH CMAKE_PREFIX_PATH CMAKE_MODULE_PATH CFLAGS CPPFLAGS LDFLAGS ivs_DIR DYLD_*
    LD_LIBRARY_PATH` in a subshell (POSIX `unset`, not `env -u`), working directory = the temp dir.
14. What it exercises, deterministic stdout compared byte-for-byte with `data/expected/pkg-consumer.golden` (comment header states tier and
    oracle): create via `ivs_machine_storage_requirements` + caller storage; load project boot stub (EXEC slot) + project-authored
    diagnostic cartridge; a refusal (duplicate load -> `IVS_E_DUPLICATE_LOAD`, truncated EXEC -> `IVS_E_MALFORMED`) with the state digest
    asserted equal before and after; reset; bounded run to an absolute machine-time target (name per decision A/API-02); a second fresh
    instance repeats and the digests must match; per-domain digests printed. The consumer prints the version tie. Content bytes come as argv paths
    that the step produces from the project's own text sources with the in-tree packer (no committed rights-bearing bytes;
    consumer itself embeds none). Golden label: regression pin with the emulator as oracle ("self-consistent, not independent");
    the independently derived predicate lives in RUN-06 and the consumer golden digest lines are required to EQUAL the headless
    runner's golden for the same scenario (cross-path agreement: public API via runner vs installed archive).
15. `data/expected/pkg-consumer-calls.ids`: the set of `ivs_*` names the consumer object references (`nm -u` on `main.c.o`) must equal the
    listed set; it must include create, load, reset, the run function, state_digest. Stops "run" or "digest" being quietly dropped.

### 4.3 Gate step
16. One new step `pkg-install` (`tools/check/steps/pkg-install.sh`, id in `data/expected/gate-steps/pkg.ids`, sorts after `c-*`):
    1) `cmake --build --preset release` (no-op); 2) install to `$T/stage/prefix-a`; 3) move to `$T/reloc pfx/prefix-b` (note the space) and DELETE
    `prefix-a`; 4) exact installed-file inventory vs `data/expected/pkg-install-files.ids`; 5) installed headers `cmp` equal to
    `core/include/ivs`, installed archive sha256 equal to the build-tree archive; 6) grep for `$ROOT/core` and `$ROOT/build` in all installed
    files and in the archive (`grep -aF`), none; 7) symbol rules (3.3); 8) each installed header alone: `${CC} -std=c17 -pedantic -Wall
    -Wextra -Werror -fsyntax-only -I prefix/include` and `${CXX}` C++17 with a visible SKIP when no C++ compiler (same rule as
    `headers-tap.sh`; reuse by giving it an optional include-dir argument rather than a copy); 9) consumer configure, build, run, golden
    compare; 10) leak assertions on the consumer build (4.4); 11) a direct `c++` compile and link of a tiny `main.cpp` against the
    installed archive (catches missing `extern "C"` at link level; visible skip without a C++ compiler); 12) selftests (section 5).
    Pass line: `-- pkg-install pass prefix-files=<n> public-symbols=<n> defined=<n> consumer-lines=<n> hdr-alone=<n>/<n> pkg-selftest=<n>/<n>`.
17. Artifacts: everything under `$T` (mktemp, removed by trap, outside the repo); only the consumer stdout is written to `build/gate/pkg-consumer.txt`
    for diagnosis. Nothing cached or uploaded (PKG-03); no rights-bearing bytes exist in the prefix (inventory).
18. Cost [INF from OBS segments]: install 0.04 s, consumer 0.9 to 1.3 s (configure compiler detection), symbol and header checks 0.2 s, selftests
    about 0.5 s (direct `cc` invocations, `LANGUAGES NONE` configures): 2 to 3 s total, inside the 40 s hosted budget. Record the step time in
    CI-EFFICIENCY.md (PKG-04 already prints `step-time pkg-install`). No workflow change: ci-parity untouched.
19. cmake floor: keep 3.25 declared in root, presets and consumer. Everything used is at or below 3.25 (`cmake_path` 3.20, `SYSTEM` property
    3.25, `cmake --install --prefix` 3.15). Tested: 3.28.3 (hosted image) and 4.4.3 (local). 3.25 itself is untested (same policy as Phase 2 D-19);
    do not add a CMake-download dependency to test it.
20. Toolchain differences handled: Darwin leading underscore (reuse `core-symbols.sh`'s `norm`), `nm -P -g --defined-only` shape on GNU and BSD,
    ELF HIDDEN vs Mach-O private extern (both appear under `-g` [OBS]), PIE default on Ubuntu (objects PIC anyway), `_GLOBAL_OFFSET_TABLE_`
    already allowlisted, `/var` vs `/private/var` symlinks (compare with `pwd -P`), dash (no `time (`, no `local`, no `[[`).

### 4.4 Meaning of "clean and independent" (all must hold)
- The prefix equals the inventory exactly; nothing else under it; no `$ROOT/core|build` string in any installed file or the archive.
- The consumer source dir is a copy outside the repo; `CMakeCache.txt` `ivs_DIR` starts with the moved prefix (`pwd -P` forms).
- `ninja -t deps` of every consumer object: all headers are the copy, system dirs, or `<prefix>/include/ivs/{ivs,ivs_machine,ivs_status}.h`
  (positive: ivs.h and both siblings present, so the check cannot pass with an empty list); none under `$ROOT/`. `ninja -t commands` link line
  names `<prefix>/lib/libivs_core.a` and no other `ivs_core` archive.
- The original prefix directory no longer exists when the consumer builds.
- Version tie holds (version file, header, library).
- What it does NOT prove [REC to state in the doc]: same compiler and machine as the build (independence of build graph and paths,
  not of toolchain or author); shared linking; 3.25 floor. Release time adds method D (consumer against the release tarball).

## 5. Selftests inside the step (named-fault style; each must fail for its intended reason)
| id | mutation | expected token |
|----|----------|----------------|
| pkg-selftest-core-src-include | consumer TU `#include "common/ivs_sha256.h"` compiled with only `-I prefix/include` | compile fails, `file not found` |
| pkg-selftest-leak-detector | same TU compiled with `-I $ROOT/core/src -MD`; compiles, but the dependency-path checker must report `pkg-leak` | detector flags it (proves the detector, not only the compiler) |
| pkg-selftest-missing-symbol | `ar d` one member of a COPY of the archive (e.g. `machine_load.c.o`) and run the symbol rule | `pkg-symbol-missing: ivs_machine_load` |
| pkg-selftest-ghost-decl | COPY of headers with an extra `IVS_API int ivs_ghost(void);` | `pkg-symbol-missing: ivs_ghost` |
| pkg-selftest-unprefixed | tiny archive defining `sha256_init` | `pkg-symbol-namespace: sha256_init` |
| pkg-selftest-extra-file | COPY of prefix plus a stray `share/ivs/rom.bin` | `pkg-inventory-extra` |
| pkg-selftest-baked-path | COPY of `ivsTargets.cmake` with `$ROOT/build` appended | `pkg-path-leak` |
| pkg-selftest-version-refused | `find_package(ivs 0.2 CONFIG)` and `find_package(ivs 1 CONFIG)` in a `LANGUAGES NONE` project | configure fails (both) |
| pkg-selftest-old-prefix | consumer configure with `CMAKE_PREFIX_PATH` set to the deleted prefix-a | configure fails (proves relocation matters) |
| pkg-selftest-golden-drift | consumer output with one digest nibble changed vs golden | `pkg-golden-diff` |
Cost about 0.5 s total; stat `pkg-selftest=<ok>/<total>`, a missing case counts as a failure (like purity-selftest). Control-runner controls
are NOT used here: they copy a tree and re-run a step, and this step needs a configure+build+install, so 10 runner controls would cost
30 s or more [INF].

## 6. PKG-03 / PKG-04 inputs
- Fast gate stays one command and one job (D-24 holds). Because libraries are static, no macOS CI leg is justified (the CI-EFFICIENCY revisit
  marker "dylib, rpath" does not fire). `tools/gate-linux.sh` (gcc 13, dash, cmake 3.28) remains the pre-push mirror.
- Skips are visible: C++ cases print `SKIP no C++ compiler` and the count appears in the pass line.
- Nothing cached/uploaded; the step writes only under `$T` and `build/gate/`. The inventory file doubles as the "no rights-bearing bytes in the
  distributable" proof.
- Append the measured hosted step time to CI-EFFICIENCY.md after the first hosted run; trigger for a lever is the existing 50 s.

## 7. Adversarial pass
- "A consumer in the same repo, toolchain and machine is not independent." True. Mitigated by output-side leak proof, relocation with deleted
  origin, an out-of-repo source copy, and the labelled limit. Real independence arrives with a release tarball test (method D).
- "Internals are link-visible in the static archive; a host defining `ivs_sha256_init` would collide." True [OBS: 36 internals]. Mitigation now:
  namespace rule plus no declaration in installed headers; later: shared build hides them, or a symbol-prefix rename. Accepted risk, low.
- "The sed parse of `IVS_API` declarations is brittle (multi-line prototype)." Mitigated by requiring equality with the api-effects.json call
  names and a nonzero count; a header style change fails loudly rather than silently shrinking the set.
- "ninja-only proof." The gate already requires ninja (`gate-no-ninja`); if the generator ever changes, the dependency-path source changes
  to compiler `-MD` depfiles (the leak checker takes a plain path list, so only the extractor changes).
- "Golden digests pinned from the emulator are circular." Yes; labelled as regression pins. Independence is RUN-06's predicate and the cross-path
  equality with the runner golden.
- "Per-config Targets files": a Debug and a Release install into one prefix both work (file glob in Targets); the gate installs one config.
- "Windows": untested, MSVC deferred by project stack; `IVS_API` dllexport is part of the shared trigger.

## 8. Risks and reversal
Risks: (1) the consumer golden churns with every Phase 5 device change (mitigate: it shares the runner golden, one edit);
(2) step cost drifts above 3 s if more consumer variants are added (budget it, no cache); (3) a static-only package gives false confidence for a
future shared build (explicit trigger in 4.1 item 10).
Reverse if: a non-C or plugin host appears (go shared, add export macros and a macOS leg); a non-CMake integrator appears (add `ivs.pc`); the gate
step measures above 5 s hosted (drop selftest granularity or move the C++ link and per-header checks to local-only); a packager needs lib64 layout
(drop the `CMAKE_INSTALL_LIBDIR` pin and compute libdir in the inventory).

## 9. Unknowns
- [UNK] Exact public names of the run function and buffer/outputs API (decisions A/B/API-02/04); the consumer and `pkg-consumer-calls.ids` depend on
  them. Not blocking the install/export plan; blocking only the consumer's body.
- [UNK] Apple clang `core-symbols.sh` on a PIC build (gcc 13 verified); verify in the first plan.
- [UNK] CMake 3.25 behaviour itself (floor untested).
- [UNK] D6 (controller representation) will extend the public headers; no packaging impact beyond the inventory staying exact.
