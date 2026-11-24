# Phase 9: Registered Evidence and Gate Cost - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-11-24
**Phase:** 9-registered-evidence-and-gate-cost
**Areas discussed:** Wave A1 scope, EV-02 registration, EV-04 machine check, EV-03 hardening, preflight partition, bookkeeping/settings/rulesets, ordering and the hosted row

---

## How this discussion was conducted

No area produced a question that was hard to reverse or a genuine preference.
Every question below is a technical question whose undo is local and cheap, so it
was decided autonomously and recorded in CONTEXT.md's "Decided autonomously"
block, as the smart-discuss classification requires. The scope itself was not
re-derived: it is frozen in `.planning/MILESTONE-CONTEXT.md` §2, retargeted by
`.planning/ops/GUARDRAILS-v0.2.md`, and priced in `.planning/CI-EFFICIENCY.md`
"next-milestone opening". Re-asking it would be re-litigating a recorded
decision, which `INDEX.md` forbids.

Two questions were considered for a prompt and rejected as already answered:

- *"Should the branch and tag rulesets be applied?"* — `.planning/AUTONOMY.md` §1
  delegates it and writes out the exact command; `MILESTONE-CONTEXT.md` item 10
  assigns it to this phase. Decided, not open.
- *"Should lever (3) fold the two lamp-tone variants?"* — the pre-committed lever
  order is recorded policy with a measured ceiling. Decided, not open.

## Wave A1 scope

| Option | Description | Selected |
|--------|-------------|----------|
| Closed file list enumerated by the plan | The plan lists every file A1 touches; no file outside the list may change | ✓ |
| Free scope within the suites | Let the executor touch whatever the fold needs | |

**User's choice:** decided autonomously (reversible technical).
**Notes:** the failure this prevents is the recorded risk "cost work grows into a
gate redesign" (`MILESTONE-CONTEXT.md` §5), countered by the seven-file scope and
`GUARDRAILS` §2.

## EV-02 registration mechanics

| Option | Description | Selected |
|--------|-------------|----------|
| Only ids that already run | 24 rows naming existing faults, controls, mutants or suite self-controls | ✓ |
| Add a fault or test id to make a requirement coverable | Rejected: EV-02 forbids it and A1's row has not been recorded | |

**User's choice:** decided autonomously (reversible technical).
**Notes:** the 306 → 330 prediction and the ten toy registries are the diff
contract; an unexpected token is a stop.

## EV-04 machine check

| Option | Description | Selected |
|--------|-------------|----------|
| Derived accepted set in `important-gates.sh` + drift check in `run-controls.sh` | Two modes of existing scripts; no new step | ✓ |
| A new gate step for the cross-check | Rejected: forbidden this milestone (38 stays 38) | |
| A generated `requirements-complete.ids` inventory | Rejected as the sole mechanism: a derived inventory can drift from the records it derives from unless something outside it checks the drift, which is the property being asked for | |

**User's choice:** decided autonomously (reversible technical).
**Notes:** "marked Complete" is read from `.planning/REQUIREMENTS.md` and the
archived `v0.1.0-REQUIREMENTS.md`; the good tree already carries `.planning/` so
the controls can overlay it, and two steps already read `.planning/`.

## EV-03 gate hardening

| Option | Description | Selected |
|--------|-------------|----------|
| One control per hardening item | Five items, five controls, each shown to fail for its intended reason | ✓ |
| A control per failure mode | More controls, more gate content than the requirement asks for | |

**User's choice:** decided autonomously (reversible technical).
**Notes:** the memmap generator item may be deferred with its reason if it cannot
be reached by a pure step without a new step; a hollow control is not acceptable.

## Preflight partition

| Option | Description | Selected |
|--------|-------------|----------|
| Derived partition, buffered output | Preflight iff the script references neither the gate results directory nor `build/`; tokens buffered and concatenated in sorted name order | ✓ |
| Hand-maintained cheap list | Rejected: drifts, and `AUTONOMY.md` §4 says derive it | |

**User's choice:** decided autonomously (reversible technical).
**Notes:** the preserving condition is byte-identical full output, not just the
final line.

## Bookkeeping, settings and rulesets

| Option | Description | Selected |
|--------|-------------|----------|
| Apply the branch ruleset; apply the tag ruleset in the only shape it can express | Deletion and non-fast-forward on `refs/tags/v*`; "only from a green main" recorded as not expressible | ✓ |
| Record both as "not applied" with a reason | Available, but the phase is the assigned owner of the action | |
| Invent a tag rule that approximates provenance | Rejected: a rule that does something else is worse than a recorded limitation | |

**User's choice:** decided autonomously, with the reversibility rating **costly**
(a public settings object that changes who can land a commit; the reversal
command is recorded beside the id).

## Ordering and the hosted row

| Option | Description | Selected |
|--------|-------------|----------|
| A1 → hosted row → A2 | The row is the gate on any new gate content | ✓ |
| A1 and A2 in one push | Rejected: the row must be attributable to the cost changes alone | |

**User's choice:** decided autonomously (governance already recorded in
`GUARDRAILS` §2.2 and `CI-EFFICIENCY.md`).

---

## Decided autonomously

[auto] [Wave A1] — Q: "Which files may wave A1 touch?" → Selected: "The plan enumerates a closed list and no file outside it changes." (reversible technical — decided autonomously: reverting one commit restores every live launch)
[auto] [Wave A2 / EV-02] — Q: "Which mechanisms do the 24 rows name?" → Selected: "Only ids that already exist in a fault, control, mutant or suite self-control inventory." (reversible technical — decided autonomously: a registry line is one line and the step fails closed on an unknown id)
[auto] [Wave A2 / EV-04] — Q: "Where does the machine check live?" → Selected: "A derived accepted set inside `important-gates.sh` plus a toy-registry drift check inside `run-controls.sh`; no new step." (reversible technical — decided autonomously: both are modes of existing scripts)
[auto] [Wave A2 / EV-04] — Q: "Which records define 'marked Complete'?" → Selected: "`.planning/REQUIREMENTS.md` and `.planning/milestones/v0.1.0-REQUIREMENTS.md`." (reversible technical — decided autonomously: two steps already read `.planning/` and the good tree carries it)
[auto] [Wave A2 / EV-03] — Q: "One control per hardening item, or a control per failure mode?" → Selected: "One control per item, token pinned, shown to fail for its intended reason only." (reversible technical — decided autonomously: a control is a file under `tools/check/controls/`)
[auto] [Wave A2 / preflight] — Q: "Hand-maintained cheap list or derived partition?" → Selected: "Derived: preflight iff the script references neither the gate results directory nor `build/`." (reversible technical — decided autonomously: one file changes and the summary line is the proof)
[auto] [Settings] — Q: "Which shape for the tag ruleset?" → Selected: "Target `tag`, ref `refs/tags/v*`, deletion and non-fast-forward refused; provenance recorded as not expressible." (reversible technical — decided autonomously: a ruleset cannot express provenance)

## Claude's Discretion

A2's plan count and the ordering of the EV-03 items; whether the memmap generator
control is reachable without a new step; where the `release-only` skip vocabulary
lives.

## Deferred Ideas

Installing the runner (D14), a windowed frontend, a palette table, a second
cartridge, any new gate step, trim (b), and parallel negative controls. Each is
recorded in CONTEXT.md's Deferred section with its reason.
