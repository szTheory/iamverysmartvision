---
phase: 17-records-cost-window-and-the-count-gate-contract
plan: 02
subsystem: records-cost-window
tags: [flatwrap-refusal-tokens, source-archive-letter, flatwrap-shim-join, ev-08-flip, count-gate]

requires:
  - phase: 17-records-cost-window-and-the-count-gate-contract
    plan: 01
    provides: "the recomputed window, the 0.11 priced line and the count-gate contract this plan's records answer"
  - phase: 14
    provides: "the eight append-only refusal tokens and the `pack.flatwrap.neg.<token>` cases that carry each trigger and its accepted boundary"
  - phase: 15
    provides: "the build-only host shim and the fake frontend, arguments six, seven and eight of the runner-process suite"

provides:
  - "the `## Wrap tool refusals` section of `docs/formats/ivsc-v1.md`: eight line-initial tokens, each with its trigger and the boundary that must still be accepted, mechanically identical to the append-only inventory"
  - "the canonical source-archive sentence in both live documents, with the superseded v0.3.0 sentence quoted and dated rather than deleted"
  - "one committed id, `runner-process.judge.flatwrap-shim`, with five limbs and a committed must-fail stand-in judged by the same comparison"
  - "the `cart-role` registry row, the Phase 17 phase-table block, the EV-08 flip and the thirteen toy lines, in one commit"

affects:
  - "17-03: the count-gate row and lesson 32 land on top of the same registry and the same flip"
  - "Phase 20: recomputes the guardrails' close line, whose inspection limb is deferred here because the tool it names does not exist until Phase 18"

actuals:
  tokens: 17888
  tasks: 3
  commits: 3
  plan_head_before: 67e35a600f9a7e965dfd7c6a76d95c075343b1c2
  plan_head_after: be69a5e6c743cf4f2bf651c4549a1e3f2a09f24f

tech-stack:
  added: []
  patterns:
    - "a document section checked by a `diff` of two `grep '^token-'` outputs, with the stand-in that renames one existing line proving the check can fail"
    - "one canonical sentence used verbatim in three places, counted in each file, with the superseded sentence preserved as a dated quote"
    - "a cross-process digest join whose must-fail stand-in is committed code in the same case, judged by the same function, so the demonstration repeats on every gate run"

key-files:
  created:
    - .planning/phases/17-records-cost-window-and-the-count-gate-contract/17-02-SUMMARY.md
  modified: []

key-decisions:
  - "The eight tokens are documented in the append-only inventory's order and each alone on its line, because the criterion is a `diff` of two `grep '^flatwrap-'` outputs and `diff` compares whole lines; the section sits between `## Refusal reasons` and `## Digests`."
  - "The inventory is documented, never edited: `data/expected/flat-refusals.ids` is byte-identical to its state at the start commit, so the oracle for the new section did not move with it."
  - "One canonical source-archive sentence, verbatim three times: twice in `docs/packaging.md` (the build-only paragraph and the release bullet, both corrected in place) and once in `RELEASE-OPS.md` section (i). The paragraph's object is now an install-archive non-member rather than a release non-member, because the source archive does carry its source."
  - "The superseded v0.3.0 release-decision sentence is quoted and dated beneath the corrected line rather than deleted, and `### (i.1)` — plan 16-04's dated append — is byte-identical to the start commit."
  - "The join is one committed id with five limbs and no typed digest value: the harness's `digest.images` equals the runner's, the runner's `slot.cart.sha256` equals the tool's printed digest, one flipped input byte moves both, and a committed stand-in that replays the first container against the second is refused with `join-images` by the same `shim_join` on every gate run."
  - "One id rather than two: the fifth limb reaches plan 15's two-id property (a stand-in that ignores its input, refused by the same check) at zero extra ids and zero extra process launches, which is what the phase's 0.10 cpu-s line prices."
  - "The `exec: -` assertion the plan states is unattainable with `content exec boot-stub-2`, whose report label is the boot-stub line; the case asserts that label, which says in words that the exec is not an EXEC."
  - "The container gate ran once and agreed: the zero-word container shape is green on hosted Ubuntu, so the research's real-program fallback was not needed."

requirements-completed:
  - EV-08

# Metrics
duration: 96min
completed: 2026-11-26
status: complete
---

# Phase 17 Plan 02: The Refusal Tokens, the Source-Archive Letter and the Join Id Summary

**The eight `flatwrap` refusal tokens documented as a contract that `diff`s clean against the append-only
inventory, the source-archive letter corrected in place in both live documents so they agree sentence for
sentence, and one committed id joining the shim's digest to the runner's — shown able to fail by a committed
stand-in the same comparison refuses — with EV-08 `Complete` in the same commit as its row.**

