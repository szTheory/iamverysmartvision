---
phase: 09-registered-evidence-and-gate-cost
plan: 08
subsystem: gate-driver
tags: [A2, EV-01, EV-04, A-05, preflight-partition, time-to-first-signal, sorted-replay, T-09-16, BUDGET-DEVIATION]
requirements-completed: [EV-01, EV-04]
dependency-graph:
  requires: [09-07]
  provides: [derived-preflight-partition, per-step-output-buffers, sorted-replay, gate-partition-line, gate-preflight-line]
  affects: [09-09]
tech-stack:
  added: []
  patterns: [derived-not-hand-maintained-classification, buffered-execution-sorted-replay, closed-derived-at-run-time-normalisation]
key-files:
  created:
    - .planning/phases/09-registered-evidence-and-gate-cost/09-PREFLIGHT-BEFORE-SH.log
    - .planning/phases/09-registered-evidence-and-gate-cost/09-PREFLIGHT-BEFORE-DASH.log
  modified:
    - tools/gate.sh
decisions:
  - "The preflight class is derived by grep over each step's source text (neither IVS_GATE_DIR nor build/), never a hand-maintained list, and the derivation is printed in the run header so a future reclassification reads as a reclassification"
  - "Every stdout-affecting emission moved into a replay loop over steps.have, so execution order (preflight then producers) and output order (sorted names) are independent; keep_fails runs in the replay, so fail.lines keeps its order"
  - "The before/after comparison normalises a closed set of nine derived-at-run-time members, not just the four measurement line shapes: leaving out ci-runs= or files= would report a correct partition as a defect"
metrics:
  duration: 32m
  completed: 2026-11-26
  tasks: 3
  files: 3
  commits: 3
  plan_head_before: 146a62d0c698b030aaa98042d69c1c242b05d0ac
  plan_head_after: e15dbbebc09d443d83daf17e280b65d328c81625
status: complete
actuals:
  tokens: 71000
  tasks: 3
  commits: 3
---

# Phase 9 Plan 08: The preflight partition

A gate run now reaches a self-consistency verdict in **31 s** instead of only at the end of the
run, while the normalised evidence a reader sees is **byte-identical** apart from two declared
signal lines and no absolute-and-frozen token moves. The 29 cheap consistency steps run first;
every step body is buffered and replayed in sorted name order afterwards.

## What changed

`tools/gate.sh` gains three things and loses one:

1. **The derived partition.** After `steps.have` is built, each step is classified by
   `grep -q -e 'IVS_GATE_DIR' -e 'build/' "$CHECK_DIR/steps/$name.sh"`: a match is a *producer*
   (it needs a built artifact), anything else is *preflight*. No list is maintained by hand, so a
   step that starts reading the results directory changes class on its own and shows up in the log.
2. **The header line**, printed before any step runs:
   `gate-partition preflight=29 producers=9: <the 29 preflight names>`.
3. **The early verdict**, printed as soon as the preflight group has finished and before any
   producer starts: `gate-preflight: PASS steps=29/29` (or `gate-preflight: FAIL <names>`). It never
   stops the run.
4. **One reused `$GATE_TMP/step.out` is gone.** Each step writes `$GATE_TMP/step.out.<name>`, plus
   `rc.<name>` and `t.<name>`; after both groups, a replay loop over `steps.have` emits `cat`,
   `keep_fails`, the pass/fail branch, the token accumulation and `step_time` - all of it - in
   sorted name order. Nothing is printed during execution.

`setup`, `controls` and `controls-wait` keep their positions and their `step-time` lines; the
negative controls still start in the background before the steps and are still waited for and
printed after them.

## The partition (derived, then asserted)

Derivation, printed in the run header:

```
gate-partition preflight=29 producers=9: api-effects c-purity ci-log ci-parity ci-summary claims
  cp1610 cp1610-reader denylist discrepancies files glossary hygiene hygiene-history
  important-gates input integrity listing manifests profile psg-vectors release-assets runner-scan
  runner-selftest sources state text video vocab
```

Producers, re-derived by the verify from step source text (not frozen, not copied):

```
c-build c-faults c-tests cp1610-vectors faults-verdict golden memmap pkg-install tests-inventory
```

**29 + 9 = 38**, matching `steps=38/38`. The frozen `preflight=29 producers=9` / `steps=29/29`
pair is asserted only as the *secondary* check, after the derived set equality holds, so a future
reclassification (09-05/09-06/09-07 edit step source text) reads as a reclassification rather than
an unexplained red.

## The preserving condition: a closed normalisation

`09-PREFLIGHT-BEFORE-{SH,DASH}.log` are the pre-change streams, taken on `146a62d` before a line of
`tools/gate.sh` changed, and committed in `fe09966`. They are the diff's only baseline.

