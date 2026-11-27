---
phase: 17-records-cost-window-and-the-count-gate-contract
verified: 2026-11-26T22:12:32Z
status: passed
score: 20/20 must-haves verified
covered_files:
  - .planning/REQUIREMENTS.md
  - .planning/ROADMAP.md
  - .planning/STATE.md
  - .planning/ops/INDEX.md
  - .planning/ops/PLAN-LESSONS.md
  - .planning/ops/RELEASE-OPS.md
  - .planning/phases/17-records-cost-window-and-the-count-gate-contract/17-01-PLAN.md
  - .planning/phases/17-records-cost-window-and-the-count-gate-contract/17-01-SUMMARY.md
  - .planning/phases/17-records-cost-window-and-the-count-gate-contract/17-02-PLAN.md
  - .planning/phases/17-records-cost-window-and-the-count-gate-contract/17-02-SUMMARY.md
  - .planning/phases/17-records-cost-window-and-the-count-gate-contract/17-03-PLAN.md
  - .planning/phases/17-records-cost-window-and-the-count-gate-contract/17-03-SUMMARY.md
  - .planning/phases/17-records-cost-window-and-the-count-gate-contract/17-REVIEW.md
  - .planning/phases/17-records-cost-window-and-the-count-gate-contract/17-SECURITY.md
  - .planning/phases/17-records-cost-window-and-the-count-gate-contract/17-VALIDATION.md
  - data/expected/important-gates.ids
  - data/expected/tests/runner-process.ids
  - docs/formats/ivsc-v1.md
  - docs/packaging.md
  - tests/runner-process-tap.sh
  - tools/check/controls/_good/tree/.planning/REQUIREMENTS.md
  - tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md
  - tools/check/controls/_good/tree/data/expected/important-gates.ids
  - tools/check/controls/important-gates-bad-req-phase5/overlay/data/expected/important-gates.ids
  - tools/check/controls/important-gates-bad-req-phase6/overlay/data/expected/important-gates.ids
  - tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md
  - tools/check/controls/important-gates-req-uncovered-phase5/overlay/data/expected/important-gates.ids
  - tools/check/controls/important-gates-req-uncovered-phase6/overlay/data/expected/important-gates.ids
  - tools/check/controls/important-gates-req-uncovered-phase7/overlay/data/expected/important-gates.ids
  - tools/check/controls/important-gates-uncovered-gate/overlay/data/expected/important-gates.ids
  - tools/check/controls/important-gates-unknown-id-phase5/overlay/data/expected/important-gates.ids
  - tools/check/controls/important-gates-unknown-id-phase6/overlay/data/expected/important-gates.ids
  - tools/check/controls/important-gates-unknown-id/overlay/data/expected/important-gates.ids
  - tools/check/steps/important-gates.sh
covered_digest: "v4:sha256:7cdcd67cce3351f1c8b2b0e394f22cc58ef3f845ea684a37ab29c12f682ddef4"
behavior_unverified: 0
overrides_applied: 0
---

# Phase 17: Records, Cost Window and the Count-Gate Contract — Verification Report

**Phase Goal:** The milestone starts on a measured ledger rather than a quoted one: the hosted cost window is
recomputed by arithmetic from `.planning/ci-runs.tsv` before anything is priced, the eight wrap refusal tokens are
documented, one id guards the wrap-to-shim join, the source-archive letter is corrected, and the count-gate
contract becomes a numbered lesson with a mechanism.

**Verified:** 2026-11-26T22:12:32Z (final tree, `b701d70` plus the untracked orchestrator state the executor may
not stage)
**Status:** passed
**Re-verification:** No — initial verification. No `*-VERIFICATION.md` existed in the phase directory before this
report.

**Tier ceiling:** none, as `GUARDRAILS-v0.4` section 3 states for this phase. This phase mints no claim:
`claims=29` and `provisional=28` are unchanged on every gate line below and `git diff --stat 21d023c -- data/claims.json` is
empty.

## Goal Achievement

### Observable Truths

