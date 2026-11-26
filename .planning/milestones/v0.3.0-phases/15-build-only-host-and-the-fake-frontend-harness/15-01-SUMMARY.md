---
phase: 15-build-only-host-and-the-fake-frontend-harness
plan: 01
subsystem: host-seam
tags: [shared-object, dlopen, export-list, version-script, digest-equality, integrity-scope]

requires:
  - phase: 14-cartridge-route-and-real-content-conformance
    provides: the build-tree images (lamp-tone.ivsc, boot-stub-2.exec), the runner-process suite shape and its id inventory
  - phase: 07
    provides: the demo-lamp-tone-pass scenario and its nine-line golden
provides:
  - host/ — the project-authored libretro-shaped subset, the nine entries, and a build-only SHARED target with a linker-enforced export list
  - tests/host/ivs_host_harness.c — a fake frontend that dlopens the object and reproduces the runner's nine digest lines
  - data/expected/host-symbols.ids — one contract file read by both the linker and the checker
  - data/expected/host-imports.ids + tools/check/host-symbols.sh — the export/import refusal, both directions
  - two runner-process ids: host.digests and host.exports
affects: [15-02 (the three vacuity guards, the packaging retargets, the claim), 15-03 (the container leg and the phase close), 16 (the release line that says no host artifact ships)]

actuals:
  tokens: 17720
  tasks: 2
  commits: 2

tech-stack:
  added: []   # nothing installed, nothing vendored (D-48): the only new dependency is the platform's own dlopen
  patterns:
    - "one contract file (data/expected/host-symbols.ids) read by the build AND the check, so artifact and pin cannot drift"
    - "the linker, not a convention, fixes an export surface: -exported_symbols_list on Darwin, --version-script with local:* elsewhere"
    - "a fake frontend as evidence: the harness is not linked against ivs_core, so what it proves is the seam"

key-files:
  created:
    - host/CMakeLists.txt
    - host/libretro_shim.h
    - host/ivs_retro_shim.c
    - tests/host/ivs_host_harness.c
    - data/expected/host-symbols.ids
    - data/expected/host-imports.ids
    - tools/check/host-symbols.sh
  modified:
    - CMakeLists.txt
    - tests/CMakeLists.txt
    - tests/runner-process-tap.sh
    - data/expected/tests/runner-process.ids
    - tools/check/integrity.cmake

key-decisions:
  - "IVS_API is defined for the shim target alone (target_compile_definitions), so the installed headers keep it empty and D-35's static package stands at eleven files."
  - "The palette is runner/frame_export.c compiled into the object and the converter is ivs_refpcm linked: one palette, one converter, no digest recomputed anywhere in host/."
  - "The harness takes the object's path from a CMake generator expression, never from the environment (T-15-01); a missing argument is `not ok`, never a skip."
  - "The `c-build` module row R6 suggested is deliberately NOT taken: a host TU that is not compiled either breaks the .so link or removes one of the nine exports, which host-symbols.ids refuses by name, so the failure class is covered at zero cost. Fallback if a verifier disagrees: R6's set (module row + ivs-host-sources.txt + toy source list + c-build-unlisted-host-source) at +0.31 cpu-s, which reprices the phase to 1.90 (+27 % over the frozen 1.5)."
  - "`imports=` in the checker reports DISALLOWED imports (the remainder after both allowlists and every prefix), so it is 0 on a correct object and nonzero exactly when a forbidden name appeared; the allowed-import count varies with fortify and stack-protector forms and is not evidence."
  - "The pixel probe verifies inverse-uniqueness (every delivered pixel equals exactly one of the 17 pinned-distinct palette triples) rather than an index it cannot see: the declared video_refresh callback carries XRGB only."

requirements-completed: [TOOL-08]

