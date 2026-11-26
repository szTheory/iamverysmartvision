---
phase: 15-build-only-host-and-the-fake-frontend-harness
plan: 02
subsystem: host-seam
tags: [shaped-entry, av-info, empty-poll, stand-ins, binary-by-name, release-member, install-inventory]

requires:
  - phase: 15-build-only-host-and-the-fake-frontend-harness
    plan: 01
    provides: host/ (the build-only shared object and its nine entries), tests/host/ivs_host_harness.c (the dlopen frontend and its --empty-poll mode), the two host ids, host-symbols.sh
  - phase: 07
    provides: data/golden/demo-lamp-tone-{pass,idle}.digests — the two committed oracles the empty-poll limbs read
  - phase: 04
    provides: tools/check/steps/pkg-install.sh's selftest machinery and the eleven-path install inventory; tools/check/release-assets.sh's member rule
provides:
  - the shaped-entry contract as three judged ids (host.api, host.frames, host.unload)
  - the digest equality closed three ways, with two committed stand-ins (judge.host-vacuous, judge.host-digest-diff)
  - "`so dylib dll bundle` in the one extension list, read by the tree, the archive and the hygiene rule"
  - two controls (files-unlisted-binary-so, -dylib) and two in-step selftests (pkg-install host-so-absent, release-assets so-member)
  - the two record lines that no host artifact is installed or shipped
