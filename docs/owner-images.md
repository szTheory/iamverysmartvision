# Owner-local EXEC and GROM images (SCN-02)

This page describes how the owner may load files that belong to the owner, an EXEC image and a GROM image and optionally a cartridge, into the project's headless runner on the owner's own machine (decision D11, route only; the cartridge role is plan 14-03, SCN-03).
These files are not part of this project, this project does not distribute them, and nothing here makes them redistributable: a digest shows which bytes a file holds, not who may copy it.
They are kept in the ignored `refs/` directory, and they are never committed, cached, uploaded or loaded by CI.
The project makes no statement about what any real EXEC, GROM or commercial cartridge does on this core.
Output of an owner-local run is the owner's private observation.
It is not evidence for any claim in this repository, and it is published only if the owner decides to.

## Label

Owner-local.
The inputs belong to the owner, are not part of this project, are not redistributable by it, and are never committed, cached, uploaded or executed by CI.
The label is also machine-read: two uncommitted manifests, `fixtures/FX-owner-exec/manifest.json` and `fixtures/FX-owner-grom/manifest.json`, have kind `system-image`, `committed: false`, no tracked path and `redistributable: no` (schema in `docs/fixture-manifest.md`).
The gate counts them as manifests with an effective `no`, and a tracked path under `refs/` fails the `files` step.

## Inputs and shapes

The runner takes two operator files, `--exec <file>` for `content exec user` and `--grom <file>` for `content grom user`, and an optional third, `--cart <file>` for `content cart user` (`docs/runner.md`).
The core accepts the two system images only in these shapes:

| Role | Shape the core accepts |
|---|---|
| EXEC | exactly 8192 bytes, that is 4096 words, the high byte of each word first, and bits 15 to 10 of every word zero |
| GROM | exactly 2048 bytes |
| cartridge | an `IVSC v1` container (`docs/formats/ivsc-v1.md`), whose own header and table state the base address, the word count and the digest |

A dump in another layout is converted by the owner.
The core refuses a wrong shape at load: a short image (reason 21), a long one (reason 22) and an EXEC word with a bit above bit 9 set (reason 23); for a container, a short one (reason 1) and a long one (reason 2) of `docs/formats/ivsc-v1.md`.
Whether a given dump matches this container without conversion is not known to the project, and this page does not say that it does.

### The cartridge role

The route takes an optional cartridge with `--cart <file>`.
That file is an `IVSC v1` container, and the project's own wrap tool writes it from the owner's flat file at the address the owner states, in the word order the owner states (`docs/packaging.md`); the route forwards the path and parses nothing, so it reads no other format, detects nothing and names no other tool's container.
The cartridge is a build-only tool's output: it is derived material of content the owner holds, it lives outside every tracked path, and it is never committed, never a fixture and never a golden.
The owner digest lock stays at two roles, `exec` and `grom`: the container's identity is in the report as `slot.cart.sha256`, so it needs no lock line, and the lock's grammar is unchanged.
The helper adds one `content cart user` line to its temporary copy of the template only when a cartridge is given, and forwards `--cart` to the runner; with no cartridge the copy is the one this page described before the cartridge role existed.
No size rule sits in the helper: the core refuses a short container (reason 1) and a long one (reason 2) at load with a precise report line.

## Where the files live

`refs/FX-owner-exec.bin` and `refs/FX-owner-grom.bin`, and the container a wrap tool wrote next to them, inside the work tree's `refs/` directory.
That directory is ignored by version control, is outside every gate step and outside the install tree, and is left out of an archive of the tracked files.
Do not keep the files in `build/`, which is cleaned and rebuilt, and do not keep them elsewhere inside the work tree: a name such as `.img` or `.grom` is ignored too, but an unknown extension is not, and a careless staging command would pick it up.
The helper refuses an image or a container whose physical directory is inside the work tree but not under `refs/`, so `refs/` is the one place for all three files.
The ignore patterns are `/refs/`, `*.bin`, `*.grom`, `*.img` and `*.ivsc`, and `tools/check/binary-ext.txt` lists `ivsc` so a tracked container is refused by name in the `files` step.

