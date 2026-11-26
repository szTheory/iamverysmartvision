---
phase: 16-owner-route-claim-and-release-posture
plan: 03
subsystem: handoff-ledger
tags: [handoffs, owner-only-questions, ledger-pins, packet, stat-not-check, ledger-copies]

requires:
  - phase: 16-02
    provides: "the owner route as a rehearsal with a transcript (`docs/owner-images.md`'s three commands, `.planning/ops/RELEASE-OPS.md` section (j)), which `H16-2` and `H16-3` point at"
  - phase: 15-build-only-host-and-the-fake-frontend-harness
    provides: "the build-only shared object (`ivs_retro_shim`, output name `ivs_retro`) and the fake frontend harness whose nine digest lines `H16-1` compares against"
  - phase: 10-claim-discipline-and-the-evidence-ceiling
    provides: "the claim hook's artifact refusal, which already lists `data/expected/handoffs.ids`"
  - phase: 16-01
    provides: "the tree this plan started from (`ca76af3`) and every token of the summary line"

provides:
  - "`H16-1`, `H16-2` and `H16-3` in `data/expected/handoffs.ids`, each `handed-off evidence: none`, beside the five existing rows"
  - "the claims step's pass line carries `handoffs=<n>`, counted from the ledger on every run"
  - "both pinned lists in `tools/check/steps/claims.sh` name the eight ids and their dispositions, so a missing, extra, duplicate or re-dispositioned row is a named finding in both directions"
  - "three packet sections in `docs/handoffs.md`, each with a command, an artefact, the deterministic identities a person compares and what a person judges"

affects:
  - "16-04: the release-decision line may cite a row id but must not read a row as a result; PKG-06 is that plan's tick"
  - "16-05: the close reads `handoffs=8` from the summary line rather than quoting a row count"
  - "every later milestone: opens the packet and cites a row instead of re-asking the owner"

actuals:
  tokens: 11500
  tasks: 2
  commits: 2
  plan_head_before: ca76af32f9ab48149b9d14463dc0b44f8ee5ff47
  plan_head_after: 43b4394

tech-stack:
  added: []
  patterns:
    - "a row count printed as a stat on an existing pass line rather than quoted in prose, so the figure is the file's at every commit"
    - "an inventory's id set and each disposition pinned in the step that reads it, in both directions, with the copies moved in one commit"
    - "a handoff packet section shaped as command, artefact, deterministic identity, judgement, so a person performs the act without rebuilding the procedure"

key-files:
  created: []
  modified:
    - tools/check/steps/claims.sh
    - data/expected/handoffs.ids
    - docs/handoffs.md
    - tools/check/controls/_good/tree/data/expected/handoffs.ids
    - tools/check/controls/claims-handoff-id-missing/overlay/data/expected/handoffs.ids
    - tools/check/controls/claims-handoff-id-duplicate/overlay/data/expected/handoffs.ids
    - tools/check/controls/claims-handoff-id-extra/overlay/data/expected/handoffs.ids
    - tools/check/controls/claims-handoff-disposition/overlay/data/expected/handoffs.ids

key-decisions:
  - "The stat is appended by the final `printf`, not inserted into the string captured from the hook, so the hook's own token order (`claims=`, `provisional=`) stays byte-identical and `handoffs=` lands immediately after them in the summary line."
  - "The count is read with the grammar pass's own row predicate (a non-comment line with fields) from `$hled` on every run; the verify asserts the printed value equals the file's own row count, which is the only thing a hardcoded literal could not satisfy once the file moves."
  - "The stat has no control on purpose: the mechanism that keeps the ledger honest is the pair of pinned lists (four named findings in both directions), and a control of the count would prove the count rather than the ledger — GUARDRAILS 2.4 asks a control only for a new check."
  - "The three rows were appended after `D10`/`D11` and the ledger's header sentence about five ids was kept as a pointer, with one sentence naming the Phase 16 additions: section 2.10 asks a live record for an inline pointer, never a silent rewrite."
  - "The four `claims-handoff-*` overlays and the good tree's ledger moved in the same commit as the rows and the pins, because the id-set pass fails in both directions and each overlay's exact `EXPECT`/`FORBID` pair would otherwise fire the wrong finding; `claims-handoff-row` was left untouched (its finding is a grammar line, and the id-set pass never runs for it)."
  - "The packet's `H16-1` command is the release preset's object with the harness from `build/release/tests/`, and it was run before being written down: nine `digest.*` lines and exit 0, so the packet quotes a command a person can perform rather than a path that looks right."

