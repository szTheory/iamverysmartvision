---
phase: 20-route-witness-and-release-posture
plan: 01
subsystem: operator-route
tags: [operator-section, executed-as-written, appended-not-rewritten, refs-containment, command-surface-agreement, owner-local-route, build-only-instrument]

requires:
  - phase: 18-the-inspection-surface
    plan: 01
    provides: "`docs/inspection.md` as the frozen contract: the `## Command surface` block (four subcommands, three exit statuses), the `## The trace file` record fields, the five refusal tokens and the six appended amendments the new section is appended after"
  - phase: 18-the-inspection-surface
    plan: 03
    provides: "`build/dev/tools/inspect/ivs-inspect` and `build/release/tools/inspect/ivs-inspect`, the real output shapes the section states"
  - phase: 19-discriminating-the-display-path
    provides: "`data/scenarios/diag-boot-pass.ivss` already registered `pass` in `data/expected/runner-scenarios.ids`, and the trace and step shapes the frozen page pins"

provides:
  - "`docs/inspection.md` `## The operator route`: seven `sh` blocks, thirteen command lines, all executed as written from the repository root against the project's own scenario, with `refs/` created by the section's own first line"
  - "the statement that `report.end` is the report's line count and never the stop tick, and that the step report is `4 + 8 + peek_count` lines"

affects:
  - "`data/expected/rights-pinned.ids`: unmoved at four pairs; the naming limb widened by one page and by zero rights tokens"
  - "the gate line: no id, control, row, golden, claim or installed file moved; only `tracked`/`hygiene-files`, by this SUMMARY"

tech-stack:
  added: []
  patterns:
    - "an operator section is proved by extracting its command lines from the one fence tag and executing them verbatim, so a line that needs a local tweak exits non-zero instead of being proofread"
    - "an appended section states a relation wherever the value is derivable from the invocation and a literal only where the frozen page pins it or where the number this tree printed is re-measured by the same task's verify"

key-files:
  created:
    - .planning/phases/20-route-witness-and-release-posture/20-01-SUMMARY.md
  modified:
    - docs/inspection.md

key-decisions:
  - "The section is appended after every amendment and is the page's last `## ` heading; the diff from the freeze commit `0627be92` adds lines and removes none."
  - "`report.end 78` is stated as the report's line count (the file holds seventy-nine printed lines), and the stop tick is taken from the trace's `end` record, which is `2084` and equals the single step record's `end=` and the report's `step.1.now_ticks`. The two are asserted separately and never conflated."
  - "The step block peeks one word, not two, because the report is `4 + 8 + peek_count` lines: thirteen for one word. The verify re-runs the extracted line itself and counts what it printed, so the page cannot drift from the invocation."
  - "The option comparison is scoped to the `ivs-inspect` invocations; every option token outside them is held to an explicit three-item allow-list (`--build`, `--preset`, `--target`) that cmake itself owns, which narrows one comparison rather than opening a hole."
  - "`docs/owner-images.md` gains its section between `## The sweep probe` and `## The template scenario and the gate`, following the page's own precedent that a second and third owner-local route sit beside the helper and the probe."

patterns-established:
  - "One fence tag, one extraction: the section uses exactly ```` ```sh ```` for its command blocks, because a differently tagged fence yields an empty command file that executes successfully and asserts nothing."
  - "`mkdir -p refs` is the section's first command line, so the command set executes as written on a clone that has never had that directory."

requirements-completed: [VID-08]

metrics:
  duration: "~1h10m"
  completed: 2026-11-27
  tasks: 2
  files: 3

status: complete

actuals:
  tokens: 9200
  tasks: 2
  commits: 2
---

# Phase 20 Plan 01: The Operator Route Summary

An operator can reach the inspection instrument from the documentation alone: the frozen page gains an appended section whose thirteen command lines execute as written against the project's own scenario, and the owner-local route page names the instrument at the path the release preset writes.

## Baselines

start-commit: 57f9b31ef53e2d54522887dbaf36bdf805ea9ffa

Measured at `start-commit` before the first edit of this plan, each count read from the file that produces it:

| Count | Value | Read from |
|---|---|---|
| gate steps | 38 | `ls tools/check/steps/*.sh \| wc -l` |
| control directories (excluding `_good`) | 405 | `ls -d tools/check/controls/*/ \| grep -v _good \| wc -l` |
| handoff ledger data rows | 8 | `grep -v '^#' data/expected/handoffs.ids \| awk 'NF' \| wc -l` |
| rights pins | 4 | `grep -v '^#' data/expected/rights-pinned.ids \| awk 'NF' \| wc -l` |
| installed paths | 11 | `grep -v '^#' data/expected/pkg-install-files.ids \| awk 'NF' \| wc -l` |
| claim entries | 29 | `grep -c '"id": "CL-' data/claims.json` |
| tracked files | 2857 | `git ls-files \| grep -c .` (captured to a file) |

