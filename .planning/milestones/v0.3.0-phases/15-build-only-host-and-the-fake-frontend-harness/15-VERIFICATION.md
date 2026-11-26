---
phase: 15-build-only-host-and-the-fake-frontend-harness
verified: 2026-11-26T08:23:16Z
status: passed
score: 13/13 must-haves verified
covered_files:
  - ".planning/phases/15-build-only-host-and-the-fake-frontend-harness/15-01-PLAN.md"
  - ".planning/phases/15-build-only-host-and-the-fake-frontend-harness/15-01-SUMMARY.md"
  - ".planning/phases/15-build-only-host-and-the-fake-frontend-harness/15-02-PLAN.md"
  - ".planning/phases/15-build-only-host-and-the-fake-frontend-harness/15-02-SUMMARY.md"
  - ".planning/phases/15-build-only-host-and-the-fake-frontend-harness/15-03-PLAN.md"
  - ".planning/phases/15-build-only-host-and-the-fake-frontend-harness/15-03-SUMMARY.md"
  - "CHANGELOG.md"
  - "CMakeLists.txt"
  - "data/claims.json"
  - "data/expected/claims.ids"
  - "data/expected/controls/rights.ids"
  - "data/expected/host-imports.ids"
  - "data/expected/host-symbols.ids"
  - "data/expected/important-gates.ids"
  - "data/expected/pkg-selftests.ids"
  - "data/expected/release-assets-selftests.ids"
  - "data/expected/tests/runner-process.ids"
  - "docs/evidence-tiers.md"
  - "docs/packaging.md"
  - "host/CMakeLists.txt"
  - "host/ivs_retro_shim.c"
  - "host/libretro_shim.h"
  - "tests/CMakeLists.txt"
  - "tests/host/ivs_host_harness.c"
  - "tests/runner-process-tap.sh"
  - "tools/check/binary-ext.txt"
  - "tools/check/controls/_good/tree/.planning/REQUIREMENTS.md"
  - "tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md"
  - "tools/check/controls/_good/tree/data/expected/important-gates.ids"
  - "tools/check/controls/files-unlisted-binary-dylib/CHECK"
  - "tools/check/controls/files-unlisted-binary-dylib/EXPECT"
  - "tools/check/controls/files-unlisted-binary-dylib/MAKE"
  - "tools/check/controls/files-unlisted-binary-so/CHECK"
  - "tools/check/controls/files-unlisted-binary-so/EXPECT"
  - "tools/check/controls/files-unlisted-binary-so/MAKE"
  - "tools/check/controls/important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids"
  - "tools/check/controls/important-gates-bad-req-phase6/overlay/data/expected/important-gates.ids"
  - "tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md"
  - "tools/check/controls/important-gates-req-uncovered-phase5/overlay/data/expected/important-gates.ids"
  - "tools/check/controls/important-gates-req-uncovered-phase6/overlay/data/expected/important-gates.ids"
  - "tools/check/controls/important-gates-req-uncovered-phase7/overlay/data/expected/important-gates.ids"
  - "tools/check/controls/important-gates-uncovered-gate/overlay/data/expected/important-gates.ids"
  - "tools/check/controls/important-gates-unknown-id-phase5/overlay/data/expected/important-gates.ids"
  - "tools/check/controls/important-gates-unknown-id-phase6/overlay/data/expected/important-gates.ids"
  - "tools/check/controls/important-gates-unknown-id/overlay/data/expected/important-gates.ids"
  - "tools/check/host-symbols.sh"
  - "tools/check/integrity.cmake"
  - "tools/check/steps/hygiene-history.sh"
  - "tools/check/steps/important-gates.sh"
  - "tools/check/steps/pkg-install.sh"
  - "tools/check/steps/release-assets.sh"
  - "tools/pcm/CMakeLists.txt"
covered_digest: "v4:sha256:892a972d831f7e52a8b10d55cc1f6bf7ce8cd72619b5bc0a813ef4677269f619"
behavior_unverified: 0
overrides_applied: 0
---

# Phase 15: Build-Only Host and the Fake-Frontend Harness Verification Report

