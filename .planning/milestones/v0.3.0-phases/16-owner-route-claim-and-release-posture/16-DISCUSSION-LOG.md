# Phase 16: Owner Route, Claim and Release Posture - Discussion Log

> **Audit trail only.** Decisions are in CONTEXT.md.

**Date:** 2026-11-26
**Phase:** 16-owner-route-claim-and-release-posture
**Areas discussed:** The owner route is a rehearsal, never a criterion; Handoffs, never criteria; Release posture; The concurrency lever, taken once, cost-only

## The owner route is a rehearsal, never a criterion

| Option | Description | Selected |
|--------|-------------|----------|
| A release-time rehearsal from the source archive, transcript recorded | Three commands; project-made pass path; one-byte mutation proves the id can fail | ✓ |
| A gate step | Needs the archive in CI; a hosted minute not spent on the gate | |

## Handoffs, never criteria

| Option | Description | Selected |
|--------|-------------|----------|
| Three new `handoffs.ids` rows, `evidence: none`, cited by nothing | D10/D11 stay not-performed | ✓ |
| A phase criterion for the owner's run | Re-asks what no tool can judge | |

## Release posture

| Option | Description | Selected |
|--------|-------------|----------|
| The existing `RELEASE-OPS.md` line, recomputed; package stays eleven files | No shared object, no host shim, no content | ✓ |
| A new release document | Duplicates a line that exists | |

## The concurrency lever, taken once, cost-only

| Option | Description | Selected |
|--------|-------------|----------|
| Overlap `c-tests` and `c-faults`, prove the token line byte-identical and compare each `step.out.<name>` | Wall time moves, evidence does not | ✓ |
| Cache, a path filter or a matrix | Forbidden: each weakens or skips evidence | |

## Decided autonomously

[auto] [Owner route] — Q: "Which three commands does the route document?" → Selected: "build from the archive, wrap a flat image, run the route" (reversible technical — a doc edit)
[auto] [Owner route] — Q: "Where does the transcript live?" → Selected: "the release record, cited from docs/owner-images.md, not a gate artifact" (reversible technical — a record path)
[auto] [Handoffs] — Q: "Do the three new rows need their own ids?" → Selected: "yes, three new handoffs.ids rows beside the existing ones" (reversible technical — inventory rows)
[auto] [Release posture] — Q: "Is the release-decision line a new document?" → Selected: "no — the existing RELEASE-OPS.md line, recomputed" (reversible technical — one line)
[auto] [Concurrency] — Q: "How is the overlap proved to change nothing?" → Selected: "byte-identical summary token line plus a cmp of each step.out.<name>" (reversible technical — the existing buffers)
[auto] [Cost] — Q: "Does the checkpoint add a measurement run?" → Selected: "no — recompute from the committed .planning/ci-runs.tsv rows" (reversible technical — the file is the arithmetic input)

## Claude's Discretion

None.

## Deferred Ideas

- D10/D11: handoff rows, never performed by the project.
- Shipping any host artifact: refused.
- A real frontend loading the shim: a handoff row.
- Bank modes and `.cfg`: FMT-01/FMT-04 stay `Deferred`.
