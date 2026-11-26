---
phase: 16-owner-route-claim-and-release-posture
plan: 02
subsystem: release-route
tags: [owner-route, source-archive, rehearsal, release-record, flatwrap, owner-run, project-made-bytes]

requires:
  - phase: 14-owner-supplied-content-and-the-cartridge-role
    provides: the cartridge role (`--cart`), the wrap tool `ivs_flatwrap` and the helper `tools/owner-run.sh`
  - phase: 08-release-artifacts-and-the-dry-run
    provides: `--source-route`'s archive extraction, cold build and filtered golden comparison, unchanged here
  - phase: 16-01
    provides: the tree this plan started from (`3dd0543`), `steps=38/38` and every token of the summary line

provides:
  - "`tools/release-rehearse.sh --source-route` performs the three commands of the owner route against the archive it already unpacks and builds, on project-made zero-filled images"
  - "the route's pass path proved able to fail in the same run: one EXEC byte after `--record` is refused at load with `IVS_E_DIGEST_MISMATCH`, exit 14, `cpu.run_state unreset`"
  - "`docs/owner-images.md` section \"The route from the source archive, in three commands\", citing `.planning/ops/RELEASE-OPS.md` section (j)"
  - "`.planning/ops/RELEASE-OPS.md` section (j), the rehearsal's transcript, appended with zero deleted lines"

affects:
  - "16-03: the route this plan rehearses is what handoff row `H16-1` is offered as"
  - "16-04: the release-decision line and the PKG-06 tick; this plan ticks nothing"
  - "16-05: the phase close reads section (j) as a record, never as gate evidence"

actuals:
  tokens: 4540
  tasks: 2
  commits: 2
  plan_head_before: 3dd0543c09b51c6f26a59940e5ae0759c071995c
  plan_head_after: 6b57148

tech-stack:
  added: []
  patterns:
    - "a release-time limb that refuses a CI environment (exit 2) rather than skipping it, because a route silently left out reads as a pass"
    - "project-made zero-filled images as the route's inputs, so the rehearsal proves the path without reading a rights-bearing byte"
    - "a mutation limb in the same run as the pass limb, so neither half of a rehearsal can be satisfied alone"
    - "a transcript whose every figure is quoted from the log it names, so a reader can check the record against the run"

key-files:
  created: []
  modified:
    - tools/release-rehearse.sh
    - docs/owner-images.md
    - .planning/ops/RELEASE-OPS.md

key-decisions:
  - "The owner-route limb runs after the existing golden comparison and inside the same `--source-route` mode, so the route's endpoint stays the committed `data/golden/demo-lamp-tone-pass.digests`; no second oracle and no new golden."
  - "The three inputs are made with `dd` from the zero device inside the extraction's `refs/` (8,192 EXEC, 2,048 GROM, 8,192 flat), because the helper's location rule allows only `refs/` inside its work tree and no owner file may be read."
  - "The wrap is the archive's own `build/release/tools/flatwrap/ivs_flatwrap` at base `4000` with the default `hi-first`, and the digest it prints is checked against the run's own `slot.cart.sha256`, so the container the route loads is the container the archive's tool wrote."
  - "The mutation run is the `--cart`-only form: a second `--record` is refused with exit 2 by design, and the `--cart`-only form is also what the gate's own `runner-process.judge.owner-run-cart-pass` case performs, so the record and the gate's case agree by construction."
  - "`usage()`'s printed line range moved from `3,38` to `3,49` in the same commit as the header it prints, because the range is by line number and a header edit alone would make `--help` print a slice of the function body below it."
  - "`docs/RIGHTS.md` did not move: no new third-party item is read or named, the flat/EXEC/GROM/cartridge rows already exist from Phases 13 and 14, and V10's co-moving page is the one whose behaviour changed."

patterns-established:
  - "A rehearsal limb that performs the documented route rather than a proxy for it: the same three commands, the archive's own tools, on files the run makes itself."
  - "A transcript as a copy of a log: each figure in the record is greppable in the log the record names, and the run's own printed lines are quoted verbatim."

requirements-completed: []