patterns-established:
  - "A measured count replaces a quoted one: the inventory's size is printed by the step that reads it, and no record restates it."
  - "A packet section ends with what a person judges, and the page says plainly that the project states nothing about the thing judged."

requirements-completed: []

# Coverage metadata (#1602)
coverage:
  - id: D1
    description: "Three owner-only questions exist as ledger rows, `handed-off evidence: none`, cited by no claim and by no phase exit criterion"
    requirement: PKG-06
    verification:
      - kind: automated
        ref: "sh tools/check/steps/claims.sh (pass line `claims=29 provisional=28 handoffs=8`, no `-- FAIL`)"
        status: pass
      - kind: automated
        ref: "sh tools/gate.sh at 43b4394 (`handoffs=8`, every other token unchanged, `controls=404/404`)"
        status: pass
    human_judgment: false
  - id: D2
    description: "The ledger's row count is measured on every gate run instead of quoted"
    requirement: PKG-06
    verification:
      - kind: automated
        ref: "the equality of `handoffs=` with `awk '!/^#/ && NF' data/expected/handoffs.ids | wc -l` in both task verifies"
        status: pass
    human_judgment: false
  - id: D3
    description: "Each row has a packet section with a command, an artefact, the deterministic identities and what a person judges"
    requirement: PKG-06
    verification:
      - kind: other
        ref: "docs/handoffs.md sections H16-1, H16-2, H16-3; the harness and wrap commands were run before being written"
        status: pass
    human_judgment: false

metrics:
  duration: 30min
  completed: 2026-11-26
  status: complete
---

# Phase 16 Plan 03: The Three Owner-Only Questions as Handoff Rows Summary

**Three owner-only questions became `H16-1`/`H16-2`/`H16-3` ledger rows with real packet procedures, the id set and each disposition pinned in both directions, and the ledger's row count printed as `handoffs=8` on the claims pass line.**

## Performance

- **Duration:** 30 min (04:45 to 05:15 local; three gate runs of 102, 285 and 267 s)
- **Started:** 2026-11-26T09:45:00Z (baseline gate at `ca76af3`)
- **Completed:** 2026-11-26T10:15:00Z
- **Tasks:** 2
- **Files modified:** 8 (all existing; nothing created)

## Accomplishments

- `data/expected/handoffs.ids` holds eight rows: `H16-1`, `H16-2` and `H16-3` are `handed-off evidence: none`, and `D10`/`D11` stay `not-performed`. The header's sentence about five ids stayed as written and gained one pointer sentence naming the Phase 16 additions, so the live record was pointed at rather than rewritten (section 2.10).
- The claims step prints `-- claims pass claims=29 provisional=28 handoffs=8`: the count is read from the file with the grammar pass's own row predicate on every run, so no record has to quote a figure that can go stale. Its header states why that is a stat and not a check, and why it adds no control.
- Both pins moved together: `HANDOFF_IDS` names eight ids and `HANDOFF_DISP` names each disposition, so a missing row, an extra row, a duplicate and a flipped disposition are four named findings in both directions — the mechanism the count is a stat *about*.
- `docs/handoffs.md` gained one section per row in the page's shape: the build of `ivs_retro_shim` (output name `ivs_retro`), the object's digest and size, and the harness command whose nine `digest.*` lines the project itself prints (`H16-1`); the three-command owner route cited together with `.planning/ops/RELEASE-OPS.md` section (j), with the run's report as the identity and no derived file of owner content in the tree (`H16-2`); the wrap run twice at `hi-first` and `lo-first` into two containers with the loader lines compared (`H16-3`). A closing paragraph restates that a row is never a pass, never evidence and never a claim's support.
- Every ledger copy moved in one commit: the good tree's ledger and the four `claims-handoff-*` overlays each gained the three rows while keeping exactly its own intended finding, and `claims-handoff-row`, `-missing` and `-unreadable` were left untouched. `controls=404/404` with no toy-drift or inventory finding.

