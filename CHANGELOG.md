# Changelog

All notable changes are recorded here in the style of Keep a Changelog: one section per release, headed with the version in square brackets, and the release body is the matching section verbatim.
The text step scans this file for claim words like the README and `docs/`.

## [Unreleased]

Pre-release of a deterministic, headless machine core and its installed package for the profile `intv-2609-ntsc-family-p1`; nothing downloadable executes a program.

### What a release contains

- The source archive of the tested commit.
- The install archive: ten files, namely the static library `libivs_core.a`, the four public headers, the CMake package files and the licence. The archive name states the operating system and architecture it was built on.
- `SHA256SUMS`, the SHA-256 of both archives.

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

Claims: 24 in total, 23 provisional, 1 established; none at tier measured-hardware.
<!-- claims-table-end -->
