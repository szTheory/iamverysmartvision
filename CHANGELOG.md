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

<!-- claims-table: rendered by plan 08-02 -->
