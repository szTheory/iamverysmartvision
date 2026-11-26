# Phase 15: Build-Only Host and the Fake-Frontend Harness - Context

**Gathered:** 2026-11-26
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 15 gives the core a host surface — and proves it with a headless harness,
never a window.

1. **One shared object**, built outside `core/` and outside the install list,
   exporting only the libretro-shaped entry points the project implements and no
   `ivs_` or palette symbol; the export surface is refused by name if it grows.
2. **A fake frontend** that `dlopen`s it, loads one project cartridge (boot stub,
   zero GROM), drives the pass scenario's six events, and prints nine digest
   lines byte-for-byte equal to `data/golden/demo-lamp-tone-pass.digests`.
3. **The vacuity closed three ways**: every line non-`-` with the observed set
   equal; a second id with an empty poll that must give a *different* set; and
   two stand-ins that must be refused — a stub printing the golden with no frame
   (`host-vacuous`) and a mutant with the input poll dropped
   (`host-digest-diff`).
4. **No palette of its own** (it links `runner/frame_export.c`, never copies it),
   no file opened but the images on its command line, and the installed package
   unchanged at eleven files with a selftest that the shared object is absent.
5. **`so` and `dylib` enter `tools/check/binary-ext.txt`** with the controls that
   refuse a tracked shared object, and the gate ends `steps=38/38` with the
   predicted delta.

Out of scope: any real frontend, any vendored libretro header, any release
member, any window, any audio device, any new gate step/fault/golden.

</domain>

<decisions>
## Implementation Decisions

### The host is build-only and the ABI subset is project-authored
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

### The harness is the evidence, never the window
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

### Packaging names the shared object as a non-member
- `tools/check/binary-ext.txt` gains `so`, `dylib`, `dll`, `bundle`; a tracked
  shared library is refused by name, with controls `files-unlisted-binary-so`
  and `-dylib`.
- One `pkg-install` selftest that the shared object is **absent** from the
  installed package (19 → 20 selftests).
- `docs/packaging.md` gains the "not installed" line; `RELEASE-OPS.md` and one
  `release-assets` case record that no host artifact ships.

### Gate cost and evidence discipline
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

### Decided autonomously
- [The host] Q: "Which eight entries does the subset declare?" → Selected: "`api_version`, `init`, `deinit`, `set_environment`, `get_system_av_info`, `load_game`, `run`, `unload_game`, plus a harness-only `ivs_retro_digest`" — the set is one header and one resolver list; undo is local.
- [The harness] Q: "Which cartridge does the harness load?" → Selected: "the project's own `diag-cpu-1.ivsc` with the boot stub and zero GROM" — a project fixture, not a content byte.
- [The harness] Q: "How is 'no palette of its own' proved?" → Selected: "the shim links `runner/frame_export.c`, and a symbol scan refuses an `ivs_` or palette symbol in the shared object's export surface" — one id plus one symbol list.
- [Packaging] Q: "Does the shared object need a fourth `files` control per extension?" → Selected: "two controls (`-so`, `-dylib`) over one extension list, matching the `tools/bench` precedent that did not add one per directory" — a control count, not a contract.
- [Packaging] Q: "Where does 'no host artifact ships' live?" → Selected: "one `release-assets` case plus the `RELEASE-OPS.md` line" — record edits.
- [Cost] Q: "Does the checkpoint add a measurement run?" → Selected: "no — recompute from the committed `.planning/ci-runs.tsv` rows" — the file is the arithmetic input.

### Resolved from the research open questions (2026-11-26)
- **The harness loads `lamp-tone.ivsc`, not `diag-cpu-1.ivsc`.** Criterion 2 names the nine lines of `demo-lamp-tone-pass.digests`, whose cartridge is the build-tree `lamp-tone.ivsc` (sha `ac0ad987…`); `diag-cpu-1`'s golden has `video=none` and no input, so it cannot satisfy the non-`-`/observed-set guard. The criterion wins over the CONTEXT bullet that named `diag-cpu-1`, and the substitution is recorded with its reason.
- **`run` carries a tick target.** The golden's final tick 806568 is not a frame boundary, and `digest.input` serialises `obs.input.floor_tick`, so stopping at a frame end moves seven of the nine lines. A strict libretro signature cannot express it; the subset is project-authored and already states it is *shaped*, never compatible, so `run(target_ticks)` is the honest signature. `load_game` likewise takes three byte spans.
- **`av_info.sample_rate` is 48000**, the rate the shim delivers through `tools/pcm/ivs_refpcm.c`, not the core's native 223721.5625. The shim converts; the harness asserts what the shim declares.
- **The TOOL-08 claim is minted in Phase 15**, at tier `deterministic-repeatability`, standing `provisional` — one claim, and only if the verifier can evidence it. It compares the same core with itself, never an independent oracle.
- **The export surface is enforced by the linker**, measured on both platforms: `-Wl,-exported_symbols_list` on Darwin and `-Wl,--version-script` on Ubuntu, each yielding exactly the listed names under the `nm -P -g --defined-only` recipe `pkg-install` already uses. No new step; one symbol list plus the existing recipe.
- **The `c-build` control R6 suggested is not taken.** The link plus a `host-symbols.ids` list already covers that failure class, and taking it would put the phase at 1.90 cpu-s (+27 % over the frozen 1.5), forcing a reprice the milestone did not price. The priced plan is 1.59.

### Claude's Discretion
None — every question was frozen upstream or decided autonomously under the reversible-technical rule.

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- `runner/frame_export.c` is the one palette; the shim links it.
- `tools/pcm/ivs_refpcm.c` is the one audio converter; the shim reuses it rather than adding a second (a second converter is refused by review).
- `data/golden/demo-lamp-tone-pass.digests` holds the nine lines the harness must reproduce.
- `data/cart/diag-cpu-1.ivsc` (boot stub, zero GROM) is the project cartridge the harness loads.
- `tools/check/binary-ext.txt`, `data/expected/pkg-install-files.ids` (eleven), `data/expected/pkg-selftests.ids` (19) are the packaging baselines.
- `tests/runner-process-tap.sh` is where a `dlopen` harness case fits.

### Established Patterns
- A capability lands as an id inside an existing suite; a new equivalence gets a control whose stand-in fails it.
- A new build target moves its V-row's whole retarget set in one commit.
- A packaging change moves `docs/packaging.md` and `RELEASE-OPS.md` together.

### Integration Points
- `host/` → the build-only shared target → `binary-ext.txt` → the two `files` controls → the `pkg-install` selftest.
- `runner-process` harness case → the nine golden lines → the three guards.
- `release-assets` case + `RELEASE-OPS.md` → "no host artifact ships".

</code_context>

<specifics>
## Specific Ideas

- The upstream freeze is the spec: `.planning/MILESTONE-CONTEXT.md` §2 (Phase 15's exit and tier ceiling), §3 (D-48, D-49), §4 (the fake frontend and its three guards) and scoping R-3/R-4/R-5.
- Phase 15 is **droppable**: if its cost or CI shape proves unpredictable, the pre-authorized fallback is a content-and-conformance v0.3.0, with TOOL-08 recorded `Deferred` and its reversal condition, and the degradation written in the close record.
- The window is never the evidence; the harness is.

</specifics>

<deferred>
## Deferred Ideas

- A real frontend loading the shim: a handoff row, never a criterion.
- Any vendored libretro header: refused unless a real frontend needs one, and its licence read first.
- Shipping the shared object: refused; the release ships the static package and the source archive.
- A window, an audio device, a capture option for owner content: out of scope.

</deferred>
