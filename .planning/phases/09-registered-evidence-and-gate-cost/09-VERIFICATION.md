---
phase: 09-registered-evidence-and-gate-cost
verified: 2026-11-26T03:10:00Z
status: passed
score: 5/5 must-haves verified
verifier: gsd-verifier
tree: 86c7878 (+ the pre-existing dirty .planning/state.json, untouched)
phase_base: 912ab9a
phase_span: "912ab9a..HEAD = 39 commits, 81 files, 4875 insertions, 140 deletions"
gates_rerun_by_verifier:
  sh: "exit 0, gate: PASS steps=38/38 controls=384/384 faults=235/235 fault-runs=235 gates=107 lines=330 goldens=29 reports=30 host-dependent=3 release-only=2 cases=12 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 claims=28 provisional=27 files=2556 tracked=2556 hygiene-files=2556, 0 top-level '-- FAIL ' lines"
  dash: "exit 0, gate: PASS, token list byte-identical to sh (diff of the space-split tokens empty), 0 top-level '-- FAIL ' lines"
  linux: "exit 0, gate-linux: exit 0, gate: PASS, cc=GNU, core-imports=5, archive=sha256, refs-verified=0/4, files=tracked=hygiene-files=2556, 0 top-level '-- FAIL ' lines, no ivs- container left behind"
