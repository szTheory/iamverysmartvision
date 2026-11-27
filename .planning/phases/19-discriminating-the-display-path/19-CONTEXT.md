# Phase 19: Discriminating the Display Path - Context

**Gathered:** 2026-11-27
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 19 is the milestone's point: it separates the **four rival display-path rules**
by discrimination rather than by fit. Four authored guests, one per rival, each with
**one machine-state observable** and a liveness guard a committed stand-in can fail;
the four fields swept as **16 cells** and shown **diagonal** (each guest moves under its
own field and under no other, at the default of the remaining three); then exactly one of
two honest exits — the decided default pinned by two ids with the whole `W1` retarget
chain in one commit, or "default unchanged, observation appended, rivals named".

Out of scope: no pixel comparison, no human judgement, no claim (`claims=29`,
`provisional=28` are invariants), no new golden (`goldens=29`; a moved golden is
retargeted), no new gate step (`steps=38`), no new fault (`faults=238`), no new
installed file (`prefix-files=11`), and no `measured-hardware` evidence — the field
stays tagged `unknown-default-chosen` in **both** exits (D-51: an observation is not a
hardware probe).

</domain>

<decisions>
## Implementation Decisions

### The four guests
- **One guest per rival rule, authored as project-owned content**: a source assembled by the project's own `ivs_asm` plus the scenario that drives it, under the fixtures the repository already uses. No third-party content, no operator file, nothing rights-bearing — the guests are ours and are committed as sources, not as binaries.
- Each guest exposes **exactly one machine-state observable**, named by the criterion and read from the runner's report (never a pixel, never a hash of a frame):
  - **G1** a WRITE to `$0020` at a cycle inside the 2000-to-2900 gap → frame 0 `displayed`.
  - **G2** a WRITE outside both window candidates with the display latch clear → `stic.blocked_writes`.
  - **G3** a READ of `$0020` with no write → frame 0 `displayed`.
  - **G4** one WRITE never re-strobed → `displayed` for frames 0, 1 and 2.
- Each guest ships a **liveness guard with a committed stand-in that fails it**, in the same commit: a guest whose observable cannot move is a dead guest, and the stand-in is what proves the guard is not vacuous.

### The 16-cell sweep, and the latch row
- The sweep is a **matrix of parameter rows**: four fields at two settings each, `2^4 = 16` cells, each cell a profile parameter row built explicitly (never a global edit), driven through the existing `stic` suite ids.
- **The latch rule becomes a row in `data/expected/profile-fields.ids` before it is swept.** That is a profile change and it drags the `W1` chain with it — the field row, its tag and cite, the parameter digest, and **every** snapshot literal in `data/expected/ivsn-literals.ids` — so the row and that chain land in **one commit**, before any sweep result is recorded. The field stays `unknown-default-chosen`.
- **Diagonality is asserted, not eyeballed**: a committed `mutant` row over the diagonal check is shown able to fail, so "each guest moves under its own field and no other" is falsifiable. A guest that moves under two fields is **re-derived**, not accepted with a note.

### The two exits
- **Exit A** (a default is decided) requires: two ids, one per setting, each building the alternate parameter row explicitly; and **one commit** carrying the whole `W1` chain — the profile field with its tag and cite, `data/expected/profile-fields.ids`, the parameter digest in `data/expected/ivsn-literals.ids` and the id that holds it, **every** snapshot literal in that file, any moved golden retargeted in place, the moved `stic` inventory ids, the C-row `evidence` line in `data/discrepancies.json` and `data/expected/discrepancies.ids`, and the prose. The commit's file list must equal the enumeration written into the phase record **before** the commit; a mismatch is a **stop**, not a re-bless.
- **Exit B** (nothing discriminates) reads "default unchanged, observation appended, rivals named" and is `Complete` for VID-07 **on its own terms** — a result, not a failure.
- In **both** exits the four rival rows in `data/discrepancies.json` carry the appended observation, the field stays `unknown-default-chosen`, no claim cites the observation, `claims=29`, `provisional=28` and `goldens=29` are unchanged, and no record holds a digest, size, path or name of an operator file.
- **Which exit is taken is decided by the measurement, not chosen in advance.** The plan writes both exits' commit shape before the sweep runs, so neither can be improvised afterwards.

### The owner-local sweep (`19-02`)
- It is a **throwaway probe and nobody's exit criterion**: it lives under `refs/` only, is bounded by a wall-clock limit and a run count, its output is refused outside `refs/` on the `tools/flatwrap` precedent, and it is `skip-ok` (exit 13, `missing-fixture`) in both test inventories.
- Its absence, or a run that ends inconclusive, leaves criteria 1 to 4 standing. **No criterion of this phase depends on a human act.**

### Decided autonomously
- [Guests] Q: "authored how, and with what observable?" → Selected: "one project-owned `ivs_asm` source plus its scenario per rival, one machine-state observable read from the report, one liveness guard with a committed stand-in" — fixture sources and scenarios; no binary, no third-party content, nothing rights-bearing.
- [Sweep] Q: "how are 16 cells driven?" → Selected: "sixteen explicitly built parameter rows through the existing `stic` ids, never a global edit" — parameter rows are data; a cell is a row, and adding or removing one moves no code.
- [Latch row] Q: "when does the latch rule become a profile field?" → Selected: "before the sweep, in one commit with the whole `W1` chain" — the chain is the price of any profile change and is enumerated in the same commit.
- [Exit choice] Q: "who decides A or B?" → Selected: "the measurement decides; both commit shapes are written before the sweep runs" — a plan that pre-writes both shapes cannot improvise the one that flatters the result.
- [Sweep containment] Q: "is the owner-local sweep an exit criterion?" → Selected: "no — `skip-ok`, bounded, `refs/`-only, and its absence leaves criteria 1 to 4 standing" — a probe under `refs/` that nothing cites cannot hold the phase hostage.