Rows 1–5 are the ROADMAP's five success criteria (the contract); rows 6–20 are the PLAN frontmatter truths that
add detail beyond them. No plan truth narrows a roadmap criterion.

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | **SC1** — the last-five `cpu_s`, `build_s`, `gate_s` medians are recomputed by arithmetic from `.planning/ci-runs.tsv` at the plan's own start commit and printed before anything is priced | ✓ VERIFIED | `V1` re-derived all six lines at `21d023c` and found them in the record as whole lines (`6`): `window: cpu_s=572 build_s=49 gate_s=164`, `previous-window: 367/34/115`, `rows: 118 lines: 119`, seven `release-rows: <id>=1`, `release-build_s-range: 34 to 51`, `r-range: 10.5143 to 11.6735`. Body order: `## The recomputed window` at `:110`, `## Cost` at `:191` |
| 2 | **SC1** — the seven previously unrecorded release-records rows are present and verified, never re-appended: 118 data rows over 119 lines at the start commit, each of the seven ids in exactly one row | ✓ VERIFIED | `git show 21d023c:.planning/ci-runs.tsv \| wc -l` → `119`; `awk -F'\t' 'NR>1' … \| wc -l` → `118`; each of `45835631107 45836207855 45839153455 45839646456 45841732895 45842795129 45843408874` → `1`; `git diff --stat 21d023c -- .planning/ci-runs.tsv` empty (byte-identical, nothing appended) |
| 3 | **SC1** — exactly one checkpoint line from the `GUARDRAILS-v0.4` §2.4 vocabulary closes the window block, after the priced addition is charged against the 21 allowance | ✓ VERIFIED | `grep -c '^checkpoint: ' 17-01-SUMMARY.md` → `1`; the line is `checkpoint: reprice` (line 221), one of the three §2.4 words, after `## Cost` (line 191); the charge is itemised `0.10 + 0.005 + 0.005 = 0.11` |
| 4 | **SC2** — `diff <(grep '^flatwrap-' data/expected/flat-refusals.ids) <(grep '^flatwrap-' docs/formats/ivsc-v1.md)` is silent over exactly the eight inventory tokens, and the inventory is unchanged | ✓ VERIFIED | `diff` exit `0` with 8 lines each side (`V4` asserts both counts before the diff may be silent); tokens in inventory order `length, word-range, words, base, cross, output-tree, output-exists, empty`; `git diff --exit-code 21d023c -- data/expected/flat-refusals.ids` → clean |
| 5 | **SC3** — one committed id `runner-process.judge.flatwrap-shim` in the existing `tests/runner-process-tap.sh` asserts the shim's cartridge-slot digest equals the container `tools/flatwrap` wrote, a committed stand-in that ignores its input is shown failing it, and `ls tools/check/steps/*.sh \| wc -l` still reads 38 | ✓ VERIFIED | `grep -n 'runner-process.judge.flatwrap-shim' tests/runner-process-tap.sh` → the case's five limbs and `result` at `:1982`; `sh tests/runner-process-tap.sh …` → `ok 113 runner-process.judge.flatwrap-shim`, `failed=0`; both presets `ok` (`V6`); the inert-flip and replay stand-ins both fail it (Demos A/B below); `ls tools/check/steps/*.sh \| wc -l` → `38` |
| 6 | **SC4** — `docs/packaging.md`'s "no host shim ships" sentence is corrected to name the source archive beside the static package and agrees, sentence for sentence, with the release-decision line of `RELEASE-OPS.md` §(i) | ✓ VERIFIED | One canonical string, `V5`: `2` occurrences in `docs/packaging.md` (`:39` paragraph, `:123` release bullet) and `1` in §(i)'s own slice (`:78`); `grep -c -F` byte-identical on both sides |
| 7 | **SC5** — `.planning/ops/PLAN-LESSONS.md` holds lesson 32 **with its mechanism**, carried by a `mutant` row in `data/expected/important-gates.ids` over an existing selftest and shown able to fail on a committed stand-in, and `faults=238`, `goldens=29`, `claims=29`, `provisional=28` are unchanged | ✓ VERIFIED | Lesson `32.` at `PLAN-LESSONS.md:85` with the three clauses and a clause-to-mechanism map; `count-gate mutant extra-file EV-08` at `important-gates.ids:646`; the `extra-file` stand-in is committed in `tools/check/steps/pkg-install.sh:663-670` and re-runs every gate run as `pkg-selftest=20/20`; with the refusal removed it fails (`-- FAIL pkg-selftest-extra-file: inventory passed`, exit 1 — Demo C); `V7` recomputed `invariants: faults=238 goldens=29 claims=29 provisional=28` and matched the lesson's line |
| 8 | **17-01** — the record prints **computed** values and names where they differ from the roadmap's rounded prose; `r` is arithmetic only and never a signal | ✓ VERIFIED | `release-build_s-range: 34 to 51` (the seven rows carry 51, 42, 34, 47, 51, 51, 49) against the roadmap's "47 to 51", named as the roadmap's rounding; `r-range: 10.5143 to 11.6735` against "10.6 to 11.7"; `r` is stated as not a decision input (lesson 29) |
| 9 | **17-01** — the gate is green under both shells with every predicted token where the plan said, and the only movement the plan causes is the record it commits | ✓ VERIFIED | `sh` and `dash` summary lines `cmp`-clean and byte-identical (see Gate below); `git show --stat --oneline 99df1e3` → `1 file changed, 326 insertions(+)` (the record only) |
| 10 | **17-02** — each documented token carries its trigger **and** the boundary case that must still be accepted, plus the three framing facts | ✓ VERIFIED | Read `## Wrap tool refusals`: eight blocks each with "must still be accepted"; framing facts: tool tokens ≠ the container format's 18 numbered refusal reasons (the `## Refusal reasons` table has 18 rows), inventory order not the tool's check order, one stderr line + exit 1 + no output file for a refusal vs no token + exit 2 otherwise. Source agrees: 8 `flatwrap-*` literals in `tools/flatwrap/flatwrap.c`, `fw_refuse` returns `1` (`:117`) and usage paths return `2` |
| 11 | **17-02** — the id's three equality limbs hold, and one flipped input byte moves **both** values, so an input-ignoring stand-in fails it | ✓ VERIFIED | Limbs (1)–(3) at `tests/runner-process-tap.sh:1911-1945`; Demo A (flip removed → both containers identical) still passed limbs (2) and (3) and was caught only by the liveness limb: `# join-immobile: … tool digest at '<same>'/'<same>' and digest.images at '<same>'/'<same>'` |
| 12 | **17-02** — the must-fail stand-in is committed and judged by the **same** `shim_join`; it is refused with `join-images`, and an acceptance records `judge-vacuous`; no second id or process; `tests-expected` `+2` | ✓ VERIFIED | Limb (5) at `:1964-1980`; Demo B (stand-in built from the **second** report, so the comparison cannot discriminate) → `not ok 113 … # judge-vacuous: shim_join accepted the stand-in that replays the first container against the second`. `tests-expected=5042 → 5044` (one id × 2 presets), one new line in `data/expected/tests/runner-process.ids`. **Recorded warning:** this truth's last clause says the priced line "stays 0.10 cpu-s" while 17-01's cost table and 17-03 both read 0.11 — see Recorded Warnings |
| 13 | **17-02** — the id reports `not ok` with a reason when any of the three arguments is missing, and never a skip; `tests-skipped` stays 4 and `tests-missing` 0; no new golden, fault, claim or installed file | ✓ VERIFIED | Ran the suite with the eighth argument omitted: `not ok 113 runner-process.judge.flatwrap-shim` + `# flatwrap-shim: the eighth argument (ivs_host_harness) is missing: the join cannot be run` (no `# SKIP`); gate line `tests-skipped=4 tests-missing=0`, `goldens=29`, `faults=238/238`, `claims=29`, `prefix-files=11`; `git diff --stat 21d023c -- data/golden data/expected/faults data/expected/pkg-install-files.ids data/claims.json` empty |
| 14 | **17-02 / 17-03** — every baseline each plan states is measured at the plan's own start commit, recorded as `start-commit: <sha>` and asserted there | ✓ VERIFIED | `17-02-SUMMARY.md:73` `67e35a6…`, `17-03-SUMMARY.md:70` `1dc57c2…`, `17-01-SUMMARY.md:137` `21d023c…`; all three `git cat-file -e` and `git merge-base --is-ancestor` clean; re-derived at those shas: `67e35a6` → registry rows `364`, runner-process ids `149`, `git ls-tree -r` `2807`; `1dc57c2` → rows `365`, tracked `2808` — every recorded figure matches |
| 15 | **17-02** — `docs/packaging.md`'s two wrong sentences are corrected **in place**, `RELEASE-OPS.md` §(i) carries the same sentence verbatim with the superseded v0.3.0 sentence quoted and dated beneath it, and §(i.1) is untouched byte for byte | ✓ VERIFIED | `git diff 21d023c -- docs/packaging.md` shows both sentences rewritten in place; §(i) at `RELEASE-OPS.md:78`; superseded sentence quoted at `:86` dated "superseded 2026-11-26"; `### (i.1)`→EOF sha256 `fc416399…` identical in the working tree and at `21d023c` |
| 16 | **17-02** — EV-08 is `Complete` in the **same commit** as its first registry row, with the thirteen toy lines, so neither `important-gates-unknown-id` nor `important-gates-complete-uncovered` fires and no pinned line moves | ✓ VERIFIED | `git show --stat be69a5e` → `REQUIREMENTS.md`, `important-gates.ids`, `runner-process.ids`, the suite, `important-gates.sh` and exactly thirteen toy lines (1 `_good` REQUIREMENTS, 1 `_good` milestone record, 1 `_good` registry, 9 overlay registries, 1 overlay REQUIREMENTS); `important-gates` prints `gates=120 lines=366` with zero findings; `sh tools/check/run-controls.sh` → `controls=404/404` |
| 17 | **17-03** — lesson 32 is scoped to its mechanism: each clause names what proves it, and the clause with **no** checker says so in as many words | ✓ VERIFIED | Clause 1 names the `count-gate` row and the `extra-file` case; clause 3 cites `d09-06-registration mutant st-pass-line EV-07` and does not duplicate it; clause 2 reads "**no mechanism**, and the gap is stated rather than implied away" with the reason (no gate step parses plans) and the decision not to register a row for a mechanism that does not exist |
| 18 | **17-03** — the three clause lines are indented list items with the number inside, `32. ` is the last column-1 numbered lesson, and `INDEX.md` carries the corrected range | ✓ VERIFIED | `grep -n '^[0-9]\+\. '` → last is `32.`; the clause lines read `  - **1.** …`, `  - **2.** …`, `  - **3.** …` (`V7` compares them as whole lines); `.planning/ops/INDEX.md:20` reads "Numbered planning lessons 1–32" and `1–25` occurs 0 times |
| 19 | **17-03** — the mechanism is a `mutant` row over an **existing** selftest case whose count comes from a named file, with no new case, test id or step, so no 0.05 selftest price is incurred and the priced line stays 0.11 | ✓ VERIFIED | `count-gate mutant extra-file EV-08` resolves through `data/expected/pkg-selftests.ids:18`; `pkg-install.sh:663` is the `extra-file` case, `check_inventory` (`:101-118`) reads `$inv` = `data/expected/pkg-install-files.ids` and prints `pkg-inventory-list: data/expected/pkg-install-files.ids` when it is missing; `pkg-selftest=20/20` unchanged; `git diff 21d023c -- data/expected/pkg-selftests.ids` empty; `ls tools/check/steps/*.sh \| wc -l` → `38` |
| 20 | **17-03** — the tail clause is cited, not duplicated, and re-demonstrated from the committed copied-runner stand-in; `gates` 119→120 and `lines` 365→366 with nothing else moving | ✓ VERIFIED | `runner-selftest` prints `-- runner-selftest proof control-pass-line-not-last: profile (observed: -- FAIL control-pass-line-not-last: profile)` with zero `^-- FAIL runner-selftest` lines (`V7`); gate line `gates=120 lines=366`; every other predicted token matches (Gate below) |

