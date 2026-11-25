---
phase: 10-reachable-runs-and-presentation-policy
verified: 2026-11-25T14:38:02Z
verifier: Claude (gsd-verifier)
status: passed
score: 5/5
re_verification: true
phase_base: d3952ea
verified_tree: 153fd51
tier_ceiling:
  documented_route: installed-host-behavior
  handoff: none
covered_files:
  - .planning/REQUIREMENTS.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-01-PLAN.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-01-SUMMARY.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-02-PLAN.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-02-SUMMARY.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-03-PLAN.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-03-SUMMARY.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-04-PLAN.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-04-SUMMARY.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-05-PLAN.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-05-SUMMARY.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-06-PLAN.md
  - .planning/phases/10-reachable-runs-and-presentation-policy/10-06-SUMMARY.md
  - README.md
  - data/expected/controls/claims.ids
  - data/expected/handoffs.ids
  - data/expected/important-gates-excluded.ids
  - data/expected/important-gates.ids
  - docs/handoffs.md
  - docs/packaging.md
  - docs/presentation.md
  - docs/running.md
  - tools/check/kinds/claim.cmake
  - tools/check/steps/claims.sh
  - tools/check/steps/text.sh
  - tools/release-rehearse.sh
covered_digest: "v4:sha256:3d860a1102215c253e11387a82004b28ffa03e05f88fe0598d262f410b465560"
behavior_unverified: 0
overrides_applied: 0
re_verification:
  previous_status: passed
  previous_score: 5/5
  gaps_closed:
    - "B-01 (review): the handoff ledger can be missing, short a row, or unreadable and the gate still passes — closed by b693b12, re-probed below"
    - "B-02 (review): the handoff-artifact predicate is lexical, so ./build/... and Build/... are accepted claim evidence — closed by 7bc5a66, re-probed below"
    - "W-02 (review): the committed stand-in covered 2 of the 5 refused artifact classes — closed by 7bc5a66 (8 claims)"
    - "W-03 (review): claims-handoff-row had no FORBID — closed by aff2f92"
    - "W-04/W-03 numbering (review): the release procedure never invoked --source-route — closed by 99a9827"
  gaps_remaining:
    - "W-04 (review): docs/running.md's nine-line tail is still a hand-quoted figure nothing pins"
    - "W-05 (review): docs/running.md's excluded-key enumeration is still incomplete"
    - "I-01 (review): the look/listen word list is still the two frozen words"
  regressions:
    - "W-01's fix (29083f0) is itself broken: tools/release-rehearse.sh now exits 1 in every mode before doing any work (new BLOCKER, gap G-01)"
gaps:
  - truth: "tools/release-rehearse.sh builds from the source archive in a clean directory and runs demo-lamp-tone-pass to its golden, so the documented release route is proved end to end (criterion 1, PKG-05)"
    status: failed
    reason: >-
      The drift guard added by 29083f0 (the W-01 fix) calls drift() with two arguments while the
      function reads $2 and $3, so the copy name expands empty, the source extraction matches no
      function and the check fails unconditionally. Every mode of the rehearsal therefore exits 1
      immediately after the tool checks, before any dry run, archive extraction or golden
      comparison. The copy it guards is byte-identical to its source; only the guard is wrong.
    artifacts:
      - path: tools/release-rehearse.sh
        issue: >-
          drift() body reads "$2" (source name) and "$3" (copy name) but the two calls pass
          `drift filter_report route_filter` / `drift live_count route_live`; $3 is empty, so
          `sed -n "/^$2() {/,/^}/p" golden.sh` extracts nothing and `[ -n "$_src" ]` fails.
      - path: .planning/ops/RELEASE-OPS.md
        issue: >-
          99a9827 (the W-03 fix) makes `sh tools/release-rehearse.sh --release --source-route` the
          pre-tag invocation, so the release procedure now aborts on the same broken guard.
    missing:
      - "the two drift calls and the function's parameter use brought into agreement (function reads $1/$2, or the calls pass the three names the body reads)"
      - "a rehearsal run in at least one mode reaching `rehearse: ok` on this tree, and a re-run of --source-route reaching the golden match line"
      - "a control or a rehearsal line that makes a broken drift guard fail where it is seen, not only at release time (the gate does not run the rehearsal, by design, so nothing caught this)"
---

# Phase 10 Verification: Reachable Runs and Presentation Policy