## Identity: three layers

1. **Shape.** The core checks the size and the word range at load (above). Automatic.
2. **Owner digest lock.** The owner writes `refs/owner-images.sha256` with one line per role, a SHA-256 digest of 64 lower-case hex digits, two spaces, then `exec` or `grom` (the form of `cmake -E sha256sum`); the helper's `--record` writes it from the two images. The lock is a convenience for the owner's own reproducibility: it states which bytes the owner meant, and it states nothing about rights. The project records no digest of any EXEC or GROM image. The lock has exactly these two roles: a cartridge is not locked there, and a lock line naming `cart` is still refused as a bad line. The cartridge's identity is the container's own trailer digest, which the loader publishes as `slot.cart.sha256` and compares at load.
3. **Core check at load.** The template scenario `data/scenarios/owner-exec-title.ivss` carries a marker comment line. The owner works on a temporary copy of the template, outside the work tree, and inserts after that marker two header lines taken from the lock: `expect-sha256 exec <digest>` and `expect-sha256 grom <digest>`. The core then compares its own digest of the loaded bytes with the stated one, so there is no gap between a read by the owner's tool and the read by the runner. A mismatch ends the run as a refusal (`unexpected-refusal`, exit 14).

## The route from the source archive, in three commands

A release carries a source archive, and this is the route a person has after downloading it: three commands, performed in order from the unpacked prefix, on the owner's own machine.
It is a release-time rehearsal and never a gate step: CI does not perform it, and its transcript is in `.planning/ops/RELEASE-OPS.md` section (j).

1. Unpack the archive and build it cold:

```sh
tar -xzf iamverysmartvision-<version>-source.tar.gz
cd iamverysmartvision-<version>
cmake --preset release
cmake --build --preset release
```

2. Wrap the owner's flat cartridge image at the address the owner states, with the archive's own build-only wrap tool (`docs/packaging.md` names it and states its contract).
The base is four hex digits without a prefix, `$4000` to `$FFFF`; `--word-order hi-first` is the default and `lo-first` swaps the two bytes of each word.
The tool prints the digest of the container it wrote:

```sh
build/release/tools/flatwrap/ivs_flatwrap refs/FX-owner-flat.bin 4000 refs/FX-owner-cart.ivsc
```

3. Load the two images and the container through the helper of the next section, with its `--cart` option:

```sh
sh tools/owner-run.sh --record --cart refs/FX-owner-cart.ivsc refs/FX-owner-exec.bin refs/FX-owner-grom.bin
```

The three files live in `refs/` (the section "Where the files live" below), and the container the second command writes is the wrap tool's output: derived material of content the owner holds, in the same class as the flat file it came from, and never a tracked path.
`--record` writes the owner digest lock from the two images and goes on with the run; it refuses to replace an existing lock, so a later run of the same files is the `--cart`-only form of the same command.

The picture and the sound of such a run belong to a frontend to present, not to this route: for a scenario with a `content <role> user` line the runner refuses the frame, video, golden, audio and psg-log output options, so the route writes no file of what the images caused (the section "The template scenario and the gate" below).
What a run shows is the owner's private observation and nothing else, and the project states no result about it.

The rehearsal of this route is `sh tools/release-rehearse.sh --release --source-route`, which performs exactly these three commands against the archive it unpacks and builds, on project-made zero-filled images of the shapes above rather than on any file of the owner's.
Its transcript is `.planning/ops/RELEASE-OPS.md` section (j), measured from `ivs-v0.2.0-source.tar.gz` at base `4000` with the default `hi-first` word order, and it carries the digest of the container that run wrapped, so the record can be read without rebuilding the procedure.
The route is rehearsed from each release's own source archive before that release is published; the v0.3.0 rehearsal ran from `ivs-v0.3.0-source.tar.gz` and matched 124 of 124 golden lines, and its figures are quoted in the `[0.3.0]` changelog section.
The same rehearsal proves its own pass path able to fail: after `--record`, one byte of the EXEC image is flipped and the same run is refused at load (`IVS_E_DIGEST_MISMATCH`, exit 14), so neither half of the rehearsal can pass alone.