**Score:** 20/20 truths verified (0 present-but-behavior-unverified, 0 overrides).

### Gate (re-run on the final tree, never read from a SUMMARY)

```sh
sh  tools/gate.sh > /tmp/verif-g.out 2>&1   # exit 0, 63 s
dash tools/gate.sh > /tmp/verif-d.out 2>&1  # exit 0, 67 s
cmp <(tail -1 /tmp/verif-g.out) <(tail -1 /tmp/verif-d.out)   # identical
grep -c '^-- FAIL ' /tmp/verif-g.out /tmp/verif-d.out         # 0 and 0
```

`sh tools/gate.sh` → (the `dash` line is byte-identical, `cmp` clean):

```
gate: PASS steps=38/38 controls=404/404 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=118 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=29 provisional=28 handoffs=8 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=695 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2811 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=63 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2811 history-commits=1 gates=120 lines=366 input-rows=31 input-policies=11 input-profile-fields=31 files=2811 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=20/20 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=12/12 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5044 tests-executed=5040 tests-skipped=4 tests-missing=0 docs=64 claim-refs=27 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

Token-by-token against the phase's predicted state — every one present (`grep -qxF` over the token list):

| predicted | found | predicted | found |
|---|---|---|---|
| `steps=38/38` | ✓ | `prefix-files=11` | ✓ |
| `faults=238/238` | ✓ | `handoffs=8` | ✓ |
| `goldens=29` | ✓ | `tests-expected=5044` | ✓ |
| `claims=29` | ✓ | `tests-executed=5040` | ✓ |
| `provisional=28` | ✓ | `tests-skipped=4` | ✓ |
| `gates=120` | ✓ | `tests-missing=0` | ✓ |
| `lines=366` | ✓ | `docs=64` | ✓ |
| `controls=404/404` | ✓ | | |

`tracked`/`files`/`hygiene-files` read `2811` rather than `2810` because this report was still untracked at that
run; that is the same one-file explanation 17-01 and 17-02 recorded, not movement. Both shells were re-run once
more **with this report in the tree**, which is the tree this record describes: `sh` and `dash` again `cmp`-clean
and byte-identical, zero `^-- FAIL` lines, every predicted token present, and
`tracked=2812 files=2812 hygiene-files=2812` — the same one-file difference and nothing else.

**Container gate** (`GUARDRAILS-v0.4` §4, and the item `R-17-E` handed to this verification):

```sh
sh tools/gate-linux.sh > /tmp/verif-linux.out 2>&1    # gate-time total=279, 5m25s wall
grep -c '^-- FAIL ' build/gate-linux.log               # 0
```

```
gate-linux: x86_64, gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0, cmake version 3.28.3
gate: PASS steps=38/38 controls=404/404 … cc=GNU … core-imports=5 … archive=sha256 … refs-verified=0/4 …
  gates=120 lines=366 … tests-expected=5044 tests-executed=5040 tests-skipped=4 tests-missing=0 docs=64 …
