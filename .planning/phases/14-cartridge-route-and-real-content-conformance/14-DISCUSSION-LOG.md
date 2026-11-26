# Phase 14: Cartridge Route and Real-Content Conformance - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-11-26
**Phase:** 14-cartridge-route-and-real-content-conformance
**Areas discussed:** The wrap tool states bytes, never a format; The cartridge role in the owner-local route; The interrupt-enable default (D-51, CPU-06); Gate cost and evidence discipline

---

## The wrap tool states bytes, never a format

| Option | Description | Selected |
|--------|-------------|----------|
| Byte length + word order + operator-stated base (D-47) | `tools/flatwrap <flat> <base> <out> [--word-order hi-first\|lo-first]`, nothing detected | ✓ |
| Auto-detect the layout | Infer word order and base from the image | |
| Parse `.cfg` and honour its memory map | The FMT-04 importer | |

**User's choice:** none asked — frozen by D-47 and scoping R-2. `.cfg` stays refused by name.
**Notes:** the output is derived material of rights-bearing content: never committed, golden, fixture or claim artifact.

---

## The cartridge role in the owner-local route

| Option | Description | Selected |
|--------|-------------|----------|
| A cartridge role with all six derived-output refusals extended | Pass path proved with project-made zero-filled images | ✓ |
| A cartridge role proved only with the owner's own images | Requires D11 files to test | |
| Leave the route EXEC/GROM only | The content path would stop at the container | |

**User's choice:** none asked — SCN-03 fixes the shape, and the frozen verification strategy requires the pass path on project-made files.

---

## The interrupt-enable default (D-51, CPU-06)

| Option | Description | Selected |
|--------|-------------|----------|
| Resolve to `1`, keep the `unknown-default-chosen` tag, pin both settings | Retarget digest, every literal and the boot-stub-2 expectations in one commit | ✓ |
| Flip the default and delete the not-cleared test | Loses the other setting's coverage | |
| Record the observation and change nothing | The EXEC still dies at the first interrupt | |

**User's choice:** none asked — D-51 freezes the mechanism; the both-settings test placement was decided autonomously.

---

## Gate cost and evidence discipline

| Option | Description | Selected |
|--------|-------------|----------|
| Recompute the last-five medians from `.planning/ci-runs.tsv` | One recorded verdict, arithmetic beside it | ✓ |
| Add a measurement run | Spends hosted minutes to re-derive a committed figure | |

**User's choice:** none asked — GUARDRAILS §2.3 freezes the checkpoint rule.

---

## Decided autonomously

[auto] [The wrap tool] — Q: "Where does the flat image's word order default come from?" → Selected: "hi-first, the project's one wire order, with lo-first accepted and nothing auto-detected" (reversible technical — decided autonomously: a flag default is one line plus its test)
[auto] [The wrap tool] — Q: "Does the refusal list live in a new file or an existing inventory?" → Selected: "a new data/expected/flat-refusals.ids, pinned append-only" (reversible technical — decided autonomously: an inventory file plus its reading id)
[auto] [The wrap tool] — Q: "How is the output's never-committed rule enforced?" → Selected: "the existing *.ivsc entry in .gitignore and binary-ext.txt, plus a files id that would refuse a tracked one" (reversible technical — decided autonomously: the surfaces already exist)
[auto] [The cartridge role] — Q: "Which images does the pass path use?" → Selected: "zero-filled 8192/2048-byte files, shape-legal, so no owner image is needed" (reversible technical — decided autonomously: test data, not a contract)
[auto] [The interrupt-enable default] — Q: "Where is the both-settings test pinned?" → Selected: "in the existing CPU test suite as a project-authored vector pair" (reversible technical — decided autonomously: an id inside a suite that exists)
[auto] [The interrupt-enable default] — Q: "Is the next blocker's row a phase criterion or a record?" → Selected: "a record with its own gate; criterion 7 says named, not fixed" (reversible technical — decided autonomously: a record edit)
[auto] [Gate cost] — Q: "Does the phase add a measurement run for the cost checkpoint?" → Selected: "no — recompute from the committed .planning/ci-runs.tsv rows" (reversible technical — decided autonomously: the file is the arithmetic input)

## Claude's Discretion

None.

## Deferred Ideas

- `.cfg` parsing and bank switching (FMT-01/FMT-04 stay `Deferred`).
- The STIC register-window / strobe rule: recorded with its own row and gate.
- Any host, shared object or libretro shim: Phase 15.
- Any real EXEC/GROM/ROM digest or committed content byte: never.