coverage:
  - id: D1
    description: "`--source-route` performs the owner route from the source archive: wrap a project-made flat image with the archive's own `ivs_flatwrap`, then load EXEC, GROM and container through the archive's own `tools/owner-run.sh --cart`, ending at the committed project-cartridge golden"
    requirement: PKG-06
    verification:
      - kind: integration
        ref: "sh tools/release-rehearse.sh --release --source-route; the three `rehearse: owner-route:` lines, `rehearse: ok`, and exactly one `rehearse: source-route: … golden match (report 131 lines, 124 of 124 compared), prefix ivs-v0.2.0` line"
        status: pass
    human_judgment: false
  - id: D2
    description: "the pass path is proved able to fail in the same run: a one-byte EXEC mutation after `--record` is refused at load, and the `exec:` label is exactly `-`"
    requirement: PKG-06
    verification:
      - kind: integration
        ref: "the rehearsal's `rehearse: owner-route: run exit=10 outcome=guest-fail exec-label=- cart-digest-matched=yes` and `rehearse: owner-route: mutation exit=14 status=IVS_E_DIGEST_MISMATCH run-state=unreset`"
        status: pass
    human_judgment: false
  - id: D3
    description: "the route is documented as three commands in `docs/owner-images.md` and its transcript is appended as `.planning/ops/RELEASE-OPS.md` section (j), every figure quoted from the log it names"
    requirement: PKG-06
    verification:
      - kind: other
        ref: "grep set over section (j) (archive name, golden-match text, 64-hex wrap digest, mutation token, `not run`, `not-performed`, `zero-filled`) plus `git diff --numstat` column 2 == 0 (append-only)"
        status: pass
    human_judgment: false

duration: 21min
completed: 2026-11-26
status: complete
---

# Phase 16 Plan 02: The Owner Route From the Source Archive, Rehearsed and Recorded

**`--source-route` now performs the three commands a person performs from the released archive — build it cold, wrap a flat image at base `4000`, load EXEC + GROM + the container — and the same run proves the pass path can fail: one flipped EXEC byte is refused at load with `IVS_E_DIGEST_MISMATCH`, exit 14, while the run's `exec:` label stays `-`.**

## Performance

- **Duration:** 21 min
- **Started:** 2026-11-26T09:19:17Z
- **Completed:** 2026-11-26T09:40:58Z
- **Tasks:** 2 of 2
- **Files modified:** 3 (no file created, no file deleted)

## The transcript

- **Path:** `.planning/ops/RELEASE-OPS.md` section **(j)**, "The owner route, rehearsed from the source archive"
  (49 inserted lines, 0 deleted).
- **Source log:** one run of the release-time shape, on the tree task 1 committed (`14f81ed`).
- **Cited from:** `docs/owner-images.md`, "The route from the source archive, in three commands".

The three commands, as `docs/owner-images.md` states them and as the rehearsal performs them:

1. `tar -xzf iamverysmartvision-<version>-source.tar.gz` then `cd iamverysmartvision-<version>`, `cmake --preset
   release`, `cmake --build --preset release`.
2. `build/release/tools/flatwrap/ivs_flatwrap refs/FX-owner-flat.bin 4000 refs/FX-owner-cart.ivsc`.
3. `sh tools/owner-run.sh --record --cart refs/FX-owner-cart.ivsc refs/FX-owner-exec.bin
   refs/FX-owner-grom.bin`.

The rehearsal's four printed lines, quoted from the log:

```
rehearse: source-route: ivs-v0.2.0-source.tar.gz, golden match (report 131 lines, 124 of 124 compared), prefix ivs-v0.2.0
rehearse: owner-route: wrap base=4000 word-order=hi-first digest=8d73b98c58bb75a6515c6a791ebd2925265d05874e324c31a5836e61055b74f5
rehearse: owner-route: run exit=10 outcome=guest-fail exec-label=- cart-digest-matched=yes
rehearse: owner-route: mutation exit=14 status=IVS_E_DIGEST_MISMATCH run-state=unreset
```

**The mutation's measured refusal.** After `--record` wrote the lock from the two images, one byte at offset 0 of
the 8,192-byte EXEC image was set to `\001` with `dd conv=notrunc`, and the same run was repeated in the
`--cart`-only form (a second `--record` is refused with exit 2, `refs/owner-images.sha256 exists: remove it to
record again`). The core refused at load: exit **14**, `outcome.detail load-refused role=exec
status=IVS_E_DIGEST_MISMATCH`, `cpu.run_state unreset`. So the pass assertions above cannot be satisfied by
anything that merely exits 10.

**The `exec:` label value is `-`.** The report's line is exactly `exec: -`, which is the statement that nothing in
this run reads as a real-EXEC result; D11 stays `not-performed`, no claim cites section (j), and the record says
in words that the EXEC, GROM and flat images are project-made zero-filled files that the run writes with `dd`.

## Task Commits