Gate summary line measured at `start-commit`, one `sh tools/gate.sh` run, verbatim:

```
gate: PASS steps=38/38 controls=405/405 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=118 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=29 provisional=28 handoffs=8 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=705 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2857 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=64 profile-keys=155 goldens=29 reports=31 host-dependent=3 release-only=2 mode=tree hygiene-files=2857 history-commits=1 gates=123 lines=378 input-rows=31 input-policies=11 input-profile-fields=31 files=2857 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=11 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=21/21 records=155 fields=155 unknown-fields=48 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=13/13 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5114 tests-executed=5108 tests-skipped=6 tests-missing=0 docs=65 claim-refs=27 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

## Task 1: The operator section appended and every one of its lines executed as written

### The appended section

`## The operator route` is appended to `docs/inspection.md` as the page's **last `## ` heading** (line 332, and `grep -n '^## '` ends on the same line), after `## Amendments` and its six notes. It opens by stating that it is appended by plan 20-01, states no new rule, corrects no sentence above it, and that a line needing a local tweak is a defect in the page rather than in the reader.

`git diff --numstat 0627be92..HEAD -- docs/inspection.md` after the commit: additions only, **zero deletions**, so no frozen sentence moved.

### The command set

Seven ```` ```sh ```` blocks, thirteen command lines, no line continuation and no placeholder. Extracted with the same `awk` the verify uses:

| # | line | exit | shape it printed |
|---|---|---|---|
| 1 | `mkdir -p refs` | 0 | the directory exists (it already did on this tree) |
| 2 | `cmake --preset dev` | 0 | `-- Configuring done (0.4s)` / `-- Generating done (0.9s)` / `-- Build files have been written to: .../build/dev` |
| 3 | `cmake --build --preset dev --target ivs_inspect ivs_images` | 0 | `[0/2] Re-checking globbed directories...` / `ninja: no work to do.` |
| 4 | `ivs-inspect trace ... --out refs/op-trace.ivst --break 1000 --other 1002 > refs/op-trace.report` | 0 | the report file and the trace file, shapes below |
| 5 | `ivs-inspect break --pc 1000 --other 1002 --trace refs/op-trace.ivst --out refs/op-break.txt` | 0 | the seven-key break report |
| 6 | `ivs-inspect dis fixtures/FX-boot-stub/boot-stub.listing.txt --out refs/op-dis.asm` | 0 | the emitted source |
| 7 | `ivs-inspect step ... --units 70 --peek 0200 1` | 0 | thirteen `key value` lines on stdout |
| 8 | `sed -n '1p;$p' refs/op-trace.report` | 0 | `ivsr 3` then `report.end 78` |
| 9 | `sed -n '$p' refs/op-trace.ivst` | 0 | `end tick=2084 records=112 units=70 steps=1 break_pc=1000 break_hits=1 other_pc=1002 other_hits=0` |
| 10 | `grep -c '^tick=' refs/op-trace.ivst` | 0 | `112` |
| 11 | `sed -n 's/^break\.hits //p' refs/op-break.txt` | 0 | `1` |
| 12 | `sed -n 's/^break\.other\.hits //p' refs/op-break.txt` | 0 | `0` |
| 13 | `sed -n '1p;$p' refs/op-dis.asm` | 0 | `# id: boot-stub` then `        J $5000` |

Whole set executed with `sh -e /tmp/p20-ops.sh` from the repository root: **exit 0**, no line needing a tweak.

### The stated shapes, against the invocation

| statement on the page | the invocation's own number |
|---|---|
| the report's first line is `ivsr 3` | `ivsr 3` |
| the report's last line is `report.end 78`, the count of lines before it | 79 printed lines, last line `report.end 78`, and `78 = 79 - 1` |
| the stop tick comes from the trace's `end` record and equals the single step record's `end=` and `step.1.now_ticks` | `end tick=2084` = `step=1 target=16000 end=2084 stop=halted` = `step.1.now_ticks 2084` |
| `report.end` is not that stop tick | `78 != 2084`, asserted as a separate limb |
| the trace's first record is an `iab` access record | `tick=4 kind=iab addr=0000 value=1000 wait=0 cycle=1 unit_start=0 unit_end=20` |
| `end`'s `records=` equals the number of `tick=` lines | `records=112` and `112` `tick=` lines |
| the break report holds the seven pinned keys; `break.hits` `1`, `break.other.hits` `0`; `break.records`/`break.end_tick` copy the trace's `end` | 7 `break.` lines, `1`, `0`, `112`, `2084` |
| the disassembly's first line is `# id: boot-stub`, it carries `ORG $1000`, it ends at the jump to `$5000` | all three |
| the step report is thirteen lines for a one-word peek, from `step.units 70` to `step.mem.0200` | thirteen lines, `step.units 70`, last `step.mem.0200 7FFF` |