coverage:
  - id: D1
    description: "A build-only shared object libivs_retro exists in both presets, carries no install rule, and exports exactly the nine names of data/expected/host-symbols.ids."
    requirement: TOOL-08
    verification:
      - kind: integration
        ref: "cmake --build --preset dev && cmake --build --preset release; nm -P -g --defined-only build/*/host/libivs_retro.dylib normalized and cmp'd against the ids"
        status: pass
      - kind: integration
        ref: "sh tests/runner-process-tap.sh ... (case runner-process.host.exports) -> ok 127"
        status: pass
    human_judgment: false
  - id: D2
    description: "A fake frontend dlopens the object, drives the project's own lamp-tone.ivsc to tick 806568, and prints the nine digest lines byte-for-byte equal to data/golden/demo-lamp-tone-pass.digests."
    requirement: TOOL-08
    verification:
      - kind: integration
        ref: "IVS_TEST_OUT_DIR=<dir> ctest --preset dev -R '^runner-process$' (case runner-process.host.digests) -> ok 126, trailer executed=137 skipped=2 failed=0"
        status: pass
      - kind: integration
        ref: "sh tools/gate.sh -> gate: PASS (both presets, both suites)"
        status: pass
    human_judgment: false
  - id: D3
    description: "host/ is covered by the integrity step's ASCII and SPDX rules in the commit that creates the directory."
    requirement: TOOL-08
    verification:
      - kind: other
        ref: "scratch probe: SPDX line deleted from host/ivs_retro_shim.c -> `-- FAIL integrity-missing-spdx: host/ivs_retro_shim.c`; restored with git checkout and cmp'd byte-identical"
        status: pass
    human_judgment: false

duration: 17min
completed: 2026-11-26
status: complete
---

# Phase 15 Plan 01: Build-Only Host and the Fake-Frontend Harness (the tracer) Summary

**The seam is open end to end: a linker-export-enforced `libivs_retro` that a `dlopen` frontend drives to tick 806568 reproduces the runner's nine digest lines byte for byte on the project's own `lamp-tone.ivsc`, as two committed `runner-process` ids and no new gate step, fault or golden.**

## Performance

- **Duration:** ~17 min measured from the baseline gate (`2026-11-26T05:35:14Z`) to the metadata commit; the pre-baseline reading of the plan, context and research is not counted
- **Tasks:** 2/2
- **Files:** 12 paths (7 created, 5 modified), 1,416 insertions and 9 deletions
- **Commits:** 2 task commits (`090f6fb`, `7fc8011`) plus this plan's metadata commit

## Start line, recomputed rather than quoted

`sh tools/gate.sh` at `6a36d5b` (exit 0) printed, verbatim:

```
steps=38/38 controls=402/402 fault-runs=238 faults=238/238 goldens=29 gates=117 lines=356 prefix-files=11
pkg-selftest=19/19 release-selftest=11/11 tests-expected=5020 tests-executed=5016 tests-skipped=4
tests-missing=0 claims=28 provisional=27 claim-refs=26 tracked=2756 binaries-listed=0 core-imports=7
public-symbols=19 defined=141 runner-files=18 mode=tree
```

**Every figure in the plan's prediction table matched this start line**, including `tracked=2756` — the first plan in four phases whose baselines were not a few files stale. Nothing was re-blessed.

## Gate line at each task commit

| Token | Start | After T1 | After T2 | Predicted after T2 | Verdict |
|---|---|---|---|---|---|
| `steps` | 38/38 | 38/38 | 38/38 | 38/38 | invariant (§2.1) |
| `controls` | 402/402 | 402/402 | 402/402 | 402/402 | V6's two controls are 15-02's |
| `faults` | 238/238 | 238/238 | 238/238 | 238/238 | invariant |
| `goldens` | 29 | 29 | 29 | 29 | invariant |
| `tests-expected` | 5020 | 5022 | 5024 | 5024 | as predicted |
| `tests-executed` | 5016 | 5018 | 5020 | 5020 | as predicted |
| `tests-skipped` | 4 | 4 | 4 | 4 | no new `skip-ok` row |
| `tests-missing` | 0 | 0 | 0 | 0 | — |
| `gates` | 117 | 117 | 117 | 117 | 15-03's change |
| `lines` | 356 | 356 | 356 | 356 | 15-03's change |
| `prefix-files` | 11 | 11 | 11 | 11 | no install rule on the object |
| `pkg-selftest` | 19/19 | 19/19 | 19/19 | 19/19 | 15-02 adds the twentieth |
| `release-selftest` | 11/11 | 11/11 | 11/11 | 11/11 | 15-02 adds the twelfth |
| `claims` / `provisional` | 28/27 | 28/27 | 28/27 | 28/27 | the claim is 15-03's |
| `binaries-listed` | 0 | 0 | 0 | 0 | no tracked `.so`/`.dylib` |
| `core-imports` / `public-symbols` / `defined` | 7/19/141 | 7/19/141 | 7/19/141 | unchanged | the core archive did not move |
| `runner-files` | 18 | 18 | 18 | unchanged | no runner TU changed |
| `tracked` / `files` / `hygiene-files` | 2,756 | 2,761 | 2,763 | 2,761 → 2,763 | as predicted, +5 then +2 |
| `scanned` | 689 | 694 | 695 | **not in the table** | see below |

