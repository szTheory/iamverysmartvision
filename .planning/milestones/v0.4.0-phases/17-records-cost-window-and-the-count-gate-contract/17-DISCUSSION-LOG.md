# Phase 17: Records, Cost Window and the Count-Gate Contract - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-11-26
**Phase:** 17 - Records, Cost Window and the Count-Gate Contract
**Areas discussed:** Recomputed ledger, Wrap refusal tokens, Wrap-to-shim join, Source-archive letter, Lesson 32 mechanism

---

## Recomputed ledger

| Option | Description | Selected |
|--------|-------------|----------|
| Last five rows at the plan's own start commit, per-column median | `awk` over `.planning/ci-runs.tsv`, `sort -n` + `sed -n 3p` on columns 13/8/10; printed before the first pricing sentence and stamped with that sha | ✓ |
| All 118 rows | A whole-history median, which no criterion asks for and which no checkpoint line in the vocabulary can be charged against | |
| The previous milestone's frozen figures | Quoting 367/34/115, which is exactly the defect EV-08 exists to remove | |

**User's choice:** none — decided autonomously (reversible technical).
**Notes:** Measured at `fb508355b6d4647b5b4fd8ee4cc4ab2d385d59ef`: `cpu_s` 572, `build_s` 49, `gate_s` 164 over rows 584/51/171, 580/51/165, 572/49/164, 530/49/154, 368/35/115. The expected verdict is `reprice`: the window moved far more than the priced addition while the token line did not move and `r` stayed flat inside 10.6 to 11.7 — a runner event, with no lever invented.

---

## Wrap refusal tokens

| Option | Description | Selected |
|--------|-------------|----------|
| New section after "Refusal reasons", ids-file order | Eight tokens in the append-only inventory's order, one row each with trigger and accepted boundary, so `diff` of the two greps is silent | ✓ |
| Append to the existing "Refusal reasons" table | Mixes the parser's 18 numbered reasons with the tool's 8 tokens; the criterion's `diff` would then compare unlike rows | |
| A separate `docs/formats/flatwrap-refusals.md` | A second document for eight lines, and the criterion names `docs/formats/ivsc-v1.md` | |

**User's choice:** none — decided autonomously (reversible technical: prose only).
**Notes:** `data/expected/flat-refusals.ids` is not edited. The ids file's own header already explains why `flatwrap-empty` is last in the append-only list, and that order is what the section reproduces.

---

## Wrap-to-shim join

| Option | Description | Selected |
|--------|-------------|----------|
| Join through the existing digest pair, with a flipped input byte | Harness `digest.images` for the tool's container against that container's runner-side `slot.cart.sha256`, which the existing roundtrip id already ties to the tool's printed digest; one input byte flipped requires both limbs to move | ✓ |
| A new shim accessor exposing the slot digest | A new core/host function — refused by D-52 (the core gains no sink, the shim gains no surface for a test) | |
| A golden per container | Forbidden: `goldens` stays 29, and a golden per trace class is on the milestone's "what would blow the allowance" list | |

**User's choice:** none — decided autonomously (reversible technical: one id and its inventory line).
**Notes:** Cost is the phase's whole line: one process-launch id, 0.10 hosted cpu-s. The case reuses `run_one`, `check_host` and the suite's `mktemp` layout; nothing is written into the tree and `exec: -` is retained so no line can read as a real-EXEC result.

---

## Source-archive letter

| Option | Description | Selected |
|--------|-------------|----------|
| Correct inline in both documents, amend (i) in place with the old sentence quoted | `docs/packaging.md` lines 39 and 123 corrected where a reader meets them; `RELEASE-OPS.md` (i) carries the corrected line with the superseded v0.3.0 sentence dated beneath it | ✓ |
| Append a new section (k) and leave (i) alone | The criterion binds packaging to the (i) line, so a reader following the pointer would land on the wrong sentence | |
| Rewrite the v0.3.0 release record | Forbidden: archives are append-only, and the record is the evidence, not the letter | |