## Baselines

start-commit: 67e35a600f9a7e965dfd7c6a76d95c075343b1c2

Measured at that sha, before the first edit. Nothing here is carried forward from 17-01's record: every figure is
re-measured at this plan's own start commit and the plan's prediction is compared against the measurement rather
than restated.

```
start-commit: 67e35a600f9a7e965dfd7c6a76d95c075343b1c2
baseline: gate: PASS steps=38/38 controls=404/404 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=118 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=29 provisional=28 handoffs=8 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=695 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2808 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=63 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2808 history-commits=1 gates=119 lines=364 input-rows=31 input-policies=11 input-profile-fields=31 files=2808 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=20/20 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=12/12 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5042 tests-executed=5038 tests-skipped=4 tests-missing=0 docs=64 claim-refs=27 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
baseline-registry-rows: 364
baseline-runner-process-ids: 149
baseline-tracked-files: 2807
```

The gate line above was produced by `sh tools/gate.sh` at that sha, before the first edit, and the other three
figures are the `awk '!/^#/ && NF'` row counts over the two named inventories and the `git ls-files` count at the
same commit. The plan's prediction was `lines=364` and `tests-expected=5042`; both matched, so no figure needed
correcting and none was corrected. `baseline-tracked-files` is `git ls-files` (2807) rather than the gate's
`tracked=2808` token: the gate counts the work tree, which at that moment already held this untracked record.

## The documented refusal tokens

`docs/formats/ivsc-v1.md` gains one section, `## Wrap tool refusals`, between `## Refusal reasons` and
`## Digests`. It lists the eight tokens of `data/expected/flat-refusals.ids` in the inventory's order, each as
the first and only text on its line, and under each token the condition that triggers it and the boundary that
must still be accepted. The two boundaries a reader would otherwise get wrong are stated plainly: 49,152 words
at base `4000` (a 98,368-byte container ending at `FFFF`) is the largest input the address space allows, so a
65,536-word input crosses it first and the count rule is the one check an accepted input cannot reach; and an
output outside the work tree, or any path under `refs/`, must still be accepted however an in-tree path is
spelled.

The closing paragraph carries the three framing facts: these are the tool's tokens rather than the container
format's eighteen numbered refusal reasons; the order is the append-only inventory's, not the tool's own check
order; and a refusal is one stderr line with exit status 1 and no output file, while a file or usage error
carries no token and exit 2.

The mechanical check is a `diff` of two `grep '^flatwrap-'` outputs: eight lines each side, silent. The check is
shown able to fail by the plan's own stand-in — one existing line-initial token renamed in a copy, the count
still eight — which makes the same `diff` non-silent. `data/expected/flat-refusals.ids` is unchanged from the
start commit (`git diff 67e35a6 --numstat` over it is empty), because this phase documents the inventory rather
than editing it.

## The source-archive letter

One canonical sentence is used verbatim in three places:

```
the release ships the static package and the source archive, which carries the source of the build-only tools and of the build-only host shim; no shared object, no built host shim
```

`docs/packaging.md` carries it exactly twice, both corrected in place: the build-only paragraph at line 39, whose
old clause said the shim was "never installed and never shipped, and not a release member", and the
release-content bullet at line 123, whose old lead clause gave the release's contents and then denied a host
shim. The paragraph's object is now an **install-archive** non-member rather than a "release" non-member,
because the source archive does carry its source and the old phrasing would have contradicted the sentence
beside it. Both paragraphs keep their other clauses.

`.planning/ops/RELEASE-OPS.md` section (i) is amended in place: the heading's version token reads `for v0.4.0`
with its parenthetical history untouched, the quoted line is the canonical sentence, and immediately beneath it
the superseded v0.3.0 sentence is quoted with the date (2026-11-26) and the reason it was corrected. One
sentence records the deferral: the inspection-limb clause of the guardrails' close line is not written here
because the tool it names does not exist until Phase 18, and Phase 20 recomputes the line with it. The section's
three numbered mechanisms are unchanged, and `### (i.1)` — the dated append from plan 16-04 — is byte-identical
to its state at the start commit (`cmp` over the slice).

## The join id

`runner-process.judge.flatwrap-shim` sits immediately after `result runner-process.judge.flatwrap-roundtrip` in
`tests/runner-process-tap.sh`, and one line is appended to `data/expected/tests/runner-process.ids` after the
roundtrip id. Both content directories, the flat input, the scenario and every report live under the suite's own
`mktemp` directory, so nothing is written into the work tree.

The case builds one comparison function, `shim_join <harness-out> <runner-out> <tool-digest> <token>`, and judges
five limbs with it. No digest value is typed anywhere: every limb is a relation between values observed in this
case.