| # | Task | Commit | Files |
|---|---|---|---|
| 1 | The three-command route in the rehearsal and in the operator page | `14f81ed` | `tools/release-rehearse.sh`, `docs/owner-images.md` |
| 2 | The transcript, measured once and recorded as a release record | `6b57148` | `.planning/ops/RELEASE-OPS.md`, `docs/owner-images.md` |

**Measured commit count:** `git rev-list --count 3dd0543..6b57148` = **2**.

## What changed

- `tools/release-rehearse.sh` — the `--source-route` limb grew the owner route after its existing golden
  comparison: a CI refusal (exit 2, one `rehearse: refused:` line), a `refs/` directory and a sibling `TMPDIR`
  inside the rehearsal workspace, three `dd`-made zero-filled inputs with an explicit size check each, the
  archive's own `ivs_flatwrap` at base `4000` / `hi-first` with its printed digest kept, the archive's own
  `tools/owner-run.sh --record --cart` with eight assertions on its report, then the one-byte EXEC mutation and
  the `--cart`-only rerun with three more. Every failure prints the captured stderr or the report's tail and
  exits 1 through the file's existing `fail` helper. `dd` joined the required-tool list. `usage()`'s range moved
  `3,38` → `3,49` in the same commit, and `--help` prints the whole contract and nothing but it.
- `docs/owner-images.md` — one new section before "The helper", "The route from the source archive, in three
  commands": the three commands as commands, the base and word-order contract, where the files live, the
  container as the tool's output and derived material, the frontend's ownership of picture and sound, and the
  pointer to `.planning/ops/RELEASE-OPS.md` section (j). Task 2 amended that pointer with the archive the
  transcript was measured from (`ivs-v0.2.0-source.tar.gz`), the base (`4000`) and word order (`hi-first`) the
  rehearsal used, and the one-byte EXEC refusal — no figure the record does not carry, and no untagged claim
  word.
- `.planning/ops/RELEASE-OPS.md` — section (j) appended: the command and the commit, the archive and its byte
  size, the prefix, the cold configure/build result, the four printed lines verbatim, the wrap and run details,
  the three facts a reader must not have to infer, and the clean-workspace statement. Sections (a) to (i) are
  untouched (`git diff --numstat` column 2 is `0`).

## Invariants, re-measured

The gate ran four times in this plan (baseline, task 1, task 2, and once more before the metadata commit) and
every token the plan predicted as an invariant held: `steps=38/38 controls=404/404 faults=238/238 goldens=29
gates=118 lines=361 docs=64 tests-expected=5042 tests-executed=5038 tests-skipped=4 claims=29 provisional=28
claim-refs=27 prefix-files=11`, with `tracked=2784 files=2784 hygiene-files=2784`. No gate step, control, fault,
golden, registry row, claim, test id or selftest was added, and the summary line is byte-identical between task
1's log and task 2's (`cmp` on `tail -1`).

The metadata commit that adds this SUMMARY moves `tracked`/`files`/`hygiene-files` by exactly `+1`
(2784 → 2785) and nothing else, for the same reason every earlier phase's completion commit did: this file is
new and `gate.sh`'s `FILELIST` counts untracked-not-ignored paths, so the gate run that proves it was made with
this file present and untracked.

## Deviations from Plan

### Plan defects (recorded with the measured reason, not silently satisfied)

**1. [Plan defect] Task 1 asks the wrap line to name the archive, and the same task pins that line's format**
- **Found during:** Task 1, writing the printed lines.
- **Measured:** the task's `<automated>` requires
  `^rehearse: owner-route: wrap base=4000 word-order=hi-first digest=[0-9a-f]{64}$` — a format with no room for
  an archive name — while the task's `<action>` asks for "one naming the archive and the wrap (base, word order,
  digest)", and the same action requires the `rehearse: source-route: … golden match …` line (which does name
  the archive) to stay byte-identical and to remain the limb's last line.
- **Substitution:** the pinned format was followed; the archive is named in the golden-match line, which the
  transcript quotes immediately above the wrap line. No fourth `rehearse: owner-route:` line was invented,
  because the verify pins that count at exactly three.

**2. [Plan defect] "`sh tools/hygiene.sh --tree` is silent"**
- **Measured:** it prints its pass line (`-- hygiene pass mode=tree hygiene-files=2784`) and exits 0. Every step
  in this project prints a pass line by contract, so silence was never available. This is the same defect plan
  16-01 recorded for the same sentence.
- **Substitution:** the check is "exit 0 and no `-- FAIL`", which is what the verify's `set -e` asserts.