gate-linux: exit 0 (log build/gate-linux.log)
```

The four differing tokens (`cc=GNU`, `core-imports=5`, `archive=sha256`, `refs-verified=0/4`) are the known
host-dependent ones and were different before this phase. `build/gate-linux.log` was copied aside first
(lesson 18) to `/tmp/verif-gate-linux.log`. No `ivs-*` container or volume was left behind.

### Required Artifacts

| Artifact | Expected | Status | Details |
|---|---|---|---|
| `docs/formats/ivsc-v1.md` | `## Wrap tool refusals`, eight line-initial tokens | ✓ VERIFIED | section present between `## Refusal reasons` and `## Digests`; 8 line-initial tokens, `diff` silent against the inventory |
| `docs/packaging.md` | the corrected clause at `:39` and the corrected bullet at `:123` | ✓ VERIFIED | both rewritten in place; canonical string ×2 |
| `.planning/ops/RELEASE-OPS.md` | §(i) amended in place, superseded sentence quoted and dated | ✓ VERIFIED | `:78` the line, `:86` the dated quote; §(i.1)→EOF byte-identical to `21d023c` |
| `tests/runner-process-tap.sh` | `runner-process.judge.flatwrap-shim` | ✓ VERIFIED | `shim_join` + five limbs + committed stand-in, `:1876-1982`; substantive (real processes, no typed digest) and wired (`result` line emits the id) |
| `data/expected/tests/runner-process.ids` | one appended line | ✓ VERIFIED | `:242`; row count 149 → 150 |
| `data/expected/important-gates.ids` | one `cart-role` row plus a Phase 17 block, one `count-gate` row | ✓ VERIFIED | `:628` and `:646`, with comments; rows 364 → 366 |
| `tools/check/steps/important-gates.sh` | the Phase 17 phase-table block | ✓ VERIFIED | `+17` lines documenting both rows; step prints `gates=120 lines=366` |
| `.planning/ops/PLAN-LESSONS.md` | lesson 32 | ✓ VERIFIED | `:85`, last column-1 numbered lesson; `invariants:` line at the end |
| `.planning/ops/INDEX.md` | lesson range corrected to 1–32 | ✓ VERIFIED | `:20` |
| `.planning/REQUIREMENTS.md` | EV-08 `Complete` | ✓ VERIFIED | `:21` checkbox, `:88` traceability row |
| `.planning/phases/…/17-01-SUMMARY.md` | the window block, the `## Cost` block, one `checkpoint:` line | ✓ VERIFIED | `:110`, `:191`, `:221` |
| ten toy registries and three toy requirement records | thirteen appended lines | ✓ VERIFIED | exactly thirteen, in `be69a5e` |

### Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| `.planning/ci-runs.tsv` | `17-01-SUMMARY.md` | the recipe (columns 13/8/10, `NR>1`, last five, `sort -n`, `sed -n 3p`) stated in the record | ✓ WIRED | `V1` re-derived all six lines from the named file and matched them as whole lines; `git diff --quiet 21d023c` clean |
| `17-01-SUMMARY.md` | `tools/check/steps/ci-log.sh` | the record's totals equal the count the `ci-log` step prints from the same file | ✓ WIRED | `-- ci-log pass ci-runs=118` == the record's `rows: 118` |
| `17-01-SUMMARY.md` | `.planning/ops/GUARDRAILS-v0.4.md` | §2.4 fixes the three-token vocabulary and the runner-event rule | ✓ WIRED | the one line is `checkpoint: reprice`, drawn from the vocabulary and selected by the §2.4 rule |
| `docs/formats/ivsc-v1.md` | `data/expected/flat-refusals.ids` | the criterion is a `diff` of two `grep '^flatwrap-'` outputs, so each token must be first and only text on its line | ✓ WIRED | `diff` silent; the renaming stand-in makes it non-silent (Demo D) |
| `tests/runner-process-tap.sh` | `core/src/machine/machine_digest.c` | `IVS_DIGEST_IMAGES` serialises `slot.cart.sha256` | ✓ WIRED | limb (2) compares the runner's `slot.cart.sha256` to the tool's printed digest; Demo A shows both values move with one input byte |
| `data/expected/important-gates.ids` | `.planning/REQUIREMENTS.md` | the step derives its accepted set from the traceability table | ✓ WIRED | `be69a5e` carries the flip and the first row in one commit; `important-gates pass gates=120 lines=366`, zero findings; `controls=404/404` |
| `docs/packaging.md` | `.planning/ops/RELEASE-OPS.md` | the two documents must agree sentence for sentence | ✓ WIRED | identical string ×2 in the page and ×1 in §(i) (`V5`) |
| `.planning/ops/PLAN-LESSONS.md` | `data/expected/important-gates.ids` | the lesson names the row that proves its mechanical clause | ✓ WIRED | clause 1 → `count-gate` row → `extra-file` case |
| `data/expected/important-gates.ids` | `data/expected/pkg-selftests.ids` | a `mutant` row resolves only through one of the three selftest inventories | ✓ WIRED | `extra-file` at `pkg-selftests.ids:18` is run by `pkg-install.sh:663`; step reports `pkg-selftest=20/20` |
| `.planning/ops/PLAN-LESSONS.md` | `tools/check/steps/runner-selftest.sh` | clause 3's proof is the in-step `st-pass-line` case, printed in every gate log | ✓ WIRED | `-- runner-selftest proof control-pass-line-not-last: …` present in both logs, zero `^-- FAIL runner-selftest` |

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
|---|---|---|---|---|
| `runner-process.judge.flatwrap-shim` | `digest.images` (harness and runner) | two real process launches over containers the real `tools/flatwrap` wrote | Yes — Demo A moved both values with one flipped input byte; the passing run is only reachable when the two processes digested the same container | ✓ FLOWING |
| `runner-process.judge.flatwrap-shim` | `slot.cart.sha256` vs the tool's printed digest | `ivs_flatwrap` stdout vs `IVS_DIGEST_IMAGES` | Yes — compared as strings in limb (2); Demo A shows both move together | ✓ FLOWING |
| `17-01-SUMMARY.md` window block | `cpu_s`/`build_s`/`gate_s` medians | `.planning/ci-runs.tsv` columns 13/8/10 at `21d023c` | Yes — every figure re-derived from the file by the printed recipe (`V1`); nothing typed | ✓ FLOWING |
| `pkg-install` `prefix-files=` | `$files` | `grep -c . "$T/inv.have"` (observed listing) reconciled against `data/expected/pkg-install-files.ids` by `comm` | Yes — the named file's absence prints `pkg-inventory-list`; the `extra-file` stand-in proves the reconciliation refuses | ✓ FLOWING (shape noted: reconciliation, not a direct read — `R-17-C`) |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|---|---|---|---|
| The join id passes on the final tree (dev preset) | `sh tests/runner-process-tap.sh $PWD build/dev/… Debug build/dev/tools/flatwrap/ivs_flatwrap build/dev/host/libivs_retro.dylib build/dev/tests/ivs_host_harness` | `ok 113 runner-process.judge.flatwrap-shim`; `executed=148 skipped=2 failed=0` | ✓ PASS |
| The id is `ok` in both presets as the gate recorded it | `grep -qE '^ok [0-9]+ runner-process\.judge\.flatwrap-shim$' build/gate/{dev,release}/runner-process.txt` (`V6`) | both match | ✓ PASS |
| Missing argument → `not ok` with a reason, never a skip | same suite with the eighth argument omitted | `not ok 113 …` + `# flatwrap-shim: the eighth argument (ivs_host_harness) is missing: the join cannot be run` | ✓ PASS |
| **Demo A** — the flipped-byte limb, and the liveness guard doing the work | scratch copy, flip line removed (`:1937`), same suite | `not ok 113` + `# join-immobile: … '<same>'/'<same>' … '<same>'/'<same>'` — limbs (2) and (3) still passed | ✓ PASS (mechanism can fail) |
| **Demo B** — the committed replay stand-in | scratch copy, stand-in built from the **second** report (`:1967`) | `not ok 113` + `# judge-vacuous: shim_join accepted the stand-in that replays the first container against the second` | ✓ PASS (mechanism can fail) |
| **Demo C** — the `count-gate` row's `extra-file` stand-in | scratch copy of `pkg-install.sh` with the extra-path refusal removed (`:116`), real step | real step: `-- pkg-install pass … pkg-selftest=20/20`; inert copy: `-- FAIL pkg-selftest-extra-file: inventory passed`, exit 1 | ✓ PASS (mechanism can fail) |
| **Demo D** — the wrap-refusal `diff`'s renaming stand-in | scratch copy of `docs/formats/ivsc-v1.md` with `flatwrap-cross` → `flatwrap-crossover` | `diff` exit 1, `5c5 < flatwrap-cross --- > flatwrap-crossover` | ✓ PASS (mechanism can fail) |
| `V1`–`V8` from `17-VALIDATION.md`, verbatim, on the final tree | each block as written | all eight exit `0` (`V1` prints the six lines and `6`; `V7` prints `invariants: faults=238 goldens=29 claims=29 provisional=28`) | ✓ PASS |