1. The tool wraps 8192 zero bytes at base `4000` into content directory A, and the harness (the build-only shim
driven by the fake frontend) digests it. The harness must report a positive frame count before anything is
compared, so a vacuous drive cannot pass.
2. The runner runs the same container through an inline scenario (`id flatwrap-shim`, profile
`intv-2609-ntsc-family-p1`, `content exec boot-stub-2`, `content grom zero-grom`,
`content cart lamp-tone.ivsc`, `run-to 1000 ticks`). The harness's `digest.images` must equal the runner's, and
the runner's `slot.cart.sha256` must equal the digest the tool printed.
3. One input byte is flipped at offset 1 (the first word's high half, still at most `03FF`, so the tool's
word-range rule stays satisfied), wrapped into a **new** path in content directory B (an existing path is refused
by `flatwrap-output-exists`), and both limbs must hold again.
4. Both values must **move** between A and B (`join-immobile`), so an equality that ignored its input cannot pass.
5. A harness report built by this case's own lines from the **first** container's `frames` and `digest.` lines is
judged by the same `shim_join` against the **second** container's real runner report and the second container's
tool digest. It must be refused with `join-images`; an acceptance records `judge-vacuous` and fails the id. The
judgement runs in a subshell with its own why target, so its expected finding cannot set the case's verdict.

The `exec:` line is asserted to be the boot-stub label, which says in words that it is not an EXEC, so no line can
read as a real-EXEC result. Missing any of the three arguments is a `not ok` with its own reason and never a skip:
`tests-skipped` stays 4 and `tests-missing` stays 0.

One id, not two: plan 15's precedent for the same kind of join minted two extra committed ids (`host-vacuous`,
`host-digest-diff`), and this phase's locked decision is one id at 0.10 cpu-s. The fifth limb reaches the same
property - a stand-in that ignores its input, committed in the same commit, refused by the same check - at zero
extra ids and zero extra process launches, and it is re-demonstrated on every gate run rather than once. The
reasoning is in the case's comment block so a later reader does not re-open it.

The observed values, read from this run rather than typed into the case (container A is 8192 zero bytes, container
B has the byte at offset 1 flipped):

```
tool digest A  8d73b98c58bb75a6515c6a791ebd2925265d05874e324c31a5836e61055b74f5
tool digest B  b6bc201272c1a65f882b7896433d8766b6e2c51ee01064d05c736076cae51ddf
digest.images A  f5e8555f96ebcf8a23c697e889886903f47f13233004ee92c934181fe05e61bf
digest.images B  693176c0513294574dd47ef83154749b7962b7f7ff75883a0de16855ad7aff3e
frames (both)  13
stand-in refusal token  join-images
```

## The row, the phase block and the flip

`data/expected/important-gates.ids` gains one row, `cart-role mutant runner-process.judge.flatwrap-shim EV-08`,
with a comment naming the join it proves and stating that the same-input-twice stand-in is the case's own second
container. The gate name is the existing `cart-role`, so no new gate name is minted. The step's phase table gains
a Phase 17 block in its existing continuation shape, naming EV-08, plan 17-02, the reused gate name, the fact that
the case is a new id in an existing suite rather than a new step, and "zero new faults, zero new steps".

`.planning/REQUIREMENTS.md` flips in the same commit: the checkbox is ticked, the traceability row reads
`Complete`, and the `Last updated` line moves. The Coverage block already read five of five mapped and does not
change. The ten whole-file toy registries and the three toy requirement records each gain their `EV-97` line
**appended as the last line** and never inserted, because two controls pin a line number in an overlay registry
(`line 19` and `line 24`) and an inserted line would shift the pinned number. Before the first append
`grep -rn -- 'EV-97' tools/check/controls/` printed nothing, so the id was free; after the appends exactly
thirteen files carry it and each carries it as its last line.

## Gate evidence

| run | summary line |
|---|---|
| `sh tools/gate.sh` at the start commit (baseline) | `gate: PASS … lines=364 … tests-expected=5042 tests-executed=5038` |
| `sh tools/gate.sh`, after Task 3 | `gate: PASS steps=38/38 controls=404/404 … gates=119 lines=365 … tests-expected=5044 tests-executed=5040 tests-skipped=4 tests-missing=0 … docs=64` |
| `dash tools/gate.sh`, after Task 3 | the same line, byte for byte |
| `sh tools/gate-linux.sh` (Ubuntu container, `cc=GNU`) | `gate-linux: exit 0`, `gate: PASS … lines=365 … tests-expected=5044 tests-executed=5040 tests-skipped=4 tests-missing=0`, `gate-time total=255` |

