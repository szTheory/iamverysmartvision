---
phase: 04-headless-runner-and-installed-consumer
plan: LEVER-01
subsystem: gate, ci
tags: [gate, controls, ci-efficiency, D-45, perf]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "04-10 end state: steps=29/29 controls=252/252 faults=134/134 tests-expected=2372; hosted rows in .planning/ci-runs.tsv (last 3 gate_s 69, 60, 54: D-45 rule fired)"
provides:
  - "faults-verdict judges each faults file in one awk pass (was about 17 processes per fault)"
  - "denylist batches one grep -l per list over all in-scope files, with a per-file fallback on any grep error"
  - "hygiene.sh --history applies the binary-name rule in one grep over the sorted name list"
  - "run-controls.sh: one cp per tree, builtin CHECK and blank-EXPECT checks, one awk for EXPECT/FORBID, one mkdir for all work dirs, one cat for results"
  - "runner-selftest cases=8 (was 4): pins the runner's token and blank-line semantics"
affects: [04-19, 04-25]

actuals:
  tokens: 5000
  tasks: 1
  commits: 1
plan_head_before: 9716adc317a7e2c3a0b419b9af1daf05aa4f7525
plan_head_after: d8d771217b9df7a998c72577986e9cb5a793f092

tech-stack:
  added: []
  patterns:
    - "Batch first, per-item fallback: run one process over all inputs; on any tool error, rerun the old per-item pass so error tokens still name the item"
    - "Sorted-list walk with a second fd: a grep over a sorted list keeps its order, so a builtin loop can merge it with the full list without a process per line"
    - "Differential proof for a rewritten check: old and new script over the same mutated inputs under sh and dash, outputs compared byte for byte"

key-files:
  created:
    - .planning/phases/04-headless-runner-and-installed-consumer/04-LEVER-01-SUMMARY.md
  modified:
    - tools/check/run-controls.sh
    - tools/check/steps/faults-verdict.sh
    - tools/check/steps/denylist.sh
    - tools/check/steps/runner-selftest.sh
    - tools/hygiene.sh
    - .planning/CI-EFFICIENCY.md

key-decisions:
  - "Measured before changing: on macOS the gate's system time is fork/exec cost (about 21000 exec calls per gate); on native Linux (the hosted OS) system time is 12 percent and the gate is compile- and test-bound, so these levers cut local CPU much more than hosted CPU"
  - "Control trees stay full copies: hardlinks or symlinks would let MAKE verbs, which append to tree files in place, write into tools/check/controls/_good/tree"
  - "runner-selftest gains four cases, so its stats token changes from cases=4 to cases=8; every other token and line of the gate output is unchanged"

requirements-completed: []

duration: about 3 h (mostly measurement; laptop load 11-36)
completed: 2026-11-11
---

# Phase 4 Lever 01: Fewer gate process spawns (D-45) Summary

**The gate now starts about 42 percent fewer processes (20988 to 12203 exec calls per run). On macOS that cuts gate CPU by about 18-25 percent under sh (12-25 percent under dash) and wall time by about 29 percent. On native Linux, which the hosted runner uses, the cut is only about 8 percent (warm) to 12 percent (cold), because there the gate is bound by compiling and testing, not by starting processes. No check, control, token, inventory or output order changed.**

## Measurement first (where the time went)

Method: a profiling copy of `tools/gate.sh` that records POSIX `times` after each step (controls run in the foreground so they are not charged to a random step); a profiling copy of `run-controls.sh` with `times` around tree build and step per control; a PATH shim directory that logs every exec of about 60 common tools with the current step as a tag; a native arm64 Ubuntu 24.04 container (gcc 13.3, dash) for the Linux profile. All of these are scratch-only and not committed.

**Exec calls per gate run (macOS, deterministic):**