## Task Commits

Each task was committed atomically:

1. **Task 1: the `handoffs=` stat — the ledger's row count on the pass line** — `4ab5388` (feat)
2. **Task 2: three rows, three packet sections, five ledger copies and the two pins** — `43b4394` (feat)

**Plan metadata:** the `docs(16-03): complete …` commit that adds this file, `.planning/STATE.md`, `.planning/ROADMAP.md` and `.planning/WINDOWS.md`.

## Files Created/Modified

- `tools/check/steps/claims.sh` — the `handoffs=<n>` stat appended after the hook's stats, the header paragraph that says why it is a stat and not a check, and both pinned lists extended to eight ids with the comment naming the phase that added the three.
- `data/expected/handoffs.ids` — three rows after `D11`, and one pointer sentence in the header naming them.
- `docs/handoffs.md` — three sections (`H16-1`, `H16-2`, `H16-3`), one intro pointer sentence, and the closing "These rows are never a result" paragraph; no row count anywhere in the page.
- `tools/check/controls/_good/tree/data/expected/handoffs.ids` — the good tree carries the three rows, so a good tree still satisfies the eight-id pin.
- `tools/check/controls/claims-handoff-id-missing/overlay/…` — the three rows present, `D11` still the one missing id.
- `tools/check/controls/claims-handoff-id-duplicate/overlay/…` — the three rows present, the duplicated `D11` still the one finding (the duplicate line is skipped before the disposition rule, so its `handed-off` does not also fire).
- `tools/check/controls/claims-handoff-id-extra/overlay/…` — the three rows present, `X-1` still the one unknown id.
- `tools/check/controls/claims-handoff-disposition/overlay/…` — the three rows present with their pinned disposition, the wrong `D10` still the one finding.

## Decisions Made

See `key-decisions` in the frontmatter. The two that a later editor is most likely to re-open: the stat is deliberately uncontrolled (the pins are the mechanism), and the header sentence about five ids was pointed at rather than rewritten.

## Invariants, re-measured

The gate ran three times in this plan: the baseline at `ca76af3`, task 1's at `4ab5388`, and task 2's at `43b4394`. Baseline and task 1:

```
gate: PASS steps=38/38 controls=404/404 … claims=29 provisional=28 handoffs=5 cp1610-rows=74 …
gate: PASS steps=38/38 controls=404/404 … claims=29 provisional=28 handoffs=8 cp1610-rows=74 …
```

A token-by-token `diff` of the two summary lines prints exactly one changed line, `handoffs=5` → `handoffs=8`, and nothing else:

```
21c21
< handoffs=5
---
> handoffs=8
```

Every invariant the plan predicted held: `steps=38/38 controls=404/404 faults=238/238 goldens=29 gates=118
lines=361 prefix-files=11 pkg-selftest=20/20 release-selftest=12/12 claims=29 provisional=28 claim-refs=27
binaries-listed=0 tests-expected=5042 tests-executed=5038`, with `tracked=2785 files=2785
hygiene-files=2785` — unchanged, because all eight paths already existed. No gate step, control, fault,
golden, registry row, claim or test id was added, and the ledger's growth is the only movement.

The metadata commit that adds this SUMMARY moves `tracked`/`files`/`hygiene-files` by exactly `+1`
(2785 → 2786) and nothing else, for the same reason every earlier phase's completion commit did: this file
is new and `gate.sh`'s `FILELIST` counts untracked-not-ignored paths.

## Deviations from Plan

### Plan defects (recorded with the measured reason, not silently satisfied)

**1. [Plan defect] Task 2's verify requires no `H16-` id in `.planning/ROADMAP.md`, and the roadmap has named
all three since planning**
- **Found during:** Task 2, the citation check at the end of the `<automated>` block.
- **Measured:** `grep -rn -e 'H16-' .planning/ROADMAP.md` prints line 296, `- [ ] 16-03-PLAN.md — the three
  handoff rows (`H16-1`, `H16-2`, `H16-3`) …`, and that line is present at the plan's own starting commit
  (`git show ca76af3:.planning/ROADMAP.md`), so no change of this plan's could make the grep empty. The
  same grep over `.planning/REQUIREMENTS.md`, `data/claims.json` and `CHANGELOG.md` is empty, as it was at
  `ca76af3`.
