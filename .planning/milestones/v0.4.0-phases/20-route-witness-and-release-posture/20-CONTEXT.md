# Phase 20: Route, Witness and Release Posture - Context

**Gathered:** 2026-11-27
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 20 closes the milestone's loop and adds **no capability**: an operator section in
`docs/inspection.md` whose command lines run **as written**, the instrument named in
`docs/owner-images.md`'s owner-local route, the **D10-VID-08 packet** as a handoff row
naming the exact trace lines a hardware probe must contradict, and a **recomputed**
release-decision line.

Out of scope: no new id in the gate, no new gate step (38), fault (238), golden (29),
claim (`claims=29`, `provisional=28`), installed file (11), and no rights token —
`data/expected/rights-pinned.ids` stays at **four pairs**. The packet is a **row, not a
gate**: no criterion in this milestone requires its answer or the D11 sweep to be
performed. Tier ceiling: **none**.

</domain>

<decisions>
## Implementation Decisions

### The operator route
- `docs/inspection.md` gains an **operator section** whose command lines are run **as written** against a **project-authored** scenario (build, `trace`, `break`, `dis`, read the output under `refs/`). A command that only works with a local tweak is a defect: the section is executed by a test-like check, not proofread.
- `docs/owner-images.md` names the instrument in the owner-local route, so a person with their own files can reach it.
- The naming limb widens by **this one exact page and zero rights tokens**: `rights-pinned.ids` stays four pairs, and no refused token spelling is introduced.

### The D10-VID-08 packet
- **Exactly one** row is added to `data/expected/handoffs.ids`: `D10-VID-08 not-performed evidence: none`, so the claims step prints `handoffs=9`. `docs/handoffs.md` carries the packet's section and `docs/stic/d10-procedure.md` carries the anchor that section cites.
- The packet names the **exact trace lines a hardware probe must contradict**: the record fields, and per guest the observable line with its **two candidate values** under the rival rules Phase 19 measured. It is written as a list a probe's capture is compared against, not as prose.
- Its own status line reads either **the four rival candidates from Phase 19's guests** or **"unproduced by instrument"** — Phase 19 shipped the instrument and took Exit B, so the candidates are available and are what the packet must carry.
- **No claim cites the row**: the claim-artifact predicate refuses a handoff id, and that refusal is **shown able to fail** on a committed stand-in.

### The release-decision line
- Recomputed per `.planning/ops/RELEASE-OPS.md` section (i) and reading exactly: *the release ships the static package and the source archive, which carries the source of the build-only tools and of the build-only host shim; no shared object, no built host shim, no built inspection tool, no trace or disassembly output, no content* — with `data/expected/pkg-install-files.ids` still **11** and `release-assets` unchanged.
- "Recomputed" means the checks behind each clause are re-run and the line is written from their output, not copied from the v0.4.0 opening record.

### Decided autonomously
- [Operator route] Q: "where does the route live, and how is it proved?" → Selected: "an operator section in `docs/inspection.md` executed as written against a project-authored scenario, plus a mention in `docs/owner-images.md`, with `rights-pinned.ids` unchanged" — prose plus one run; no token, no id.
- [Packet shape] Q: "prose or a comparison list?" → Selected: "a list of exact trace lines with, per guest, the observable and its two candidate values, plus a status line naming the candidates" — a document section; the row is one line in an existing inventory.
- [Handoff row] Q: "one row or several?" → Selected: "exactly one, `D10-VID-08 not-performed evidence: none`" — `handoffs=9` is the whole count change.
- [Release line] Q: "quote or recompute?" → Selected: "recompute each clause's check and write the line from the output" — the line is one sentence; the checks are already in the gate.

### Claude's Discretion
- The operator section's exact command spellings (within what the frozen page states), the packet's table versus list layout, and where in `docs/handoffs.md` the section sits.

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- `docs/inspection.md` — the frozen contract with its `## Amendments`; the operator section extends it (an appended section is an amendment, not a rewrite).
- `docs/owner-images.md` — the owner-local route already rehearsed from the source archive; the instrument joins it.
- `data/expected/handoffs.ids` — eight rows today; the ninth is this phase's.
- `docs/handoffs.md` and `docs/stic/d10-procedure.md` — the packet's home and the anchor it cites.
- `.planning/ops/RELEASE-OPS.md` section (i) — the release-decision line, already carrying the source-archive correction Phase 17 made.
- `data/expected/rights-pinned.ids` — four pairs; unchanged is the assertion.

### Established Patterns
- A handoff row is a ledger row with `evidence: none`, never a phase exit criterion, and no claim may cite it.
- A document section that states a command is proved by running the command.
- A release line is recomputed from checks, never quoted from an earlier record.

### Integration Points
- `claims` step prints `handoffs=<n>`; `release-assets` and `pkg-install` are the two checks behind the release line.
- `docs/inspection.md`'s operator section must agree with its `## Command surface`; a disagreement is a deviation recorded against the page.

</code_context>

<specifics>
## Specific Ideas

`GUARDRAILS-v0.4` §5 answered before planning:

1. **W rows and rights rows touched.** **W6** (a handoff row: `data/expected/handoffs.ids`, a `docs/handoffs.md` section, the `docs/stic/d10-procedure.md` anchor, never an exit criterion, no claim cites it) and **W10** (release text: `pkg-install-files.ids`, `docs/packaging.md`, `.planning/ops/RELEASE-OPS.md`, `release-assets`). **No `data/rights/` row changes**: `rights-pinned.ids` stays four pairs.
2. **Rival rule pair separated.** None new — the packet names the pairs Phase 19 separated, with their two candidate values each.
3. **Count delta and its price.** `handoffs` 8 → **9**; `steps` 38, `faults` 238, `goldens` 29, `claims` 29, `provisional` 28, `prefix-files` 11, `rights-pinned` 4, `tests-expected`/`tests-executed` unchanged (no new id), `gates`/`lines` unchanged unless a registry row is needed for the claim-refusal stand-in — if one is, the plan states it. Priced **0.02 hosted cpu-s**.
4. **Oracle, freeze commit, liveness guard, stand-in.** The oracle is `docs/inspection.md`'s command surface (frozen) and Phase 19's measured candidates. The liveness guard is the claim-artifact predicate refusing a handoff id, with a **committed stand-in** shown failing.
5. **Rights posture in one line.** Nothing rights-bearing is added: the packet names trace *lines*, not content; no operator file's digest, size, path or name appears anywhere.
6. **What is deferred, and where.** The hardware probe itself (D10) and the owner-local sweep (D11) stay `not-performed` handoffs; this phase records them, it does not perform them.
7. **Evidence tier.** **None.** The packet is a handoff row with `evidence: none`; `claims=29` and `provisional=28` are invariants.

</specifics>

<deferred>
## Deferred Ideas

- Performing the D10-VID-08 probe or the D11 sweep — owner hardware and owner files; both stay `not-performed`.
- A prebuilt host or a shared object in a release — D-49 stays, and the release line says so.
- Widening the naming limb beyond the one page — refused by the criterion.

</deferred>