| part | before | after |
|------|-------:|------:|
| whole gate | 20988 | 12203 |
| control runner and its 252+24 step runs | 13436 | 9040 (9102 inside a gate run) |
| faults-verdict (main step) | 2518 | 240 |
| hygiene-history (main step) | 1393 | 52 |
| denylist (main step) | 854 | 19 |
| controls: faults-verdict / denylist / hygiene-history | 1279 / 984 / 347 | 538 / 79 / 160 |
| control runner's own calls (tree builds 1964 to 1412, token checks 1053 to 252, inventory and results 837 to 44) | 3854 | 1708 |

**Ranked CPU before (cpu-seconds, user+sys):**

| step | macOS warm (load 10-36) | Linux native arm64, cold build |
|------|------------------------:|-------------------------------:|
| controls (overlapped) | 51-62 (sys 36-45) | 10.9 |
| c-build | 0.5 warm, 44 cold | 30.4 |
| c-faults (134 fault executables) | 13-14 | 20.3 |
| c-tests (ctest dev + release) | 3.3 | 18.0 |
| faults-verdict | 4.2 (sys 2.6) | 0.9 |
| hygiene-history | 5.8-10.2 | 1.3 |
| cp1610-vectors | 2.9-3.2 | 5.0 |
| hygiene | 2.3-4.4 | 0.3 |
| c-purity | 2.0-2.5 | 1.5 |
| denylist | 1.6-2.1 | 0.3 |
| whole gate | 104-138 | 91 (sys 10.8) |

Control CPU by step on macOS: the 26 c-purity controls take 11.5 cpu-s (the 10 symbol-layer ones compile the core twice and run the stage-3 self-test), cp1610-vectors 4.3, faults-verdict 3.3, denylist 2.2, hygiene-history 1.6, the rest under 1.6 each. Tree builds took 9.4 cpu-s in total, about 34 ms per tree, mostly in process starts (8 processes per tree), not in copying.

## Levers applied

1. **faults-verdict: one awk per faults file.** Ids, duplicates, columns, unknown vectors, nonce, list equality, not-detected, collateral and wrong-reason are judged in one awk pass that reads the result files with getline. Result-file existence still uses the shell's `[ -f ]` (a glob over `faults/*/<suite>.txt`). Prefix splitting copies the shell's `IFS='|'` rules (an empty field matches every vector; a trailing `|` adds none). Standalone on macOS: 4.2 cpu-s to 0.47.
2. **denylist: batched prefilter.** One `grep -l -a -i -F -f <list>` per list over every in-scope file (xargs -0, in scan order). Only files that match the cite list are read line by line. If any batch grep returns an error, the old per-file pass runs over every file. Standalone: 1.6 cpu-s to 0.23.
3. **hygiene.sh --history: one grep for binary names.** The matches keep the sorted list's order, so a builtin loop walks both lists in step (second fd). A grep error is now a finding (`hygiene-unreadable: history-names-grep`); before, it read as no match. This is the only behaviour change, and it fails closed.
4. **Control runner.** One `cp -R` of the good tree per control, with no `rm` and `mkdir`. `CHECK` is read with `read`. The blank-EXPECT check uses `read` plus `case *[![:space:]]*`. One awk checks all EXPECT and FORBID tokens: fixed strings within a line, as `grep -F` did, and an awk failure counts as a missing token. All work dirs are made with one `mkdir`. Results are printed by one `cat` when every result file exists, and one by one otherwise. Control dir names come from parameter expansion instead of `basename`.

## Equivalence evidence

