---
phase: 13-rights-ledger-and-enforced-claim-ceiling
verified: 2026-11-26T00:20:40Z
status: passed
score: 18/18 must-haves verified
covered_files: [".planning/phases/13-rights-ledger-and-enforced-claim-ceiling/13-01-PLAN.md", ".planning/phases/13-rights-ledger-and-enforced-claim-ceiling/13-01-SUMMARY.md", ".planning/phases/13-rights-ledger-and-enforced-claim-ceiling/13-02-PLAN.md", ".planning/phases/13-rights-ledger-and-enforced-claim-ceiling/13-02-SUMMARY.md", ".planning/phases/13-rights-ledger-and-enforced-claim-ceiling/13-03-PLAN.md", ".planning/phases/13-rights-ledger-and-enforced-claim-ceiling/13-03-SUMMARY.md", ".planning/phases/13-rights-ledger-and-enforced-claim-ceiling/13-04-PLAN.md", ".planning/phases/13-rights-ledger-and-enforced-claim-ceiling/13-04-SUMMARY.md", ".planning/phases/13-rights-ledger-and-enforced-claim-ceiling/13-05-PLAN.md", ".planning/phases/13-rights-ledger-and-enforced-claim-ceiling/13-05-SUMMARY.md", "AGENTS.md", "data/expected/controls/claims.ids", "data/expected/controls/important-gates.ids", "data/expected/controls/rights.ids", "data/expected/important-gates.ids", "data/expected/rights-pinned.ids", "data/expected/rights.ids", "data/expected/runner-selftest.ids", "data/expected/sources.ids", "data/rights/bank-modes.json", "data/rights/cfg-container.json", "data/rights/commercial-rom.json", "data/rights/core-info-strings.json", "data/rights/exec-image.json", "data/rights/flat-cartridge-image.json", "data/rights/grom-image.json", "data/rights/libretro-abi.json", "data/rights/rendered-frame.json", "data/rights/save-state.json", "data/sources.json", "docs/RIGHTS.md", "tools/check/controls/_good/tree/.planning/REQUIREMENTS.md", "tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md", "tools/check/controls/_good/tree/data/expected/important-gates.ids", "tools/check/controls/_good/tree/data/expected/rights-pinned.ids", "tools/check/controls/_good/tree/data/expected/rights.ids", "tools/check/controls/_good/tree/data/rights/toy-owner-image.json", "tools/check/controls/_good/tree/data/rights/toy-reference-doc.json", "tools/check/controls/_good/tree/docs/RIGHTS.md", "tools/check/controls/claim-established-non-source/CHECK", "tools/check/controls/claim-established-non-source/EXPECT", "tools/check/controls/claim-established-non-source/overlay/data/claims.json", "tools/check/controls/claim-established-positive/CHECK", "tools/check/controls/claim-established-positive/EXPECT", "tools/check/controls/claim-established-positive/FORBID", "tools/check/controls/claim-established-positive/overlay/data/claims.json", "tools/check/controls/claim-established-without-tier1/CHECK", "tools/check/controls/claim-established-without-tier1/EXPECT", "tools/check/controls/claim-established-without-tier1/overlay/data/claims.json", "tools/check/controls/glossary-no-files/MAKE", "tools/check/controls/important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-bad-req-phase6/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md", "tools/check/controls/important-gates-excluded-stale/CHECK", "tools/check/controls/important-gates-excluded-stale/EXPECT", "tools/check/controls/important-gates-excluded-stale/FORBID", "tools/check/controls/important-gates-excluded-stale/overlay/data/expected/important-gates-excluded.ids", "tools/check/controls/important-gates-req-uncovered-phase5/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-req-uncovered-phase6/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-req-uncovered-phase7/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-uncovered-gate/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-unknown-id-phase5/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-unknown-id-phase6/overlay/data/expected/important-gates.ids", "tools/check/controls/important-gates-unknown-id/overlay/data/expected/important-gates.ids", "tools/check/controls/rights-ledger/CHECK", "tools/check/controls/rights-ledger/EXPECT", "tools/check/controls/rights-ledger/FORBID", "tools/check/controls/rights-ledger/overlay/data/expected/rights.ids", "tools/check/controls/rights-ledger/overlay/data/rights/toy-blank.json", "tools/check/controls/rights-ledger/overlay/data/rights/toy-cleared.json", "tools/check/controls/rights-ledger/overlay/data/rights/toy-owner-image.json", "tools/check/controls/rights-ledger/overlay/data/rights/toy-stray.json", "tools/check/controls/rights-ledger/overlay/data/rights/toy_stray.json", "tools/check/controls/rights-ledger/overlay/docs/RIGHTS.md", "tools/check/controls/rights-naming/CHECK", "tools/check/controls/rights-naming/EXPECT", "tools/check/controls/rights-naming/FORBID", "tools/check/controls/rights-naming/overlay/core/src/common/planted_pinned.h", "tools/check/controls/rights-naming/overlay/data/expected/rights-pinned.ids", "tools/check/controls/rights-naming/overlay/runner/planted_open.c", "tools/check/controls/sources-no-files/MAKE", "tools/check/controls/text-no-files/MAKE", "tools/check/files.cmake", "tools/check/kinds/claim.cmake", "tools/check/manifests.cmake", "tools/check/rights-lib.cmake", "tools/check/selftest-inventory.sh", "tools/check/steps/important-gates.sh", "tools/check/steps/runner-selftest.sh"]
covered_digest: "v4:sha256:bd55f1704f9cf5389383e1b8be84dc5a3e1130d932a6e6e86a7c8a3aedcbe5dc"
behavior_unverified: 0
overrides_applied: 0
re_verification:
  previous_status: passed
  previous_score: 18/18
  gaps_closed:
    - "STATE.md:584's enumerated last-five `cpu_s` window was two rows stale — closed: the checkpoint bullet now carries a final recompute (`357 360 363 368 438` → median 363; `gate_s` `119 120 121 122 135` → median 121) that matches this verifier's arithmetic exactly, with the superseded paragraph kept intact per GUARDRAILS 2.10"
    - "STATE.md:43's hosted-row prose read `not-recorded` while `.planning/ci-runs.tsv` carried three hosted rows — closed for the three `pull_request` rows (`45717868904`, `45718553735`, `45720934719`), which this verifier confirmed live with `gh run view`; a new contradiction on the merge `push` row is recorded as a warning below"
    - "STATE.md:631's D-09-06 Deferred row read as still open after Phase 13 discharged it — closed: inline pointer `discharged by Phase 13 plan 13-04` at the original sentence, original text intact, plus an appended amendment at `:645` naming the ten rows and `gates=114 lines=352`"
    - "`rights-blank` (criterion 1's blank-cell limb) had no committed control — closed: `tools/check/controls/rights-ledger/overlay/data/rights/toy-blank.json` (`taken: \"   \"`), its id in the overlay's `data/expected/rights.ids`, its `item` phrase in the overlay's `docs/RIGHTS.md`, and a sixth `EXPECT` line; the control now fails for exactly six intended reasons, observed this session"
  gaps_remaining: []
  regressions: []
