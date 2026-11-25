---
phase: 10-reachable-runs-and-presentation-policy
plan: "01"
subsystem: release-route
tags: [docs, release, runner, golden, tracer]
status: complete
requires:
  - "the release preset and its image build (Phase 1-9)"
  - "data/golden/demo-lamp-tone-pass.digests and data/scenarios/demo-lamp-tone-pass.ivss"
provides:
  - "docs/running.md: the only route document, from the released source archive to the committed demo golden"
  - "the checked phrase 'nothing downloadable is runnable' once in README.md and once in docs/packaging.md"
affects:
  - "the gate's text step (docs 60 -> 61) and glossary step (doc-files 59 -> 60)"
tech-stack:
  added: []
  patterns: ["measure the route, then write the route", "the committed golden is the oracle, not a second expectation"]
key-files:
  created:
    - docs/running.md
  modified:
    - README.md
    - docs/packaging.md
decisions:
  - "the nine-line tail is quoted with host.calls qualified in the same paragraph (11 at the default slice, 1599 at --slice-ticks 1000) rather than dropped"
  - "the phrase is made identical, not kept identical (amendment A-01); README.md gained it, docs/packaging.md:121 is untouched"
  - "the tier ceiling for the documented route is installed-host-behavior, stated as a ceiling in this file only; no claim was minted"
metrics:
  duration: "7m40s (base commit to the task-2 commit, including four gate runs)"
  completed: 2026-11-25
estimate:
  tokens: 60000
actuals:
  tokens: 2386
  tasks: 2
  commits: 2
  plan_head_before: 8e658b95545f83a1c1f00c17b8915f996bd5ca3b
  plan_head_after: 0f1c0a1
---

# Phase 10 Plan 01: The source-archive route, measured then documented

A person holding only the released source archive builds the emulator with the release preset and takes `demo-lamp-tone-pass` to the committed golden, and one document says exactly how; both public files now carry the same checked phrase about what a release can do.

## What was measured, on this commit

Everything below is this session's run on `8e658b9`, from a source archive built out of that commit and extracted into a clean directory with no `.git`. The work directory was a `mktemp -d` root outside the repository.

| Fact | Measured value | How |
|---|---|---|
| archive name | `ivs-v0.1.0-source.tar.gz` | read from the `--out` directory listing, not re-derived from the version header |
| archive size | 5,760,692 bytes | `stat` on the produced asset |
| archive members | 4228 | `tar -tzf \| wc -l` |
| archive prefix | `ivs-v0.1.0` | first `tar -tzf` member with its trailing slash stripped, not hard-coded |
| configure | 0 s (sub-second), exit 0 | `cmake --preset release` |
| build | 2 s, exit 0 | `cmake --build --preset release` |
| build targets | 170 | `ninja -t targets all` in `build/release` (object-file rules; the generator is Ninja) |
| runner produced | `build/release/runner/ivs-run`, executable | direct test |
| images produced | 7 files: `boot-stub.exec`, `boot-stub-2.exec`, `diag-cpu-1.ivsc`, `diag-ctrl-psg-1.ivsc`, `diag-stic-1.ivsc`, `diag-stic-2.ivsc`, `lamp-tone.ivsc` | `ls build/release/images` |
| report | 131 lines, first `ivsr 3`, last `report.end 130` | `wc -l` on the captured stdout |
| golden body | 124 non-comment lines | `awk '!/^#/ && NF' data/golden/demo-lamp-tone-pass.digests` |
| golden match | 124 of 124, each exactly once, verbatim | `grep -n -F -x -f body report \| wc -l` |
| outcome | `outcome pass`, `outcome.detail -`, `exit.code 0` | the report's own lines |

The nine tail lines, as measured and as quoted in `docs/running.md`:

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

Every one matched the plan's prediction. No measured value differed from the plan, so nothing had to be recorded as an open item.

`host.calls` was measured twice, because it is the only host-dependent key in the block: 11 at the default slice and **1599** at `--slice-ticks 1000`, with `output.video.frames 13` and `report.end 130` unchanged. That measurement is why the document qualifies the key in the same paragraph instead of folding it into an unconditional byte-for-byte statement.

Two measurements the plan predicted and the run confirmed, used to keep the document honest:

- Without `--content-dir`, the run ends `outcome missing-fixture` (`missing role=exec path=boot-stub-2.exec`, `exit.code 13`) - the default is the scenario file's directory, never the working directory, so the `/images` component is load-bearing. `docs/running.md` says so.
- `--frame-ppm` adds no report key: the report of a run with it is `diff`-identical to the plain report (0 differing lines), still 131 lines.

## The two operator landings, measured

| Landing | Measured | Notes |
|---|---|---|
| `build/listen/demo-lamp-tone-pass.wav` | 21674 bytes, mode 0644, gitignored (`.gitignore:30` `/build/`) | written by `sh tools/listen-demo.sh demo-lamp-tone-pass`; it printed `refpcm.sha256 e3b6638887947d64f7fa7d2be5687aff0eca85981b0ea79cfcfd3c74d057f1ad` and the line `reference conversion v1, converter repeatability only: export path check, not a claim` |
| a `.ppm` at an operator-chosen path | 45806 bytes, mode 0600, header `P6` / `159 96` / `255` | `--frame-ppm` has no default path; the option adds no report key |

Both files were deleted after measuring. `git status --porcelain` showed no `.ppm`, no `.wav` and no archive at any point, and none is committed.