Constraints respected: no server started, no state mutated, one named suite run per demonstration, scratch copies
only under `/tmp`.

### Probe Execution

Step 7c: **SKIPPED (no probes)**. Neither plan nor summary declares a `probe-*.sh`, and
`find scripts -path '*/tests/probe-*.sh'` is empty; this phase is a records/guard phase, not a migration or CLI
phase. The equivalent evidence is the gate runs and `V1`–`V8` above.

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|---|---|---|---|---|
| `EV-08` | 17-01, 17-02, 17-03 (all three declare `requirements: [EV-08]`) | measured ledger, documented tokens, one join id, corrected letter, lesson with a mechanism | ✓ SATISFIED | `REQUIREMENTS.md:21` `[x]`, `:88` `EV-08 \| Phase 17 \| Complete`; every clause mapped to rows 1–7 above; `requirements-completed: [EV-08]` in 17-02 and 17-03 |

Orphaned requirements mapped to Phase 17: **none** — `awk -F'|' '$3 ~ /Phase 17/ {print $2}'` returns only
`EV-08`, and all three plans claim it. No requirement was reduced in scope by a plan.

Handoff ledger (`GUARDRAILS-v0.4` §6.8): `data/expected/handoffs.ids` is byte-identical to `21d023c`, still eight
rows (`handoffs=8`), and this phase adds none. No `H<n>-<k>` id is mentioned by any phase record and no claim
cites a handoff row (`git diff --stat 21d023c -- data/claims.json` empty). The `D1`–`D6` tokens in the phase
records are the research deliverables and the summaries' `coverage:` ids — a different namespace from the
`D10`/`D11` owner-gate ids this ledger holds — not handoff rows.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---|---|---|---|
| — | — | `TBD`/`FIXME`/`XXX` in a line this phase modified | none | The only `TBD` hits in phase-modified files are `mktemp …XXXXXX` templates in `tests/runner-process-tap.sh:166,2080` and `tools/check/steps/important-gates.sh:137` (not debt markers), plus `**Plans**: TBD` on `ROADMAP.md:259,277,294` for phases 18–20 — pre-existing, untouched by this phase |
| — | — | empty/stub implementation in the new code | none | The new id has five real limbs over real processes; `result` emits the id; the three `why …` argument checks make a missing argument a failure, not a skip (demonstrated). No `return null`/`=> {}` class pattern exists in POSIX shell here; the only `return` shapes are the real verdicts |

