# Durable knowledge index (the project's second brain)

Purpose: an agent landing cold in this repository must be able to find every
durable decision, rule and lesson in one hop, and must never re-litigate
something already settled. **If you are about to ask the owner a question, check
this index first.**

## Standing policy (read before doing anything)

| File | Holds |
|---|---|
| `AGENTS.md` | Project instructions: owner direction, workflow, engineering and evidence rules, the milestone ritual pointer |
| `.planning/AUTONOMY.md` | Owner delegation, the discuss fan-out protocol, the verification protocol (shift left), the CI-efficiency rule, **green-locally-before-push**, **process/container/resource cleanup (`ivs-<purpose>`)**, the release/CD flow, the hygiene check, the milestone ritual, local CI replay (`act`, `tools/gate-linux.sh`) |
| `.planning/ops/MILESTONE-RITUAL.md` | The six-step ritual every new milestone runs: refresh rolling roadmap → inspect → fan out → upstream scoping pass → freeze and run → one command; plus the mandatory clean-state close |
| `.planning/ops/GUARDRAILS-v0.3.md` | **Current.** The retarget table V1–V11 (what must move in the same commit as what), the milestone rules (no new step/fault/golden; cost is `cpu_s`; the checkpoint vocabulary; rights before bytes), the tier ceilings, the definition of done, the discuss/verify boilerplate, the close-state checklist |
| `.planning/ops/GUARDRAILS-v0.2.md` | **Superseded 2026-11-26, kept as recorded.** Its R-rows are cited by `GUARDRAILS-v0.3` V5/V7 and still bind where cited |
| `.planning/ops/EXEC-RULES.md` | What an executor may and may not do (no `gh`, no tags, never stage orchestrator state) |
| `.planning/ops/RELEASE-OPS.md` | The release step order and the checks P1–P8 |
| `.planning/ops/PLAN-LESSONS.md` | Numbered planning lessons 1–25; cite them by number in plans |
| `.planning/ops/verify-push.sh` | Helper for the verification-push shape |

## Project state (the current truth)

| File | Holds |
|---|---|
| `.planning/PROJECT.md` | What this is, Core Value, requirements by state, constraints, the Decision Register (owner decisions, delegated decisions, recommendations, open questions) |
| `.planning/REQUIREMENTS.md` | The current milestone's REQ-IDs and traceability (archived per milestone under `.planning/milestones/`) |
| `.planning/ROADMAP.md` | The rolling roadmap: near / mid / long horizons, plus the archived milestone detail |
| `.planning/STATE.md` | Frontmatter state, current position, releases, accumulated context |
| `.planning/MILESTONES.md` | What shipped, per milestone, with the key accomplishments |
| `.planning/RETROSPECTIVE.md` | What worked, what was inefficient, patterns, key lessons, cross-milestone trends, and **"Carried Into the Next Milestone"** |
| `.planning/CI-EFFICIENCY.md` | Append-only hosted-run log and the cost rule: budget 7.8, revisit 8.3, the pre-committed lever order, per-phase checkpoints |
| `.planning/ci-runs.tsv` | The machine-readable hosted rows the cost rule is computed from |
| `.planning/milestones/<version>-*/` | Archived phases, requirements, roadmap and close record for a shipped milestone |
| `data/claims.json` | The claim ledger: every claim with its profile, evidence tier, artifact and standing |
| `data/discrepancies.json` | Conflicts and pinned defaults with their resolvers |
| `data/expected/important-gates.ids` | The registry that ties each requirement to a mechanism that cannot pass vacuously |
| `data/expected/handoffs.ids` | Items only a human can judge (D10, D11, H7-1/H7-2 look and listen) — never evidence, never citable by a claim |

## Rules of the second brain

1. **Write it once, then cite it.** A rule that exists in two places will drift;
   put it in one file and reference it by path and section.
2. **Label the claim.** `[OBS]` observed in the repo, `[INF]` inference,
   `[REC]` recommendation, `[UNK]` unknown. Owner decisions, delegated
   decisions, recommendations and open questions stay in separate tables in
   `PROJECT.md` so a recommendation is never mistaken for a decision.
3. **Recompute figures, never quote them.** Cost medians, counts and coverage
   are derivable from committed files. A quoted figure that disagrees with the
   file is a defect; record the correction where the stale figure lives.
4. **Archives are append-only.** A correction to a frozen oracle or an archived
   record is an appended note with its own commit, never an edit of the frozen
   line.
5. **Close the loop.** A lesson becomes a check, a control, a guardrail row or a
   ledger entry — otherwise it will recur. "We learned X" with no mechanism is
   not a lesson, it is a note.