`scanned=` is the `denylist` step's count of in-scope tracked files; it moves with `tracked` (+5, then +2) and for no other reason. The plan's table did not list it — the one token that differed from the prediction, and its cause is the same five files, not a new mechanism.

## The exported symbol set, quoted once

`nm -P -g --defined-only`, normalized (one leading Darwin underscore stripped, `@version` dropped, sorted), from **both** presets:

```
ivs_retro_digest
retro_api_version
retro_deinit
retro_get_system_av_info
retro_init
retro_load_game
retro_run
retro_set_environment
retro_unload_game
```

Nine names, and `cmp` against the normalized `data/expected/host-symbols.ids` is byte-equal on `build/dev` and `build/release` alike. The object carries no `ivs_` name but the pinned `ivs_retro_digest`, and no palette word: the palette entry `ivs_frameppm_palette` is compiled in from `runner/frame_export.c` with hidden visibility and never reaches the export list.

`host/CMakeLists.txt` contains zero `install(` occurrences and `core/CMakeLists.txt` is untouched; `prefix-files=11` in both presets.

## The measured drive

`build/dev/tests/ivs_host_harness build/dev/host/libivs_retro.dylib build/dev/images`:

```
av.base_width 159 / av.base_height 96 / av.max_width 159 / av.max_height 96
av.frame_phase_ticks 0 / av.nominal_frame_ticks 59736
av.fps_num 3579545 / av.fps_den 59736 / av.sample_rate 48000
env.refused 1 / env.accepted 1 / load.images 1
frames 13
video.plane.bytes 198432
audio.frames 10815
audio.bytes 21630
pixel.probes ok 198432
```

- **13 frames, ending at exactly 806568.** The drive is one call per frame boundary (`nominal_frame_ticks * k`, k = 1..13, ending at 776568) and then one call to 806568, which completes no frame. The plan's warning is closed by measurement: a frame-only stop at 776568 moves seven lines, and the final call is what makes them equal.
- **`video.plane.bytes 198432` = 13 × 15264**, the 159 × 96 index plane the callback receives — deliberately *not* the golden's `output.video.bytes 198848`, which counts 13 *slots* including the 32-byte header. The plan's note closed that trap before it was stepped in.
- **`pixel.probes ok 198432`**: every delivered pixel equals exactly one of the 17 pinned-distinct palette triples, so the conversion is a palette image and not an approximation.
- **The nine lines**, byte-for-byte equal to `data/golden/demo-lamp-tone-pass.digests`, printed in the golden's order:

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

The observed domain set equals the golden's eight observed domains (the composite apart), which is the same statement as "every line non-`-`". `grep -rn 'sha256\|SHA-256' host/ tests/host/` returns one line and it is a comment: no digest is recomputed.

`--empty-poll` (implemented now, judged by 15-02) drives one call to 806568 with no events and gives `digest.input 27da2e1f98668440bff121cbb89cc30d4f959332cf9fa3d12cf15e855c15dda0` — the empty-chain value the research measured — while `digest.images` stays equal to the pass golden's.

## The two ids

`IVS_TEST_OUT_DIR=<dir> ctest --preset dev -R '^runner-process$'`:

```
ok 126 runner-process.host.digests
ok 127 runner-process.host.exports
# suite=runner-process executed=137 skipped=2 failed=0     (after T1)
# suite=runner-process executed=138 skipped=2 failed=0     (after T2)
```

