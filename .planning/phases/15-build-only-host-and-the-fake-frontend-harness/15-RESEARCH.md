# Phase 15: Build-Only Host and the Fake-Frontend Harness - Research

**Researched:** 2026-11-26
**Domain:** a build-only shared object over an existing static C core, plus a `dlopen` frontend whose evidence is nine digest lines
**Confidence:** HIGH on every mechanism (measured this session on both linkers), MEDIUM on three design decisions the frozen documents leave open (see Open Questions)

## Summary

The phase is a *seam*, not a feature: nothing new is computed. The core already takes a
tick target, already publishes frames and level samples, already answers nine state digests,
and the runner already produces exactly those nine lines. The work is (a) one shared target
that adds no palette and no file I/O, (b) one frontend that `dlopen`s it and drives it, and
(c) the packaging rows that keep the artifact out of the install list.

Three things were measured this session rather than assumed, and each one changes the plan:

1. **A `dlopen`-shaped host reproduces the golden exactly.** A throwaway probe (never
   committed) created a machine from `boot-stub-2.exec` + 2048 zero bytes + `lamp-tone.ivsc`,
   passed the six scenario events with their absolute `at_tick`s, and printed the nine
   digests. All nine were byte-equal to `data/golden/demo-lamp-tone-pass.digests` under
   **three different call slicings**: one call to 806568, four calls at the scenario's
   `run-to` bounds (which also reproduces the golden's `step.<k>.now_ticks` 200024 / 440012 /
   640004 / 806568), and one frame per call (13 frame calls + a final call to 806568).