**Goal (ROADMAP):** A person obtains and runs the emulator from the released source archive using documented commands, the presentation and palette policy is frozen and enforced, and the look/listen handoff is recorded rather than assumed.

**Verified tree:** `6dbf4ca` (phase branch after the code-review fixes; the working tree is that commit plus the orchestrator's `.planning/state.json`), phase base `d3952ea` (the research commit; amendment A-04's baseline).
**Method:** every criterion re-measured on this tree in this session. No SUMMARY value was accepted as evidence; each row below names the command and quotes its output.

## The gate, re-run three ways on the final tree

```
sh    tools/gate.sh        -> exit 0, 530 log lines, 41 step-time lines, 0 lines beginning '-- FAIL', gate-time total=192
dash  tools/gate.sh        -> exit 0, 530 log lines, 41 step-time lines, 0 lines beginning '-- FAIL', gate-time total=243
sh    tools/gate-linux.sh  -> gate-linux: exit 0 (x86_64, gcc 13.3.0, cmake 3.28.3), gate-time total=833,
                              531 log lines, 0 lines beginning '-- FAIL'; build/gate-linux.log copied to
                              /tmp/ivs10verify/gate-linux.log before anything else ran; no ivs- container left
                              (docker ps -a shows none)
```

The `sh` and `dash` summary lines are **byte-identical** (1462 characters each; `[ "$s" = "$d" ]` true). The tails, on all three shapes:

```
gate: PASS steps=38/38 controls=393/393 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 ...
            fault-runs=235 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=79 ... claims=28 provisional=27
            ... gates=109 lines=333 ... prefix-files=11 ... files=2622 tracked=2622 hygiene-files=2622 ...
            tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0
            docs=63 claim-refs=26 d10-items=47
gate-preflight: PASS steps=29/29      gate-partition preflight=29 producers=9
```