---

# Phase 13: Rights Ledger and Enforced Claim Ceiling Verification Report

**Phase Goal:** Every third-party-content question this milestone touches has a written
disposition before any byte is read, a refused item cannot be named in code or in an operator
page without failing the gate, and the claim ceiling is mechanical rather than quoted.
**Verified:** 2026-11-26T00:20:40Z
**Status:** passed
**Re-verification:** Yes — after `962deaf` ("close the four verification warnings")

**Method.** Nothing here is taken from a SUMMARY or from the previous report. Every number is a
command run in this session against the tree, or a `file:line`. The full gate was re-run under
`sh` (`gate-time total=90`), under `dash` (tail line `cmp`-identical) and inside the Ubuntu 24.04
container (`gate-time total=320`, `gate-linux: exit 0`); each mechanism was observed firing, not
read about. The four hosted rows were confirmed live with `gh run view`, not accepted from the
TSV. Scratch mutations were made on `/tmp` copies of `tools/check/controls/_good/tree` and removed
afterwards; no source file was modified and nothing was committed.

**Tree state at verification.** `HEAD` is `962deaf`. Two planning files carry uncommitted edits
from the phase close — `.planning/STATE.md` (the final recompute paragraph) and
`.planning/ci-runs.tsv` (the merge `push` row `45723269901`) — plus this report as an untracked
file. Because the medians are read from that TSV, both readings were computed: the worktree's last
five successful `cpu_s` are `357 360 363 438 368` (median **363**) and `HEAD`'s are
`497 357 360 363 438` (median **363**); `gate_s` is `119 120 121 122 135` and `148 121 120 119 135`
respectively, median **121** in both. The verdict and the margin do not depend on the uncommitted
row.

## Goal Achievement

### Observable Truths

The first five rows are the ROADMAP's five success criteria — the contract, verified one at a
time. Rows 6-18 are the plans' `must_haves` truths, deduplicated against the criteria.

