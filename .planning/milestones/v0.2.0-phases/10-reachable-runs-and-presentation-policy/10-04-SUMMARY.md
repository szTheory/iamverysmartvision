---
phase: 10-reachable-runs-and-presentation-policy
plan: "04"
subsystem: release-route
tags: [release, rehearsal, golden, route, docs]
status: complete
requires:
  - "plan 10-01: docs/running.md, the route this mode automates"
  - "tools/release.sh and its source archive (git archive of HEAD, prefix ivs-<tag>/)"
  - "tools/check/steps/golden.sh:146-161 and data/golden/demo-lamp-tone-pass.digests"
provides:
  - "tools/release-rehearse.sh --source-route: the archive a release carries, unpacked, built cold, run on the demo scenario, compared with the committed golden"
  - "route_live/route_filter: the gate's golden filter, extractable and re-runnable from the committed script"
  - "10-REHEARSAL-TRANSCRIPT.md: the recorded run, stated as release-time and not gate evidence"
affects:
  - "docs/packaging.md: the rehearsal sentence names the mode"
tech-stack:
  added: []
  patterns:
    - "consume what an earlier step proved instead of proving it twice"
    - "read the archive name and prefix from the run, never write a version literal"
    - "a copied filter is safe when it fails closed and can be re-run on a non-report"
key-files:
  created:
    - .planning/phases/10-reachable-runs-and-presentation-policy/10-REHEARSAL-TRANSCRIPT.md
  modified:
    - tools/release-rehearse.sh
    - docs/packaging.md
decisions:
  - "the mode is release-time only and adds no gate step, control, registry row or claim; the gate already pays for a cold build"
  - "the filter is a verbatim copy of the gate's own program, not a second oracle, because a stale copy selects a different set and the cmp stops the run"
  - "the empty-filtered-report case fails the mode, so a filter that matched nothing cannot compare two empty files and appear to pass"
  - "the header block and usage()'s sed range moved together (3,29 -> 3,38) and --help was proved to end on the block's last line"
  - "the transcript writes the workspace root as <work>, because a resolved scratch path is refused by tools/hygiene.sh"
metrics:
  duration: "18m (plan start to the task-2 commit, including two rehearsal runs and two gate runs)"
  completed: 2026-11-25
estimate:
  tokens: 70000
actuals:
  tokens: 4316
  tasks: 2
  commits: 2
  plan_head_before: 751c501145979fcde813c1a55e9aa5bdc4b33e5c
  plan_head_after: a488d83f2d521995e5758d8fdb168a53ce1c01fa
---

# Phase 10 Plan 04: The source route, rehearsed from the archive a release carries

`tools/release-rehearse.sh --source-route` unpacks the source archive a release will carry, builds it cold with the release preset, takes `demo-lamp-tone-pass` to its report and compares that report with the committed golden using a copy of the gate's own filter, and the run is recorded as a release-time transcript that says what it is not.

## The mode's output, verbatim

The recorded run at `0cce18b` (`sh tools/release-rehearse.sh --no-gate --source-route`), the rehearsal's own lines in order:

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

Exit 0, and `rehearse: ok` is the script's last line (83 lines of output including the readiness sections A, F and G).

## The archive, as read at run time

| Quantity | Value | Read from |
|---|---|---|
| archive name | `ivs-v0.1.0-source.tar.gz` | the workspace listing (`$src_a`), not re-derived from the tag |
| archive size | 5786235 bytes | the step-3 line, `wc -c` |
| extraction prefix | `ivs-v0.1.0` | the archive's first `tar -tzf` member, trailing slash stripped |
| report | 131 lines | the clean build's `demo-lamp-tone-pass` report |
| golden body | 124 lines | `data/golden/demo-lamp-tone-pass.digests` with its `#` lines removed |
| compared | 124 of 124 lines, `cmp` equal | `route_filter(report)` vs the golden body |

Neither the name nor the prefix is written anywhere in the script — a version bump needs no edit here, and the plan's verify reads the prefix out of the tarball at run time to prove it.

## The two non-vacuity results

Both taken from the committed script alone, by extracting the two functions with `sed -n '/^route_live() {$/,/^}$/p;/^route_filter() {$/,/^}$/p'`:

- `route_filter` applied to a file that is not a report (`/dev/null`) emits **0 lines**, so it cannot equal the golden's 124-line body: the comparison is not vacuous (lesson 20).
- `route_filter` applied to `data/golden/demo-lamp-tone-pass.expect` emits **0 bytes** — a second non-report input, same result.
- The golden's body is **124 lines**, so the equality that passed is an equality over real content.

## The cost

`/usr/bin/time -p` over the whole invocation at the recorded run:

| Quantity | Value |
|---|---|
| real | 11.30 s |
| user | 19.01 s |
| sys | 8.38 s |
| load average | 7.15 10.57 11.08 |
| configure (`cmake --preset release`) | 0.24 s real |
| cold build (`cmake --build --preset release`) | 2.26 s real, 244 edges, user 12.55 s sys 5.59 s |

The configure and build figures were measured on the same archive in a separate extraction, because the mode prints one result line; the real/user/sys triple is the mode's own. This cost is paid once per release and never in CI.

