# Owner-local EXEC and GROM images (SCN-02)

This page describes how the owner may load two files that belong to the owner, an EXEC image and a GROM image, into the project's headless runner on the owner's own machine (decision D11, route only).
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

The runner takes two operator files, `--exec <file>` for `content exec user` and `--grom <file>` for `content grom user` (`docs/runner.md`).
The core accepts them only in these shapes:

| Role | Shape the core accepts |
|---|---|
| EXEC | exactly 8192 bytes, that is 4096 words, the high byte of each word first, and bits 15 to 10 of every word zero |
| GROM | exactly 2048 bytes |

A dump in another layout is converted by the owner.
The core refuses a wrong shape at load: a short image (reason 21), a long one (reason 22) and an EXEC word with a bit above bit 9 set (reason 23).
Whether a given dump matches this container without conversion is not known to the project, and this page does not say that it does.
The route has no cartridge role: the scenario names the two system images only.

## Where the files live

`refs/FX-owner-exec.bin` and `refs/FX-owner-grom.bin`, inside the work tree's `refs/` directory.
That directory is ignored by version control, is outside every gate step and outside the install tree, and is left out of an archive of the tracked files.
Do not keep the files in `build/`, which is cleaned and rebuilt, and do not keep them elsewhere inside the work tree: a name such as `.img` or `.grom` is ignored too, but an unknown extension is not, and a careless staging command would pick it up.
The ignore patterns are `/refs/`, `*.bin`, `*.grom` and `*.img`.

## Identity: three layers

1. **Shape.** The core checks the size and the word range at load (above). Automatic.
2. **Owner digest lock.** The owner writes `refs/owner-images.sha256` with one line per role, a SHA-256 digest of 64 lower-case hex digits, two spaces, then `exec` or `grom` (the form of `cmake -E sha256sum`); the helper's `--record` writes it from the two images. The lock is a convenience for the owner's own reproducibility: it states which bytes the owner meant, and it states nothing about rights. The project records no digest of any EXEC or GROM image.
3. **Core check at load.** The template scenario `data/scenarios/owner-exec-title.ivss` carries a marker comment line. The owner works on a temporary copy of the template, outside the work tree, and inserts after that marker two header lines taken from the lock: `expect-sha256 exec <digest>` and `expect-sha256 grom <digest>`. The core then compares its own digest of the loaded bytes with the stated one, so there is no gap between a read by the owner's tool and the read by the runner. A mismatch ends the run as a refusal (`unexpected-refusal`, exit 14).

## The helper

`sh tools/owner-run.sh [--preset dev|release] [--record] <exec-image> <grom-image>` does the three steps in one place.
It is an operator tool, not a gate step, and CI never calls it.
The runner of the chosen preset (default `release`) must be built first, for example with `cmake --build --preset release`.
It checks the following in this order and stops at the first refusal with one line on stderr and exit 2, except for the skip:

1. The environment: `CI` or `GITHUB_ACTIONS` set to anything but the empty string.
2. The arguments: an unknown option, a preset other than `dev` or `release`, or not exactly two image paths.
3. When neither image path exists it prints `SKIPPED owner-run: no operator images (D11)` on stdout and exits 77, the skip exit code of the repository's test conventions. This comes before every path and file check, so a path that does not exist is never refused as a wrong shape.
4. The location: an image whose physical directory lies inside the work tree but not under `refs/` is refused, because a careless staging command would pick it up there. A directory outside the work tree is fine.
5. The shape: each image must be a regular file of exactly 8192 bytes (EXEC) or 2048 bytes (GROM). One missing image of two is refused here, not skipped.
6. The lock `refs/owner-images.sha256` must exist, and every line must be exactly 64 lower-case hex digits, two spaces, then `exec` or `grom`, once each.
7. The runner of the preset and the template scenario must exist.

It then inserts the two `expect-sha256` lines after the marker of the template into a copy made in a fresh `mktemp -d` directory outside the work tree (a `TMPDIR` inside the work tree is refused), prints the banner

`OWNER-LOCAL RUN (D11): inputs are the owner's, not redistributable, not run in CI, not evidence, no claim`

starts `ivs-run --exec <exec-image> --grom <grom-image>` on that copy and removes the copy when it exits.
The helper's exit code is the runner's (`docs/runner.md`), and the report is on stdout for the owner to read.
The helper gives the runner no output-file option, so no frame, sound, log or golden file is written.
A lock that does not match the loaded bytes ends the run at load: the core refuses with `IVS_E_DIGEST_MISMATCH` (exit 14) and nothing is executed.

`--record` writes the lock from the two images (`cmake -E sha256sum`) and goes on with the run.
That is the owner's statement of which bytes are meant, taken at that moment, and it says nothing about rights.
It refuses to replace an existing lock: remove the file first to record again.

The gate tests each refusal, the skip and the identity mismatch (`runner-process.owner-run.*`) on a copy of the helper inside a scratch directory outside the work tree, with zero-filled files made by the test and never an owner image.
The tests read and write nothing of the real `refs/`.

## The template scenario and the gate

`data/scenarios/owner-exec-title.ivss` asks for `content exec user` and `content grom user`, names no cartridge, bounds the run at 600 frames and has no `assert` line: it states no result, because the project does not describe a real EXEC.
The gate never supplies an operator image, so the scenario ends `missing-fixture` (exit 13, detail `missing role=exec user`), which is its expected outcome in `data/expected/runner-scenarios.ids`.
It is registered `skip-ok`, so the suite `runner-process` reports a passing judgement as a skip, and `tools/check/steps/tests-inventory.sh` prints one `-- SKIPPED <preset>/<suite>/<id>: <reason>` line per skipped id and preset and counts it in the `tests-skipped=` token.
A rule of the `golden` step keeps the route out of ordinary tests: a scenario with a `content <role> user` line must be `skip-ok`, and a `skip-ok` row must name such a scenario.
Without an owner run the outcome vocabulary is only this skip.
With an owner run it is `pass` (the run reached its bound), `guest-fail` (the machine halted or faulted early, a fact about the run and not a judgement of the image) or a refusal (identity or shape).

The runner refuses frame, video, golden and sound output for such a scenario (`--frame-ppm`, `--video-raw`, `--update-golden`, `--audio-raw`, `--psg-log-out` and `--audio-wav`), each with exit 14 and one line on stderr, before any file is created, so no picture, golden, sample, write log or WAV derived from the images is written.

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