137 = the 136 executed today + T1's id; 138 adds T2's. No `not ok` line in either run, and the same two ids are `ok` under the release preset inside `sh tools/gate.sh`.

`runner-process.host.digests` requires a positive `frames` integer **before** it compares anything (`frames-zero` otherwise), then makes exactly the two comparisons `check_one_golden` makes — the sorted `digest.` line set and the observed domain set. `runner-process.host.exports` requires `-- host-symbols pass exported=9 imports=0` by exact match and proves the missing-name direction live inside the case: a scratch ids copy with `ivs_not_defined` appended is refused with `-- FAIL host-symbols-missing: ivs_not_defined`.

## The integrity scope widening, and why no control directory was added

`tools/check/integrity.cmake` gained `host` in the ASCII rule's directory alternation, in the SPDX `(c|h|sh|cmake)` group and in the CMakeLists group. `tests/host/` was already covered by the `tests/` alternative; `host/` was a new top-level directory and both rules are directory-anchored, so without the widening the phase's own sources would escape the licence and ASCII rules.

**This is a scope widening, not a new check**, and it was proved by a scratch probe rather than by a control: the SPDX line was deleted from `host/ivs_retro_shim.c`, the `integrity` step was run, and it printed

```
-- FAIL integrity-missing-spdx: host/ivs_retro_shim.c
```

The file was then restored with `git checkout -- host/ivs_retro_shim.c` and `cmp` confirmed it byte-identical; the tree was clean afterwards. The mechanisms (`integrity-missing-spdx`, `integrity-non-ascii`) stay proved by the eight committed controls (`integrity-missing-spdx-c`, `-runner`, `-tools-c`, `-tools-pcm`, `-tools-asm`, `integrity-non-ascii-c`, `-runner`, `integrity-nul-byte`). No control directory was added because one costs 0.31 cpu-s and would take the phase past the 25 % repricing line; the phase's three committed stand-ins are criterion 3's, in plan 15-02.

## The `c-build` deviation (deliberate, priced)

No `c-build` module row was added for `host/`. `c-build` walks seven directories (`tools/check/steps/c-build.sh:48-51`) and a tracked `host/*.c` it does not compile is invisible to it — but unlike a tool directory, a host TU that is not compiled either breaks the `.so` link with an undefined reference or removes one of the nine exports, which `data/expected/host-symbols.ids` refuses by name. The failure class is covered at zero cost (research Pitfall 7). R6's `c-build-unlisted-host-source` would add 0.31 cpu-s and put the phase at 1.90, +27 % over the frozen 1.5, forcing a reprice the milestone did not price. A verifier who disagrees has the fallback in hand: the module row plus `ivs-host-sources.txt`, the toy source list and the control, at +0.31 cpu-s.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] `ivs_frameppm_palette` returns 1 on success, not `IVS_FRAMEPPM_OK`**
- **Found during:** Task 1, first harness run (frames 0, every digest of a fresh machine)
- **Issue:** `IVS_FRAMEPPM_OK` is 0 and belongs to `ivs_frameppm_bound`/`_convert`; the palette entry point returns 1 and fills `rgb`. The shim and the harness both compared against `IVS_FRAMEPPM_OK`, so every frame was refused at the first pixel and `retro_run` returned 0 before delivering anything.
- **Fix:** both call sites compare `!= 1`.
- **Files modified:** `host/ivs_retro_shim.c`, `tests/host/ivs_host_harness.c`
- **Verification:** `frames 13`, `pixel.probes ok 198432`, all nine lines equal the golden
- **Commit:** `090f6fb`

