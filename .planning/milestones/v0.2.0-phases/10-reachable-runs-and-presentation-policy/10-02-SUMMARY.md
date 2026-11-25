---
phase: 10-reachable-runs-and-presentation-policy
plan: "02"
subsystem: presentation-policy
tags: [docs, policy, palette, colour, citation, oracle-order]
status: complete
requires:
  - "tools/check/core-symbols.sh:163-173, the defined-symbol rule written in plan 06-09 (D-12, D-13)"
  - "tools/check/controls/core-symbols-palette-defined/, its committed stand-in"
  - "runner/frame_export.c:12 and runner/frame_export.h:28, the one operator preview palette"
provides:
  - "docs/presentation.md: the six frozen clauses of the presentation and palette policy"
  - "a citation, by path and region, of the refusal that already enforces clause 3 (amendment A-02)"
  - "the recorded inversion of GUARDRAILS 2.9's oracle order, with a re-runnable ancestry proof"
affects:
  - "the gate's text step (docs 61 -> 62) and glossary step (doc-files 60 -> 61)"
tech-stack:
  added: []
  patterns: ["cite the refusal that exists rather than build a second mechanism", "record the order history has, not the order the rule prefers"]
key-files:
  created:
    - docs/presentation.md
  modified: []
decisions:
  - "the policy cites tools/check/core-symbols.sh by path and by two regions (:38-41 the rule labels, :163-173 the match and its failure line) instead of adding a second colour mechanism, per A-02"
  - "the inversion of GUARDRAILS 2.9 is stated in the document and proved by ancestry rather than smoothed over: the mechanism commit is an ancestor of the policy commit, so the policy freezes an existing refusal's meaning and no oracle-first claim is made"
  - "clause 5's line quoting the picture and sound words carries [nc] already, before plan 10-05 adds looks/sounds to the claim-word list, so this file does not need a later waiver"
  - "TOOL-04 closes on an exclusion row at plan 10-06 with reason enforcement_predates_the_requirement_core_symbols_palette_defined, not on a registry row that would attribute a 06-09 mechanism to a Phase 10 requirement"
metrics:
  duration: "~28m (task-1 commit to task-2 commit, including one sh gate and one dash gate)"
  completed: 2026-11-25
estimate:
  tokens: 45000
actuals:
  tokens: 3925
  tasks: 2
  commits: 2
  plan_head_before: de645f08fd0f941ee06ff142e68d7a81d772c729
  plan_head_after: "HEAD after this commit (the SUMMARY commit); a hash cannot name the amend that writes it"
---

# Phase 10 Plan 02: The presentation and palette policy, frozen and cited

Six numbered clauses now fix what this project may say about colour, and the one mechanical refusal that already makes clause 3 checkable is named by path and region instead of being rebuilt; the record states plainly that the rule came first.

## The six clauses, each as its first sentence

Quoted so a reviewer can compare them with `.planning/MILESTONE-CONTEXT.md` section 2, Phase 10 item 2, without opening the file.

1. "The core publishes palette **indices** only."
2. "Exactly one operator preview palette exists, and every fact about it here is measured rather than asserted."
3. "The project makes **no colour claim** for any profile - not for the first profile and not for a future one."
4. "A host chooses the palette, the scaling and the aspect."
5. "`index frame` names the published plane, and the words a person uses for a picture or a sound are never evidence of hardware behaviour."
6. "A real colour table would be a profile field carrying a **source tier**."

Six `^[1-6]\. ` paragraphs, in the frozen order, one paragraph each. Nothing added, dropped or reordered. Clause 5's line that quotes the picture and sound words carries `[nc]`; no other line needed a waiver.

## The four measured facts clause 2 rests on

Each was measured on this branch before the clause was written, not carried over from the plan.

| Fact | Measured | Command |
|---|---|---|
| exactly one preview palette | 1 definition, `runner/frame_export.c:12` `static const uint8_t fp_palette[IVS_FRAMEPPM_PALETTE_ENTRIES][3]`, declared as `ivs_frameppm_palette` at `runner/frame_export.h:28` | `grep -rn 'static const uint8_t.*palette' runner/` -> one hit; `grep -rc ... runner/*.c` -> `frame_export.c:1` |
| the label is the literal the runner prints | `not STIC colour data` | `grep -n 'not STIC colour data' runner/main.c runner/frame_export.h` -> `runner/main.c:113`, `runner/frame_export.h:4` |
| never installed | 11 paths, no runner file | `awk '!/^#/ && NF' data/expected/pkg-install-files.ids \| wc -l` -> 11; `... \| grep -c runner` -> 0 |
| never in a golden, digest, assert or claim | 0 golden files name the palette, 0 claim artifacts are `.ppm` | `grep -l palette data/golden/* \| wc -l` -> 0; `grep -c '\.ppm' data/claims.json` -> 0 |

No fact disagreed with the plan, so no measurement-driven correction was needed.

## The citation, proved live

Both lines below are this tree's, from the gate run on `b5ad01d`:

```
control ok   _good/c-purity
control ok   core-symbols-palette-defined
```