Every frozen token of criterion 5 is where the phase froze it: `steps=38/38`, `claims=28`, `provisional=27`, `claim-refs=26`, `gates=109`, `lines=333`. `controls` moved `389 -> 393` (+4, exactly the four new controls B-01's fix adds) and nothing else moved. `core-imports` is 7 on Darwin and 5 on Linux, and `archive=members` on Darwin versus `archive=sha256` on Linux — pre-existing platform differences, not frozen tokens.

## Success Criteria

| # | Criterion (frozen, as amended) | Verdict | Evidence (command and output) |
|---|---|---|---|
| 1 | `docs/running.md` names the three commands and the exact expected stdout tail, and `tools/release-rehearse.sh` builds from the **source archive** in a clean directory and runs `demo-lamp-tone-pass` to its golden — recorded at release time, not as a gate step | **FAILED (BLOCKER G-01)** — the document half still holds; the rehearsal cannot run at all on this tree | (a) Unchanged and re-measured: the three commands and the nine-line tail are in `docs/running.md`; the archive route was executed end to end in the first pass of this record (archive → clean directory with no `.git` → `cmake --preset release` → `ivs-run` → 131-line report → 124 of 124 golden lines, document tail diff empty). (b) **Broken by the review fixes:** `sh tools/release-rehearse.sh --no-gate` and `sh tools/release-rehearse.sh --no-gate --source-route` both print exactly `rehearse: FAIL:  has drifted from tools/check/steps/golden.sh (route_filter)` and exit **1**; `dash` gives the same. Nothing else runs — no dry run, no archive, no cold build, no golden comparison. See "Re-verification after the review fixes" for the diagnosis and the one-line cause. (c) The copy the guard is about **is** byte-identical: `filter_report`→`route_filter` 777 bytes equal, `live_count`→`route_live` 127 bytes equal (both extracted with the same `sed` the script uses), so the defect is the guard's parameter mapping and not the copy. (d) Still not a gate step: `grep -n 'rehearse' tools/gate.sh` → nothing; `grep -rn 'release-rehearse' tools/check/` → nothing; 38 step scripts. This is exactly why three green gate runs coexist with a rehearsal that cannot start. |
| 2 | `docs/presentation.md` holds the six-clause policy, and one deny-list check refuses a colour symbol defined under `core/src`, shown able to fail by a committed scratch stand-in; the check adds no gate step | **met on recorded terms (A-02)** — unchanged by the fixes | Re-measured: `grep -c -E '^[1-6]\. ' docs/presentation.md` → **6**, at lines 12, 18, 31, 38, 43, 50. The refusal is still the pre-existing `tools/check/core-symbols.sh:171` (`if (tolower(n) ~ /(palette|rgb|ppm|framepreview)/)`) with its committed stand-in `core-symbols-palette-defined`; the gate log reads `control ok   core-symbols-palette-defined` (line 224 of the `sh` log). Ancestry unchanged (`51515f1` is an ancestor of `b5ad01d`), so A-02's inversion still stands; `git diff --stat d3952ea..HEAD -- data/expected/gate-steps/` empty. |
| 3 | `data/expected/handoffs.ids` holds H7-1, H7-2, H8-3, D10, D11, every row `evidence: none` and each dispositioned `handed-off`/`not-performed`; a claim citing a handoff artifact, a `build/` path, a `.wav`, a `.ppm` or a bench `.tsv` is refused, and the `text` step's claim-word list contains the look/listen words — both shown able to fail | **met, and stronger than at the first pass** (B-01 and B-02 fixed; four new controls) | (a) `awk '!/^#/ && NF {print NF" | "$0}' data/expected/handoffs.ids` → five rows, 4 tokens each: `H7-1 handed-off evidence: none`, `H7-2 handed-off evidence: none`, `H8-3 handed-off evidence: none`, `D10 not-performed evidence: none`, `D11 not-performed evidence: none`. (b) The ledger is now **required** and its row set pinned in both directions: the two blocker probes below refuse on the real tree, and the four new controls (`claims-handoff-missing`, `-unreadable`, `-id-missing`, `-id-extra`) each fail for exactly their own reason. (c) The artifact predicate now tests the normalised (real, ROOT-relative, lower-cased) path: `./build/...`, `Build/...` and `out/x.WAV` are all refused on the real tree, and the committed stand-in carries eight claims covering all five classes plus the three spellings. (d) The text guard is unchanged and still sound: `looks-like` and `sounds-like` are caught, `overlooks`/`Sounding`/`Overlooked` are not. |
| 4 | `README.md` and `docs/packaging.md` carry one identical "nothing downloadable is runnable" sentence (a grep proves a single matching line in each) and one paragraph pointing at `docs/running.md` | **met, as amended by A-01** — unchanged by the fixes | Re-measured: `grep -c -F 'nothing downloadable is runnable' README.md docs/packaging.md` → `README.md:1`, `docs/packaging.md:1`. `docs/packaging.md:121` is still byte-identical to the phase base; the pointer paragraphs are still `README.md:18` and `docs/packaging.md:125`. No fix commit touched either file (`git diff --stat 5731f4a..HEAD -- README.md docs/packaging.md` empty). |
| 5 | The gate summary keeps `steps=38/38` and `claims=` at its Phase 9 value, and no claim cites a handoff row: the documented route reaches at most `installed-host-behavior` and the handoff carries no tier | **met** | Re-measured on the final tree, all three gate shapes: `steps=38/38 claims=28 provisional=27 claim-refs=26`. 38 step scripts (`git ls-tree -r --name-only HEAD tools/check/steps/ \| grep -c '\.sh$'`), `data/expected/gate-steps/` unchanged since the base, `claims.json`/`claims.ids` still byte-identical to the base (`git diff --stat d3952ea..HEAD` names neither). `gates=109 lines=333` as A-04 permits (and as A-04's `lines` was corrected by measurement). `controls` 393 is not frozen by this criterion. |

**Score: 4/5 criteria met** — criterion 1 failed on the rehearsal regression introduced by `29083f0`; criterion 2 on the recorded terms of amendment A-02, criterion 4 on the recorded terms of amendment A-01. No criterion required human judgement.

## Requirements

| Requirement | Source plan | Description (REQUIREMENTS.md) | Status | Evidence |
|---|---|---|---|---|
| PKG-05 | 10-01, 10-04, 10-06 | A person can build and run the emulator from the released source archive using documented commands only, and the release rehearsal proves that route end to end against the demo cartridge golden from a moved prefix | ⚠️ **AT RISK (G-01)** — the route works, its release-time proof no longer starts | `.planning/REQUIREMENTS.md:102` reads `Complete` and the exclusion row `important-gates-excluded.ids:27` still holds, but the mechanism that discharges the requirement at release time (`tools/release-rehearse.sh --source-route`) exits 1 before it unpacks anything. The requirement's own evidence cannot be produced until G-01 is fixed. |
| TOOL-04 *(narrowed)* | 10-02, 10-06 | A frozen presentation and palette policy keeps colour, scaling and aspect out of the core and out of every claim, and a check refuses a colour symbol in the core | ✓ SATISFIED (closed on an **exclusion** row, per A-02) | Criterion 2's measurements; `.planning/REQUIREMENTS.md:103` `Complete`; `important-gates-excluded.ids:38`; `core-symbols-palette-defined` green in the gate. |
| EV-05 | 10-03, 10-05, 10-06 | A handoff ledger records every human-only item with `evidence: none`, a claim may not cite a handoff artifact, and a later milestone cites the row instead of re-asking | ✓ SATISFIED (three **registry** rows, now enforced by seven controls) | Criterion 3's measurements and the two blocker probes; `.planning/REQUIREMENTS.md:101` `Complete`; `data/expected/important-gates.ids:469-471`; `data/expected/controls/claims.ids` now carries `claims-handoff-missing`, `-unreadable`, `-id-missing`, `-id-extra` beside the original three. |

The derived must-be-covered identity still closes in both directions: **Complete 57 − Excluded 12 = Accepted 45 = Registered 45** (`gates=109 lines=333` unmoved, so no registry row was added or lost). No orphaned requirement: no id in `REQUIREMENTS.md` maps to Phase 10 that no plan claims.

## What was automated / handed off / could not be judged by a tool

(GUARDRAILS §5.6.)

**Automated, and re-run by me in this session.** The three gate shapes (sh, dash, container) on the final tree; both guards made to fail on scratch copies of the real tree and both shown green on the unmodified tree; the four new controls plus the extended artifact control, the row control and the text control run in isolation; the word-boundary probe for `looks`/`sounds`; the phrase counts and the base-body comparison for criterion 4; the registry identity; the base-versus-final token comparison. The archive route and the rehearsal's `--source-route` mode were executed end to end in the first pass of this record; the rehearsal was re-run on the final tree and **now fails** (G-01), and the route code itself is unchanged since that green run (`git diff 5731f4a..HEAD -- tools/release-rehearse.sh` adds only the drift guard, 17 lines, no other change).

**Handed off, and why.** Five rows in `data/expected/handoffs.ids`, every one `evidence: none`, none citable by a claim:

| Row | Disposition | What the owner owes |
|---|---|---|
| `H7-1` | handed-off | listen to `build/listen/demo-lamp-tone-pass.wav` (21674 bytes) and say whether the tone is the one the program intends |
| `H7-2` | handed-off | look at a `--frame-ppm` export (45806 bytes, `P6` / `159 96` / `255`) and say whether it is the intended picture |
| `H8-3` | handed-off | the same two acts plus a look at a `diag-stic2-` picture; one sitting discharges H8-3 and both Phase 7 rows |
| `D10` | not-performed | physical-hardware measurement (47 `D10-*` items in `data/expected/d10-measurements.ids`) |
| `D11` | not-performed | owner-local real EXEC/GROM images (`sh tools/owner-run.sh refs/FX-owner-exec.bin refs/FX-owner-grom.bin` prints `SKIPPED owner-run: no operator images (D11)` and exits 77 without them; the bare invocation exits 2 with a usage line — corrected here, see review I-02) |

These are the phase's **deliverable**, not its open criteria: the frozen goal is "the look/listen handoff is recorded rather than assumed", and it is recorded — now with the ledger's existence and row set enforced. Nothing here raises the phase to `human_needed`, because no criterion was left to a human.

**Could not be judged by a tool.** Only the content of the five handoff rows above (whether the tone is right, whether the picture is intended, what real hardware does, what an owner-local ROM does) — deliberately, structurally, and by prior decision.

## Deviations accepted

1. **A-01 — the phrase is made identical, not kept identical.** `d3952ea:README.md` contains the phrase **0** times; the frozen sentence "kept identical" was unsatisfiable. Accepted because the amendment is appended, measured and specific: the checked object is the 34-byte substring, the proof is `grep -c -F` = 1 in each file, and `docs/packaging.md:121` is left byte-identical.
2. **A-02 — the oracle order is inverted.** The colour mechanism (`51515f1`, Phase 6) is an ancestor of the policy (`b5ad01d`), so the policy does not create the check. Accepted because it is recorded in the document itself and in `10-02-SUMMARY.md`, proved by `git merge-base --is-ancestor`, and closed on an exclusion row rather than a registry row that would mis-attribute a Phase 6 mechanism to a Phase 10 requirement.
3. **A-04's `lines` figure is wrong and was corrected by measurement.** A-04 predicted `gates=109, lines=335`; the measured values are **109** and **333**. The mechanism is recorded: `lines` counts registry rows only, and the two new exclusion rows live in a different file. `gates`/`lines` were never frozen for this phase.
4. **`controls` moved 386 → 389 → 393.** The first three controls were plan 10-05's (`claims-handoff-artifact`, `claims-handoff-row`, `text-untagged-look-listen`; one more than plan 10-05 predicted, because the ledger's own grammar needed a stand-in). The review then added four more (`claims-handoff-missing`, `-unreadable`, `-id-missing`, `-id-extra`) to prove B-01's fix, taking `controls` to 393 — the +4 predicted before the commit that moves it. `controls` is not frozen by criterion 5.
5. **Two pre-existing documents gained `[nc]` waivers** (`docs/provenance.md`, `docs/video/demo-lamp-tone-1-derivation.md`) where the newly-added words `looks`/`sounds` appear in descriptive prose. Accepted: `[nc]` is the documented waiver (R5), and no claim tag was silently removed.
6. **The rehearsal transcript records commit `0cce18b`, not the final tree.** Accepted: a transcript is a point-in-time record; the transcript is explicitly not gate evidence.
7. **`10-06-SUMMARY.md` recorded that its post-SUMMARY gate re-run was not taken** and stated the prediction instead. Closed here: the `sh`, `dash` and container gates were all re-run on the final tree and all three PASS with every frozen token as predicted.

## Re-verification after the review fixes

The phase was first verified `passed` at `5731f4a`. `10-REVIEW.md` then raised 2 blockers and 6 warnings, and five commits fixed four of the warnings and both blockers. This section re-measures the final tree (`6dbf4ca`) rather than the tree the first pass saw.

**Commits in scope:** `b693b12` (B-01: the ledger is required and its row set pinned, four new controls), `7bc5a66` (B-02: the artifact predicate normalises; the stand-in grows to eight claims), `29083f0` (W-01: the rehearsal asserts its copied functions), `aff2f92` (W-02: `FORBID` on `claims-handoff-row`), `99a9827` (W-03: `RELEASE-OPS.md` invokes `--source-route` before the tag).

### The gates, on this tree

```
sh    tools/gate.sh        exit 0  |  530 log lines  |  41 step-time lines  |  0 '-- FAIL'  |  gate-time total=192
dash  tools/gate.sh        exit 0  |  530 log lines  |  41 step-time lines  |  0 '-- FAIL'  |  gate-time total=243
sh    tools/gate-linux.sh  exit 0  |  531 log lines  |  41 step-time lines  |  0 '-- FAIL'  |  gate-time total=833
gate: PASS steps=38/38 controls=393/393 ... claims=28 provisional=27 ... gates=109 lines=333 ... claim-refs=26
```

`sh` and `dash` summaries byte-identical (1462 chars). Container: `x86_64, gcc 13.3.0, cmake 3.28.3`; log copied to `/tmp/ivs10verify/gate-linux.log` before anything else ran; `docker ps -a` shows no `ivs-` container. The token movement is exactly the predicted one: `controls 389 -> 393`, every other frozen token flat.

### Blocker probe (a) — the ledger is deleted

A `git archive HEAD` extract of the real tree, `data/expected/handoffs.ids` removed, the `claims` step run with `ROOT`/`CHECK_DIR` set:

```
-- claim pass records=28 claims=28 provisional=27
-- FAIL claim-handoff-row: no ledger (data/expected/handoffs.ids)      EXIT=1
```

The step refuses. Before the fix this exact probe printed `-- claims pass claims=28 provisional=27` and exited 0 (review probe 9).

### Blocker probe (b) — the artifact is spelled `./build/...`

The same extract, a `CL-0029` claim whose `artifact` is `./build/gate/dev/runner/x.report` (file created so `claim-dangling-path` cannot fire; `data/expected/claims.ids` extended so `claim-id-extra` cannot):

```
-- FAIL claim-handoff-artifact: CL-0029                                EXIT=1
```

And the two spellings the review named, one run, `CL-0029` = `Build/gate/dev/runner/x.report`, `CL-0030` = `out/x.WAV` (both present on this case-insensitive host):

```
-- FAIL claim-handoff-artifact: CL-0029
-- FAIL claim-handoff-artifact: CL-0030                                EXIT=1
```

Before the fix all three spellings were accepted with `-- claims pass`, exit 0 (review probes 7 and 8).

### The fixes weakened nothing

| Check | Command | Result |
|---|---|---|
| the positive control still passes with the ledger required | `_good` tree, `claims` step | `-- claims pass claims=1 provisional=0`, exit 0; the gate log carries `control ok   _good/claims`; `_good/tree/data/expected/handoffs.ids` holds the five well-formed rows |
| the four new controls fail for their intended reason **only** | each control tree built and run the way `run-controls.sh` builds it | `claims-handoff-missing` → `-- FAIL claim-handoff-row: no ledger (data/expected/handoffs.ids)`; `-unreadable` → `… : unreadable (…)`; `-id-missing` → `… : missing id D11`; `-id-extra` → `… : unknown id X-1`. One FAIL line each, exit 1 each, and each control's `FORBID` names the other five details plus `claims pass`, so none can be satisfied by a sibling reason |
| the extended artifact stand-in still isolates its token | `claims-handoff-artifact` | eight `-- FAIL claim-handoff-artifact: CL-0001…CL-0008` lines and nothing else; `FORBID` holds `claim-dangling-path`, `claim-handoff-row`, `claim-order`, `claim-id-extra` |
| the row control's new `FORBID` holds | `claims-handoff-row` | `-- FAIL claim-handoff-row: line 3` only; `FORBID` names `no ledger`, `unreadable`, `no rows`, `line 1`, `line 2`, `missing id`, `unknown id`, `claim-dangling-path`, `claim-order`, `claim-id-extra` |
| the look/listen words still catch and still do not over-catch | the step's own regex over a probe file | caught: `1:A looks-like console.`, `2:It sounds-like the arcade.`; not caught: `The emulator overlooks nothing.`, `Sounding board prose.`, `Overlooked prose.` — no false positive, `-w` boundary intact |
| the text control is unchanged | `text-untagged-look-listen` | `-- FAIL text-claim-untagged: docs/x.md:1`, `:2`, exit 1 |
| `steps` and `claims` unmoved, no new gate step | three gate summaries; `git ls-tree -r --name-only HEAD tools/check/steps/ \| grep -c '\.sh$'` | `steps=38/38`, `claims=28`, 38 step scripts, `data/expected/gate-steps/` unchanged |
| nothing was relaxed in the guards | `git diff 5731f4a..HEAD -- tools/check/steps/claims.sh tools/check/kinds/claim.cmake tools/check/run-controls.sh` | every removed line is replaced by a stronger one: the lexical predicate → the normalised one, the `-f`-guarded reader → the required-ledger reader, and `run-controls.sh` only *gains* the `symlink` MAKE verb (with the same absolute/`..` refusal) |
| the ledger and the claim ledger are untouched | `git diff --stat d3952ea..HEAD -- data/expected/handoffs.ids data/claims.json data/expected/claims.ids` | `handoffs.ids` +34 lines only (the phase's own file); `claims.json` and `claims.ids` absent from the diff |

### The regression the fixes introduced — G-01

`29083f0` (the W-01 fix) adds a drift guard to `tools/release-rehearse.sh` whose function reads two names the call sites do not pass:

```sh
drift() {
  _src=$(sed -n "/^$2() {/,/^}/p" tools/check/steps/golden.sh | sed "s/^$2() {/$3() {/")
  _cpy=$(sed -n "/^$3() {/,/^}/p" "$self")
  [ -n "$_src" ] && [ "$_src" = "$_cpy" ] || fail "$3 has drifted from tools/check/steps/golden.sh ($2)"
}
drift filter_report route_filter      # $1=filter_report $2=route_filter $3=(empty)
drift live_count route_live           # $1=live_count   $2=route_live   $3=(empty)
```

With `$3` empty, `_src` extracts `route_filter()` from `golden.sh` — a function that does not exist there — so `[ -n "$_src" ]` is false and the guard fails unconditionally. Measured on the final tree, under both shells, in every mode:

```
$ sh tools/release-rehearse.sh --no-gate --source-route
rehearse: FAIL:  has drifted from tools/check/steps/golden.sh (route_filter)      EXIT=1
$ sh tools/release-rehearse.sh --no-gate
rehearse: FAIL:  has drifted from tools/check/steps/golden.sh (route_filter)      EXIT=1
$ dash tools/release-rehearse.sh --no-gate --source-route
rehearse: FAIL:  has drifted from tools/check/steps/golden.sh (route_filter)      EXIT=1
```

It aborts immediately after the tool checks, so no dry run, archive, cold build or golden comparison happens. The copy is **not** at fault: with the same `sed` expressions the script uses, `filter_report`→`route_filter` compares equal (777 bytes) and `live_count`→`route_live` compares equal (127 bytes). The fix is a parameter-mapping change in one function (or in its two call sites); the guard's *intent* — a byte comparison against the code that actually runs, plus a `-n` liveness check on the source — is exactly right and is worth keeping.

Two consequences beyond criterion 1: `99a9827` makes `sh tools/release-rehearse.sh --release --source-route` the mandatory pre-tag invocation, so the release procedure now halts on this guard; and the gate cannot see any of it, because the rehearsal is deliberately not a gate step (`grep -rn 'release-rehearse' tools/check/` → nothing), so three green gate runs coexist with a rehearsal that cannot start. That is G-01's third `missing` item: the guard needs a place where a break is visible, not only at release time.

**Verdict change:** `passed` (5/5) → `gaps_found` (4/5). The two review blockers are genuinely closed and the enforcement is stronger than before; the phase's release-time proof is now unrunnable, which is a blocker on criterion 1 and on PKG-05.

## Open items

- **G-01 — blocker, new:** `tools/release-rehearse.sh` exits 1 in every mode because of the `drift()` parameter mapping added by `29083f0`. Structured in the frontmatter `gaps:`; criterion 1 and PKG-05 stay open until a rehearsal reaches `rehearse: ok` on this tree.
- **W-04 — warning, deliberately unfixed:** `docs/running.md`'s nine-line tail is a hand-quoted figure that nothing pins (three of the nine lines — `audio.chain.check`, `host.calls`, `report.end` — are in no data file). Prose-only; not a gap against the frozen criteria.
- **W-05 — warning, deliberately unfixed:** `docs/running.md:89-94` presents the golden's excluded keys as an exhaustive list and omits `audio.chain.check`, `exec:` and `grom:`.
- **I-01 — info:** the look/listen word list remains the two frozen words; `The sound is right.` and `The picture looked right.` are not caught. A recorded scope decision, not an error.
- **I-02 — corrected in this record:** the D11 row above now names the two-argument invocation that exits 77; the bare `sh tools/owner-run.sh` exits 2 with a usage line.
- ℹ️ **Info — a cosmetic doc/implementation mismatch.** `data/expected/handoffs.ids`'s header says the reader "must guard on existence with `if(EXISTS ...)`" (CMake idiom); the reader is `tools/check/steps/claims.sh`, a shell `[ -e ]`/`[ -f ]` pair. The header's stated semantics ("a ledger that records nothing is a different fact from a ledger that is not there") is now exactly what the step implements; only the idiom named is stale.
- ℹ️ **Info — `H7-3` is mentioned in this phase's records** and is **not** a row in `handoffs.ids`. That is correct: criterion 3 freezes the ledger at exactly H7-1, H7-2, H8-3, D10 and D11 (now enforced in both directions by the step), and H7-3 is the Phase 7 record's summary of the D10 items. A later milestone must cite `D10`.
- ℹ️ **Info — the residual the B-02 fix names and does not close:** a symlink whose target lies outside the repository normalises to a `../`-relative path that no clause matches; it is refused by `claim-dangling-path` only when broken. Recorded in `tools/check/kinds/claim.cmake`'s token prose, not papered over.
- **Deferred, correctly:** installing `bin/ivs-run` or any image (D14, structurally blocked), a windowed frontend, a palette table, a second cartridge, and any new gate step — all confirmed absent by measurement (`prefix-files=11`; `ls bin` → no such directory; 38 step scripts; `gate-steps/` unchanged).

## Rights, hygiene and integrity sweep

| Check | Command | Result |
|---|---|---|
| no tracked binaries | `git ls-files \| grep -E '\.(bin\|rom\|ivsc\|wav\|ppm)$'` | none |
| `binary-ext.txt`, `.gitignore` unchanged | `git diff --stat d3952ea..HEAD -- tools/check/binary-ext.txt .gitignore` | empty |
| single author | `git log --format='%an <%ae>' d3952ea..HEAD \| sort -u` | one author |
| no tilde-home literal in the phase's files | grep over every file in `git diff --name-only d3952ea..HEAD` | no hits |
| no secrets | `git grep -i -E 'BEGIN (RSA\|OPENSSH\|PGP) PRIVATE KEY\|ghp_[A-Za-z0-9]{20,}\|sk-[A-Za-z0-9]{20,}\|AKIA[0-9A-Z]{16}'` | none |
| nothing weakened to go faster | `git diff --stat d3952ea..HEAD -- tools/gate.sh tools/gate-linux.sh .github/ CMakeLists.txt CMakePresets.json` | empty; no `continue-on-error`, no path filter, no dropped control/fault/test (`controls` 386→393, `faults` 235/235, `goldens` 29, `tests-*` 4958/4954/4/0 all unmoved) |
| no archive rewritten | `git show --numstat 4ff8a7c -- <the four corrected files>` | zero deleted lines in all four |
| the guards were strengthened, not relaxed | `git diff d3952ea..HEAD -- tools/check/steps/text.sh tools/check/steps/claims.sh tools/check/kinds/claim.cmake` | the only removed lines are two header comments, the claim-word regex line (replaced by the same regex plus `looks\|sounds`), the lexical predicate (replaced by the normalised one) and the `-f`-guarded reader (replaced by the required-ledger reader) |
| frozen criteria untouched | `git diff d3952ea..HEAD -- .planning/ROADMAP.md` | only the `**Plans**` checklist was filled; the five criteria, the tier ceiling and the not-in-this-phase line are byte-identical |
| no control tree carries a tracked symlink | `git ls-files -s \| awk '$1=="120000"'` | empty; the symlink case is assembled at run time by the new `symlink` MAKE verb |

## G-01 closed (orchestrator close-out, tree `153fd51`)

**G-01 was a real regression and it is fixed in `153fd51`.** `29083f0`'s `drift()`
read `$2`/`$3` while its call sites pass the source name first and the copy name
second, so the copy name was empty, the guard failed unconditionally, and
`tools/release-rehearse.sh` aborted before any dry run. The parameter mapping is
corrected and its header comment now says which argument is which.

**Probed in both directions on the final tree:**

```
sh tools/release-rehearse.sh --no-gate --source-route   -> rc 0
  rehearse: source-route: ivs-v0.1.0-source.tar.gz,
    golden match (report 131 lines, 124 of 124 compared), prefix ivs-v0.1.0
  rehearse: ok

route_filter tampered on a copy                          -> rc 1
  rehearse: FAIL: route_filter has drifted from
    tools/check/steps/golden.sh (filter_report)
```

The guard works, and it works for its intended reason: the copy is proved against
its source on every rehearsal, and a one-line divergence is caught.

**Gates re-run on `153fd51`, one at a time:**

| Shell | Verdict |
|---|---|
| `sh tools/gate.sh` | `gate: PASS steps=38/38 controls=393/393 gates=109 lines=333 claims=28 provisional=27 claim-refs=26`, zero `-- FAIL` lines |
| `dash tools/gate.sh` | `gate: PASS`, exit 0 |
| `sh tools/gate-linux.sh` | `gate-linux: exit 0`, `gate: PASS`; log copied before anything else ran; no `ivs-` container left behind |

**Criterion 1 is met again**, from the archive to the golden, and the other four
are unchanged from the deep pass above. The token movement across the whole phase
is the predicted one: `controls 386 -> 393`, `gates 107 -> 109`,
`lines 330 -> 333`, `doc-files 61 -> 62`, `docs 62 -> 63`; `steps=38/38`,
`claims=28`, `provisional=27`, `claim-refs=26`, `faults=235/235`, `goldens=29`
and `tests-*` are all unmoved.

**Unfixed review warnings remain open items, not gaps:** W-04 (three values in
`docs/running.md`'s nine-line tail are not pinned by a data file), W-05 (the
"keys left out" enumeration is incomplete), I-01 (a two-word list), and the
residual recorded in the B-02 token prose (a symlink whose target lies outside the
repository normalises to a path no clause matches). Each is recorded in
`10-REVIEW.md` and carried to the phase's deferred items.

## VERIFICATION: passed