The normalisation is **closed over the whole derived-at-run-time class**, nine members, each for a
stated reason:

| Member | Why it must be normalised |
|---|---|
| `^step-time <name> <s>` | wall-clock seconds per step; load-dependent |
| `^gate-cpu ...` | children CPU from `times`; load-dependent |
| `^gate-time ...` | total wall clock; load-dependent |
| `^gate-env ...` | `ncpu=` and the resolved `cc=` identity are host facts |
| `ci-runs=<n>` | both streams carry it (`-- ci-log pass ci-runs=<n>` and the summary line); a hosted row appended by the orchestrator between Task 1 and Task 3 moves it |
| `files=<n>` | this plan's own commits add tracked files, so the count moves between the two runs |
| `tracked=<n>` | same reason |
| `hygiene-files=<n>` | same reason |
| `history-commits=<n>` | same reason |

The four whole-line shapes become `<shape> ...`; the five inline values become `=N` wherever they
appear. The two declared new line shapes (`^gate-partition `, `^gate-preflight: `) are dropped from
the after-capture only. The function, used identically for the before-capture and the after-run:

```sh
norm() { sed -E \
 -e 's/^step-time [^ ]+ [0-9]+$/step-time .../' \
 -e 's/^gate-cpu .*$/gate-cpu .../' \
 -e 's/^gate-time .*$/gate-time .../' \
 -e 's/^gate-env .*$/gate-env .../' \
 -e 's/(^| )ci-runs=[0-9]+/\1ci-runs=N/g' \
 -e 's/(^| )files=[0-9]+/\1files=N/g' \
 -e 's/(^| )tracked=[0-9]+/\1tracked=N/g' \
 -e 's/(^| )hygiene-files=[0-9]+/\1hygiene-files=N/g' \
 -e 's/(^| )history-commits=[0-9]+/\1history-commits=N/g' "$1"; }
```

The `(^| )` anchor matters: it normalises `files=2554` but leaves `prefix-files=11`,
`blind-files=12` and `runner-files=16` alone, which are separate frozen tokens.

### The two diffs, both empty

```sh
sh tools/gate.sh > /tmp/after-sh.raw 2>&1
dash tools/gate.sh > /tmp/after-dash.raw 2>&1
for s in SH DASH; do
  raw=/tmp/after-$(printf '%s' "$s" | tr 'A-Z' 'a-z').raw
  norm "$raw" | grep -v -e '^gate-partition ' -e '^gate-preflight: ' > "/tmp/after-$s.log"
  diff -q "/tmp/after-$s.log" \
    ".planning/phases/09-registered-evidence-and-gate-cost/09-PREFLIGHT-BEFORE-$s.log"
done
```

`diff` output: **empty for both shells**. 518 lines each, before and after.

## Time to first signal

Measured by prefixing every output line with elapsed seconds:

| Figure | Before | After |
|---|---|---|
| elapsed at `gate-partition` line | n/a (line did not exist) | 0 s |
| elapsed at `gate-preflight: PASS steps=29/29` | n/a | **31 s** |
| elapsed when the last preflight step's body was printed | **79 s** (of a 79 s run - the last preflight step, `vocab`, sorts after the build and both suites) | 31 s (of a 76 s run) |
| elapsed at the first step body line | 1 s | 75 s (the replay is after both groups) |

