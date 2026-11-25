---
phase: 09-registered-evidence-and-gate-cost
plan: 07
subsystem: gate
tags: [gate, controls, sources, manifests, memmap, ev-03]
dependency-graph:
  requires: [09-06]
  provides:
    - "whole-token SRC- extraction with a control"
    - "two-root manifest output anchoring with a control"
    - "the memmap generator running under the gate, with its two tokens and its control"
  affects: [09-08, 09-09]
tech-stack:
  added: []
  patterns:
    - "a token the generator emits is additive: the step's own checker still runs"
    - "a control pins the reason, a committed selftest case proves the liveness guard"
key-files:
  created:
    - tools/check/controls/sources-src-token-whole/CHECK
    - tools/check/controls/sources-src-token-whole/EXPECT
    - tools/check/controls/sources-src-token-whole/FORBID
    - tools/check/controls/sources-src-token-whole/overlay/docs/evidence-tiers.md
    - tools/check/controls/manifests-output-anchor/CHECK
    - tools/check/controls/manifests-output-anchor/EXPECT
    - tools/check/controls/manifests-output-anchor/overlay/fixtures/FX-good/manifest.json
    - tools/check/controls/memmap-generator-refused/CHECK
    - tools/check/controls/memmap-generator-refused/EXPECT
    - tools/check/controls/memmap-generator-refused/FORBID
    - tools/check/controls/memmap-generator-refused/overlay/data/memmap/intv-2609-ntsc-family-p1.literal.txt
  modified:
    - tools/check/steps/sources.sh
    - tools/check/manifests.cmake
    - tools/check/steps/memmap.sh
    - tools/check/steps/runner-selftest.sh
    - data/expected/controls/provenance.ids
    - data/expected/controls/rights.ids
    - data/expected/controls/memmap.ids
decisions:
  - "the SRC- class is [A-Za-z0-9_-]+ with no dot, chosen by measurement not by the research's proposal"
  - "manifest outputs are anchored to two roots (the fixture directory or data/) because one root refuses the pristine tree"
  - "the memmap generator's refusal does not short-circuit the step, because six existing controls build trees it refuses"
  - "the denylist-no-files / runner-scan-empty extension is declined, not silently dropped"
metrics:
  duration: "~13 min wall clock (plan_head_before 21:21 to closing task 21:34 local)"
  completed: "2026-11-24"
status: complete
actuals:
  tokens: 4310
  tasks: 3
  commits: 3
  plan_head_before: 548ea76ede627e358f5ab0c4a0d03de4ed3a0cf3
  plan_head_after: 659c29e367da94ef388b2f41c8e07ad20b73ac61
---

# Phase 9 Plan 7: Registered gate evidence for four EV-03 items Summary

Three gate-hardening items landed with a control each and a fourth was closed by re-running the two
controls it already had: `SRC-` references are now extracted whole, manifest outputs are anchored to
their fixture directory or to `data/`, and the build's own memory-map generator runs under the gate
with a refusal and an absence that are distinguishable.

## What landed

### Item 1 - the whole-token `SRC-` match (`sources-src-token-whole`)

The extraction at `tools/check/steps/sources.sh:48` was `SRC-[a-z0-9-]+`. Two defects, both
reproduced before the fix and both recorded as a committed control:

| Overlay line | Old extraction | New extraction |
|---|---|---|
| `SRC-bsr-2609_nope` | `SRC-bsr-2609` - **registered**, so the step exits zero | `SRC-bsr-2609_nope` - `source-unknown-ref` |
| `SRC-Nope-Doc` | **no match at all** | `SRC-Nope-Doc` - `source-unknown-ref` |

The class is now `SRC-[A-Za-z0-9_-]+`. **The research's proposed class was measured and rejected**:
`SRC-[A-Za-z0-9_.-]*[A-Za-z0-9]` swallows the file extension and extracts **three unregistered
tokens from the real tree** - `SRC-gi-1980-catalog.pdf`, `SRC-gi-1982-catalog.pdf`,
`SRC-gi-cp1600-um-1975.pdf` - which would turn the gate red. Measured over the real tree
(`data/**/*.json` except `data/sources.json`, `docs/**/*.md`, `README.md`):

| Class | distinct tokens | unregistered |
|---|---|---|
| `SRC-[a-z0-9-]+` (old) | 11 | 0 |
| `SRC-[A-Za-z0-9_.-]*[A-Za-z0-9]` (research) | 14 | **3** |
| `SRC-[A-Za-z0-9_-]+` (landed) | 11 | 0 |

No registry id contains a dot. This is why the plan's prose and its regex differ, and the reason is
written beside the extraction.

