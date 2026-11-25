# Phase 10: Reachable Runs and Presentation Policy - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-11-24
**Phase:** 10-reachable-runs-and-presentation-policy
**Areas discussed:** the documented route, the presentation policy, the handoff ledger, the two guards, ordering

---

## How this discussion was conducted

No question here was hard to reverse or a genuine preference. The scope is frozen
in `.planning/MILESTONE-CONTEXT.md` §2 Phase 10 as five numbered deliverables, so
re-asking it would be re-litigating a recorded decision. Five technical questions
were decided autonomously; one was rated **costly** and is recorded as such.

## The documented route

| Option | Description | Selected |
|--------|-------------|----------|
| `docs/running.md` with the three commands and the exact tail | Document only | ✓ |
| A helper script wrapping the three commands | Needs zero logic or a new gate id | |

**User's choice:** decided autonomously (reversible technical).
**Notes:** the reachability proof is the release rehearsal, not a CI step — the
gate already pays for a cold build and a second one would be duplicate cost.

## The presentation policy

| Option | Description | Selected |
|--------|-------------|----------|
| Reuse `core-symbols`'s deny list and its existing control | One control file, no new step | ✓ |
| A new deny-list step for colour symbols | A new gate step, forbidden this milestone | |

**User's choice:** decided autonomously (reversible technical).
**Notes:** the policy document is committed before the check that enforces it
(oracle-first), and the ancestry is proved.

## The handoff ledger

| Option | Description | Selected |
|--------|-------------|----------|
| `data/expected/handoffs.ids`, five rows, `evidence: none`, each dispositioned | The ledger becomes the project's record of what a human owes | ✓ |
| A prose section in STATE.md listing the same items | Prose drifts and cannot be cited by a claim check | |

**User's choice:** decided autonomously, rated **costly** — a later milestone
cites the row id instead of re-asking, so changing a row's meaning after the fact
re-opens a closed question.

## The two guards

| Option | Description | Selected |
|--------|-------------|----------|
| Two guards: `claims` for artifacts, `text` for words | They fail for different reasons; one guard would blur which fired | ✓ |
| One combined guard | Simpler, but the failure reason becomes ambiguous | |

**User's choice:** decided autonomously (reversible technical).

## Ordering

| Option | Description | Selected |
|--------|-------------|----------|
| Policy documents first, then their checks, with the ancestry proved | Oracle-first (§2.9) | ✓ |
| Checks first, documents after | Unprovable order | |

**User's choice:** decided autonomously (reversible technical).

---

## Decided autonomously

[auto] [Route] — Q: "Document or helper script for the three commands?" → Selected: "Document only." (reversible technical — decided autonomously: one markdown file)
[auto] [Route] — Q: "Where is the reachability proof taken?" → Selected: "In the release rehearsal, not in CI." (reversible technical — decided autonomously: the gate already pays for a cold build)
[auto] [Policy] — Q: "A new deny-list step or the existing `core-symbols` one?" → Selected: "The existing step." (reversible technical — decided autonomously: a control file)
[auto] [Ledger] — Q: "Ids file grammar?" → Selected: "`<id> <disposition> evidence: none`, one line each, five rows." (reversible technical — decided autonomously: an ids file)
[auto] [Guards] — Q: "One combined guard or two?" → Selected: "Two." (reversible technical — decided autonomously: two controls)

## Claude's Discretion

The rehearsal mode's shape; the handoff packet's layout; whether the `text`
claim-word addition needs one control or two.

## Deferred Ideas

Installing the runner (D14), a windowed frontend, a palette table, a second
cartridge, any new gate step, and performing H7-1/H7-2 (the owner's).
