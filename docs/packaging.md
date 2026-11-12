# The installed package (`ivs::core`)

The machine core installs as a static CMake package named `ivs` with one imported target, `ivs::core` (PKG-01, D-33).
A host builds against the installed headers and archive alone; it needs neither this repository nor its build tree.
The gate step `pkg-install` installs the package, moves it, builds an independent consumer project against the moved copy and compares that consumer's output with the headless runner's golden for the same scenario (plans 04-22 and 04-23).

## Install

Build the `release` preset, then install that tree to a prefix of your choice:

```
cmake --preset release
cmake --build --preset release
cmake --install build/release --prefix <dir>
```

Install from the `release` tree only.
A sanitized tree (the `dev` preset, `IVS_SAN`) refuses to install, because its archive would need the sanitizer runtimes at the consumer's link (D-34).

## Installed layout

The prefix holds exactly these files (the list is pinned by `data/expected/pkg-install-files.ids`):

| Path | Content |
|---|---|
| `include/ivs/ivs.h` | the umbrella header a host includes; the API version macros |
| `include/ivs/ivs_machine.h` | sizing, create, destroy, info, load, unload, reset, state digest |
| `include/ivs/ivs_run.h` | bounded run, input events, CPU state, peek |
| `include/ivs/ivs_status.h` | status codes, `IVS_API` |
| `lib/libivs_core.a` | the static library |
| `lib/cmake/ivs/ivsConfig.cmake` | the package config (hand-written, no paths) |
| `lib/cmake/ivs/ivsConfigVersion.cmake` | the version rule below |
| `lib/cmake/ivs/ivsTargets.cmake`, `ivsTargets-release.cmake` | the exported target |
| `share/doc/ivs/LICENSE` | the licence |

Nothing else is installed: no internal header, no runner, no image, no `pkg-config` file and no package registry entry.
The exported include directory is `${_IMPORT_PREFIX}/include`, computed from where the config file lies, so the prefix can be moved after installation.

## Consume

```
cmake_minimum_required(VERSION 3.25)
project(my_host LANGUAGES C)
find_package(ivs 0.1 CONFIG REQUIRED)
add_executable(my_host main.c)
target_link_libraries(my_host PRIVATE ivs::core)
```

Configure with `-DCMAKE_PREFIX_PATH=<dir>`.
Include `<ivs/ivs.h>`; it brings in the other three headers.
`tests/installed-consumer/` is a complete example: it sizes and creates an instance in storage it owns, loads the boot stub, a zero GROM and the diagnostic cartridge from files named on its command line, shows two refusals, resets, makes two bounded run calls with two input events and prints the per-domain digests; since plan 05-11 it takes a third image, the second diagnostic, and replays its seven controller events through the same headers with audio and write-log buffers sized from `ivs_info`, printing the peeked `$01FE` after the first event and `ctrl.digest.<domain>` lines.

Static only (D-35): the package has no shared library, and `IVS_API` expands to nothing.
A shared build would export no symbol until `IVS_API` is defined (visibility or `dllexport`) before any `ivs/` header is included; that is a later decision, not a build switch.

## Version rule

The version has one source: the `IVS_API_VERSION_MAJOR`, `_MINOR` and `_PATCH` macros of `ivs.h`, which the root `CMakeLists.txt` parses before `project()` (D-34).
The config version file uses `SameMinorVersion`: `find_package(ivs 0.1)` accepts any 0.1.x and refuses 0.0, 0.2 and 1.
Before 1.0 a minor step may change the API; struct sizes and the append-only status and reason numbers let additions land without a break.

Three values must agree, and the consumer prints all three on its `version` line:

- `ivs_VERSION`, the version the package config reports to CMake;
- `IVS_API_VERSION`, the packed version of the headers the host compiled against (`major << 16 | minor << 8 | patch`);
- `ivs_api_version()`, the packed version the archive was built with.

A host that compares `ivs_api_version()` with `IVS_API_VERSION` at start-up detects a header and library mismatch.

## What the gate step checks

`pkg-install` (in `tools/check/steps/pkg-install.sh`) installs `build/release` into a temporary prefix, moves the prefix into a path that contains a space and deletes the original, then:

1. the moved prefix equals the pinned file list, its headers equal `core/include/ivs`, its archive equals the build tree's archive, and no installed file contains a repository path;
2. every `IVS_API` name of the installed headers is defined in the archive, the set equals the call list of `data/api/api-effects.json`, and every external definition starts with `ivs_`; each installed header compiles alone as C17 and as C++17;
3. the consumer in `tests/installed-consumer/` is copied outside the repository and configured with only the moved prefix (package registries off, path variables unset), built with strict C17 flags (`-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`, with the package headers not treated as system headers) and run on `build/release/images/boot-stub.exec`, `diag-cpu-1.ivsc` and `diag-ctrl-psg-1.ivsc`;
4. its output equals `data/expected/pkg-consumer.golden` (a regression pin; the emulator is the oracle), and its `digest.<domain>` lines equal those of `data/golden/diag-input-pass.digests`, which the headless runner produced for the same scenario through the build tree; its `ctrl.digest.<domain>` lines and observed domains (audio and the write log included) equal those of `data/golden/diag-ctrl-psg-pass.digests` in the same way [claim:CL-0015];
5. the consumer's build records show where its inputs came from: `ninja -t deps` lists `ivs.h`, `ivs_machine.h`, `ivs_run.h` and `ivs_status.h` under the moved prefix and nothing under the repository; the link line names `<prefix>/lib/libivs_core.a` and no other copy of the library; `ivs_DIR` lies under the moved prefix; the `ivs_*` names the consumer object references equal `data/expected/pkg-consumer-calls.ids`;
6. a small C++17 program (`tests/installed-consumer/main.cpp`) compiles and links directly against the installed headers and archive, so a missing `extern "C"` would show at link time (a visible skip without a C++ compiler);
7. thirteen in-step selftests (`data/expected/pkg-selftests.ids`) mutate copies and require each check to fail for its intended reason, among them a consumer build that adds the repository's `core/src` include directory (the dependency check must report `pkg-leak`) and a consumer output with one digest nibble changed (`pkg-golden-diff`); three more concern the second replay: a changed `ctrl.digest` nibble, a dropped `ctrl.digest.psglog` line (`pkg-runner-golden-diff`) and a changed peeked `$01FE` (`pkg-golden-diff`).

Only the consumer's stdout is kept, as `build/gate/pkg-consumer.txt`; everything else lives in a temporary directory outside the repository and is removed when the step ends.

## What the gate step does not prove

- Independence of the build graph and of paths, not of the toolchain or the author: the consumer is compiled by the same compiler, on the same machine and from the same hands as the package.
  A package built on one host and consumed on another is not exercised; a consumer check against a published release archive belongs to release packaging, which is outside this phase.
- Agreement of two paths through the same core, not correctness: the consumer and the runner compute the same digests through the same library, so a core defect would appear in both.
  The digests are self-recorded repeatability values; independent expected values live in the scenario assertions and the CPU vectors.
- Static linking only: no shared library is built or tested.
- The CMake floor: 3.25 is the declared minimum of the project, its presets and the consumer, but only 3.28.3 (the hosted Ubuntu image) and 4.4.3 (the local machine) are exercised (D-35).
- Platforms: the consumer is built on macOS with Apple clang and on Ubuntu with GCC, both 64-bit; the frozen size of the run request (64 bytes) holds only where pointers are 64 bits wide, and the consumer prints a visible skip for that size elsewhere.
