# Milestone ritual (standing policy)

**Status:** owner instruction, 2026-11-24. Applies to every future milestone in
this project, not only the one that prompted it. `AGENTS.md` points here and
`.planning/AUTONOMY.md` section 8 restates it. Do not re-ask the owner for any of
it; do not re-derive it.

Every new milestone runs these six steps **in this order**, before any phase
planning starts.

## 1. Refresh the rolling roadmap

`.planning/ROADMAP.md` carries three horizons, always current, always
prioritized and triaged against the project's intent — not against a feature
backlog:

| Horizon | Contents |
|---|---|
| **Near** | The milestone being scoped now, phase by phase |
| **Mid** | The next two or three milestones, one line each with its gate |
| **Long** | The standing intent and the explicit non-goals |

Rules for the refresh:

- Every near/mid item names **what gates it** (rights, owner hard stop D10/D11,
  a consumer, a cost line, a missing source). An item with no named gate is a
  wish, not a roadmap row.
- Re-triage, do not append. If an item no longer earns its position, say why in
  the row that removes it.
- The rolling roadmap is a **recommendation register**, labelled `[REC]`. It is
  not a promise and not a requirement list. Requirements are minted per
  milestone in `.planning/REQUIREMENTS.md`.
- The archived milestone roadmaps stay untouched under
  `.planning/milestones/<version>-ROADMAP.md`. Archives are append-only.

## 2. Inspect state and evidence before choosing

Read, at minimum: `.planning/PROJECT.md`, `.planning/RETROSPECTIVE.md`
(especially "Carried Into the Next Milestone"), `.planning/CI-EFFICIENCY.md`
(last entries), `.planning/MILESTONES.md`, the last milestone's close record,
`.planning/REQUIREMENTS.md` v2 list, and `data/claims.json` (tier standing).

**Recompute every figure; never quote a prior entry.** The cost median, the
claim count, the requirement coverage and the gate counts are all derivable from
committed files. A quoted figure that disagrees with the file is a defect —
record the correction where the stale figure lives.

## 3. Fan out, then auto-select one milestone

Use the discuss protocol of `.planning/AUTONOMY.md` section 2, applied to the
milestone choice itself: one research subagent per decision-relevant lens
(product/integrator, technical/architecture, QA/evidence,
rights/security/devops, and any lens the specific choice needs), each covering
breadth and depth through all relevant role perspectives with an adversarial
pass, then one coherent recommendation that the orchestrator **auto-selects and
records**. Do not present a menu to the owner and do not stop for approval on a
delegated decision.

Stop when more research stops changing the recommendation, and say so.

## 4. Fan out the upstream scoping pass

This is the step that makes the downstream work directionally correct, and it is
separate from step 3. Once the milestone is chosen, fan out again — one
subagent per **scoping decision point**, not per feature:

1. Phase decomposition and sequencing (what the phases are, in what order, what
   each must exit with, and which phase is droppable).
2. Gate cost and evidence durability (how the milestone may add checks at all,
   and what it retires).
3. Rights, format and packaging policy (what may be written down, what may be
   shipped, what degrades and how).
4. Verification strategy (what is automated, what is throwaway, what goes in CI,
   what is handed off, and the must-fail control plus liveness guard for every
   new capability).
5. Downstream guardrails and autonomous execution (the retarget checklist, the
   definition of done, the discuss/verify boilerplate, the clean-state
   reconciliation).

The output is **not** a requirements list. It is the frozen upstream set: the
phase shape, the decision rules, the policies and the oracles that must exist
*before* a phase plan is written, so that no phase planner has to invent them
and no plan can drift from them.

## 5. Freeze the upstream artifacts, then run the GSD cycle

Write the frozen set (phase shape, decision rows, format/policy documents,
oracle documents, guardrail file, requirements, roadmap) and commit it. Then
`/gsd-new-milestone` finishes normally: PROJECT.md current milestone, STATE.md
switch, REQUIREMENTS.md with REQ-IDs, roadmap by `gsd-roadmapper`.

Only after that is the milestone ready to execute.

## 6. Hand the owner one command

When the milestone is fully scoped and committed, print the single command that
runs the whole milestone end to end after `/clear`:

```
/gsd-autonomous
```

(plus any scope argument the installed skill needs). The owner runs one command;
the orchestrator drives discuss → plan → execute → verify per phase, keeps
`main` green, triages PRs, records CI rows and runs the release flow.

## 7. Close in a clean state (mandatory)

No milestone ends until all of these are true and **stated in the close record**:

- `main` is green at the merge commit, with the push run recorded as an
  `event=push` row in `.planning/ci-runs.tsv`.
- `git status --porcelain` is empty on `main`; one worktree; no stashes; no
  stray unmerged phase branches.
- Open pull requests and issues are triaged to zero, or each one is recorded
  with its disposition.
- The rulesets are applied (with ids recorded) or recorded as "not applied" with
  a reason. Never silently skipped.
- Records reconciled: REQUIREMENTS traceability and close-out notes, STATE.md,
  ROADMAP archive, one CI-EFFICIENCY entry per run, RETROSPECTIVE lessons,
  ACCEPTED-DEBT for every deferral, every handoff row dispositioned.
- A stale-text sweep: any recorded fact contradicted by the tree (a config key,
  a count, a version) is corrected or recorded as accepted debt.
- The release decision is one line: `PUBLISHED vX.Y.Z (prerelease)` or
  `READY, NOT PUBLISHED: blocked by P<n>`, with the checks recomputed, not
  assumed.

A dirty tree or an unrecorded run is a close blocker, not a note.
