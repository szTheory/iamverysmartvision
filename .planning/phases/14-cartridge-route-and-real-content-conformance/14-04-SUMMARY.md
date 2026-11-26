---
phase: 14-cartridge-route-and-real-content-conformance
plan: 04
subsystem: records (requirement flips, the important-gate registry, the toy tree, the cost checkpoint and the validation sign-off)
tags: [records, gate-registry, toy-tree, cost-checkpoint, container, validation]
status: complete
dependency-graph:
  requires:
    - .planning/REQUIREMENTS.md (the traceability table is the parsed accepted set)
    - data/expected/important-gates.ids, data/expected/controls/*.ids, data/expected/faults/*.ids, data/expected/tests/*.ids
    - tools/check/run-controls.sh (the toy-registry drift check in both directions)
    - .planning/ci-runs.tsv (the arithmetic input for the cost checkpoint)
    - plans 14-01, 14-02 and 14-03 (the mechanisms these five rows register)
  provides:
    - TOOL-07, SCN-03 and CPU-06 Complete with five live mechanism rows
    - the re-derived registry baseline (gates=117 lines=357; toy tree gates=46 lines=49)
    - the closing gate line under sh, dash and the Ubuntu 24.04 container
    - one cost verdict, `reprice`, with its arithmetic
    - a signed 14-VALIDATION.md
  affects:
    - .planning/ROADMAP.md Phase 14 (4/4 plans executed, phase complete)
    - the milestone close (the phase's cost row and its hosted row)
tech-stack:
  added: []
  patterns:
    - a requirement flip and the registry rows that name it land in one commit, because the step refuses both directions
    - the registry baseline is the step's own printed stat, re-derived by running it against the tree
    - the cost verdict is arithmetic over the committed TSV, with the five values and both medians printed beside it
key-files:
  created: []
  modified:
    - .planning/REQUIREMENTS.md
    - data/expected/important-gates.ids
    - tools/check/steps/important-gates.sh
    - tools/check/controls/_good/tree/.planning/REQUIREMENTS.md
    - tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md
    - tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md
    - tools/check/controls/_good/tree/data/expected/important-gates.ids
    - tools/check/controls/important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-bad-req-phase6/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-req-uncovered-phase5/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-req-uncovered-phase6/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-req-uncovered-phase7/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-uncovered-gate/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-unknown-id/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-unknown-id-phase5/overlay/data/expected/important-gates.ids
    - tools/check/controls/important-gates-unknown-id-phase6/overlay/data/expected/important-gates.ids
    - .planning/STATE.md
    - .planning/phases/14-cartridge-route-and-real-content-conformance/14-VALIDATION.md
decisions:
  - the three flips and the five rows are one commit because the step refuses a row naming a Pending requirement and refuses a Complete requirement with no row
  - the GUARDRAILS 2.4 deviation plan 14-01 recorded closes over exactly the five mechanisms its table names; the two ids that owe no row are named rather than covered
  - the toy extension is 39 lines, not 36, because the third requirement record is a whole-file overlay whose derived set would otherwise open
  - the cost verdict is reprice, because the median moved while the token line was where it was predicted
  - the plan-time 14 s feedback-latency figure is stale by measurement and is amended to the measured 29.2 s cold / 37.6 s warm
metrics:
  duration: ~28 min
  completed: 2026-11-26
  tasks: 2
  files: 21
actuals:
  tokens: 10095
  tasks: 2
  commits: 2
plan_head_before: 40d501ed69e511b79de28e805ee82d8dee6763d3
plan_head_after: bee451127d9930199603a40eddf1bbab635c837c
requirements: [TOOL-07, SCN-03, CPU-06]
---

# Phase 14 Plan 04: the closing line, measured

TOOL-07, SCN-03 and CPU-06 become `Complete` in the same commit as the five registry rows that name
them, the toy tree gains its 39 lines and still closes, the registry baseline is re-derived from the
step's own printed stat (`gates=117 lines=357`), and the phase closes on one gate line measured under
`sh`, under `dash` (byte-identical) and inside the Ubuntu 24.04 container - with exactly one cost
verdict, `reprice`, recomputed by arithmetic from `.planning/ci-runs.tsv`, and a signed
`14-VALIDATION.md`.

## The closing gate line, quoted once

`sh tools/gate.sh` and `dash tools/gate.sh` both exit 0 and their `tail -1` lines are byte-identical
(`cmp` clean). The line, from `/tmp/g14sh.tail`:

```
gate: PASS steps=38/38 controls=402/402 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=100 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=689 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2745 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=63 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2745 history-commits=1 gates=117 lines=357 input-rows=31 input-policies=11 input-profile-fields=31 files=2745 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5014 tests-executed=5010 tests-skipped=4 tests-missing=0 docs=64 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

No `-- FAIL` line in either log. `gate-time total=` was **86, 105 and 147** across the three `sh` runs
this plan took on this host (the spread is host load, not tree size; 147 at the sign-off measurement,
which is the figure `14-VALIDATION.md` originally quoted - amended there to the observed range).

## The container run, and its four host tokens

One container gate (`sh tools/gate-linux.sh`, image tagged `ivs-gate-linux:14-04` for the run and the tag
removed afterwards; `docker run --rm` leaves no container, and `docker image ls | grep '^ivs-'` and
`docker ps -a | grep '^ivs-'` are both empty after the run). `build/gate-linux.log` was copied to
`/tmp/g14linux.log` immediately, before anything else ran (plan lesson 18: a second `gate-linux` run
rewrites the log and destroys the first run's evidence). Result: `gate-linux: x86_64, gcc (Ubuntu
13.3.0-6ubuntu2~24.04.1) 13.3.0, cmake version 3.28.3`, `gate: PASS`, `gate-time total=487` (amd64
emulated on an arm64 host), `gate-linux: exit 0`.

The two lines carry 105 tokens each. Token-by-token, exactly **four** differ, and all four are the
documented host differences:

| Token | `sh` (macOS, AppleClang) | Container (Ubuntu 24.04, GNU) | Why |
|---|---|---|---|
| `cc` | `cc=AppleClang` | `cc=GNU` | the compiler the `c-build` step identifies |
| `core-imports` | `core-imports=7` | `core-imports=5` | the container's gate runs without the macOS-only import surface |
| `archive` | `archive=members` | `archive=sha256` | `ar` vs `llvm-ar` listing shape for the installed archive |
| `refs-verified` | `refs-verified=3/4` | `refs-verified=0/4` | the three verified references are local git refs the tar stream does not carry |

Every other token is identical, including all of this phase's own figures.

## Token reconciliation: prediction versus measurement

Every token in the plan's prediction table, against the fresh `sh` line. Nothing was re-blessed: the two
cells that do not read as predicted are chased below the table.

| Token | Start (plan) | Predicted | Measured | Verdict |
|---|---|---|---|---|
| `steps` | 38/38 | 38/38 | 38/38 | ✓ invariant |
| `controls` | 402/402 | 402/402 | 402/402 | ✓ 401 + 1 (plan 14-01) |
| `faults` | 238/238 | 238/238 | 238/238 | ✓ invariant |
| `goldens` | 29 | 29 | 29 | ✓ invariant |
| `tests-expected` | 5014 | 5014 | 5014 | ✓ 4,990 + 18 (9 pack ids x 2) + 6 (3 runner-process ids x 2) |
| `tests-executed` | 5010 | 5010 | 5010 | ✓ 5,014 - 4 skipped |
| `tests-skipped` | 4 | 4 | 4 | ✓ no new `skip-ok` row |
| `tests-missing` | 0 | 0 | 0 | ✓ |
| `gates` | 114 | 117 | 117 | ✓ + `flatwrap`, `cart-role`, `cpu06` |
| `lines` | 352 | 357 | 357 | ✓ + 5 registry rows |
| `prefix-files` | 11 | 11 | 11 | ✓ the tool is not installed |
| `pkg-selftest` | 19/19 | 19/19 | 19/19 | ✓ no new selftest |
| `claims` / `provisional` | 28 / 27 | 28 / 27 | 28 / 27 | ✓ no claim |
| `discrepancies` / `open` | 110 / 101 | 110 / 101 | 110 / 101 | ✓ C41 and C82 amended, no id added |
| `vectors` | 412 | 412 | 412 | ✓ five vectors gained a `config` key, no id added |
| `cases` | 14 | 14 | 14 | ✓ no new `runner-selftest` case |
| `tracked` / `files` / `hygiene-files` | 2736 (plan) | 2745 | 2745 | ✓ predicted; the **Start** cell is stale, chased below |
| `rights` / `rights-cleared` | 10 / 1 | (not predicted) | 10 / 1 | ✓ unchanged |
| `mode` | `tree` | (not predicted) | `tree` | ✓ unchanged |

**The one stale cell, chased.** The plan's Start column reads 2,736 for `tracked`, and its own note says
that is the figure *before* this phase's four `PLAN.md` landed. Measured at this plan's starting commit
(`40d501e`), the gate printed **`tracked=2745 files=2745 hygiene-files=2745`** - already 2,745, not
2,736. The +9 is named: the six new source/control files plan 14-01 created (`tools/flatwrap/flatwrap.c`,
`tools/flatwrap/CMakeLists.txt`, `data/expected/flat-refusals.ids`, and the control's `CHECK`, `EXPECT`
and `overlay/tools/flatwrap/extra.c`) plus the three `SUMMARY.md` files plans 14-01, 14-02 and 14-03
committed. So the prediction's *closing* figure was right and its *starting* figure was stale; the
measured start equals the measured close because this plan creates no file. The metadata commit that
carries this file reads 2,746, the one added file being this `SUMMARY.md`.

No other token differs, and the invariants (`steps`, `faults`, `goldens`) are exactly where they were
frozen.

## The five registry rows, and the inventory each resolves in

Appended to `data/expected/important-gates.ids` under three new gate names, each row checked with
`grep -x -F` (or, for a fault, a first-column match) against its own inventory **before** the commit:

| Row | Kind | Resolves in | Proves |
|---|---|---|---|
| `flatwrap control c-build-unlisted-flatwrap-source TOOL-07` | control | `data/expected/controls/runner-hygiene.ids:21` | the tool is build-only and its sources are the build's; the control is shown to fail when a `tools/flatwrap/*.c` file is not listed |
| `flatwrap mutant pack.judge.flatwrap-digest-live TOOL-07` | mutant | `data/expected/tests/pack.ids:52` | the printed digest is the file's digest by an implementation other than the core's, and one flipped input byte moves it |
| `cart-role mutant runner-process.judge.flatwrap-roundtrip SCN-03` | mutant | `data/expected/tests/runner-process.ids:186` | `slot.cart.sha256` equals the printed digest, and one flipped input byte moves both |
| `cart-role mutant runner-process.judge.owner-run-cart-pass SCN-03` | mutant | `data/expected/tests/runner-process.ids:199` | the route's pass path is shown able to fail: the pinned pass report **and** exit 14 with `IVS_E_DIGEST_MISMATCH` under a one-byte EXEC mutation, so neither half passes alone |
| `cpu06 fault ACCEPT_CLEARS_INTFF_IGNORED CPU-06` | fault | `data/expected/faults/cpu.ids:54` (first column) | the field's effect is shown able to break; the fault's own `must-pass` is `cpu.UNIT-INT.accept-keeps-intff`, so the not-cleared case is the half that must keep passing |

`runner-process.judge.owner-run-cart-pass` is judge-shaped deliberately: `important-gates.sh:151`-style
resolution accepts a `mutant` row only through `^[a-z0-9-]+[.](judge|mutant)[.][^ ]+$`, so a case id
without the `judge` segment would have made this row `important-gates-unknown-id`. Plan 14-03 named it
accordingly.

## The GUARDRAILS 2.4 deviation, stated and closed

**The rule.** §2.4 requires each new check's must-fail mechanism row in `data/expected/important-gates.ids`
to ship in the check's own commit.

**Why it could not.** `important-gates.sh` refuses a row naming a `Pending` requirement
(`important-gates-unknown-id`) and refuses a `Complete` requirement with no row
(`important-gates-complete-uncovered`). TOOL-07, SCN-03 and CPU-06 all stay `Pending` until this plan's
flip, so neither half could ship earlier: a row in plan 14-01's commit would have been the first token, a
flip there would have been the second.

**How it closed.** The three flips and the five rows land in **one commit** (`548e80d`), over exactly the
five mechanisms plan 14-01's deviation table names - no more and no fewer. `14-01-SUMMARY.md` and
`14-03-SUMMARY.md` each recorded the deferral; this commit discharges it.

**The two ids that owe no row, named so a reader does not have to count.** `pack.judge.flatwrap-tokens`
is a coverage diff - its failure is a mismatch between the pinned token file and the literals the source
can print, which needs no scratch mutation, so it has no stand-in limb to register.
`runner-process.cart-user.refuse-derived` is a refusal asserted by exit status plus one typed line, not
an equivalence with a stand-in. A row for either would register a mechanism that does not exist, and the
deviation record would then be smaller than the registry. No sixth row was invented to "cover" them.

**Zero new faults holds.** The one `fault` row above is a fault that already exists, registered because
CPU-06's flip owes its coverage - not a new fault. `faults=238/238` before and after.

## The toy extension: 13 files, 39 added lines, zero deletions

`git show --numstat 548e80d` over the thirteen toy paths, summed:

```
files=13 added=39 deleted=0
```

Per file the shape is uniform, `3 0`: ten whole-file toy registries x 3 lines
(`toy-phase14 fault GOOD_FAULT {TOOL-97,SCN-97,CPU-97}`) = **30**, plus three toy requirement records x 3
rows (`| TOOL-97 | Phase 14 | Complete |` and its two siblings) = **9**. 30 + 9 = **39**. No `DEVIATION`
file was added: the addition is intended in all ten overlays, and a `DEVIATION` would have tripped
`control-deviation-unused` on the pristine tree. `TOOL-97`, `SCN-97` and `CPU-97` were checked unused in
the toy tree with `grep -rn -- '-97'` before the append (no match). The third requirement record is not
optional: the toy tree's derived set names three accepted requirements, so a record without its row is
`important-gates-unknown-id` in the toy tree, and dropping it to reach a tidier 36 would open the
`important-gates-complete-uncovered` overlay's own derived set.

## The registry baseline, re-derived by running the step

| Tree | Before | After | Arithmetic |
|---|---|---|---|
| real registry | `gates=114 lines=357-5=352` | **`gates=117 lines=357`** | 114 + 3 new gate names; 352 + 5 rows |
| toy tree | `gates=45 lines=46` | **`gates=46 lines=49`** | 45 + 1 gate name (`toy-phase14`); 46 + 3 rows |

Both figures are the step's own printed `pass` line, produced by running it against the tree - never
quoted from a document. `awk '!/^#/ && NF' data/expected/important-gates.ids | wc -l` reads 357
independently. `sh tools/check/run-controls.sh` prints `controls=402/402` with zero `control-toy-drift`,
`control-deviation-unused`, `control-good-failed`, `control-token-missing` or `control-exit-zero` lines.
The phase table in `tools/check/steps/important-gates.sh` gained its Phase 14 block in the same commit,
in the same two-line-continuation shape, naming the three requirements, the three gate names, the five
rows with their kinds, and "zero new faults".

## The cost checkpoint: one verdict, `reprice`, with its arithmetic

Recomputed from `.planning/ci-runs.tsv` (the last five data rows, by run id), never quoted:

| run id | `cpu_s` | `build_s` | `gate_s` |
|---|---|---|---|
| 45683886915 | 357 | 33 | 121 |
| 45717868904 | 360 | 34 | 120 |
| 45718553735 | 363 | 35 | 119 |
| 45720934719 | 438 | 39 | 135 |
| 45723269901 | 368 | 35 | 122 |

- `cpu_s` sorted: **357 360 363 368 438** -> median **363**
- `build_s` sorted: **33 34 35 35 39** -> median **35**
- `gate_s` sorted: **119 120 121 122 135** -> median **121**

The frozen figures 361 / 34 / 118 are **stale by measurement**, by **+2 / +1 / +3**.

The priced addition, with its arithmetic:

| Item | Unit | Count | cpu-s |
|---|---|---|---|
| New test ids (9 `pack` + 3 `runner-process`) | 0.10 | 12 | 1.200 |
| New control directories (`c-build-unlisted-flatwrap-source`) | 0.31 | 1 | 0.310 |
| `important-gates.ids` rows | 0.005 | 5 | 0.025 |
| New faults - forbidden | - | 0 | 0.000 |
| **Total** | | | **1.535** |

- **`discharged-within-allowance`** requires median `cpu_s` <= 361 + 1.535 = **362.535**. Measured 363
  is **0.465 above** it, so the verdict is not reachable, whatever the token line did.
- **`lever-required`** requires cumulative milestone spend past 21 (1.935 from Phase 13 + 1.535 =
  **3.470**, far under) or this phase overrunning its line by more than 25 % (1.535 against 1.3 is
  **+18.1 %**, under the 25 % line). Neither holds, so no lever is invented for a runner event.
- **`reprice`** is therefore what the rule names: the median moved more than the priced addition while
  the token line is exactly where it was predicted.

The priced count landed exactly: `controls` 401 -> 402 (one new directory), `gates` 114 -> 117 and
`lines` 352 -> 357 (five rows, three names), `tests-expected` +24 over the phase (18 + 6),
`steps=38`, `faults=238` and `goldens=29` unmoved.

## The hosted row

`hosted row: not-recorded - no push run for 548e80d at 2026-11-26; the orchestrator records it from sh
tools/ci-record.sh <run-id> after the push`. `gh run list --commit 548e80d` returns no row, because the
plan does not push. `.planning/ci-runs.tsv` is untouched by this plan - no row was fabricated. The last
recorded row remains `45723269901` (`push`, `151b688`, the Phase 13 merge), which is the fifth value in
the checkpoint window above.

## The `14-VALIDATION.md` sign-off

Amended, not rewritten (GUARDRAILS 2.10):

- Frontmatter: `status: validated`, `nyquist_compliant: true`, `wave_0_complete: true`; `created` and
  `phase`/`slug` unchanged.
- Test Infrastructure: the framework line as planned; the quick run
  `ctest --preset dev -R '^(pack|runner-process|state|run|stic|cpu-vectors)$'`; the full suite
  `sh tools/gate.sh` (`sh tools/gate-linux.sh` for the Ubuntu substance); runtime the measured
  `gate-time total=` 86-147 on this host and 487 in the container.
- Sampling Rate: the quick run after every task commit, the full gate after every wave, the measured
  max feedback latency 37.6 s (29.2 s cold), and a note that the closing task ran the full gate after
  its own commit - stronger than the quick run.
- Per-Task Verification Map: **nine** rows (14-01-T1/T2, 14-02-T1/T2, 14-03-T1/T2/T3, 14-04-T1/T2), each
  carrying its plan, wave, requirement, its threat refs, its test type, the automated command its own
  plan states, `File Exists` and `✅ green`.
- Wave 0 Requirements: all ten bullets ticked, each amended to name the file that satisfies it, and the
  last one recording the measured `steps=38/38 faults=238/238 goldens=29 controls=402/402` - no new
  framework, no new suite, no new gate step.
- Manual-Only Verifications: `All phase behaviors have automated verification.` - the phase's ceiling is
  no claim, nothing is handed off, and no row carries `evidence: none`.
- Validation Sign-Off: all six boxes ticked and measured; `Approval:` carries the full measured gate line
  and the date.

## Deviations from Plan

### Plan defects found by measurement (recorded, not faked)

**1. The plan's "14 s" feedback latency is stale by measurement; the quick run is 29-38 s.**
The plan instructed the sign-off to record "the measured quick-run wall time, which is under the limit"
(14 s). Measured with `/usr/bin/time -p`: the quick run is **29.21 s** cold and **37.61 s** warm, and the
`runner-process` suite alone is **29.50 s** of it (`ctest --preset dev -R '^(pack|runner-process|state|run|stic|cpu-vectors)$'`).
The 14 s figure is therefore not the measured value and was not ticked as written: the Sampling Rate
section and the `Feedback latency` sign-off box both carry the measured 37.6 s bound with the plan-time
14 s named as stale. The box is ticked because the *measured* bound is what the phase sampled at, and
the full gate (86-147 s) is the outer bound; it is not ticked against 14 s.

**2. The plan's Start column for `tracked`/`files`/`hygiene-files` is stale (2,736 against a measured
2,745).** Chased to named commits in the reconciliation table above; the closing prediction (2,745) was
right, and the metadata commit reads 2,746 because this `SUMMARY.md` is one new file.

**3. The full gate's wall time varies by 70 % run to run on this host (86, 105 and 147 s), so any single
`gate-time` figure quoted as *the* runtime is a sample, not a property.** `14-VALIDATION.md` originally
carried the 147 s sign-off sample; it now carries the observed range. The container's 487 s is not
comparable (amd64 emulated on an arm64 host).

No other figure differed: `gates=117`, `lines=357`, the toy tree's `gates=46 lines=49`, `controls=402/402`,
the thirteen-file/39-line toy diff and the four container host tokens all matched the prediction exactly.

### Auto-fixed issues

None. No source file, gate step, fault, golden, control or registry row moved outside the plan's own
scope, and no fix attempt was needed.

## §2.4 mechanisms and guards observed in this plan

- `c-build-unlisted-flatwrap-source` (control) - `control ok` in the closing run, `controls=402/402`; the
  plan's own guard mutation (deleting the line from `data/expected/controls/runner-hygiene.ids`) makes
  `run-controls.sh` refuse, which is the property the `-c` limb of the verify is written to catch.
- `important-gates` in both directions - `important-gates-unknown-id` (a row naming a `Pending`
  requirement, or an id in no inventory) and `important-gates-complete-uncovered` (a `Complete`
  requirement with no row) are the pair that forced the flips and the rows into one commit; both are
  exercised by the committed controls `important-gates-unknown-id` and
  `important-gates-complete-uncovered` in the same `controls=402/402` run.
- `control-toy-drift` / `control-deviation-unused` - the toy registry's two-direction drift check ran
  green over the ten extended overlays, which is what makes "no `DEVIATION` entry" a measured fact
  rather than a claim.

## Threat surface scan

No new network endpoint, auth path, file access pattern or schema at a trust boundary. This plan edits
records, one registry, thirteen toy files and one validation document; it adds no code, no dependency and
no installed file (`prefix-files=11`, `binaries-listed=0` unchanged). The one new *evidence* surface is
`/tmp/g14linux.log`, a copy outside the tree, deliberately not committed.

## Threat register dispositions

| Threat ID | Disposition | Where it is mitigated and measured |
|---|---|---|
| T-14-17 | mitigate | the three flips and the five rows are one commit; the measured `gates=117 lines=357` pass line is the both-directions proof |
| T-14-18 | mitigate | the five `cpu_s`/`build_s`/`gate_s` values and both medians are printed beside the verdict, recomputed from the committed TSV; the frozen figures are named stale by measurement; the verdict is one rule word with its arithmetic |
| T-14-19 | mitigate | every predicted token is compared against the fresh line; the one stale cell is chased to named commits; the invariants are stated to be where they were frozen |
| T-14-20 | mitigate | one container gate, log copied before anything else ran, image tagged `ivs-gate-linux:14-04` and the tag removed, `--rm` container, `ivs-*` images and containers both empty afterwards |
| T-14-21 | mitigate | three appended lines per file across all ten registries and three rows per toy requirement record, zero deletions, no `DEVIATION`; `control-toy-drift` and `control-deviation-unused` both green |
| T-14-SC | mitigate | no package installed; the plan edits records and one registry |

## Known Stubs

None. Every figure in this plan's records is the output of a command run in this session - the step's
own `pass` line, `cmp` on two gate tails, `git show --numstat`, and arithmetic over the committed TSV.
There is no placeholder, no unwired value and no skipped check.

## Deferred Issues

- The hosted row is `not-recorded` by design (no push in this plan); the orchestrator records it from
  `sh tools/ci-record.sh <run-id>` after the push. Named in `.planning/STATE.md` and here rather than
  fabricated.
- The plan-time 14 s feedback-latency figure is stale; the measured bound (37.6 s) is recorded. If the
  quick run is to come back under ~15 s, the lever is `runner-process` (29.5 s of the 37.6 s), which is
  a cost question for the milestone close, not a defect in this plan.
- `14-02-SUMMARY.md`'s recorded finding still stands: `data/expected/important-gates.ids:337` names
  `runner-process.mutant.SCN-STUB-JD-AS-J` as one of SCN-01's must-fail mechanisms, and after plan
  14-02 that row pins the *absence* of a failure. SCN-01 keeps eleven other failing mutant rows, so its
  coverage does not depend on it; the line should be dropped or repointed by a later plan. Not touched
  here because this plan's registry change is scoped to the five Phase 14 rows.

## Self-Check

- `.planning/REQUIREMENTS.md` - FOUND (three `- [x]` boxes, three `Complete` traceability rows)
- `data/expected/important-gates.ids` - FOUND (357 non-comment lines; the five Phase 14 rows present)
- `tools/check/steps/important-gates.sh` - FOUND (the Phase 14 block in the phase table)
- the thirteen toy files - FOUND (39 added lines, zero deletions, `gates=46 lines=49`)
- `.planning/STATE.md` - FOUND (Phase 14 complete; exactly one `- [Phase 14]: 14-04: checkpoint: ` bullet)
- `14-VALIDATION.md` - FOUND (`status: validated`, nine per-task rows, ten ticked Wave 0 bullets)
- commit `548e80d` - FOUND (ancestor of HEAD)
- commit `bee4511` - FOUND (ancestor of HEAD)
- `/tmp/g14sh.tail` and `/tmp/g14dash.tail` - byte-identical (`cmp` exit 0)
- `/tmp/g14linux.log` - FOUND (`gate: PASS`, `gate-linux: exit 0`)

**Self-Check: PASSED**

---

## Amendment, 2026-11-26 (the phase's code review round)

Appended, never rewritten: every figure above was measured at this plan's own commit and stays as it was
measured there. The review (`14-REVIEW.md`) closed two of the items this summary recorded as open, and
two of them move a token in the line quoted above.

1. **The deferred `important-gates` row (WR-03).** The item recorded above under "recorded finding still
   stands" is fixed at `fc25ce9`: `scn-mutants mutant runner-process.mutant.SCN-STUB-JD-AS-J SCN-01` is
   deleted, not repointed, because every one of SCN-01's eleven still-failing mutant rows is already
   registered and the project's own practice is that a `pass 0` row is not a registry line (the other
   three are deliberately absent). The case is not dropped: it stays a live inventory row whose stated
   outcome the `scn-mutants` case judges. The step now prints `-- important-gates pass gates=117
   lines=356`: `lines` moves `357` -> `356`, and `gates` does not move because it counts distinct gate
   names and `scn-mutants` keeps eleven rows. This summary's prediction of `gates=117 lines=357` and its
   `357 non-comment lines` self-check line were correct at `548e80d`; the review's own Fix text predicted
   `116`/`356` and was wrong about `gates`.
2. **The other two tokens that differ at the fix head**, neither caused by this plan's own edits:
   `tests-expected=5014 tests-executed=5010` -> `5016`/`5012` is CR-01's `pack.flatwrap.neg.flatwrap-empty`
   (one id, counted once per preset, added at `bf3215d` after this line was measured), and
   `tracked=2745 files=2745 hygiene-files=2745` -> `2747` is the two record files written after that
   measurement (this summary in `2e27267`, `14-REVIEW.md`). `steps=38/38`, `faults=238`, `goldens=29` and
   `controls=402/402` are unchanged, so no gate step, fault, golden or control was added or dropped. The
   line at the fix head is quoted in `14-REVIEW.md`'s Dispositions section.
3. **The plan's predicted token is not re-blessed.** `14-04-PLAN.md`'s `<fails_when>` names
   `-- important-gates pass gates=117 lines=357`; that was true at this plan's commit and the plan is a
   record of an execution, not a re-runnable oracle. A reader re-running it gets a red on `lines` and this
   amendment is the reason.