The control's `EXPECT` pins both tokens and its `FORBID` names `sources-no-files` (the wrong reason
a bad overlay could produce). There is deliberately **no `FORBID` on the truncated form**:
`EXPECT` is a substring match and `SRC-bsr-2609` is a prefix of the intended token, so such a line
would forbid the correct finding. A truncating regex is detected by `control-exit-zero` - observed
directly: with the old class restored in a temporary copy of `tools/check`, the runner printed
`-- FAIL control-exit-zero: sources-src-token-whole`. On the pristine good tree the same `CHECK`
passes (`control ok   _good/sources`), so the control fails for its intended reason only.

### Item 2 - the zero-document item, verified not rebuilt

Both guards were already landed and both already had controls; this plan re-ran them rather than
duplicating them:

| Guard | Control | Result |
|---|---|---|
| `tools/check/steps/glossary.sh:48` (`bad glossary-no-files "README.md docs/"`) | `glossary-no-files` | `control ok` |
| `tools/check/steps/sources.sh:33-35` (`sources-no-files: data/`) | `sources-no-files` | `control ok` |

Item 2 is **closed**. The remaining two uncontrolled zero-document tokens, `denylist-no-files`
(`tools/check/steps/denylist.sh:43`) and `runner-scan-empty`
(`tools/check/steps/runner-scan.sh:58`), are recorded as a **declined extension**: amendment A-02's
honest count for EV-03 is three controls plus one verification plus one step-side mechanism, and two
more controls is scope the phase did not price. They stay uncontrolled and the disposition belongs
to `09-09`.

### Item 4 - manifest output anchoring (`manifests-output-anchor`)

`check_entries` now refuses an `outputs[].path` that begins with neither `fixtures/<dirname>/` nor
`data/`, as `manifest-format: <label>/outputs/path`, raised **before** the digest comparison.
`notices` and `recipe.sources` are not anchored - recipe sources legitimately name
`tools/asm/asm.c` and `tools/pack/pack.c`.

The **two-root rule is measured**, not stylistic. All 11 real manifests, and every declared output:

| Manifest | outputs | paths |
|---|---|---|
| `FX-boot-stub-2` | 2 | `fixtures/FX-boot-stub-2/boot-stub-2.asm`, `…/boot-stub-2.listing.txt` |
| `FX-boot-stub` | 1 | `fixtures/FX-boot-stub/boot-stub.listing.txt` |
| `FX-demo-lamp-tone-1` | 2 | `fixtures/FX-demo-lamp-tone-1/lamp-tone.asm`, `…/lamp-tone.listing.txt` |
| `FX-diag-cpu-1` | 1 | `fixtures/FX-diag-cpu-1/diag-cpu-1.listing.txt` |
| `FX-diag-ctrl-psg-1` | 1 | `fixtures/FX-diag-ctrl-psg-1/diag-ctrl-psg-1.listing.txt` |
| `FX-diag-stic-1` | 1 | `fixtures/FX-diag-stic-1/diag-stic-1.listing.txt` |
| `FX-diag-stic-2` | 2 | `fixtures/FX-diag-stic-2/diag-stic-2.asm`, `…/diag-stic-2.listing.txt` |
| `FX-ivsc-vectors` | 2 | **`data/cart/ivsc-parse.json`, `data/cart/ivsc-load.json`** |
| `FX-owner-exec` | 0 | - |
| `FX-owner-grom` | 0 | - |
| `FX-sample-rights-unknown` | 0 | - |

11 manifests, 8 declaring a non-empty `outputs[]`, and the two `data/cart/` paths are why a
fixtures-only rule would refuse the pristine tree. The gate passing at this commit **is** the proof
that they are still accepted; the plan's python re-derivation is the authority.

Control output with the overlay applied (`outputs[0].path` -> `docs/other.pdf`):

```
-- FAIL manifest-format: FX-good/outputs/path
-- FAIL manifest-format: FX-good/path
-- FAIL manifest-missing-output: FX-good docs/other.pdf
```

There is **no `FORBID`**, and the reason is measured in that block: `gate_fail` accumulates, so the
output-bytes pass legitimately also fires for the moved path, and the manifest's own top-level
`path` is no longer one of its outputs. The control's discrimination is `control-token-missing`
when the anchoring branch is absent. Removing the overlay makes the step pass, which is the
`control-exit-zero` observation (`control ok   _good/manifests`).

### Item 5 - the memmap generator under the gate (`memmap-generator-refused`, `memmap-generator-missing`)

`tools/check/steps/memmap.sh` now invokes `cmake/IvsMemmapProbes.cmake` **first**, writing
`-DOUT` into its own `mktemp -d`, with the path derived from `CHECK_DIR`
(`$CHECK_DIR/../../cmake/IvsMemmapProbes.cmake`) because a control tree has no `cmake/` directory.
The header's "writes nothing" sentence is now "writes only inside its own temporary directory".

