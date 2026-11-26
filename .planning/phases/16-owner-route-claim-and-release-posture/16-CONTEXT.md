# Phase 16: Owner Route, Claim and Release Posture - Context

**Gathered:** 2026-11-26
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 16 closes the milestone's loop: the owner route from the source archive is
documented and rehearsed, the questions only the owner can answer are handoff
rows, and the release posture says plainly that no host artifact and no content
ships.

1. **One documented owner route** — EXEC + GROM + a wrapped cartridge, three
   commands — rehearsed **from the source archive** to a project-cartridge
   golden, with the transcript recorded. A release-time rehearsal, not a gate
   step.
2. **The route's pass path exercised with project-made files**, and a one-byte
   EXEC mutation proving the id can fail; the `exec:` label stays `-` so no run
   reads as a real-EXEC result.
3. **The owner-only questions are `handoffs.ids` rows**, `evidence: none`,
   dispositioned, cited by no claim and by no phase exit criterion.
4. **A tracked shared object is refused by name in both directions**, the
   installed package stays eleven files, and the release-decision line reads: the
   release ships the static package and the source archive; no shared object, no
   host shim, no content.
5. **The concurrency lever taken once, cost-only**: `c-tests` and `c-faults`
   overlapped, the summary token line byte-identical, each `step.out.<name>`
   compared, and one checkpoint line in the §2.3 vocabulary with `steps=38/38`.

Out of scope: any new claim (the claim was minted in Phase 15), any new gate
step, fault or golden, any content byte, any host artifact.

</domain>

<decisions>
## Implementation Decisions

### The owner route is a rehearsal, never a criterion
- One documented route from the **source archive** to a project-cartridge golden,
  three commands, with the transcript recorded. It is a release-time rehearsal,
  not a gate step: the archive is the released artifact and the rehearsal proves
  the route a person actually has. — **Reversibility:** costly — the transcript
  is a record other documents cite; re-cutting it later re-opens the release
  notes and the rehearsal record.
- The pass path uses project-made files (zero-filled 8192/2048-byte images are
  shape-legal) and a one-byte EXEC mutation proves the id can fail. The `exec:`
  label stays `-`, so nothing reads as a real-EXEC result (D11 is still
  not-performed).
- `--source-route` stays release-time, never CI: it needs the archive, and a
  hosted minute spent on it is a hosted minute not spent on the gate.

### Handoffs, never criteria
- The owner-only questions are `handoffs.ids` rows with `evidence: none`,
  dispositioned, cited by no claim and by no phase exit criterion (GUARDRAILS
  V8). D10 and D11 stay `not-performed`; the three new rows are "a real frontend
  loads the shim", "an owner image's picture and sound look right", and "the
  owner's flat image needs no word-order guess".
- A handoff row is never a pass, never a claim's evidence, and never re-asked in
  a later milestone.

### Release posture
- A tracked shared object is refused by name in both directions (Phase 15's
  controls already exist; this phase proves the direction that catches a
  *committed* one), the installed package stays eleven files, and the
  release-decision line is recomputed rather than quoted.
- The release ships the static package and the source archive; no shared object,
  no host shim, no content.

### The concurrency lever, taken once, cost-only
- `c-tests` and `c-faults` are overlapped: two steps whose trees are disjoint and
  whose output the partition already buffers and replays in sorted order. The
  summary token line must stay byte-identical and each `step.out.<name>` must be
  compared, so the evidence is unchanged and only wall time moves.
- It buys wall time and Actions minutes, not ratio: no checkpoint credit, and
  `cpu_s` is expected to be unchanged.

### Gate cost and evidence discipline
- Priced addition **0.4 hosted cpu-s** against the 21 cpu-s allowance. No new
  gate step, fault or golden: `steps=38`, `faults=238`, `goldens=29` invariants.
- The close records one checkpoint line in the §2.3 vocabulary with
  `steps=38/38`.

### Decided autonomously
- [Owner route] Q: "Which three commands does the route document?" → Selected: "build from the archive, wrap a flat image, run the route — the same three a person performs" — a doc edit.
- [Owner route] Q: "Where does the transcript live?" → Selected: "the release record, cited from `docs/owner-images.md`, not a gate artifact" — a record path.
- [Handoffs] Q: "Do the three new rows need their own ids?" → Selected: "yes, three new `handoffs.ids` rows beside the existing ones" — inventory rows.
- [Release posture] Q: "Is the release-decision line a new document?" → Selected: "no — the existing `RELEASE-OPS.md` line, recomputed" — one line.
- [Concurrency] Q: "How is the overlap proved to change nothing?" → Selected: "byte-identical summary token line plus a `cmp` of each `step.out.<name>`" — the existing partition's own buffers.
- [Cost] Q: "Does the checkpoint add a measurement run?" → Selected: "no — recompute from the committed `.planning/ci-runs.tsv` rows" — the file is the arithmetic input.

### Claude's Discretion
None — every question was frozen upstream or decided autonomously under the reversible-technical rule.

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- `tools/release-rehearse.sh` already rehearses the source route (124 of 124 golden lines at the v0.2.0 release); `--source-route` is release-time.
- `tools/owner-run.sh` carries the EXEC/GROM/cartridge roles from Phase 14.
- `data/expected/handoffs.ids` and `docs/handoffs.md` are the handoff ledger and its packet.
- `data/expected/pkg-install-files.ids` (eleven) and the two `files-unlisted-binary-*` controls from Phase 15.
- `RELEASE-OPS.md` holds the release-decision line.

### Established Patterns
- A handoff row is `evidence: none`, cited by no claim, never a criterion (V8).
- A release-time rehearsal is recorded, not gated.
- A cost-only lever proves itself by a byte-identical token line and compared step buffers.

### Integration Points
- `docs/owner-images.md` → the three-command route → the rehearsal transcript in the release record.
- `data/expected/handoffs.ids` → `docs/handoffs.md`.
- `RELEASE-OPS.md` → the release-decision line.
- `tools/gate.sh` → the `c-tests`/`c-faults` overlap → the summary line.

</code_context>

<specifics>
## Specific Ideas

- The upstream freeze is the spec: `.planning/MILESTONE-CONTEXT.md` §2 (Phase 16's exit), §4 (verification strategy: the owner PASS path, the handoff rows), §5 (the concurrency lever, taken here, cost-only) and scoping R-5.
- The milestone's one claim (`CL-0029`) was minted in Phase 15; Phase 16 adds none.

</specifics>

<deferred>
## Deferred Ideas

- D10 physical measurement and D11 owner-local runs: handoff rows, never performed by the project.
- Shipping any host artifact: refused.
- A real frontend loading the shim: a handoff row.
- Bank modes and `.cfg`: FMT-01/FMT-04 stay `Deferred`.

</deferred>