Debt-marker gate: **no unreferenced marker** in a line this phase changed → no blocker.

### Human Verification Required

**None.** Every truth is settled by a command in this record. The two record criteria (1 and 4) carry residuals
that no tool can settle — named under Recorded Warnings — but neither is an outstanding human *decision*: the
discuss record already decided the sentence (`17-DISCUSSION-LOG.md:51-60`), the review already fixed its pointer
(`CR-01`), and the figures are re-derivable from the named file.

## Recorded Warnings and Residuals

Nothing here leaves a ROADMAP criterion unmet. Each item is either a plan-text residue or a recorded, owned debt.

**1. A plan truth quotes a superseded price.** `17-02-PLAN.md`'s fourth truth ends "the phase's priced line stays
0.10 cpu-s"; 17-01's published cost table (`0.10` id + `0.005` `cart-role` + `0.005` `count-gate` = **0.11**) and
17-03's truth both read **0.11**, and the arithmetic re-derives to 0.11. The truth's substantive clauses — no
second id, no second process, `tests-expected` `+2` — are verified (row 12). This is a plan-text residue of the
same class as the `exec: -` deviation 17-02 recorded, superseded by two records; correcting it would mean rewriting
a completed plan record, which `GUARDRAILS-v0.4` §2.12 forbids. The roadmap's `**Cost**: 0.10` line is the
pre-execution estimate, and 17-01 records the 10 percent overage explicitly rather than rounding it.

**2. Record criterion 1's residual — computed or typed to agree.** A tool cannot observe authorship. What can be
observed is that all six printed lines re-derive from `.planning/ci-runs.tsv` at `21d023c` by the printed recipe
(`V1`), that the file is byte-identical to that commit, and that the one independent count (`ci-log`) agrees. The
residual is therefore only whether the author ran that recipe before writing — unobservable in principle, and
narrowed to nothing material by the recomputation. Ordering: the window block is at `:110` and the first pricing
block at `:191`; the summary blurb at `:108` prints the three medians before it names the addition, and the
frontmatter `provides`/`key-decisions` carry both.

**3. Record criterion 4's residual — is the agreed sentence the right sentence.** A tool can prove the two
documents agree byte for byte and that §(i.1) is frozen; it cannot judge whether the sentence is true of the
release. That judgement was made: the discuss record chose "correct both wrong sentences inline and amend §(i) in
place, quoting the superseded sentence beneath it" (`17-DISCUSSION-LOG.md:51-60`), and the review fixed the
pointer's wording (`CR-01`, `aee972a`) and the three deferred clauses (`WR-02`, `01baed9`). The support a tool can
give is present: the release's source archive really does carry the tools' and the shim's source
(`docs/packaging.md:108,118`), and the member check refuses a built object by extension.

**4. Open review findings — both are debts inside met criteria, not gaps.**

