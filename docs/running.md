# Running the emulator from a source release

This is the route for a person who holds the source archive of a release: build
the emulator, then take one original program to its expected output.
It is the only route document.

It is not the install route. Nothing downloadable is runnable, and
`docs/packaging.md` carries the packaging statement that says why: neither the
runner nor the two image tools is installed, so a release ships source and a
reader builds from it. Nothing below installs anything into a prefix, and no
package manager is involved.

Every command in this file was executed on the commit this file was written from,
in a clean extraction outside the repository, and the values quoted under
"Expected output" are that run's. The extraction had no `.git` directory and no
build output of its own.

`docs/runner.md` documents the report keys, the scenario grammar and the
operator output options. This file is the route; that one is the reference.

## Build the source archive

Take the archive name and the top-level directory from the release you
downloaded rather than from this file. The members sit under one directory whose
name is the archive's stem, and every path below is relative to it.

```sh
tar -xzf ivs-v0.3.0-source.tar.gz
cd ivs-v0.3.0
```

## The three commands

```sh
cmake --preset release
cmake --build --preset release
./build/release/runner/ivs-run --content-dir build/release/images data/scenarios/demo-lamp-tone-pass.ivss
```

The first command configures the release preset, the second builds it, and the
third executes one scenario against the images the build produced. The preset's
requirements and its cache entries are in `CMakePresets.json`; nothing outside
the extracted directory is needed.

Two build outputs matter here, and both are build output rather than tracked
files. The executable is `build/release/runner/ivs-run`. The images are seven
files under `build/release/images` - `boot-stub.exec`, `boot-stub-2.exec`,
`diag-cpu-1.ivsc`, `diag-ctrl-psg-1.ivsc`, `diag-stic-1.ivsc`,
`diag-stic-2.ivsc` and `lamp-tone.ivsc` - assembled from tracked source by the
project tools during the build. The `--content-dir` value is the image
directory, with its `images` component. Content paths resolve against it, and
its default is the directory of the scenario file rather than the working
directory, so dropping the `images` component leaves the run with
`outcome missing-fixture` instead of a report.

The third command writes its report to stdout, one `key value` line per key, and
exits 0 for the outcome `pass`.

## Expected output

The report is 131 lines. The first is `ivsr 3` and the last is
`report.end 130`, the count of lines before it, so a truncated report cannot be
read as complete. The run ends with these nine lines:

```
audio.sample_ticks 16
audio.chain.check pass
output.video.frames 13
video.geometry 159x96
video.row_lines 2
video.frame.last_tick 776568
video.chain.check pass
host.calls 11
report.end 130
```

`host.calls` is the one key here that depends on the host: it counts host
service calls, so it changes with the slice size, while every other line above
is a function of the scenario and the images. It is 11 at the default slice and
1599 at `--slice-ticks 1000`. A reader who changes the slice, or who adds
`--roundtrip-at-ticks`, should expect that line to move and nothing else to.

## The golden the report is compared against

The expected outcome is the committed golden
`data/golden/demo-lamp-tone-pass.digests`, which is the same oracle the gate's
`golden` step uses. Its body is the run's report filtered to a fixed key list,
in report order, so the comparison is a filtered equality: every non-comment
line of the golden appears once, verbatim, in the report, and the golden holds
124 such lines against the report's 131. The keys left out are the ones that
are not a function of the scenario and the images - `ivsr`, `api.version`,
`host.calls`, `report.end` and the operator keys - which is why the tail above
is quoted with `host.calls` qualified rather than folded into the comparison.

`docs/runner.md` lists the golden key list and the `--update-golden` invocation
that writes a golden. A changed golden is a reviewed diff; the gate never writes
one.

## Where an operator render lands

Two output options write a file, and neither file is evidence. Both belong
outside the repository.

`--frame-ppm <file>` writes the last delivered frame as a binary PPM. The path
is the operator's choice and there is no default. The file is created
exclusively with mode 0600, and its header is `P6`, then `159 96`, then `255`,
for 45806 bytes in total. The option adds no report key, so the report of a run
with it is the report without it. An exported frame is never evidence: no
golden, claim, digest or test reads one, and a viewer's judgement is not a test.

`sh tools/listen-demo.sh demo-lamp-tone-pass` renders the same program's audio
to `build/listen/demo-lamp-tone-pass.wav`, 21674 bytes, and prints the
`refpcm.sha256` line of that file. The path is under `build/`, which is
gitignored, and a `.wav` is never committed. The converter behind it is labelled
reference conversion v1, converter repeatability only: it shows that the export
path works, not what the sound chip did.

Neither a `.ppm` nor a `.wav` is committed by this repository, and neither is a
record of the machine's behaviour.
