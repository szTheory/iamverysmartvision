# Phase 11: Input Record and Replay - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-11-25
**Phase:** 11-input-record-and-replay
**Areas discussed:** the format, the two options, the refusal family, the liveness guards

---

## How this discussion was conducted

No question was hard to reverse except the format, which is rated **costly** and
recorded as such. The scope is frozen in `.planning/MILESTONE-CONTEXT.md` §2
Phase 11 and by decision **D15**, so re-asking it would re-litigate a recorded
decision.

## The format

| Option | Description | Selected |
|--------|-------------|----------|
| One `at_tick` line per event, header carries version/scenario/profile/content | Provably input-only; no field can hold a digest | ✓ |
| A field for content identity | Would make the log derived material under D11 | |
| A binary format | Not reviewable, and the rights story is a text story | |

**User's choice:** decided autonomously, rated **costly** — the format is a
published contract; a change means a version bump and a dual reader.

## The two options

| Option | Description | Selected |
|--------|-------------|----------|
| `runner/` only | The core gains no file I/O | ✓ |
| In the core | Would break the core's purity rule | |

**User's choice:** decided autonomously (reversible technical).

## The refusal family

| Option | Description | Selected |
|--------|-------------|----------|
| One token per class, seven of them, pinned | A reader can tell which rule failed | ✓ |
| One token for all malformed logs | Cheap, but the failure reason is lost | |

**User's choice:** decided autonomously (reversible technical).

## The liveness guards

| Option | Description | Selected |
|--------|-------------|----------|
| Chains plus a liveness guard, with two self-controls | A hook that does nothing matches everything otherwise (08-16) | ✓ |
| The equality alone | That is exactly the defect this phase exists to avoid | |

**User's choice:** decided autonomously (reversible technical).

---

## Decided autonomously

[auto] [Format] — Q: "One directive per event or one line per event?" → Selected: "One `at_tick` line per event." (reversible technical — decided autonomously: a text file, reversible before publication)
[auto] [Options] — Q: "Where do the two options live?" → Selected: "`runner/` only." (reversible technical — decided autonomously: a runner change)
[auto] [Refusals] — Q: "One token or seven?" → Selected: "One per class, seven, pinned." (reversible technical — decided autonomously: ids files)
[auto] [Guards] — Q: "Chains alone or chains plus a guard?" → Selected: "Both, with two self-controls." (reversible technical — decided autonomously: test ids)

## Claude's Discretion

The exact token spellings, whether the process case is one id or two, and where the
`at_tick` bound comes from.

## Deferred Ideas

A content-identity field (D15), promoting process cases, snapshot file options, and
any new gate step.