| #   | Truth   | Status     | Evidence       |
| --- | ------- | ---------- | -------------- |
| 1 | **SC1** `data/rights/` holds one row per item the milestone touches (flat image, `.cfg`, bank modes, EXEC, GROM, commercial ROM, frame/screenshot, save state, libretro-shaped ABI, core-info strings), each `cleared`/`refused`/`owner-local-only`, with a `SRC-` id resolvable in `data/expected/sources.ids` when `cleared` and a non-empty "what was taken" cell; a row missing any of those is refused by name | ✓ VERIFIED | `ls data/rights/*.json` → exactly 10 files matching the ten questions; `cat data/expected/rights.ids` → the same 10 ids, set-equal both ways (`rights-id-missing`/`rights-id-extra`, `rights-lib.cmake:249-259`); gate prints `rights=10 rights-cleared=1`. Dispositions read off the ten files this session: 1 `cleared` (`libretro-abi`), 4 `refused` (`bank-modes`, `cfg-container`, `commercial-rom`, `rendered-frame`), 5 `owner-local-only`. `data/rights/libretro-abi.json` is the only `cleared` row and cites `SRC-libretro-api-docs`, present at `data/expected/sources.ids:14`. `taken` is non-empty on all ten (`"nothing"` is the legal value). All three limbs now have a **committed** stand-in in one control: `rights-missing-key`, `rights-source-missing` and `rights-blank` all appear in `tools/check/controls/rights-ledger/EXPECT`, and this session's replay of that overlay produced exactly `rights-unlisted`, `rights-blank: toy-blank/taken`, `rights-source-missing: toy-cleared`, `rights-missing-key: toy-owner-image/disposition`, `rights-id-extra: toy-stray`, `rights-doc-row: toy-cleared` — six findings, one per intended reason, nothing co-firing |
| 2 | **SC2** A `cleared` row with no source is refused, and a refused item's spellings are refused in `core/src`, `runner/`, `tools/` and the operator pages of `docs/`, except where a pre-existing refusal surface is pinned by a `(token, path)` pair; both limbs are shown able to fail by a committed control | ✓ VERIFIED | Source limb `rights-lib.cmake:221-235` (`rights-source-missing` at `:227`, `rights-source-unresolved` at `:232`), observed in the committed control. Naming limb `rights_scan_naming()` at `rights-lib.cmake:352-441`: scope is `^(core/src/\|runner/\|tools/)` minus `^tools/check/controls/` (`:372`), plus six **exact** pages from `set(pages …)` at `:353-354`, never a glob. Both limbs fail in one committed control: `tools/check/controls/rights-naming/EXPECT` carries `FAIL rights-token: toy-image runner/planted_open.c` **and** `FAIL rights-pinned-stale: grom-image tools/nowhere.sh`, while its `FORBID` names `rights-token: toy-image core/src/common/planted_pinned.h` (the suppressed pinned hit) plus `rights-pinned-malformed` and `rights-pinned-inventory-missing`; gate prints `control ok   rights-naming`. **Live this session**: `toy-image` planted in `runner/b.c` on a `/tmp` toy tree → `-- FAIL rights-token: toy-image runner/b.c`, and the pristine toy tree → `-- files pass tracked=129 binaries-listed=0 rights-pinned=0/0` |
| 3 | **SC3** The claims step refuses an `established` claim that is not a `source-expectation` claim resting on a `tier1-primary-doc` dependency, and a legal `established` claim still passes; both are shown able to fail | ✓ VERIFIED | Condition (a) `tools/check/kinds/claim.cmake:107` (`gate_fail(claim-established-non-source …)`), placed before the artifact check and before every early `return()`; condition (b) `:186` (`gate_fail(claim-established-without-tier1 …)`), inside the `depends_on` loop so no early exit can skip it. Two must-fail controls (`EXPECT` = `FAIL claim-established-non-source: CL-0001` and `FAIL claim-established-without-tier1: CL-0001/model`) and one legal positive whose `EXPECT` names an unrelated refusal (`claim-dangling-path: CL-0001`) and whose `FORBID` names both new tokens. All three print `control ok` inside `controls=401/401` |
| 4 | **SC4** Each of the ten D-09-06 mechanisms has an `important-gates.ids` row over its existing selftest, the registry baseline is re-derived in the same commit, all ten toy registries gain exactly one line each with zero deletions, and `faults` stays 238. **Amended by plan 13-04 to six `control` + four `mutant` rows** | ✓ VERIFIED (amended, see below) | `grep -c '^d09-06-registration' data/expected/important-gates.ids` → **10**, at `:525-534`: 6 `control` (`important-gates-excluded-stale`, `-complete-uncovered`, `-excluded-malformed`, `sources-src-token-whole`, `manifests-output-anchor`, `memmap-generator-refused`) and 4 `mutant` (`st-toy-drift`, `st-deviation-unused`, `st-pass-line`, `st-memmap-generator-missing`). Every one maps 1:1 onto the canonical ten of `09-05-SUMMARY.md`. Six control directories exist and print `control ok`; the four mutant names are in `data/expected/runner-selftest.ids` and the step prints `-- runner-selftest pass cases=14`. Baseline re-derived by running the step: `-- important-gates pass gates=114 lines=352`, and `awk '!/^#/ && NF' data/expected/important-gates.ids \| wc -l` → 352. `git show --numstat 2c32390` → exactly 13 files at `+1 -0` (ten whole-file toy registries, three toy requirement records), zero deletions on any of them; the commit's only deletions are two lines of `.planning/REQUIREMENTS.md` (EV-07 `Pending` → `Complete`). `git show --numstat e93dbe8` → `9 0` on the registry only. `faults=238/238` unmoved |
| 5 | **SC5** The gate ends `steps=38/38` with a predicted and then measured token delta, and one hosted row is recorded after the content lands with the last-five `cpu_s` median recomputed against the 21 cpu-s allowance | ✓ VERIFIED | `sh tools/gate.sh` → `gate: PASS steps=38/38 … faults=238/238 … goldens=29`, `gate-time total=90`; `dash tools/gate.sh` → **byte-identical tail line** (`cmp` clean); `sh tools/gate-linux.sh` re-run fresh this session → `gate-linux: exit 0`, `gate-time total=320`, differing from the Darwin line in exactly the four documented host tokens (`cc=GNU`, `core-imports=5`, `archive=sha256`, `refs-verified=0/4`). Predicted-vs-measured delta reconciled in `13-05-SUMMARY.md:162-174` (`tracked=2718` predicted, 2722 measured, chased over four named commits). Four hosted rows for this phase's content are in `.planning/ci-runs.tsv` and each was confirmed **live** with `gh run view`: `45717868904` (`pull_request`, success, `14d72b8`), `45718553735` (`pull_request`, success, `e2d3a40`), `45720934719` (`pull_request`, success, `b482e91`), `45723269901` (`push`, success, `151b688`, `cpu_s` 368, `gate_s` 122) — three PR rows and one merge push row. Median recomputed by this verifier: worktree last five `cpu_s` 357, 360, 363, 438, 368 → **363**; `gate_s` 119, 120, 121, 122, 135 → **121**; `363 > 361 + 1.935 = 362.935` → `reprice`, exactly as `STATE.md:589-591` records. The same two medians hold with the uncommitted row removed (`HEAD` window `497 357 360 363 438`) |
| 6 | A ledger row missing its disposition key is refused by name (`rights-missing-key`), observed in a committed control's output | ✓ VERIFIED | `rights-lib.cmake:74` (`gate_fail(rights-missing-key …)`); `tools/check/controls/rights-ledger/EXPECT:1` = `FAIL rights-missing-key: toy-owner-image/disposition`; replayed this session → that exact line, and `control ok   rights-ledger` in the gate log |
| 7 | A row present in `data/rights/` but absent from `data/expected/rights.ids` is refused (`rights-id-extra`), and a tracked `data/rights/` path the selection drops is refused (`rights-unlisted`) | ✓ VERIFIED | `rights-lib.cmake:257` and the reverse-direction loop at `:137`; the control EXPECTs both `FAIL rights-id-extra: toy-stray` and `FAIL rights-unlisted: data/rights/toy_stray.json` (the review's CR-01 fix, stand-in committed at `overlay/data/rights/toy_stray.json`), both observed in this session's replay |
| 8 | Every row's `item` phrase is free of every one of its own `tokens` under the character-bounded rule, so the operator page cannot pin itself | ✓ VERIFIED | Replayed the bounded predicate over all ten rows: zero self-pins. `docs/RIGHTS.md` carries all ten `item` phrases verbatim — checked one by one this session (`a bank-switched cartridge mode`, `the configuration container`, `a commercial cartridge`, `the core information strings`, `the executive image`, `a flat cartridge image`, `the graphics image`, `the frontend ABI subset`, `a rendered page image`, `a saved machine state`), and the `rights-doc-row` limb is green in the committed control (which supplies the negative direction) |
| 9 | A refused spelling planted in `core/src/` or `runner/` is refused by name, and the same spelling in a pinned `(token, path)` pair is not | ✓ VERIFIED | Committed control plants both directions (`rights-naming`, `EXPECT` + `FORBID` above); **live this session** on a `/tmp` toy tree: unpinned `toy-image` in `runner/b.c` → `-- FAIL rights-token: toy-image runner/b.c`, while the pinned hit is suppressed by the control's `FORBID` |
| 10 | A pin whose pair no longer occurs is a failure (`rights-pinned-stale`), and a pin line that is not exactly two fields is `rights-pinned-malformed` | ✓ VERIFIED | `rights-lib.cmake:411` (`read_ids`), `:426` (malformed), `:433` (stale), `:438` (unpinned `rights-token`); control EXPECTs `FAIL rights-pinned-stale: grom-image tools/nowhere.sh` and FORBIDs `rights-pinned-malformed`; real tree `rights-pinned=4/4` |
| 11 | The four measured pre-existing hits of `exec-image`/`grom-image` are pinned and suppressed, and `files` reports `rights-pinned=4/4` | ✓ VERIFIED | `data/expected/rights-pinned.ids` holds exactly four `(token, path)` pairs (`exec-image`/`grom-image` × `docs/owner-images.md`/`tools/owner-run.sh`); gate prints `rights-pinned=4/4` under `sh`, `dash` and the container |
| 12 | `CL-0002` — the only live `established` claim — still passes, and `claims=28 provisional=27` does not move | ✓ VERIFIED | `data/claims.json` last changed at `3fe5373` (Phase 8), so it is untouched by this phase. `CL-0002` is `tier: source-expectation`, `standing: established`, `depends_on` `stic.part`, `crystal_hz`, `region_video`; gate prints `claims=28 provisional=27` in all three runs |
| 13 | All ten D-09-06 mechanisms resolve to a live inventory id: six `control` rows and four `mutant` rows whose names come from `data/expected/runner-selftest.ids` | ✓ VERIFIED | Ten rows at `important-gates.ids:525-534`; six `control ok` lines in this session's gate log; `runner-selftest` prints `cases=14` and the four mutant names are in the inventory it enforces. No row names a mechanism that no longer exists (an unknown id would be `important-gates-unknown-id`, and the step passes) |
| 14 | The `runner-selftest` step fails on an inventory name it does not run, and that guard has a committed stand-in | ✓ VERIFIED | Rule in `tools/check/selftest-inventory.sh:31` (`-- FAIL runner-selftest-missing: <name>`), called at `steps/runner-selftest.sh`; committed stand-in `st-no-such-case`; this session's gate log carries `-- runner-selftest proof runner-selftest-missing: st-no-such-case (observed: -- FAIL runner-selftest-missing: st-no-such-case)`, and the accepting half runs on the real inventory in the same case, so the rule cannot become one that refuses everything |
| 15 | The registry baseline is re-derived by running the step and quoting its printed `gates=`/`lines=`, never quoted from a frozen document, and the step header gains the Phase 13 block in the same commit | ✓ VERIFIED | `-- important-gates pass gates=114 lines=352` in all three runs, equal to the arithmetic over the committed file (352 non-comment rows); `tools/check/steps/important-gates.sh:58-61` carries the Phase 13 block naming RTS-01/RTS-02/EV-06/EV-07 and plan 13-04 (five control rows plus the one new directory, four `mutant` rows through `runner-selftest.ids`, zero new faults) |
| 16 | All ten toy registries gain exactly one line each (13 files, 13 added lines, zero deletions, no `DEVIATION` entry) and the toy tree's derived set still closes | ✓ VERIFIED | `git show --numstat 2c32390` → 13 files at `+1 -0` each and no `-` line on any of them (the commit's only two deletions are `.planning/REQUIREMENTS.md`'s EV-07 status line, 2 added 2 deleted); `control ok   _good/important-gates` and the `control-toy-drift`/`control-deviation-unused` pair are green, so the curated mirror is declared and closed in both directions |
| 17 | The gate ends `steps=38/38`, `faults=238/238`, `goldens=29`, `controls=401/401` under both `sh` and `dash`, with the per-step payloads as predicted | ✓ VERIFIED | `cmp` on the two tail lines → identical; `steps=38/38 controls=401/401 fault-runs=238 faults=238/238 goldens=29`; `ls tools/check/steps/*.sh \| wc -l` → 38; `ls data/golden \| wc -l` → 29; the five per-step payloads read `manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1`, `files tracked=2728 … rights-pinned=4/4`, `claims=28 provisional=27`, `important-gates gates=114 lines=352`, `runner-selftest cases=14` |
| 18 | Exactly one checkpoint verdict is recorded in the frozen vocabulary with its arithmetic beside it, and the medians are recomputed from committed rows rather than quoted | ✓ VERIFIED | `STATE.md:589-591` — one `reprice` verdict, the final window `357 360 363 368 438` / `119 120 121 122 135`, both medians, the `361 + 1.935 = 362.935` bound and the reason `lever-required` is not reached. The frozen 361/118 figures are named stale rather than forced to match. Recomputed independently here: median 363, `gate_s` 121, verdict `reprice`, and the same two medians from `HEAD`'s rows as from the worktree's |

**Score:** 18/18 truths verified (0 present-but-behavior-unverified)

**On SC4's recorded deviation.** The ROADMAP criterion says "a `mutant` row" for each of the ten
mechanisms; the delivery is **six `control` rows and four `mutant` rows**. The deviation is
recorded in three places, not one: `13-04-SUMMARY.md`, `.planning/STATE.md` `[Phase 13]: 13-04`
(`:585`), and as an inline amendment in the ROADMAP criterion itself, beside an amended
`REQUIREMENTS.md` sentence. Its reason is measurable, not stylistic: five of the ten mechanisms
are already proved by control directories that exist (`important-gates-complete-uncovered`,
`-excluded-malformed`, `sources-src-token-whole`, `manifests-output-anchor`,
`memmap-generator-refused`) and one is proved only as a co-firing `EXPECT` line
(`important-gates-excluded-stale`), so a `mutant` row for them would be a second registration of
one proof and would need an invented selftest that adds no evidence. Judged as before, and the
tree still says the same thing: the step header itself reads "five control rows, four mutant rows
… and one new control directory", which is six controls in total.

**On RTS-02's deliberate widening.** The criterion names `core/src`, `runner/`, `tools/` and the
operator pages; the scan also covers the two `owner-local-only` rows' tokens, not only `refused`
ones, because the four measured pins belong to those rows and a stale pin is a failure
(`rights-lib.cmake:335-339` states this as a load-bearing choice). Judged as before: this widens
enforcement rather than narrowing it, the pins are enumerated with their reason in
`data/expected/rights-pinned.ids`, and no token of the `cleared` row is refused.

**EV-07's intent is satisfied by what was delivered, and each clause is checked separately:**

| EV-07 clause | Held? | Evidence |
|---|---|---|
| every one of the ten D-09-06 mechanisms is **registered** | yes | 10 rows, 1:1 onto `09-05-SUMMARY.md`'s canonical ten |
| the registration is the kind that **actually proves** the mechanism | yes | 6 resolve to directories that exist and print `control ok`; 4 resolve to case names the step must run, and an inventory name it does not run is refused |
| the registry **baseline is re-derived** in the same commit | yes | `gates=114 lines=352` is the step's own printed stat and equals the arithmetic over the file; the count edit is in the registration commits, never a later re-bless |
| all ten toy registries extended | yes | 13 files, `+1 -0` each, zero deletions on those files, no `DEVIATION` |
| **zero new faults** | yes | `faults=238/238`, `fault-runs=238`, unmoved in all three runs |

The only thing the deviation gives up is the literal word `mutant` on six rows. It gives up no
coverage, no liveness guard and no fault budget — and it buys the opposite: a row whose kind
matches the proof that exists, enforced by a new inventory that refuses a renamed or deleted case.

### Deferred Items

None. No gap identified in this verification is addressed by a later milestone phase: the four
warnings the previous report raised are closed on the tree (see the re-verification block), and the
one new finding below is a stale sentence in a record, which no later phase's goal or success
criterion claims.

### Advisory (New Scope, Unevidenced)

None. The one new finding this re-verification raises is evidenced twice — by the committed row in
`.planning/ci-runs.tsv` and by a live `gh run view` of that run id — so it is a warning, not an
advisory.

### Required Artifacts

| Artifact | Expected | Status | Details |
| -------- | -------- | ------ | ------- |
| `data/rights/*.json` (10) | one question-shaped row per item | ✓ VERIFIED | 10 files, keys `id`/`item`/`disposition`/`source`/`taken`/`tokens`/`note`; dispositions 1 `cleared`, 4 `refused`, 5 `owner-local-only` |
| `data/expected/rights.ids` | membership baseline, set-equal both ways | ✓ VERIFIED | 10 ids; both directions are findings (`rights-id-missing`/`-extra`) |
| `data/expected/rights-pinned.ids` | four `(token, path)` pins | ✓ VERIFIED | exactly 4; `rights-pinned=4/4`; stale = failure, not NOTE |
| `tools/check/rights-lib.cmake` | both limbs, one place, data-driven | ✓ VERIFIED | 442 lines, SPDX MIT, wired at `manifests.cmake:34` + `:480` and `files.cmake:43` + `:190`; not a stub — every finding observed firing |
| `tools/check/kinds/claim.cmake` | D-50's two conditions | ✓ VERIFIED | `:107`, `:186`; `claim-overstated` retained, not subsumed |
| `data/expected/runner-selftest.ids` | the four mutant case names | ✓ VERIFIED | 4 names, all run; an unrun name is `runner-selftest-missing` |
| `tools/check/selftest-inventory.sh` | the guard as its own file | ✓ VERIFIED | refuses absent/empty/duplicate inventory and an unrun name |
| `data/expected/important-gates.ids` | ten new rows, baseline re-derived | ✓ VERIFIED | 352 non-comment rows, `gates=114`; header carries the phase block |
| `data/sources.json` + `sources.ids` | one `reference-only` record making one live `cleared` row | ✓ VERIFIED | `SRC-libretro-api-docs` at `sources.ids:14`; the only non-null `source` among the ten rows |
| `docs/RIGHTS.md` | operator table carrying all ten `item` phrases | ✓ VERIFIED | all ten phrases matched one by one; `rights-doc-row` green |
| 6 new control directories | each fails for its own reason | ✓ VERIFIED | `rights-ledger`, `rights-naming`, `claim-established-non-source`, `-without-tier1`, `-positive`, `important-gates-excluded-stale`; all `control ok`, `controls=401/401` |
| `tools/check/controls/rights-ledger/overlay/data/rights/toy-blank.json` | committed stand-in for the blank-cell limb | ✓ VERIFIED | `taken: "   "` (whitespace only, so the strip is what makes it blank); its id is in the overlay's `rights.ids` and its phrase in the overlay's `docs/RIGHTS.md`, so it raises exactly one finding — observed as `FAIL rights-blank: toy-blank/taken` |

### Key Link Verification

| From | To  | Via | Status | Details |
| ---- | --- | --- | ------ | ------- |
| `data/rights/<slug>.json` | `data/sources.json` | the row's `source` resolved by registry membership | WIRED | `rights_source_ids()` (`rights-lib.cmake:94`) reads the registry once, fails closed to the empty set; `libretro-abi` resolves, and the control's unregistered id is refused |
| `tools/check/rights-lib.cmake` | `tools/check/manifests.cmake` | `rights_check()` called once, before `gate_finish(manifests)` | WIRED | `include` at `:34`, call at `:480`, `gate_finish` at `:481` — the position is load-bearing so `rights=`/`rights-cleared=` print last |
| `tools/check/rights-lib.cmake` | `tools/check/files.cmake` | `rights_scan_naming(paths)` before `gate_finish(files)` | WIRED | `include` at `:43`, call at `:190`, `gate_finish` at `:191`; `rights-pinned=4/4` prints last |
| `data/expected/rights.ids` | `data/rights/` | `read_ids` set equality, both directions | WIRED | `:249-259`; both directions observed in the control replay |
| `data/expected/rights-pinned.ids` | the naming scan | one pair suppresses exactly one `(token, path)` | WIRED | `:411` reads the pins; `4/4` on the real tree, and the control's suppressed direction is what its `FORBID` pins |
| `data/expected/runner-selftest.ids` | `tools/check/steps/important-gates.sh` | the existing `mutant` inventory loop | WIRED | the four rows resolve, so `gates=114 lines=352` with no unknown id |
| `data/expected/runner-selftest.ids` | `tools/check/steps/runner-selftest.sh` | the step records what it ran and refuses an unrun name | WIRED | proof line in this session's gate log |
| `tools/check/kinds/claim.cmake` | `data/profiles/intv-2609-ntsc-family-p1.json` | the `CLAIM_PROFILE_TAG` cache decides `tier1-primary-doc` | WIRED | `CL-0002`'s three fields resolve to `tier1-primary-doc`; the without-tier1 control's field does not |
| `.planning/ci-runs.tsv` | `.planning/STATE.md` | recomputed medians written as one bullet | WIRED | `STATE.md:589-591`; recomputed independently here, median 363, verdict `reprice` |

### Data-Flow Trace (Level 4)

These are data-driven checks, so the analogue of "does the rendered value reach a real source" is
"does a ledger row's own data reach the finding that fires".

| Artifact | Data variable | Source | Produces real data | Status |
| -------- | ------------- | ------ | ------------------ | ------ |
| `rights_check()` | refused token set | `data/rights/*.json` `tokens`, read per file | yes — a row's edit changes the finding (`toy-image` planted → `rights-token`; `taken` blanked → `rights-blank`) | ✓ FLOWING |
| `rights_scan_naming()` | `(token, path)` pairs | `FILELIST` ∩ the three prefixes + six pages | yes — a pair appears only where the token occurs; a file spelling it twice contributes one pair | ✓ FLOWING |
| `rights_source_ids()` | `srcids` | `data/sources.json` | yes — a non-member id is `rights-source-unresolved`; an unparseable registry yields the empty set (fail-closed) | ✓ FLOWING |
| claim ceiling | `ftag` | `data/profiles/*.json` field `tag` | yes — `stic.part`/`crystal_hz`/`region_video` are `tier1-primary-doc`, the control's `model` is not | ✓ FLOWING |
| `d09-06-registration` rows | mechanism id | `data/expected/controls/*.ids` and `runner-selftest.ids` | yes — a row with no matching inventory id is an unknown id; a renamed case is `runner-selftest-missing` | ✓ FLOWING |

No static return, hardcoded literal or mock stands in for any of these chains.

### Behavioral Spot-Checks

All commands were run in this session. Mutations were made on `/tmp` copies and removed; the tree
is unchanged apart from the two pre-existing planning edits and this report.

| Behavior | Command | Result | Status |
| -------- | ------- | ------ | ------ |
| Full gate, POSIX `sh` | `sh tools/gate.sh` | `gate: PASS steps=38/38 … faults=238/238 … goldens=29`, `gate-time total=90`, exit 0 | ✓ PASS |
| Full gate, `dash` | `dash tools/gate.sh` | exit 0; tail line byte-identical to the `sh` tail (`cmp` clean) | ✓ PASS |
| Full gate, Ubuntu 24.04 container | `sh tools/gate-linux.sh` | `gate-linux: exit 0`, `gate-time total=320`, `steps=38/38 … faults=238/238 … goldens=29`; token-by-token diff shows exactly four host differences (`cc`, `core-imports`, `archive`, `refs-verified`) | ✓ PASS |
| The ledger limb on the real tree | `… steps/manifests.sh` | `-- manifests pass manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1` | ✓ PASS |
| The naming limb on the real tree | `… steps/files.sh` | `-- files pass tracked=2728 binaries-listed=0 rights-pinned=4/4` | ✓ PASS |
| The claims step | `… steps/claims.sh` | `-- claims pass claims=28 provisional=27` | ✓ PASS |
| The `rights-ledger` control's six findings | overlay copied over `_good/tree`, then `manifests.sh` | exactly six FAIL lines, one per `EXPECT` line: `rights-unlisted`, `rights-blank: toy-blank/taken`, `rights-source-missing: toy-cleared`, `rights-missing-key: toy-owner-image/disposition`, `rights-id-extra: toy-stray`, `rights-doc-row: toy-cleared`; nothing extra | ✓ PASS |
| Criterion 2's naming limb, live | `toy-image` planted in `/tmp/…/runner/b.c`, then `files.sh` | `-- FAIL rights-token: toy-image runner/b.c` | ✓ PASS |
| The toy tree still passes the new checks | unmodified `/tmp` copy of `_good/tree` | `-- files pass tracked=129 binaries-listed=0 rights-pinned=0/0` | ✓ PASS |
| The four mutant cases are run, not named | `… steps/runner-selftest.sh` | `-- runner-selftest pass cases=14`, with the `runner-selftest-missing: st-no-such-case` proof line | ✓ PASS |
| The baseline is the file's arithmetic | `awk '!/^#/ && NF' data/expected/important-gates.ids \| wc -l` | `352`, equal to the printed `lines=352` | ✓ PASS |
| The four hosted rows are real runs | `gh run view <id> --json event,conclusion,headSha` | `45717868904` PR/success/`14d72b8`; `45718553735` PR/success/`e2d3a40`; `45720934719` PR/success/`b482e91`; `45723269901` **push**/success/`151b688` | ✓ PASS |
| Cost medians recomputed | arithmetic over the last five successful `ci-runs.tsv` rows (worktree and `HEAD`) | `cpu_s` median **363**, `gate_s` median **121** from both windows → `363 > 362.935` → `reprice` | ✓ PASS |
| The phase added no step, fault or golden | `ls tools/check/steps/*.sh \| wc -l`; `ls data/golden \| wc -l`; gate line | `38`; `29`; `faults=238/238` | ✓ PASS |
| `CL-0002` untouched | `git log -1 -- data/claims.json` | `3fe5373` (Phase 8) — no Phase-13 commit touches it | ✓ PASS |

### Probe Execution

SKIPPED (no runnable probe scripts): this phase declares no `probe-*.sh` and is not a migration or
tooling phase. `find scripts -path '*/tests/probe-*.sh'` → no such tree in this repository; the
phase's runnable surface is the gate's own 38 steps, exercised above.

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
| ----------- | ---------- | ----------- | ------ | -------- |
| **RTS-01** | 13-01, 13-02, 13-05 | One rights-ledger row per third-party-content question, each with a disposition, a `SRC-` for `cleared`, a "what was taken" cell, and a row missing one refused by name | ✓ SATISFIED | Truths 1, 6, 7, 8; `rights=10 rights-cleared=1`; six distinct named refusals now observed in **one committed control** (`rights-missing-key`, `rights-source-missing`, `rights-id-extra`, `rights-unlisted`, `rights-blank`, `rights-doc-row`) |
| **RTS-02** | 13-03, 13-05 | A refused item's spellings cannot appear in `core/src`, `runner/`, `tools/` or the operator pages except via a pinned `(token, path)`; both limbs shown able to fail | ✓ SATISFIED | Truths 2, 9, 10, 11; scope is three prefixes + six exact pages, never a glob; `rights-pinned=4/4`; both directions fail in one committed control, and the unpinned direction was re-observed live this session |
| **EV-06** | 13-03, 13-05 | The claims step refuses a non-`source-expectation` `established` claim or one resting on a non-`tier1-primary-doc` dependency; a legal `established` claim still passes; both shown able to fail | ✓ SATISFIED | Truths 3, 12; two must-fail controls + one legal positive; `CL-0002` unchanged and still passing; `claims=28 provisional=27` unmoved |
| **EV-07** | 13-04, 13-05 | Each of the ten D-09-06 mechanisms gains a registry row over its existing selftest, baseline re-derived, ten toy registries extended, zero new faults | ✓ SATISFIED (delivered as six `control` + four `mutant`; intent held — see the table above) | Truths 4, 13, 14, 15, 16; ten rows 1:1 onto the canonical list; `gates=114 lines=352`; 13 files `+1 -0`; `faults=238/238` |

**Orphaned requirements:** none. `REQUIREMENTS.md:104-107` maps exactly RTS-01, RTS-02, EV-06 and
EV-07 to Phase 13, all four read `Complete`, and all four are declared by at least one plan's
`requirements:` frontmatter (13-01 RTS-01; 13-02 RTS-01; 13-03 RTS-02 + EV-06; 13-04 EV-07; 13-05
all four). No other requirement names Phase 13.

### Anti-Patterns Found

Debt-marker scan (`TBD|FIXME|XXX|TODO|HACK|PLACEHOLDER`) over the phase's 79 changed non-planning
files: **no real marker**. The only `XXXXXX` matches are `mktemp -d` templates
(`steps/important-gates.sh:90`, `steps/runner-selftest.sh:37`) — not debt. No empty implementation,
no `return null`, no hardcoded empty data reaching a finding.

| File | Line | Pattern | Severity | Impact |
| ---- | ---- | ------- | -------- | ------ |
| `.planning/STATE.md` | 51-53 | The hosted-row prose still reads "The merge push run is still absent — `gh run list --commit 151b688 …` returns `[]` — so no `event=push` row for the merge is written", while `.planning/ci-runs.tsv` now carries `45723269901` (`push`, `151b688`, success, `cpu_s` 368, `gate_s` 122) and `gh run view 45723269901` confirms exactly that run | ⚠️ WARNING | A record contradicting its own data file. SC5's hosted-row limb is **met either way** — and now more strongly, by three `pull_request` rows *and* the merge `push` row — so this is a stale sentence, not a failed criterion. One-line edit at the milestone close (GUARDRAILS §7.1/§7.4) |
| `.planning/STATE.md` | 39; `13-VALIDATION.md` Approval block | `tracked=2722` / `files=2722` / `ci-runs=96` are quoted as "the phase's own tokens"; the current gate reads `tracked=2728 files=2728 ci-runs=100` | ℹ️ INFO | Dated quotes of counts that move whenever a file or a CI row is added — and the phase's own close added both (the stand-in, the security record, the four hosted rows). Every frozen invariant (`steps`, `faults`, `goldens`, `controls`, `gates`, `lines`, `rights`, `claims`, `cases`) is unmoved. `tracked=2728` is 2727 tracked files plus this untracked report |
| `.planning/STATE.md` | 590 | The intermediate recompute paragraph enumerates `357 360 363 438 497`; the file's current last five are `357 360 363 438 368` | ℹ️ INFO | Superseded in the same bullet by the final recompute that matches this verifier's arithmetic exactly, and kept per GUARDRAILS §2.10 (original text intact). The **median and the verdict are the same in every window measured** — 363 / 121 / `reprice` |
| `.planning/STATE.md` | 583-591 | The `reprice` verdict sits on a **0.065 cpu-s margin** (`363 > 362.935`); the CONTEXT's rounded price of 2.0 would put the bound at exactly 363.0 and flip the same measurement to `discharged-within-allowance` | ℹ️ INFO | The arithmetic is the recomputed one (6 × 0.31 + 15 × 0.005 = 1.935) and is the right figure per "recompute, never quote". Recorded so the milestone close does not read more certainty into `reprice` than one cpu-second |
| `.planning/ROADMAP.md` | Phase 13 `Plans:` | Reads "4/5 plans executed" and leaves `- [ ] 13-05-PLAN.md` unchecked, while `STATE.md:31` reads "Plan: 5 of 5" | ℹ️ INFO | Record bookkeeping only; the fifth plan's SUMMARY, commits and gate evidence all exist |
| `13-REVIEW.md` IN-01..IN-05; `13-SECURITY.md` R-13-A..R-13-G | — | Accepted, dispositioned residuals (scope excludes `core/include/ivs/`; a `cleared` row lists an inert token; punctuation-only tokens; the page's disposition column unchecked; a duplicated case-name list; `control-make-path`/`-verb` without a stand-in; the naming match is case-sensitive) | ℹ️ INFO | All seven security ids and all five review info ids are still present in their records with reasons and a named owner, all below the `high` block threshold, none reachable today. Not re-litigated here |

### Human Verification Required

None. This phase's entire surface is mechanical — a data-driven checker, a naming scan, a claim
rule and a registry — and every behaviour above was observed firing by command rather than judged.
There is no visual, real-time, external-service or UX-quality property in scope, and the validation
contract records the same conclusion (`13-VALIDATION.md`, "Manual-Only Verifications: none").

### Gaps Summary

No gaps. All five ROADMAP success criteria and all thirteen plan-level must-have truths hold
against the tree, verified by re-run rather than by claim:

- **The ledger is real and fail-closed.** Ten rows, ten ids, one live `cleared` row whose source
  resolves, and six distinct refusals — the blank-cell limb that the last report flagged as having
  no committed stand-in now has one, and the control was observed failing for exactly six intended
  reasons and nothing else.
- **The naming limb is bounded, pinned and non-vacuous.** Three path prefixes plus six exact
  pages, never a glob; four pins that are failures when stale; both directions failing inside one
  committed control; and a live plant refused on a scratch tree this session.
- **The claim ceiling is mechanical.** Two named findings in `claim.cmake`, two must-fail controls
  isolating each condition, one legal positive that survives both, and `CL-0002` asserted rather
  than edited.
- **The ten registrations are the kind that prove something.** Six controls that already existed
  (plus one added), four mutant rows behind a new inventory the step must actually run — with zero
  new faults, the baseline re-derived from the step's own printed stat, and the toy registries
  extended in the same commit with zero deletions on those files.
- **The gate is unmoved where it must be unmoved.** `steps=38/38`, `faults=238/238`, `goldens=29`,
  `controls=401/401`, identical under `sh`, `dash` and the container; `claims=28 provisional=27`.
- **The cost checkpoint is recomputed, not quoted.** Four hosted rows, each confirmed live against
  GitHub; median 363 / 121; `reprice` on a 0.065 cpu-s margin, and the same medians from `HEAD`'s
  rows as from the worktree's.

Three of the previous report's four warnings are closed outright and the fourth is closed for the
limb it named. One **new** warning is recorded: the hosted-row sentence at `STATE.md:51-53` still
denies the merge `push` row that the phase's own close appended and that GitHub confirms. It is a
stale sentence in a record, not a failed truth, and it closes with one edit at the milestone close.

---

_Verified: 2026-11-26T00:20:40Z_
_Verifier: Claude (gsd-verifier)_