## The prose half (amendment A-01)

The phrase is **made** identical, not kept identical. Measured at plan start: `grep -c -F 'nothing downloadable is runnable'` returned **0** for `README.md` and **1** for `docs/packaging.md` (line 121). After the edit:

```
grep -c -F 'nothing downloadable is runnable' README.md          -> 1
grep -c -F 'nothing downloadable is runnable' docs/packaging.md  -> 1
```

`README.md:17` was edited minimally: the paraphrase "none of it can run a program" became the phrase, and the reason clause ("because neither the runner nor `ivs_pack` nor `ivs_asm` is installed") was kept, so the sentence states the fact and its cause rather than a bare assertion. `docs/packaging.md:121` is byte-identical to its parent - `[ "$(git show HEAD^:docs/packaging.md | sed -n '121p')" = "$(sed -n '121p' docs/packaging.md)" ]` - because it is a `- ` bullet whose sentence continues past the phrase, and the checked object is the substring, never `grep -x`.

The pointer paragraphs, one per file, both saying only where the commands are:

- `README.md:18` - "The commands that build the emulator from the source archive and take the demo program to its expected output are in `docs/running.md`."
- `docs/packaging.md:125` - the same sentence plus "This statement is about what a release contains, not about how to build from it."

Neither file gained a `[claim:CL-NNNN]` tag or an `[nc]` waiver, and no sentence needed one: `runnable` is not a claim word (`-w` does not match `runs` inside it), and every other claim word already on those files sits on a line that already carries a tag. `docs/running.md` needed one prose fix of the same kind: "the third runs one scenario" became "the third executes one scenario" after the text step reported `text-claim-untagged: docs/running.md:41`.

## Token counts

Predicted and observed absolute-and-frozen tokens, both gate runs:

`steps=38/38` `controls=386/386` `claims=28` `provisional=27` `gates=107` `lines=330` `faults=235/235` `cases=13` `goldens=29` `claim-refs=26` `prefix-files=11` `d10-items=47` `tests-expected=4958` `tests-executed=4954` `tests-skipped=4` `tests-missing=0`.

Moved as predicted: `docs` 60 -> **61**, `doc-files` 59 -> **60**.

Derived, recorded as an observation, never a predicate: `files=2579` `tracked=2579` `hygiene-files=2579`, against a task-start baseline of `git ls-tree -r --name-only` = 2578 - a delta of exactly +1, the one new document. Task 2's delta is 0. `gate-time total=69` on the first run; `gate-cpu user=111.77 sys=89.62`.

Two gate lines:

```
sh tools/gate.sh    -> gate: PASS steps=38/38 controls=386/386 claims=28 provisional=27 gates=107 lines=330 faults=235/235 cases=13 goldens=29 claim-refs=26 prefix-files=11 doc-files=60 docs=61 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0
dash tools/gate.sh  -> gate: PASS (same tokens; the `dash` run of the same tree)
```

## Tier ceiling

The documented route's ceiling is **`installed-host-behavior`**: the run is a headless replay of a tracked scenario through the public `ivs_core` API by a host that is not installed, so the highest tier it can reach is that of the installed consumer's replay, and this plan makes no claim at all. The ceiling is stated here as a ceiling; nothing was written into `data/claims.json`, and `claims=28` did not move.

## Deviations from Plan

### Budget-driven, recorded rather than hidden

**1. [Rule 3 - Blocking] The dispatch budget replaced three of the plan's read-first instructions with execution**
- **Found during:** Task 1
- **Issue:** The dispatch carried a hard 1800-second limit and instructed that the first task start immediately and that the repository not be explored beyond the plan and the files it names. Three `read_first` items therefore were satisfied by execution instead of by reading: `tools/release.sh:160-230` (the archive name and prefix were read from the produced asset and its first member, which is the same fact from a stronger source - research Common Pitfalls 4), `tools/listen-demo.sh` (run, and its `.wav` path, size, gitignore status and `refpcm.sha256` line recorded from the run), and `tools/check/steps/golden.sh:146-161,353-355` (the comparison was performed directly against the committed golden, 124/124, which is the step's own body rule as `docs/runner.md:243-245` states it).
- **Fix:** none needed; the facts came from measurement rather than from prose, and `docs/runner.md` (read in full for the regions the plan names) supplied the invocation shape.
- **Files modified:** none.

**2. [Rule 3 - Blocking] The container gate was not run**
- **Found during:** Task 1
- **Issue:** The dispatch forbade the container gate under the budget, so only the host `sh` and `dash` gate runs were performed (four runs in total: after each task, plus `dash` at the end).
- **Fix:** none; recorded here so the missing evidence is visible. `tools/gate-linux.sh` remains the Ubuntu substance check and was not exercised by this plan.
- **Files modified:** none.

### Plan-versus-tree

None. The plan text and the tree agreed on every measured value, on every token, and on both prose targets; nothing was re-blessed and no baseline was re-derived.

## Known Stubs

None. `docs/running.md` documents a route that was executed end to end on this commit; it carries no placeholder, no "coming soon" and no path it did not exercise.

## Threat Flags

None beyond the plan's register. The two prose edits add no network surface, no auth path and no file access; the document names only tracked paths and build outputs.

## Self-Check

- `docs/running.md` exists and is committed in `32f6083`.
- `README.md` and `docs/packaging.md` carry the phrase once each and are committed in `0f1c0a1`.
- Both commits are ancestors of `HEAD`.

## Self-Check: PASSED