- **Substitution:** the substantive check was run and is clean — no `H16-` id in
  `.planning/REQUIREMENTS.md`, `data/claims.json` or `CHANGELOG.md`, and no `H16-` id on any ROADMAP line
  other than the plan-inventory entry that names the ids as rows *to add*. A plan-inventory line is not a
  citation of a result and not an exit criterion, which is what V8 forbids; nothing was edited to silence
  the grep, because editing the roadmap's plan list would misreport what the plan is.
- **Ledger:** `.planning/WINDOWS.md` row 10, kind `deviation`, waived with this reason, so a later ship gate
  sees it without reading this SUMMARY.

### Not this plan's to do, by name

- **PKG-06 is not ticked.** The plan's frontmatter names it, but the phase's artifact table assigns
  `REQUIREMENTS.md` (PKG-06 `Complete`) to **16-04**, so `requirements-completed` is empty.
- **No `docs/RIGHTS.md` move.** No third-party item is read or named: the rows name owner-held material by
  procedure only, and the packet says no digest of any owner image is recorded.
- **`D10`/`D11` stay `not-performed`** and their rows were not edited; `D10`'s inventory
  (`data/expected/d10-measurements.ids`) was not touched.
- **Nothing was pushed, tagged or published**, and no `.planning/config.json` or `.planning/state.json` was
  staged (D-31, D-42).

## Issues Encountered

- The two wrap commands in `H16-3` were run before being written down, and on a zero-filled 8,192-byte flat
  image the `hi-first` and `lo-first` containers are byte-identical (both digests `8d73b98c…`): zeros are
  the one input where the two word orders cannot differ. That is a property of the test file, not of the
  option — the packet claims only that each wrap prints its container's digest and that a person compares
  the two loader lines, which is true for an owner's real image and false to assert about zeros.
- The wrap tool takes `--word-order` *after* its three positional arguments (`ivs_flatwrap <flat> <base>
  <out> [--word-order hi-first|lo-first]`), so the packet's second command puts the option last; the first
  draft had it first and would have printed the usage line instead of a digest.
- The harness lives at `build/release/tests/ivs_host_harness`, not under a `tests/host/` build directory
  (the source is `tests/host/ivs_host_harness.c`, the target belongs to `tests/CMakeLists.txt`), and the
  release shim needs no sanitizer runtime, so the packet's command is exactly the one that was run.

## Known Stubs

None. No `<verify>` was skipped, no test was left unwired, and the two new printed values (the row count and
the packet's commands) were measured rather than typed.

## Threat Flags

None. The change adds no network surface, no executable input and no tracked artifact: it edits an
inventory, a packet and a shell script, and the register's T-16-10 to T-16-12 are each exercised above — the
pins name every finding in both directions, every row is `evidence: none` and the packet states in words that
an answer is a result and never a tier, and the six ledger copies moved in one commit with `controls=404/404`.

## User Setup Required

None — no external service, credential or manual step.

## Next Phase Readiness

- 16-04 may cite a row id when it says what the release does not ship, but it must not read a row as a
  result and must not tick PKG-06 on the strength of one: the rows are offered, not performed.
- 16-05's close should read `handoffs=8` from the summary line rather than quoting a row count; the ledger
  is the inventory and the pass line is the arithmetic.
- The owner's answer to any of the three arrives as a handoff result: the row's disposition changes, the last
  two tokens stay `evidence: none`, and a claim that wants to rest on the result is a separate decision.

## Self-Check

- `tools/check/steps/claims.sh` — FOUND (stat and pins)
- `data/expected/handoffs.ids` — FOUND, eight rows, `H16-1`/`H16-2`/`H16-3` present
- `docs/handoffs.md` — FOUND, sections `H16-1`, `H16-2`, `H16-3`
- the five ledger copies — FOUND, each with the three rows
- commit `4ab5388` — ancestor of HEAD
- commit `43b4394` — ancestor of HEAD

## Self-Check: PASSED