must_haves:
  truths:
    - "SC1 the hosted row settles the cost level: every wave A1 commit is an ancestor of the row's head, the row's head is an ancestor of HEAD, the last-five median r recomputed from .planning/ci-runs.tsv is 7.6957 (<= 7.8), and .planning/CI-EFFICIENCY.md carries exactly one 'phaseA1:' line, 'discharged-below-budget'"
    - "SC2 no commit before the row added a test id, control, fault or gate step: 38 step scripts, data/expected/gate-steps/ unchanged across the whole phase, no test-id row moved in any data/expected/tests/*.ids, and the ancestry check passes"
    - "SC3 data/expected/important-gates.ids grows 306 -> 330 rows (24 additions, 0 deletions) and 90 -> 107 gate names, naming >= 1 mechanism for each of FND-01..07, CPU-01..05, MEM-01..05; the registration commits' steps/controls/faults/tests-* delta is 0 and gates/lines is +17/+24; all ten toy registries gain 17 lines each with 0 deletions"
    - "SC4 each of the five gate-hardening items has a named mechanism shown to fail for its intended reason (three new controls, one verification of two existing controls, one step-side mechanism), with the A-02 count deviation recorded and its reason stated"
    - "SC5 the EV-04 machine check derives its must-be-covered set from the requirement records, refuses a Complete-uncovered requirement and a stale exclusion row, refuses a drifted toy registry and an unused deviation declaration, each is shown able to fail by a committed control or selftest case, and Complete - excluded = registered holds (54 - 10 = 44)"
  artifacts:
    - path: data/expected/important-gates.ids
      provides: "330 non-comment registry rows over 107 gate names"
      verified: true
    - path: data/expected/important-gates-excluded.ids
      provides: "the ten-row exclusion ledger, a different file from the registry"
      verified: true
    - path: tools/check/steps/important-gates.sh
      provides: "the derived must-be-covered set and the two-directional ledger refusals"
      verified: true
    - path: tools/check/run-controls.sh
      provides: "the toy-registry drift check, the DEVIATION declaration check, the pass-line guard"
      verified: true
    - path: tools/gate.sh
      provides: "the derived preflight partition with buffered sorted replay"
      verified: true
  key_links:
    - from: tools/check/steps/important-gates.sh
      to: .planning/REQUIREMENTS.md + .planning/milestones/*-REQUIREMENTS.md
      via: "awk over the '## Traceability' rows, '^Complete' prefix, minus the ledger"
      verified: true
    - from: tools/check/run-controls.sh
      to: tools/check/controls/*/overlay/data/expected/important-gates.ids
      via: "sorted non-comment comm in both directions against _good/tree, minus the DEVIATION declaration"
      verified: true
    - from: tools/gate.sh
      to: tools/check/steps/*.sh
      via: "grep -q -e IVS_GATE_DIR -e build/ per step source, printed as gate-partition"
      verified: true
deviations_accepted: 8
open_items: 6
---

# Phase 9: Registered Evidence and Gate Cost — Verification Report

**Phase goal:** The gate's cost level is settled by measurement before the gate grows, and the 17
requirements whose named faults already run in every gate are registered with mechanisms that cannot
pass vacuously — with a machine check that makes the gap unrepeatable.

**Verified:** 2026-11-26
**Status:** passed
**Score:** 5/5 frozen success criteria verified (2 of the 5 met on recorded terms, named below)
**Re-verification:** No — initial verification (no previous `09-VERIFICATION.md` existed)

Everything below was re-measured on the final tree. No SUMMARY claim is quoted as evidence; where a
SUMMARY and the tree agree, the tree is what is cited.

---

## Gates, re-run by the verifier on the final tree

| Shell | Command | Exit | Tail (absolute-and-frozen tokens) |
|---|---|---|---|
| `sh` | `sh tools/gate.sh` | 0 | `gate: PASS steps=38/38 controls=384/384 faults=235/235 fault-runs=235 gates=107 lines=330 goldens=29 reports=30 host-dependent=3 release-only=2 cases=12 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 claims=28 provisional=27 files=2556 tracked=2556 hygiene-files=2556` |
| `dash` | `dash tools/gate.sh` | 0 | the same line; `diff` of the space-split token lists is empty |
| container | `sh tools/gate-linux.sh` | 0 | `gate-linux: exit 0`, `gate: PASS`, `cc=GNU`, `core-imports=5`, `archive=sha256`, `refs-verified=0/4`, `files=tracked=hygiene-files=2556` |

`grep -c '^-- FAIL '` is **0** in all three logs. The two partition lines are the second and third
lines of each local log: `gate-partition preflight=29 producers=9: …` and
`gate-preflight: PASS steps=29/29`. No `ivs-` container was left behind.

This matches the orchestrator's reported tails token for token, including the four container-only
host-dependent fields (`cc`, `core-imports`, `archive`, `refs-verified`) that have been recorded
since Phase 6.

---

## Success Criteria

| # | Criterion | Verdict | Evidence (command → output) |
|---|---|---|---|
| 1 | A hosted row recorded after the lever and trim commits settles the cost level: the row's head is a descendant of those commits, exactly one checkpoint line with one of three words, median `r` recomputed by arithmetic | **met** | **Ancestry:** `git merge-base --is-ancestor <c> 93e6b9f` → exit 0 for all seven A1 commits (`8d28dce`, `dcb50f4`, `f271769`, `0c217ed`, `d7f5ec4`, `0a91804`, `e8a740c`); `git merge-base --is-ancestor 93e6b9f HEAD` → exit 0; and `git merge-base --is-ancestor 93e6b9f 081d441` → exit 0, so the row also precedes the first content commit. **Median:** `tail -5 .planning/ci-runs.tsv \| awk -F'\t' '{printf "%.4f\n",$13/$8}' \| sort -n` → `7.4167 7.5333 7.6957 8.2326 8.3810` → median **7.6957** (7.696). The five rows with their operands: `bd6f2e36` build_s 48 / cpu_s 356 = 7.4167; `3808e6fc` 46 / 354 = 7.6957; `aa96e9a1` 45 / 339 = 7.5333; `10905005` 42 / 352 = 8.3810; `93e6b9f9` (the row, run 45527058543) 43 / 354 = 8.2326. **Checkpoint:** `grep -c '^phaseA1:' .planning/CI-EFFICIENCY.md` → **1**, the line `phaseA1: discharged-below-budget`; the word is one of the three permitted and is the correct one for 7.6957 ≤ 7.8. (The neighbouring `waveA1:` line at :1719 is a different key and explicitly disclaims being the checkpoint.) The CI-EFFICIENCY entry itself prints the same five rows with their `build_s`/`cpu_s` and the median 7.6957, so the recorded figure and the arithmetic agree |
| 2 | No commit before that row adds a test id, control, fault or gate step: 38 steps, `data/expected/gate-steps/` unchanged, ancestry passes | **met on recorded terms** | `ls tools/check/steps/*.sh \| wc -l` → **38**. `git diff --stat 93e6b9f^..HEAD -- data/expected/gate-steps/` → empty; the stronger `912ab9a..HEAD` range is also empty (`git diff --name-only … \| wc -l` → 0). Non-comment row counts of every `data/expected/tests/*.ids` are unchanged from `912ab9a` to HEAD (loop prints nothing) → **no test id was added anywhere in the phase**. `git show --stat` on the seven A1 commits lists only `data/expected/runner-scenarios.ids`, `data/expected/tests/{runner-process,state}.ids`, `tests/runner-process-tap.sh`, `tests/state/{state_util.h,state_util.c,state_restore.c,state_fuzz.c}`, `tools/check/steps/golden.sh` and `.planning/CI-EFFICIENCY.md`; every added line in the three `.ids` files begins with `#` (comment prose only), and no control, fault, gate-step inventory or registry file appears. **The recorded term:** `tools/check/steps/golden.sh` — a step *script* — was modified before the row. It is declared in advance as D-09-01 in `09-01-PLAN.md` with its measured reason (`golden.sh:417-419` refuses a missing report (the plan cited `:402-406` before the change) and a `release-only` id has none by construction) and its predicted token delta; it adds no step (38 unchanged), no control, no fault, no test id and no registry line. The criterion's three measurable conditions all hold |
| 3 | The registry grows 306 → 330 (24 additions) covering FND-01..07, CPU-01..05, MEM-01..05; the registration commit's `steps`/`controls`/`faults`/`tests-*` delta equals its prediction; each of ten toy registries gains one line per new requirement with zero deletions | **met** | `grep -c -v -E '^\s*(#\|$)' data/expected/important-gates.ids` → **330**; distinct first fields → **107**. `git show 081d441^:data/expected/important-gates.ids \| grep -c -v …` → **306** and 90 gate names. Non-comment delta over the registration range: `git diff 081d441^..285807e -- … \| grep -E '^[+-][^+-]' \| grep -v '^[+-]\s*#' \| awk '{print substr($0,1,1)}' \| sort \| uniq -c` → **24 `+`, 0 `-`**. Per-requirement coverage by `awk '{print $4}' \| sort \| uniq -c`: FND-01 2, FND-02 2, FND-03 1, FND-04 2, FND-05 1, FND-06 1, FND-07 1, CPU-01 2, CPU-02 2, CPU-03 1, CPU-04 1, CPU-05 1, MEM-01 2, MEM-02 2, MEM-03 1, MEM-04 1, MEM-05 1 — **all 17 covered, 24 rows**. Toy registries: `git diff --numstat add28f71..285807e -- <file>` → **17 added / 0 deleted on all ten**. Accepted set: `081d441` extended it with FND-01..07 (+7 toy lines, 10 registry rows) and `285807e` with CPU/MEM (+10 toy lines, 14 registry rows), each in its own commit, so 17 toy lines = one per requirement and the tree stayed green at each commit. Token delta: `git diff --name-only 081d441^..285807e` lists only registry files and the step, so `steps`/`controls`/`faults`/`tests-*` deltas are **structurally 0** and `gates`/`lines` are +17/+24 — exactly A-08's prediction; observed at HEAD: `steps=38/38 controls=384/384` (3 controls added later by 09-07) `faults=235/235 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0`. Every one of the 24 ids resolves against an inventory that already ran: rebuilding the step's own `known` set (730 `<mechanism> <id>` lines) and testing each of the 24 → `checked=24 unresolved=0`, and the registration commits touched no `faults/*.ids`, `controls/*.ids` or `tests/*.ids`, so no mechanism was created to make a row resolve |
| 4 | Each of the five gate-hardening items has a named mechanism shown to fail for its intended reason | **met on recorded terms** | All five mechanisms fired in the verifier's own gate run (`sh tools/gate.sh`): ① whole-token `SRC-` — guard `tools/check/steps/sources.sh:48` (`SRC-[A-Za-z0-9_-]+`), control `sources-src-token-whole` (EXPECT `FAIL source-unknown-ref: SRC-bsr-2609_nope` **and** `… SRC-Nope-Doc`, FORBID `sources-no-files`), `control ok`; ② zero-document failures — guards `glossary.sh:48` and `sources.sh:34`, controls `glossary-no-files` and `sources-no-files` (both pre-existing, `data/expected/controls/gate-hardening.ids:2-3`), both `control ok`; ③ per-step pass lines — `tools/check/run-controls.sh:182` (`fail control-pass-line-not-last`) in `good_one`, proved by `tools/check/steps/runner-selftest.sh:162-174`, observed `-- runner-selftest proof control-pass-line-not-last: profile (observed: -- FAIL control-pass-line-not-last: profile)`; ④ manifest output anchoring — guard `tools/check/manifests.cmake:135-139` (two roots, `fixtures/<dirname>/` or `data/`, raised before the digest comparison), control `manifests-output-anchor` (EXPECT `FAIL manifest-format: FX-good/outputs/path`), `control ok`; ⑤ memmap generator — guard `tools/check/steps/memmap.sh:45-63` with `memmap-generator-missing` as its liveness guard, control `memmap-generator-refused` (EXPECT the generator's own reason, FORBID the liveness token), `control ok`, plus `-- runner-selftest proof memmap-generator-missing: cmake/IvsMemmapProbes.cmake (observed)`. **The recorded term (A-02):** the literal "five controls" is not met — two items already had controls and the `tools/gate.sh:111-116` pass-line branch is unreachable from the control runner without recursion, so the count is three new controls, one verification and one step-side mechanism. The deviation, its measured per-item table and its reason are recorded in `09-CONTEXT.md` A-02, `09-06-SUMMARY.md`, `09-07-SUMMARY.md`, `09-09-SUMMARY.md` and `.planning/STATE.md:557`. Every item has a mechanism that can fail, which is the criterion's intent (lesson 20) |
| 5 | The EV-04 machine check refuses a Complete requirement with no registry line and a toy registry that drifts, both shown able to fail, and `sh`/`dash`/container end PASS with the predicted token | **met** | **Derived, not hand-written:** `tools/check/steps/important-gates.sh:80-89` builds `$tmp/complete` with `awk -F'\|' '/^\| *[A-Z]+-[0-9]+ *\|/ { … if (st ~ /^Complete/) print id }'` over `$ROOT/.planning/REQUIREMENTS.md` and every `$ROOT/.planning/milestones/*-REQUIREMENTS.md`, then subtracts `$tmp/excluded`; `grep -n 'nreq = split'` finds nothing (the hand-written string is gone) and `accepted[]` is built at `:131` from the derived set. **Refusals:** `important-gates-complete-uncovered` (`:149`) and `important-gates-excluded-stale` (`:150`), both emitted by the committed control `important-gates-complete-uncovered`, whose EXPECT pins `FAIL important-gates-complete-uncovered: FND-99` **and** `FAIL important-gates-excluded-stale: EV-99` — `control ok` in my run, and the same `CHECK` passes on the pristine toy tree, so the EXPECT lines exist only because of the overlay. **Drift:** `run-controls.sh` compares sorted non-comment lines both ways and refuses `control-toy-drift` / `control-deviation-unused`; `runner-selftest.sh:93-105,118` constructs both cases and asserts the runner emits `-- FAIL control-toy-drift: st-toy-drift` and `-- FAIL control-deviation-unused: st-deviation-unused` — my run printed `-- runner-selftest pass cases=12`, which includes both. **Identity:** `complete=54 excluded=10 registered=44` → **54 − 10 = 44 = registered**. **Step resolution:** `ROOT=$PWD CHECK_DIR=$PWD/tools/check FILELIST=… sh tools/check/steps/important-gates.sh` → `-- important-gates pass gates=107 lines=330`, exit 0, zero findings. **Three gates PASS** with the predicted token (table above) |

**Score: 5/5.** Two criteria (2 and 4) are met on the recorded terms named in their rows; both terms
are pre-declared deviations with their reasons and their measured effects, not post-hoc excuses.

---

## Requirements

| Requirement | Source plan | Verdict | Evidence |
|---|---|---|---|
| **EV-01** — the lever (3) and trims (a), (c), (d) land in cost-only commits with trim (b) declined by measurement, before any new test id/control/fault/gate step, and a hosted row recorded after them settles the cost level | 09-01, 09-02, 09-03 (+ the orchestrator's row) | ✓ SATISFIED | Cost-only: the seven A1 commits touch no test id (test-id row counts unchanged 912ab9a→HEAD), no control, no fault, no gate step, no registry line; `steps=38/38 controls=384/384 faults=235/235` at HEAD with the three controls coming from 09-05/09-07. Row `45527058543` on head `93e6b9f` with `r` 8.2326, ancestry proved both ways, checkpoint `phaseA1: discharged-below-budget`, median 7.6957. Trim (b) declined in writing with its 0.09 local cpu-s measurement (`.planning/CI-EFFICIENCY.md`, wave A1 entry). Marked Complete in `.planning/REQUIREMENTS.md:20,97` |
| **EV-02** — the 17 requirements each have at least one cannot-pass-vacuously mechanism registered, with the accepted set and all ten toy registries extended in the same commit and no new fault or test id | 09-04 | ✓ SATISFIED | 24 rows over 17 requirements (table in SC3), 306 → 330 rows, 90 → 107 gate names, ten toy registries +17/−0, accepted set extended in the same commit as each batch, no fault/control/test inventory file created or modified, 24/24 ids resolve against pre-existing inventories. Marked Complete at `:21,98` |
| **EV-03** — each gate-hardening item has a control that fails for its intended reason | 09-06 (pass line), 09-07 (SRC-, zero-document, manifest, memmap) | ✓ SATISFIED (on the A-02 count: three controls + one verification + one step-side mechanism) | All five mechanisms observed firing in the verifier's run (SC4). The count deviation is recorded with its per-item measurement and its reason in A-02 and in `.planning/STATE.md:557`; `EV-03`'s exclusion row names it (`gate_machinery_self_evidence_three_controls_and_one_verification`). Marked Complete at `:22,99` |
| **EV-04** — a machine check refuses a Complete requirement with no registry line and a toy registry that drifts, both shown able to fail | 09-05, 09-06 | ✓ SATISFIED | Derived set from the records, two-directional ledger refusals, drift + unused-deviation refusals, each proved by a committed control or selftest case; identity 54 − 10 = 44. Marked Complete at `:23,100` |

**Orphaned requirements:** none. `git diff --name-only 912ab9a..HEAD -- .planning/REQUIREMENTS.md`
touches only the four EV rows and their traceability lines; no other requirement was expected of this
phase by the roadmap.

---

## What was automated / handed off / could not be judged by a tool

Per `GUARDRAILS-v0.2.md` §5.6.

**Automated (re-run by the verifier on the final tree, not read from a SUMMARY):**

- Three gate shapes: `sh`, `dash`, and the Ubuntu container gate (`sh tools/gate-linux.sh`), all
  exit 0, `gate: PASS`, zero top-level `-- FAIL ` lines.
- 38 steps × 2 local shells + container; **384/384** negative controls; **235/235** faults
  (`fault-runs=235`); **12** runner-selftest cases including the four proofs the phase relies on.
- The `important-gates` step run standalone: `gates=107 lines=330`, exit 0, zero findings.
- The requirement-ledger identity recomputed with the step's own awk: `complete=54 excluded=10
  registered=44`, `54 − 10 = 44`.
- Ancestry for seven A1 commits against the row and the row against HEAD and against the first
  content commit.
- The last-five median recomputed from `.planning/ci-runs.tsv` and cross-checked against the figure
  the record prints.
- `git diff --numstat` on all ten toy registries; non-comment row delta on the real registry;
  test-id row counts across the whole phase; the phase's file-area inventory.
- The preflight partition re-derived independently: 29 preflight + 9 producers = 38, producer set
  `c-build c-faults c-tests cp1610-vectors faults-verdict golden memmap pkg-install tests-inventory`.
- The evidence-preservation diff: my own `sh` run, normalised over the closed nine-member set, diffed
  against the committed `09-PREFLIGHT-BEFORE-SH.log` → **the only difference is my own `/usr/bin/time`
  wrapper's three lines**; the gate's normalised streams are byte-identical to the pre-change capture.
- Hygiene: `git log --format='%ae' \| sort -u` → only `szTheory@users.noreply.github.com`; no
  absolute home path in any tracked file (`git ls-files -z \| xargs -0 grep -l -E '/Users/…'` → empty);
  no `.bin`/`.rom`/`.ivsc` tracked; `tools/check/binary-ext.txt` and `.gitignore` unchanged across the
  phase; no `data/` path outside `data/expected/` changed; no `data/golden/` change; the single
  `core/` touch is `machine_digest.c` 4/3 lines, comment-only.
- Anti-patterns: no `TBD`/`FIXME`/`XXX` and no `TODO`/`HACK`/`PLACEHOLDER` in any non-planning file
  the phase touched; no stub shape (`return null`, empty arrow body, "not yet implemented") in any
  step or test file the phase touched; no hardcoded-empty data introduced.

**Handed off: nothing.** No item in this phase is a human-only item, and no claim cites one.

- No `<human-check>` block exists in any of the nine `09-0*-PLAN.md` files.
- No UAT file was produced for this phase, and none is owed.
- `data/expected/claims.ids` is unchanged by the phase (**28** rows) and contains no reference to any
  handoff or decision id; `claims=28 provisional=27` in all three gate runs.
- `data/expected/handoffs.ids` — the path `GUARDRAILS` §5.7 names — **does not exist in this tree**
  (it is a v0.1.0-era name). The tree's handoff inventory is
  `data/expected/vector-review-handoff.ids` (12 rows), unchanged by this phase. The check's intent
  ("every H-/D- id mentioned has a row, and no claim cites one") is therefore satisfied vacuously
  here, and that is stated rather than assumed.

**Could not be judged by a tool (and why none of it moves the verdict):**

- **The developer-experience *value* of the 31 s time-to-first-signal.** The latency itself is
  measured (31 s at `gate-preflight: PASS steps=29/29` against 79 s to the previous arrangement's
  last preflight body), but whether that is worth the change is a judgement. It is not one of the
  five frozen criteria, and the phase explicitly does not claim it as a hosted saving.
- **Whether a hosted runner's speed is representative of a developer's machine.** The record states
  the load conditions as taken (lesson 23) instead of pretending the host was idle; the metric is a
  ratio precisely so single-row speed does not decide anything.
- **The one absent measurement** — the alternated `/usr/bin/time -p` before/after for lever (3) and
  trim (a). This is not a judgement call; it is a measurement that was not taken, recorded as open
  with its reason in three places. It is a plan-level preserving-condition gap, not a success
  criterion: SC1 is satisfied by the hosted row and the arithmetic, and the lever's 0.12 local cpu-s
  is labelled a **ceiling** everywhere it appears, never a saving. It is listed under Open items.

Because every *criterion* was judged by a tool and no human-only item exists, the status is
`passed`, not `human_needed`.

---

## Nothing was weakened to go faster

The four trims, each with its preserving condition located in the code (not only in a comment):

| Trim | The trim | Preserving condition, and its guard site |
|---|---|---|
| lever (3), the variant fold | `demo-lamp-tone-shift` / `demo-lamp-tone-swap` are column-5 `release-only` (`data/expected/runner-scenarios.ids:41-42`), run on Release and judged statically otherwise (`tests/runner-process-tap.sh:605-620`) | **The derivation-equality judgement**: `derivation_equal` at `tests/runner-process-tap.sh:455`, with its liveness guard `variant-derivation-empty` at `:464` (an empty derivation block is a refusal, never a pass), the assert-count/header/input-transform checks at `:470-513`, the whole-body equality at `:514`, and the **in-case negative half** `variant_negative` at `:521-545`, called from `static_scenario` at `:560` on every non-Release preset, requiring the one-value stand-in to be rejected for `variant-derivation-differs` (`:539-540`). `tests-skipped` stays 4: both ids print an ordinary `ok` line, so the fold is not a skip. `golden.sh:411-414` skips a `release-only` id **by name** and counts it; the `[ ! -f dv ] \|\| [ ! -f rl ]` → `golden-dev-release-diff` refusal at `:417-419` is untouched for every other id |
| trim (a), the round-trip refusals | `above-max`, `no-value`, `twice` are static judgements (`tests/runner-process-tap.sh:1117-1160`) | **The live `zero` launch** at `:1116`, which alone proves exit 14, the empty report, the first stderr line and the usage text (`:1111-1114`); `result runner-process.roundtrip.refuse-range` is still called exactly once (`grep -c` → 1). Each static judgement names the parser's real guard site and fails if it moves: `runner/main.c:589-592` (arm binds `seen`/`hi`, no `slot`), `:609-610` (missing value), `:613-621` (repeat via `*seen`), `:623-624` (`main_number` range check), plus the `IVS_RUN_TARGET_MAX` macro and its `2^63` comment |
| trim (c), the sampled digests | `tests/state/state_util.c:487` — nine digests computed only when `r == 0 \|\| r % 8 == 0 \|\| r + 1 == n` | **The whole-instance comparison of every row** at `:479` and the `produced` accumulation at `:494`, both **outside** the digest branch (the comment at `:485-486` says so, and the code proves it); the **liveness guard** is `state.restore.reasons-covered` (`tests/state/state_restore.c:1077-1099`), which requires `produced == RS_ALL_REASONS`, i.e. every reason 58–81 produced by some row. Sampling is a second field (`digest_sampled`), so every other case of the family keeps its every-row comparison |
| trim (d), the unsealed flip | `tests/state/state_fuzz.c:931-933` — `if (!sealed && j == 1u) continue;` | **The offset set is unchanged** because the offset loop bound `k < fz.n` (`:925`), `fz_pick` (`:927`) and `masks[]` (`:923`) are untouched, and the resealed path still takes both values. **The "flip moves the digest" property** is enforced operationally by the acceptance refusal at `:954-957` (`if (c.accepted != 0u) … ok = IVS_TEST_FAIL`): with the trailer left as it was, a payload byte the digest does not cover would be accepted and `state.fuzz.byte-flips` would fail. `09-02-SUMMARY.md` records honestly that no purpose-built "moves the digest" mechanism exists in this file and that none was invented to satisfy a plan sentence (lesson 20) |

No control, fault, judge, oracle, golden or pinned-default test was dropped anywhere in the phase:
`controls` rose 380 → 384, `faults` stayed 235/235 with `fault-runs=235`, `goldens=29`,
`tests-expected/executed/skipped` are 4958/4954/4 throughout, and no `.github/` file changed (no
`continue-on-error`, matrix or path filter was added).

---

## The preflight partition preserves evidence

| Check | Command | Result |
|---|---|---|
| Committed before-capture exists | `ls -l .planning/phases/09-…/09-PREFLIGHT-BEFORE-{SH,DASH}.log` | both present, 20 926 bytes, **518** lines each, last line `gate: PASS …`, added by `fe09966` (which does **not** touch `tools/gate.sh`) |
| Normalisation is closed | `grep -c 'step-time \.\.\.\|gate-cpu \.\.\.\|gate-time \.\.\.\|gate-env \.\.\.'` on the capture → 44; `grep -o -E '(ci-runs\|files\|tracked\|hygiene-files\|history-commits)=N' \| sort \| uniq -c` | all nine members present: the four measurement line shapes plus `ci-runs=N files=N tracked=N hygiene-files=N history-commits=N`, 2 occurrences each for the five inline ones |
| The partition is derived | `grep -n -e 'IVS_GATE_DIR' -e 'build/' tools/gate.sh` | `:121` — `if grep -q -e 'IVS_GATE_DIR' -e 'build/' "$CHECK_DIR/steps/$name.sh"; then … producers … else … preflight`; no hand-maintained list |
| `preflight + producers = 38` | independent re-derivation over `tools/check/steps/*.sh` | 29 + 9 = **38**; producers `c-build c-faults c-tests cp1610-vectors faults-verdict golden memmap pkg-install tests-inventory`; the run prints `gate-partition preflight=29 producers=9` and `gate-preflight: PASS steps=29/29` as lines 2 and 3 |
| Output preserved | my `sh` run, normalised with the same closed function, diffed against the committed capture | **empty diff** apart from my own `/usr/bin/time` wrapper's three lines (`real/user/sys`), which the gate does not emit |

---

## Anti-patterns found

None. `TBD`/`FIXME`/`XXX`: none in any non-planning file the phase touched (the debt-marker gate is
clean, so no blocker). `TODO`/`HACK`/`PLACEHOLDER`: none. Stub shapes in new or modified step/test
files: none. Hardcoded-empty rendering data: not applicable — this phase renders nothing; its added
files are registry rows, control declarations, inventory comments and shell checkers.

One anti-pattern was found and **fixed during the phase rather than left for verification**:
`d3ff4bd` (below) stopped the pass-line proof from printing a line matching `^-- FAIL `, which
`tools/gate.sh`'s `keep_fails` would have advertised as a failure of a passing run. My run confirms
the fix: the proof prints as `-- runner-selftest proof control-pass-line-not-last: profile
(observed: …)` and `grep -c '^-- FAIL '` on all three logs is 0.

---

## Deviations accepted

| Id | Deviation | Why it is accepted | Verified in the tree |
|---|---|---|---|
| **D-09-01** | `tools/check/steps/golden.sh` joined wave A1's closed file list | Measured: `golden.sh:402-406` as the plan recorded it before the change — `:417-419` on this tree — refuses `golden-dev-release-diff` for any id with one missing report, and a `release-only` scenario has no dev report by construction. Declared before the commit with its predicted delta. It is a skip **by name** plus a count, not a loosening: the missing-report refusal is untouched for every other id | `golden.sh:411-419`; my run reads `reports=30 release-only=2`, the predicted `32 → 30` |
| **D-09-02** | Trim (a) judged against `runner/main.c`'s guard sites, not "the option table" | Measured: no option table exists in this tree — the parser is a `strcmp` chain at `runner/main.c:588-624`, and `runner/exec.c` contains neither `roundtrip-at-ticks` nor either refusal message. PLAN-LESSONS 19: reality wins, the plan sentence becomes a deviation | `tests/runner-process-tap.sh:1117-1160` names `:589-592`, `:609-610`, `:613-621`, `:623-624` |
| **D-09-05** | The EV status flips land **with** their exclusion rows, in 09-09's commit | The ledger is two-directional, so either half alone turns the tree red; the ordering is proved by the refusals observed in tree (`important-gates-complete-uncovered: EV-02` in 09-05, `: EV-03`/`: EV-04` in 09-06's orchestrator re-run) and pinned by the committed control's `excluded-stale` token | `data/expected/important-gates-excluded.ids` has 10 rows; identity `54 − 10 = 44` |
| **D-09-06** | Nine new must-fail mechanisms carry **no** registry row | `lines=330` is frozen by SC3 and A-08, so any row after 09-04 would make the phase end at a count its own criterion forbids. The other two limbs of GUARDRAILS 2.3 (liveness guard, committed re-runnable stand-in) are met. The deferral names all nine and carries a reversal condition | `grep -c -F <m> data/expected/important-gates.ids` → **0** for all nine; `.planning/STATE.md:558` names all nine individually with the reason and the reversal condition |
| **A-02's five-controls deviation** | EV-03's count is three new controls, one verification and one step-side mechanism, not five controls | Measured per item: two already had controls, and the `tools/gate.sh:111-116` pass-line branch cannot be reached from `run-controls.sh` (which only executes `tools/check/steps/*.sh`; a `CHECK gate` control would recurse). A control aimed at an unreachable guard is lesson 20's check that cannot fail | All five mechanisms observed firing (SC4) |
| **09-07's `SRC-` regex correction** | The class is `SRC-[A-Za-z0-9_-]+`, not the research's `SRC-[A-Za-z0-9_.-]*[A-Za-z0-9]` | Measured over the real tree: the research's class extracts **three unregistered tokens** (`SRC-gi-1980-catalog.pdf`, `SRC-gi-1982-catalog.pdf`, `SRC-gi-cp1600-um-1975.pdf`) and would turn the gate red; no registry id contains a dot | `sources.sh:48`, with the measurement in the comment above it (`:43`) |
| **Budget deviations: no container gate inside any executor dispatch** | 09-04, 09-05, 09-06, 09-07, 09-08 and 09-09 each record `container gate: not run` with its reason (the 1800 s dispatch limit) rather than reporting a pass they did not observe | The container gate is a wave/phase-boundary action owned by the orchestrator; lesson 24 forbids a fabricated pass | The orchestrator ran it, and the **verifier re-ran it**: `sh tools/gate-linux.sh` → exit 0, `gate: PASS`, `cc=GNU`. The gap is closed |
| **`d3ff4bd`, the orchestrator's post-hoc fix** | The `st-pass-line-not-last` proof printed a bare `-- FAIL …` line, which `keep_fails` collected into the job summary as a failure of a passing run; and `EV-03`/`EV-04` had been marked Complete without their exclusion rows, firing `important-gates-complete-uncovered` on the pristine tree | Found by re-running the gate **after** the closing records commit — the defect class PLAN-LESSONS 1 describes one step further out. Both halves fixed in one commit | `git show --stat d3ff4bd` → `runner-selftest.sh` (12 +/3 −) and `.planning/REQUIREMENTS.md` (8 +/4 −); my run shows the proof line in its new shape and 0 `^-- FAIL ` lines |

---

## Open items

| # | Item | State | Where it is recorded |
|---|---|---|---|
| 1 | The **alternated `/usr/bin/time -p` before/after capture** for lever (3) and trim (a) was never taken (two executors stopped by the 1800 s dispatch limit) | Open, with its reason, **not** closed by a figure that was not taken. What stands in its place is measured: per-launch medians of five (dev 62 ms, release 9.4–9.8 ms) that the 0.12 local cpu-s **ceiling** is arithmetic over, the post-fold suite times (21.55 s dev / 22.69 s release), and the structural proof the two dev launches are gone (`reports=32 → 30`, `release-only=2`, the `# mutants:` two-mode lines). The lever is labelled a ceiling and never a saving | `09-01-SUMMARY.md`, `09-03-SUMMARY.md`, `.planning/CI-EFFICIENCY.md` ("The open item, recorded rather than dropped") |
| 2 | `denylist-no-files` (`tools/check/steps/denylist.sh:43`) and `runner-scan-empty` (`tools/check/steps/runner-scan.sh:58`) have no control | **Declined, not silently dropped**: A-02's honest count for EV-03 is three controls plus one verification plus one step-side mechanism, and two more controls is scope the phase did not price | `.planning/STATE.md:557` (row with its reason and its milestone) |
| 3 | The **nine** D-09-06 mechanisms have no `data/expected/important-gates.ids` row: `important-gates-complete-uncovered`, `important-gates-excluded-stale`, `control-toy-drift`, `control-deviation-unused`, `control-pass-line-not-last`, `sources-src-token-whole`, `manifests-output-anchor`, `memmap-generator-refused`, `memmap-generator-missing` | Registration deferred against the frozen `lines=330`, with the reversal condition stated (a later milestone re-derives the baseline, then adds the nine rows under EV-03/EV-04 with the non-vacuity sentence every other row carries). Verified: **0** registry mentions of all nine | `.planning/STATE.md:558`; `09-05-SUMMARY.md` is the canonical list |
| 4 | Trim (d)'s "the flip moves the digest" is not a purpose-built mechanism | Recorded as a plan-sentence deviation: the property is enforced operationally by the acceptance refusal at `tests/state/state_fuzz.c:954-957`; no hollow guard was invented (lesson 20) | `09-02-SUMMARY.md` deviation 2 |
| 5 | Phase 9's own `09-VALIDATION.md` remains `status: draft` (`nyquist_compliant: false`) | Consistent with the standing disposition of `02`–`08` (cited to `08-ACCEPTED-DEBT.md` §1 in `.planning/STATE.md`), and not one of the five frozen criteria. Named here so a later reader does not discover it as a surprise | `.planning/STATE.md` Deferred Items (seven `Validation` rows) |
| 6 | Wave A2's cost is only partly priced plan by plan: 09-09 records that "at least 1.59 local cpu-s of the 12.9 recorded allowance is measured" and that 09-06 to 09-09 each changed a count the gate reads without pricing it | Recorded as a lesson rather than filled with an estimate. No success criterion depends on it: the hosted row that settles the level is SC1's, and it reads `discharged-below-budget` with 0.604 `r` of headroom to the 8.3 stop line | `09-09-SUMMARY.md` "Cost" |

---

## Gaps summary

No gaps. Every frozen success criterion is met by measurement on the final tree; the two criteria
met "on recorded terms" name their terms and the terms are pre-declared deviations with measured
reasons, not discoveries made here. Nothing in this phase is a human-only item and no claim cites
one. The gate's cost level is settled by a hosted row whose ancestry and arithmetic both reproduce;
the registry grew by exactly the predicted 24 rows over 107 gate names with 17 toy lines per file and
zero deletions; all five gate-hardening items have a mechanism that fired in the verifier's own run;
and the EV-04 check derives its must-be-covered set from the requirement records, refuses in both
directions, is shown able to fail by committed controls and selftest cases, and closes its identity
at `54 − 10 = 44`.

---

_Verified: 2026-11-26_
_Verifier: gsd-verifier_