## The helper

`sh tools/owner-run.sh [--preset dev|release] [--record] [--cart <cart-image>] <exec-image> <grom-image>` does the three steps in one place.
`--cart` is an option and not a third positional: the two image paths stay the only positional arguments, and nine test ids keep exercising exactly that arity.
It is an operator tool, not a gate step, and CI never calls it.
The runner of the chosen preset (default `release`) must be built first, for example with `cmake --build --preset release`.
It checks the following in this order and stops at the first refusal with one line on stderr and exit 2, except for the skip:

1. The environment: `CI` or `GITHUB_ACTIONS` set to anything but the empty string.
2. The arguments: an unknown option, an option without its value, a preset other than `dev` or `release`, or not exactly two image paths.
3. When neither image path exists it prints `SKIPPED owner-run: no operator images (D11)` on stdout and exits 77, the skip exit code of the repository's test conventions. This comes before every path and file check, so a path that does not exist is never refused as a wrong shape.
4. The location: an image or a container whose physical directory lies inside the work tree but not under `refs/` is refused, because a careless staging command would pick it up there. A directory outside the work tree is fine.
5. The shape: each image must be a regular file of exactly 8192 bytes (EXEC) or 2048 bytes (GROM), and the container must be a regular file (its shape is the loader's question, not the helper's). One missing image of two is refused here, not skipped.
6. The lock `refs/owner-images.sha256` must exist, and every line must be exactly 64 lower-case hex digits, two spaces, then `exec` or `grom`, once each.
7. The runner of the preset and the template scenario must exist.

It then inserts the two `expect-sha256` lines after the marker of the template into a copy made in a fresh `mktemp -d` directory outside the work tree (a `TMPDIR` inside the work tree is refused), and one `content cart user` line after them when `--cart` was given (the helper asserts the count against the arguments, so the conditional cannot silently invert), prints the banner

`OWNER-LOCAL RUN (D11): inputs are the owner's, not redistributable, not run in CI, not evidence, no claim`

starts `ivs-run --exec <exec-image> --grom <grom-image>`, with `--cart <cart-image>` when one was given, on that copy and removes the copy when it exits.
The helper's exit code is the runner's (`docs/runner.md`), and the report is on stdout for the owner to read.
The helper gives the runner no output-file option, so no frame, sound, log or golden file is written.
A lock that does not match the loaded bytes ends the run at load: the core refuses with `IVS_E_DIGEST_MISMATCH` (exit 14) and nothing is executed.

`--record` writes the lock from the two images (`cmake -E sha256sum`) and goes on with the run.
That is the owner's statement of which bytes are meant, taken at that moment, and it says nothing about rights.
It refuses to replace an existing lock: remove the file first to record again.

The gate tests each refusal, the skip, the identity mismatch and the cartridge pass path (`runner-process.owner-run.*` and `runner-process.judge.owner-run-cart-pass`) on a copy of the helper inside a scratch directory outside the work tree, with zero-filled files made by the test and never an owner image;
for the cartridge it also builds its container with the project's own wrap tool from a zero-filled flat file.
The pass path is pinned to what a run of that shape measured, and the same case flips one EXEC byte after `--record` to show the pin can fail: the core then refuses at load with `IVS_E_DIGEST_MISMATCH` (exit 14).
The tests read and write nothing of the real `refs/`.

## The template scenario and the gate

