# Phase 10 rehearsal transcript: the source route

This is the record of one run of `sh tools/release-rehearse.sh --no-gate --source-route`
on commit `0cce18b` of this branch, on Darwin 25.6.0 arm64 with Apple clang 21.0.0
(clang-2100.1.1.101). It is **not a gate step**, **not a gate result**, **not evidence
for any claim** and it carries **no tier**: the release rehearsal is release-time only,
`tools/gate.sh` never runs it, no control asserts it, `data/claims.json` does not cite it
and `claims=28` is unchanged by it. What it records is that the archive a release will
carry was unpacked into a clean directory, built cold, taken through the demo scenario and
compared with the committed golden — by the release tool, at release time, before a tag is
pushed.

## The command and its output

```
sh tools/release-rehearse.sh --no-gate --source-route
```

The rehearsal's own lines, in order, from that run:

```
rehearse: dry run 1 (sh tools/release.sh --no-gate)
  notes: CHANGELOG.md section [0.1.0] present
  tag v0.1.0, publish=0, release-checks=0, gate=skipped
  install archive built with cmake-tar (members sorted, mtime of the commit)
rehearse: tar mode cmake-tar
rehearse: dry run 2 (sh tools/release.sh --no-gate)
rehearse: changelog-section: not required
rehearse: source-archive: equal (5786235 bytes)
rehearse: source-route: ivs-v0.1.0-source.tar.gz, golden match (report 131 lines, 124 of 124 compared), prefix ivs-v0.1.0
install-archive: equal-except-ranlib-stamp (Darwin: only the time field of the first member header of lib/libivs_core.a differs, bytes 25 to 36; byte equality is decided on Linux)
rehearse: tag-glob: v[0-9]*.[0-9]*.[0-9]* matches v0.1.0 and v1.2.3, not v1.0, v0.1, vfoo, v, 1.0.0
```

The readiness sections follow those lines and the script's last line is `rehearse: ok`.

The source-route step consumes the archive step 3 already proved byte-equal to a second
run's, so nothing is rebuilt or re-hashed twice. The archive name (`ivs-v0.1.0-source.tar.gz`)
and the extraction prefix (`ivs-v0.1.0`) in that line were **read at run time** — the name
from the workspace listing, the prefix from the archive's first member — and neither is
written anywhere in the script, so a version bump needs no edit there.

Every temporary path of the run was under one `mktemp` directory, written here as
`<work>`, removed on exit by the script's existing trap: `<work>/a` and `<work>/b` hold the
two dry runs, `<work>/route` holds the extraction, the release-preset build, the report and
the two compared files. Nothing was written into the repository.

## The golden match

| Quantity | Value |
|---|---|
| the archive | `ivs-v0.1.0-source.tar.gz`, 5786235 bytes, prefix `ivs-v0.1.0` |
| the report | `demo-lamp-tone-pass`, 131 lines |
| the committed golden's body | `data/golden/demo-lamp-tone-pass.digests` with its `#` lines removed, 124 lines |
| compared | 124 of 124 lines, `cmp` equal |

The oracle is the committed `data/golden/demo-lamp-tone-pass.digests` and nothing else: the
mode applies the gate's own golden key list, copied verbatim from
`tools/check/steps/golden.sh` into the rehearsal's `route_live` and `route_filter`, to the
report of the clean build, and compares the result with that file. There is no second
expectation and no second comparison of the rehearsal's own making. The copy fails closed —
a stale key list selects a different set of lines and the comparison stops the rehearsal
with exit 1 — and it is written as two functions that can be extracted from the committed
script and re-run on their own: applied to a file that is not a report they emit nothing,
so the equality cannot hold vacuously.

## The cost

`/usr/bin/time -p` over the whole invocation, with the load average of the host at the time:

| Quantity | Value |
|---|---|
| real | 11.30 s |
| user | 19.01 s |
| sys | 8.38 s |
| load average | 7.15 10.57 11.08 |

The two cold-build stages inside that figure, measured on the same archive in a separate
extraction because the mode prints one result line: `cmake --preset release` 0.24 s real,
`cmake --build --preset release` 2.26 s real (244 build edges, `user` 12.55 s, `sys` 5.59 s).
The rest of the time is the two packaging dry runs and the readiness script.

This cost is paid once per release and never in CI: the mode runs no gate step, adds no
control, adds no registry row and moves no count, so `steps`, `controls`, `gates`, `lines`,
`goldens` and `claims` are exactly as they were before it existed.

## How to repeat it

```
sh tools/release-rehearse.sh --no-gate --source-route
```

`--no-gate` reuses the existing `build/release` and its images; drop it to run the gate
first. The route the mode automates, written out for a person, is the three commands in
`docs/running.md`.