- **All 252 controls and 24 positive controls:** every step output and every result file is byte-identical between old runner + old steps and new runner + new steps (2302 lines; commit hashes in hygiene-history outputs are normalized, since they change on every run). The runner's own output is byte-identical between the old runner and the new runner under sh and under dash.
- **faults-verdict:** 35 mutation cases, each run on old and new under sh and dash, all with identical output and exit status. The cases cover a missing result, a result that is a directory, a stale nonce, no nonce line, an empty gate nonce, a dropped or duplicated line, an extra id, a tab after the number, an empty NOTOK id, not-detected, collateral, wrong-reason, empty/leading/trailing/only `|` prefix fields, duplicates across and within files, a bad id, 3 and 5 columns, blank-and-tab lines, a GEN_ id without a result, an unknown vector, a missing suite header, an unknown suite, a file with no faults, a leading-space id, no trailing newline, no ids files, a shrunken suite list and a missing site.
- **denylist:** 11 cases with old and new under sh and dash, all identical. They cover a clean tree, a hit, an uppercase hit in a name with a space, cite waived, cite mixed, out of scope, NUL bytes, names starting with `-`, an unreadable file (fallback path, `denylist-grep-error`), 3000 files (several xargs batches) and no files.
- **hygiene --history:** a repository with 12 names (binary extensions in mixed case, `refs/`, `.env` forms, a space, a backslash, non-ASCII, look-alikes) and a clean repository give identical output under sh and dash.
- **runner-selftest (committed):** the new cases are `st-expect-space`, `st-second-token-missing`, `st-forbidden-token` and `st-ok`. They pass against the old runner too, so the semantics are unchanged. They fail against three mutants: a runner that checks only the first token, one that ignores FORBID, and one whose blank check sees only spaces or drops a last line without a newline.

## Before / after

Same-session A/B on two copies of the tree (HEAD before, working tree after), alternating, warm build, macOS 18 cores, Apple clang 21, load 11-20 (cpu-s = user+sys from `gate-cpu`):

| | before | after (all levers) | change |
|---|---|---|---|
| sh cpu-s | 137.6 128.5 130.1 129.9 108.2 121.3 (mean 125.9) | 93.5 99.5 99.7 (mean 97.6) | -22 percent (-18.5 against the same round alone) |
| sh wall s | 50 45 49 46 38 44 (mean 45) | 29 38 30 (mean 32) | -29 percent |
| dash cpu-s | 113.1 112.3 123.3 112.8 (mean 115.4) | 87.0 108.4 109.2 (mean 101.5); 85.8 in round 1 with four of five levers | -12 to -25 percent |
| dash wall s | 39 37 47 41 (mean 41) | 26 35 36 (mean 32) | -21 percent |
| Linux arm64 native, warm, cpu-s | 61.9 57.2 71.3 (mean 63.5) | 53.4 56.0 66.6 (mean 58.7) | -8 percent |
| Linux arm64 native, cold, cpu-s | 108.3 | 94.8 | -12 percent (one run) |
| Linux arm64 native, warm, wall s | 31 30 39 | 27 30 34 | -9 percent |

Per step on macOS (step-time s, sh, before / after): controls 26 23 23 / 20 20 20, faults-verdict 7 6 6 / 1 1 1, hygiene-history 7 6 8 / 3 2 2, denylist 4 3 3 / 0 1 0. Other steps are unchanged (c-faults 6-7, c-tests 4-5, cp1610-vectors 4-5, hygiene 3-4, c-purity 3-4).

The 25 percent CPU target is met on macOS only in some runs. sys time there depends on load, so the runs spread by about 15 percent. The deterministic exec-call count fell by 42 percent. On Linux the target is not met: the levers remove about 5 cpu-s of a 60-110 cpu-s gate. The hosted effect has to be read from the next `ci-runs.tsv` rows, which the orchestrator records.

## Levers rejected or deferred

- **Hardlinked or symlinked control trees:** rejected. MAKE verbs (`nonascii`, `homepath`, `secret` and others) append to tree files in place, and through a link those writes would reach `tools/check/controls/_good/tree`. The per-tree cost was process starts, not bytes, and that part was cut.
- **Skipping the c-purity stage-3 self-test inside controls:** rejected. It would change what the step checks under a control, even though stage 3 does not read ROOT.
- **Batching hygiene --tree candidate filters:** deferred. It would gain about 2 percent locally, but it needs filename parsing of multi-file `grep -o` output, and the step is the public-repo leak guard.
- **vectors JSON:** deferred. `string(JSON ... GET "${doc}" vectors ${i})` re-parses the whole file for each vector (branch.json: 98 vectors x 237 KB). On Linux this costs about 5 cpu-s in cp1610-vectors plus the ivs_vectors.inc generators in the cold build (1.3 s dev, 1.3 s release and 2.7 s for GEN_EXPECT_MUTATION on macOS). A rewrite needs the generator and validator outputs proven byte-identical.
- **faults-verdict site scan** (19 files x 3 processes): left as it was, because the gain is small.
- **ccache, workflow, matrix or a two-job split:** not allowed by this task, and ccache is banned by D-45 while the gate is within budget.