2. **The nine lines are a function of (the six events' `at_tick`s, the final machine tick).**
   Stopping at the 13th frame end (776600) instead of 806568 moves **seven** lines
   (`cpu mem instance composite audio input psglog`; `video` and `images` stay equal). The
   reason is a documented field: domain 18 serializes `obs.input.floor_tick`
   (`core/src/machine/machine_digest.c:163`), so `digest.input` — and through the composite,
   nothing else — depends on where the run stopped, not only on the applied events. **A
   frame-only drive cannot hit 806568**, so the shim's `run` must carry a tick target.
3. **The export surface is enforced by the linker, not by convention.** Measured on Darwin
   (`-Wl,-exported_symbols_list`) and on Ubuntu 24.04 (`-Wl,--version-script`), with
   default-visibility objects (which is what the core archive is): `nm -P -g --defined-only`
   reports **exactly** the listed names on both, and the same `norm()` recipe the `pkg-install`
   step already uses reads both. Apple's linker *errors* when a listed name is not defined;
   GNU's silently accepts it, so the check needs a missing-name direction as well.

**Primary recommendation:** `host/` holds one TU (the nine shaped entries + the pixel
conversion), `host/libretro_shim.h` states the subset, and `host/CMakeLists.txt` builds
`ivs_retro_shim` as a `SHARED` library with no install rule, `target_compile_definitions(…
IVS_API=__attribute__((visibility("default"))))`, `runner/frame_export.c` compiled in for
`ivs_frameppm_palette` only, and an export list **generated from
`data/expected/host-symbols.ids`**. The fake frontend is `tests/host/ivs_host_harness.c`,
built in `tests/CMakeLists.txt`, linked against `ivs_san`/`ivs_warnings` and
`runner/frame_export.c` but **not** `ivs_core`, and passed to the existing `runner-process`
suite as a seventh `$<TARGET_FILE:…>` argument. Every digest assertion reuses
`check_one_golden`'s two rules verbatim.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|---|---|---|---|
| The nine state digests | Core (`ivs_machine_state_digest`) | — | the core is the only producer; the host forwards, never recomputes |
| Input schedule (six `at_tick`/word pairs) | Host (fake frontend) | Core (applies at unit boundaries) | the core keeps no queue and takes no setter; the host owns the script |
| Frame delivery | Core (slot bytes) | Host (index→RGB through `ivs_frameppm_palette`) | the core publishes indices; the palette is operator-side (`runner/frame_export.c`) |
| Audio delivery | Core (level stream, 4 bytes/sample) | Host (`tools/pcm/ivs_refpcm.c` → PCM) | one converter, uninstalled, reused; the core never converts |
| Tick target per call | Host | Core (units, overshoot) | the core's `target_ticks` is the only time authority |
| Export surface | Build (export list + hidden visibility) | Check (`nm` comparison) | the linker refuses growth; the check refuses a list/artifact disagreement |
| Install inventory | `core/CMakeLists.txt` (unchanged) | `pkg-install` step + selftest | the shared object is a non-member by construction |

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

#### The host is build-only and the ABI subset is project-authored
- No libretro header is vendored (D-48, scoping R-3). `host/libretro_shim.h`
  states only the subset this project implements — the eight entries the harness
  resolves — and says *libretro-shaped*, never compatible. Rights-clean: nothing
  to licence, nothing to notice. If an exact header is ever needed, its licence
  text is read and recorded as `licence_evidence` first. — **Reversibility:**
  one-way — the export surface is a published contract that criterion 1 refuses
  by name; growing it later re-opens the export check, the harness and the
  control that pins it.
- The shared target is the only place `IVS_API` is defined
  (`target_compile_definitions`); installed headers keep it empty, so D-35's
  static-only package stands and `pkg-install-files.ids` stays eleven paths.
- The shim links `runner/frame_export.c` rather than copying the palette array
  (scoping R-4): one palette, one definition, labelled `not STIC colour data`,
  never in `core/src`, never in a golden, digest, assert or claim, and no capture
  option for `content <role> user`.

#### The harness is the evidence, never the window
- The fake frontend lives in `runner-process` and `dlopen`s the built shim,
  resolving the eight entries plus a harness-only `ivs_retro_digest(domain,out32)`.
  It asserts: version; a denied environment command refused rather than ignored;
  `av_info` equal to `ivs_info`; an index-plane frame of `width*height` bytes at
  the declared pitch; one project cartridge; a poll callback returning the
  scenario's words; and `frames>0 && samples>0`.
- Digest equality is guarded three ways, exactly as the frozen verification
  strategy states, and each guard must be shown able to fail: the observed-set
  equality, the empty-poll difference, and the two stand-ins.
- The tier ceiling is `deterministic-repeatability`, **provisional**: the same
  core compared with itself, never an independent oracle. One claim only, and
  only if the verifier can evidence it.

#### Packaging names the shared object as a non-member
- `tools/check/binary-ext.txt` gains `so`, `dylib`, `dll`, `bundle`; a tracked
  shared library is refused by name, with controls `files-unlisted-binary-so`
  and `-dylib`.
- One `pkg-install` selftest that the shared object is **absent** from the
  installed package (19 → 20 selftests).
- `docs/packaging.md` gains the "not installed" line; `RELEASE-OPS.md` and one
  `release-assets` case record that no host artifact ships.

#### Gate cost and evidence discipline
- Priced addition **1.5 hosted cpu-s** against the 21 cpu-s allowance. No new
  gate step, fault or golden: `steps=38`, `faults=238`, `goldens=29` invariants.
- Same-commit retargets per GUARDRAILS V6: a `host/` shared target moves the
  build-only target, `binary-ext.txt`'s four extensions, the two controls, the
  `pkg-install` selftest, `docs/packaging.md`, `RELEASE-OPS.md` and one
  `release-assets` case.
- Every new equivalence gets a control whose stand-in fails it; no case may
  assert only `exit 0` / `outcome pass`.
- The boundary checkpoint recomputes the last-five medians from
  `.planning/ci-runs.tsv` and records exactly one verdict.

#### Decided autonomously
- [The host] Q: "Which eight entries does the subset declare?" → Selected: "`api_version`, `init`, `deinit`, `set_environment`, `get_system_av_info`, `load_game`, `run`, `unload_game`, plus a harness-only `ivs_retro_digest`" — the set is one header and one resolver list; undo is local.
- [The harness] Q: "Which cartridge does the harness load?" → Selected: "the project's own `diag-cpu-1.ivsc` with the boot stub and zero GROM" — a project fixture, not a content byte.
- [The harness] Q: "How is 'no palette of its own' proved?" → Selected: "the shim links `runner/frame_export.c`, and a symbol scan refuses an `ivs_` or palette symbol in the shared object's export surface" — one id plus one symbol list.
- [Packaging] Q: "Does the shared object need a fourth `files` control per extension?" → Selected: "two controls (`-so`, `-dylib`) over one extension list, matching the `tools/bench` precedent that did not add one per directory" — a control count, not a contract.
- [Packaging] Q: "Where does 'no host artifact ships' live?" → Selected: "one `release-assets` case plus the `RELEASE-OPS.md` line" — record edits.
- [Cost] Q: "Does the checkpoint add a measurement run?" → Selected: "no — recompute from the committed `.planning/ci-runs.tsv` rows" — the file is the arithmetic input.

### Claude's Discretion

None — every question was frozen upstream or decided autonomously under the reversible-technical rule.

### Deferred Ideas (OUT OF SCOPE)

- A real frontend loading the shim: a handoff row, never a criterion.
- Any vendored libretro header: refused unless a real frontend needs one, and its licence read first.
- Shipping the shared object: refused; the release ships the static package and the source archive.
- A window, an audio device, a capture option for owner content: out of scope.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| TOOL-08 | The core can be driven in real time by a host that loads it as a shared object outside the installed package, and an automated frontend reproduces the runner's nine digest lines on a project cartridge — with the vacuity closed three ways and the host holding no palette of its own. | Pattern 1 (the nine lines and their single producer), Pattern 2 (the exact drive, measured three ways), Pattern 3 (the ABI subset), Pattern 4 (the build-only target), Pattern 5 (the export-surface mechanism), and the two `files`/`pkg-install` rows of Pattern 7. |
</phase_requirements>

## Standard Stack

### Core

Nothing is installed. The phase adds **zero** third-party dependencies, by construction
(D-48: no libretro header) and by rights (nothing to licence).

| Component | Version | Purpose | Why this |
|---|---|---|---|
| CMake `add_library(... SHARED)` | 4.4.3 (measured: `cmake --version`) | the build-only shared target | the tree already builds everything with CMake presets; a second build system is a new failure class |
| the C toolchain already in the presets | Apple clang 21.0.0 / Ubuntu 24.04 gcc | objects, `dlopen`, `nm` | `gate-linux.sh` already runs `ubuntu:24.04` in docker |
| `runner/frame_export.c` | in tree | `ivs_frameppm_palette(i, rgb)` — the one palette | R-4: linked, never copied |
| `tools/pcm/ivs_refpcm.c` | in tree | the one converter (levels → PCM) | R-4: a second converter is refused by review |

### Supporting

| Component | Purpose | When to Use |
|---|---|---|
| `$<TARGET_FILE:…>` generator expression | hands the built `.so`/`.dylib` path to the suite | `tests/CMakeLists.txt`, the same way `$<TARGET_FILE:ivs_flatwrap>` is passed today |
| `CMAKE_DL_LIBS` | `dlopen` on platforms that still need `-ldl` | the harness target only |
| `nm -P -g --defined-only` | the export/defined-symbol read | the check; the `pkg-install` step's `norm()` already normalizes it |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|---|---|---|
| an export list + `nm` comparison | `-fvisibility=hidden` alone | hidden visibility is **already on for every project target** (`cmake/IvsWarnings.cmake:14`), so it hides everything including the nine entries; it cannot express "these nine and no more", and it refuses nothing by name |
| `-Wl,--exclude-libs,ALL` (GNU only) | an export list | it hides archive symbols but not the shim TU's own; it does not exist on Apple `ld`; it refuses no name |
| linking `ivs_runner_lib` into the shim | compiling `runner/frame_export.c` into the shim target | `ivs_runner_lib` contains `exec.c`, which imports `fopen`/`open` — it would break criterion 4's "opens no file" as a *static* fact |
| a new ctest suite for the host | ids inside `runner-process` | a new suite changes `ctest-list`, `tests-inventory`'s suite set and `ctest-runs`; the frozen verification strategy says 8 ids in `runner-process` |

**Installation:** none.

**Version verification:** not applicable (no package is added). The toolchain versions
above were read from this host this session; the gate itself records `cc=AppleClang` in its
summary line.

## Package Legitimacy Audit

Not applicable: the phase installs no external package and vendors no header (D-48). The
only "dependency" it gains is the platform's own `dlopen`, which is part of the C library
on both gate platforms.

| Package | Registry | Age | Downloads | Source Repo | Verdict | Disposition |
|---|---|---|---|---|---|---|
| — | — | — | — | — | — | none added |

## Architecture Patterns

### System Architecture Diagram

```
                      tests/host/ivs_host_harness   (the fake frontend; the evidence)
                       |        |          |
        dlopen+resolve |        |          |  reads the three images from argv (only file I/O in the process)
                       v        |          v
             host/libivs_retro.so/.dylib   <-- built by host/CMakeLists.txt, no install rule
                       |        |          |
   nine shaped entries |        |          |  ivs_retro_digest(domain, out32)
                       v        v          v
             core/src  (ivs_core.a, unchanged, hidden visibility)
             ivs_machine_create/load/reset/run/state_digest
                       |                    |
        frame slots (index plane)      level samples (4 bytes / 16 ticks)
                       |                    |
        ivs_frameppm_palette --> XRGB    ivs_refpcm --> int16 PCM
                       |                    |
                       v                    v
             video_refresh(plane,pitch)  audio_batch(pcm, frames)      [host callbacks]
                       |                    |
                       +------> the harness's counters and pixel probes
                                        |
                              nine `digest.<domain> <sha>` lines on stdout
                                        |
                              runner-process judge: check_one_golden + frames>0 + samples>0
```

### Recommended Project Structure

```
host/
├── CMakeLists.txt          # SHARED target, no install rule, export list, IVS_API definition
├── libretro_shim.h         # the project-authored subset: eight entries + ivs_retro_digest
└── ivs_retro_shim.c        # the entries, the pixel conversion, the refpcm feed
tests/host/
└── ivs_host_harness.c      # dlopen, drive, assert, print; NOT linked against ivs_core
data/expected/
├── host-symbols.ids        # the export contract (nine names) — also the linker's export list
└── host-imports.ids        # the shim's allowed imports beyond core-symbols-allow.txt
```

### Pattern 1: the nine digest lines and their single producer

`data/golden/demo-lamp-tone-pass.digests` is written by `ivs-run --update-golden`
(`docs/runner.md:258`) and is the report filtered to the golden key list
(`runner/report.c:501`, `ivs_runner_golden_keep`). The nine lines are exactly:

```
digest.cpu       1ac14963e046e4c5c2eaf14fede2671dd2f6b42e0aa2edfa717862889ad292ab
digest.mem       0b23446d3b97b6b440100df6a122efbf9247a3ee3b314e4c881e0376ed3d4172
digest.images    09679d87b9851e20d6c32494887429c18be14d9182252de2a881a4633a38ad48
digest.instance  627a4c963a2f0cc121b82fdd7dc7e940f54442c41789e4056be4d538359e0cf7
digest.composite 7f71d668beded13af2893c0dbb9b37cbf23aeaa8a37c042b16912d5db0536909
digest.video     66c5747555074a7a8fe1455e331930b3f3d84e06cd6dbb5c92fb490d8fd85546
digest.audio     71f2939b1e35360b55177d983cd64a3f8248256375c13e4a12c716fcc2245490
digest.input     832906ce775d32a98ae607386ce4bbcdd4d08017e3b914b6089e50a5364c7761
digest.psglog    d224205dad483db6c04d991424051d794aedd1db3ab67546fd44b6626a0c9407
```

`[VERIFIED: data/golden/demo-lamp-tone-pass.digests:111-119]` (quoted verbatim above).

The domains are `IVS_DIGEST_*`, append-only, `[VERIFIED: core/include/ivs/ivs_machine.h:61-69]`:

```
#define IVS_DIGEST_COMPOSITE 0u
#define IVS_DIGEST_CPU 1u
#define IVS_DIGEST_MEM 2u
#define IVS_DIGEST_IMAGES 3u
#define IVS_DIGEST_INSTANCE 4u
#define IVS_DIGEST_VIDEO 16u
#define IVS_DIGEST_AUDIO 17u
#define IVS_DIGEST_INPUT 18u
#define IVS_DIGEST_PSGLOG 19u
```

The producer is `ivs_machine_state_digest`, called once per domain at the end of the run,
`[VERIFIED: runner/exec.c:114-116]`:

```
static const uint32_t ex_domains[EX_DOMAINS] = {IVS_DIGEST_CPU,       IVS_DIGEST_MEM,   IVS_DIGEST_IMAGES,
                                                IVS_DIGEST_INSTANCE,  IVS_DIGEST_COMPOSITE, IVS_DIGEST_VIDEO,
                                                IVS_DIGEST_AUDIO,     IVS_DIGEST_INPUT, IVS_DIGEST_PSGLOG};
```

and printed in record order by `rp_tail` (`runner/report.c:121-130`). **A host reproduces
them by calling the same function at the same machine tick** — nothing is recomputed, and
`ivs_retro_digest(domain, out32)` is that call.

### Pattern 2: the exact drive that reproduces the golden (measured three ways)

The scenario's own shape, `[VERIFIED: data/scenarios/demo-lamp-tone-pass.ivss:17-29,53]`:

```
content exec boot-stub-2
content grom zero-grom
content cart lamp-tone.ivsc
capture video 0 13
capture psglog
capture audio
input at_tick=100000 c0=KEY_1 c1=0x0
input at_tick=220000 c0=KEY_1+KEY_9 c1=0x0
input at_tick=340000 c0=0x0 c1=DISC_E
input at_tick=460000 c0=SIDE_TOP c1=DISC_E
input at_tick=580000 c0=0x0 c1=0x0
input at_tick=700000 c0=KEY_1+KEY_9 c1=SIDE_LOWER_LEFT
run-to 200000 ticks
...
run-to 806568 ticks
```

`lamp-tone.ivsc` is a **build-tree** image (460 bytes, SHA-256
`ac0ad987da1330101f423b294b6c24ea14de36f43f6b2dae82cd91d32b8e0ba8`), built by `ivs_pack`
from `fixtures/FX-demo-lamp-tone-1/lamp-tone.listing.txt`
(`tools/pack/CMakeLists.txt:89-98`). See Open Question 1 — CONTEXT names
`diag-cpu-1.ivsc`, whose SHA-256 is `243626a75f7592fcb904a6c2cba2de307f19f08eb484ae9f94fd0383526ee5c0`
and whose golden (`diag-boot-pass`) has `video=none` and zero input events, so it cannot
produce these nine lines.

**Measured, this session** (`cc -fsanitize=address,undefined -I core/include` against
`build/dev/core/libivs_core.a`, a throwaway probe, nothing committed):

| Drive | calls | result |
|---|---|---|
| one call to 806568 | 1 | nine lines equal; 13 frames; 198848 video bytes; 50410 samples; 480 log bytes |
| the scenario's four bounds | 4 | nine lines equal; `now_ticks` 200024 / 440012 / 640004 / 806568 — **the golden's own step lines** |
| one frame per call + final | 14 | nine lines equal; one frame per call; the final call completes none |
| stop at the 13th frame end (776600) | 13 | **7 lines differ** (`cpu mem instance composite audio input psglog`); `video` and `images` equal |

The third row is the shape TOOL-08's "driven in real time" means: a frontend that calls
`run` once per frame. The fourth row is why it needs a tick target for the last leg.

Shapes the harness needs, `[VERIFIED: core/include/ivs/ivs_run.h:140-143]`:

```
#define IVS_RUN_F_STOP_ON_FRAME 2u  /* stop at the end of the frame in progress (plan 06-09, D-14): the effective target is
                                     * the smaller of target_ticks and the frame end (next_frame_end_ticks of the report), the
                                     * call behaves as a run to that target, at most one frame completes and the video buffer
                                     * needs exactly one slot, whatever the target (IVS_RUN_TARGET_MAX included) */
```

Instance constants measured from `ivs_machine_info` on this profile:
`nominal_frame_ticks 59736`, `frame_phase_ticks 0`, `video_slot_bytes 15296` (32-byte header
+ 159×96 = 15264), `audio_bytes_per_frame 15060`, `audio_sample_ticks 16`,
`psg_log_record_bytes 16`, `psg_log_records_per_frame 2152`, `video 159x96`. Frame ends are
`59736·k`; **806568 is not one** (13·59736 = 776568, 14·59736 = 836304).

### Pattern 3: the ABI subset is shaped by *name*, and two signatures are project-shaped

`load_game` cannot be libretro's `retro_load_game(const struct retro_game_info *)`: the core
needs **three** byte spans (EXEC 8192, GROM 2048, cartridge ≤ 131256) and criterion 4 says
the host opens no file but the images named on its command line, so the spans are bytes, not
a path. `run` cannot be `void retro_run(void)`: the nine lines require the final tick 806568
(Pattern 2, row 4). Both are recorded as the project's own shape and documented as such in
`host/libretro_shim.h`; the *count* stays eight plus `ivs_retro_digest`. Recommended shape:

```c
/* host/libretro_shim.h — project-authored subset; libretro-shaped, never compatible. */
#define IVS_RETRO_SHAPE_VERSION 1u

typedef struct ivs_retro_images {
  const uint8_t *exec;   size_t exec_len;
  const uint8_t *grom;   size_t grom_len;
  const uint8_t *cart;   size_t cart_len;
} ivs_retro_images;

typedef struct ivs_retro_input {   /* the frontend's script, absolute master ticks */
  uint64_t at_tick;
  uint32_t controller[4];          /* PSG port index, the core's logical words */
} ivs_retro_input;

typedef struct ivs_retro_host {
  void *ctx;
  void (*video_refresh)(void *ctx, const uint32_t *xrgb, unsigned w, unsigned h, size_t pitch_bytes);
  void (*audio_batch)(void *ctx, const int16_t *pcm, size_t frames);
  /* poll: the next input change at or after now_ticks, or 0 when the script is exhausted.
     Returns 1 and fills *ev when there is one. This is the "poll callback returning the
     scenario's words". */
  int (*input_poll)(void *ctx, uint64_t now_ticks, ivs_retro_input *ev);
} ivs_retro_host;

unsigned     retro_api_version(void);                                  /* 1: the shape's version */
void         retro_init(void);
void         retro_deinit(void);
int          retro_set_environment(ivs_retro_host *host);              /* 0 = refused */
void         retro_get_system_av_info(retro_system_av_info *out);      /* from ivs_info */
int          retro_load_game(const ivs_retro_images *imgs);            /* 0 = refused */
int          retro_run(uint64_t target_ticks);                         /* 1 = a frame was delivered */
void         retro_unload_game(void);
int          ivs_retro_digest(uint32_t domain, uint8_t out32[32]);      /* the one harness entry */
```

`retro_set_environment` returning `0` for a host it will not accept is the "denied
environment command refused rather than ignored" assertion: the harness installs a host
whose `input_poll` is NULL and requires `0`, then installs the real one and requires `1`.
`retro_unload_game` must be a real teardown (`ivs_machine_destroy`), because the core has no
unload-while-powered path — the harness proves it by requiring `ivs_retro_digest` to fail
afterwards (otherwise a stub that never unloads would pass).

### Pattern 4: the build-only shared target

`[VERIFIED: core/CMakeLists.txt:126-128]` — the archive is static and `IVS_API` stays empty:

```
# Static only (D-35): STATIC is explicit, so BUILD_SHARED_LIBS is ignored and IVS_API stays empty.
add_library(ivs_core STATIC ${ivs_core_objects})
set_target_properties(ivs_core PROPERTIES INTERPROCEDURAL_OPTIMIZATION OFF LINKER_LANGUAGE C
  EXPORT_NAME core)
```

The install rule is `install(TARGETS ivs_core …)` plus `install(DIRECTORY include/ivs …)` in
the same file; the package is eleven paths, `[VERIFIED: data/expected/pkg-install-files.ids:5-15]`
(`include/ivs/ivs.h`, `ivs_machine.h`, `ivs_run.h`, `ivs_state.h`, `ivs_status.h`,
`lib/cmake/ivs/ivsConfig.cmake`, `ivsConfigVersion.cmake`, `ivsTargets-release.cmake`,
`ivsTargets.cmake`, `lib/libivs_core.a`, `share/doc/ivs/LICENSE`). A `SHARED` target that is
never named in an `install()` command is a non-member; the `pkg-install` inventory check is
what proves it.

Hidden visibility is already the house rule, `[VERIFIED: cmake/IvsWarnings.cmake:13-14]`:

```
    -Wimplicit-fallthrough -Wdouble-promotion -Werror=date-time
    -ffp-contract=off -fvisibility=hidden)
```

so the shim's own palette/helpers cannot leak; only the entries marked `IVS_API` can, and
`IVS_API` is defined for this one target (CONTEXT). The dev preset sets `IVS_SAN=ON`
(`CMakePresets.json`), and the core archive is instrumented there — **the shim must link
`ivs_san`, and so must the harness**, or the instrumented `.so` fails to load into a plain
process. This is the single most likely "works on my machine, red in the gate" trap.

Recommended target (sketch; the plan owns the exact text):

```cmake
add_library(ivs_retro_shim SHARED ivs_retro_shim.c "${PROJECT_SOURCE_DIR}/runner/frame_export.c")
target_include_directories(ivs_retro_shim PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/runner" "${PROJECT_SOURCE_DIR}/core/include")
target_compile_definitions(ivs_retro_shim PRIVATE
  IVS_API=__attribute__((visibility("default"))))   # the only place IVS_API is defined
target_link_libraries(ivs_retro_shim PRIVATE ivs_core ivs_refpcm ivs_warnings ivs_san)
set_target_properties(ivs_retro_shim PROPERTIES OUTPUT_NAME ivs_retro PREFIX "lib")
# the export list: generated from data/expected/host-symbols.ids, both platform forms
```

and in `CMakeLists.txt` a `add_subdirectory(host)` **before** `enable_testing()`, with no
`install()` line anywhere.

### Pattern 5: the export surface, refused by name

Recommended: one contract file, two generated linker inputs, one comparison.

```
data/expected/host-symbols.ids        (one name per line, the nine entries)
        |                                     |
        +--> ELF:   { global: a; b; …; local: *; };      -Wl,--version-script,<f>
        +--> Mach-O: _a  _b  …                            -Wl,-exported_symbols_list,<f>
        |
        +--> the check: nm -P -g --defined-only <so> | norm | sort  ==  sorted ids
```

Measured this session, with **default-visibility** objects (the core archive's case) so the
result is not an artifact of `-fvisibility=hidden`:

| Platform | mechanism | `nm -P -g --defined-only` |
|---|---|---|
| Darwin (Apple clang 21) | `-Wl,-exported_symbols_list,list` | exactly the listed names; the others become `non-external (was a private external)`; `dyld_info -exports` agrees |
| Ubuntu 24.04 (gcc, docker) | `-Wl,--version-script,map` | exactly the listed names; `readelf --dyn-syms` agrees |

Two asymmetries the plan must absorb: Apple `ld` **errors** on an export-list name the link
does not define (`Undefined symbols … "_name", referenced from: <initial-undefines>`), while
GNU `ld` accepts it silently — hence the check's missing-name direction. And `nm -P -u` on a
linked `.so`/`.dylib` shows the imports cleanly (`___memcpy_chk`, `_free`, `_malloc`), so
criterion 4's "opens no file" is a *static* rule: the shim's undefined names must be a
subset of `core-symbols-allow.txt` ∪ `data/expected/host-imports.ids`
(`malloc calloc realloc free`).

The palette limb is already the house rule for the core,
`[VERIFIED: tools/check/core-symbols.sh:38-42]`: "the archive must not DEFINE a symbol …
whose name contains palette, rgb, ppm or framepreview in any letter case". `core-symbols.sh`
cannot be run on the shim (the shim *must* define those), so the new check restates the four
words over the shim's **exported** names only — which the exact-set comparison already
implies, but the criterion states it, so it is one explicit line.

### Pattern 6: the fake frontend, judged by the existing rules

The judge is not new logic. `check_one_golden`
`[VERIFIED: tools/check/steps/pkg-install.sh:463-476]` already compares exactly what
criterion 3.1 asks for — the sorted `digest.` line set **and** the `# domains:` observed set:

```
  grep '^digest\.' "$co_gold" | sort > "$T/rg.want"
  sed -n "s/^${co_pre}digest\./digest./p" "$co_out" | sort > "$T/rg.have"
  …
  sed -n 's/^# domains: //p' "$co_gold" | tr ' ' '\n' | sed -n 's/=observed$//p' | sort > "$T/rg.dom.want"
  sed -n "s/^${co_pre}digest\.\([a-z]*\) .*/\1/p" "$co_out" | grep -v -x -e composite | sort > "$T/rg.dom.have"
```

The harness prints only the domains it observed, so "every line non-`-`" is the same
statement as "the observed set equals the golden's" — no new rule. The two stand-ins use the
suite's own self-control helper, `[VERIFIED: tests/runner-process-tap.sh:2215-2230]`:

```
# self_control <slug> <token> <check...>: runs the check; ok only when it fails with <token>.
self_control() {
  _slug=$1; _tok=$2; shift 2
  …
  result "runner-process.judge.$_slug"
}
```

so `host-vacuous` and `host-digest-diff` are two fake frontends written into the temp dir
(one prints the golden's nine lines with `frames 0`, one prints them with the poll dropped),
each passing only when the judge rejects it with its token. No second `.so` build, no new
fault, no new golden.

### Pattern 7: the packaging rows

`binary-ext.txt` is a flat list read by `files.cmake`, `integrity.cmake`, `binscan.sh`,
`release-assets.sh` and `hygiene --history`
(`[VERIFIED: tools/check/binary-ext.txt:1-3]` "Extensions treated as binary by name … Read by
tools/check/integrity.cmake, tools/check/files.cmake and tools/check/binscan.sh"). The
refusal, `[VERIFIED: tools/check/files.cmake:139-150]`:

```
  if(NOT "${ext}" STREQUAL "" AND "${ext}" IN_LIST BINARY_EXT)
    set(is_binary TRUE)
  endif()
  …
    if("${p}" IN_LIST binary_ok)
      math(EXPR nbin "${nbin} + 1")
    else()
      gate_fail(files-unlisted-binary "${p}")
```

The closest control is the `ivsn` one, added for exactly this reason (a new rights-bearing
extension), and it is three files: `CHECK` = `files`, `EXPECT` =
`FAIL files-unlisted-binary: docs/save.ivsn`, `MAKE` = `write docs/save.ivsn x`. The new
`files-unlisted-binary-so` / `-dylib` are the same three files with `.so` / `.dylib`, and
they register in `data/expected/controls/rights.ids` beside `files-unlisted-binary-ivsn`.
`release-assets` then refuses a `.so` **member** with no new code
(`release-assets-binary-member`, "whatever the list says"), so the priced case only has to
pin the new extension by name, the way `rom-member` pins `.rom`.

### Anti-Patterns to Avoid

- **Recomputing a digest in the shim or the harness.** The only producer is
  `ivs_machine_state_digest`; a recomputation is a second oracle and would hide a core change.
- **A frame-counted stop.** See Pattern 2, row 4: 806568 is mid-frame.
- **A second palette or a second converter.** R-4; also `core-symbols.sh`'s defined-symbol
  rule would fire if either landed in `core/`.
- **Linking `ivs_runner_lib`** into the shim (imports `fopen`/`open`).
- **`nm` output as evidence.** Compare a parsed, sorted name set; never print a dump.
- **A `[claim:…]`-less claim word** in the new `docs/packaging.md` sentence: `runs`,
  `supports`, `compatible`, `plays`, `looks`, `sounds`, `passes`, `accurate`, `verified`,
  `playable` are tripwired by `text.sh` in `README.md` and `docs/**` and need `[nc]` or a tag.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---|---|---|---|
| index → RGB | a palette in `host/` | `ivs_frameppm_palette` from `runner/frame_export.c` | one definition; the 17 triples are already pinned as distinct by `tests/runner/runner_exec.c:2352-2370`, which is what makes the harness's inverse lookup sound |
| levels → PCM | a converter in `host/` | `tools/pcm/ivs_refpcm.c` (`ivs_refpcm_init`/`ivs_refpcm_feed`) | R-4; one converter, already tested, already excluded from the core by a defined-symbol rule |
| instance storage | a core allocator | the shim's `malloc` + hand alignment, as `runner/exec.c:1428` does | the core takes no allocator (D-01) |
| the nine lines | a digest reimplementation | `ivs_machine_state_digest` behind `ivs_retro_digest` | the golden is the producer's own output |
| the observed-set + line-set rule | a new judge | `check_one_golden`'s two comparisons | identical semantics, already proven able to fail by `scn-digest-drift` / `scn-domain-missing` |
| the export allowlist | a hand-written `nm` grep | `data/expected/host-symbols.ids` + `norm()` from `pkg-install.sh` | one normalizer already handles `@version` and the Darwin underscore |

**Key insight:** every piece of this phase already exists somewhere in the tree; the phase's
whole risk is *wiring*, and every wire has a precedent to copy.

## Runtime State Inventory

Not a rename/refactor phase — omitted by the phase's own definition (greenfield target,
no stored data, no live service, no OS registration).

## Common Pitfalls

### Pitfall 1: `digest.input` includes the input floor, so the final tick is part of the equality
**What goes wrong:** a host drives frame-by-frame, stops at the 13th frame end, and compares
nine lines — seven of them differ, and the failure looks like a broken shim.
**Why it happens:** `[VERIFIED: core/src/machine/machine_digest.c:161-164]` serializes the
whole INPUT row, including the floor:

```
    IVS_DF("obs.input.words", obs.input.words, 4u, 4u, IVS_DIGEST_INPUT, IVS_DIGEST_KIND_UINT, IVS_SNAP_STATE),
    IVS_DF("obs.input.count", obs.input.count, 8u, 1u, IVS_DIGEST_INPUT, IVS_DIGEST_KIND_UINT, IVS_SNAP_STATE),
    IVS_DF("obs.input.floor_tick", obs.input.floor_tick, 8u, 1u, IVS_DIGEST_INPUT, IVS_DIGEST_KIND_UINT, IVS_SNAP_STATE),
    IVS_DF("obs.input.chain", obs.input.chain, 1u, 32u, IVS_DIGEST_INPUT, IVS_DIGEST_KIND_UINT, IVS_SNAP_STATE),
```

and the floor is set by the step seam to "the start of the last begun unit + 1"
(`core/src/machine/machine_run.c:206`). The *chain* is slicing-invariant
(`core/src/machine/machine_input.c:9-11`: "over the event's ABSOLUTE at_tick … so the log
equals the accepted script whatever the slicing"); the *digest* is not.
**How to avoid:** the shim's `run` carries the target tick and the harness ends at exactly
806568. State the invariant in the plan: *the nine lines are a function of (six `at_tick`s,
final tick)* — measured.
**Warning signs:** exactly seven lines differ and `digest.video`/`digest.images` agree.

### Pitfall 2: frame-quantized input cannot reproduce the golden
**What goes wrong:** a "realistic" libretro loop polls the controller once per frame and
applies the words at the frame boundary. The scenario's six ticks are deliberately
"at least 16,800 ticks from any frame boundary"
(`data/scenarios/demo-lamp-tone-pass.ivss:7-9`), and the chain hashes `be64(at_tick)`
(`runner/exec.c:1068`), so `digest.input` — and the guest's reads — differ.
**How to avoid:** the input poll carries the tick (`ivs_retro_input.at_tick` above), and the
shim submits the **unconsumed suffix** each call, exactly as the runner does
(`runner/exec.c:1184`: `q.events = consumed < x->sched_n ? &x->sched[consumed] : NULL`).
Resubmitting the whole array is refused `INPUT_BEFORE_FLOOR` — measured: reason 50 on the
second call of a naive probe.
**Warning signs:** `IVS_E_BAD_ARG` with `refusal_reason 50` on the second `run`.

### Pitfall 3: the dev preset is sanitized, and `dlopen` of an instrumented `.so` fails
**What goes wrong:** `ctest --preset dev` dies with "ASan runtime does not come first" or an
undefined `__asan_*` at link of the shim.
**Why it happens:** `CMakePresets.json` sets `IVS_SAN=ON` for `dev`; `ivs_san` adds
`-fsanitize=address,undefined` to compile *and link*.
**How to avoid:** the shim links `ivs_san` (it links the instrumented archive anyway) and the
harness links `ivs_san` too, so the runtime is present before `dlopen`. Scoping-4's "no
sanitizer lane for the shim" means *no extra lane*, not *no sanitizer*.

### Pitfall 4: hidden visibility is already on, so nothing is exported by default
**What goes wrong:** the `.so` exports zero symbols and every resolver returns NULL.
**Why it happens:** every project target links `ivs_warnings`, which carries
`-fvisibility=hidden`; `IVS_API` is empty in the headers.
**How to avoid:** define `IVS_API` for the shim target only
(`target_compile_definitions(... IVS_API=__attribute__((visibility("default"))))`) and
declare the nine entries in `host/libretro_shim.h` (also satisfying `-Wmissing-prototypes`).

### Pitfall 5: a version script cannot hide a name that must exist
**What goes wrong:** on macOS the link fails with `Undefined symbols … "_ivs_retro_digest",
referenced from: <initial-undefines>` because the export list names a symbol the link does
not define; on Linux the same mistake passes silently and the check's missing-name direction
is the only detector.
**How to avoid:** generate the export list from the same `host-symbols.ids` the check reads,
and keep both directions of the comparison.

### Pitfall 6: the toy registries and the `important-gates` same-commit rule
**What goes wrong:** the gate goes red in `important-gates` with
`important-gates-complete-uncovered: TOOL-08` (a Complete requirement with no row) or
`important-gates-unknown-id` (a row naming a Pending requirement).
**Why it happens:** the accepted set is derived from the `## Traceability` table of
`REQUIREMENTS.md`, and the step refuses both directions in one pass.
**How to avoid:** the TOOL-08 flip to `Complete`, its two `mutant` rows
(`runner-process.judge.host-vacuous`, `runner-process.judge.host-digest-diff`) and the new
gate name land **in one commit** — exactly what Phase 14 did for TOOL-07/SCN-03/CPU-06. The
two `files` controls land here but their `important-gates` rows belong to PKG-06, which is
Pending until Phase 16, so no row is written for them yet (a control need not be registered).

### Pitfall 7: `c-build` does not see a new source directory
**What goes wrong:** a tracked `host/*.c` that no preset compiles is invisible to `c-build`,
which only walks the seven directories in its module table
(`[VERIFIED: tools/check/steps/c-build.sh:48-51]`, `^core/`, `^runner/`, `^tools/pack/`,
`^tools/pcm/`, `^tools/asm/`, `^tools/bench/`, `^tools/flatwrap/`).
**Why it is acceptable here (recommended deviation, R6 not taken):** unlike a tool directory,
a host TU that is not compiled either breaks the `.so` link (undefined reference) or removes
one of the nine exports, which `host-symbols.ids` refuses by name. The failure class is
covered at a cost of zero. Record the reasoning in the plan and in the phase SUMMARY; if the
verifier disagrees, the fallback is R6's set (module row + `ivs-host-sources.txt` + toy source
list + `c-build-unlisted-host-source`) at **+0.31 cpu-s**, which reprices the phase.

### Pitfall 8: the claim-word tripwire covers the new doc sentences
**What goes wrong:** `text-claim-untagged` on the `docs/packaging.md` line.
**Why it happens:** `text.sh` scans `README.md` and `docs/**` for
`supports|compatible|passes|accurate|verified|runs|plays|playable|looks|sounds`.
**How to avoid:** write "build-only … not installed, not shipped, not a release member" and
never "the shim runs/plays/supports". The word `libretro-shaped` is safe; `compatible` is not.

### Pitfall 9: the `pkg-install` leak check greps every installed file for `$ROOT/build`
**What goes wrong:** an installed file that names the build tree fails `pkg-path-leak`.
**Why it is relevant:** the shim is *not* installed, so nothing changes — but do not add a
`host` path to any installed CMake file while wiring the target (the check greps the logical
and physical forms of `$ROOT/core` and `$ROOT/build`).

## Code Examples

### 1. The measured drive, as the harness will implement it (frame per call)

```c
/* after retro_init + retro_set_environment + retro_load_game(imgs) — the shim does the rest */
unsigned frames = 0, samples = 0;
for (;;) {
  uint64_t target = (frames == 0) ? info.frame_phase_ticks + info.nominal_frame_ticks
                                  : last_next_frame_end;      /* from the shim's report */
  if (target > 806568u) target = 806568u;                    /* the scenario's last bound */
  int delivered = retro_run(target);                         /* 1 = a frame arrived */
  frames  += delivered;
  if (target == 806568u) break;
}
/* then, and only then, the nine lines: */
for (unsigned d = 0; d < 9; d++) { uint8_t sha[32]; ivs_retro_digest(domain[d], sha); print_digest(d, sha); }
```

Measured equivalent forms: one call, four bounds, frame-per-call. The harness needs the
frame count and the sample count from the shim (its callbacks count them), and must not call
`retro_unload_game` before reading the digests.

### 2. The export list generation (one source of truth)

```cmake
file(STRINGS "${PROJECT_SOURCE_DIR}/data/expected/host-symbols.ids" IVS_HOST_SYMBOLS
  REGEX "^[A-Za-z_][A-Za-z0-9_]*$")
if(APPLE)
  set(lines "")
  foreach(s IN LISTS IVS_HOST_SYMBOLS)
    string(APPEND lines "_${s}\n")
  endforeach()
  file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/host-exports.txt" CONTENT "${lines}")
  target_link_options(ivs_retro_shim PRIVATE "LINKER:-exported_symbols_list,${CMAKE_CURRENT_BINARY_DIR}/host-exports.txt")
else()
  set(lines "{ global:\n")
  foreach(s IN LISTS IVS_HOST_SYMBOLS)
    string(APPEND lines "  ${s};\n")
  endforeach()
  string(APPEND lines "  local: *;\n};\n")
  file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/host-exports.map" CONTENT "${lines}")
  target_link_options(ivs_retro_shim PRIVATE "LINKER:--version-script,${CMAKE_CURRENT_BINARY_DIR}/host-exports.map")
endif()
```

### 3. The check, as one shell function (both directions, both platforms)

```sh
# host-symbols.sh <so> [<ids>]: exported set == the ids; no palette word; imports allowed.
norm() { awk -v darwin="$darwin" 'NF < 2 { next } $0 ~ /:$/ { next }
  { n = $1; sub(/@.*/, "", n); if (darwin && n ~ /^_/) n = substr(n, 2); if (n != "") print n }'; }
"$NM" -P -g --defined-only "$so" | norm | sort -u > "$tmp/have"
awk '/^#/ { next } NF { print $1 }' "$ids" | sort -u > "$tmp/want"
comm -23 "$tmp/want" "$tmp/have"   # host-symbols-missing: the list over-claims
comm -13 "$tmp/want" "$tmp/have"   # host-symbols-extra:   the artifact grew
grep -i -E 'palette|rgb|ppm|framepreview' "$tmp/have"   # host-symbols-palette
"$NM" -P -u "$so" | norm | sort -u | comm -23 - "$tmp/allowed"   # host-symbols-import
```

Measured: this returns the nine names on both platforms and nothing else; the imports are
`___memcpy_chk`-style fortify forms that `norm()` already maps onto
`core-symbols-allow.txt`.

### 4. The `pkg-install` selftest that the shared object is absent (20th case)

```sh
    host-so-absent)
      # The installed package must not carry the host shim: a copy of the prefix with
      # lib/libivs_retro.so planted is refused by name (the inventory rule, applied to the
      # artifact this phase creates).
      cp -R "$pb" "$d/prefix" || { detail="copy"; return 1; }
      printf 'not a library\n' > "$d/prefix/lib/libivs_retro.so"
      if check_inventory "$d/prefix" > "$d/out"; then detail="inventory passed"; return 1; fi
      grep -q -x -F -e '-- FAIL pkg-inventory-extra: lib/libivs_retro.so' "$d/out" \
        || { detail="no pkg-inventory-extra: lib/libivs_retro.so"; return 1; }
      ;;
```

This is the `extra-file` arm with the artifact's *name* pinned; the difference is that this
one names what this phase builds, so a future `install(TARGETS ivs_retro_shim …)` fails here
first. Add `host-so-absent` to `data/expected/pkg-selftests.ids` in the same commit.

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|---|---|---|---|
| vendoring the libretro header | a project-authored subset, *libretro-shaped* | D-48, 2026-11-26 | nothing to licence, nothing to notice; the cost is that two signatures are project-shaped (Pattern 3) |
| a windowed demo as "the host works" | a `dlopen` harness judged by nine committed digests | scoping-4, 2026-11-26 | a window is a CI blind spot; the harness is the evidence |
| shipping a shared core | the static package stays eleven files; the `.so` is a non-member | D-49, 2026-11-26 | the release line reads "no shared object, no host shim, no content" |

**Deprecated/outdated:** the assumption that a libretro-shaped `retro_run(void)` can drive a
deterministic tick-addressed core to an arbitrary tick. It cannot (measured), so `run` carries
a target — the only deviation from the eight names' *signatures*, not their count.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | The shim delivers PCM at 48 000 Hz through `ivs_refpcm` (one converter, R-4), so `av_info.sample_rate` is a constant the shim declares, not a derived value | Pattern 3, Open Question 3 | a frontend plays at the wrong rate; the `av_info` assertion has to compare against `ivs_info`'s `audio_sample_ticks` instead |
| A2 | `retro_api_version()` returns `1` (the *shape* version), not the core's packed `ivs_api_version()` | Pattern 3 | if the plan wants a "the .so is the core" assertion, it must come from the digests (which it does) |
| A3 | The two `files` controls and the `release-assets` case are Phase 15's (CONTEXT V6) even though scoping-2 priced them in Phase 16 | Cost | the phase line reprices to ~1.6 and Phase 16's line falls to ~0.05; the milestone total is unchanged |
| A4 | The TOOL-08 claim row is minted in Phase 15 (GUARDRAILS §3, MILESTONE-CONTEXT §2) although scoping-2's price table puts "+1 claim row" in Phase 16 | Cost, Open Question 4 | a claim minted a phase late, or `claims=28` where the close expects 29 |
| A5 | `host/` needs no `c-build` module row because a dropped host TU breaks the link or the export set (Pitfall 7) | Pitfall 7 | the verifier may insist on R6: +0.31 cpu-s, and the toy tree gains a `host/` source list |

## Open Questions

1. **Which cartridge?** (MATERIAL — the two frozen documents disagree.)
   - What we know: criterion 2 names `data/golden/demo-lamp-tone-pass.digests`, whose
     `slot.cart.sha256` is `ac0ad987…` = the 460-byte build-tree `lamp-tone.ivsc`;
     15-CONTEXT's autonomous decision and scoping-4 both name `diag-cpu-1.ivsc`, whose
     golden (`diag-boot-pass`) has `video=none`, `input.events 0` and `slot.cart.length_bytes 176`.
   - What's unclear: nothing factual — the two goldens are different runs. The only question
     is which document wins.
   - Recommendation: **the criterion wins** (ROADMAP is frozen above the discuss record), so
     the harness loads `lamp-tone.ivsc` with `boot-stub-2.exec` and zero GROM. `diag-cpu-1`
     cannot satisfy criterion 3.1 (its golden has no observed video domain, so `digest.video`
     would be `-`). Record the correction in the plan and in the phase SUMMARY; the CONTEXT
     sentence is amended by a live-record pointer, not rewritten.

2. **The `run` signature.** (MATERIAL — measured, not a preference.)
   - What we know: 806568 is not a frame end; a frame-only stop moves seven lines.
   - What's unclear: whether the plan prefers `run(target_ticks)` (recommended: one entry, one
     parameter, the fake frontend drives the scenario's four bounds) or `run(void)` plus a
     limit delivered through the environment call (keeps the void signature, puts the end
     tick in the same channel as the input script).
   - Recommendation: `run(target_ticks)`. It is the smallest honest change, it is what the
     core's own API already takes, and a real frontend calls it with `now + nominal_frame_ticks`.

3. **`av_info.sample_rate`.** Either 48 000 (the converter's output, A1) or the native level
   rate `crystal_hz/(den·audio_sample_ticks)` = 3 579 545/16 = 223 721.5625 (exact as a
   double). The first reuses the one converter as R-4 requires; the second would push
   conversion into the frontend, which R-4 refuses.
   - Recommendation: 48 000, with `retro_get_system_av_info` filling width/height from
     `ivs_info.video_width/video_height` and `fps = crystal_hz_num/(crystal_hz_den·nominal_frame_ticks)`
     = 3 579 545/59 736 ≈ 59.9236; the harness recomputes all four from `ivs_info` and
     requires equality.

4. **Which phase mints the claim.** GUARDRAILS §3 and MILESTONE-CONTEXT §2 say Phase 15;
   scoping-2's table prices "+1 claim row" in Phase 16.
   - Recommendation: Phase 15 (`claims 28→29`, `provisional 27→28`), because the two
     normative documents agree and the claim rests on this phase's golden. The V3 retarget
     (data/claims.json, data/expected/claims.ids, a `[claim:CL-0029]` tag on the line, the
     generated CHANGELOG table) lands in the same commit.

5. **Is the empty-poll id compared against the idle golden?** Measured: with zero events the
   seven non-input lines equal `data/golden/demo-lamp-tone-idle.digests` exactly, and
   `digest.input` is the empty-chain value `27da2e1f…`; eight of nine lines differ from the
   pass golden and `digest.images` is identical.
   - Recommendation: assert both directions — "at least one line differs from the pass golden"
     *and* "`digest.images` is equal" *and* "the seven non-input lines equal the idle golden".
     That is a stronger anti-constant-judge guard than "a different set", and it costs nothing
     because the idle golden is already committed.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|---|---|---|---|---|
| C compiler (Apple clang) | every target | ✓ | 21.0.0 (arm64) | — |
| CMake | presets, `LINKER:` options | ✓ | 4.4.3 | — |
| Ninja | presets | ✓ | 1.13.2 | — |
| `nm` (llvm-nm, GNU-compatible) | the export/import check | ✓ | llvm-nm | GNU `nm` in the container |
| `dlopen` | the harness | ✓ | libc / libSystem | `${CMAKE_DL_LIBS}` |
| docker | `tools/gate-linux.sh` | ✓ | 29.5.2 | a visible `not run` line, never a pass |
| `ubuntu:24.04` image | the Linux gate | ✓ (used by `gate-linux.sh`) | 24.04 | pull; a missing image is a `not run` |
| `act` | workflow shape | ✓ | 0.2.87 | — |
| `dash` | GUARDRAILS §4's `dash tools/gate.sh` line | ✗ on this host (Darwin has `sh` = bash 3.2) | — | run the `dash` leg inside the Ubuntu image, or record a visible `not run` for that half of the DoD |
| `python3` | probes only (never the gate) | ✓ | 3.14.4 | — |

**Missing dependencies with no fallback:** none.
**Missing dependencies with fallback:** `dash` on the host — the container leg covers it.

## Validation Architecture

`workflow.nyquist_validation` is true, so this section is required.

### Test Framework

| Property | Value |
|---|---|
| Framework | CTest (`tests/CMakeLists.txt`, one `add_test` per suite) + POSIX-sh TAP suites (`tests/*-tap.sh`), judged by `tools/check/steps/tests-inventory.sh` |
| Config file | `CMakePresets.json` (`dev`: ASan+UBSan+Werror+fault controls; `release`: -O2 -DNDEBUG+Werror) |
| Quick run command | `cmake --build --preset dev && ctest --preset dev -R runner-process` |
| Full suite command | `cmake --preset dev && cmake --build --preset dev && ctest --preset dev` then `sh tools/gate.sh` |

### Phase Requirements → Test Map

| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|---|---|---|---|---|
| TOOL-08 (criterion 1) | the `.so` exports exactly the nine shaped names | process (TAP) | `sh tests/runner-process-tap.sh …` case `runner-process.host.exports` | ❌ Wave 0 |
| TOOL-08 (criterion 1) | no palette/`ivs_` symbol beyond the pinned entry; no file-opening import | process (TAP) | same case (the second and third rules of `host-symbols.sh`) | ❌ Wave 0 |
| TOOL-08 (criterion 2) | `dlopen`, `api_version`, denied environment refused, `av_info` == `ivs_info`, geometry/pitch | process (TAP) | `runner-process.host.api` | ❌ Wave 0 |
| TOOL-08 (criterion 2) | nine lines equal to `demo-lamp-tone-pass.digests`, observed set equal | process (TAP) | `runner-process.host.digests` | ❌ Wave 0 |
| TOOL-08 (criterion 2) | `frames>0 && samples>0`, one frame per call, pixel probes | process (TAP) | `runner-process.host.frames` | ❌ Wave 0 |
| TOOL-08 (criterion 3.1) | every line non-`-`, observed set equal | process (TAP) | inside `host.digests` (`check_one_golden`) | ✅ rule exists |
| TOOL-08 (criterion 3.2) | empty poll gives a different set (and the idle golden's seven lines) | process (TAP) | `runner-process.host.empty-poll` | ❌ Wave 0 |
| TOOL-08 (criterion 3.3) | `host-vacuous` and `host-digest-diff` stand-ins refused | process (TAP) | `runner-process.judge.host-vacuous`, `.host-digest-diff` | ❌ Wave 0 |
| TOOL-08 (criterion 4) | the installed package is unchanged and the `.so` is absent | in-step selftest | `sh tools/check/steps/pkg-install.sh` case `host-so-absent` | ❌ Wave 0 |
| TOOL-08 (criterion 5) | `so`/`dylib` refused by name, two controls | control | `sh tools/check/run-controls.sh` (CHECK=files) | ❌ Wave 0 |
| TOOL-08 (criterion 5) | `steps=38/38` with the predicted delta | gate | `sh tools/gate.sh`; compare the token line | ✅ |

### Sampling Rate

- **Per task commit:** `cmake --build --preset dev && ctest --preset dev -R runner-process`
- **Per wave merge:** both presets' `ctest`, then `sh tools/gate.sh`
- **Phase gate:** `sh tools/gate.sh` and `dash tools/gate.sh` byte-equal on the summary line;
  `sh tools/gate-linux.sh` with `gate: PASS`; `sh tools/hygiene.sh --tree` silent;
  `git ls-files | grep -E '\.(ivsc|bin|rom|so|dylib|cfg|ivsn)$'` empty.

### Wave 0 Gaps

- [ ] `host/libretro_shim.h` — the subset (Pattern 3)
- [ ] `host/ivs_retro_shim.c` — the nine entries, the pixel conversion, the refpcm feed
- [ ] `host/CMakeLists.txt` — the `SHARED` target, the export list, `IVS_API`, no install rule
- [ ] `tests/host/ivs_host_harness.c` — `dlopen`, drive, assert, print
- [ ] `tests/CMakeLists.txt` — the harness target and the seventh `$<TARGET_FILE:…>` argument
- [ ] `tools/check/host-symbols.sh` — the export/import check (Pattern 5)
- [ ] `data/expected/host-symbols.ids` — the nine names
- [ ] `data/expected/host-imports.ids` — `malloc calloc realloc free`
- [ ] `data/expected/tests/runner-process.ids` — 8 id lines + the header comment
- [ ] `tools/check/controls/files-unlisted-binary-so/{CHECK,EXPECT,MAKE}` and `-dylib`
- [ ] `data/expected/controls/rights.ids` — the two control lines
- [ ] `data/expected/pkg-selftests.ids` — `host-so-absent`
- [ ] `data/expected/release-assets-selftests.ids` — the `.so`-member case
- [ ] `data/expected/important-gates.ids` — TOOL-08's two `mutant` rows + one gate name
- [ ] `data/claims.json`, `data/expected/claims.ids`, the tagged line, `CHANGELOG.md` table (if A4 holds)
- [ ] `tools/check/binary-ext.txt` — `so dylib dll bundle`
- [ ] `docs/packaging.md`, `.planning/ops/RELEASE-OPS.md` — the "not installed / not shipped" lines

## Cost checkpoint (recomputed this session from `.planning/ci-runs.tsv`, 99 usable success rows)

Last five successful rows, by arithmetic (not quoted):

| run | `build_s` | `gate_s` | `cpu_s` |
|---|---|---|---|
| 45718553735 | 35 | 119 | 363 |
| 45720934719 | 39 | 135 | 438 |
| 45723269901 | 35 | 122 | 368 |
| 45747709457 | 34 | 115 | 366 |
| 45751979661 | 34 | 120 | 359 |

Medians: **`cpu_s` 366**, **`build_s` 35**, **`gate_s` 120** (the freeze recorded 361 / 34 / 118;
the `cpu_s` median has moved +5 with the token line moving for phases 13 and 14, so the
boundary checkpoint's verdict is `reprice` only if it exceeds the priced additions — it does
not yet: 2.0 + 1.3 = 3.3 of the 5 moved).

Baseline token line (last line of `build/gate-linux.log` this session): `steps=38/38
controls=402/402 claims=28 faults=238/238 tracked=2748 goldens=29 gates=117 lines=356
prefix-files=11 pkg-selftest=19/19 release-selftest=11/11 tests-expected=5016 tests-skipped=4`.

**What 1.5 hosted cpu-s buys at the frozen unit prices** (process id 0.10, control 0.31,
in-step selftest 0.05, registry row 0.005):

| item | count | cpu-s |
|---|---|---|
| process ids in `runner-process` | 8 | 0.80 |
| `files` controls (`-so`, `-dylib`) | 2 | 0.62 |
| in-step selftests (`pkg-install`, `release-assets`) | 2 | 0.10 |
| registry rows (ids, controls, selftests, 2 important-gates rows, 1 claim) | ~14 | 0.07 |
| **total** | | **≈1.59** |

So 1.5 buys *eight process ids, two controls and two in-step selftests*, with the rows as
rounding; the priced line is short by ~0.09 (+6 %), inside the 25 % repricing threshold. It is
**not** enough for R6's `c-build-unlisted-host-source` (+0.31, which would take the phase to
1.90, +27 % and a mandatory reprice) — hence Pitfall 7's recommended deviation.

Predicted gate-summary delta for this phase:

```
controls      402 → 404        (the two files controls)
tests-expected 5016 → 5032     (8 ids × 2 presets)
pkg-selftest   19 → 20         (host-so-absent)
release-selftest 11 → 12       (the .so-member case)
gates          117 → 118       (TOOL-08's new gate name)
lines          356 → 358       (its two mutant rows)
claims          28 → 29        (if A4/Open Question 4 holds)
tracked       2748 → ≈2762     (host/, tests/host/, the two ids files, two control dirs, docs)
steps=38/38  faults=238  goldens=29  prefix-files=11  tests-skipped=4   (unchanged)
```

## Security Domain

`security_enforcement` is enabled (absent = enabled in `.planning/config.json`).

### Applicable ASVS Categories

| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V2 Authentication | no | — |
| V3 Session Management | no | — |
| V4 Access Control | no | — |
| V5 Input Validation | yes | the core's existing typed refusals (`IVS_REASON_*`) are the boundary; the shim adds `retro_load_game`'s three spans and `retro_run`'s target, both passed straight through so the core refuses |
| V6 Cryptography | no | no new cryptography; SHA-256 is the core's own (`core/src/common/ivs_sha256.c`) |
| V12 File/Resource | yes | the shim opens nothing (the import rule, Pattern 5); the harness opens only `argv` paths |

### Known Threat Patterns for a C shared object + `dlopen` harness

| Pattern | STRIDE | Standard Mitigation |
|---|---|---|
| a host path or build path baked into a shipped artifact | Information disclosure | the shim is not installed; `pkg-install`'s leak grep and the `pkg-selftest` `baked-path` case stay unchanged |
| a `.so` entering the tree or the release | Tampering | `binary-ext.txt` + `files-unlisted-binary` + `release-assets-binary-member` + the `git ls-files` DoD line |
| a `dlopen` of a path outside the build tree | Elevation | the harness takes the `.so` path from `$<TARGET_FILE:…>` (a CMake-provided absolute path), never from the environment |
| an over-broad export surface (an accidental ABI promise) | Tampering | the linker export list + the exact-set check, both directions |
| a `.so` that reads a file (a second content route, bypassing the runner's caps) | Information disclosure | the import rule: no `fopen`/`open`/`read`/`stat` in the shim's undefined set |
| unbounded host memory in the shim | Denial of service | the shim sizes its buffers from `ivs_info`'s documented per-frame formulas, exactly as `runner/exec.c:952` does; a `BUFFER_TOO_SMALL` from the core is a shim failure, never a truncation |

## Sources

### Primary (HIGH confidence — read or measured in this tree this session)

- `core/include/ivs/ivs_run.h:130-180,180-410` — run flags, stop reasons, request/report layouts, input event semantics, the input floor.
- `core/include/ivs/ivs_machine.h:59-69,150-260,320-400` — digest domains, `ivs_info` layout, load/reset rules.
- `core/src/machine/machine_digest.c:161-164` — the INPUT row includes `floor_tick`.
- `core/src/machine/machine_input.c:9-11,159-250` — the chain over absolute `at_tick`; `ivs_input_apply_due`.
- `core/src/machine/machine_run.c:204-206` — the floor is set at every unit start.
- `runner/exec.c:114-116,952-990,1038-1320,1400-1425,1428-1480` — domains, buffer planning, the run loop, the final digest collection, the aligned create.
- `runner/report.c:107-130,501-528` — the key table and `ivs_runner_golden_keep`.
- `runner/frame_export.c:1-40`, `runner/frame_export.h:1-33` — the one palette, its label and its API.
- `data/golden/demo-lamp-tone-pass.digests`, `data/golden/demo-lamp-tone-idle.digests`, `data/golden/diag-cpu-1.digests`.
- `data/scenarios/demo-lamp-tone-pass.ivss:17-29,53`.
- `tools/pack/CMakeLists.txt:29-98` — `diag-cpu-1.ivsc`, `lamp-tone.ivsc`, `boot-stub-2.exec` are build-tree images.
- `core/CMakeLists.txt:100-170`, `runner/CMakeLists.txt:1-50`, `cmake/IvsWarnings.cmake:1-30`, `CMakePresets.json` (`dev` sets `IVS_SAN=ON`).
- `tools/check/binary-ext.txt`, `tools/check/files.cmake:136-152`, `tools/check/steps/files.sh`, `tools/check/steps/c-build.sh:48-60`.
- `tools/check/steps/pkg-install.sh:455-478,690-760`, `data/expected/pkg-selftests.ids`, `data/expected/pkg-install-files.ids`.
- `tools/check/steps/release-assets.sh:1-45`, `tools/check/release-assets.sh:1-40,150-240`, `data/expected/release-assets-selftests.ids`.
- `tools/check/core-symbols.sh:1-45`, `tools/check/core-symbols-allow.txt`, `tools/check/core-symbols-deny.txt`.
- `tools/check/steps/important-gates.sh:1-95`, `data/expected/important-gates.ids` (117 gates, 356 rows).
- `tools/check/steps/text.sh:37-60` — the claim-word tripwire.
- `tests/CMakeLists.txt:735-746`, `tests/runner-process-tap.sh:113-135,2215-2267`, `data/expected/tests/runner-process.ids:150-209`.
- `.planning/ci-runs.tsv` — 99 usable success rows; the medians above are arithmetic over them.
- `build/gate-linux.log` last line — the baseline token line.
- Measured probes (throwaway, `/tmp`, not committed): three drive shapes; the frame-end stop; the empty-poll run; the delivered-slot pixel probes; `nm`/link experiments on Darwin 25.6 (Apple clang 21) and `ubuntu:24.04` (gcc, docker).

### Secondary (MEDIUM confidence — project records read, not re-measured)

- `.planning/MILESTONE-CONTEXT.md` §2–§5; `.planning/ops/GUARDRAILS-v0.3.md` §1–§3; `.planning/ops/GUARDRAILS-v0.2.md` R1–R11.
- `.planning/research/v0.3.0-scoping-2-gate-cost.md`, `-3-rights-format-packaging.md`, `-4-verification.md`.
- `.planning/phases/15-…/15-CONTEXT.md`, `15-DISCUSSION-LOG.md`; `.planning/PROJECT.md` D-48, D-49.
- `docs/packaging.md:20-40`, `docs/runner.md:96,253-266`, `.planning/ops/RELEASE-OPS.md`.

### Tertiary (LOW confidence — not used for any decision)

- None. No web search was used: every claim above is either read from this tree or measured
  in this session.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — nothing is added; the toolchain versions are measured.
- Architecture (the drive, the ABI subset, the export mechanism): HIGH — the nine-line
  equality is measured three ways, and the linker mechanism is measured on both platforms.
- Packaging rows: HIGH — every retarget has a named precedent in the tree.
- Pitfalls: HIGH for 1, 2, 3, 4, 5, 6, 8, 9 (each measured or quoted); MEDIUM for 7 (a
  recommended deviation from R6, argued from the link/export coverage).

**Research date:** 2026-11-26
**Valid until:** 30 days (a stable stack, no fast-moving dependency); the measured figures
(`nominal_frame_ticks`, slot sizes, digests) move only if a profile field or the core changes,
which would be a V12 retarget anyway.
