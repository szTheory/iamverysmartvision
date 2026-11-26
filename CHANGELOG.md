# Changelog

All notable changes are recorded here in the style of Keep a Changelog: one section per release, headed with the version in square brackets, and the release body is the matching section verbatim.
The text step scans this file for claim words like the README and `docs/`.

## [Unreleased]

Nothing is unreleased yet.

## [0.3.0]

Pre-release of a deterministic, headless machine core and its installed package for the profile `intv-2609-ntsc-family-p1`; nothing downloadable executes a program, and a person builds the emulator from the source archive.
This section is the pre-release of the exact commit tagged v0.3.0; the release's own date is on the releases page.

### What changed since 0.2.0

- **A rights ledger, and a claim ceiling the gate enforces.** `data/rights/` holds one question-shaped row per third-party-content item the milestone touched, each with a disposition of `cleared`, `refused` or `owner-local-only`; a `cleared` row must cite a resolvable `SRC-` id, and a row missing its disposition, its source or its "what was taken" cell is refused by name. A refused item's spellings are refused inside `core/src`, `runner/`, `tools/` and the seven operator pages, with four pre-existing hits pinned by a `(token, path)` pair and a stale pin treated as a failure. Independently, an `established` claim must now be a `source-expectation` claim resting on a `tier1-primary-doc` dependency, checked mechanically rather than quoted.
- **The ten previously-unregistered must-fail mechanisms now have registry rows** over selftests that already existed, with **zero new faults**: six `control` rows over existing control directories, four `mutant` rows over a new runner-selftest inventory, and the baseline re-derived in the same commit.
- **A cartridge route for the operator's own content.** `tools/flatwrap` wraps a flat cartridge image of the operator's own, at an address the operator states, into this project's container. It is build-only, it parses no other tool's format, it auto-detects nothing, and it refuses eight malformed inputs by typed token with a boundary positive beside each. Its output is derived material: never committed, never a golden, never a fixture, and never created by CI. The owner-local route gained a cartridge role, with every derived-output refusal extended to it.
- **The first real-content blocker, fixed rather than recorded.** A run with the owner's own images showed that accepting an interrupt did not clear the CPU's interrupt enable, so the EXEC re-entered its handler on every instruction boundary and died at `$FFFF`. The default is now resolved from that observation, a project-authored test pins **both** settings, and the profile parameter digest, every snapshot literal and every dependent expectation moved in the same commit. The field stays tagged `unknown-default-chosen`: an observation is not a hardware probe and mints no claim.
- **A build-only host, and the harness that proves it.** One shared object lives outside the core and outside the installed package, exporting exactly nine libretro-shaped entry points and nothing else, with the export surface enforced by the linker on both platforms. A headless fake frontend loads it, drives a project cartridge, and reproduces the runner's nine digest lines byte for byte; the equality is guarded three ways, including a stub that prints the golden with no frame and a mutant with the input poll dropped. No libretro header is vendored and the host holds no palette of its own.
- **The owner route, documented and rehearsed.** One route from the source archive to a project-cartridge golden, in three commands, with the transcript recorded. The questions only the owner can answer are handoff rows, not criteria.
- **The concurrency lever, once, cost only.** Two independent gate steps now overlap: wall time falls, the summary token line is byte-identical, and the evidence does not move.

### What a release contains

- The source archive of the tested commit.
- The install archive: eleven files, namely the static library `libivs_core.a`, the five public headers, the CMake package files and the licence. The archive name states the operating system and architecture it was built on. **No shared object, no host shim and no content is in it or beside it.**
- `SHA256SUMS`, the SHA-256 of both archives.

### What this release does not claim

It contains no EXEC, GROM or commercial cartridge, and it asserts nothing about what any of them would do on this core. That question is the owner's own run, recorded as a handoff row with no evidence and cited by no claim. No claim reaches the `measured-hardware` tier, which needs a physical measurement this project has not made. [nc]

### Rights

No EXEC, GROM, MTE-201 image, commercial ROM, manual or overlay is included, redistributed or required by any check. The operator's own files stay on the operator's machine.
iamverysmartvision is not affiliated with or endorsed by Mattel, Intellivision Entertainment, Atari or any other rights holder.
"Intellivision" and related names are trademarks of their respective owners.