### Claude's Discretion
- The guest source shape (address layout, cycle offsets within the stated gaps), the scenario files' names, and the module split of the sweep driver.
- How the 16 cells are enumerated in the suite (one id per cell versus one id that loops the matrix) — bounded by the criterion's "diagonal" assertion and the cost line.

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- `data/expected/profile-fields.ids` — 154 fields, including `stic.access_effects`, `stic.reg_window_cpu_cycles` and `stic.access_gating_when_off`; the latch rule is the fourth field and is **not yet a row**.
- `data/discrepancies.json` — the `C82` row already states the four rivals in one `default_policy` sentence and names the resolver `D10-VID-08`; its `evidence` line names the six `stic.enable.seq.*` ids that exercise today's default.
- `tests/stic/` — the suite that already drives the machine through the STIC's own seams (`stic_bounds.c` steps through `ivs_machine_step_unit_trace_internal`), so a guest's observable can be read without a pixel.
- `tools/asm` (`ivs_asm`) and `fixtures/` — the project's own assembler and the fixture convention for authored sources.
- `data/expected/ivsn-literals.ids` — the snapshot literals and the parameter digest that any profile-field change must retarget.
- Phase 18's `ivs-inspect` — the instrument that makes a cycle-stamped trace available; the four guests' observables are read from the report, and the trace is available when a guest's timing needs explaining.

### Established Patterns
- A profile field carries a tag and a cite; `unknown-default-chosen` is a tag, not a placeholder.
- A parameter row is built explicitly per cell; nothing edits a global default to sweep.
- A new check ships a must-fail registry row, a liveness guard and a committed stand-in in one commit.
- `skip-ok` in both inventories is how a probe that needs operator content stays out of CI without becoming a silent skip.

### Integration Points
- `tests/CMakeLists.txt` registers the `stic` suite; the four guests reach it the same way the existing STIC programs do.
- `data/expected/discrepancies.ids` and the `C82` row are where the appended observation lands, in both exits.
- `refs/` is the one path the tool refuses to write outside (Phase 18's output ladder), which is what contains the owner-local sweep.

</code_context>

<specifics>
## Specific Ideas

The discuss boilerplate of `GUARDRAILS-v0.4` §5, answered in writing before planning:

1. **W rows and rights rows touched.** **W1** (a display-path rule: the profile field with tag and cite, `profile-fields.ids`, the parameter digest and the id holding it, **every** snapshot literal, a memmap literal if a map row moves, the moved `stic` inventory ids, the C-row `evidence` line and `discrepancies.ids`, the new pin-both-settings ids, a `PROJECT.md` decision row, an appended note in `docs/stic/reference-notes.md`; the field stays `unknown-default-chosen`, **zero claims move**) and **W2** (a test id, control or registry row: the pending line deleted, the ten toy registries extended, the orphan site failing; zero new faults, zero new goldens). **No `data/rights/` row is added**: the guests are project-authored, the sweep's output is refused outside `refs/`, and nothing third-party is read.
2. **Rival rule pair separated.** All four, diagonally: window length vs gating-when-off vs read-vs-write effect vs latch-per-frame. The trace line that differs is the guest's own observable in `ivs-inspect trace` (the access record's tick and kind at `$0020`, and the frame in which the display latch is set).
3. **Count delta and its price.** `steps` **38**, `faults` **238**, `goldens` **29**, `claims` **29**, `provisional` **28**, `prefix-files` **11**; `controls` unchanged unless the diagonal mutant needs a directory (then +1, stated); `tests-expected` grows by the new `stic` ids across both presets and by the two pinning ids if Exit A is taken (the plan states the exact number before execution); `gates`/`lines` move only by the registry rows added. Priced **1.64 hosted cpu-s** against the 21 allowance; the same cases as faults would cost 1.76 and move `faults`, which is why they are ids.
4. **Oracle, freeze commit, liveness guard, stand-in.** Oracle: the four guests' observables, each read from the runner's report, with the 16-cell matrix as the discriminating apparatus. Liveness guards: each guest's observable must be able to move (a stand-in that freezes it fails), and the diagonal check must be able to fail (a committed `mutant` row). Stand-ins: one per guest, plus the diagonal mutant.
5. **Rights posture in one line.** Everything here is project-authored: four sources, four scenarios, sixteen parameter rows. The owner-local sweep reads operator content only under `refs/`, is bounded, and is never an exit criterion; no claim, tier or golden records an operator file's digest, size, path or name.
6. **What is deferred, and where.** The D10-VID-08 hardware probe stays a handoff (`evidence: none`), recorded in Phase 20's packet; if nothing discriminates, the milestone exits "default unchanged, observation appended, rivals named" and the packet becomes the resolver. Both are recorded in the phase record and the C-row.
7. **Evidence tier.** **None.** The field stays `unknown-default-chosen` even when a default is decided (D-51: an observation is not a hardware probe); `claims=29` and `provisional=28` are invariants and nothing reaches `measured-hardware`.

No "I want it like X" reference was raised: the criterion names every guest, observable, field, exit and count.

</specifics>

<deferred>
## Deferred Ideas

- A fifth rival rule (for example a strobe that latches the *next* frame) — not one of the four the sources leave open; named here so a later reader does not add a guest to the sweep without a new source.
- Any pixel or frame-hash comparison — refused by the criterion and by D-51; the observables are machine state.
- Turning the owner-local sweep into a CI case — refused: its input is operator content (D11), so it stays `skip-ok` and bounded.

</deferred>