`data/scenarios/owner-exec-title.ivss` asks for `content exec user` and `content grom user`, names no cartridge, bounds the run at 600 frames and has no `assert` line: it states no result, because the project does not describe a real EXEC. The helper's `--cart` adds the cartridge role to its own temporary copy, and the tracked template is unchanged.
The gate never supplies an operator image, so the scenario ends `missing-fixture` (exit 13, detail `missing role=exec user`), which is its expected outcome in `data/expected/runner-scenarios.ids`.
It is registered `skip-ok`, so the suite `runner-process` reports a passing judgement as a skip, and `tools/check/steps/tests-inventory.sh` prints one `-- SKIPPED <preset>/<suite>/<id>: <reason>` line per skipped id and preset and counts it in the `tests-skipped=` token.
A rule of the `golden` step keeps the route out of ordinary tests: a scenario with a `content <role> user` line must be `skip-ok`, and a `skip-ok` row must name such a scenario.
Without an owner run the outcome vocabulary is only this skip.
With an owner run it is `pass` (the run reached its bound), `guest-fail` (the machine halted or faulted early, a fact about the run and not a judgement of the image) or a refusal (identity or shape).

The runner refuses frame, video, golden and sound output for such a scenario (`--frame-ppm`, `--video-raw`, `--update-golden`, `--audio-raw`, `--psg-log-out` and `--audio-wav`), each with exit 14 and one line on stderr, before any file is created, so no picture, golden, sample, write log or WAV derived from the images is written.

## Snapshots

A snapshot is the byte string that `ivs_machine_snapshot_save` fills into a buffer of the caller (`docs/formats/ivsn-v1.md`).
It never holds an image byte: the three image arrays are not serialized, and only the SHA-256 identities and the loaded flags of the slots are written.
It does hold the machine state, which includes guest RAM, graphics RAM and the registers of the display and sound devices, and when a run used owner-supplied images those values are what the images caused to be written.
A snapshot of such a run is therefore derived rights-bearing material, in the same class as the raw audio, the raw video and the frame outputs above, and the same rule applies: it is never committed, cached or uploaded, and a digest of it states identity, not rights.

What the project does about it:

- The core writes no file and chooses no name, slot or location. It fills a buffer that the caller owns; where the bytes go is the host's decision.
- The runner has no snapshot file option in this phase: the options to write and to read a snapshot file are deferred. Its `--roundtrip-at-ticks` hook (`docs/runner.md`) keeps the snapshot in memory for the length of one cut and never writes it, which is why the hook is allowed for a scenario with a `content <role> user` line.
- The extension `ivsn` is in `tools/check/binary-ext.txt`, so a tracked file with that extension is refused by name in the `files` step (the control `files-unlisted-binary-ivsn`), and `*.ivsn` is in `.gitignore`.
- The format is exact-build and not archival: one build restores exactly one pair of format version and schema version and refuses any other, so a snapshot is not a way to keep a run for later.

A snapshot shows repeatability of one build on one profile and nothing else: it says nothing about accuracy, and nothing about any real EXEC or GROM image, which this project has not run (D11 is open).

## Record keeping

The owner keeps the note of a run in private notes, not in the repository.
It states the two digests, the project commit, the scenario id and what was observed.
Nothing derived from the images is kept in the repository.

## What this route does not do

- It does not say which images are valid and it lists no known digest of any image.
- It does not claim that this core executes any real EXEC correctly. Whether a real EXEC reaches a title screen here at all is the open item D11-EXEC-03 (`data/expected/d10-measurements.ids`), and the answer may be no.
- It does not run in CI and it adds no claim and no evidence tier.
- It does not include the project's own program: `ivs-demo-lamp-tone-1` loads through the project boot stub contract 2 and has its cartridge code at `$5000`, a layout that is not an EXEC cartridge header, and it is not claimed to start under a real EXEC.

D11 is open and is not performed by this project: no real EXEC, GROM or commercial content has been run, and no result about any of them is stated or implied.
The procedure pages `docs/stic/d10-procedure.md` and `docs/input/d10-procedure.md` point here from their `owner-exec` sections.