**2. [Rule 1 - Bug] `retro_init` cleared the installed host**
- **Found during:** Task 1, same run (`env.accepted 1` yet every run refused)
- **Issue:** `retro_init` zeroed `shim.host` and `have_host`, so the callbacks installed before it (the order the plan's own assertions imply) were dropped and `retro_run` refused at its first guard.
- **Fix:** `retro_init` no longer clears the host (a frontend may install its callbacks before creating the instance); the harness now calls `retro_init` before `retro_set_environment`, which is also the shape's own convention.
- **Files modified:** `host/ivs_retro_shim.c`, `tests/host/ivs_host_harness.c`
- **Verification:** the full measured drive above
- **Commit:** `090f6fb`

**3. [Rule 1 - Plan defect] `host/CMakeLists.txt` could not satisfy the plan's own `install(` assertion**
- **Found during:** Task 1, running the plan's `<automated>` verbatim
- **Issue:** the plan's verify requires `grep -c -F -e 'install(' host/CMakeLists.txt` to be 0, and the comments the plan asks for ("no `install()` line anywhere in this file") contained the literal. A task's gate must be satisfiable at its own commit (PLAN-LESSONS 1).
- **Fix:** the comments say "install rule" instead of the call form; the meaning is unchanged and the assertion is now live.
- **Files modified:** `host/CMakeLists.txt`
- **Commit:** `090f6fb`

**4. [Rule 1 - Plan defect] Nothing carried the fake frontend's own path**
- **Found during:** Task 1, writing the suite case
- **Issue:** the plan fixes the seventh argument as `$<TARGET_FILE:ivs_retro_shim>` and says to register nothing new with `add_test` — but the suite must *run* `ivs_host_harness`, and no argument named it.
- **Fix:** the shim stays the seventh argument exactly as the plan and the `must_haves` key-link state; an optional **eighth** argument (`$<TARGET_FILE:ivs_host_harness>`) carries the executable. A missing eighth is `not ok` with that reason, never a skip.
- **Files modified:** `tests/CMakeLists.txt`, `tests/runner-process-tap.sh`
- **Commit:** `090f6fb`

**5. [Rule 1 - Plan defect] The shared object is not executable**
- **Found during:** Task 1, argument validation
- **Issue:** the plan asks for the shim argument to be "required to be executable"; a `.dylib` is mode 0644 on Darwin and would fail that test on a correct build.
- **Fix:** the case requires `-f` (a regular file) for the shim and keeps `-x` for the harness executable.
- **Files modified:** `tests/runner-process-tap.sh`
- **Commit:** `090f6fb`

**6. [Rule 2 - Correctness] The pixel probe cannot see the index plane**
- **Found during:** Task 1, implementing the probe
- **Issue:** the plan asks the harness to "map back to exactly the index the plane carried", but the declared `video_refresh` callback carries XRGB only — the plane is never handed to the frontend, so the literal comparison has no input.
- **Fix:** the strongest implementable form: every delivered pixel must equal **exactly one** of the 17 palette triples (pinned distinct by `tests/runner/runner_exec.c`), counted as `pixel.probes ok <n>` with `n` required to be positive. Measured n = 198432 = every delivered pixel of all 13 frames.
- **Files modified:** `tests/host/ivs_host_harness.c`
- **Commit:** `090f6fb`

**7. [Rule 2 - Correctness] A host missing any callback is refused, not only one with a NULL poll**
- **Found during:** Task 1, writing the acceptance check
- **Issue:** the plan names only `input_poll`; a host with a NULL `video_refresh` would have been accepted and then dereferenced by the first delivered frame.
- **Fix:** `retro_set_environment` refuses a host missing any of the three callbacks, and the header states it.
- **Files modified:** `host/ivs_retro_shim.c`, `host/libretro_shim.h`
- **Commit:** `090f6fb`

**Total deviations:** 7 auto-fixed (5 Rule 1 — two real bugs, three plan defects that made a task's own gate unsatisfiable or its required input missing; 2 Rule 2 — two correctness gaps the plan's wording could not express). **Impact:** none on the plan's contract: the nine names, the two ids, the export enforcement, the import allowlist and every gate token are exactly as planned.

## Auth gates

None.

## Known stubs

None. `grep -rn 'TODO\|FIXME\|coming soon\|not available' host/ tests/host/` is empty, no component is wired to empty data, and every value the harness prints is measured from the object.

## Threat surface scan

No new surface beyond the plan's `<threat_model>`. All six T-15-* mitigations are implemented: the `dlopen` path is CMake-provided (T-15-01), the export surface is linker-enforced and checked in both directions (T-15-02), the import allowlist makes "opens no file" static (T-15-03), `host/` has no install rule and no tracked binary (T-15-04), the shim sizes its buffers from `ivs_info`'s formulas and refuses rather than truncating (T-15-05), and no digest is recomputed (T-15-06). No package was installed and no header vendored (T-15-SC).

## Verification results

| Check | Result |
|---|---|
| `cmake --preset dev && cmake --build --preset dev` | pass; `build/dev/host/libivs_retro.dylib`, nine exported names |
| `cmake --preset release && cmake --build --preset release` | pass; `build/release/host/libivs_retro.dylib`, the same nine |
| `IVS_TEST_OUT_DIR=<dir> ctest --preset dev -R '^runner-process$'` | pass; both host ids `ok`, trailer `executed=138 skipped=2 failed=0`, zero `not ok` |
| `sh tools/check/host-symbols.sh --allow-prefix __asan_ --allow-prefix __ubsan_ --allow-prefix __sanitizer_ <so>` (no ids path) | `-- host-symbols pass exported=9 imports=0`, no `-- FAIL` |
| the same with a scratch ids copy carrying `ivs_not_defined` | exit 1 with `-- FAIL host-symbols-missing: ivs_not_defined` |
| the same with `retro_deinit` removed from the ids | exit 1 with `-- FAIL host-symbols-extra: retro_deinit` |
| the same with no `--allow-prefix` (dev object) | exit 1 with 41 `host-symbols-import` lines — the prefixes are load-bearing, and `fopen`/`open`/`read`/`stat` are matched by none of them |
| `sh tools/gate.sh` | exit 0, `gate: PASS`, no `-- FAIL`, every predicted token present |
| `sh tools/hygiene.sh --tree` | `-- hygiene pass mode=tree hygiene-files=2763` |
| `git ls-files \| grep -E '[.](ivsc\|bin\|rom\|so\|dylib\|cfg\|ivsn)$'` | empty |
| `grep -rn 'sha256\|SHA-256' host/ tests/host/` | one comment line, no recomputation |

**The container leg is not taken in this plan.** `sh tools/gate-linux.sh` runs once at the phase close (plan 15-03), one container at a time (PLAN-LESSONS 18). This plan's Linux half is the `--version-script` branch of `host/CMakeLists.txt`, which the close's container run exercises; it is **not** claimed green here.

## Next

Ready for `15-02`: the three vacuity guards (`host.empty-poll`, the two judge stand-ins), the packaging retargets (`binary-ext.txt`, the two `files` controls, the `pkg-install` selftest, `docs/packaging.md`, `RELEASE-OPS.md`, one `release-assets` case), and the four remaining `runner-process` ids.

## Self-Check: PASSED

- All seven created paths exist on disk.
- Both task commits (`090f6fb`, `7fc8011`) are ancestors of `HEAD`.
- Every `<automated>` block was run verbatim with its `<fails_when>` checked; the two that failed at first (T1's `install(` assertion and the palette return value) were fixed before the commit and re-run green.
- `sh tools/gate.sh` exits 0 with `gate: PASS` and no `-- FAIL` line at the final task commit.

## Gate line at this metadata commit

`sh tools/gate.sh` at `11f2d57`, exit 0, zero `-- FAIL` lines, byte-for-byte the tokens the plan
predicted for the metadata commit:

```
steps=38/38 controls=402/402 faults=238/238 goldens=29 gates=117 lines=356 prefix-files=11
pkg-selftest=19/19 release-selftest=11/11 tests-expected=5024 tests-executed=5020 tests-skipped=4
tests-missing=0 claims=28 provisional=27 tracked=2764 files=2764 hygiene-files=2764 scanned=695
binaries-listed=0 core-imports=7 public-symbols=19 defined=141 runner-files=18 mode=tree
```

`tracked`/`files`/`hygiene-files` read 2,764 and not 2,763 because this SUMMARY is itself a tracked
file — the exact figure the plan's table named ("this plan's own `SUMMARY.md` makes the metadata commit
read 2,764"). `scanned` stays at 695 because the `denylist` step excludes `.planning/`.
