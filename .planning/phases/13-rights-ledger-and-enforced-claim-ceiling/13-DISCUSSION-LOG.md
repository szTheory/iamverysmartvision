# Phase 13: Rights Ledger and Enforced Claim Ceiling - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-11-26
**Phase:** 13-rights-ledger-and-enforced-claim-ceiling
**Areas discussed:** Rights ledger shape and enforcement; Enforced claim ceiling (D-32 / D-50); The ten D-09-06 registrations (EV-07); Gate cost and evidence discipline

---

## Rights ledger shape and enforcement

| Option | Description | Selected |
|--------|-------------|----------|
| One JSON file per item, question-shaped (D-46) | `data/rights/<slug>.json`, keys `id`/`item`/`disposition`/`source`/`taken`/`tokens`/`note`; enforced as ids inside `files`/`manifests` | ✓ |
| A second manifest schema | Rights rows as another byte-set manifest with its own step | |
| Extend `data/sources.json` | Add a disposition to each reference document record | |

**User's choice:** none asked — frozen upstream by D-46 / scoping R-1, and the alternative is a recorded reversal condition ("a question whose answer is a byte set becomes a fixture manifest").
**Notes:** `data/expected/gate-steps/rights.ids` already owns `manifests` + `files`, so enforcement costs zero new steps.

---

## Enforced claim ceiling (D-32 / D-50)

| Option | Description | Selected |
|--------|-------------|----------|
| A rule inside `kinds/claim.cmake` | `established` requires `source-expectation` plus all `depends_on` fields at `tier1-primary-doc`; toy legal positive + two must-fail controls | ✓ |
| A separate claims step | New step, new cost line | |
| Prose policy in `PROJECT.md` only | D-32 stated, not enforced | |

**User's choice:** none asked — D-50 freezes the mechanism; the open sub-question (is the positive `CL-0002` itself or a toy row) was decided autonomously as a toy row so no live baseline moves for a test.

---

## The ten D-09-06 registrations (EV-07)

| Option | Description | Selected |
|--------|-------------|----------|
| Ten `mutant` rows over existing selftests | Registry rows only; baseline `lines=` re-derived in the same commit; ten toy registries +1 line each | ✓ |
| Ten new faults | Costs about 4.4 hosted cpu-s, 82 % of the milestone allowance | |
| Leave the ten unregistered | The v0.2.0 audit's open row stays open | |

**User's choice:** none asked — EV-07 and GUARDRAILS V11 freeze the mutant form; the naming and baseline mechanics were decided autonomously.

---

## Gate cost and evidence discipline

| Option | Description | Selected |
|--------|-------------|----------|
| Recompute from `.planning/ci-runs.tsv` | Last-five medians by arithmetic; record one checkpoint verdict | ✓ |
| Add a measurement run | Extra hosted minutes to re-derive what the committed rows already say | |

**User's choice:** none asked — GUARDRAILS §2.3 freezes the checkpoint rule.

---

## Decided autonomously

[auto] [Rights ledger shape and enforcement] — Q: "What slugs name the ten RTS-01 rows?" → Selected: "kebab-case item nouns, one file each: flat-cartridge-image, cfg-container, bank-modes, exec-image, grom-image, commercial-rom, rendered-frame, save-state, libretro-abi, core-info-strings" (reversible technical — decided autonomously: a file rename plus its id line; undo is local)
[auto] [Rights ledger shape and enforcement] — Q: "Does a cleared row's source resolvability consult the source record's rights field?" → Selected: "No — membership in data/sources.json only; the source rights field answers the reference document's own rights, a different question" (reversible technical — decided autonomously: the rule is one branch in the check)
[auto] [Rights ledger shape and enforcement] — Q: "Which pages count as the operator pages of docs/?" → Selected: "an explicit pinned list (README.md, docs/running.md, docs/owner-images.md, docs/packaging.md, docs/diagnostics.md, docs/RIGHTS.md), not a docs glob" (reversible technical — decided autonomously: the scope is a data list; changing it is a one-line diff)
[auto] [Rights ledger shape and enforcement] — Q: "Where does the rights check get its SRC- resolution and step membership helpers?" → Selected: "reuse tools/check/manifest-lib.cmake and the existing files/manifests step harness" (reversible technical — decided autonomously: a helper extraction is local to two files)
[auto] [Enforced claim ceiling (D-32 / D-50)] — Q: "Is the legal established positive CL-0002 itself or a toy row?" → Selected: "a toy legal claim in the checker's fixture set; CL-0002 is asserted unchanged" (reversible technical — decided autonomously: a fixture swap touches no live baseline)
[auto] [The ten D-09-06 registrations (EV-07)] — Q: "How are the ten rows named and where does the baseline live?" → Selected: "mutant rows named after their existing selftest cases; the lines= header of data/expected/important-gates.ids is re-derived in the same commit" (reversible technical — decided autonomously: a count is arithmetic over a committed file)
[auto] [The ten D-09-06 registrations (EV-07)] — Q: "Do the ten toy registries gain their line in the same commit as the rows?" → Selected: "yes — one line each, zero deletions, in the row commit (GUARDRAILS V7)" (reversible technical — decided autonomously: the toy registries are test data)
[auto] [Gate cost and evidence discipline] — Q: "Does the phase-boundary checkpoint add a CI measurement run?" → Selected: "no — it recomputes from the committed .planning/ci-runs.tsv rows" (reversible technical — decided autonomously: re-reading the file costs nothing)

## Claude's Discretion

None.

## Deferred Ideas

- `refs/`-backed content inventory and any real EXEC/GROM/ROM digest (D11 handoff).
- `.cfg` parsing and bank-mode implementation (FMT-01/FMT-04 stay `Deferred`).
- A vendored libretro header (D-48 refuses it unless a real frontend needs one).
- The debugger/inspection surface (TOOL-02), mid horizon.