## The gate did not move — measured, not claimed

The gate's own shapes, unchanged by this plan: `step-time <name> <s>` per step (slowest here: `c-faults 110`, `sources 54`, `controls 18`, `c-tests 16`, `profile 11`), `gate-env ncpu=18 cc=AppleClang-21.0.0.21000101`, `gate-cpu user=126.12 sys=93.78`, `gate-time total=236`. No `step-time` line exists for the rehearsal: it is not a step.

Both shells, on the committed tree:

```
gate: PASS steps=38/38 controls=386/386 ... claims=28 provisional=27 ... goldens=29 ... doc-files=62 ... docs=63 claim-refs=26 ... gates=107 lines=330 ... faults=235/235 ... files=2586 ... hygiene-files=2586 ...
gate: PASS steps=38/38 controls=386/386 ... claims=28 provisional=27 ... goldens=29 ... doc-files=62 ... docs=63 claim-refs=26 ... gates=107 lines=330 ... faults=235/235 ... files=2586 ... hygiene-files=2586 ...
```

Every absolute-and-frozen token is exactly as plan 10-03 left it: `steps=38/38`, `controls=386/386`, `claims=28`, `provisional=27`, `gates=107`, `lines=330`, `faults=235/235`, `goldens=29`, `claim-refs=26`. `docs=63` and `doc-files=62` did **not** move, because the transcript lives under `.planning/` and is a planning record, not a document; `docs/packaging.md` was already counted. The observed triple is `files=2586 tracked=2586 hygiene-files=2586`, a delta of +1 per task: 0 for task 1 (a modified file only), 1 for task 2 (the transcript). `sh tools/hygiene.sh --tree` exits 0.

## `--help`

The header block and `usage()`'s `sed` range moved together, from `3,29` to `3,38`. `sh tools/release-rehearse.sh --help` prints the new mode and ends on the block's last line:

```
Exit: 0 ok, 1 a check failed, 2 usage error or a missing tool.
```

## Deviations from Plan

### Approved budget deviation

**1. [Plan step, budget] One gate run per shell at plan end, not per task**

- **Found during:** Task 1, step 6 and Task 2, step 5
- **Issue:** the plan asks for `sh tools/gate.sh` and `dash tools/gate.sh` at the end of each task; on this host a gate run is 214 to 236 s wall (load average 7 to 11), and four per-task runs plus the two after the closing records commit would not fit the 1800 s dispatch limit
- **Fix:** ran one `sh` and one `dash` gate at the end of the plan and the same pair after the closing records commit, exactly as the dispatch's budget warning approves. The first pair was green (`steps=38/38`, every absolute-and-frozen token unchanged); the second pair first failed on the PKG-05 refusal recorded below and was green after that amend, with the same tokens
- **Files modified:** none
- **Commit:** n/a (execution order only)

### Auto-fixed Issues

**1. [Rule 1 - Bug] PKG-05 stays Pending: a complete-but-uncovered important requirement is refused**

- **Found during:** the gate run after the records commit
- **Issue:** `gsd_run query requirements.mark-complete PKG-05` flipped PKG-05 to Complete in `.planning/REQUIREMENTS.md`, and the gate's `important-gates` step then failed the run at the preflight: `important-gates-complete-uncovered: PKG-05` and `important-gates-uncovered-gate: PKG-05` (`steps=37/38`). No row in `data/expected/important-gates.ids` names PKG-05, and this plan's prohibitions forbid adding a registry row (`gates=107`, `lines=330`)
- **Fix:** reverted `.planning/REQUIREMENTS.md` to its state before the mark-complete call, so PKG-05 stays Pending here; plan 10-06 lists PKG-05 among its requirements and is the plan that can pair it with a registry row without breaking this plan's frozen tokens. The refusal is the registry doing its job, not a defect in it
- **Files modified:** `.planning/REQUIREMENTS.md` (reverted, no net change)
- **Commit:** the records commit of this plan, amended

**2. [Rule 1 - Bug] The first records commit left the gate red; it was amended**

- **Found during:** `sh tools/gate.sh` immediately after the records commit
- **Issue:** the amended-and-committed records produced `gate: FAIL steps=37/38` from the item above
- **Fix:** the revert plus this deviation record, committed as an amend of the records commit, after which `sh` and `dash` both end `gate: PASS steps=38/38`
- **Files modified:** `.planning/REQUIREMENTS.md`, this SUMMARY
- **Commit:** the records commit of this plan, amended

## Auth gates

None.

## Known stubs

None. The mode has no placeholder: every sub-step fails closed through `fail()` (missing archive, failed extraction, failed configure, failed build, failed runner, empty report, empty filtered report, differing comparison).

## Threat Flags

None beyond the plan's `<threat_model>`. The mode writes nothing into the repository, extracts only into the existing `mktemp` workspace removed by the existing trap, and the transcript carries `<work>` rather than a resolved scratch path.

## Self-Check: PASSED

- `tools/release-rehearse.sh`, `docs/packaging.md` and `10-REHEARSAL-TRANSCRIPT.md` all exist.
- Commits `0cce18b` (task 1) and `a488d83` (task 2) are ancestors of `HEAD`.
