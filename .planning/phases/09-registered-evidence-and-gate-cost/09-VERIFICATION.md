---
phase: 09-registered-evidence-and-gate-cost
verified: 2026-11-26T15:00:00Z
status: passed
score: 5/5 must-haves verified
verifier: gsd-verifier
phase_base: 912ab9a
# The two measurements below are the INITIAL verification's (tree 86c7878). The final tree's are in
# re_verification: tree 768e97e, phase_span_final, gates_rerun_on_final_tree, covered_digest.
tree_at_initial_verification: 86c7878 (+ the pre-existing dirty .planning/state.json, untouched)
phase_span_at_initial_verification: "912ab9a..86c7878 = 39 commits, 81 files, 4875 insertions, 140 deletions"
phase_span_final: "912ab9a..768e97e = 44 commits, 94 files, 5910 insertions, 144 deletions"
gates_rerun_at_initial_verification:
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
open_items: 7
re_verification:
  previous_status: passed
  previous_score: 5/5
  previous_verified: 2026-11-26T03:10:00Z
  previous_tree: 86c7878
  tree: 768e97e
  commits:
    - 0d84484 # WR-01, the exclusion ledger's declared row format is validated
    - 9f5e08c # WR-03, the release-only count is derived from the inventory
    - ba34aba # WR-02, the pass-line rule runs in the gate replay, all 38 steps
  gaps_closed: []
  gaps_remaining: []
  regressions: []
  status_unchanged: true
  verdict: "passed on the final tree - the three review fixes add mechanisms and weaken none; one bookkeeping defect (the D-09-06 count still reads nine where the tree holds ten) is recorded as open item 7, not absorbed into the pass"
  gates_rerun_on_final_tree:
    sh: "exit 0, gate: PASS steps=38/38 controls=386/386 faults=235/235 fault-runs=235 gates=107 lines=330 goldens=29 reports=30 host-dependent=3 release-only=2 cases=13 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 claims=28 provisional=27 files=2568 tracked=2568 hygiene-files=2568, 0 top-level '-- FAIL ' lines"
    dash: "exit 0, gate: PASS, token list byte-identical to sh (diff of the space-split tokens empty), 0 top-level '-- FAIL ' lines"
    linux: "exit 0, gate-linux: exit 0, gate: PASS, cc=GNU, core-imports=5, archive=sha256, refs-verified=0/4, files=tracked=hygiene-files=2568, controls=386/386, cases=13, 0 top-level '-- FAIL ' lines, no ivs- container or volume left behind"
  token_delta_vs_initial: "controls 384/384 -> 386/386, cases 12 -> 13, files/tracked/hygiene-files 2556 -> 2568 (+12 files from the fix commits); every other token byte-identical"
  output_order_unchanged: "normalised diff of the final sh log against the committed pre-change capture 09-PREFLIGHT-BEFORE-SH.log differs only by the 2 partition lines, the 1 new proof line, cases 12->13, the 2 new 'control ok' lines in sorted position, controls 384->386, and the /usr/bin/time wrapper's 3 lines"