**3. [Plan defect, latent] Task 2 asks the record to carry "the cold configure and build result", and the
rehearsal prints no line for either step**
- **Measured:** a non-zero status at `cmake --preset release` or `cmake --build --preset release` ends the run
  with a `rehearse: FAIL:` line and exit 1; a success prints nothing. The log therefore carries the *absence* of
  a FAIL and the golden line that is only reachable after a successful build, not a quotable result line.
- **Substitution:** section (j) states exactly that ("the log holds no `rehearse: FAIL:` line, and the golden
  comparison below is reached only after that build succeeds, because a non-zero status at either step ends the
  rehearsal") rather than inventing a figure the log does not hold.

**4. [Plan defect, vestigial] Task 2's `<fails_when>` guard annotations name figures no assertion reads**
- **Measured:** its `[HEAD=1509 EXPECT==1509 …]` line is 16-01's summary-line byte count, and its
  `[HEAD=0 EXPECT==0 …]`, `[HEAD=27 …]` and `[HEAD=2784 …]` lines annotate tokens that Task 2's `<automated>`
  does not assert (Task 2 checks the numstat column, the `## (j)` count and the token line by `cmp`).
- **Substitution:** none needed — the `<automated>` block is the contract and it passed in full. Recorded
  because the annotation is not the check it looks like.

All four are also in the cross-phase register `.planning/WINDOWS.md` as rows 6 to 9, kind `deviation`, each waived with its reason, so a later ship gate sees them without reading this SUMMARY:
`open_count` stays 3 (the three Phase 2 stubs) and `waived_count` moves 2 → 6.

### Not this plan's to do, by name

- **`docs/RIGHTS.md` did not move.** No new third-party item is read or named; the flat, EXEC, GROM and
  cartridge rows already exist from Phases 13 and 14; and V10's co-moving page is the one whose behaviour
  changed (`docs/owner-images.md`, with `tools/release-rehearse.sh`).
- **PKG-06 is not ticked.** The plan's frontmatter names it, but the phase's artifact table assigns
  `REQUIREMENTS.md` (PKG-06 `Complete`) to **16-04 T2**, so `requirements-completed` is empty.
- **`sh tools/gate-linux.sh` was not run**: the phase's single container gate is plan 16-05's, and a second one
  would overwrite the first run's log (lesson 18).
- **Nothing was pushed, tagged or published**, and no `.planning/config.json` or `.planning/state.json` was
  staged (D-31, D-42).

## Issues Encountered

- The source archive's byte size is a function of the commit, so the two rehearsal shapes in this plan measure
  different archives: `--no-gate --release --source-route` at `14f81ed` read 6,757,112 bytes (the tree before
  task 1's commit), and the release-time run at the same commit read 6,759,881 bytes. Both are correct for the
  tree they measured; section (j) quotes the release-time run, which is the shape a release uses, and says which
  commit it ran on.
- The wrap digest is stable across runs (`8d73b98c…` in every one), which is what a container built from a
  zero-filled 8,192-byte flat image at a fixed base and word order must be; the transcript's digest is therefore
  checkable by re-running the mode, not only by trusting the record.

## Known Stubs

None. No `<verify>` was skipped, no test was left unwired, and the two new printed lines carry measured values
rather than placeholders.

## Threat Flags

None. The change adds no network surface, no new executable input and no tracked artifact: every byte the route
writes lands in one `mktemp` workspace removed by the existing `EXIT` trap (`git status --porcelain` was empty
after the run), and the register's T-16-05 to T-16-08 are each exercised by the run above — project-made inputs
only, the `exec:` label pinned to `-`, the mutation limb, and the transcript quoting its own log.

## User Setup Required

None — no external service, credential or manual step.

## Next Phase Readiness

- 16-03 can offer this route as handoff row `H16-1`: the route now exists as a rehearsal with a transcript, and
  what remains owner-only is "a real frontend loads the shim", not "does the route exist".
- 16-04's release-decision recomputation may cite section (j) as a *record*; it must not read it as gate
  evidence, and the phase's checkpoint verdict stays 16-05's arithmetic over `.planning/ci-runs.tsv`.
- The one open risk is staleness: section (j) is a snapshot of one commit. A later release re-runs the mode and
  appends a new section rather than editing this one (§2.10).

## Self-Check

- `tools/release-rehearse.sh` — FOUND
- `docs/owner-images.md` — FOUND
- `.planning/ops/RELEASE-OPS.md` — FOUND
- `.planning/ops/RELEASE-OPS.md` section `## (j)` — FOUND, exactly once
- commit `14f81ed` — ancestor of HEAD
- commit `6b57148` — ancestor of HEAD

## Self-Check: PASSED