**Phase Goal:** The core can be driven in real time by a host that loads it as a shared object, and the
proof is a headless frontend that reproduces the runner's nine digest lines — not a window.
**Verified:** 2026-11-26T08:23:16Z at `0fc4d33` (the merge of PR #34, i.e. the tree carrying the five
post-review fixes)
**Status:** passed
**Re-verification:** No — initial verification

**What was re-derived rather than read.** Every figure below was produced in this session on this tree:
the full gate under `sh` and under `dash` (byte-identical `tail -1`, 1509 bytes, exit 0 both), the full
gate a third time inside Ubuntu 24.04 on amd64 (`gate: PASS`, `gate-time total=288`, exit 0), the
`runner-process` suite directly (`executed=147 skipped=2 failed=0`), both built shared objects against
`data/expected/host-symbols.ids`, both directions of the export check by hand, the import limb by hand,
the harness in pass and `--empty-poll` modes against both committed goldens, a real `cmake --install`
counted file by file, a synthetic release archive with and without a `.so` member, and the cost
checkpoint recomputed in Python from the committed TSV. No SUMMARY figure was accepted as evidence.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | **Criterion 1.** One shared object is built outside `core/` and outside the install list, exporting only its shaped entry points and no `ivs_` or palette symbol; the export surface is refused by name if it grows. | ✓ VERIFIED | `nm -P -g --defined-only` on `build/dev/host/libivs_retro.dylib` **and** `build/release/host/libivs_retro.dylib` yields exactly nine names, `cmp`-equal to the normalized `data/expected/host-symbols.ids` in both presets; `host-symbols.sh` → `-- host-symbols pass exported=9 imports=0` exit 0 on both. The export list is **generated** from that same ids file into the linker (`-exported_symbols_list` / `--version-script` + `local: *`), so an unlisted name cannot exist in a built object. `grep -rn 'install(' host/` = 0; `core/CMakeLists.txt` is not in the phase diff; `prefix-files=11`. Both directions proved live by hand: appended `ivs_not_defined` → `-- FAIL host-symbols-missing: ivs_not_defined` (exit 1); `retro_run` deleted → `-- FAIL host-symbols-extra: retro_run` (exit 1). The palette limb fires too: a synthetic object exporting `ivs_palette_dump` → `host-symbols-extra` **and** `host-symbols-palette`. The one `ivs_` name is the pinned harness-only `ivs_retro_digest` (CONTEXT resolution; the exact-set comparison is what makes any other `ivs_` name an `extra`). |
| 2 | The import allowlist makes "the shim opens no file and reads no clock" a static fact, and a forbidden import is refused by name. | ✓ VERIFIED | `nm -P -u` on the release object → `__memcpy_chk __memmove_chk __stack_chk_fail __stack_chk_guard bzero free malloc memcpy memmove memset realloc`, no file/clock/env/thread/random/assert/abort/libm name; the dev object adds only `__asan_*`/`__ubsan_*`. The limb is live: pointed at `build/dev/tests/ivs_host_harness` the same checker refuses 43 names including `fopen`, `fread`, `fclose`, `dlopen`, `dlsym`. The four ELF crt stubs are allowlisted **by name**, not by prefix (see Anti-Patterns / container-leg note). |
| 3 | **Criterion 2.** An automated fake frontend `dlopen`s it, loads one project cartridge with the boot stub and zero GROM, drives the pass scenario's six events, and prints nine digest lines equal byte-for-byte to `data/golden/demo-lamp-tone-pass.digests`. | ✓ VERIFIED | `build/dev/tests/ivs_host_harness build/dev/host/libivs_retro.dylib build/dev/images` exit 0; `diff` of the nine sorted `digest.` lines against the golden is **empty**; `frames 13`, `frame.geometry ok 13`, `video.plane.bytes 198432` (= 13 × 159 × 96), `audio.frames 10815`, `pixel.probes ok 198432`, `unload.digest.refused 1`. The six `input.words` ticks (100000/220000/340000/460000/580000/700000) and words are the scenario's own six lines in `data/scenarios/demo-lamp-tone-pass.ivss`, resolved through `data/expected/input-bits.ids` by the suite, not typed. `tests/CMakeLists.txt` links the harness against `ivs_warnings ivs_san ${CMAKE_DL_LIBS}` only — **not** `ivs_core` — so the seam is what is proved. The last call is `run(806568)`, a mid-frame stop, in both modes. |
| 4 | **Criterion 3.** The equality is guarded three ways: every line non-`-` with the observed set equal; a second id with an empty poll giving a **different** set; two stand-ins refused (`host-vacuous`, `host-digest-diff`). Each guard must be shown able to fail. | ✓ VERIFIED | (a) `check_digest_lines` compares the sorted `digest.` line set **and** the observed domain set (golden's `# domains:` line, 8 observed) — the pass run matches both. (b) My own `--empty-poll` run: seven of nine lines differ from the pass golden, `digest.images` **equal**, `digest.input` different, and all eight non-input lines the idle golden carries match it one by one (8/8) — read from the golden, never from a list in the case. (c) Both stand-ins are committed cases judged by the **same** `check_host`, `ok 138 judge.host-vacuous` (`frames-zero`) and `ok 139 judge.host-digest-diff` (`digest-diff`); `self_control` now requires the observed token set to equal the declared set exactly (WR-02), and the guard **on** the guard is `ok 140 judge.host-two-limb`, which fails a two-limb stand-in and a declaration claiming an unexercised limb. Zero `not ok` in the suite. |
| 5 | **Criterion 4a.** The host holds no palette of its own: it links `runner/frame_export.c` rather than copying it. | ✓ VERIFIED | `host/CMakeLists.txt` compiles `${PROJECT_SOURCE_DIR}/runner/frame_export.c` into the `SHARED` target and links `ivs_refpcm`; `grep -rn 'palette\|PALETTE' host/ tests/host/` finds only calls to `ivs_frameppm_palette` and comments — **no array**. The dynamic half is the pixel probe: every delivered pixel must equal exactly one of the 17 pinned-distinct triples, and its count must equal `video.plane.bytes` (198432 = 198432), so a copied-and-drifted palette fails `host.frames`. |
| 6 | **Criterion 4b.** The host opens no file but the images named on its command line. | ✓ VERIFIED | The shim's undefined set (truth 2) contains no file, clock or environment name — a static fact, shown able to fire. The harness's only `fopen` call sites are `read_span(content_dir, "boot-stub-2.exec")` and `read_span(content_dir, "lamp-tone.ivsc")`, both under the content directory it is given as `argv[2]`; the zero GROM is `calloc`'d, never read; `grep -rn 'getenv\|environ' host/ tests/host/` → nothing; the `dlopen` path is `argv[1]`, supplied by CMake's `$<TARGET_FILE:…>`. |
| 7 | **Criterion 4c.** The installed package is unchanged at eleven files, with a selftest that the shared object is absent. | ✓ VERIFIED | `cmake --install build/release --prefix <tmp>` → **11 files**, `find` for `*.so|*.dylib|*.dll|*.bundle` → **0**, and the installed list `diff`s clean against `data/expected/pkg-install-files.ids`. `pkg-install` selftest `host-so-absent` plants `lib/libivs_retro.so` in a copy of the prefix and requires `-- FAIL pkg-inventory-extra: lib/libivs_retro.so`; `pkg-selftest=20/20` in the gate. `release-assets` selftest `so-member` plants one in an archive **whose own list names it** and requires `release-assets-binary-member` alone; `release-selftest=12/12`. |
| 8 | **Criterion 5.** `so` and `dylib` enter `tools/check/binary-ext.txt` with the controls that refuse a tracked shared object, and the gate ends `steps=38/38` with the predicted delta. | ✓ VERIFIED | `binary-ext.txt` carries `so dylib dll bundle` as one list read by the files step, the release member check and the hygiene rule. `files-unlisted-binary-so` and `-dylib` are committed, inventoried in `data/expected/controls/rights.ids`, and both print `control ok` in my gate run; `controls=404/404`, `binaries-listed=0`, `git ls-files \| grep -E '\.(so\|dylib\|dll\|bundle)$'` empty. My independent archive probe: a tar whose list names `lib/libivs_retro.so` → `release-assets-binary-member: lib/libivs_retro.so`, exit 1; the same member renamed `.a` → `release-assets install ok members=1`, exit 0 (boundary positive). Gate: `steps=38/38 faults=238/238 goldens=29` — the invariants are exactly where they were frozen — with the phase's own additions at their predicted counts (`controls` 402→404, `pkg-selftest` 19→20, `release-selftest` 11→12, `gates` 117→118). `lines` reads **361**, not the plan's predicted 358: the review round added three more live `mutant` rows (`ccfc04b` 359, `567b790` 360, `d45f87f` 361), each named in its own commit — a chased delta, not a re-blessed one. |
| 9 | **TOOL-08** is `Complete` in `.planning/REQUIREMENTS.md` (checkbox **and** traceability row) in the same commit as its `important-gates` rows, and every row resolves to a live, shown-able-to-fail `judge` id. | ✓ VERIFIED | `- [x] **TOOL-08**` at `:38` and `\| TOOL-08 \| Phase 15 \| Complete \|` at `:111`. Five `host mutant` rows, all resolving through `runner-process.judge.*`: `host-vacuous`, `host-digest-diff`, `host-schedule`, `host-load-refused`, `host-two-limb`. All five ids are `ok` in the suite I ran (`ok 131`, `ok 132`, `ok 138`, `ok 139`, `ok 140`), and `important-gates` prints `gates=118 lines=361` with `rows-pending=0`. |
| 10 | **One claim**, `CL-0029`, at tier `deterministic-repeatability`, standing `provisional`, resting on this phase's nine-digest equality; it compares the same core with itself and never claims an independent oracle. | ✓ VERIFIED | `data/claims.json` CL-0029: `"tier": "deterministic-repeatability"`, `"standing": "provisional"`, `"artifact": "tests/host/ivs_host_harness.c"` (tracked, non-handoff), `depends_on` = the four profile fields that fix `nominal_frame_ticks`. The statement ends with the tier's own limit and the note states plainly that the digest values are "the core's own producer's output on both sides of the comparison, not an independent oracle", that the shim is never installed so `installed-host-behavior` does **not** apply, and that D10/D11 are unperformed. The note's id count matches measurement: it cites `# suite=runner-process executed=147 skipped=2 failed=0`, and my run and both presets in the gate print exactly `147`. GUARDRAILS V3's set moved together: `data/expected/claims.ids`, one `[claim:CL-0029]` tag in `docs/evidence-tiers.md`, and the CHANGELOG table — `cmake -P tools/check/claims-table.cmake` is **byte-equal** to the committed block (`cmp` silent). Gate: `claims=29 provisional=28 claim-refs=27`. |
| 11 | The registry baseline is re-derived by running the step and quoting its own printed stat. | ✓ VERIFIED | My gate run prints `-- important-gates pass gates=118 lines=361`; `awk` over the file agrees (`118` distinct gate names, `361` non-comment lines), and the toy tree's derived set still closes (`_good/*` controls `control ok`). |
| 12 | The closing gate line is measured under two shells (byte-identical) and once in the Ubuntu container, and compared token by token. | ✓ VERIFIED | `sh tools/gate.sh` exit 0 and `dash tools/gate.sh` exit 0, `tail -1` **byte-identical** (1509 bytes each, `cmp` silent), zero `^-- FAIL` lines. A third run at this HEAD inside Ubuntu 24.04 (amd64, emulated) — `sh tools/gate-linux.sh` — printed `gate-linux: x86_64, gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0, cmake version 3.28.3`, `gate: PASS`, `gate-time total=288`, `gate-linux: exit 0`, zero `-- FAIL`. Token-by-token against the macOS line, **exactly four** differ and all four are the documented host differences: `cc=AppleClang\|GNU`, `core-imports=7\|5`, `archive=members\|sha256`, `refs-verified=3/4\|0/4`. This is the leg the plan's own container log could not cover: that log is the `85470b8` run, and the five post-review commits changed the shim and added three ids — so the Ubuntu substance at HEAD is now measured, not inherited. |
| 13 | The phase boundary records **exactly one** cost verdict, recomputed by arithmetic from `.planning/ci-runs.tsv`. | ✓ VERIFIED | Recomputed in Python from the last five committed rows (`45718553735 363/35/119`, `45720934719 438/39/135`, `45723269901 368/35/122`, `45747709457 366/34/115`, `45751979661 359/34/120`): `cpu_s` sorted `359 363 366 368 438` → median **366**; `build_s` `34 34 35 35 39` → **35**; `gate_s` `115 119 120 122 135` → **120**. The frozen 361/34/118 (STATE.md §1) are stale by **+5/+1/+2**, exactly as recorded. `discharged-within-allowance` needs ≤ 361 + 1.535 = 362.535 and 366 is 3.465 above; `lever-required` needs cumulative spend past 21 and it is 5.005; so `reprice` is what the rule names, and it is the single verdict in `.planning/STATE.md:681`. |
| 14 | `15-VALIDATION.md` is signed off with the measured evidence, and `host/` is inside the integrity rules. | ✓ VERIFIED | `status: validated`, `nyquist_compliant: true`, `wave_0_complete: true`, eight `15-0X-TY` rows each carrying an automated command, every Wave 0 bullet ticked with the file that satisfies it, no `evidence: none` row, and an Approval line quoting the measured gate. `tools/check/integrity.cmake` carries `host` in the ASCII alternation and the SPDX groups; `host/*.c|h|txt` all carry `SPDX-License-Identifier: MIT` and the `integrity` step is green in all three gate runs. |

**Score:** 13/13 truths verified (0 present-but-behavior-unverified, 0 overrides).

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `host/CMakeLists.txt` | `SHARED` target, no install rule, generated export list, the only `IVS_API` definition | ✓ VERIFIED | 53 lines; `install(` count 0; `file(STRINGS …)` reads the ids file; both linker branches; `OUTPUT_NAME ivs_retro`, `PREFIX lib`; no other target in the tree defines `IVS_API`. |
| `host/libretro_shim.h` | the project-authored subset, *libretro-shaped, never compatible* | ✓ VERIFIED | 136 lines; the header states the shape, the two project-shaped signatures and that nothing was vendored (D-48); declares exactly the nine names. |
| `host/ivs_retro_shim.c` | the nine entries, the pixel conversion, the refpcm feed | ✓ VERIFIED | 542 lines, substantive: buffer planning from `ivs_info` formulas, refuse-never-truncate, poll schedule with a bound and a reclaim (CR-01), rollback of a refused load (WR-01), one `ivs_machine_state_digest` call and nothing else. |
| `tests/host/ivs_host_harness.c` | `dlopen`, drive, assert, print | ✓ VERIFIED | 619 lines; resolves the nine names and fails by name on a NULL; asserts version, a denied host, `av_info`, per-frame geometry and pitch, the palette probe, positive frames/samples, the six words, a real teardown, plus the four extra modes (empty-poll, schedule-full/long, refused-load). |
| `data/expected/host-symbols.ids` | nine names; also the linker's export list | ✓ VERIFIED | Nine non-comment lines; read by the build **and** the check; `cmp`-equal to both built objects. |
| `data/expected/host-imports.ids` | the names the shim may import beyond `core-symbols-allow.txt` | ✓ VERIFIED | `malloc calloc realloc free` plus the four named ELF crt stubs, with the reason each is there and an explicit "never add a filesystem/clock/…" instruction. |
| `tools/check/host-symbols.sh` | the export/import check, both directions, palette and `ivs_` limbs | ✓ VERIFIED | 170 lines; the three limbs fired by hand in this session. |
| `data/expected/tests/runner-process.ids` | 8 id lines + header paragraph (as planned) | ✓ VERIFIED | 11 host ids are present and `ok`; the file carries 149 id lines and the trailer reads `executed=147 skipped=2 failed=0` in both presets. |
| `tools/check/controls/files-unlisted-binary-{so,dylib}/` | the two refusal controls | ✓ VERIFIED | `CHECK=files`, `EXPECT=FAIL files-unlisted-binary: docs/save.{so,dylib}`, `MAKE=write …`; inventoried; `control ok` twice in my gate run. |
| `data/expected/pkg-selftests.ids` | `host-so-absent` (19 → 20) | ✓ VERIFIED | Present; `pkg-selftest=20/20`. |
| `data/expected/release-assets-selftests.ids` | `so-member` (11 → 12) | ✓ VERIFIED | Present; `release-selftest=12/12`. |
| `data/expected/important-gates.ids` | TOOL-08's rows and the `host` gate name | ✓ VERIFIED | Five `host mutant` rows; `gates=118 lines=361` re-derived. |
| `data/claims.json`, `data/expected/claims.ids`, `docs/evidence-tiers.md`, `CHANGELOG.md` | `CL-0029` and V3's retarget set | ✓ VERIFIED | All four moved; the rendered table `cmp`s clean. |
| `docs/packaging.md`, `.planning/ops/RELEASE-OPS.md` | the "not installed / not shipped" statements | ✓ VERIFIED | `docs/packaging.md:39` and `:123`; `RELEASE-OPS.md:78` carries the release-decision line. |

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| `data/expected/host-symbols.ids` | `host/CMakeLists.txt` | one contract file read by the build and the check | ✓ WIRED | The generated `host-exports.txt` exists in both presets and carries the nine `_`-prefixed names; `cmp` of the built set against the ids is byte-equal. |
| `tests/CMakeLists.txt` | `tests/host/ivs_host_harness.c` | `$<TARGET_FILE:…>` as the suite's 7th and 8th arguments | ✓ WIRED | The registration line carries both; the suite reports `host-shim`/`host-harness` findings when either is missing (never a skip); no environment path anywhere. |
| `tests/runner-process-tap.sh` | `data/golden/demo-lamp-tone-pass.digests` | the two comparisons `check_one_golden` makes | ✓ WIRED | `check_digest_lines` is the same two comparisons; my run reproduces them. |
| `tests/runner-process-tap.sh` | `data/golden/demo-lamp-tone-idle.digests` | the empty-poll limb reads the idle golden's own lines | ✓ WIRED | 8/8 non-input lines match; the set is read from the golden, not typed. |
| `host/ivs_retro_shim.c` | `runner/frame_export.c`, `tools/pcm/ivs_refpcm.c` | linked/compiled in, never copied | ✓ WIRED | One palette definition and one converter in the tree; `tools/pcm` gained PIC (no install rule, `prefix-files=11`). |
| `tools/check/binary-ext.txt` | `files.cmake`, `release-assets.sh`, `hygiene.sh` | one extension list, three surfaces | ✓ WIRED | My archive probe and the two controls prove two of the three by hand; `binaries-listed=0` and `hygiene pass` cover the third. |
| `tools/check/steps/pkg-install.sh` | `data/expected/pkg-install-files.ids` | the planted `.so` is an extra path | ✓ WIRED | `host-so-absent` requires `pkg-inventory-extra: lib/libivs_retro.so`; my install produced exactly the eleven paths. |
| `.planning/REQUIREMENTS.md` | `data/expected/important-gates.ids` | the traceability table is the derived covered set | ✓ WIRED | `gates=118 lines=361`, `rows-pending=0`, and both refusal directions are exercised by committed controls (`control ok`). |
| `data/claims.json` | `CHANGELOG.md` | `claims-table.cmake` renders, `release-assets` compares | ✓ WIRED | `cmp` silent against a fresh render. |
| `.planning/ci-runs.tsv` | the checkpoint bullet in `.planning/STATE.md` | last five rows, sorted, middle value | ✓ WIRED | Recomputed in Python: 366 / 35 / 120, identical to the record. |

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
|----------|---------------|--------|--------------------|--------|
| `tests/host/ivs_host_harness.c` `digest.<domain>` lines | `sha[32]` from `ivs_retro_digest` | `ivs_machine_state_digest` inside the shim, over the machine the harness drove | Yes — nine 32-byte digests that `diff` empty against the committed golden | ✓ FLOWING |
| `frames`, `video.plane.bytes`, `pixel.probes` | the host struct's counters | the object's own `video_refresh` callbacks during the drive | Yes — 13 frames, 198432 plane bytes, 198432 probes, and `probes == plane bytes` is asserted | ✓ FLOWING |
| `input.words` | `host.poll_got[]` | the object's `input_poll` return values, recorded as handed over | Yes — six lines whose ticks/words are compared against the scenario + `input-bits.ids` | ✓ FLOWING |
| `av_info.*` | `ivs_retro_av_info` | the struct the object filled from `ivs_machine_info` | Yes — asserted against profile-derived constants, printed from the struct | ✓ FLOWING |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|----------|---------|--------|--------|
| the pass drive reproduces the nine golden lines | `build/dev/tests/ivs_host_harness build/dev/host/libivs_retro.dylib build/dev/images` then `diff` on the sorted `digest.` lines | empty diff; `frames 13`, `pixel.probes ok 198432`, `unload.digest.refused 1`; exit 0 | ✓ PASS |
| the empty poll gives a different set while the images stay equal | the same with `--empty-poll` | 7/9 lines differ, `digest.images` equal, 8/8 idle-golden non-input lines match, `digest.input` differs | ✓ PASS |
| the export set is exactly the pin, both presets | `nm -P -g --defined-only` + `cmp` | byte-equal on `build/dev` and `build/release` | ✓ PASS |
| the export check refuses growth, both directions | `host-symbols.sh <so> <scratch ids>` | `host-symbols-missing: ivs_not_defined`; `host-symbols-extra: retro_run`; exit 1 each | ✓ PASS |
| the import limb fires | `host-symbols.sh build/dev/tests/ivs_host_harness` | 43 refusals including `fopen`/`fread`/`fclose`/`dlopen`/`dlsym` | ✓ PASS |
| the palette limb fires | a synthetic `.dylib` exporting `ivs_palette_dump` | `host-symbols-extra` **and** `host-symbols-palette` | ✓ PASS |
| the installed package is eleven files and no shared object | `cmake --install build/release --prefix <tmp>` | 11 files, 0 shared objects, list `diff`-clean against the inventory | ✓ PASS |
| an archive naming a `.so` is refused; the boundary positive passes | `tools/check/release-assets.sh install …` on two synthetic tars | `.so` → `release-assets-binary-member`, exit 1; `.a` → `ok members=1`, exit 0 | ✓ PASS |
| the suite's host ids are green | `ctest --preset dev -R '^runner-process$'` | 1/1 passed, 20.68 s; trailer `executed=147 skipped=2 failed=0`; 11 host ids `ok`, zero `not ok` | ✓ PASS |
| the full gate, twice, two shells | `sh tools/gate.sh`, `dash tools/gate.sh` | exit 0 both, `tail -1` byte-identical (1509 bytes), zero `-- FAIL` | ✓ PASS |
| the full gate in Ubuntu 24.04 at HEAD | `sh tools/gate-linux.sh` | `gate: PASS`, `gate-time total=288`, exit 0, zero `-- FAIL`, exactly 4 differing tokens | ✓ PASS |

### Probe Execution

| Probe | Command | Result | Status |
|-------|---------|--------|--------|
| — | — | The phase declares no `scripts/*/tests/probe-*.sh` and names none in its PLAN/SUMMARY files; its evidence is suite ids, controls and in-step selftests. `find scripts -path '*/tests/probe-*.sh'` → nothing. | N/A |

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|-------------|-------------|-------------|--------|----------|
| TOOL-08 | 15-01, 15-02, 15-03 | The core can be driven in real time by a host that loads it as a shared object outside the installed package, and an automated frontend reproduces the runner's nine digest lines on a project cartridge — with the vacuity closed three ways and the host holding no palette of its own. | ✓ SATISFIED | Truths 1–10 above; `Complete` in both the checkbox and the traceability table; five live `mutant` rows; one `provisional` `deterministic-repeatability` claim whose note matches the measured `executed=147`. |

No orphaned requirement: `grep -E "Phase 15" .planning/REQUIREMENTS.md` maps only TOOL-08, and all three plans declare it.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| — | — | No `TBD`/`FIXME`/`XXX` debt marker in any file the phase changed (the only `XXXXXX` hits are `mktemp` templates); no `TODO`/`HACK`/`PLACEHOLDER`/`coming soon` in `host/` or `tests/host/`; no stub, no hardcoded-empty render, no unwired artifact. | — | — |
| `.planning/phases/15-…/15-03-SUMMARY.md` | 117, 240 | A stale figure: the SUMMARY states the suite trailer as `executed=144 skipped=2 failed=0` and calls it "the same trailer `CL-0029`'s note cites". The note cites **147**, which is what the suite prints. | ℹ️ Info | The **claim** is correct against measurement; the stale sentence is in a plan record and is already recorded as `UF-15-01` in `15-SECURITY.md` with its recommended amendment. No threat, control, claim or token is involved. |
| `.planning/STATE.md` / `15-03-SUMMARY.md` | — | `lines=358` (predicted) vs `lines=361` (measured) | ℹ️ Info | Chased, not re-blessed: three post-review `mutant` rows (`ccfc04b` 359, `567b790` 360, `d45f87f` 361), each named in its commit message and in `15-SECURITY.md` T-15-14. The invariants (`steps`, `faults`, `goldens`) are unmoved. |
| `tools/check/host-symbols.sh` | pass line | `exported=` is printed from the **pin's** count, not the object's | ℹ️ Info | Accepted residual `R-15-D`: the comparison is exact in both directions and fails correctly; I confirmed the artifact independently (`nm` → exactly nine on both presets), so the printed figure is currently true as well as printed. |
| `tools/gate-linux.sh` | 40-55 | The helper runs `docker run --rm -i` with no `--name`, so the project's `ivs-<purpose>` naming rule is not applied to its container (the image is the tool's default `ubuntu:24.04`) | ℹ️ Info | Pre-existing tooling, not introduced by this phase; `--rm` leaves no container behind (verified: `docker ps -a` has no `ubuntu:24.04` entry after my run). Recorded so the next change to the helper closes it. |