affects: [15-03 (the TOOL-08 flip and the claim), 16 (the release-decision line and PKG-06's registration of the two controls)]

actuals:
  tokens: 12174      # chars/4 over the realized diff (48,697 bytes) of the three task commits; this SUMMARY is not counted
  tasks: 3
  commits: 3

tech-stack:
  added: []   # nothing installed, nothing vendored (D-48); no second shared object, no second converter
  patterns:
    - "a pinned expectation lives beside a comment naming the oracle's own field and the arithmetic from it, so a drift names its cause"
    - "a stand-in is a committed case judged by the SAME check as the real artifact, differing from a passing output in one limb only (a rejection for another token is judge-wrong-reason)"
    - "one extension list feeds every surface that refuses a file by name, so an artifact cannot be refused in the tree and accepted in an archive"

key-files:
  created:
    - tools/check/controls/files-unlisted-binary-so/{CHECK,EXPECT,MAKE}
    - tools/check/controls/files-unlisted-binary-dylib/{CHECK,EXPECT,MAKE}
  modified:
    - tests/host/ivs_host_harness.c
    - tests/runner-process-tap.sh
    - data/expected/tests/runner-process.ids
    - tools/check/binary-ext.txt
    - data/expected/controls/rights.ids
    - tools/check/steps/pkg-install.sh
    - data/expected/pkg-selftests.ids
    - tools/check/steps/release-assets.sh
    - data/expected/release-assets-selftests.ids
    - docs/packaging.md
    - .planning/ops/RELEASE-OPS.md

key-decisions:
  - "`env.refused` became `env.denied` and the `av.*` lines became `av_info.*` because the plan names those lines; the harness keeps its own `fail <key> <detail>` shape rather than the plan's `assert.fail <text>`, because the suite's `host-run` finding quotes `sed -n 's/^fail //p'` and the rename would have silently emptied it (plan defect, Rule 1)."
  - "The nine `av_info` lines are printed from the struct the object filled and asserted against constants pinned from data/profiles/intv-2609-ntsc-family-p1.json's own fields (159 = 19 x 8 + 7, 96 = 12 x 8, 59736 = 262 x 57 x 4, 0 = stic.frame_phase_ticks, 3579545 = crystal_hz.num) plus the shim's converter rate 48000; the harness is not linked against the core and does not pretend to read ivs_info."
  - "`input.words` is printed one line per returned event (`input.words <at_tick> <c0> <c1>`, six lines), and the suite derives the six expected words at run time from data/scenarios/demo-lamp-tone-pass.ivss and data/expected/input-bits.ids rather than typing them: the plan's own list was correct, and it is now also checked against its two oracles."
  - "The pixel probe's count is a PIXEL count (198432 = 13 x 159 x 96), not a frame count: plan 15-01's probe tests every delivered pixel against the 17 pinned-distinct palette triples. `host.frames` therefore requires `pixel.probes ok <n>` to equal `video.plane.bytes` and be positive, which is a stronger equality than the plan's `pixel.probes ok 13` and keeps 15-01's evidence (plan defect, Rule 1)."
  - "The two stand-ins are built from the committed golden and from the empty-poll run's OBSERVED `digest.input`, never typed; with no value to substitute the digest-diff fake prints the golden unchanged, so check_host ACCEPTS it and the case fails as judge-vacuous — the honest report of an upstream failure rather than a rejection for a wrong reason."
  - "No `important-gates` row is written for the two new controls: PKG-06 is still `Pending` until Phase 16 and important-gates.sh refuses a row naming a Pending requirement. A control needs no row to run — the runner finds it by directory and by its rights.ids line."

requirements-completed: []   # TOOL-08 stays Pending: plan 15-03 is the phase's flip (its `judge` rows and the claim)

coverage:
  - id: D1
    description: "The shaped-entry contract is asserted and judged: the shape version, a host with a NULL input_poll refused rather than ignored, the av_info geometry and the two project-shaped tick fields, per-frame geometry and pitch, the scenario's six words, positive frames and samples, and a real teardown."
    requirement: TOOL-08
    verification:
      - kind: integration
        ref: "IVS_TEST_OUT_DIR=<dir> ctest --preset dev -R '^runner-process$' -> ok 127/128/129 host.api/host.frames/host.unload; trailer executed=144 skipped=2 failed=0"
        status: pass
      - kind: other
        ref: "scratch probe (throwaway, /tmp): mutated pass-mode outputs refused for av_info.sample_rate 223722, env.refused (the old name), frames 12, video.plane.bytes 198848, audio.bytes != 2 x audio.frames, a frame-quantized at_tick, a deleted unload line and a digest line moved after it; the unchanged output accepted"
        status: pass
    human_judgment: false
  - id: D2
    description: "The digest equality is guarded three ways, each shown able to fail: the observed-set/line-set equality (15-01), the empty-poll run's three limbs, and the two committed stand-ins refused for their own tokens."
    requirement: TOOL-08
    verification:
      - kind: integration
        ref: "ok 130 host.empty-poll; ok 136 judge.host-vacuous; ok 137 judge.host-digest-diff"
        status: pass
      - kind: other
        ref: "scratch probe: check_host_empty extracted from the suite and run on mutated empty-poll outputs — the pass golden verbatim refused for empty-poll-identical, a moved digest.images for empty-poll-images, a moved digest.video and a pass-equal digest.input for empty-poll-idle, frames 0 for empty-poll-frames; the unchanged output accepted"
        status: pass
    human_judgment: false
  - id: D3
    description: "A tracked shared object is refused by name in the tree, in an archive and in the installed prefix; the package stays eleven files and the two record lines state that no host artifact is installed or shipped."
    requirement: TOOL-08
    verification:
      - kind: integration
        ref: "sh tools/check/run-controls.sh -> controls=404/404, both new controls `control ok`, zero drift/orphan/wrong-reason findings"
        status: pass
      - kind: integration
        ref: "sh tools/gate.sh -> -- pkg-install pass ... pkg-selftest=20/20 and -- release-assets pass release-selftest=12/12, prefix-files=11, binaries-listed=0"
        status: pass
      - kind: other
        ref: "git ls-files | grep -E '\\.(so|dylib|dll|bundle)$' -> empty"
        status: pass
    human_judgment: false

duration: 22min
completed: 2026-11-26
status: complete
---

# Phase 15 Plan 02: The Shaped-Entry Contract, the Three-Way Vacuity Closure and the Non-Member Set Summary

**The seam can no longer lie: six more `runner-process` ids assert what the object actually returned and prove the digest judge able to fail, and a shared object is now refused by name in the tree, in an archive and in a copy of the installed prefix — with the package still at eleven files and `steps=38/38`, `faults=238/238`, `goldens=29` untouched.**

## Performance

- **Duration:** ~22 min measured from the baseline gate to the metadata commit; the pre-baseline reading of the plan, context, research and GUARDRAILS is not counted
- **Tasks:** 3/3
- **Files:** 17 paths (6 created, 11 modified), 495 insertions and 18 deletions
- **Commits:** 3 task commits (`5a73e96`, `fe2b4c3`, `1260022`) plus this plan's metadata commit

## Start line, recomputed rather than quoted

`sh tools/gate.sh` at `dadefcd` (exit 0) printed, among the full line:

```
steps=38/38 controls=402/402 faults=238/238 goldens=29 tests-expected=5024 tests-executed=5020
tests-skipped=4 tests-missing=0 gates=117 lines=356 prefix-files=11 pkg-selftest=19/19
release-selftest=11/11 tracked=2764 files=2764 hygiene-files=2764 scanned=695 binaries-listed=0
claims=28 provisional=27 mode=tree
```

Every figure in the plan's prediction table matched this start line exactly — the second consecutive plan whose baselines were not stale. Nothing was re-blessed.

## Gate line at each task commit

| Token | Start | After T1 | After T2 | After T3 | Predicted after T3 | Verdict |
|---|---|---|---|---|---|---|
| `steps` | 38/38 | 38/38 | 38/38 | 38/38 | 38/38 | invariant (§2.1) |
| `controls` | 402/402 | 402/402 | 402/402 | **404/404** | 404/404 | as predicted |
| `faults` | 238/238 | 238/238 | 238/238 | 238/238 | 238/238 | invariant |
| `goldens` | 29 | 29 | 29 | 29 | 29 | invariant |
| `tests-expected` | 5024 | 5030 | 5036 | 5036 | 5036 | as predicted |
| `tests-executed` | 5020 | 5026 | 5032 | 5032 | 5032 | as predicted |
| `tests-skipped` | 4 | 4 | 4 | 4 | 4 | no new `skip-ok` row |
| `tests-missing` | 0 | 0 | 0 | 0 | 0 | — |
| `gates` / `lines` | 117/356 | 117/356 | 117/356 | 117/356 | 117/356 | 15-03's change |
| `prefix-files` | 11 | 11 | 11 | 11 | 11 | the object still has no install rule |
| `pkg-selftest` | 19/19 | 19/19 | 19/19 | **20/20** | 20/20 | as predicted |
| `release-selftest` | 11/11 | 11/11 | 11/11 | **12/12** | 12/12 | as predicted |
| `binaries-listed` | 0 | 0 | 0 | 0 | 0 | the list widened; nothing tracked carries an extension on it |
| `claims` / `provisional` | 28/27 | 28/27 | 28/27 | 28/27 | 28/27 | the claim is 15-03's |
| `tracked` / `files` / `hygiene-files` | 2,764 | 2,764 | 2,764 | **2,770** | 2,770 | +6 = the two controls' three files each |
| `scanned` | 695 | 695 | 695 | 695 | not in the table | the `denylist` step excludes `.planning/` and the new files are all under `tools/` |
| `docs` | 64 | 64 | 64 | 64 | not in the table | `docs/packaging.md` was already scanned |

`tracked` reads 2,770 and not 2,771 because this SUMMARY is written after the task commits; the metadata commit adds it (see the closing line).

## The harness's printed values, quoted once

`build/dev/tests/ivs_host_harness build/dev/host/libivs_retro.dylib build/dev/images`:

```
api.version 1
env.denied 1 / env.accepted 1
av_info.base_width 159 / av_info.base_height 96 / av_info.max_width 159 / av_info.max_height 96
av_info.frame_phase_ticks 0 / av_info.nominal_frame_ticks 59736
av_info.fps_num 3579545 / av_info.fps_den 59736 / av_info.sample_rate 48000
load.images 1
frames 13
frame.geometry ok 13
video.plane.bytes 198432
audio.frames 10815 / audio.bytes 21630
pixel.probes ok 198432
input.words 100000 00000001 00000000
input.words 220000 00000101 00000000
input.words 340000 00000000 00010000
input.words 460000 10000000 00010000
input.words 580000 00000000 00000000
input.words 700000 00000101 20000000
… the nine digest lines, byte-equal to data/golden/demo-lamp-tone-pass.digests …
unload.digest.refused 1
```

- **`av_info` is asserted, not merely printed.** Each line comes from the struct the object filled; the expectation is a constant pinned beside a comment naming the profile field it is derived from — `159 = (video.card_columns 20 - 1) x video.card_size_px.w 8 + video.last_card_column_px 7`, `96 = video.card_rows 12 x video.card_size_px.h 8`, `59736 = scanlines_per_frame 262 x cpu_cycles_per_scanline 57 x ticks_per_cpu_cycle 4`, `0 = stic.frame_phase_ticks`, `3579545 = crystal_hz.num`. `sample_rate 48000` is the shim's converter rate (`tools/pcm/ivs_refpcm.c`), not the core's native level rate — the resolved CONTEXT question.
- **`frame.geometry ok 13` equals `frames 13`**: the count is incremented per delivered frame that satisfied the callback contract (declared geometry, a pitch of at least one XRGB pixel per position, an index plane of `w x h` positions), and the harness fails if the two disagree.
- **`video.plane.bytes 198432 = 13 x 159 x 96`**, the index plane the callback receives — deliberately *not* the golden's `output.video.bytes 198848`, which counts 13 slot bytes of 15,296 with the core's 32-byte header included. The 416-byte difference is exactly 13 headers, and the case comment states it so the trap is closed before it is stepped in.
- **`pixel.probes ok 198432`** — one probe per delivered pixel, each equal to exactly one of the 17 pinned-distinct palette triples.
- **The six `input.words` triples are the poll's own return values**, recorded in the host struct as they were handed over, not restated from the script; the harness asserts there are exactly six and that each `at_tick` is the scenario's.
- **`unload.digest.refused 1`**, and `host.unload` also requires a `digest.` line *before* it by line number, so the order proves the digests were taken while the machine was still loaded.

## The empty poll, beside both goldens

`--empty-poll` drives one call to 806568 with no events. `frames 13`, `video.plane.bytes 198432`, `pixel.probes ok 198432` — the same drive, the same rendering.

| line | pass golden | idle golden | empty-poll run |
|---|---|---|---|
| `digest.cpu` | `1ac14963…` | `6ebf8379…` | `6ebf8379…` = idle |
| `digest.mem` | `0b23446d…` | `7924cea7…` | `7924cea7…` = idle |
| `digest.images` | `09679d87…` | `09679d87…` | `09679d87…` = **both** |
| `digest.instance` | `627a4c96…` | `73c16aa3…` | `73c16aa3…` = idle |
| `digest.composite` | `7f71d668…` | `fed08780…` | `fed08780…` = idle |
| `digest.video` | `66c57475…` | `ff984bb1…` | `ff984bb1…` = idle |
| `digest.audio` | `71f2939b…` | `6b11bf70…` | `6b11bf70…` = idle |
| `digest.input` | `832906ce…` | (none: `input=none`) | `27da2e1f…` ≠ pass |
| `digest.psglog` | `d224205d…` | `0a56d85c…` | `0a56d85c…` = idle |

`runner-process.host.empty-poll` asserts all three limbs: at least one line differs from the pass golden (seven do), `digest.images` is equal to it, and **every non-input `digest.` line the idle golden carries — eight today, read from the golden's own lines, never from a list in the case —** equals that golden's, with `digest.input` differing from the pass golden's. The research's "seven non-input lines" was one short (`composite` is a domain the idle golden carries); the plan already corrected it to eight and the measurement confirms eight.

## The two stand-ins, with the exact token each was refused for

Both are shell scripts this suite writes into its temp dir and judges with the SAME `check_host` the real run is judged by — committed cases, not scratch probes. No second `.so`, no fault site, no golden.

| stand-in | what it prints | token it was refused for | the limb it proves |
|---|---|---|---|
| `host-vacuous` | the pass golden's nine `digest.` lines verbatim (read from the committed golden) and `frames 0` | `frames-zero` | without the frame limb the judge would accept a stub that rendered nothing |
| `host-digest-diff` | the pass golden's nine lines with `digest.input` replaced by the empty-poll run's **observed** `27da2e1f…` and `frames 13` | `digest-diff` | the digest a poll-dropped host prints is exactly the empty chain's input digest |

Each differs from a passing output in one limb only, which is why `self_control` names one token: a rejection for any other token is `judge-wrong-reason`, and an acceptance is `judge-vacuous`. Both ids are `ok`.

## The non-member set: four extensions, three refusal surfaces, two controls, two selftests

`tools/check/binary-ext.txt` now carries `so`, `dylib`, `dll` and `bundle` after `ivsn`, and one list feeds every surface that refuses a file by name:

1. **the tree** — `tools/check/files.cmake` refuses a tracked file whose last extension is on the list (`files-unlisted-binary`), proved by two controls: `files-unlisted-binary-so` plants `docs/save.so`, `files-unlisted-binary-dylib` plants `docs/save.dylib`, each `control ok` and each inventoried in `data/expected/controls/rights.ids` in the same commit;
2. **an archive** — `tools/check/release-assets.sh` refuses a member by name (`release-assets-binary-member`) *whatever the list says*, proved by the twelfth selftest `so-member`, which builds a synthetic archive whose own list names `lib/libivs_retro.so` and requires that finding with no extra-member or missing-member finding;
3. **the installed prefix** — `tools/check/steps/pkg-install.sh`'s twentieth selftest `host-so-absent` plants `lib/libivs_retro.so` in a copy of the installed prefix and requires `-- FAIL pkg-inventory-extra: lib/libivs_retro.so`, so a future `install(TARGETS ivs_retro_shim …)` fails there first.

The fourth reader is `tools/hygiene.sh`, whose binary-by-name rule is built from the same file; `dll` and `bundle` are listed with no control because the four extensions are one line of code in `files.cmake` and a control per extension would prove it again for no new evidence (the `tools/bench` precedent, and the CONTEXT decision).

Measured: `controls=404/404` with zero drift, orphan, wrong-reason or exit-zero findings; `pkg-selftest=20/20`; `release-selftest=12/12`; `prefix-files=11`; `binaries-listed=0`; `git ls-files | grep -E '\.(so|dylib|dll|bundle)$'` empty.

**No `important-gates` row is written for the two controls.** PKG-06 stays `Pending` until Phase 16 and `important-gates.sh` refuses a row naming a `Pending` requirement (`important-gates-unknown-id`). A control needs no row to run: the runner discovers it by directory and by its `rights.ids` line. Registering them is PKG-06's work, not this plan's.

## The two record lines

- `docs/packaging.md` gained one sentence on the build-only paragraph (one shared object over the unchanged static core, no install rule, never installed, never shipped, not a release member; the subset is *libretro-shaped*, the project authored it, nothing was vendored, and it offers no **compatibility** toward that project's frontend ABI) and one release-content bullet (the release ships the static package and the source archive, and no host shim). Neither line carries an untagged claim word: the `text` step is green and `docs=64` is unchanged.
- `.planning/ops/RELEASE-OPS.md` gained appended section **(i)**, recording the release-decision line — `the release ships the static package and the source archive; no shared object, no host shim, no content` — and naming the three mechanisms that hold it. Appended, never a rewrite (§2.10).

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Plan defect] The plan's `pixel.probes ok 13` cannot pass on a correct implementation**
- **Found during:** Task 1, first run of `runner-process.host.frames`
- **Issue:** the plan's action says "Keep the pixel probe from plan 15-01 and print `pixel.probes ok <n>`", and 15-01's probe counts **delivered pixels** (`pixel.probes ok 198432` = 13 × 159 × 96). The same plan's case requirement and its `<automated>` both demand `pixel.probes ok 13`, which is a frame count. The two cannot both hold; the `<automated>` line `grep -x -F -e 'pixel.probes ok 13' /tmp/h15out3/host-pass.out` fails on the correct build.
- **Fix:** the case requires `pixel.probes ok <n>` to be a positive count **equal to `video.plane.bytes`**, which is the equality the frame-count form would lose (every delivered pixel was probed) and keeps 15-01's evidence. Measured `pixel.probes ok 198432` = `video.plane.bytes 198432`.
- **Files modified:** `tests/runner-process-tap.sh`
- **Verification:** the block passes with that one line replaced by the measured equivalent; `not ok 128` with `# host-frames: pixel.probes 'ok 198432', expected 'pixel.probes ok 13'` is the failure the literal line produced before the fix.
- **Commit:** `5a73e96`

**2. [Rule 1 - Plan defect] `assert.fail` would have emptied the suite's host failure message**
- **Found during:** Task 1, writing the assertions
- **Issue:** the plan asks a failed internal assertion to print `assert.fail <text>`. Plan 15-01's harness prints `fail <key> <detail>` and the suite's `host-run` / `host-empty-run` findings quote it with `sed -n 's/^fail //p'`, which does not match `assert.fail`. Renaming would have made every harness failure report an empty reason.
- **Fix:** the harness keeps `fail <key> <detail>`, which is the same contract (one line the suite can quote) under the name the suite already parses.
- **Files modified:** `tests/host/ivs_host_harness.c` (unchanged in this respect), `tests/runner-process-tap.sh`
- **Commit:** `5a73e96`

**3. [Rule 1 - Plan defect] The plan names five profile fields the profile does not carry under those names**
- **Found during:** Task 1, pinning the `av_info` constants
- **Issue:** the plan says the pinned constants come from `data/profiles/intv-2609-ntsc-family-p1.json`'s `video_width`, `video_height`, `nominal_frame_ticks`, `frame_phase_ticks` and `crystal_hz`. That file has no `video_width`, `video_height` or `nominal_frame_ticks` field: the published plane is derived (`(video.card_columns − 1) × video.card_size_px.w + video.last_card_column_px`, `video.card_rows × video.card_size_px.h`) and the frame spacing is derived (`scanlines_per_frame × cpu_cycles_per_scanline × ticks_per_cpu_cycle`). The tick fields are `stic.frame_phase_ticks` and the `ivs_info` pair the shim reports.
- **Fix:** the constants are pinned beside a comment that names each real field and states the arithmetic, so a drift names its cause; the harness asserts the object's values equal them.
- **Files modified:** `tests/host/ivs_host_harness.c`
- **Commit:** `5a73e96`

**4. [Rule 1 - Plan defect] "the eight `av_info.*` lines" is nine**
- **Found during:** Task 1, writing `runner-process.host.api`
- **Issue:** the plan's bullet lists `base_width`, `base_height`, `max_width`, `max_height`, `frame_phase_ticks`, `nominal_frame_ticks`, `sample_rate`, `fps_num` and `fps_den` and then calls them eight.
- **Fix:** all nine are printed and required. The case comment names them without a count so the prose cannot go stale again.
- **Files modified:** `tests/runner-process-tap.sh`
- **Commit:** `5a73e96`

**5. [Rule 1 - Plan defect] `grep -c -F -e 'host' core/CMakeLists.txt` cannot be 0**
- **Found during:** Task 3, running the plan's `<automated>` verbatim
- **Issue:** the plan's closing assertion is `test "$(grep -c -F -e 'host' core/CMakeLists.txt)" -eq 0`, but `core/CMakeLists.txt:121` carries the word in a pre-existing comment — `# PIC (D-34, plan 04-22): a host may link the archive into a shared plugin.` — which predates this phase and is not a path. The assertion is unsatisfiable on any correct tree, and a task's gate must be satisfiable at its own commit (PLAN-LESSONS 1).
- **Fix:** the assertion became the one the plan's own `<fails_when>` states — no `host/` **path** and no `libivs_retro` — checked in `core/CMakeLists.txt` and in the two generated installed export files (`build/release/core/CMakeFiles/Export/*/ivsTargets*.cmake`). Both read 0, and `pkg-install`'s `pkg-path-leak` and `prefix-files=11` hold the same statement from the other side.
- **Files modified:** none (the assertion is in the verify block)
- **Commit:** `1260022`

**6. [Rule 2 - Correctness] The digest-diff stand-in needed a guard against substituting nothing**
- **Found during:** Task 2, writing the stand-ins
- **Issue:** the stand-in takes the empty run's `digest.input` and substitutes it. With an empty value the `sed` would write `digest.input ` — a difference, so `check_host` would reject it for `digest-diff` and the case would pass for the wrong reason (a broken empty-poll run would look like a working stand-in).
- **Fix:** with no value the fake prints the committed golden unchanged, so `check_host` **accepts** it and the case fails as `judge-vacuous` — the honest report of the upstream failure. `host_stand_in` also refuses a stand-in that printed nothing (`stand-in-empty`).
- **Files modified:** `tests/runner-process-tap.sh`
- **Commit:** `fe2b4c3`

**Total deviations:** 6 auto-fixed (5 Rule 1 — one unsatisfiable `<automated>` line, one silently-emptied failure message, one set of mis-named profile fields, one miscount, one unsatisfiable assertion; 1 Rule 2 — a stand-in that could have passed for the wrong reason). **Impact:** none on the plan's contract. Six ids, two controls, two in-step selftests, `controls=404/404`, `pkg-selftest=20/20`, `release-selftest=12/12`, `prefix-files=11`, `binaries-listed=0`, `steps=38/38`, `faults=238/238`, `goldens=29` — every token is exactly as planned.

## Auth gates

None.

## Known stubs

None. `grep -rn 'TODO\|FIXME\|coming soon\|not available'` over the changed files is empty, no case is wired to empty data, and every value the harness prints is measured from the object. The two stand-ins are deliberately defective outputs inside a case, not stubs in shipped behaviour.

## Threat surface scan

No new surface beyond the plan's `<threat_model>`. All seven T-15-* mitigations are implemented: T-15-07's three limbs are the empty-poll case's three assertions and the two committed stand-ins, each shown able to fail; T-15-08's final tick is `run(806568)` in both modes; T-15-09's poll words are the object's own return values with the scenario's absolute ticks; T-15-10's single palette and converter are unchanged and `host-symbols.sh` still refuses a palette word; T-15-11's absence from the package is the `host-so-absent` selftest over the eleven-path inventory; T-15-12's refusal is the one extension list on three surfaces; T-15-13's claim-word tripwire is green. No package was installed and nothing was vendored (T-15-SC).

## Verification results

| Check | Result |
|---|---|
| `sh tools/gate.sh` at `1260022` | exit 0, `gate: PASS`, zero `-- FAIL`, every predicted token present |
| `dash tools/gate.sh` at `1260022` | exit 0, byte-identical tokens under the second shell |
| `IVS_TEST_OUT_DIR=<dir> ctest --preset dev -R '^runner-process$'` | exit 0; 146 ids, six new ones `ok`, trailer `executed=144 skipped=2 failed=0`, zero `not ok` |
| `sh tools/check/run-controls.sh` | `controls=404/404`, both new controls `control ok`, no drift/orphan/wrong-reason finding |
| `sh tools/hygiene.sh --tree` | `-- hygiene pass mode=tree hygiene-files=2770` |
| `git ls-files \| grep -E '\.(so\|dylib\|dll\|bundle)$'` | empty |
| `grep -c -E '(^\|[^-A-Za-z0-9_])host/\|libivs_retro' core/CMakeLists.txt build/release/core/CMakeFiles/Export/*/ivsTargets*.cmake` | 0 |
| `grep -c -F -e 'libivs_retro' data/expected/pkg-install-files.ids` | 0 |

**The container leg is not taken in this plan.** `sh tools/gate-linux.sh` runs once at the phase close (plan 15-03), one container at a time (PLAN-LESSONS 18). The Linux half of this plan's evidence is the `.so` refusal, which is extension-based and platform-independent, and the `--version-script` branch of `host/CMakeLists.txt` that the close's container run exercises; neither is claimed green here.

## Calibration

`actuals.tokens` is 12,174 against the plan's `estimate.tokens` of 62,000 — 0.20 of the estimate. The miss is not work skipped: the plan priced the *thinking* (three mechanisms, seventeen paths, five design questions) on the scale of a plan that writes new machinery, while this one mostly wires rows into machinery that already exists (two 3-file controls, two selftest arms, four list entries, two doc lines). Per changed line the plan's own unit was ~120 tokens/line; the realized diff is ~24. Recorded as measured, not rounded toward the estimate.

## Next

Ready for `15-03`: the TOOL-08 flip with its two live `judge` rows, the thirteen-file toy extension, the one `deterministic-repeatability` claim with GUARDRAILS V3's whole retarget set, the re-derived `gates=`/`lines=`, the two-shell and container gate evidence, the recomputed cost checkpoint and the validation sign-off.

## Self-Check: PASSED

- All six created paths exist on disk (`tools/check/controls/files-unlisted-binary-{so,dylib}/{CHECK,EXPECT,MAKE}`).
- The three task commits (`5a73e96`, `fe2b4c3`, `1260022`) are ancestors of `HEAD`.
- Every `<automated>` block was run. Two of their literal lines cannot pass on a correct build — `grep -x -F -e 'pixel.probes ok 13'` (task 1) and `test "$(grep -c -F -e 'host' core/CMakeLists.txt)" -eq 0` (task 3) — and both are recorded above as plan defects with their measured replacements; each replacement was run green. The other four blocks run verbatim.
- `sh tools/gate.sh` and `dash tools/gate.sh` both exit 0 with `gate: PASS`, identical tokens and zero `-- FAIL` lines at `1260022`.

## Gate line at this metadata commit

`sh tools/gate.sh` at this commit, exit 0, zero `-- FAIL` lines, byte-for-byte the tokens the plan predicted:

```
gate: PASS steps=38/38 controls=404/404 faults=238/238 goldens=29 tests-expected=5036
tests-executed=5032 tests-skipped=4 tests-missing=0 gates=117 lines=356 prefix-files=11
pkg-selftest=20/20 release-selftest=12/12 binaries-listed=0 tracked=2771 files=2771
hygiene-files=2771 scanned=695 claims=28 provisional=27 docs=64 mode=tree
```

`tracked`/`files`/`hygiene-files` read 2,771 and not 2,770 because this SUMMARY is itself a tracked file — the exact
figure the plan's table named ("this plan's own `SUMMARY.md` makes the metadata commit read 2,771"). `scanned` stays at
695 because the `denylist` step excludes `.planning/`, and `docs` stays at 64 because `docs/packaging.md` was already
scanned.
