# iamverysmartvision

iamverysmartvision is the working name of a deterministic, evidence-bound emulator core in progress for the Mattel Intellivision family.
The name is the owner's working choice and has not been cleared as a trademark.
Every statement about hardware or compatibility is bound to a named profile, an evidence tier and an artifact.
No unqualified claim about which games work is made anywhere in this repository.

## Status

The repository holds a deterministic machine core, a headless runner and a first original program. All of it is in progress, and the claims about its behaviour are provisional.

- The core (`core/`) is a C17 library with a narrow public API. It implements a CP-1610 CPU, a memory map, two hand controllers as machine input, the PSG sound chip and a first pass of the STIC video chip for the profile `intv-2609-ntsc-family-p1`. Machine progress, input, raw video, raw audio and a PSG write log are separate observations.
- The runner (`runner/`, the executable `ivs-run`) replays scripted scenarios against the core. A separate consumer in `tests/installed-consumer/` is built against the installed package only and replays the same scenarios through the public API.
- The first original program, "Lamp Tone", is tracked as source (`fixtures/FX-demo-lamp-tone-1/lamp-tone.asm`) with its generated listing. It is entered through the project boot stub (contract 2), shows each controller port as a row of eight lamps and gives each port one sound voice. The statement about it is playable guest progress, for this original program only [claim:CL-0023], with the installed-package replay in [claim:CL-0024]. Both claims are provisional: the expected values are hand derived by the author of the program and the core, the controller, sound and video rules are community or inferred sources, nothing was measured on a console, and no person has looked at the picture or listened to the sound.
- Programs are built from tracked source by project tools (`ivs_asm`, `ivs_pack`) that exist for development and testing only.

No release has been published. A release ships the program only as tracked source. Neither the runner nor `ivs_pack` nor `ivs_asm` is installed, so nothing downloadable is runnable yet.
No EXEC, GROM or commercial content is included or used by any check. The route for running images that the owner supplies locally is described in `docs/owner-images.md`; it is not part of CI and has not been performed.

## Evidence

The first profile is `intv-2609-ntsc-family-p1`.
The NTSC video part in that profile is the AY-3-8900-1, driven from a 3.579545 MHz crystal [claim:CL-0002].
A nominal NTSC frame is 262 scanlines of 57 CPU cycles, which is 14,934 CPU cycles per frame [claim:CL-0003].
The tiers and the claim ledger are defined in `docs/evidence-tiers.md`.
The time vocabulary is defined in `docs/time-base.md`.

## Checking the tree

Run one command from the repository root.

```sh
sh tools/gate.sh
```

It prints one line per check and a final summary line that starts with `gate: PASS` when every check and every negative control is green.
CI executes the same command.
It needs a POSIX shell, git, CMake 3.25 or newer, Ninja and a C17 compiler (cc).

Before pushing, `sh tools/gate-linux.sh` executes the same gate on the current working tree inside a throwaway `ubuntu:24.04` container, with the hosted runner's toolchain (amd64, gcc 13, dash, CMake 3.28).
It catches failures that only the hosted runner would show, such as compiler-specific evaluation order or ELF-only symbols, without a CI round trip.
It needs Docker and network access for the container's packages, and it takes a few minutes on an arm64 host, where amd64 is emulated.
It prints every failure, the gate time, the summary line and the exit status, and keeps the full log in `build/gate-linux.log`.
It is a local helper only: it is not a gate step and CI does not run it.

## Layout

- `docs/` holds the policy and reference documents.
- `data/` holds the profile, the claim ledger, the source registry, the CP-1610 family table, the scenarios and the hand-derived expectations.
- `core/` holds the machine core and its public headers, `runner/` the headless runner and `tests/` the test suites and the installed consumer.
- `fixtures/` holds project-authored fixtures, programs and their manifests.
- `tools/` holds the gate and its checks, and the development tools that build fixture images.
- `refs/` is a local, ignored cache for reference documents and is never tracked.

## Rights and provenance

No ROMs, system images, manuals or scans are included in this repository.
The rights policy is in `docs/RIGHTS.md`.
Source handling is described in `docs/provenance.md`.
Fixture manifests are described in `docs/fixture-manifest.md`.

## Licence

The project is released under the MIT licence, see `LICENSE`.

## Trademarks

iamverysmartvision is not affiliated with or endorsed by Mattel, Intellivision Entertainment, Atari or any other rights holder.
"Intellivision" and related names are trademarks of their respective owners.
They are used here only to say what the emulator targets.
The name "Intellivision" is never used in a package or product name of this project.
