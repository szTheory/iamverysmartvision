# iamverysmartvision

iamverysmartvision is the working name of a deterministic, evidence-bound emulator core in progress for the Mattel Intellivision family.
The name is the owner's working choice and has not been cleared as a trademark.
Every statement about hardware or compatibility is bound to a named profile, an evidence tier and an artifact.
No unqualified claim about which games work is made anywhere in this repository.

## Status

This repository holds the Phase 1 evidence foundation only.
No machine code exists yet.
There is no software release.

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
- `data/` holds the profile, the claim ledger, the source registry and the CP-1610 family table.
- `fixtures/` holds project-authored fixtures and their manifests.
- `tools/` holds the gate and its checks.
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