<!-- claims-table-begin -->
| Claim | Tier | Standing | Artifact |
|---|---|---|---|
| CL-0001 | source-expectation | provisional | docs/cp1610/reference-notes.md |
| CL-0002 | source-expectation | established | data/profiles/intv-2609-ntsc-family-p1.json |
| CL-0003 | source-expectation | provisional | data/profiles/intv-2609-ntsc-family-p1.json |
| CL-0004 | source-expectation | provisional | docs/cp1610/reference-notes.md |
| CL-0005 | source-expectation | provisional | data/cp1610/opcode-families.json |
| CL-0006 | source-expectation | provisional | data/memmap/intv-2609-ntsc-family-p1.literal.txt |
| CL-0007 | source-expectation | provisional | docs/formats/ivsc-v1.md |
| CL-0008 | original-diagnostic | provisional | fixtures/FX-diag-cpu-1/diag-cpu-1.listing.txt |
| CL-0009 | source-expectation | provisional | data/input/intv-2609-ntsc-family-p1.literal.txt |
| CL-0010 | source-expectation | provisional | data/input/intv-2609-ntsc-family-p1.literal.txt |
| CL-0011 | deterministic-repeatability | provisional | docs/input/controller-notes.md |
| CL-0012 | source-expectation | provisional | data/psg/rules.json |
| CL-0013 | deterministic-repeatability | provisional | docs/psg/reference-notes.md |
| CL-0014 | original-diagnostic | provisional | fixtures/FX-diag-ctrl-psg-1/diag-ctrl-psg-1.listing.txt |
| CL-0015 | installed-host-behavior | provisional | tests/installed-consumer/main.c |
| CL-0016 | source-expectation | provisional | data/stic/intv-2609-ntsc-family-p1.regmap.literal.txt |
| CL-0017 | deterministic-repeatability | provisional | docs/stic/timing-notes.md |
| CL-0018 | source-expectation | provisional | data/video/geometry.literal.txt |
| CL-0019 | source-expectation | provisional | docs/video/frame-derivations.md |
| CL-0020 | original-diagnostic | provisional | fixtures/FX-diag-stic-1/diag-stic-1.listing.txt |
| CL-0021 | deterministic-repeatability | provisional | docs/stic/reference-notes.md |
| CL-0022 | installed-host-behavior | provisional | tests/installed-consumer/main.c |
| CL-0023 | playable-guest-progress | provisional | data/golden/demo-lamp-tone-pass.expect |
| CL-0024 | installed-host-behavior | provisional | tests/installed-consumer/main.c |
| CL-0025 | source-expectation | provisional | tests/video/video_mob.c |
| CL-0026 | source-expectation | provisional | tests/video/video_collision.c |
| CL-0027 | original-diagnostic | provisional | fixtures/FX-diag-stic-2/diag-stic-2.listing.txt |
| CL-0028 | deterministic-repeatability | provisional | tests/state/state_cut.c |
| CL-0029 | deterministic-repeatability | provisional | tests/host/ivs_host_harness.c |

Claims: 29 in total, 28 provisional, 1 established; none at tier measured-hardware.
<!-- claims-table-end -->

Claims: 29 in total, 28 provisional, 1 established; none at tier measured-hardware.

## [0.2.0]

Pre-release of a deterministic, headless machine core and its installed package for the profile `intv-2609-ntsc-family-p1`; nothing downloadable executes a program, and a person builds the emulator from the source archive.
This section is the pre-release of the exact commit tagged v0.2.0; the release's own date is on the releases page.

### What changed since 0.1.0

- **Registered evidence.** Seventeen requirements whose named faults already executed in every gate (FND-01..07, CPU-01..05, MEM-01..05) now have a registered cannot-pass-vacuously mechanism: `data/expected/important-gates.ids` grew to 337 non-comment rows naming 110 gates, with no new test id, control or fault. The must-be-covered set is derived from the requirement records and a two-directional exclusion ledger accompanies it, so deleting a registry line is a failure rather than a silent loss of coverage.
- **Gate cost, settled before the gate grew.** Lever (3) and trims (a), (c) and (d) landed in cost-only commits, a hosted row was recorded after them, and trim (b) was declined by measurement. The preflight partition takes local time-to-first-signal from 79 s to 31 s with byte-identical normalised output. The gate is 38 steps, before and after.
- **The documented person route.** `docs/running.md` names three commands and the exact expected stdout tail, measured in a clean extraction of the source archive; `tools/release-rehearse.sh --source-route` unpacks that archive, builds cold, executes `demo-lamp-tone-pass` and compares 124 of 124 golden lines.
- **Presentation policy.** `docs/presentation.md` freezes six clauses that keep colour, scaling and aspect out of the core and out of every claim; the refusal that enforces the load-bearing clause is the existing `core-symbols` rule.
- **Input record and replay.** IVSL v1 (`docs/formats/ivsl-v1.md`) is a new format: an ASCII, input-only log of the events a run applied, with no field able to hold a content digest, an image name, a length, a frame byte, an audio byte or a path. Two new runner options write and apply it. A replay of a recorded log reproduces the recorded report's nine digest lines and its applied-event chain, with the liveness guards and the two named faults that make a vacuous match impossible.
- **Handoff ledger.** `data/expected/handoffs.ids` records every human-only item with `evidence: none`, and a claim can no longer cite a handoff artifact.
- **Dropped, not delivered.** The `.bin`/`.cfg` importer and the bank-mode model (FMT-01, FMT-04) were dropped at a written entry gate before any third-party file was read: the only description of the `.cfg` container the project could find is a GPL implementation, which this project's research rules exclude. Both requirements are recorded `Deferred` with their reversal conditions.