covered_files:
  - .planning/phases/09-registered-evidence-and-gate-cost/09-01-PLAN.md
  - .planning/phases/09-registered-evidence-and-gate-cost/09-01-SUMMARY.md
  - .planning/phases/09-registered-evidence-and-gate-cost/09-02-PLAN.md
  - .planning/phases/09-registered-evidence-and-gate-cost/09-02-SUMMARY.md
  - .planning/phases/09-registered-evidence-and-gate-cost/09-03-PLAN.md
  - .planning/phases/09-registered-evidence-and-gate-cost/09-03-SUMMARY.md
  - .planning/phases/09-registered-evidence-and-gate-cost/09-04-PLAN.md
  - .planning/phases/09-registered-evidence-and-gate-cost/09-04-SUMMARY.md
  - .planning/phases/09-registered-evidence-and-gate-cost/09-05-PLAN.md
  - .planning/phases/09-registered-evidence-and-gate-cost/09-05-SUMMARY.md
  - .planning/phases/09-registered-evidence-and-gate-cost/09-06-PLAN.md
  - .planning/phases/09-registered-evidence-and-gate-cost/09-06-SUMMARY.md
  - .planning/phases/09-registered-evidence-and-gate-cost/09-07-PLAN.md
  - .planning/phases/09-registered-evidence-and-gate-cost/09-07-SUMMARY.md
  - .planning/phases/09-registered-evidence-and-gate-cost/09-08-PLAN.md
  - .planning/phases/09-registered-evidence-and-gate-cost/09-08-SUMMARY.md
  - .planning/phases/09-registered-evidence-and-gate-cost/09-09-PLAN.md
  - .planning/phases/09-registered-evidence-and-gate-cost/09-09-SUMMARY.md
  - data/expected/controls/golden.ids
  - data/expected/controls/important-gates.ids
  - tools/check/controls/_good/tree/.planning/REQUIREMENTS.md
  - tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md
  - tools/check/controls/_good/tree/data/expected/important-gates-excluded.ids
  - tools/check/controls/golden-release-only-no-report/CHECK
  - tools/check/controls/golden-release-only-no-report/EXPECT
  - tools/check/controls/golden-release-only-no-report/FORBID
  - tools/check/controls/golden-release-only-no-report/MAKE
  - tools/check/controls/golden-release-only-no-report/overlay/data/expected/runner-scenarios.ids
  - tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md
  - tools/check/controls/important-gates-complete-uncovered/overlay/data/expected/important-gates-excluded.ids
  - tools/check/controls/important-gates-excluded-malformed/CHECK
  - tools/check/controls/important-gates-excluded-malformed/EXPECT
  - tools/check/controls/important-gates-excluded-malformed/FORBID
  - tools/check/controls/important-gates-excluded-malformed/overlay/data/expected/important-gates-excluded.ids
  - tools/check/pass-line.sh
  - tools/check/steps/golden.sh
  - tools/check/steps/important-gates.sh
  - tools/check/steps/runner-selftest.sh
  - tools/gate.sh
covered_digest: "v4:sha256:8b8e6a20c5c304785156dca59c5ac6bad0502ba76f917a818277d6b3093a52a7"
---

# Phase 9: Registered Evidence and Gate Cost — Verification Report

**Phase goal:** The gate's cost level is settled by measurement before the gate grows, and the 17
requirements whose named faults already run in every gate are registered with mechanisms that cannot
pass vacuously — with a machine check that makes the gap unrepeatable.