### The two constraints on the added text

No claim word (`supports`, `compatible`, `passes`, `accurate`, `verified`, `runs`, `plays`, `playable`, `looks`, `sounds`, word-bounded and case-insensitive) appears on any added line, and neither pinned rights spelling (`exec-image`, `grom-image`) appears anywhere in the section. Checked over `git diff` added lines: zero hits on both.

### The command surface

Every subcommand used (`trace`, `break`, `dis`, `step`) is one the frozen `## Command surface` block names. Every option token on an `ivs-inspect` invocation (`--content-dir`, `--out`, `--break`, `--other`, `--pc`, `--trace`, `--units`, `--peek`) is one that block names. The only option tokens outside those invocations are cmake's own `--preset`, `--build` and `--target`, held to an explicit three-item allow-list.

### Cleanup

The four files under `refs/` (`op-trace.ivst`, `op-trace.report`, `op-break.txt`, `op-dis.asm`) were removed before the commit. `git status --porcelain` is empty apart from the files this commit stages, and `git ls-files` names no `.ivst` path and no path under `refs/`.

## Token line, before and after

| token | before | after | why |
|---|---|---|---|
| `steps` | 38/38 | 38/38 | no gate step added |
| `controls` | 405/405 | 405/405 | no control added |
| `faults` | 238/238 | 238/238 | no fault site |
| `goldens` | 29 | 29 | no golden |
| `claims` / `provisional` | 29 / 28 | 29 / 28 | no claim minted or edited |
| `handoffs` | 8 | 8 | plan 20-02 adds the ninth row |
| `prefix-files` | 11 | 11 | no installed path |
| `rights-pinned` | 4/4 | 4/4 | naming limb unmoved |
| `gates` / `lines` | 123 / 378 | 123 / 378 | no registry row |
| `tests-expected` / `tests-executed` | 5114 / 5108 | 5114 / 5108 | no test id |
| `tracked` / `hygiene-files` | 2857 | 2858 | `20-01-SUMMARY.md`; `docs/inspection.md` is a modification |

The gate summary lines are recorded in the closing section below, each run sequentially, `sh` then `dash`, with the two lines compared by `cmp`.

## Cost arithmetic

Recomputed by arithmetic from the last five rows of `.planning/ci-runs.tsv`, never quoted:

| column | the five values | median |
|---|---|---|
| `build_s` | 51, 51, 49, 49, 35 | **49** |
| `gate_s` | 171, 165, 164, 154, 115 | **164** |
| `cpu_s` | 584, 580, 572, 530, 368 | **572** |

This plan adds zero in-process ids (0.065), zero process-launch ids (0.10) and zero registry rows (0.005), so the frozen unit prices give **0.00**. Its share of the phase's notional 0.02 is **0.01**, charged against the 21 hosted cpu-s allowance (`GUARDRAILS-v0.4` §2.3 prices Phase 20 at 0.02 of the 4.72 total; the ceiling is 5.6). The checkpoint verdict is **`discharged-within-allowance`**: the median moved by a runner event, not by this plan, and the token line moved by one tracked file.

## Deviations from Plan

None - the plan executed as written. Two points worth recording as confirmations rather than deviations:

- The plan's predicted shapes matched the invocation on the first execution with no adjustment: `ivsr 3`, `report.end 78`, 79 report lines, `end tick=2084`, `records=112`, thirteen step lines, `step.mem.0200 7FFF`, the seven break keys with `1`/`0`.
- `refs/` already existed on this tree (the owner keeps `refs/FX-owner-exec.bin` and `refs/FX-owner-grom.bin` there), so `mkdir -p refs` was a no-op here. The line is still the section's first command, and it is what makes the set executable on a clone that has never had the directory.

## Open items

- `tools/gate-linux.sh` was **not run** in this plan: the container daemon is unresponsive on this host, which the orchestrator's rules for this phase record. A visible `not run` line is carried here rather than a pass.

## Final gate

Recorded in the completion section: `sh` then `dash`, sequentially, both summary lines compared with `cmp`.

## Self-Check: PASSED

- `docs/inspection.md` exists and carries `## The operator route` as its last `## ` heading.
- `build/dev/tools/inspect/ivs-inspect` and `build/release/tools/inspect/ivs-inspect` both exist and are executable.
- `refs/` holds no `op-*` file after the runs.
