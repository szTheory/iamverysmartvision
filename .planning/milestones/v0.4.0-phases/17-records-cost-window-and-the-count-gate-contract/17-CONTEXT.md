# Phase 17: Records, Cost Window and the Count-Gate Contract - Context

**Gathered:** 2026-11-26
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 17 retires v0.3.0's records debt and puts the milestone's arithmetic on a
measured footing before anything is priced. It delivers five things and nothing
else: the cost window **recomputed by arithmetic** from
`.planning/ci-runs.tsv` at the plan's own start commit, printed before the first
pricing sentence with exactly one checkpoint token; the eight `ivs_flatwrap`
refusal tokens **documented** under `docs/formats/ivsc-v1.md`; **one** committed
test id guarding the flatwrap-to-shim join, shown able to fail; the
"no host shim ships" letter **corrected** to name the source archive; and the
count-gate contract recorded as `PLAN-LESSONS` lesson 32 **with a mechanism**.

Out of scope by construction: no new gate step (38 stays 38), no new fault (238),
no new golden (29), no new installed file (11), no claim (`claims=29`,
`provisional=28`), and no touch on the display path, the inspection surface or
any rights-bearing bytes. This phase moves documents, records and one test id.

</domain>

<decisions>
## Implementation Decisions

### The recomputed ledger
- The phase record prints the last-five `cpu_s`, `build_s` and `gate_s` medians **computed by `awk` over `.planning/ci-runs.tsv` at the plan's own start commit**, before its first pricing sentence, and then exactly one checkpoint line from the `GUARDRAILS-v0.4` §2.4 vocabulary.
- The seven release-records rows are **verified, never re-appended**: 118 data rows over 119 lines, and each of `45835631107`, `45836207855`, `45839153455`, `45839646456`, `45841732895`, `45842795129` and `45843408874` appears in exactly one row. A missing row is a stop for the operator, not a re-append by the executor.
- The measured window at start commit `fb508355b6d4647b5b4fd8ee4cc4ab2d385d59ef` is `cpu_s` **572**, `build_s` **49**, `gate_s` **164** (last five `cpu_s`/`build_s`/`gate_s` rows 584/51/171, 580/51/165, 572/49/164, 530/49/154, 368/35/115), against the previous 367/34/115. **Recomputed, never quoted:** `r` (= `cpu_s`/`build_s`) over those five rows is 11.4510, 11.3725, 11.6735, 10.8163, 10.5143 — **10.51 to 11.67**, not the ROADMAP's rounded "10.6 to 11.7" — and the seven release rows' `build_s` spans **34 to 51**, not the ROADMAP's "47 to 51". The roadmap's two prose figures are rounded; the record prints the computed values and their recipe. The gate's token line did not move, so the expected verdict is **`reprice`** — a runner event, and no lever is invented to explain it.
- The **statistic** is a per-column median: last five data rows only, `sort -n` then the third value (`sed -n 3p`), one column at a time (13 = `cpu_s`, 8 = `build_s`, 10 = `gate_s`). Every printed number is reproducible by that one recipe, and the recipe is stated in the record beside the numbers.

### The eight wrap refusal tokens
- `docs/formats/ivsc-v1.md` gains a **"Wrap tool refusals"** section immediately after "Refusal reasons", because a reader meeting the parser's 18 numbered reasons next needs the tool's separate token set and must not confuse the two.
- The section lists **exactly** the eight tokens in `data/expected/flat-refusals.ids` order — `flatwrap-length`, `flatwrap-word-range`, `flatwrap-words`, `flatwrap-base`, `flatwrap-cross`, `flatwrap-output-tree`, `flatwrap-output-exists`, `flatwrap-empty` — so that `diff` of the two greps is silent. Order is part of the criterion, not a style choice.
- Each row carries the token, the input or output condition that triggers it, and the boundary case that must still be accepted, so the section is a contract and not a glossary.
- **`data/expected/flat-refusals.ids` is not edited.** The inventory is append-only and already correct; this phase documents it.

### The wrap-to-shim join
- One committed id, `runner-process.judge.flatwrap-shim`, inside the existing `tests/runner-process-tap.sh` suite — no new suite, no new step.
- The id asserts that the shim **accepted and digested the container `tools/flatwrap` wrote in the same case**, joined through digests that already exist: the harness's `digest.images` for a container built by the tool, against the same container's runner-side `slot.cart.sha256`, which `runner-process.judge.flatwrap-roundtrip` already ties to the digest the tool printed.
- Non-vacuity is proved in the same case by **one flipped input byte**: re-wrap, re-run, and require both digest limbs to move. A stand-in that ignores its input therefore fails the id rather than passing it.
- The container, the flat input and both reports live under the suite's `mktemp` directory; nothing is written into the tree. The `exec:` label stays `-`, so no line can read as a real-EXEC result.
- Cost: this is the phase's single process-launch id, **0.10 hosted cpu-s**, which is the whole of the phase's priced line.