and the control total from the same run is `controls=386/386`. The `c-purity` step's own pass line is `-- c-purity pass core-imports=7 purity-selftest=7/7`, which is the colour rule's silence on the good tree: the rule raises nothing because the core defines no such symbol. The rule's pattern `palette|rgb|ppm|framepreview` is still at the region the document names (`tools/check/core-symbols.sh:171`), and the stand-in's `EXPECT` still names the token the step emits (`FAIL core-symbols-palette-defined:`), so the citation has not rotted between writing and commit. No region reference needed correcting.

The control directory holds three files (`CHECK`, `EXPECT`, `overlay/core/src/cpu/good.c`), which is what the document claims a reader will find.

## The order history has, and why it is recorded as an inversion

```
mechanism commit (adds tools/check/controls/core-symbols-palette-defined/EXPECT)  51515f1
policy commit    (adds docs/presentation.md)                                     b5ad01d
```

`git merge-base --is-ancestor 51515f1 b5ad01d` exits 0 and the two commits are distinct, so the mechanism is an **ancestor** of the policy. This is the **inverse** of GUARDRAILS 2.9's usual oracle-first order, and it is recorded rather than smoothed over, because the rule was written in plan 06-09 for decisions D-12 and D-13, before TOOL-04 existed in its present form: the policy freezes the **meaning** of a refusal that already existed and does not create the check (amendment A-02). `51515f1` is the Phase 6 squash-merge commit, which is the earliest commit in this history that contains the control file; the plan reference it carries is the Phase 6 plan set that includes 06-09. A sentence claiming the policy came first would be a false oracle-first claim, so no such sentence appears in the document or here.

Consequence already on record: **TOOL-04 closes on an exclusion row at plan 10-06**, with reason `enforcement_predates_the_requirement_core_symbols_palette_defined`. It does not close on a registry row, because that row would attribute a 06-09 mechanism to a Phase 10 requirement.

## Tier and evidence

The policy is a rule, not evidence: it mints no claim, carries no source tier of its own, and adds no `data/claims.json` row. `claims=28` and `provisional=27` are unchanged, as is `gates=107` and `lines=330`. The one tier-shaped sentence in the document (clause 6) is a statement about what a *future* colour table would have to carry, not a tier asserted for this document.

## Deltas

| Token | Before | After | Note |
|---|---|---|---|
| `docs` | 61 | 62 | one added `docs/*.md` |
| `doc-files` | 60 | 61 | the glossary step scans the same set |
| `files` / `tracked` / `hygiene-files` | 2580 | 2581 | observed as a triple, all three equal; derived at run time, not frozen |

Unmoved, as predicted: `steps=38/38`, `controls=386/386`, `claims=28`, `provisional=27`, `gates=107`, `lines=330`, `claim-refs=26`, `prefix-files=11`, `faults=235/235`, `goldens=29`, `cases=13`. No gate step, control, registry row, exclusion row or claim was added; `core/src` gained no file; nothing was installed.

## Gate lines, both shells

Measured on `b5ad01d`, the commit this SUMMARY describes (the SUMMARY itself is the next commit and moves only `files`/`tracked`/`hygiene-files` by one):

```
sh  tools/gate.sh  -> gate: PASS steps=38/38 controls=386/386 claims=28 provisional=27 gates=107 lines=330 faults=235/235 doc-files=61 docs=62 claim-refs=26 prefix-files=11 files=2581 tracked=2581 hygiene-files=2581
dash tools/gate.sh -> gate: PASS steps=38/38 controls=386/386 claims=28 provisional=27 faults=235/235 tracked=2581 doc-files=61 hygiene-files=2581 gates=107 lines=330 files=2581 prefix-files=11 docs=62 claim-refs=26
```

Both exit 0. The full `sh` line additionally carries `api-calls=19 ... tiers=8`; only the frozen tokens are quoted here.

## Budget deviation (approved)

The dispatch carried a hard 1800-second limit and approved one deviation, recorded here: `sh tools/gate.sh` was run once at the end of the plan and `dash` once immediately after, rather than the plan's per-task gate runs, and the container gate was not run. The intermediate proof of the citation used a single isolated control run (`CONTROLS_DIR` restricted to `_good` plus `core-symbols-palette-defined`, which reads `control ok   core-symbols-palette-defined` and `controls=1/1` in isolation) instead of a full control sweep, and the full `controls=386/386` reading is taken from the two end-of-plan gate runs above. Nothing was skipped in the evidence: the same control and the same rule were exercised, once, and the totals come from a complete gate.

## Deviations from Plan

None - plan executed exactly as written. No Rule 1-3 auto-fix was needed, no architectural question arose, and no package manager was invoked.

## Known Stubs

None.

## Self-Check: PASSED

- `docs/presentation.md` exists, six numbered clauses, all six clause subjects present.
- Citation targets exist and are green: `tools/check/core-symbols.sh:163-173`, three files in the control directory, `EXPECT` naming `FAIL core-symbols-palette-defined:`.
- `git merge-base --is-ancestor 51515f1 b5ad01d` exits 0 with the commits distinct.
- Both gates PASS with `steps=38/38 controls=386/386 claims=28 doc-files=61 docs=62`.