## What remains the biggest cost

- **Hosted / Linux:** cold `c-build` (about 30 of 91 cpu-s native, 22-32 s hosted wall); `c-faults` (about 20 cpu-s: 134 fault executables, each running its whole suite); `c-tests` (about 18 cpu-s and 15-18 s wall, because ctest runs serially). Under gcc ASan+UBSan, dev `memmap` takes 5.4 cpu-s, `cpu-sweep` 2.3, `cpu-fuzz` 2.3, `machine` 2.0 and `run` 1.7; on macOS/clang the whole dev ctest takes 1.9 s. After those come controls (about 11) and cp1610-vectors (about 5).
- **macOS:** controls (about 45 cpu-s; the c-purity controls are a third of the control exec calls) and c-faults (about 13).
- **Next hosted wall lever (not applied, orchestrator/owner call):** run ctest with `-j` in c-tests. It is CPU-neutral and could save about 10 s of hosted wall on the serial 15-25 s step. Other options are D-45 lever 2 (background c-build) or the vectors JSON rewrite above.

## Task Commits

1. **Lever: fewer process spawns in controls, faults-verdict, denylist, hygiene history** - `d8d7712` perf(04)

## Final gate summary lines

- `sh tools/gate.sh` (macOS): `gate: PASS steps=29/29 controls=252/252 api-calls=14 ... fault-runs=134 ... faults=134/134 ... tests-expected=2372 tests-executed=2372 tests-skipped=0 tests-missing=0 ...`, `gate-cpu user=46.20 sys=46.95`, `gate-time total=28`. Apart from timing lines, the output equals the pre-lever run except `-- runner-selftest pass cases=8` (was 4) and that same token in the summary line.
- `dash tools/gate.sh`: the same summary line byte for byte; `gate-cpu user=51.13 sys=52.88`, `gate-time total=35`.
- `sh tools/gate-linux.sh` (Ubuntu 24.04 amd64 under emulation, gcc 13.3, cmake 3.28, dash): `gate: PASS steps=29/29 controls=252/252 ... cc=GNU ... core-imports=3 ...`, `-- runner-selftest pass cases=8`, `gate-cpu user=582.83 sys=55.69`, `gate-time total=156`, `gate-linux: exit 0`.

## Deviations from Plan

- This is not a numbered plan. The brief expected system time to be the hosted cost. Measurement showed that this holds on macOS only (see the decisions), so the record reports both platforms instead of one number.
- `runner-selftest` stats change from `cases=4` to `cases=8`. The brief asked for self-test proof when runner code paths change, so this one stats token differs from the pre-lever output.
- `hygiene.sh --history`: a grep error over the name list is now a finding (fails closed); before, it was silently no match.

## Open Items

- D-45: the median of the last 3 hosted gate-times is 60 s (69, 60, 54), and one run is above 60 s, so the "revisit the budget" trigger has fired too. The orchestrator should record the next hosted rows and compare `cpu_s` and `gate_s`. This lever is expected to remove about 5-10 cpu-s and 2-4 s of hosted wall. The remaining hosted cost is c-build, c-faults and c-tests, as listed above.
- Carried from 04-10: unchanged.

## Self-Check: PASSED

- Files present: the five modified tools files and this SUMMARY.
- Commit reachable from HEAD: d8d7712 (`git rev-list --count 9716adc..HEAD` = 1 before the docs commit).