### The source-archive letter
- `docs/packaging.md` carries the wrong claim twice — the paragraph at line 39 ("never installed and never shipped") and the bullet at line 123 ("no shared object is in a release ... and no host shim"). Both are corrected **inline**, in place, to say what is true: the eleven-path static package ships no object, and the **source archive** carries the source of the build-only tools and of the build-only host shim, and ships no built object.
- `RELEASE-OPS.md` section (i) is amended **in place** to carry the corrected release-decision line, with the superseded v0.3.0 sentence quoted beneath it and dated. `GUARDRAILS-v0.4` §2.12 requires that a live document a person reads shows the wrong sentence as well as the amendment; it does not permit rewriting an archived record, and no archived record is touched.
- The packaging wording and the `RELEASE-OPS` (i) line must agree **sentence for sentence**. The inspection tool clause of the v0.4.0 line (`GUARDRAILS-v0.4` §7.4) is **not** written here: the tool does not exist until Phase 18 and Phase 20 recomputes the line. Phase 17's line ends at "no built host shim".

### Lesson 32 and its mechanism
- `.planning/ops/PLAN-LESSONS.md` gains lesson **32** with the three clauses verbatim in substance: every count gate names the file it counts; every baseline names the plan's own start commit sha and is measured there; no verify block decides an outcome by a pipeline tail.
- The mechanism is a **`mutant` row in `data/expected/important-gates.ids` over an existing selftest, shown able to fail on a committed stand-in**. The row that already discharges the tail clause is `d09-06-registration mutant st-pass-line EV-07`: `st-pass-line` is an in-step case of the `runner-selftest` step that runs a **copied runner** (the committed stand-in) whose output continues past its pass line and requires the refusal. Plan 17-03 cites that row and re-demonstrates the failure from the committed stand-in rather than inventing a case.
- If, and only if, clause 1 (the file a count gate counts) or clause 2 (the baseline's start commit sha) has **no** existing selftest that can be made to fail, plan 17-03 adds **one** `mutant` row over an existing in-step selftest family (`runner-selftest.ids`, `ci-summary-selftests.ids`, `pkg-selftests.ids`) and no new case. Adding a *new* case costs 0.05 cpu-s and would put the phase at 0.15 against a 0.10 line — a 50 percent overrun that must be **repriced and recorded**, never hidden.
- The lesson's numbers (`faults=238`, `goldens=29`, `claims=29`, `provisional=28`) are restated in the lesson as invariants the reader can recompute, not as assertions.

### Decided autonomously
- [Recomputed ledger] Q: "which rows and which statistic define this phase's window?" → Selected: "the last five data rows of the named file at the plan's own start commit, per-column median by sort -n plus sed -n 3p" — a summary paragraph, no artifact and no digest move, so undoing it re-runs an awk one-liner.
- [Wrap refusal tokens] Q: "where does the token section go and in what order?" → Selected: "a new 'Wrap tool refusals' section directly after 'Refusal reasons', the eight tokens in the ids-file order, one row per token with its trigger and its accepted boundary" — prose only, and the ids file is untouched, so a reordering is a text edit with no dependent expectation.
- [Wrap-to-shim join] Q: "how does one id prove the shim consumed what the tool wrote?" → Selected: "join the harness's digest.images for the tool's container to that container's runner-side slot.cart.sha256, with one flipped input byte requiring both to move" — one id and its inventory line; reverting is the same two-line edit.
- [Source-archive letter] Q: "correct in place, or append a new lettered section?" → Selected: "correct both wrong sentences inline and amend section (i) in place, quoting the superseded sentence beneath it" — two live documents, no archive rewritten, and no count, digest or golden depends on the wording.
- [Lesson 32 mechanism] Q: "which existing selftest carries the contract?" → Selected: "cite the registered d09-06-registration mutant st-pass-line EV-07 row and re-demonstrate its committed copied-runner stand-in; add at most one more mutant row over an existing selftest family, no new case" — one registry row plus a lessons paragraph, both revertible without touching behaviour.

### Claude's Discretion
- The mechanical shape of the `flatwrap-shim` case — which harness arguments and which temp layout — is the planner's, bounded by: one id, both digest limbs asserted, one flipped input byte required to move them, nothing written into the tree, `exec: -` retained.
- The exact wording of lesson 32 and of the corrected sentences is the planner's, bounded by the three clauses and by sentence-for-sentence agreement between `docs/packaging.md` and `RELEASE-OPS.md` (i).

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- `data/expected/flat-refusals.ids` — the append-only eight-token inventory with its own header explaining why `flatwrap-empty` is last; unchanged, cited as the source of the documented order.
- `data/expected/important-gates.ids` — the registry (620 lines) whose `mutant` kind resolves through `pkg-selftests.ids`, `ci-summary-selftests.ids`, `runner-selftest.ids` or a `<suite>.judge.<slug>` id; `d09-06-registration mutant st-pass-line EV-07` is already present at the EV-07 block.
- `data/expected/runner-selftest.ids` — four cases, `st-pass-line` among them; `tools/check/steps/runner-selftest.sh` builds its copied-runner stand-in and refuses a registry row that names a case the step does not run.
- `tests/runner-process-tap.sh` — already carries `runner-process.judge.flatwrap-roundtrip` (the tool's printed digest versus the runner's `slot.cart.sha256`, with a flipped input byte), `check_host`, `self_control`, `run_one` and the eight-argument usage line; the new case reuses all of them.
- `tests/host/ivs_host_harness.c` — the fake frontend that `dlopen`s the build-only shim, reads `lamp-tone.ivsc` from its content dir, loads the three images and prints nine `digest.<domain>` lines including `digest.images`.
- `tools/check/steps/ci-log.sh` — the count gate that already names the file it counts (`ci-runs=<rows>`) and enforces the record's own shape, header and duplicate rules.

### Established Patterns
- A count is stated as a token (`steps=38/38`, `tests-expected=<ids x presets>`, `faults=238/238`) and a plan predicts the **whole** summary line before execution; an unexpected token is a stop, never a re-bless.
- A new mechanism ships a must-fail registration plus a liveness guard plus a stand-in; the stand-in for an in-step selftest is a mutated copy built inside the committed step.
- Records are append-only for archives, inline-plus-amendment for live documents; a disposition lives in a live record, never in a rewrite.
- Suite ids live in `data/expected/tests/<suite>.ids` and are counted across **both** presets: one new id is `tests-expected` **+2**.

### Integration Points
- `tests/CMakeLists.txt` already passes the shim and harness paths as the seventh and eighth `runner-process` arguments; no build-graph change is needed for the new id.
- `data/expected/tests/runner-process.ids` gains exactly one line; `data/expected/controls/*` and `data/expected/gate-steps/*` are untouched.
- `.planning/ops/RELEASE-OPS.md` section (i) and `docs/packaging.md` are the two documents criterion 4 binds together.

</code_context>

<specifics>
## Specific Ideas

The discuss boilerplate of `GUARDRAILS-v0.4` §5, answered in writing before planning:

1. **W rows and rights rows touched.** `GUARDRAILS-v0.4` **W8** (a count gate: the plan names the file it counts and the plan's own start commit sha in one sentence; the mechanism is a `mutant` row plus lesson 32), **W9** (a refusal token or a format sentence: the ids file stays append-only, one `docs/formats/ivsc-v1.md` section, one new id guarding the flatwrap-to-shim join) and **W10** (release text: `docs/packaging.md`, `.planning/ops/RELEASE-OPS.md`, with `pkg-install-files.ids` and `release-assets` unchanged and re-checked). **No `data/rights/` row is touched**: no third-party bytes, no trace, no disassembly, no operator content.
2. **Rival rule pair separated.** None. This phase separates no display-path rules and its trace line is empty; the four rivals and their differing trace lines belong to Phase 19.
3. **Count delta and its price.** `tests-expected` **+2** (one `runner-process` id across the dev and release presets), `controls` +0, `faults` 238/238, `goldens` 29, `claims` 29, `provisional` 28, `steps` 38, install list 11, `ci-runs` 118 rows (unchanged by this phase; it grows only when the phase's own hosted run is recorded). Priced **0.10 hosted cpu-s** against the 21 allowance: one process-launch id at 0.10. The window is recomputed from `.planning/ci-runs.tsv` at the plan's start commit and printed before this sentence's first number is used.
4. **Oracle, freeze commit, liveness guard, stand-in.** The oracle is the eight-token inventory (`data/expected/flat-refusals.ids`, unchanged) and the recorded run rows (`.planning/ci-runs.tsv`, append-only); neither is frozen by this phase because neither moves. The liveness guard for the new id is the flipped input byte; its stand-in is the same one-byte flip, which must move both digest limbs. The liveness guard for lesson 32's mechanism is the committed copied runner inside `st-pass-line`.
5. **Rights posture in one line.** No rights-bearing material is read, named or produced: the phase authors documentation about project-owned tools and records, so no `data/rights/` row is added or amended and nothing is refused-by-name newly.
6. **What is deferred, and where.** The inspection-limb clause of the release-decision line is deferred to Phase 20 (`GUARDRAILS-v0.4` §7.4), recorded here and in `RELEASE-OPS.md` (i); its reversal condition is Phase 18 shipping the build-only tool. Phase 19's four rivals, and any decision about them, are deferred whole to Phase 19.
7. **Evidence tier.** **None.** This phase mints no claim and moves none: `claims=29` and `provisional=28` are invariants, nothing reaches `measured-hardware`, and `CL-0002` remains the only `established` claim.

No specific "I want it like X" reference was raised: the frozen criteria and `GUARDRAILS-v0.4` already name every artifact, token, id and count this phase produces.

</specifics>

<deferred>
## Deferred Ideas

- The inspection-tool clause of the release-decision line — Phase 18 builds the tool, Phase 20 recomputes the line with it.
- Whether `ci-summary`'s `summary.line` read (`sed -n '$p'`) is itself a forbidden pipeline tail — not this phase's question: the helper reads a defined one-line file, and lesson 32 binds verify blocks, not a helper's contract. Recorded so a later reader does not re-open it as a defect.
- Any mechanism for the count-gate clauses beyond the one `mutant` row — a second mechanism would be a second registration of one proof (the precedent is plan 13-04's narrowing of EV-07), so it is named and refused rather than silently skipped.

</deferred>