### Human Verification Required

None. The phase is headless by construction: every truth above is judged by a committed case, control,
in-step selftest or a recomputed figure, the phase declares no `handoffs.ids` row, and
`15-VALIDATION.md`'s Manual-Only table is empty with the reason stated. Nothing was handed off and no
row carries `evidence: none`.

### Gaps Summary

No gaps. All five ROADMAP success criteria and TOOL-08 are observably true in the tree at `0fc4d33`:

- the object is build-only, linker-pinned to nine names in both presets, and refused by name in both
  directions, with the palette and `ivs_` limbs firing;
- the frontend is not linked against the core, drives the project's own `lamp-tone.ivsc` to tick 806568
  and reproduces the nine golden lines byte for byte — I reproduced that myself, not from the SUMMARY;
- the three vacuity guards are all live, and the judge's own guard was made to fail in both directions;
- the palette is linked rather than copied, the host opens only the images on its command line, and the
  installed package is eleven files with the object's absence checked by name;
- `so dylib dll bundle` are refusal surfaces with two inventoried controls, and the gate ends
  `steps=38/38 faults=238/238 goldens=29 controls=404/404 prefix-files=11` under `sh`, under `dash`
  (byte-identical) and in Ubuntu 24.04 (four documented host tokens differing and no other).

The one claim is at its frozen ceiling, `deterministic-repeatability` / `provisional`, explicitly the
same core with itself, and its note's `executed=147` matches the suite I ran. The cost checkpoint's
`reprice` verdict is arithmetic over the committed rows and reproduces exactly (366/35/120 against the
frozen 361/34/118).

**Observations that do not change the verdict, recorded so they are not re-litigated:**
three newer hosted runs for Phase 14's close (`45752237522`, `45756308953`, `45756570481`) and the
merge run for this phase (`45779277467`, success at `0fc4d33`) exist on GitHub and are **not** in the
committed `.planning/ci-runs.tsv`; `tools/ci-record.sh` is orchestrator-only, so recording them — and
recomputing the medians if they land — is the orchestrator's action, not this phase's gap.

---

_Verified: 2026-11-26T08:23:16Z_
_Verifier: Claude (gsd-verifier)_
