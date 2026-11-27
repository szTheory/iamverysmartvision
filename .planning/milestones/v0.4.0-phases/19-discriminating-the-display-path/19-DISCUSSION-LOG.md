# Phase 19: Discriminating the Display Path - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-11-27
**Phase:** 19 - Discriminating the Display Path
**Areas discussed:** The four guests, The 16-cell sweep and the latch row, The two exits, The owner-local sweep

---

## The four guests

| Option | Description | Selected |
|--------|-------------|----------|
| One project-owned `ivs_asm` source plus its scenario per rival, one machine-state observable read from the report, one liveness guard with a committed stand-in | The observable is state, not a picture; the stand-in proves the guard is not vacuous | ✓ |
| One guest per field, judged by a frame hash | A hash of a frame is not an observable the criterion names, and it collapses the four rules into one number | |
| Reuse the existing `stic.enable.seq.*` ids | They exercise today's default; none of them separates the rivals, which is the whole point | |

**User's choice:** none — decided autonomously (reversible technical: four fixture sources and four scenarios).
**Notes:** G1 a write inside the 2000-to-2900 gap; G2 a write outside both window candidates with the latch clear (`stic.blocked_writes`); G3 a read with no write; G4 one write never re-strobed (frames 0, 1, 2).

---

## The 16-cell sweep and the latch row

| Option | Description | Selected |
|--------|-------------|----------|
| Sixteen explicitly built parameter rows, with the latch rule added to `profile-fields.ids` and the whole `W1` chain in one commit before the sweep | Each cell is a row; the profile change is enumerated once, before any result is recorded | ✓ |
| Sweep by editing the global default per cell | A global edit is not a parameter row, and the cells would not be reproducible from the tree | |
| Sweep only the four fields one at a time (4 cells) | That is not the 16-cell matrix the criterion names, and it cannot show diagonality | |

**User's choice:** none — decided autonomously (reversible technical: parameter rows are data).
**Notes:** The latch row is the fourth field and does not exist yet; it drags the parameter digest and every snapshot literal, so the row and that chain land together.

---

## The two exits

| Option | Description | Selected |
|--------|-------------|----------|
| Both commit shapes written before the sweep runs; the measurement decides which lands | Neither exit can be improvised to flatter the result | ✓ |
| Decide the exit after seeing the sweep | That is fit, not discrimination — the exact failure D-54 forbids | |
| Always record Exit B (never change the default) | Safe, but it forecloses a discrimination that may well succeed, and the milestone's point is to separate the rules | |

**User's choice:** none — decided autonomously (reversible technical: both shapes are prose plus an enumeration).
**Notes:** Exit A's commit must equal the enumeration written beforehand; a mismatch is a stop. Exit B is `Complete` on VID-07's own terms.

---

## The owner-local sweep

| Option | Description | Selected |
|--------|-------------|----------|
| A bounded throwaway probe under `refs/`, `skip-ok` in both inventories, nobody's exit criterion | Its input is operator content (D11); it must never hold the phase hostage | ✓ |
| A CI case with operator content | Impossible without the operator's files, and D11 keeps them out of CI | |
| Drop it entirely | It is the one probe that could contradict the instrument; dropping it removes a falsifier, not a criterion | |

**User's choice:** none — decided autonomously (reversible technical: a `skip-ok` probe).
**Notes:** Bounded by wall clock and run count; output refused outside `refs/` on the `tools/flatwrap` precedent.

---

## Decided autonomously

[auto] [Guests] — Q: "authored how, and with what observable?" → Selected: "one project-owned ivs_asm source plus its scenario per rival, one machine-state observable read from the report, one liveness guard with a committed stand-in" (reversible technical — decided autonomously: fixture sources and scenarios; no binary, no third-party content)
[auto] [Sweep] — Q: "how are 16 cells driven?" → Selected: "sixteen explicitly built parameter rows through the existing stic ids, never a global edit" (reversible technical — decided autonomously: a cell is a row of data)
[auto] [Latch row] — Q: "when does the latch rule become a profile field?" → Selected: "before the sweep, in one commit with the whole W1 chain" (reversible technical — decided autonomously: the chain is enumerated in the same commit)
[auto] [Exit choice] — Q: "who decides A or B?" → Selected: "the measurement decides; both commit shapes are written before the sweep runs" (reversible technical — decided autonomously: a plan that pre-writes both shapes cannot improvise)
[auto] [Sweep containment] — Q: "is the owner-local sweep an exit criterion?" → Selected: "no — skip-ok, bounded, refs/-only, and its absence leaves criteria 1 to 4 standing" (reversible technical — decided autonomously: a probe nothing cites cannot hold the phase hostage)

## Claude's Discretion

- The guest source shape (address layout, cycle offsets within the stated gaps), the scenario file names, and the module split of the sweep driver.
- How the 16 cells are enumerated in the suite (one id per cell versus one id that loops the matrix), bounded by the diagonal assertion and the cost line.

## Deferred Ideas

- A fifth rival rule (a strobe that latches the next frame): not one of the four the sources leave open.
- Any pixel or frame-hash comparison: refused by the criterion and by D-51.
- Turning the owner-local sweep into a CI case: refused — its input is operator content (D11).