| Finding | Verdict | Why it does not leave a criterion unmet |
|---|---|---|
| `IN-02` — lesson 32's clause-1 exemplar cites `pkg-install`'s `prefix-files=`, which is counted from the observed listing and pinned to the named file by `comm`, not read from it | **owned debt** (`R-17-C`, owner: Phase 20's stale-text sweep) | SC5 requires the lesson **with a mechanism** carried by a `mutant` row over an existing selftest and shown able to fail — all true (rows 7, 19, Demo C). The step does name the file it reconciles against and prints its name when it is missing, so the exemplar is weaker than the clause, not false. No count, case, row, gate or invariant moves; the direct-count shape (`ci-log`'s `ci-runs=`) exists and was measured here |
| `IN-03` — EV-08 read `Complete` for exactly one commit (`be69a5e`) before the `count-gate` row landed (`bfa0815`) | **owned debt** (`R-17-D`, historical; no owner needed) | At the verified tree the requirement is Complete **and** every clause has its artifact; `important-gates` refuses a `Complete` requirement with no row, so a persistent drift is caught. A one-commit historical window cannot be closed retroactively and rewrites are forbidden by §2.12. No criterion is judged on an intermediate commit |

**5. The five accepted security debts — all inside met criteria.**

| Debt | Criterion affected | Verdict |
|---|---|---|
| `R-17-A` — the window medians are re-derived by a committed recipe and `V1`, not by a gate step | SC1 | **inside a met criterion.** SC1 asks for the recomputation to be printed in the phase record before pricing, which it is; `steps` may not grow (§2.1), and `ci-log` catches ledger-row drift every run |
| `R-17-B` — the doc-to-inventory `diff` is a committed verify command, not a gate step | SC2 | **inside a met criterion.** SC2's own wording is that the `diff` is silent; it is. The inventory side is gate-enforced in both directions, so the open drift is doc-only |
| `R-17-C` — see `IN-02` above | SC5 | **inside a met criterion** (row 7/19 verified, Demo C) |
| `R-17-D` — see `IN-03` above | SC5 / EV-08 traceability | **inside a met criterion** at the verified tree |
| `R-17-E` — the container gate was not re-run by the security audit | §4 definition of done | **discharged by this verification**: `gate-linux: exit 0`, `gate-time total=279`, zero `^-- FAIL` lines, same tokens apart from the four known host-dependent ones |

`threats_open: 0` (16 distinct threats, 16 closed, none at or above `high` open) — re-read, and the one `high` row
is closed by measurement, not assumption.

## Evidence Standard (what was automated, thrown away, handed off, and what no tool can judge)

**Automated (re-run on the final tree, not read from a SUMMARY):** `sh` and `dash` gate runs (`cmp`-identical,
zero leading `-- FAIL` lines), the Ubuntu container gate, `V1`–`V8` verbatim, the `runner-process` suite in the dev
preset, the id in both presets via the gate's own TAP files, `ci-log`, `important-gates`, `runner-selftest`,
`pkg-install`, `hygiene --tree`, the `binary-ext`/`git ls-files` checks, the frozen-section sha256 comparison,
`git cat-file -e` + `merge-base --is-ancestor` + figure recomputation at all three start commits, and four
must-fail demonstrations.

**Thrown away:** four scratch copies under `/tmp` only — `tap-A.sh` (flip removed), `tap-B.sh` (stand-in from the
second report), `pkg-install-inert.sh` (extra-path refusal removed), `ivsc-renamed.md` (one token renamed) — plus
their logs and the temporary gate outputs. Nothing was written into the work tree; `git status --porcelain` shows
only `.planning/state.json`, which the executor may not stage. All `/tmp` scratch was removed at the end of this
verification.

**Handed off:** nothing new. `handoffs=8` and `data/expected/handoffs.ids` are unchanged. The only item that had
been handed to this verification, `R-17-E`, is discharged above.

**What no tool can judge:** whether a figure was computed or typed to agree (residual 2), and whether the agreed
release-decision sentence is true of the release as a matter of judgement (residual 3). Both are recorded rather
than converted into a human-verification item, because the discuss record and the review already decided them.

**Tier:** none — this phase mints no claim. `claims=29` and `provisional=28` on all three gate lines, and
`data/claims.json` is untouched.

## Gaps Summary

No gaps. All five ROADMAP success criteria and all fifteen additional plan truths are verified on the final tree:
the window is recomputed and re-derivable at the plan's own start commit, the seven ledger rows were read and left
byte-identical, the eight refusal tokens are documented in inventory order and agree with the append-only
inventory, one committed id guards the join and is refused by a committed stand-in while a second limb proves the
flip is not inert, the source-archive letter is corrected in both live documents with the frozen §(i.1) untouched,
and lesson 32 carries a mechanism that fails when the count stops being pinned to the named file.

---

_Verified: 2026-11-26T22:12:32Z_
_Verifier: Claude (gsd-verifier)_
