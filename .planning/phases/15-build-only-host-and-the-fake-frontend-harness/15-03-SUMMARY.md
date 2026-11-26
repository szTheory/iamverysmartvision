---
phase: 15-build-only-host-and-the-fake-frontend-harness
plan: 03
subsystem: records (the requirement flip, the important-gate registry, the toy tree, the one claim, the cost checkpoint and the validation sign-off)
tags: [records, gate-registry, toy-tree, claim-ceiling, cost-checkpoint, container, validation]
status: complete
dependency-graph:
  requires:
    - .planning/REQUIREMENTS.md (the traceability table is the parsed accepted set)
    - data/expected/important-gates.ids and data/expected/tests/runner-process.ids (a `mutant` row resolves only through a `judge`/`mutant` id)
    - data/claims.json, data/expected/claims.ids, CHANGELOG.md, docs/evidence-tiers.md (GUARDRAILS V3's retarget set)
    - .planning/ci-runs.tsv (the arithmetic input for the cost checkpoint)
    - plans 15-01 and 15-02 (the eight green `runner-process` ids the claim rests on)
  provides:
    - TOOL-08 `Complete` with two live, shown-able-to-fail `judge` rows in one commit
    - the re-derived registry baseline (`gates=118 lines=358`; toy tree `gates=47 lines=50`)
    - one claim, `CL-0029`, at `deterministic-repeatability`, standing `provisional`
    - the closing gate line under `sh`, `dash` and the Ubuntu 24.04 container
    - one cost verdict, `reprice`, with its arithmetic
    - a signed 15-VALIDATION.md
  affects:
    - .planning/ROADMAP.md Phase 15 (3/3 plans executed, phase complete)
    - the milestone close (the phase's cost row and its hosted row)
tech-stack:
  added: []   # nothing installed: this plan edits records, one registry, thirteen toy files, one ledger, one generated table and two record documents
  patterns:
    - a requirement flip and the registry rows that name it land in one commit, because the step refuses both directions
    - a `mutant` row names a `judge`-shaped id, so the row points at a mechanism that exists and has been shown to fail
    - the registry baseline is the step's own printed stat, re-derived by running it against the tree
    - the cost verdict is arithmetic over the committed TSV, with the five values and all three medians printed beside it
key-files:
  created:
    - .planning/phases/15-build-only-host-and-the-fake-frontend-harness/15-03-SUMMARY.md
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
    - data/claims.json
    - data/expected/claims.ids
    - CHANGELOG.md
    - docs/evidence-tiers.md
    - data/expected/host-imports.ids
    - tools/pcm/CMakeLists.txt
    - tools/check/steps/hygiene-history.sh
    - .planning/STATE.md
    - .planning/ROADMAP.md
    - .planning/phases/15-build-only-host-and-the-fake-frontend-harness/15-VALIDATION.md
decisions:
  - the flip and its two rows are one commit because `important-gates-unknown-id` and `-complete-uncovered` fire in both directions
  - both rows name `runner-process.judge.*` ids, not the plain `host.*` ones, because a `mutant` row resolves only through `^[a-z0-9-]+[.](judge|mutant)[.][^ ]+$`
  - five host ids deliberately owe no row; a row for a mechanism that does not exist would make the registry larger than the evidence
  - the toy extension is 13 lines (10 registries + 3 requirement records), not phase 14's 39, because this phase accepts one requirement
  - `tools/check/kinds/claim.cmake` did not move: `CL-0029` exercises the hook's existing rules and a new rule there would be a new mechanism needing a stand-in (§2.4)
  - the cost verdict is `reprice`, because the median moved by more than the priced addition while the token line is exactly where it was predicted
  - the plan's rounded 1.54 is recorded beside the exact 1.535; the verdict is unaffected either way
metrics:
  duration: ~48 min (two executor sessions, separated by a dispatch timeout)
  completed: 2026-11-26
  tasks: 3
  files: 27
actuals:
  tokens: 2684     # chars/4 over the realized diff, 94e39a2..85470b8 (10,737 changed characters)
  tasks: 3
  commits: 3       # MEASURED: git rev-list --count 94e39a2..85470b8
plan_head_before: 94e39a20c369fcbe94410252c5ac9c0be8eb7f7e
plan_head_after: 85470b80771da9157b38496a19b3d81302a130d5
requirements: [TOOL-08]
---

# Phase 15 Plan 03: the closing line, measured, under two shells and in the container

TOOL-08 flipped with the two live `judge` rows that name it and thirteen toy lines in the same commit, one
`deterministic-repeatability` claim minted at `provisional` with GUARDRAILS V3's whole retarget set, and the
phase closed on one line measured under `sh`, `dash` and Ubuntu 24.04 with exactly one cost verdict, `reprice`.

## How this plan was executed, stated plainly

Tasks 1 and 2 and the container leg ran in an earlier executor session that ended when the dispatch timed
out (a 30-minute dispatch timeout). Their work is committed and is not re-done here: `d712f02` (the TOOL-08
flip, the two rows, the toy tree), `f9b7864` (`CL-0029` with V3's retarget set) and `85470b8` (the two
portability defects the container leg found). **This session did task 3 only**: the two-shell gate evidence,
the token reconciliation, the cost checkpoint, the `15-VALIDATION.md` sign-off, the `STATE.md`/`ROADMAP.md`
records and this file. The container leg was **not** re-run — its result was already measured and committed.

## The closing gate line, quoted once

Measured fresh at `85470b8` (the tree this plan closes on), `sh tools/gate.sh` exit 0, `gate-time total=74`,
`gate-cpu user=115.23 sys=104.77`, zero `^-- FAIL` lines, `-- hygiene pass mode=tree hygiene-files=2771`:

```
gate: PASS steps=38/38 controls=404/404 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=102 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=29 provisional=28 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=695 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2771 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=63 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2771 history-commits=1 gates=118 lines=358 input-rows=31 input-policies=11 input-profile-fields=31 files=2771 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=20/20 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=12/12 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5036 tests-executed=5032 tests-skipped=4 tests-missing=0 docs=64 claim-refs=27 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

## Two shells, byte-identical

| Shell | Exit | `gate-time total` | `tail -1` bytes |
|---|---|---|---|
| `sh tools/gate.sh` | 0 | 74 | 1509 |
| `dash tools/gate.sh` | 0 | 74 | 1509 |

`cmp /tmp/g15sh.tail /tmp/g15dash.tail` → silent: **byte-identical**, not "equivalent". Both logs carry zero
`^-- FAIL` lines. The quick run the sampling rate asks for, measured twice for the latency figure:
`IVS_TEST_OUT_DIR=<dir> ctest --preset dev -R '^runner-process$'` → **19.01 s** and **19.76 s** wall,
trailer `# suite=runner-process executed=144 skipped=2 failed=0` (the same trailer `CL-0029`'s note cites).

## The container run, and its four host tokens

Already measured in the earlier session and **not re-run** (plan lesson 18: a second `gate-linux` run
rewrites `build/gate-linux.log` and destroys the first run's evidence). One container gate,
`sh tools/gate-linux.sh`, against the tree carrying both portability fixes: `gate-linux: x86_64, gcc
(Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0, cmake version 3.28.3`, `gate: PASS`, `gate-time total=279`,
`gate-linux: exit 0`. `docker image ls` and `docker ps -a` filtered on `^ivs-` are both empty afterwards, and
`build/gate-linux.log` is left as the tool wrote it.

**Evidence-copy note, recorded rather than glossed.** The plan asks for the container log at
`/tmp/g15linux.log`, copied *before anything else ran*. The earlier session left that path holding the
**pre-fix** run (01:48, `gate: FAIL steps=37/38` — the run that found the two defects). The passing run's log
is `build/gate-linux.log` (01:58:18, eight seconds before `85470b8` was made, i.e. against the fixed working
tree). At this close it was copied to `/tmp/g15linux.log` and the pre-fix log preserved at
`/tmp/g15linux-prefix-fail.log`, so both runs remain readable and neither is mistaken for the other. Nothing
was re-run to produce this.

The two lines carry 105 tokens each. Token-by-token, exactly **four** differ, and all four are the documented
host differences:

| Token | `sh` (macOS, AppleClang) | Container (Ubuntu 24.04, GNU) | Why |
|---|---|---|---|
| `cc` | `cc=AppleClang` | `cc=GNU` | the compiler the `c-build` step identifies |
| `core-imports` | `core-imports=7` | `core-imports=5` | the container's gate runs without the macOS-only import surface |
| `archive` | `archive=members` | `archive=sha256` | `ar` vs `llvm-ar` listing shape for the installed archive |
| `refs-verified` | `refs-verified=3/4` | `refs-verified=0/4` | the three verified references are local git refs the tar stream does not carry |

Every other token is identical, this phase's own included: `gates=118 lines=358`, `claims=29 provisional=28
claim-refs=27`, `public-symbols=19 defined=141`, `prefix-files=11`, `pkg-selftest=20/20
release-selftest=12/12`, `tests-expected=5036 tests-executed=5032`. This was the first container leg to
compile the host at all, which is why the four tokens were expected to move and no other was.

## Token reconciliation: prediction versus measurement

Every token in the plan's context table, against the fresh `sh` line. **Nothing was re-blessed, and nothing
needed to be chased: all seventeen predicted tokens measured as predicted.**

| Token | Start (plan) | Predicted at T3 | Measured | Verdict |
|---|---|---|---|---|
| `steps` | 38/38 | 38/38 | 38/38 | ✓ invariant, no new step |
| `controls` | 404/404 | 404/404 | 404/404 | ✓ unchanged this plan |
| `faults` | 238/238 | 238/238 | 238/238 | ✓ invariant, no new fault |
| `goldens` | 29 | 29 | 29 | ✓ invariant, no new golden |
| `tests-expected` | 5036 | 5036 | 5036 | ✓ no new id in this plan |
| `tests-executed` | 5032 | 5032 | 5032 | ✓ 5,036 − 4 skipped |
| `tests-skipped` | 4 | 4 | 4 | ✓ no new `skip-ok` row |
| `tests-missing` | 0 | 0 | 0 | ✓ |
| `gates` | 117 | 118 | 118 | ✓ +1 distinct gate name (`host`) |
| `lines` | 356 | 358 | 358 | ✓ +2 registry rows |
| `claims` / `provisional` | 28 / 27 | 29 / 28 | 29 / 28 | ✓ `CL-0029` at `provisional` |
| `claim-refs` | 26 | 27 | 27 | ✓ one `[claim:CL-0029]` tag on one `docs/` line |
| `prefix-files` | 11 | 11 | 11 | ✓ the object is never installed |
| `pkg-selftest` / `release-selftest` | 20/20, 12/12 | 20/20, 12/12 | 20/20, 12/12 | ✓ records only |
| `binaries-listed` | 0 | 0 | 0 | ✓ no tracked shared object |
| `tracked` / `files` / `hygiene-files` | 2,771 | 2,771 | 2,771 | ✓ no file created before this SUMMARY |

The invariants are exactly where they were frozen: **no new gate step, fault or golden** (`steps=38`,
`faults=238`, `goldens=29`). The metadata commit that carries this file reads 2,772 — one added file, this
`SUMMARY.md`, which is the plan's own prediction.

**How `tracked` is counted, stated so the two figures are not read as a contradiction.** `tools/gate.sh`'s
header says its `FILELIST` is "tracked plus untracked-not-ignored paths", so `tracked`/`files` count the
working tree, not `git ls-files`. The line quoted above (`tracked=2771 files=2771`) was measured at `85470b8`
with a clean tree and **no** untracked file, i.e. 2,771 tracked files. The moment this `SUMMARY.md` exists as
an untracked file the same line reads `tracked=2772 files=2772` with 2,771 tracked files; after the metadata
commit it reads 2,772 with 2,772 tracked files. Measured both ways at the close, in that order.

## The two registry rows, and the inventory each resolves in

| Row | Kind | Resolves through | Refused for | Proves |
|---|---|---|---|---|
| `host mutant runner-process.judge.host-vacuous TOOL-08` | `mutant` | `data/expected/tests/runner-process.ids` → `runner-process.judge.host-vacuous` | `frames-zero` | the "frames>0" limb is load-bearing: the digest equality cannot pass on a frontend that rendered nothing |
| `host mutant runner-process.judge.host-digest-diff TOOL-08` | `mutant` | `data/expected/tests/runner-process.ids` → `runner-process.judge.host-digest-diff` | `digest-diff` | the equality is proved live against a second, measured input state (the empty-chain `digest.input`), not against a constant |

Both ids were checked with `grep -x -F` against `data/expected/tests/runner-process.ids` before the commit;
both are `judge`-shaped because `important-gates.sh` resolves a `mutant` row only through
`^[a-z0-9-]+[.](judge|mutant)[.][^ ]+$`, so a plain `runner-process.host.*` id would have made the row
`important-gates-unknown-id`. **Five host ids deliberately owe no row** — `host.exports`, `host.api`,
`host.frames`, `host.unload`, `host.empty-poll` — because their must-fail limbs are the missing-name
comparison, the pinned values and the ordering, which are asserted *inside* the cases and have no separate
stand-in to point at. A row for a mechanism that does not exist would make the registry larger than the
evidence (§2.4's own rule, as plan 14-04 applied it to two ids).

Re-derived by running the step, never quoted: `-- important-gates pass gates=118 lines=358` for the real
registry (117 + 1 gate name; 356 + 2 rows) and `-- important-gates pass gates=47 lines=50` for
`tools/check/controls/_good/tree` (46 + 1; 49 + 1). `sh tools/check/run-controls.sh` → `controls=404/404`,
with no `control-toy-drift`, `control-deviation-unused`, `control-good-failed`, `control-token-missing`,
`control-exit-zero`, `control-not-in-inventory` or `control-inventory-orphan` line. The phase table in
`tools/check/steps/important-gates.sh` gained its Phase 15 block in the same commit, in the same
two-line-continuation shape.

## The toy diff, measured

`git show --numstat d712f02`, over the thirteen toy paths:

```
1  0  tools/check/controls/_good/tree/.planning/REQUIREMENTS.md
1  0  tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md
1  0  tools/check/controls/_good/tree/data/expected/important-gates.ids
1  0  tools/check/controls/important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids
1  0  tools/check/controls/important-gates-bad-req-phase6/overlay/data/expected/important-gates.ids
1  0  tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md
1  0  tools/check/controls/important-gates-req-uncovered-phase5/overlay/data/expected/important-gates.ids
1  0  tools/check/controls/important-gates-req-uncovered-phase6/overlay/data/expected/important-gates.ids
1  0  tools/check/controls/important-gates-req-uncovered-phase7/overlay/data/expected/important-gates.ids
1  0  tools/check/controls/important-gates-uncovered-gate/overlay/data/expected/important-gates.ids
1  0  tools/check/controls/important-gates-unknown-id/overlay/data/expected/important-gates.ids
1  0  tools/check/controls/important-gates-unknown-id-phase5/overlay/data/expected/important-gates.ids
1  0  tools/check/controls/important-gates-unknown-id-phase6/overlay/data/expected/important-gates.ids
```

**13 files, 13 added lines, 0 deletions** — 10 whole-file registries × `toy-phase15 fault GOOD_FAULT
TOOL-96` + 3 requirement records × `| TOOL-96 | Phase 15 | Complete |`. The arithmetic is 13, not phase 14's
39, because this phase accepts one requirement. No `DEVIATION` file was added: the addition is intended in
all ten, and a `DEVIATION` would have tripped `control-deviation-unused`. `TOOL-96` was checked free with
`grep -rn -- '-96'` over the good tree and the `important-gates-complete-uncovered` overlay before the
append (no match; `EV-98`, `TOOL-97`, `SCN-97` and `CPU-97` are taken by Phases 13 and 14).

## `CL-0029` and GUARDRAILS V3's whole retarget set

Minted **only after** the eight host ids measured green: `IVS_TEST_OUT_DIR=<dir> ctest --preset dev -R
'^runner-process$'` → trailer `# suite=runner-process executed=144 skipped=2 failed=0`, zero `not ok`.

| Field | Value |
|---|---|
| id | `CL-0029` |
| tier | `deterministic-repeatability` (the phase's frozen ceiling, D-50) |
| standing | `provisional` |
| profile | `intv-2609-ntsc-family-p1` |
| artifact | `tests/host/ivs_host_harness.c` (tracked, non-handoff: not `build/`, not `.wav`/`.ppm`/`.tsv`) |
| depends_on | `crystal_hz`, `ticks_per_cpu_cycle`, `cpu_cycles_per_scanline`, `scanlines_per_frame` — four members of the profile, the fields that fix `nominal_frame_ticks` and therefore the drive |

The statement names the profile, the project-authored subset, the project's own `lamp-tone.ivsc` with the
zero graphics ROM and no EXEC image, the six scripted events of `data/scenarios/demo-lamp-tone-pass.ivss`,
the `dlopen` load of the build-only object, and the nine `digest.<domain>` lines equal to
`data/golden/demo-lamp-tone-pass.digests` — and ends with the tier's own limit. The `note` states the limits
plainly: the shim is build-only and never installed, so `installed-host-behavior` does **not** apply (D-48,
GUARDRAILS §3's stricter reading); the evidence is eight `runner-process` ids, two of them stand-ins that
prove the judge can fail; the digest values are the core's own producer's output on both sides, not an
independent oracle; D10 and D11 are unperformed, so nothing is measured on hardware.

Four files moved with it, in one commit (`f9b7864`), which is V3's set:

1. `data/claims.json` — the ledger row.
2. `data/expected/claims.ids` — `CL-0029`; a row without a line is `claims-id-missing` and a line without a
   row is `claims-id-extra`, so the two are one commit by construction.
3. `docs/evidence-tiers.md` — one sentence in the `deterministic-repeatability` section carrying exactly one
   `[claim:CL-0029]` tag; without the tag the `text` step refuses the line with `text-claim-untagged`. No new
   tier row: a new tier would be a claim-ceiling change this phase may not make.
4. `CHANGELOG.md` — the table between the `claims-table-begin`/`claims-table-end` markers regenerated by
   `cmake -DROOT=<repo> -P tools/check/claims-table.cmake` and pasted verbatim; `cmp` against a fresh render
   is silent, and the trailing summary line moved 28/27 → 29/28. A hand-typed row would be
   `release-changelog-claims-drift`.

**`tools/check/kinds/claim.cmake` did not move, and this is the recorded reason rather than an omission.**
V3 lists it as a retarget target. Read against `CL-0029`, the hook is data-driven — order, the artifact path
rule, the handoff-artifact rule, profile/field membership, D-50's two conditions and the weakest-field rule —
and the new row exercises those existing rules: it is a new row, not a new rule. The `claims` step is green
with the row and the file untouched. Adding a rule to satisfy a retarget row would be a **new mechanism**,
which §2.4 would then require a must-fail mechanism and a committed stand-in for; inventing one is worse than
recording that the checker needed no edit. The file stayed in the plan's `files_modified` precisely so this
paragraph exists.

## The cost checkpoint: one verdict, `reprice`, with its arithmetic

Recomputed from `.planning/ci-runs.tsv` (the last five data rows, by run id), never quoted:

| run id | `cpu_s` | `build_s` | `gate_s` |
|---|---|---|---|
| 45718553735 | 363 | 35 | 119 |
| 45720934719 | 438 | 39 | 135 |
| 45723269901 | 368 | 35 | 122 |
| 45747709457 | 366 | 34 | 115 |
| 45751979661 | 359 | 34 | 120 |

- `cpu_s` sorted: **359 363 366 368 438** → median **366**
- `build_s` sorted: **34 34 35 35 39** → median **35**
- `gate_s` sorted: **115 119 120 122 135** → median **120**

The frozen figures 361 / 34 / 118 are **stale by measurement**, by **+5 / +1 / +2**.

The priced addition, with its arithmetic:

| Item | Unit | Count | cpu-s |
|---|---|---|---|
| `important-gates.ids` rows (2 `mutant` rows) | 0.005 | 2 | 0.010 |
| Claim ledger row (`data/claims.json` + `claims.ids`) | 0.005 | 1 | 0.005 |
| New test ids, controls, faults, goldens, steps | — | 0 | 0.000 |
| **Total** | | | **0.015** |

With plan 15-01's 0.20 and plan 15-02's 1.32 the phase's priced addition is **1.535** (the plan's table
rounds it to 1.54; the exact sum is 0.20 + 1.32 + 0.015 = 1.535 and that is what is used below).

- **`discharged-within-allowance`** requires median `cpu_s` ≤ 361 + 1.535 = **362.535**. Measured **366** is
  **3.465 above** it, so the verdict is not reachable, whatever the token line did.
- **`lever-required`** requires cumulative milestone spend past 21 — **1.935** (Phase 13) + **1.535**
  (Phase 14) + **1.535** (this phase) = **5.005** of 21, far under — or this phase overrunning its line by
  more than 25 %: 1.535 against the plan's own 1.5 comparison line is **+2.33 %** (the plan's rounded 1.54
  gives +2.7 %), and against the research's frozen 1.59 it is 0.055 *under*. Neither limb holds, so no lever
  is invented for a runner event.
- **`reprice`** is therefore what the rule names: the median moved **+5** — more than the **+1.535** priced
  addition — while the token line is exactly where it was predicted (`gates=118 lines=358`, `claims=29
  provisional=28 claim-refs=27`, `steps`/`faults`/`goldens` unmoved).

**The single verdict is `reprice`.** Written as one bullet in `.planning/STATE.md`'s `## Accumulated Context`.

**Window honesty note.** Three newer hosted runs exist on GitHub for Phase 14's close (`45752237522` push,
`45756308953` pull_request, `45756570481` push) and are **not** in the committed `.planning/ci-runs.tsv`.
They are not recorded here: `tools/ci-record.sh`'s own header says "orchestrator only; never called by the
gate, CI or an executor" (AUTONOMY §4, the shared API rate limit), and §2.3 defines the arithmetic input as
the **committed** file. So the window above is the last five committed rows, and recording those three is the
orchestrator's call. If they land, the medians move and this verdict must be recomputed rather than
re-quoted.

## The hosted row

`hosted row: not-recorded — no push run for 85470b8 at 2026-11-26; the orchestrator records it from sh
tools/ci-record.sh <run-id> after the push`. `gh run list --commit 85470b8` returns no row, because this plan
does not push. `.planning/ci-runs.tsv` is untouched by this plan — no row was fabricated (plan lesson 30: a
milestone's close rows go in one records PR, and the push run that merge creates is the next milestone's
opening row).

## The `15-VALIDATION.md` sign-off

Amended, not rewritten (§2.10):

- Frontmatter: `status: validated`, `nyquist_compliant: true`, `wave_0_complete: true`; `created`, `phase`
  and `slug` unchanged.
- Test Infrastructure: framework `CTest over the registered suites plus the 38-step gate`; config
  `CMakePresets.json` and `data/expected/tests/*.ids`; quick run
  `IVS_TEST_OUT_DIR=<dir> ctest --preset dev -R '^runner-process$'`; full suite `sh tools/gate.sh`
  (`sh tools/gate-linux.sh` for the Ubuntu substance); runtime the measured `gate-time total=74` on this host
  (86–147 s at Phase 14's close, so a range, because this host's figure has varied by 70 % run to run) and
  **279 s** in the amd64-emulated container.
- Sampling Rate: the quick run after every task commit, the full gate after every wave, and **max feedback
  latency 19.0–19.8 s** measured twice this session. The plan-time "≈29.5 s" figure is **stale by
  measurement** and is named as stale rather than ticked.
- Per-Task Verification Map: **eight** rows — `15-01-T1`, `15-01-T2`, `15-02-T1`, `15-02-T2`, `15-02-T3`,
  `15-03-T1`, `15-03-T2`, `15-03-T3` — each with its plan, wave, requirement (TOOL-08), its threat refs
  (T-15-01…T-15-19 as each plan's register assigns them), the test type, its own plan's automated command,
  `File Exists` and `✅ green`. Eight rows before the sign-off, eight after: the sign-off added none and
  deleted none.
- Wave 0 Requirements: every bullet ticked, each amended to name the file that actually satisfies it, with
  the measured `steps=38/38 faults=238/238 goldens=29 controls=404/404` — no new framework, no new suite, no
  new gate step.
- Manual-Only Verifications: `All phase behaviors have automated verification.` — the ceiling is one
  `deterministic-repeatability` claim resting on committed ids, nothing is handed off, no row carries
  `evidence: none`.
- Validation Sign-Off: all six boxes ticked, each true and measured.
- `**Approval:**` carries the full measured gate line and the date.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] `tools/pcm/ivs_refpcm` was built without `POSITION_INDEPENDENT_CODE`**
- **Found during:** Task 3, the container leg (committed as `85470b8`)
- **Issue:** since plan 15-01 links `ivs_refpcm` into `host/libivs_retro`, an instrumented non-PIC member
  carries a relocation against `__asan_option_detect_stack_use_after_return` that a shared object cannot use:
  `relocation R_X86_64_PC32 … can not be used when making a shared object`. GNU `ld` refuses it; Darwin's
  `ld` accepts it — which is exactly why the container leg exists.
- **Fix:** PIC on that target, for the same reason the core's object libraries already carry it (D-34).
- **Files modified:** `tools/pcm/CMakeLists.txt`
- **Commit:** `85470b8`

**2. [Rule 3 - Blocking] `hygiene-history.sh`'s throwaway repo launched a detached `git gc` that raced its own cleanup**
- **Found during:** Task 2, as a gate failure (committed inside `f9b7864`)
- **Issue:** `git commit` in the step's temp repo launched a DETACHED auto-maintenance run that held
  `.git` open and raced the EXIT-trap `rm -rf`, so `rm: … Directory not empty` landed in the shared gate
  log **after** the pass line and the runner reported `gate-step-no-pass-line`. The trigger is the temp
  repo's loose-object count crossing git's sampled `gc.auto` threshold once the file set's blob hashes
  distribute that way — which this commit's new blobs did.
- **Fix:** `-c gc.auto=0 -c maintenance.auto=false` on that commit. Nothing there wants a pack, so the
  auto run is disabled rather than waited for. No new token, no new mechanism, no new control: the fix
  removes a race, it adds no refusal.
- **Files modified:** `tools/check/steps/hygiene-history.sh`
- **Commit:** `f9b7864`

**3. [Rule 1 - Bug] the ELF crt's four weak undefined stubs were refused as host imports**
- **Found during:** Task 3, the same container leg
- **Issue:** the ELF crt emits `_ITM_deregisterTMCloneTable`, `_ITM_registerTMCloneTable`, `__cxa_finalize`
  and `__gmon_start__` into every shared object; `tools/check/host-symbols.sh` refused
  `_ITM_deregisterTMCloneTable` as `host-symbols-import`.
- **Fix:** the four names are listed in `data/expected/host-imports.ids` rather than hidden behind an
  allow-prefix — a prefix would permit `_ITM_*` and `__gmon_*` spellings the contract never reviewed; each is
  weak undefined, so a build without them still links.
- **Files modified:** `data/expected/host-imports.ids`
- **Commit:** `85470b8`

### Recorded deviations (not code defects)

**4. The container evidence path.** `/tmp/g15linux.log` held the pre-fix FAIL run when this session resumed;
the passing run's log was `build/gate-linux.log`. It was copied to the plan's named path at the close and the
pre-fix log preserved beside it. Nothing was re-run to produce this (plan lesson 18). Recorded in full above.

**5. The plan's arithmetic rounds 1.535 to 1.54, and its lever comparison uses a 1.5 line while its context
names a frozen 1.59.** The exact 1.535 is used in every figure in this SUMMARY, with the plan's rounded
values printed beside it. The verdict is unaffected: `reprice` holds at either rounding, and both the
cumulative spend (5.005 of 21) and the overrun (+2.33 % or −3.5 %) are far from the lever line.

**6. Three newer hosted runs exist on GitHub and are not in the committed TSV.** Recorded as the window
honesty note above rather than appended, because `tools/ci-record.sh` is orchestrator-only.

### Estimate versus actuals

The plan estimated 60,000 tokens; the realized diff is **2,684** on the same scale (chars/4 over 10,737
changed characters in `94e39a2..85470b8`). The estimate is **22.4×** the realized diff. This is recorded as
measured rather than rounded toward the estimate, and it is a calibration finding about the instrument, not
about this plan's size: phase 14's `14-04` estimated 52,000 against a realized 4,616 (11.3×). Both plans are
record-heavy and cheap to execute; the estimate scale is roughly an order of magnitude above the realized-diff
scale and should be recalibrated before it is used as a budget again.

## Known Stubs

None. This plan adds no code path, no placeholder and no unwired component: it edits records, one registry,
thirteen toy files, one ledger, one generated table and two record documents.

## Threat Flags

None. No new network endpoint, auth path, file access pattern or schema change at a trust boundary. The two
files touched outside `.planning/` and the registries — `tools/pcm/CMakeLists.txt` (a PIC flag) and
`data/expected/host-imports.ids` (four weak undefined ELF stubs) — narrow surface rather than widen it, and
both are inside the phase's `<threat_model>` as T-15-11…T-15-13 territory.

## Self-Check: PASSED

- `d712f02`, `f9b7864`, `85470b8` are ancestors of `HEAD` (`git merge-base --is-ancestor`, each silent).
- The realized diff is 23 files (`git diff --name-only 94e39a2..85470b8`), plus this plan's four record
  files (`15-03-SUMMARY.md`, `15-VALIDATION.md`, `STATE.md`, `ROADMAP.md`): 27 in total.
- `git rev-list --count 94e39a2..85470b8` = **3**, matching `actuals.commits`.
- `data/expected/important-gates.ids` carries both `host mutant` rows; both ids are present in
  `data/expected/tests/runner-process.ids`.
- `awk '!/^#/ && NF' data/expected/important-gates.ids | wc -l` = **358**.
- TOOL-08's traceability row reads `Complete` and no `- [ ] **TOOL-08**` line remains.
- `CL-0029` is in `data/claims.json` and `data/expected/claims.ids`; `docs/evidence-tiers.md` carries exactly
  one `[claim:CL-0029]` tag; the CHANGELOG block `cmp`s clean against a fresh render.
- `sh`/`dash` `tail -1` `cmp` clean at 1509 bytes each; both exit 0; zero `-- FAIL` lines.
- `sh tools/hygiene.sh --tree` silent (`-- hygiene pass mode=tree hygiene-files=2771` with a clean tree
  before this file existed; `hygiene-files=2772` once it did — the same `FILELIST` convention as `tracked`,
  and `git ls-files | wc -l` reads 2,771 until the metadata commit);
  `git ls-files | grep -E '[.](ivsc|bin|rom|so|dylib|dll|bundle|cfg|ivsn)$'` empty.
- `15-VALIDATION.md` carries `status: validated`, exactly eight `| 15-0X-TY |` rows, and every box ticked.