The container gate ran once, after `build/gate-linux.log` was copied aside first. It agrees with the case: the
zero-word container shape is green on hosted Ubuntu as well, so the plan's A1 fallback (the real-program shape of
the research's Code Example 3) was not needed. The container line's platform tokens are the known ones
(`cc=GNU`, `core-imports=5`, `archive=sha256`, `refs-verified=0/4`); every token this plan predicts is identical.

The step ran directly as well: the real tree prints `-- important-gates pass gates=119 lines=365` with no
`-- FAIL important-gates-` line, the toy tree prints `-- important-gates pass gates=49 lines=52`, and
`sh tools/check/run-controls.sh` prints `controls=404/404` with none of the seven named findings. Both pinned
`EXPECT` files still read `FAIL important-gates-unknown-id: line 19` and `… line 24`.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] The plan's `exec: -` assertion is unattainable with the shape the plan requires**

- **Found during:** Task 3, first run of the new case.
- **Issue:** the plan and its acceptance criteria ask the case to assert `exec: -` in the report. The report's
exec label is derived from the symbolic content name only (D-30, `runner/report.c`): a plain-file exec prints
`exec: -`, but `content exec boot-stub-2` — which is what makes the runner's `slot.exec` a boot stub and its
`digest.images` comparable to the harness's — prints
`exec: project boot stub contract 2, not an EXEC sha256=<64 hex>`. Asserting `exec: -` would fail every run.
- **Fix:** the case asserts the boot-stub label with `grep -F 'exec: project boot stub contract 2, not an EXEC
sha256='`, which is strictly stronger than `exec: -` for the property the plan states: the line says in words
that the exec is not an EXEC, so no line can read as a real-EXEC result.
- **Files modified:** `tests/runner-process-tap.sh`.
- **Commit:** this task's commit.

**2. [Rule 3 - Blocking] `sh tools/hygiene.sh --tree` prints its pass line rather than being silent**

- **Found during:** Task 1's verify.
- **Issue:** the plan's `fails_when` says the hygiene command must print nothing. It prints
`-- hygiene pass mode=tree hygiene-files=<n>` on success; the gate's own `hygiene-files=` token is that line.
The `silent` phrasing is a guardrails-text artifact, not this plan's change.
- **Fix:** the intent — no finding — is asserted instead: the command exits 0 and prints no finding line. The
gate, which runs the same command as a step, is green under both shells.
- **Files modified:** none.
- **Commit:** n/a (no code change).

### Recorded, not auto-fixed

**3. [Rule 4 - scope] The plan commits on `main`; the harness's protected-branch assertion is inapplicable**

- **Found during:** the first commit.
- **Issue:** the harness's pre-commit assertion refuses a commit whose branch
`gsd_run query git.base-branch --is-protected` calls protected, and it calls `main` protected. This project's
`git.branching_strategy` is `none` and the orchestrator dispatched this plan as a sequential executor on the main
working tree, which is where every prior phase of this milestone committed.
- **Fix:** none attempted. Editing `.planning/config.json` to add `allow_default_branch_commits` would have
modified orchestrator state this plan may not stage. The commits are on `main`, as the project does, and the
condition is recorded here rather than silently ignored or worked around.
- **Files modified:** none.
- **Commit:** n/a.

### Notes on the plan's figures

- The baseline gate line's `tracked=2808` is one more than `git ls-files` (2807), because the gate counts the work
tree and this plan's own record was already present, untracked, when the baseline ran. The recorded
`baseline-tracked-files:` is the `git ls-files` figure the plan asks for, and every verify pins it to the tree at
the start commit.
- The flipped-byte values differ from the research's table because the research flipped offset 8191 and this case
flips offset 1, as the plan states. Both limbs still move; no value is typed into the case either way.
- `docs=64` did not move: the new section is inside an existing page, not a new file.

## Open items

None. No stub, skipped test or unrun verify was left behind: the container gate ran and passed, and the case is
`ok` in both presets with `tests-missing=0`.

## Self-Check: PASSED

- Every file this plan claims to have created or modified exists: the format page, the packaging page,
  `RELEASE-OPS.md`, the suite, both inventories, the step, `REQUIREMENTS.md` and this record.
- All three task commits are ancestors of `HEAD`: `4e0c894` (the tokens), `135bfa5` (the letter), `be69a5e`
  (the id, the row and the flip).
- `commits:` is measured, not narrated: `git rev-list --count 67e35a6..HEAD` at this record's write reads 3, and
  `plan_head_before` / `plan_head_after` are the two shas of that range.
- The final gate line, under both shells and in the Ubuntu container, carries `lines=365`, `gates=119`,
  `tests-expected=5044`, `tests-executed=5040`, `tests-skipped=4` and `tests-missing=0`.