**User's choice:** none — decided autonomously (reversible technical: prose in two live documents).
**Notes:** `GUARDRAILS-v0.4` §2.12 is the shape: a live document gets an inline pointer at the wrong sentence *and* an amendment; an archived record is never rewritten. The inspection-tool clause of the v0.4.0 line is not written here — the tool does not exist until Phase 18 and Phase 20 recomputes the line.

---

## Lesson 32 mechanism

| Option | Description | Selected |
|--------|-------------|----------|
| Cite the registered `d09-06-registration mutant st-pass-line EV-07` row and re-demonstrate its committed stand-in | `st-pass-line` is an in-step case of the `runner-selftest` step that runs a copied runner whose output continues past its pass line and requires the refusal; the copied runner is the committed stand-in | ✓ |
| A new in-step selftest case | 0.05 cpu-s on top of the id's 0.10 puts the phase 50 percent over its line, which must be repriced and recorded — taken only if a clause has no existing selftest | |
| Register the clauses as faults | Four faults would cost about 1.76 cpu-s and move `faults=238`, which the milestone forbids | |

**User's choice:** none — decided autonomously (reversible technical: one registry row plus a lessons paragraph).
**Notes:** The three clauses are: every count gate names the file it counts; every baseline names the plan's own start commit sha and is measured there; no verify block decides an outcome by a pipeline tail. Plan 13-04's narrowing of EV-07 is the precedent for refusing a second registration of one proof.

---

## Decided autonomously

[auto] [Recomputed ledger] — Q: "which rows and which statistic define this phase's window?" → Selected: "the last five data rows of the named file at the plan's own start commit, per-column median by sort -n plus sed -n 3p" (reversible technical — decided autonomously: a summary paragraph, no artifact and no digest move, so undoing it re-runs an awk one-liner)
[auto] [Wrap refusal tokens] — Q: "where does the token section go and in what order?" → Selected: "a new 'Wrap tool refusals' section directly after 'Refusal reasons', the eight tokens in the ids-file order, one row per token with its trigger and its accepted boundary" (reversible technical — decided autonomously: prose only, and the ids file is untouched)
[auto] [Wrap-to-shim join] — Q: "how does one id prove the shim consumed what the tool wrote?" → Selected: "join the harness's digest.images for the tool's container to that container's runner-side slot.cart.sha256, with one flipped input byte requiring both to move" (reversible technical — decided autonomously: one id and its inventory line, revertible with the same two-line edit)
[auto] [Source-archive letter] — Q: "correct in place, or append a new lettered section?" → Selected: "correct both wrong sentences inline and amend section (i) in place, quoting the superseded sentence beneath it" (reversible technical — decided autonomously: two live documents, no archive rewritten, no count or golden depends on the wording)
[auto] [Lesson 32 mechanism] — Q: "which existing selftest carries the contract?" → Selected: "cite the registered d09-06-registration mutant st-pass-line EV-07 row and re-demonstrate its committed copied-runner stand-in; add at most one more mutant row over an existing selftest family, no new case" (reversible technical — decided autonomously: one registry row and a lessons paragraph, both revertible without touching behaviour)

## Claude's Discretion

- The mechanical shape of the `flatwrap-shim` case (harness arguments, temp layout), bounded by: one id, both digest limbs asserted, one flipped input byte required to move them, nothing written into the tree, `exec: -` retained.
- The exact wording of lesson 32 and of the corrected sentences, bounded by the three clauses and by sentence-for-sentence agreement between `docs/packaging.md` and `RELEASE-OPS.md` (i).

## Deferred Ideas

- The inspection-tool clause of the release-decision line: Phase 18 builds the tool; Phase 20 recomputes the line with it.
- Whether `ci-summary`'s `summary.line` read (`sed -n '$p'`) is itself a forbidden pipeline tail: not this phase's question — the helper reads a defined one-line file and lesson 32 binds verify blocks — recorded so a later reader does not re-open it as a defect.
- Any second mechanism for the count-gate clauses: a second registration of one proof, refused by the plan 13-04 precedent and named rather than skipped.