Two tokens:

- `memmap-generator-missing: cmake/IvsMemmapProbes.cmake` - the invocation's liveness guard, and
  the only path on which the step stops early.
- `memmap-generator-refused: <the generator's own first message>` - additive. **The generator runs
  first but does not short-circuit the step's own checker**: the generator's finding is printed,
  `memmap.cmake` still runs and prints its findings, and the step exits non-zero only after both.
  An early return is the defect the ordering exists to avoid - six existing controls build trees
  the generator refuses (`memmap-literal-grammar`, `memmap-exception-unknown`,
  `memmap-literal-hole`, `memmap-literal-overlap`, `memmap-literal-missing`,
  `memmap-literal-orphan`), so each would have lost its pinned token to `memmap-generator-refused`
  and the count would have read `controls=378/384`. Measured after the change: all six are `ok`.

One measured correction inside the token: cmake wraps a `FATAL_ERROR` message at the terminal width
and indents the continuation by two spaces, so a single-line extraction reported
`IvsMemmapProbes: …:2: mirror` and dropped the reason. The step re-joins the continuation lines.
Control output with `mirror=-` changed to `mirror=3` on the first row:

```
-- FAIL memmap-generator-refused: IvsMemmapProbes: data/memmap/intv-2609-ntsc-family-p1.literal.txt:2: mirror 3 is not a power of two up to 65536
-- FAIL memmap-literal-grammar: data/memmap/intv-2609-ntsc-family-p1.literal.txt:2 mirror 3 is not a power of two up to 65536 undriven needs impl=none w=0 mirror=- gate=-
```

`FORBID` is `memmap-generator-missing` and **not** `memmap-literal-`: the second line above is
`memmap.cmake:278-283` refusing the same row, and it is a legitimate part of this control's output
once the checker keeps running - the same reason the manifest control carries no `FORBID`.
`memmap-generator-missing` still discriminates (a step that reported the liveness guard instead of
a refusal is caught), and a step with no generator invocation at all fails `control-token-missing`
on `EXPECT`.

`memmap-generator-missing` is proved by a **committed, re-runnable** case rather than prose, because
no control directory can reach it (the path is derived from `CHECK_DIR`, and a control may only
change the tree). `tools/check/steps/runner-selftest.sh` gained `st-memmap-generator-missing`,
reusing 09-06's copied-`CHECK_DIR` helper:

```
-- runner-selftest proof memmap-generator-missing: cmake/IvsMemmapProbes.cmake (observed)
-- runner-selftest pass cases=12
```

The copied `CHECK_DIR` was moved one level deeper (`<tmp>/checkcopy/tools/check`) so that its `../..`
is a directory this case owns. The plan's layout (`<tmp>/checkcopy`) put `../..` at `$TMPDIR`, where
a stray `cmake/` would silently make the case pass for the wrong reason - a nondeterminism a
committed liveness case cannot carry.

**Partition guard.** `grep -n -e 'IVS_GATE_DIR' -e 'build/' tools/check/steps/runner-selftest.sh`
is **empty, exit status 1**, before and after the edit. The new case supplies `ROOT`, `CHECK_DIR` and
`FILELIST` only, and the filelist is built with `find … -type f`, so the script's own source never
spells the tree's generated-directory name; `runner-selftest` stays a preflight step and 09-08's
frozen 29/9 split is untouched. `memmap` itself remains a producer step (it references `build/` and
`IVS_GATE_DIR` at `:24`), so the partition is undisturbed.

## Measured tokens

Predicted before each commit, matched exactly:

| Task | `controls` | `cases` | `files` delta | commit |
|---|---|---|---|---|
| start | 381/381 | 11 | - | `548ea76` |
| 1 | **382/382** | 11 | +4 | `14878c0` |
| 2 | **383/383** | 11 | +3 | `f04b42f` |
| 3 | **384/384** | **12** | +4 | `659c29e` |

Unchanged throughout: `steps=38/38`, `faults=235/235`, `gates=107`, `lines=330`,
`tests-expected=4958`, `tests-executed=4954`, `tests-skipped=4`, `tests-missing=0`, `claims=28`,
`manifests=11`, `effective-yes=8`, `literal-rows=80`, `views=2`, `dump-lines=160`, `sources=12`,
`refs-verified=3/4`.

Observed absolute file triple (a recorded observation, never a predicate): `files=2551`,
`tracked=2551`, `hygiene-files=2551` - eleven added files over the plan's `2540` start.

Gate lines: `sh` and `dash` both `gate: PASS steps=38/38 controls=384/384 faults=235/235 gates=107
lines=330 cases=12 manifests=11 effective-yes=8 literal-rows=80 views=2 dump-lines=160
tests-expected=4958 tests-skipped=4 claims=28`, with zero top-level `-- FAIL` lines.