The recorded expectation was 21-24 s; the measurement is **31 s**, taken on a machine at
**load average 12.86** with 18 CPUs, which is the honest reason for the gap: the preflight group is
mostly process-spawning, so it tracks load. The previous arrangement's comparable figure is 79 s
(the run's end), not the 44 s the CI-EFFICIENCY entry estimated.

**Hosted cost is unchanged by construction**: the same 38 steps run, in the same total work, and
produce the same evidence. The win is local time-to-first-signal - a developer-experience win -
and it is **not** claimed as a hosted saving.

## Capture provenance (Task 1)

| | `sh` | `dash` |
|---|---|---|
| raw capture | `/tmp/before-sh.raw` | `/tmp/before-dash.raw` |
| normalised file | `09-PREFLIGHT-BEFORE-SH.log` | `09-PREFLIGHT-BEFORE-DASH.log` |
| normalised line count | 518 | 518 |
| raw last line | `gate: PASS ...` (verbatim below) | identical to `sh` |

Taken on commit `146a62d0c698b030aaa98042d69c1c242b05d0ac`, load average `12.86 14.01 13.99`
(18 CPUs), both runs back to back with nothing edited between them, both `rc=0`.

Raw summary lines, verbatim (identical for both shells):

```
gate: PASS steps=38/38 controls=384/384 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=235 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=78 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=666 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=235/235 tracked=2552 binaries-listed=0 terms=9 doc-files=59 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2552 history-commits=1 gates=107 lines=330 input-rows=31 input-policies=11 input-profile-fields=31 files=2552 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=16 cases=12 sources=12 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 docs=60 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

## Tokens

**Absolute-and-frozen - nothing moved.** `steps=38/38`, `controls=384/384`, `faults=235/235`,
`gates=107`, `lines=330`, `cases=12`, `tests-expected=4958`, `tests-executed=4954`,
`tests-skipped=4`, `tests-missing=0`, `claims=28`, `provisional=27`, `docs=60`.

**Derived-at-run-time - recorded, not asserted as equality.** Observed triple, in steps:
`files=2554 tracked=2554 hygiene-files=2554` after the two captures (+2), and
`files=2555 tracked=2555 hygiene-files=2555` after this SUMMARY (+3 in total), against a pre-plan
baseline of 2552 on `146a62d`. **+2, then +1, exactly the three added files**, with
`files == tracked == hygiene-files` at both points. `history-commits=1` and `ci-runs=78` unchanged
in this run, and both normalised regardless, because either can move for reasons this plan does not
control.

The two shells' summary tokens agree after the closed normalisation (verify 2 of Task 3): PASS.

## Container gate

**not run** - approved budget deviation for this dispatch (see below). The orchestrator runs
`tools/gate-linux.sh` at the wave boundary.

## Deviations from Plan

### Approved budget deviation (instructed by the dispatch, recorded here as required)

The dispatch carried a hard 1800-second limit and instructed: run `sh tools/gate.sh` at the end of
each task, run `dash tools/gate.sh` once at the end of Task 3, and **do not run the container
gate** - the orchestrator runs it at the wave boundary. Task 3's step 3 therefore did not run
`tools/gate-linux.sh`, and no `build/gate-linux-a2-preflight.log` was produced. Everything else in
Task 3 ran: both after-captures, the closed normalisation, both empty diffs, the token comparison
and the time-to-first-signal measurement. Task 3's container verify is satisfied by the "not run"
line above (the plan's `fails_when` for that verify explicitly allows it).

Consequence for the wave: the container half of this plan's evidence is outstanding at hand-off.

### Auto-fixed issues

**1. [Rule 3 - Blocking] The plan's Task 3 verify has an unset-variable typo**

- **Found during:** Task 3, verify 1
- **Issue:** the verify's loop variable is `$s` but the `diff` line reads `/tmp/after-$S.log`.
  `$S` is unset, so the command becomes `diff -q /tmp/after-.log ...` and fails with
  `No such file or directory` on a correct partition. The defect is in the verify script, not in
  the gate.
- **Fix:** ran the same verify with `$s` in both places. No production code changed.
- **Files modified:** none (the plan's `<verify>` block is not a committed file).

No other fix was needed; the first `sh` and `dash` runs after the edit were both green with the
predicted token, and both normalised diffs were empty on the first attempt.

## Known Stubs

None. The partition, the buffers and the replay are complete mechanisms that run in every gate; the
two captures are committed evidence.

## Threat Flags

None beyond the plan's register. `T-09-16` (tampering with the step loop and replay) is mitigated
as planned: every stdout-affecting emission is produced during the sorted replay, `keep_fails` runs
in the replay so `fail.lines` and the job summary keep their order, and the normalised diff against
a committed pre-change capture is a required verify rather than a claim. `T-09-SC` is accepted: no
package manager was invoked and no dependency was added.

## Self-Check

- `09-PREFLIGHT-BEFORE-SH.log`, `09-PREFLIGHT-BEFORE-DASH.log` - both exist, 518 lines, last line
  `gate: PASS`, every measurement shape normalised to `<shape> ...`, all five inline tokens at `=N`
- commit `fe09966` touches only the two captures and **not** `tools/gate.sh` (the capture precedes
  the change)
- `sh tools/gate.sh` - `gate: PASS`, one `gate-partition preflight=29 producers=9:` line, one
  `gate-preflight: PASS steps=29/29` line, both before the first step body (lines 2 and 3, first
  body at line 4)
- `dash tools/gate.sh` - same three lines, `gate: PASS`
- derived producer set equals `c-build c-faults c-tests cp1610-vectors faults-verdict golden memmap
  pkg-install tests-inventory`; `preflight + producers = 29 + 9 = 38`
- `grep -c 'step\.out\.' tools/gate.sh` - 6; no path writes `$GATE_TMP/step.out` without a suffix
- normalised before/after diff - empty for both shells
- `LC_ALL=C grep -n '[^ -~]' tools/gate.sh` - EMPTY (plain ASCII hyphens in the new comments)
- commits `fe09966`, `119bd32` - both ancestors of `HEAD`
