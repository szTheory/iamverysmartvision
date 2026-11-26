# Phase 18: The Inspection Surface - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-11-26
**Phase:** 18 - The Inspection Surface
**Areas discussed:** Frozen oracle, The tool, Trace format, Breakpoints, Disassembly limb, Rights

---

## Frozen oracle

| Option | Description | Selected |
|--------|-------------|----------|
| Freeze `docs/inspection.md` alone in wave 1, prove ancestry, document wins over plan prose | The contract is written before the code exists, and a plan sentence that disagrees becomes a recorded deviation | ✓ |
| Write the document alongside the tool | Cheaper, but the document then describes what was built rather than what must be built — the criterion's falsifiability is exactly the ordering | |
| No document; the tests are the contract | The tests cannot state the refusal tokens or the rights posture a reader needs | |

**User's choice:** none — decided autonomously (reversible technical: one new file).
**Notes:** `GUARDRAILS-v0.4` W5 and criterion 1 both pin the ordering. The document is written from `ivs_machine_peek` and `ivs_machine_step_unit_trace_internal`, the seams that exist today.

---

## The tool

| Option | Description | Selected |
|--------|-------------|----------|
| One binary, four subcommands, reusing the runner's scenario and observation sources | Its `digest.*` lines are byte-identical to `ivs-run`'s by construction, so the reconciliation id is falsifiable | ✓ |
| One binary that re-implements the report writer | A second truth: the equality would hold only by luck, and the id would pass vacuously on a shared bug | |
| Four separate binaries | Four targets, four install-less rules and four argument parsers for one command surface | |

**User's choice:** none — decided autonomously (reversible technical: a build target and a source list; the rejected option is the one that would be expensive to undo, and it is refused on that ground).
**Notes:** No new core function, no new sink, no file I/O in the core (D-52). The tool opens its own output file.

---

## Trace format

| Option | Description | Selected |
|--------|-------------|----------|
| One text record per line, tick first, then kind, address, value and the unit's stamps | A shell id asserts `records > 0`, non-decreasing ticks and the last tick without a parser | ✓ |
| A binary record file | Needs a reader in the test, which is a second parser for a build-only artifact | |
| JSON lines | A dependency or a hand-rolled emitter for no evidence the text form lacks | |

**User's choice:** none — decided autonomously (reversible technical: a writer function in a build-only tool).
**Notes:** The `ivst` extension is refused by name in `tools/check/binary-ext.txt` and `.gitignore` in the same commit as the rights row.

---

## Breakpoints

| Option | Description | Selected |
|--------|-------------|----------|
| Hit count and non-hit count, asserted as a pair in one id, always-hits stand-in failing the zero case | A matcher that always hits cannot pass the 0 case, so the pair is falsifiable | ✓ |
| Hit count only | A matcher that always hits passes it; the id would prove nothing | |
| A separate id per breakpoint | More ids for one mechanism, and the pair is the property | |

**User's choice:** none — decided autonomously (reversible technical: one report field and one id).
**Notes:** A breakpoint must not alter `digest.*`; that non-interference is asserted separately from the trace's, because it is a separate seam.

---

## Disassembly limb

| Option | Description | Selected |
|--------|-------------|----------|
| Build `18-03`, record the drop's reversal condition | The limb is priced and pre-authorized either way; the round trip is the only limb whose absence costs no falsifiability, so building it is upside with a cheap reversal | ✓ |
| Take the pre-authorized drop now | Legitimate and recorded, but it removes a reader's convenience for a saving the milestone's allowance does not need | |
| Build it without the two-word-form and immediate requirement | The round trip would pass on a corpus of single-word instructions — a vacuous case | |

**User's choice:** none — decided autonomously (reversible technical: a suite case in an existing suite).
**Notes:** The drop's condition is "cannot round-trip a corpus containing a two-word form and an immediate inside the priced line", not "we ran out of time".

---

## Rights

| Option | Description | Selected |
|--------|-------------|----------|
| One `inspection-output` row, three tokens, `ivst` refusals in the same commit | One row for one artifact class; the three tokens name the three shapes the tool can write | ✓ |
| One row per token | Three rows for one item, and `rights.ids` would carry three ids for one disposition | |
| No row; rely on `.gitignore` | A `.gitignore` line establishes nothing about rights, and the criterion asks for the row | |

**User's choice:** none — decided autonomously (reversible technical: a JSON row, an ids line and a document row).
**Notes:** `disposition: owner-local-only`, `source: null`, nothing taken. Refused inside the work tree with `inspect-output-tree`, `refs/` the documented exception, existing file refused.

---

## Decided autonomously

[auto] [Frozen oracle] — Q: "what does the document pin, and how is its priority enforced?" → Selected: "the command surface, the record fields, the liveness guards and the refusal tokens, frozen alone in wave 1 with an ancestry proof, and the document wins over plan prose" (reversible technical — decided autonomously: prose in one new file; no code depends on its layout beyond the section names)
[auto] [The tool] — Q: "one binary or four, and does it re-implement the report?" → Selected: "one binary with four subcommands, reusing the runner's scenario and observation sources so its digest.* lines are byte-identical to ivs-run's" (reversible technical — decided autonomously: a build target and a source list; the refused alternative is the only one that would be costly to undo)
[auto] [Trace format] — Q: "what shape are the records?" → Selected: "one text record per line, tick first, then kind, address, value and the unit's stamps" (reversible technical — decided autonomously: a writer function in a build-only tool, invisible to the core and to every digest)
[auto] [Breakpoints] — Q: "hit count only, or hit and non-hit?" → Selected: "both, asserted as a pair in one id, with an always-hits stand-in failing the zero case" (reversible technical — decided autonomously: one report field and one id)
[auto] [Disassembly limb] — Q: "build 18-03 or take the pre-authorized drop?" → Selected: "build it, with the drop's reversal condition recorded" (reversible technical — decided autonomously: a suite case in an existing suite, and the drop costs a document paragraph and a TOOL-02 note)
[auto] [Rights] — Q: "one row or several, and which tokens?" → Selected: "one inspection-output row with three tokens, plus the ivst refusals in the same commit" (reversible technical — decided autonomously: a JSON row, an ids line and a document row, revertible together)

## Claude's Discretion

- The exact record field names and their order after the tick, the subcommand flag spellings beyond the four names, and the internal module split under `tools/inspect/`.
- How much of `runner/` the tool links: the smallest set that makes the `digest.*` equality true rather than approximate, stated in the plan.

## Deferred Ideas

- A windowed or interactive inspection surface: refused — the tool is scripted and headless, and a window belongs to a host (D-49).
- Reading third-party or operator content through the tool: allowed only as an operator-local artifact under the rights row, never in CI, never committed.
- A second decoder table for disassembly: refused — the assembler and the committed listings already pin one, and a second table is a second truth.