## Deviations from Plan

### Approved budget deviation

The dispatch carried a hard 1800-second limit. The plan's Task 1 and Task 2 verify blocks each run
both `sh tools/gate.sh` and `dash tools/gate.sh`. Executed as: `sh tools/gate.sh` at the end of
**every** task, and `dash tools/gate.sh` **once**, at Task 3 (plus once more on the final tree).
Recorded here as the approved deviation the dispatch authorised; `dash` and `sh` both end `gate:
PASS` on the final tree, and the shell is not the variable under test in this plan.

### Auto-fixed issues

**1. [Rule 1 - Bug] The generator's refusal message lost its reason**

- **Found during:** Task 3, while confirming the control's `EXPECT` lines against a real run.
- **Issue:** cmake wraps a `FATAL_ERROR` message at the terminal width and indents the
  continuation by two spaces, so extracting "the generator's own first message line" produced
  `IvsMemmapProbes: …:2: mirror` and dropped `3 is not a power of two up to 65536`. The control's
  second `EXPECT` token was then satisfied only by `memmap.cmake`'s own line, so the control would
  have proved the checker's reason, not the generator's.
- **Fix:** the step re-joins the message's indented continuation lines before printing the token.
- **Files modified:** `tools/check/steps/memmap.sh`
- **Commit:** `659c29e`

**2. [Rule 3 - Blocking] The copied `CHECK_DIR` for the liveness case had a nondeterministic `../..`**

- **Found during:** Task 3, before writing the case.
- **Issue:** the plan places the copied `CHECK_DIR` at `<tmp>/checkcopy`, so the step's
  `$CHECK_DIR/../../cmake/…` resolves to `$TMPDIR/cmake/…`. A stray directory there would make the
  liveness case pass for the wrong reason.
- **Fix:** the copy now lives at `<tmp>/checkcopy/tools/check`, so `../..` is `<tmp>/checkcopy`,
  owned by the case. The existing pass-line case is unaffected (all its paths are relative to `$cc`).
- **Files modified:** `tools/check/steps/runner-selftest.sh`
- **Commit:** `659c29e`

### Not fixed, by scope

Nothing else was found. `deferred-items.md` was not needed.

## D-09-06 registration

This plan's four mechanisms are four of the **nine** mechanisms enumerated as the canonical list in
deviation **D-09-06** of `09-05-SUMMARY.md`:
`important-gates-complete-uncovered`, `important-gates-excluded-stale`, `control-toy-drift`,
`control-deviation-unused`, `control-pass-line-not-last`, `sources-src-token-whole`,
`manifests-output-anchor`, `memmap-generator-refused`, `memmap-generator-missing`. The list is cited
by name and not re-derived as a subset. GUARDRAILS 2.3's registration limb stays deferred against
the frozen `lines=330` (ROADMAP criterion 3, amendment A-08); the conflict and the reversal
condition are written out in `09-05-SUMMARY.md`. The liveness-guard and committed-stand-in limbs are
met here by the three controls and the `st-memmap-generator-missing` case.

## Threat surface

`T-09-13` (tampering with the `SRC-` extraction and the output anchoring) is `mitigate` and
addressed: the class is measured against the pristine tree (0 unregistered of 11), its control pins
both reproductions, and the anchoring rule's second root is re-derived by an automated check so the
rule cannot be quietly widened. `T-09-14` (the memmap control) is `mitigate` and addressed: the
generator's finding is additive so six existing controls keep their tokens, the `FORBID` separates a
refusal from the liveness guard, and the guard is re-proved on every gate by a committed case.
`T-09-15` (the zero-document item) is `accept` and the declined extension is recorded rather than
claimed as done. `T-09-SC` is accepted: no package manager was invoked; `cmake -P` runs an in-repo
script and no dependency was added. No Threat Flags.

## Known Stubs

None. Every added file is a control declaration or an inventory line, and every mechanism added here
runs in every gate.

## Self-Check: PASSED

- `tools/check/steps/sources.sh` contains `SRC-[A-Za-z0-9_-]+` - FOUND
- `tools/check/manifests.cmake` anchors outputs to `fixtures/` and `data/` - FOUND
- `tools/check/steps/memmap.sh` contains `memmap-generator-refused` and `memmap-generator-missing` - FOUND
- `tools/check/controls/sources-src-token-whole/EXPECT` contains `SRC-bsr-2609_nope` - FOUND
- `tools/check/controls/manifests-output-anchor/EXPECT` contains `outputs/path` - FOUND
- `tools/check/controls/memmap-generator-refused/EXPECT` contains `not a power of two` - FOUND
- eleven added control files, three inventory lines - FOUND
- commits `14878c0`, `f04b42f`, `659c29e` - all ancestors of `HEAD`