**Verified:** 2026-11-26
**Status:** passed
**Score:** 5/5 frozen success criteria verified (2 of the 5 met on recorded terms, named below)
**Re-verification:** Yes — after the three code-review fixes (WR-01, WR-02, WR-03). The
initial verification's measurements are kept as recorded below; the final tree's re-measurement is
in [Re-verification after the review fixes](#re-verification-after-the-review-fixes).

Everything below was re-measured on the final tree. No SUMMARY claim is quoted as evidence; where a
SUMMARY and the tree agree, the tree is what is cited.

---

## Gates, re-run by the verifier at the initial verification

> This section records the **initial** verification's run on tree `86c7878` (`controls=384/384`,
> `cases=12`). The three review fixes moved exactly two fields; the final tree's runs on `768e97e`
> (`controls=386/386`, `cases=13`) are in
> [Re-verification after the review fixes](#re-verification-after-the-review-fixes).

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
| **D-09-06** | Nine new must-fail mechanisms carry **no** registry row (**ten** on the final tree — WR-01 added `important-gates-excluded-malformed`; see open item 7) | `lines=330` is frozen by SC3 and A-08, so any row after 09-04 would make the phase end at a count its own criterion forbids. The other two limbs of GUARDRAILS 2.3 (liveness guard, committed re-runnable stand-in) are met. The deferral names all nine and carries a reversal condition | `grep -c -F <m> data/expected/important-gates.ids` → **0** for all nine; `.planning/STATE.md:558` names all nine individually with the reason and the reversal condition |
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
| 3 | The **nine** D-09-06 mechanisms have no `data/expected/important-gates.ids` row: `important-gates-complete-uncovered`, `important-gates-excluded-stale`, `control-toy-drift`, `control-deviation-unused`, `control-pass-line-not-last`, `sources-src-token-whole`, `manifests-output-anchor`, `memmap-generator-refused`, `memmap-generator-missing` | Registration deferred against the frozen `lines=330`, with the reversal condition stated (a later milestone re-derives the baseline, then adds the nine rows under EV-03/EV-04 with the non-vacuity sentence every other row carries). Verified: **0** registry mentions of all nine. **The list is ten on the final tree** — WR-01 added `important-gates-excluded-malformed`, also unregistered (0 mentions); the count that needs correcting in the records is open item 7 | `.planning/STATE.md:558`; `09-05-SUMMARY.md` is the canonical list |
| 4 | Trim (d)'s "the flip moves the digest" is not a purpose-built mechanism | Recorded as a plan-sentence deviation: the property is enforced operationally by the acceptance refusal at `tests/state/state_fuzz.c:954-957`; no hollow guard was invented (lesson 20) | `09-02-SUMMARY.md` deviation 2 |
| 5 | Phase 9's own `09-VALIDATION.md` remains `status: draft` (`nyquist_compliant: false`) | Consistent with the standing disposition of `02`–`08` (cited to `08-ACCEPTED-DEBT.md` §1 in `.planning/STATE.md`), and not one of the five frozen criteria. Named here so a later reader does not discover it as a surprise | `.planning/STATE.md` Deferred Items (seven `Validation` rows) |
| 6 | Wave A2's cost is only partly priced plan by plan: 09-09 records that "at least 1.59 local cpu-s of the 12.9 recorded allowance is measured" and that 09-06 to 09-09 each changed a count the gate reads without pricing it | Recorded as a lesson rather than filled with an estimate. No success criterion depends on it: the hosted row that settles the level is SC1's, and it reads `discharged-below-budget` with 0.604 `r` of headroom to the 8.3 stop line | `09-09-SUMMARY.md` "Cost" |
| 7 | **Introduced by the review fixes**: deviation D-09-06's count still reads **nine** mechanisms where the tree now holds **ten**. `.planning/STATE.md:558` says "the **nine** mechanisms Phase 9 landed" and enumerates nine; `09-05-SUMMARY.md:214` — the file `STATE.md` names as the canonical list — says "nine mechanisms and no fewer" and enumerates nine. WR-01 added `important-gates-excluded-malformed`, a must-fail mechanism with no registry row (verified: `grep -c -F important-gates-excluded-malformed data/expected/important-gates.ids` → 0), so the deferral covers ten | **Open, disclosed, and named here rather than absorbed into the pass.** The deviation's substance is unchanged and still correct: registration is deferred against the frozen `lines=330` (ROADMAP criterion 3, amendment A-08), which still holds, and the other two GUARDRAILS 2.3 limbs are met for the tenth mechanism too. `09-REVIEW.md`'s `## Fixes applied` does name the tenth and says it extends the list to ten, so the fact is disclosed somewhere; what is stale is the standing-gap row a later milestone reads. **Fix:** add `important-gates-excluded-malformed` to the enumeration and change nine → ten in both records, and "these nine rows" → "these ten rows". Not done here because this re-verification is confined to `09-VERIFICATION.md` | `.planning/STATE.md:558`; `09-05-SUMMARY.md:214`; `09-REVIEW.md` "Fixes applied" |

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

---

## Re-verification after the review fixes

**Tree:** `768e97e` — the initially-verified tree `86c7878` plus `0d84484` (WR-01), `9f5e08c`
(WR-03), `ba34aba` (WR-02) and the two records commits `f8800c0`, `768e97e`. The working tree holds
only the pre-existing `.planning/state.json` timestamp change, untouched, as at the initial
verification.
**Re-verified:** 2026-11-26 (the record's existing convention; the authoritative identity of what was
measured is the commit `768e97e` and `covered_digest` in the frontmatter, not the clock)
**Verdict:** **unchanged — `passed`, 5/5.** Each of the three fixes adds a mechanism; none removes,
narrows or replaces one. One bookkeeping defect was introduced alongside them and is recorded as
**open item 7** below rather than passed over.

### The three gates, re-run by the verifier on the final tree

| Shell | Command | Exit | Tail (absolute-and-frozen tokens) |
|---|---|---|---|
| `sh` | `sh tools/gate.sh` | 0 | `gate: PASS steps=38/38 controls=386/386 … cases=13 … goldens=29 reports=30 host-dependent=3 release-only=2 … gates=107 lines=330 … tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0 claims=28 provisional=27 files=2568 tracked=2568 hygiene-files=2568 … tiers=8` |
| `dash` | `dash tools/gate.sh` | 0 | the same line; `diff` of the space-split token lists is **empty** |
| container | `sh tools/gate-linux.sh` | 0 | `gate-linux: exit 0`, `gate: PASS`, `cc=GNU`, `core-imports=5`, `archive=sha256`, `refs-verified=0/4`, `files=tracked=hygiene-files=2568`, `controls=386/386`, `cases=13` |

The full `sh` tail, verbatim, is the same 100-field line the initial verification recorded with the
two fields the fixes moved changed and nothing else:

```
gate: PASS steps=38/38 controls=386/386 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120
cc=AppleClang c-presets=2 fault-runs=235 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=78
ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74
patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291
policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12
scanned=666 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=235/235
tracked=2568 binaries-listed=0 terms=9 doc-files=59 profile-keys=154 goldens=29 reports=30
host-dependent=3 release-only=2 mode=tree hygiene-files=2568 history-commits=1 gates=107 lines=330
input-rows=31 input-policies=11 input-profile-fields=31 files=2568 listings=7 listing-words=1386
manifests=11 effective-yes=8 effective-no=3 literal-rows=80 views=2 dump-lines=160 prefix-files=11
archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok
pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120
psg-policies=12 release-selftest=11/11 runner-files=16 cases=13 sources=12 refs-verified=3/4
state-rows=88 state-cuts=18 tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0
docs=60 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18
video-frame-pairs=9 tags=10 tiers=8
```

`grep -c '^-- FAIL '` is **0** in all three logs. `sh` and `dash` are 526 lines each; the container
log is 524, and the whole difference is accounted for: it carries one extra line of its own (the
`gate-linux: x86_64, gcc (Ubuntu 13.3.0…)` banner) and lacks my `/usr/bin/time` wrapper's three
`real`/`user`/`sys` lines, which the gate does not emit. After normalising the nine measurement
members, the two streams are otherwise identical. `build/gate-linux.log` was copied before anything
else ran; `docker ps -a` and `docker volume ls` show **no** `ivs-` container and **no** `ivs-` volume
left behind.

**Token delta against the initial verification — exactly the predicted one:**

| Field | Initial (`86c7878`) | Final (`768e97e`) | Cause |
|---|---|---|---|
| `controls` | `384/384` | **`386/386`** | +2: `important-gates-excluded-malformed` (WR-01), `golden-release-only-no-report` (WR-03) |
| `cases` | `12` | **`13`** | +1: the `st-gate-pass-line` case (WR-02) |
| `files` / `tracked` / `hygiene-files` | `2556` | **`2568`** | +12 files: 4 + 4 control declarations, 3 overlay files, `tools/check/pass-line.sh`, 2 `_good` record files (verified: `git diff --name-status 86c7878..HEAD` lists exactly 12 `A`) |
| every other token | — | — | byte-identical, including `steps=38/38`, `faults=235/235`, `fault-runs=235`, `gates=107`, `lines=330`, `goldens=29`, `reports=30`, `release-only=2`, `host-dependent=3`, `tests-*`, `claims=28`, `provisional=27`, `tiers=8` |

### Output order and the summary token are unchanged (WR-02)

The decisive check is a normalised diff of my `sh` log against the committed pre-change capture
`09-PREFLIGHT-BEFORE-SH.log`, using the same closed nine-member normalisation the initial
verification used (`step-time`/`gate-cpu`/`gate-time`/`gate-env` lines and the five inline
`ci-runs=`/`files=`/`tracked=`/`hygiene-files=`/`history-commits=` values). The **complete** diff is:

| Diff | Lines | Attribution |
|---|---|---|
| added | `gate-partition preflight=29 producers=9: …` and `gate-preflight: PASS steps=29/29` | the phase's own partition lines (09-08); the capture predates the partition, so these are not from the fixes |
| added | `-- runner-selftest proof pass-line-not-last: profile (exit 2 observed)` | WR-02's stand-in, inside `runner-selftest`'s own buffered output |
| changed | `-- runner-selftest pass cases=12` → `cases=13` | the same case |
| added | `control ok   golden-release-only-no-report`, `control ok   important-gates-excluded-malformed` | the two new controls, in **sorted** position within the controls output |
| changed | `controls=384/384` → `controls=386/386` | the same two controls |
| changed | the summary token's `controls=` and `cases=` fields | the same |
| added | `real` / `user` / `sys` | my own `/usr/bin/time` wrapper, which the gate does not emit |

Nothing else differs: no step's output changed, no line moved, and `keep_fails` still runs **inside**
the replay (`tools/gate.sh:167-168`, before the pass/fail branch). `tools/check/pass-line.sh` prints
nothing on exit 0, which is why a green run gains no line from the rule itself. `sh` and `dash`
produce identical token lists.

### The five frozen success criteria, re-checked on the final tree

| # | Criterion | Verdict | Re-measurement |
|---|---|---|---|
| 1 | The hosted row settles the cost level | **met** | Untouched by the fixes (`git diff --name-only 86c7878..HEAD -- .planning/ci-runs.tsv .planning/CI-EFFICIENCY.md` → 0). Re-ran the arithmetic: `tail -5 .planning/ci-runs.tsv \| awk -F'\t' '{printf "%.4f\n",$13/$8}' \| sort -n` → `7.4167 7.5333 7.6957 8.2326 8.3810`, median **7.6957** ≤ 7.8; `grep -c '^phaseA1:'` → **1**, `phaseA1: discharged-below-budget`; `git merge-base --is-ancestor 93e6b9f HEAD` and `… 93e6b9f 081d441` both exit 0 |
| 2 | No commit before that row added a test id, control, fault or gate step | **met on recorded terms** (D-09-01, unchanged) | `ls tools/check/steps/*.sh \| wc -l` → **38**; `git diff --name-only 912ab9a..HEAD -- data/expected/gate-steps/` → **0 files**; `tests-expected/executed/skipped` still `4958/4954/4`. The three fix commits add **2 controls and 1 selftest case**, which is a change *after* the row, not before it, so this criterion's subject (what preceded the row) is untouched |
| 3 | Registry 306 → 330 rows, 107 gate names, 17 toy lines per file | **met** | `data/expected/important-gates.ids` is **unchanged since `285807e`** (`git diff --name-only 285807e..HEAD` → 0), so `lines=330` and `gates=107` hold by construction and the gate reads them: **330** non-comment rows, **107** distinct gate names |
| 4 | Each of the five gate-hardening items has a named mechanism shown to fail for its intended reason | **met on recorded terms** (A-02, unchanged) — and **strengthened, not weakened** | All five still fire in my run (the same proof and `control ok` lines as the initial verification). The fixes close the two edges the reviewer named: the pass-line rule now runs in the replay and covers all 38 steps (WR-02, so the five producers no longer keep only "a pass line exists somewhere"), and each new mechanism has a pinned token, a control or case that fails for its intended reason, and a committed re-runnable stand-in — all three shown live below |
| 5 | The EV-04 machine check derives its set from the records, refuses both directions and a drift, `Complete − excluded = registered` | **met** — and the ledger now enforces its own declared format | Derivation still reads the records (`important-gates.sh:86-93`, `awk` over `## Traceability` rows of `REQUIREMENTS.md` and every `milestones/*-REQUIREMENTS.md`; no hand-written id string). Recomputed independently: `complete=54 excluded=10 accepted=44`, all **44** accepted requirements have a registry row, **0** without one, **0** stale exclusion rows → `54 − 10 = 44`. Standalone step: `-- important-gates pass gates=107 lines=330`, exit 0, zero findings. The malformed-row probe is below |

### The ledger now enforces its declared row format (WR-01) — probed, not read

`important-gates.sh:99-121` runs one `awk` pass over the ledger: a row must be exactly two fields,
`$1` shaped `^[A-Z][A-Z0-9]*-[0-9]+$`, `$2` at least three underscore-joined words with no leading,
trailing or doubled underscore, and `$2 != "none"`. A row that fails is printed as
`MALFORMED line <n>: <row>`, kept **out** of the exclusion set, and emitted as
`important-gates-excluded-malformed`. Probes on a symlinked copy of the real tree (the real ledger
restored afterwards; `git status` clean):

| Probe | Ledger edit | Observed | Exit |
|---|---|---|---|
| baseline | none | `-- important-gates pass gates=107 lines=330` | 0 |
| B (the reviewer's) | append a bare `API-02` | `-- FAIL important-gates-excluded-malformed: line 37: API-02` | 1 |
| C (the reviewer's) | append `API-02 none` | `-- FAIL important-gates-excluded-malformed: line 37: API-02 none` | 1 |
| D | append `API-02 a_b` (two words) | `-- FAIL important-gates-excluded-malformed: line 37: API-02 a_b` | 1 |
| E | append a **well-formed** `API-02 close_record_and_release_only_…` | `-- FAIL important-gates-unknown-id: line 16` … `line 24` (nine lines) | 1 |

Probe B is the reviewer's probe, and it now refuses instead of printing `pass`: the one-word edit
that used to silence a Complete requirement is a red gate. Probe E is the case the reviewer did not
raise and is worth recording: a **well-formed** exclusion row for a requirement that already carries
registry rows is refused too, because it removes API-02 from the derived accepted set and every one
of API-02's nine registry lines (`important-gates.ids:16-24`) becomes `important-gates-unknown-id`.
So the ledger cannot quietly un-register a covered requirement in either shape.

The accepting half is exercised by the `_good` positive control, which is the reason WR-01 also had
to add a well-formed row to it: `_good`'s ledger holds exactly one row,
`FND-98 close_record_only_toy_positive_row_for_the_ledger_format`, for a toy requirement that is
`Complete` and unregistered. Run standalone: `-- important-gates pass gates=43 lines=44`, exit 0 —
the `gates=43 lines=44` the review recorded. A validator that refused **every** row would make FND-98
uncovered and turn `_good` red, so the positive control exercises both halves. Adding a malformed row
to `_good` (`FND-97`) gives `-- FAIL important-gates-excluded-malformed: line 15: FND-97`, exit 1.

### Every new mechanism has a pinned token, a failing control or case, and a live stand-in

| Fix | Token | Control / case that makes it fail for its intended reason | Observed in my run | Stand-in is live (mutant check) |
|---|---|---|---|---|
| WR-01 | `important-gates-excluded-malformed` (**new**) | `tools/check/controls/important-gates-excluded-malformed` — CHECK `important-gates`, EXPECT `FAIL important-gates-excluded-malformed: line 7: API-02` **and** `… line 8: API-03 none`, FORBID `important-gates-complete-uncovered`, `important-gates-excluded-stale`, `important-gates-uncovered-gate`, `important-gates-unknown-id`; registered at `data/expected/controls/important-gates.ids:26` | `control ok   important-gates-excluded-malformed`; `control ok   _good/important-gates` | yes — the malformed `FND-97` probe above refuses, and `_good` accepts the well-formed row |
| WR-03 | `golden-dev-release-diff` (**reused** — it already means "a report of one preset is missing"; the detail names the id) | `tools/check/controls/golden-release-only-no-report` — CHECK `golden`, MAKE `delete build/gate/release/runner/toy.report`, EXPECT `FAIL golden-dev-release-diff: toy (a release-only scenario has no release report)`, FORBID `golden-mismatch` **and** `-- golden pass`; registered at `data/expected/controls/golden.ids:31` | `control ok   golden-release-only-no-report`; `control ok   _good/golden` | yes — the FORBID `-- golden pass` is exactly the pre-fix output the reviewer saw (`release-only=1`), so the control cannot pass on the old behaviour |
| WR-02 | `gate-step-no-pass-line` (**reused**, detail names the shape; `tools/gate.sh` has no token inventory of its own to pin a second name in) | `tools/check/steps/runner-selftest.sh:182-202` — feeds `tools/check/pass-line.sh` three outputs and requires exit 0, exit 2 and exit 1 | `-- runner-selftest proof pass-line-not-last: profile (exit 2 observed)`; `-- runner-selftest pass cases=13` | yes — I ran the case's three assertions against four rules: the real one passes; `always-exit-0`, `always-exit-2`, and the **pre-fix shape** (`grep` for the pass line anywhere) each make the assertions fail |

The three mutants in the WR-02 row matter: the third one is the reviewer's finding reproduced as a
mutant, so the case does not merely check "some non-zero exit happens" — it rejects the exact weaker
rule the gate used to apply.

### Nothing was weakened by the fixes

| Check | Command | Result |
|---|---|---|
| The four lamp-tone scenarios behave as before | `grep -n 'lamp-tone' data/expected/runner-scenarios.ids` | `demo-lamp-tone-pass` / `-idle` `deterministic`, `demo-lamp-tone-shift` / `-swap` **`release-only`** — unchanged, and `data/expected/runner-scenarios.ids` is not in `86c7878..HEAD` |
| The release-only count did not shrink | gate token, all three shapes | `reports=30`, `release-only=2`, `tests-skipped=4`, `goldens=29` — identical to the initial verification |
| The derivation is from the inventory, not the reports found | `grep -n invskip tools/check/steps/golden.sh` | `$tmp/invskip` built at `:120` from `$ROOT/$inv` column 5; `relonly` counted at `:437-443`, and **each** such row must have `build/gate/release/runner/<id>.report` or it is `golden-dev-release-diff` |
| No step output changed for a green run | normalised diff above | only the lines tabulated; every other step byte-identical |
| The `_good` positive control still passes with the new well-formed row | standalone step on `_good/tree` | `-- important-gates pass gates=43 lines=44`, exit 0 |
| No control, fault, golden or pinned test was dropped | gate token | `controls` 384 → **386**, `faults=235/235`, `fault-runs=235`, `goldens=29`, `tests-*` unchanged; `.github/` untouched by the fixes (`git diff --name-only 86c7878..HEAD -- .github/` → 0) |

### The no-regression set, re-confirmed

| Item | Check | Result |
|---|---|---|
| No new claim | gate token | `claims=28 provisional=27` — unchanged; `data/expected/claims.ids` not in the fix range |
| No new tier | gate token | `tiers=8` — unchanged |
| 38 steps | `ls tools/check/steps/*.sh \| wc -l`; gate token | **38**; `steps=38/38` in all three shapes |
| `data/expected/gate-steps/` unchanged | `git diff --name-only 912ab9a..HEAD -- data/expected/gate-steps/` | **0 files** |
| Registry unchanged | `git diff --name-only 285807e..HEAD -- data/expected/important-gates.ids` | **0 files**; `gates=107 lines=330` |
| No binary fixtures tracked | `git ls-files \| grep -E '\.(bin\|rom\|ivsc)$' \| wc -l` | **0** |
| `binary-ext.txt` / `.gitignore` unchanged | `git diff --stat 86c7878..HEAD -- …` | empty |
| No `data/` change outside `data/expected/` | `git diff --name-only 86c7878..HEAD -- data/ \| grep -v '^data/expected/'` | empty |
| Single noreply author | `git log 86c7878..HEAD --format='%ae' \| sort -u` | `szTheory@users.noreply.github.com` only |
| No home paths | `grep -E '/Users/\|/home/'` over every file the fixes changed | none (the only match in the phase range is this report's own prose quoting the check, as before) |
| No debt markers | `grep -E 'TBD\|FIXME\|XXX'` and `TODO\|HACK\|PLACEHOLDER` over the five changed source files | none (the `XXXXXX` in `mktemp` templates is the only `X`-run match, and it is not a marker) |
| Only the intended files changed | `git diff --name-only 86c7878..HEAD` | 23 files: 21 fix files + `09-REVIEW.md` + this report. No source outside the five shell files, the two `data/expected/controls/*.ids`, and the control directories |

### The one defect the fixes introduced — recorded, not passed over

WR-01's fix adds a **tenth** must-fail mechanism with no `data/expected/important-gates.ids` row,
which is exactly what deviation D-09-06 discloses. The registry is confirmed clean
(`grep -c -F <mechanism> data/expected/important-gates.ids` → **0** for all ten, including
`important-gates-excluded-malformed`), and `lines=330` still holds, so the deviation's substance is
unchanged and still correct. What is stale is its **count**: `.planning/STATE.md:558` still says
"the **nine** mechanisms Phase 9 landed" and lists nine, and `09-05-SUMMARY.md:214` — the file
`STATE.md` names as "the canonical list" — still says "nine mechanisms and no fewer" and enumerates
nine. `09-REVIEW.md`'s `## Fixes applied` does name the tenth and says it extends the list to ten,
so the fact is disclosed somewhere; but the standing-gap row a later milestone reads understates the
tree by one. That is a record/tree disagreement of the class `AGENTS.md` names ("recompute, never
quote — a quoted figure that disagrees with the file is a defect to correct"), it is a one-line fix
in each of the two records, and it is **open item 7** below. It is not one of the five frozen
criteria, it hides no weakened check, and it does not move the verdict — which is why it is an open
item rather than a gap, and why it is stated here in full rather than left for a reader to find.

### Verdict

**Unchanged: `passed`, 5/5 frozen success criteria.**

- All three gate shapes end `gate: PASS` on `768e97e` with the predicted token
  (`controls 384 → 386`, `cases 12 → 13`, everything else unchanged), zero top-level `-- FAIL ` lines,
  and no `ivs-` container or volume left behind.
- The three fixes each add a mechanism with a pinned token, a control or case that fails for its
  intended reason, and a committed re-runnable stand-in — and I proved each stand-in live by mutating
  the rule it guards, including the reviewer's own pre-fix shape.
- The normalised diff against the committed pre-change capture shows no output-order change and no
  changed step output; the pass-line rule prints nothing when it accepts, so a green run's summary
  token moves only by the two fields the fixes intend.
- The five criteria re-measure as before, and criterion 4 and 5 are strictly stronger: the pass-line
  rule now covers all 38 steps instead of 33, and the ledger — the one file that can silence the
  EV-04 check — now refuses the malformed rows its own header forbids, in both shapes.
- The no-regression set is intact: no new claim, no new tier, 38 steps, `gate-steps/` and the
  registry unchanged, no binary fixtures, no home paths, one noreply author.
- One bookkeeping defect was introduced (the D-09-06 count still reads nine where the tree holds
  ten) and is recorded as open item 7 rather than absorbed into a pass.

Nothing in this re-verification is a human-only item, and no claim cites one. The status remains
`passed` for the same reason it was `passed` initially: every criterion is judged by a tool, and no
item is left that a tool cannot judge.

---

_Re-verified: 2026-11-26_
_Verifier: gsd-verifier_