### What a release contains

- The source archive of the tested commit.
- The install archive: eleven files, namely the static library `libivs_core.a`, the five public headers, the CMake package files and the licence. The archive name states the operating system and architecture it was built on.
- `SHA256SUMS`, the SHA-256 of both archives.

### Format and API versions

- The API version is 0.2.0, the version macros of `ivs.h`; the installed package reports the same version. The minor bump is the new format and the two new runner options; no API call changed or was removed.
- The formats are the cartridge container IVSC v1, the snapshot container IVSN v1 with schema version 1, the video slot format INDEX8_FRAMES_V1 with its 32-byte slot header, and the new input log IVSL v1.
- The state digest composite is version 3.

### How to check a download

- Compare the SHA-256 of each archive with the lines of `SHA256SUMS`.
- Unpack the install archive and configure a host project with `find_package(ivs 0.2 CONFIG REQUIRED)` and `CMAKE_PREFIX_PATH` set to the unpacked directory.
- `docs/running.md` is the route from the source archive to a completed original program, and `docs/packaging.md` lists the installed files.

### What a release does not contain

- No EXEC, GROM, ROM, manual or overlay of any kind, and none was used by any check.
- No runner, assembler or pack tool. A reader builds those from the source archive.
- No cartridge of any third-party program, and no way to load one: the core accepts the project's own IVSC v1 container, and the `.bin`/`.cfg` importer is deferred.
- No binary for any system other than the one named in the install archive.

### Not qualified

- No measured-hardware claim: nothing was measured on a console (D10 not performed).
- No run with a real EXEC or GROM image: no owner-supplied system image was used (D11 not performed).
- No person has looked at a frame or listened to the sound.
- The controller, STIC and PSG rules rest on community sources (tier 4) or on inference.

### Name and rights

iamverysmartvision is a working name and has not been cleared as a trademark.

iamverysmartvision is not affiliated with or endorsed by Mattel, Intellivision Entertainment, Atari or any other rights holder.
"Intellivision" and related names are trademarks of their respective owners.

The claim table below is rendered from `data/claims.json`; a claim stands as provisional unless the table says established.

The claim table is in the `[0.3.0]` section above, rendered from `data/claims.json`; one marker pair in this file, as the release-assets check requires.

## [0.1.0]

Pre-release of a deterministic, headless machine core and its installed package for the profile `intv-2609-ntsc-family-p1`; nothing downloadable executes a program.
This section is the pre-release of the exact commit tagged v0.1.0; the release's own date is on the releases page.

### What a release contains

- The source archive of the tested commit.
- The install archive: eleven files, namely the static library `libivs_core.a`, the five public headers, the CMake package files and the licence. The archive name states the operating system and architecture it was built on.
- `SHA256SUMS`, the SHA-256 of both archives.

### Format and API versions

- The API version is 0.1.0, the version macros of `ivs.h`; the installed package reports the same version.
- The formats are the snapshot container IVSN v1 with schema version 1, the cartridge container IVSC v1 and the video slot format INDEX8_FRAMES_V1 with its 32-byte slot header.
- The state digest composite is version 3.

### How to check a download

- Compare the SHA-256 of each archive with the lines of `SHA256SUMS`.
- Unpack the install archive and configure a host project with `find_package(ivs 0.1 CONFIG REQUIRED)` and `CMAKE_PREFIX_PATH` set to the unpacked directory.
- `docs/packaging.md` lists the installed files and describes the installed consumer that the release job builds against the very archive it ships.

### What a release does not contain

- No EXEC, GROM, ROM, manual or overlay of any kind, and none was used by any check.
- No runner, assembler or pack tool. A reader builds those from the source archive.
- No binary for any system other than the one named in the install archive.

### Not qualified

- No measured-hardware claim: nothing was measured on a console (D10 not performed).
- No run with a real EXEC or GROM image: no owner-supplied system image was used (D11 not performed).
- No person has looked at a frame or listened to the sound.
- The controller, STIC and PSG rules rest on community sources (tier 4) or on inference.

### Name and rights

iamverysmartvision is a working name and has not been cleared as a trademark.

iamverysmartvision is not affiliated with or endorsed by Mattel, Intellivision Entertainment, Atari or any other rights holder.
"Intellivision" and related names are trademarks of their respective owners.

The claim table below is rendered from `data/claims.json`; a claim stands as provisional unless the table says established.

The claim table of this release was rendered from `data/claims.json` when the release was cut.
The current table is in the `[0.2.0]` section, which is the release body of that release; the ledger itself is `data/claims.json`.
