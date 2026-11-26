# Phase 17: Records, Cost Window and the Count-Gate Contract - Research

**Researched:** 2026-11-26
**Domain:** Records recomputation, format documentation, one judge test id, release-letter correction, planner contract
**Confidence:** HIGH (every number below was recomputed or executed in this session; every file:line was opened)

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

#### The recomputed ledger
- The phase record prints the last-five `cpu_s`, `build_s` and `gate_s` medians **computed by `awk` over `.planning/ci-runs.tsv` at the plan's own start commit**, before its first pricing sentence, and then exactly one checkpoint line from the `GUARDRAILS-v0.4` §2.4 vocabulary.
- The seven release-records rows are **verified, never re-appended**: 118 data rows over 119 lines, and each of `45835631107`, `45836207855`, `45839153455`, `45839646456`, `45841732895`, `45842795129` and `45843408874` appears in exactly one row. A missing row is a stop for the operator, not a re-append by the executor.
- The measured window at start commit `fb508355b6d4647b5b4fd8ee4cc4ab2d385d59ef` is `cpu_s` **572**, `build_s` **49**, `gate_s` **164** (last five `cpu_s`/`build_s`/`gate_s` rows 584/51/171, 580/51/165, 572/49/164, 530/49/154, 368/35/115), against the previous 367/34/115. `r` is flat inside 10.6 to 11.7 and the gate's token line did not move, so the expected verdict is **`reprice`** - a runner event, and no lever is invented to explain it.
- The **statistic** is a per-column median: last five data rows only, `sort -n` then the third value (`sed -n 3p`), one column at a time (13 = `cpu_s`, 8 = `build_s`, 10 = `gate_s`). Every printed number is reproducible by that one recipe, and the recipe is stated in the record beside the numbers.

#### The eight wrap refusal tokens
- `docs/formats/ivsc-v1.md` gains a **"Wrap tool refusals"** section immediately after "Refusal reasons", because a reader meeting the parser's 18 numbered reasons next needs the tool's separate token set and must not confuse the two.
- The section lists **exactly** the eight tokens in `data/expected/flat-refusals.ids` order - `flatwrap-length`, `flatwrap-word-range`, `flatwrap-words`, `flatwrap-base`, `flatwrap-cross`, `flatwrap-output-tree`, `flatwrap-output-exists`, `flatwrap-empty` - so that `diff` of the two greps is silent. Order is part of the criterion, not a style choice.
- Each row carries the token, the input or output condition that triggers it, and the boundary case that must still be accepted, so the section is a contract and not a glossary.
- **`data/expected/flat-refusals.ids` is not edited.** The inventory is append-only and already correct; this phase documents it.

#### The wrap-to-shim join
- One committed id, `runner-process.judge.flatwrap-shim`, inside the existing `tests/runner-process-tap.sh` suite - no new suite, no new step.
- The id asserts that the shim **accepted and digested the container `tools/flatwrap` wrote in the same case**, joined through digests that already exist: the harness's `digest.images` for a container built by the tool, against the same container's runner-side `slot.cart.sha256`, which `runner-process.judge.flatwrap-roundtrip` already ties to the digest the tool printed.
- Non-vacuity is proved in the same case by **one flipped input byte**: re-wrap, re-run, and require both digest limbs to move. A stand-in that ignores its input therefore fails the id rather than passing it.
- The container, the flat input and both reports live under the suite's `mktemp` directory; nothing is written into the tree. The `exec:` label stays `-`, so no line can read as a real-EXEC result.
- Cost: this is the phase's single process-launch id, **0.10 hosted cpu-s**, which is the whole of the phase's priced line.

#### The source-archive letter
- `docs/packaging.md` carries the wrong claim twice - the paragraph at line 39 ("never installed and never shipped") and the bullet at line 123 ("no shared object is in a release ... and no host shim"). Both are corrected **inline**, in place, to say what is true: the eleven-path static package ships no object, and the **source archive** carries the source of the build-only tools and of the build-only host shim, and ships no built object.
- `RELEASE-OPS.md` section (i) is amended **in place** to carry the corrected release-decision line, with the superseded v0.3.0 sentence quoted beneath it and dated. `GUARDRAILS-v0.4` §2.12 requires that a live document a person reads shows the wrong sentence as well as the amendment; it does not permit rewriting an archived record, and no archived record is touched.
- The packaging wording and the `RELEASE-OPS` (i) line must agree **sentence for sentence**. The inspection tool clause of the v0.4.0 line (`GUARDRAILS-v0.4` §7.4) is **not** written here: the tool does not exist until Phase 18 and Phase 20 recomputes the line. Phase 17's line ends at "no built host shim".

#### Lesson 32 and its mechanism
- `.planning/ops/PLAN-LESSONS.md` gains lesson **32** with the three clauses verbatim in substance: every count gate names the file it counts; every baseline names the plan's own start commit sha and is measured there; no verify block decides an outcome by a pipeline tail.
- The mechanism is a **`mutant` row in `data/expected/important-gates.ids` over an existing selftest, shown able to fail on a committed stand-in**. The row that already discharges the tail clause is `d09-06-registration mutant st-pass-line EV-07`: `st-pass-line` is an in-step case of the `runner-selftest` step that runs a **copied runner** (the committed stand-in) whose output continues past its pass line and requires the refusal. Plan 17-03 cites that row and re-demonstrates the failure from the committed stand-in rather than inventing a case.
- If, and only if, clause 1 (the file a count gate counts) or clause 2 (the baseline's start commit sha) has **no** existing selftest that can be made to fail, plan 17-03 adds **one** `mutant` row over an existing in-step selftest family (`runner-selftest.ids`, `ci-summary-selftests.ids`, `pkg-selftests.ids`) and no new case. Adding a *new* case costs 0.05 cpu-s and would put the phase at 0.15 against a 0.10 line - a 50 percent overrun that must be **repriced and recorded**, never hidden.
- The lesson's numbers (`faults=238`, `goldens=29`, `claims=29`, `provisional=28`) are restated in the lesson as invariants the reader can recompute, not as assertions.

#### Decided autonomously
- [Recomputed ledger] Q: "which rows and which statistic define this phase's window?" → Selected: "the last five data rows of the named file at the plan's own start commit, per-column median by sort -n plus sed -n 3p" - a summary paragraph, no artifact and no digest move, so undoing it re-runs an awk one-liner.
- [Wrap refusal tokens] Q: "where does the token section go and in what order?" → Selected: "a new 'Wrap tool refusals' section directly after 'Refusal reasons', the eight tokens in the ids-file order, one row per token with its trigger and its accepted boundary" - prose only, and the ids file is untouched, so a reordering is a text edit with no dependent expectation.
- [Wrap-to-shim join] Q: "how does one id prove the shim consumed what the tool wrote?" → Selected: "join the harness's digest.images for the tool's container to that container's runner-side slot.cart.sha256, with one flipped input byte requiring both to move" - one id and its inventory line; reverting is the same two-line edit.
- [Source-archive letter] Q: "correct in place, or append a new lettered section?" → Selected: "correct both wrong sentences inline and amend section (i) in place, quoting the superseded sentence beneath it" - two live documents, no archive rewritten, and no count, digest or golden depends on the wording.
- [Lesson 32 mechanism] Q: "which existing selftest carries the contract?" → Selected: "cite the registered d09-06-registration mutant st-pass-line EV-07 row and re-demonstrate its committed copied-runner stand-in; add at most one more mutant row over an existing selftest family, no new case" - one registry row plus a lessons paragraph, both revertible without touching behaviour.

### Claude's Discretion
- The mechanical shape of the `flatwrap-shim` case - which harness arguments and which temp layout - is the planner's, bounded by: one id, both digest limbs asserted, one flipped input byte required to move them, nothing written into the tree, `exec: -` retained.
- The exact wording of lesson 32 and of the corrected sentences is the planner's, bounded by the three clauses and by sentence-for-sentence agreement between `docs/packaging.md` and `RELEASE-OPS.md` (i).

### Deferred Ideas (OUT OF SCOPE)

- The inspection-tool clause of the release-decision line - Phase 18 builds the tool, Phase 20 recomputes the line with it.
- Whether `ci-summary`'s `summary.line` read (`sed -n '$p'`) is itself a forbidden pipeline tail - not this phase's question: the helper reads a defined one-line file, and lesson 32 binds verify blocks, not a helper's contract. Recorded so a later reader does not re-open it as a defect.
- Any mechanism for the count-gate clauses beyond the one `mutant` row - a second mechanism would be a second registration of one proof (the precedent is plan 13-04's narrowing of EV-07), so it is named and refused rather than silently skipped.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| EV-08 | The milestone starts on a measured ledger rather than a quoted one: the hosted cost window is recomputed by arithmetic from `.planning/ci-runs.tsv` at the plan's own start commit and **before** anything is priced; the seven release-records rows are recorded and verified, never re-appended; the eight `flatwrap` refusal tokens are documented under `docs/`; one committed test id guards the `flatwrap`-to-shim join and is shown able to fail; the "no host shim ships" sentence is corrected to name the source archive; and the count-gate contract is a numbered lesson **with a mechanism**. | `## Deliverables, one by one` gives the exact `awk` recipe and its verified output (D1), the doc section's placement and line-start token format (D2), the executable shape of the new id with measured digest values on both presets (D3), the sentence-for-sentence wording constraint (D4), the clause-to-mechanism map for lesson 32 (D5), and the **16-file** EV-08 acceptance flip the `important-gates` step forces (D6). |

## Summary

Phase 17 is a records-and-documents phase whose only executable artefact is one judge id. Every number it prints must be recomputed, and the two things that can actually go wrong are (a) a plan that quotes the window instead of measuring it, and (b) a plan that treats the EV-08 acceptance flip as one table row when the `important-gates` step forces a 16-file change.

Three findings dominate planning:

1. **The window is exactly as CONTEXT states, and the `awk` recipe reproduces it.** `572 / 49 / 164` at HEAD (columns 13, 8, 10; last five data rows; `sort -n` then `sed -n 3p`), against the previous window's `367 / 34 / 115`. Two of CONTEXT's *prose* figures do not survive recomputation and must not be restated: the seven release rows carry `build_s` 34 to 51 (not "47 to 51"), and `r` over the last five rows spans 10.51 to 11.67 (not "10.6 to 11.7"). The verdict `reprice` stands on the median move alone (367 → 572, +205, with an unchanged token line).

2. **The new id is executable today, on both presets, with no golden and no new shim accessor.** `ivs_retro_digest(IVS_DIGEST_IMAGES, ...)` is the shim-side value that moves when the cartridge container bytes change, because the IMAGES domain serialises `slot.cart.sha256` (`core/src/machine/machine_digest.c:52`). Measured: a container `tools/flatwrap` writes from 8192 zero bytes at base `4000` prints `8d73b98c...`; the harness and the runner both print `digest.images f5e8555f...`; one flipped input byte moves the tool's digest to `c41b6dab...` and both `digest.images` lines to `ed1fbaf7...`. Identical on the dev and release presets.

3. **EV-08's acceptance is a 16-file commit, not a table row.** `tools/check/steps/important-gates.sh:207-210` derives the accepted set from the traceability table (`:128-137`) and refuses a row naming a Pending requirement, so the first EV-08 row and the `Pending → Complete` flip must land together; the flip also appends one toy line to each of **ten** toy registries and one row to each of **three** toy requirement records (the exact file list is in D6, taken from commit `298be25`, the PKG-06 precedent).

**Primary recommendation:** Plan three serial plans as ROADMAP states, but land the EV-08 flip in the plan that adds the *first* EV-08 registry row (17-02), not in 17-03, so `GUARDRAILS-v0.4` §2.7 holds in one commit. Build the new id from `dd if=/dev/zero bs=8192` at base `4000` (the roundtrip case's own idiom) rather than from the lamp-tone program: it needs no extraction step, its `digest.images` differs from the pass golden's so an input-ignoring stand-in fails on *both* limbs, and it was measured green on both presets.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Recompute the cost window | Records (`.planning/ci-runs.tsv`) | Phase record prose | The ledger is append-only data; the arithmetic is a one-line `awk` over it and belongs in the record, not in a tool |
| Document the eight refusal tokens | Documentation (`docs/formats/ivsc-v1.md`) | Inventory (`data/expected/flat-refusals.ids`, read-only) | The inventory is the oracle and stays append-only; the doc is the reader-facing contract |
| Guard the wrap-to-shim join | Test suite (`tests/runner-process-tap.sh`) | Registry (`data/expected/important-gates.ids`) | The join is a cross-process digest equality, so it can only live in a suite that already launches both processes |
| Correct the release letter | Documentation (`docs/packaging.md`) | Records (`.planning/ops/RELEASE-OPS.md` (i)) | Both are live documents a person reads; no archive and no gate step reads either wording |
| Record the count-gate contract | Records (`.planning/ops/PLAN-LESSONS.md`) | Registry row over an existing selftest | A prose contract with no parser needs a pointer to the mechanism that proves its one mechanical clause |

## Deliverables, one by one

### D1. The recomputed ledger (criterion 1)

**What the plan must produce:** one prose block in the phase record whose first sentence is the recipe and whose next three figures are the medians, printed **before** any pricing sentence, followed by exactly one line from the §2.4 vocabulary.

**Anchors:** `.planning/ci-runs.tsv` (119 lines, header + 118 rows); columns `cpu_s` 13, `build_s` 8, `gate_s` 10; the start commit is whatever HEAD is when the plan begins (HEAD is `2f460c0964913104236a3b1150538b072b6e9150` and `.planning/ci-runs.tsv` is byte-identical to `fb508355b6d4647b5b4fd8ee4cc4ab2d385d59ef`, so the numbers below hold at both).

**Verified output:** `572 / 49 / 164`; previous window `367 / 34 / 115`; seven rows present once each; 118 data rows over 119 lines. The `awk` recipe, the last-five rows and the two corrected prose figures are in Code Example 1.

**The checkpoint line.** The vocabulary is fixed (`GUARDRAILS-v0.4` §2.4): `discharged-within-allowance`, `reprice`, `lever-required`. The expected verdict is `reprice` because the median `cpu_s` moved 367 → 572 (+205) with an unchanged token line: a runner event, and no lever is invented. The plan must also charge the phase's own priced addition against 21 — see D6's cost note.

**A missing row is a stop.** If any of the seven run ids is absent, or any appears twice, the executor stops and reports; it never appends.

### D2. The eight refusal tokens (criterion 2)

**Placement:** a new `## Wrap tool refusals` section between `## Refusal reasons` (ends at line 110) and `## Digests` (starts at line 111) of `docs/formats/ivsc-v1.md`.

**The mechanical constraint:** the criterion's check is `diff` of `grep '^flatwrap-'` over the inventory and over the doc, so each token must be the **first text on its line** in the doc. A table row or a bullet fails the check. See Pattern 2 and Code Example 4.

**Content:** one row per token in ids order, each carrying the trigger and the boundary that must still be accepted (the table in Code Example 5, sourced from `tests/pack-tap.sh:247-360`). The section must also state three things: the tokens are the **tool's**, not the format's; the section's order is the inventory's append-only order and not the tool's check order (`tools/flatwrap/flatwrap.c:31-42`); and a refusal is one stderr line with exit 1 and no output file, while a file or usage error has no token and exits 2.

**Not edited:** `data/expected/flat-refusals.ids`. It stays append-only, and `tests/pack-tap.sh`'s `pack.judge.flatwrap-tokens` keeps pinning it against the source's own literals.

### D3. The wrap-to-shim id (criterion 3)

**One id:** `runner-process.judge.flatwrap-shim`, added to `tests/runner-process-tap.sh` immediately after `result runner-process.judge.flatwrap-roundtrip` (currently line 1841), and one line to `data/expected/tests/runner-process.ids` after line 241. `ls tools/check/steps/*.sh | wc -l` stays 38; `tests/CMakeLists.txt` is untouched (it already passes the tool, the shim and the harness as arguments 6, 7 and 8).

**The four limbs, all measured green on both presets** (values and commands in Code Example 2):
1. the harness's `digest.images` for the tool's container equals the runner's `digest.images` for the same container;
2. the runner's `slot.cart.sha256` equals the digest the tool printed;
3. after one flipped input byte, both limbs still hold for the second container; and
4. the two containers' values differ, so a stand-in that ignores its input fails.

**Why no golden and no new accessor:** `IVS_DIGEST_IMAGES` serialises `slot.cart.sha256` (`core/src/machine/machine_digest.c:52`, quoted verbatim in Code Example 2), so the IMAGES domain is the shim-side value that moves with the container bytes. `host/libretro_shim.h` offers only `ivs_retro_digest(domain, out32)`, and D-52 forbids adding a slot accessor.

**The committed stand-in.** The precedent is the registered `cart-role mutant runner-process.judge.flatwrap-roundtrip SCN-03` row (`data/expected/important-gates.ids:555`), whose comment says "one flipped input byte moves both; the same-input-twice stand-in shows the equality is live". The new id uses the same shape: the second container **is** the committed stand-in, and limb 4 is what refuses an implementation that ignores it. `GUARDRAILS-v0.4` §2.7's three things (must-fail row, liveness guard, committed stand-in) are therefore discharged by one row plus the in-case flip — exactly as the roundtrip row discharged them.

**The harness's hardcoded names** are why the content dir must hold `boot-stub-2.exec` and a file literally named `lamp-tone.ivsc` (`tests/host/ivs_host_harness.c:430-435`). Extending the harness would be new host surface, which D-52 forbids; the plan documents the name in the case's comment instead.

### D4. The source-archive letter (criterion 4)

**Two wrong sentences, both live:** `docs/packaging.md:39` (the paragraph: "never installed and never shipped" for the host shim) and `docs/packaging.md:123` (the bullet: "No shared object is in a release: the release ships the static package and the source archive, and no host shim.").

**The corrected line, used verbatim in three places** — the packaging paragraph, the packaging bullet, and `RELEASE-OPS.md` section (i) — with Phase 17 ending at "no built host shim" (the inspection-tool clause is Phase 20's, `GUARDRAILS-v0.4` §7.4):

> the release ships the static package and the source archive, which carries the source of the build-only tools and of the build-only host shim; no shared object, no built host shim

**Section (i) keeps its history.** `GUARDRAILS-v0.4` §2.12 forbids rewriting an archived record and requires a live document to show the wrong sentence as well as the amendment, so (i) carries the corrected line, then the superseded v0.3.0 sentence quoted beneath it with its date. Section (i.1) is a dated append by plan 16-04 and is **not** touched, even though its quoted line is now superseded.

**The sentence-for-sentence check** (no gate step reads either document — `grep -rn "packaging.md" tools/ .github/` returns only the rights scanner's page list at `tools/check/rights-lib.cmake:359`): extract the line from `RELEASE-OPS.md` and require exactly two verbatim occurrences in `docs/packaging.md`. Use `grep -c -F -e "$line"`, never a pipeline tail.

**Wording hazards:** `docs/packaging.md` is scanned by the rights page list, so the correction must introduce no refused or owner-local token from `data/rights/*.json`; it is also scanned by `text` (claim words) and `glossary` (the time-base word), so the sentence above deliberately contains none of them.

### D5. Lesson 32 and its mechanism (criterion 5)

**The clause-to-mechanism map.** Lesson 31's own warning ("a rule whose scope is wider than its mechanism teaches readers to quote it") binds lesson 32, so each clause must name what proves it and what does not:

| clause | mechanism | what a plan can cite |
|---|---|---|
| 1. every count gate names the file it counts | a count case over a named file, registered by one `mutant` row | `slowest-eight` (`tools/check/steps/ci-summary.sh:72-82`): 12 entries written to `step.times`, exactly the 8 slowest required in order — a count that stops reading that file, or takes 12 or 7, fails the exact string comparison |
| 2. every baseline names the plan's own start commit sha and is measured there | **none** | no gate step parses plans (`grep -rln "PLAN[.]md\|-PLAN" tools/check/` is empty), so this is a planner obligation; the phase record's own D1 block is the only evidence, and it is reproducible by one recipe |
| 3. no verify block decides an outcome by a pipeline tail | the committed in-step case `st-pass-line` | `data/expected/important-gates.ids:529` (`d09-06-registration mutant st-pass-line EV-07`); `tools/check/steps/runner-selftest.sh:148-188` (a **copied** runner whose step prints a line after its pass line must be refused as `control-pass-line-not-last: profile`), and its gate-side twin `:190-213` (`tools/check/pass-line.sh` refuses a verdict followed by a line with exit 2 and no verdict with exit 1, and accepts the one legal shape) |

**The row.** `count-gate mutant slowest-eight EV-08`, appended to `data/expected/important-gates.ids` with a comment block that says what it proves and that clause 2 has no mechanism. `count-gate` is a new gate name, so `gates` 119 → 120 and `lines` 364 → 366. No case is added, so no 0.05 cpu-s selftest price applies. Clause 3's row already exists and is cited, not duplicated.

**The lesson's text** must carry the three clauses, the map above (including clause 2's honest gap), and the four invariants as recomputable statements (`faults=238`, `goldens=29`, `claims=29`, `provisional=28`). It must not repeat lessons 1-31; the nearest neighbours are 20 (a check that cannot fail proves nothing) and 31 (a rule with no mechanism is quoted, not obeyed), so lesson 32 should open by naming what is *new*: the three clauses are one defect class (a gate a correct implementation cannot satisfy), and the mechanism is a pointer at an existing case rather than a new one.

**Optional stale-figure fix:** `.planning/ops/INDEX.md`'s row reads "Numbered planning lessons 1–25" while 31 exist and 32 lands here. Correcting it to 1–32 in the same commit is the same discipline the lesson records.

### D6. The EV-08 acceptance flip (the 16-file commit)

`tools/check/steps/important-gates.sh` derives the accepted requirement set from the `## Traceability` table (`:128-137`) and refuses any row whose fourth column is not in it (`:207-210`), while `important-gates-complete-uncovered` refuses a Complete requirement with no row. So **the first EV-08 registry row and the `Pending → Complete` flip must land in one commit.** The shape is the PKG-06 precedent, commit `298be25`, whose file list was exactly:

```
.planning/REQUIREMENTS.md                                            (checkbox, traceability row, Coverage prose, Last updated)
data/expected/important-gates.ids                                    (the new rows + the Phase 17 phase-table block)
tools/check/steps/important-gates.sh                                 (the Phase 17 phase-table block, comment only)
tools/check/controls/_good/tree/.planning/REQUIREMENTS.md            (toy row, appended)
tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md   (toy row, appended)
tools/check/controls/_good/tree/data/expected/important-gates.ids    (toy line, appended)
+ nine overlay registries under tools/check/controls/important-gates-*/overlay/data/expected/important-gates.ids
+ tools/check/controls/important-gates-complete-uncovered/overlay/.planning/REQUIREMENTS.md
```

Sixteen files. Ten toy registries, three toy requirement records. The Phase 17 toy identifiers are `toy-phase17 fault GOOD_FAULT EV-97` and `| EV-97 | Phase 17 | Complete |` (`EV-98` is Phase 13's, `PKG-96` Phase 16's).

**Append, never insert.** Three controls pin a *line number*: `important-gates-bad-req-phase5/EXPECT` reads `FAIL important-gates-unknown-id: line 19` and `-phase6` reads `line 24`. Every toy line goes at the **end** of its file, after `toy-phase16 fault GOOD_FAULT PKG-96` / `| PKG-96 | Phase 16 | Complete |`, so no pinned number moves.

**Cost note.** Frozen unit prices (`.planning/milestones/v0.3.0-phases/15-.../15-RESEARCH.md:922`): process id 0.10, control 0.31, in-step selftest 0.05, registry row 0.005 hosted cpu-s. Phase 17 therefore prices 0.10 (one process id) + 0.005-0.01 (one or two rows) = **0.105-0.11**, against the 0.10 line in ROADMAP: over by 5-10 percent, inside the 25 percent repricing threshold. The *measured* local cost of the recommended id shape is 0.33 cpu-s on dev plus 0.04 on release (0.37 local, about 0.62 hosted at the 1.675 factor) — 3 percent of the 21 allowance, and it must be **recorded beside the unit price** in the phase record rather than hidden, because §2.4 asks for the measured addition and the checkpoint's verdict is `reprice` in any case.

## Standard Stack

**No library, package or tool is added.** This phase writes Markdown, appends registry rows and adds one case to an existing shell suite. The frozen stack it uses:

### Core
| Item | Version/path | Purpose | Why this one |
|------|--------------|---------|--------------|
| `awk`, `sort`, `sed`, `dd`, `od`, `grep`, `diff` | POSIX | The window recipe, the container, the digest reads | Already the gate's whole vocabulary; the criterion names the `awk` recipe |
| `tools/flatwrap` (`ivs_flatwrap`) | build tree, `build/<preset>/tools/flatwrap/ivs_flatwrap` | Writes the container under test | The tool the id is about; build-only, no install rule |
| `tests/host/ivs_host_harness` (`ivs_host_harness`) | build tree, `build/<preset>/tests/ivs_host_harness` | The fake frontend that dlopens the shim and prints the nine `digest.` lines | It is the only committed way to read a digest through the shim |
| `host/libivs_retro.dylib` / `.so` (`ivs_retro_shim`) | build tree | The build-only object under test | D-48/D-49; no new accessor is added (D-52) |
| `runner/ivs-run` | build tree | The second entry point whose `digest.images` and `slot.cart.sha256` complete the join | Already the suite's main subject |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| A zero-word container (`dd if=/dev/zero`) | Re-wrapping the real lamp-tone program, extracted from `build/<preset>/images/lamp-tone.ivsc` | The real program gives `outcome pass` under the pass scenario and an already-proven harness drive, but its `digest.images` equals the pass golden's, so an input-ignoring stand-in passes the first limb, and the extraction makes the input a slice of a container the project's own packer wrote. Code Examples 2 and 3 give both, with commands |
| A second harness run for the flipped container | Reusing the existing `host-pass` output | Reuse saves one process (measured 0.12 local cpu-s on dev) but the `host-pass` run digests the build-tree image, not the tool's container, so it cannot serve this id without a byte equality that makes the flip unobservable |

**Installation:** none. No `npm`, `pip`, `cargo` or vendored file is involved, so the Package Legitimacy Gate is not triggered (see below).

## Package Legitimacy Audit

**Not applicable — this phase installs nothing.** It adds no dependency, no vendored file and no new build target. The three executables it exercises (`ivs_flatwrap`, `ivs_host_harness`, `ivs_retro_shim`) already exist in the build graph and are built by the presets that already exist (`tests/CMakeLists.txt:735-760` passes them as the suite's sixth, seventh and eighth arguments).

*No package was discovered via search or training data, so no `[ASSUMED]` package name appears anywhere in this research.*

## Architecture Patterns

### System Architecture Diagram

```
                      .planning/ci-runs.tsv  (append-only, 119 lines)
                                 |
                    awk col13|col8|col10, NR>1, tail -5, sort -n, sed -n 3p
                                 |
                    +------------+-------------+
                    |                          |
             572 / 49 / 164             previous 367 / 34 / 115
                    |                          |
                    +------ phase record -------+
                                 |
                    exactly ONE checkpoint line (§2.4 vocabulary)

  dd if=/dev/zero (8192 B) --.
                             +--> ivs_flatwrap <flat> 4000 <out.ivsc> --> stdout: sha256(container)
                                        |                                        |
                             (one byte flipped, re-wrapped)                    |
                                        |                                        |
              +-------------------------+--------------------------+             |
              |                                                    |             |
     content dir (mktemp): boot-stub-2.exec + lamp-tone.ivsc        |             |
              |                                                    |             |
      ivs_host_harness <shim> <dir> --> digest.images (H)     ivs-run --content-dir <dir> <scn>
              |                                                    |             |
              |                                       digest.images (R), slot.cart.sha256 (S)
              |                                                    |             |
              +---- require H == R  AND  S == the tool's stdout ----+-------------+
                             (repeated for the flipped container:
                              H', R' must both differ from H, R;
                              S' must differ from S)
```

### Recommended Project Structure (files this phase touches)

```
docs/formats/ivsc-v1.md              # + "Wrap tool refusals" section, inserted at line 111
docs/packaging.md                    # lines 39 and 123 corrected in place
.planning/ops/RELEASE-OPS.md         # section (i) amended in place, superseded line quoted + dated
.planning/ops/PLAN-LESSONS.md        # + lesson 32
.planning/REQUIREMENTS.md            # EV-08 checkbox, traceability row, Coverage prose, Last updated
data/expected/important-gates.ids    # + 1-2 mutant rows + a Phase 17 phase-table block comment
data/expected/tests/runner-process.ids   # + 1 line (after line 241)
tests/runner-process-tap.sh          # + 1 case after `result runner-process.judge.flatwrap-roundtrip`
tools/check/steps/important-gates.sh # + the Phase 17 phase-table block
tools/check/controls/**              # 10 toy registries + 3 toy requirement records (append at END)
```

### Pattern 1: A judge id that proves a cross-process join

**What:** one case that (a) has a tool write a container, (b) reads the same container's digest through two independent entry points, (c) ties the identity to the tool's own published value, and (d) repeats all of it on a one-byte mutation that must move every limb.

**When to use:** when the claim is "process A consumed what process B produced" and no single process can observe both sides.

**Shape, taken from the already-registered `runner-process.judge.flatwrap-roundtrip`** (`tests/runner-process-tap.sh:1809-1841`): the case opens with `: > "$tmp/why"`, `ok=1`, builds under `$tmp`, uses `run_one` for each process, uses `op_value <name> <key>` (`tests/runner-process-tap.sh:1454`) to read a report line, and closes with `result runner-process.judge.<slug>`. Findings go through `why <token> <detail>` (`:275`), which appends to `$tmp/why` and is printed under the `not ok` line by `result` (`:615-621`).

### Pattern 2: A doc section whose tokens start their own line

**What:** the criterion's check is `diff` of two `grep '^flatwrap-'` outputs, so every token must be the *first* thing on its line in the doc.

**When to use:** here, because a table row (`| \`flatwrap-length\` | ... |`) or a bullet (`- flatwrap-length`) does not match `^flatwrap-` and would make the `diff` non-silent.

### Pattern 3: A lesson scoped to its mechanism

**What:** lesson 31's own warning is that "a rule whose scope is wider than its mechanism teaches readers to quote it". Lesson 32 has three clauses and one-and-a-half mechanisms, so the lesson must say which clause each mechanism covers and which clause is a planner obligation with no parser.

**When to use:** mandatory here — see D5 for the clause-to-mechanism map.

### Anti-Patterns to Avoid
- **Restating a quoted figure.** CONTEXT's "`build_s` 47 to 51" and "`r` flat inside 10.6 to 11.7" are rounded; the file says 34 to 51 and 10.51 to 11.67. The record prints the computed values and the recipe, never the quote (project rule: recompute, never quote).
- **A pipeline tail in a verify block.** `grep ... | tail -1` or `... | sed -n '$p'` deciding an outcome is the exact defect lesson 32 records. Use `grep -q -x -F` on a whole line, or compare two files with `cmp -s`.
- **Process substitution in a verify block.** The criterion's literal `diff <(grep ...) <(grep ...)` is bash-only; the project runs `sh` and `dash`. Write the two `grep`s to files and `diff` the files.
- **Re-appending a ledger row.** A missing row is an operator stop. The seven rows are present (verified: each of the seven run ids appears exactly once).
- **Adding a case to a selftest family.** A registry row must resolve to an *existing* case; a new case costs 0.05 cpu-s and reprices the phase.
- **A second mechanism row for the count-gate contract.** The Deferred list refuses it by name.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Reading a digest through the shim | A new `slot.cart.sha256` accessor on `libretro_shim.h` | `ivs_retro_digest(IVS_DIGEST_IMAGES, out32)` | D-52 forbids new core/host surface; the IMAGES domain already serialises `slot.cart.sha256` |
| Getting a runnable cartridge without operator bytes | A new fixture or a committed container | `dd if=/dev/zero` + `ivs_flatwrap` under `$tmp` | `.ivsc` is refused in the tree by `tools/check/binary-ext.txt`; the roundtrip case already uses exactly this idiom |
| Reading a report value | A new `sed` per case | `op_value <name> <key>` (`tests/runner-process-tap.sh:1454`) | One reader, one behaviour, already used by every case |
| Running a process with a bound | `timeout(1)` or a bare `&` | `run_one` (`:250`), which arms the suite's watchdog and sets `rc` and `hung` | A hung child is a finding (`hung`), never a hang |
| Recomputing the window | A script, a CSV parse or a checked-in number | The one `awk` recipe, printed in the record beside the numbers | The criterion is that a reader can re-derive every figure from the named file |
| Counting what a step prints | A new count gate | The step's own `Stats:` line plus the ids file it names | W8's mechanism is a pointer, not a new gate |

## Runtime State Inventory

This is a records phase, so the "what still holds the old string" question is real even though no code is renamed.

| Category | Items Found | Action Required |
|----------|-------------|------------------|
| Stored data | `.planning/ci-runs.tsv` holds the 118 rows the window is computed from; **append-only**, and unchanged by this phase (it grows only when the phase's own hosted run is recorded) | None — verified, never re-appended |
| Live service config | None. No workflow, ruleset or service configuration names the packaging sentence; `.github/workflows/*` is untouched and `ci-parity` pins nothing about `docs/` | None — verified by `grep -rn "packaging.md" tools/ .github/` (only `tools/check/rights-lib.cmake:359`) |
| OS-registered state | None. No task scheduler, launchd or pm2 unit exists in this repository | None |
| Secrets and env vars | None. No secret name or env var references the corrected sentence; `docs/packaging.md`'s claim is prose only | None |
| Build artifacts | None carry the wording. `build/` holds images and tools only; `release-assets` checks archive *members*, never document text | None |
| Superseded prose in a live record | `.planning/ops/RELEASE-OPS.md` (i.1) quotes the v0.3.0 line as a dated record; `docs/packaging.md` lines 39 and 123 carry the wrong claim twice | Amend (i) in place with the superseded sentence quoted and dated; correct both packaging sentences inline. **Do not rewrite (i.1)** — it is a dated append |

**Canonical question answered:** after every file in the repo is updated, no runtime system still holds the old packaging sentence, because no runtime system reads it. The only place the old sentence survives is where `GUARDRAILS-v0.4` §2.12 requires it: quoted beside the amendment in the live record.

## Common Pitfalls

### Pitfall 1: A registry row naming a Pending requirement
**What goes wrong:** `important-gates-unknown-id: line <n>` on a tree that is otherwise green.
**Why it happens:** `tools/check/steps/important-gates.sh:207-210` accepts a row only when `$4` is in the derived *Complete* set (`:128-137`); EV-08 is Pending in the traceability table until the phase flips it.
**How to avoid:** land the first EV-08 row and the `Pending → Complete` flip in **one commit**, with the thirteen toy lines, exactly as commit `298be25` did for PKG-06.
**Warning signs:** `important-gates-unknown-id` on the new row's line number, or `important-gates-complete-uncovered: EV-08` if the flip lands first.

### Pitfall 2: A pinned control line number moving
**What goes wrong:** `tools/check/controls/important-gates-bad-req-phase5/EXPECT` reads `FAIL important-gates-unknown-id: line 19`, and `-phase6` reads `line 24`. A toy line inserted anywhere but the end shifts one of them and the control fails for the wrong reason.
**Why it happens:** the EXPECT pins a line number of an overlay registry.
**How to avoid:** append the Phase 17 toy line at the **end** of each of the ten toy registries (after `toy-phase16 fault GOOD_FAULT PKG-96`) and the toy row at the end of each of the three toy requirement records (after `| PKG-96 | Phase 16 | Complete |`).
**Warning signs:** `control-failed: important-gates-bad-req-phase5` with a different line number.

### Pitfall 3: The doc section fails the criterion's own `diff`
**What goes wrong:** the section reads well but `grep '^flatwrap-' docs/formats/ivsc-v1.md` returns nothing, because each token is inside a table cell or behind a bullet.
**Why it happens:** the doc's other sections are tables and bullets.
**How to avoid:** make each token the first text on its line, bare or backticked **only if the backtick is not first** — the safest form is the bare token followed by the trigger and the accepted boundary on the same or the next line. Then `grep '^flatwrap-'` yields the eight tokens in ids order.
**Warning signs:** `diff` prints all eight lines as differing, or the doc grep is empty.

### Pitfall 4: A claim word or a bare unit trips `text`/`glossary`
**What goes wrong:** `text-claim-untagged: docs/...:<line>` or `glossary-bare-clock: docs/...:<line>`.
**Why it happens:** `tools/check/steps/text.sh:6` treats `supports, compatible, passes, accurate, verified, runs, plays, playable, looks, sounds` as claim words needing a `[claim:CL-NNNN]` or `[nc]` on the same line, and `tools/check/steps/glossary.sh:51-59` refuses any line in `README.md` or `docs/**/*.md` containing the word "clock" outside four named contexts.
**How to avoid:** write the new prose in the doc's existing voice; describe conditions ("an input of an odd byte count") rather than outcomes ("this runs"), and never use the time-base word.
**Warning signs:** the token names above in the gate log.

### Pitfall 5: A hygiene hit from the research record itself
**What goes wrong:** `hygiene.sh --tree` refuses a home-path literal, an absolute scratch prefix or an email-shaped string in **any** tracked file, including `.planning/` (lesson 25/26: a record that certifies the absence of a pattern must not quote the pattern).
**Why it happens:** prose about paths and locators is exactly where these shapes appear.
**How to avoid:** name shapes, not literals; never write an at-sign followed by a dotted word; never write an absolute home prefix.
**Warning signs:** `-- FAIL hygiene-...` with a `.planning/` path.

### Pitfall 6: `tests/` and `tools/` are ASCII-only
**What goes wrong:** `integrity-non-ascii` on `tests/runner-process-tap.sh` or `data/expected/important-gates.ids`.
**Why it happens:** `tools/check/integrity.cmake:60-66` applies the ASCII rule to `data|fixtures|tools|.github|core|include|tests|cmake|runner|host`, and `docs/` is **not** in that set.
**How to avoid:** keep the new shell comments and the registry comments pure ASCII (no em dash, no arrows); `docs/` prose may keep the em dashes the existing files use.
**Warning signs:** `integrity-non-ascii: <path>`.

### Pitfall 7: Predicting a token line that the phase's own `.planning/` files move
**What goes wrong:** `files=`, `tracked=` and `hygiene-files=` all count the `git ls-files` list, which includes `.planning/`. Every PLAN.md and SUMMARY.md the phase commits moves them.
**Why it happens:** the counts are over the whole tracked file list (`git ls-files | wc -l` = 2801 at HEAD).
**How to avoid:** derive the baseline at the plan's own start commit and add the exact files the plan commits; state the sha beside it (this is lesson 32 clause 2 applied to the plan itself).
**Warning signs:** `files=2726` where the plan predicted 2725.

## Code Examples

All of the following were executed in this session; the outputs are quoted verbatim.

### 1. The window, by the one recipe (columns 13 = `cpu_s`, 8 = `build_s`, 10 = `gate_s`)

```sh
for c in 13 8 10; do
  printf 'col%s: ' "$c"
  awk -F'\t' 'NR>1{print $'"$c"'}' .planning/ci-runs.tsv | tail -5 | sort -n | sed -n 3p
done
```
Output at HEAD (`2f460c0`, `.planning/ci-runs.tsv` unchanged since `fb50835`):
```
col13: 572
col8: 49
col10: 164
```
The last five rows, as `cpu_s/build_s/gate_s`:
```
584/51/171
580/51/165
572/49/164
530/49/154
368/35/115
```
The previous window (the five rows ending at run `45833228626`, i.e. before the seven release rows): `367 / 34 / 115`.

Row verification (each of the seven ids appears exactly once):
```sh
for id in 45835631107 45836207855 45839153455 45839646456 45841732895 45842795129 45843408874; do
  printf '%s ' "$id"; awk -F'\t' -v i="$id" 'NR>1 && $2==i' .planning/ci-runs.tsv | wc -l
done
```
Output: `1` for every id. Line count: `wc -l` = 119, `awk -F'\t' 'NR>1' | wc -l` = 118.

**Two recomputed corrections to CONTEXT's prose** (print these, not the quotes):
```
the seven rows' build_s: 51, 42, 34, 47, 51, 51, 49   -> range 34 to 51 (CONTEXT says "47 to 51")
the seven rows' cpu_s:  588, 467, 365, 513, 584, 580, 572 -> range 365 to 588
r = cpu_s/build_s, last five rows: 11.4510, 11.3725, 11.6735, 10.8163, 10.5143 -> 10.51 to 11.67
```

### 2. The new id's four measured values, on both presets

Content dir under `$tmp`: `boot-stub-2.exec` copied from the build images dir, plus the tool's container named `lamp-tone.ivsc` (the harness reads that literal name; `tests/host/ivs_host_harness.c:434`).

```sh
dd if=/dev/zero of="$tmp/fw-flat.bin" bs=8192 count=1
d1=$("$flatwrap" "$tmp/fw-flat.bin" 4000 "$tmp/fwshim/lamp-tone.ivsc")
"$harness" "$shim" "$tmp/fwshim"                     # -> digest.images  f5e8555f96ebcf8a...
"$ivs_run" --content-dir "$tmp/fwshim" "$tmp/fwshim/scn.ivss"   # -> digest.images f5e8555f... ; slot.cart.sha256 8d73b98c58bb75a6...
printf '\001' | dd of="$tmp/fw-flat.bin" bs=1 seek=8191 conv=notrunc
d2=$("$flatwrap" "$tmp/fw-flat.bin" 4000 "$tmp/fwshim/lamp-tone.ivsc")
"$harness" "$shim" "$tmp/fwshim"                     # -> digest.images  ed1fbaf748795b95...
"$ivs_run" --content-dir "$tmp/fwshim" "$tmp/fwshim/scn.ivss"   # -> digest.images ed1fbaf7... ; slot.cart.sha256 c41b6dab1d86806c...
```

Measured values (identical on `build/dev` and `build/release`):

| limb | container A (8192 zero bytes, base 4000) | container B (last input byte flipped) |
|---|---|---|
| `ivs_flatwrap` stdout | `8d73b98c58bb75a6515c6a791ebd2925265d05874e324c31a5836e61055b74f5` | `c41b6dab1d86806c20af8af5a4af21d72046ec639de108a558eb142ca9607b7e` |
| harness `digest.images` | `f5e8555f96ebcf8a23c697e889886903f47f13233004ee92c934181fe05e61bf` | `ed1fbaf748795b95f78bdb7cded7a784fa5d86211de2dd68e0900765d491ac25` |
| runner `digest.images` | `f5e8555f...` (equal) | `ed1fbaf7...` (equal) |
| runner `slot.cart.sha256` | `8d73b98c...` (equal to the tool's stdout) | `c41b6dab...` (equal) |
| runner `outcome` / `exit.code` | `pass` / `0` | `pass` / `0` |
| harness `frames`, `audio.frames`, `pixel.probes` | `13`, `10816`, `ok 198432` | `13`, `10816`, `ok 198432` |

The inline scenario that makes the runner limb cheap (1000 ticks, no asserts; the suite already writes scenarios this way for `cart-user`):
```
ivss 1
id flatwrap-shim
profile intv-2609-ntsc-family-p1
content exec boot-stub-2
content grom zero-grom
content cart lamp-tone.ivsc
run-to 1000 ticks
```
`runner/scenario.c:324-369` resolves `boot-stub-2` to the content dir's `boot-stub-2.exec`, `zero-grom` to an in-process zero GROM, and a bare name to a content-dir path — the same three resolutions the pass scenario uses, so `digest.images` is comparable to the harness's.

**Why `digest.images` is the right limb** (verbatim, `core/src/machine/machine_digest.c:40-52`):
```c
  IVS_DF("slot." label ".kind", slot[i].kind, 4u, 1u, IVS_DIGEST_IMAGES, IVS_DIGEST_KIND_UINT,            \
  IVS_DF("slot." label ".loaded", slot[i].loaded, 4u, 1u, IVS_DIGEST_IMAGES, IVS_DIGEST_KIND_UINT,    \
  IVS_DF("slot." label ".length_bytes", slot[i].length_bytes, 4u, 1u, IVS_DIGEST_IMAGES,               \
  IVS_DF("slot." label ".length_units", slot[i].length_units, 4u, 1u, IVS_DIGEST_IMAGES,               \
  IVS_DF("slot." label ".unit_bits", slot[i].unit_bits, 4u, 1u, IVS_DIGEST_IMAGES,                     \
  IVS_DF("slot." label ".base_addr", slot[i].base_addr, 4u, 1u, IVS_DIGEST_IMAGES,                     \
  IVS_DF("slot." label ".sha256", slot[i].sha256, 1u, 32u, IVS_DIGEST_IMAGES, IVS_DIGEST_KIND_UINT,    \
```
So the IMAGES domain is a serialisation **containing** `slot.cart.sha256`; equality of the domain across the shim and the runner, plus the runner's `slot.cart.sha256` equalling the tool's stdout, is the join. There is no slot accessor in `host/libretro_shim.h` (only `ivs_retro_digest(domain, out32)`), and D-52 forbids adding one.

### 3. The alternative shape (real lamp-tone program), if the zero-word container is rejected

Re-wrapping the project's own container reproduces it byte for byte, which is why this shape is available at all:
```sh
sh_img="$content/lamp-tone.ivsc"; sh_size=$(wc -c < "$sh_img")
dd if="$sh_img" of="$tmp/sh-flat.bin" bs=1 skip=32 count=$((sh_size - 64))   # one segment: data starts at 24 + 8*1
"$flatwrap" "$tmp/sh-flat.bin" 5000 "$tmp/sh/lamp-tone.ivsc"                # base 5000 = the entry's addr field
```
Measured: the output's sha256 equals `ac0ad987da1330101f423b294b6c24ea14de36f43f6b2dae82cd91d32b8e0ba8`, the build image's own. Flipping the flat byte at offset 5 (the low half of word `5002`, a listing line commented `HLT fill`, unreachable behind the branch at `5001`) keeps the harness green and moves `digest.images` from `09679d87b9851e20...` (the pass golden's value) to `32a174092852b690...`, with `outcome pass` on both. **Cost:** 6 more shell lines, a base read from the container's own entry, and a first limb whose value equals the pass golden's.

### 4. The doc section's mechanical check, in a dash-safe form

```sh
grep '^flatwrap-' data/expected/flat-refusals.ids > /tmp/fw.ids
grep '^flatwrap-' docs/formats/ivsc-v1.md       > /tmp/fw.doc
diff /tmp/fw.ids /tmp/fw.doc          # silent
wc -l < /tmp/fw.doc                   # 8
```
The ids file yields, in order: `flatwrap-length`, `flatwrap-word-range`, `flatwrap-words`, `flatwrap-base`, `flatwrap-cross`, `flatwrap-output-tree`, `flatwrap-output-exists`, `flatwrap-empty`. Note `flatwrap-empty` sits after a comment line in the ids file, so the doc's order is *not* the tool's check order — the section must say so (the tool's order is empty, length, words, base, crossing, output tree, output exists, word range, `tools/flatwrap/flatwrap.c:31-42`).

### 5. The eight rows' trigger and accepted boundary (from `tests/pack-tap.sh:247-360`)

| token | trigger | boundary that must still be accepted |
|---|---|---|
| `flatwrap-length` | an input whose byte count is odd (1 byte is not a whole word) | 2 bytes = one word, a 66-byte container |
| `flatwrap-word-range` | a word above `03FF` (`0400` refused) | the word `03FF`, read back as `03ff` |
| `flatwrap-words` | more than 65,536 words (65,537 refused) | the count rule's threshold is exactly 65,536 — 65,536 words is refused by `flatwrap-cross` instead; the real accepted boundary is 49,152 words at base 4000 |
| `flatwrap-base` | a base below `4000` (`3FFF` refused) | base `4000` |
| `flatwrap-cross` | `base + words` above `10000` (49,153 words at 4000 refused) | 49,152 words at 4000, a 98,368-byte container ending at `FFFF` |
| `flatwrap-output-tree` | the output's physical parent directory inside the work tree and not under `refs/` — the literal spelling, a `..` spelling and a symlink all answer the same | an output outside the tree |
| `flatwrap-output-exists` | the output path already exists (the refused run leaves it byte-identical) | a new path |
| `flatwrap-empty` | a zero-byte input: no word, and a segment holds at least one | one word, the 66-byte smallest container |

A refusal is one stderr line `ivs_flatwrap: <token>: <detail>`, exit status 1, and no output file; a file or usage error has no token and exits 2 (`tools/flatwrap/flatwrap.c:43-48`).

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Quote the cost window from a record | Recompute the last-five medians by one `awk` recipe at the plan's own start commit | v0.4.0 (`GUARDRAILS-v0.4` §2.4, D-55) | The phase record must print the recipe and the numbers before its first pricing sentence |
| Price a phase by its hosted median | Price locally, times 1.675, against an allowance recomputed from the ledger; monitor hosted | v0.4.0 (D-55) | The frozen unit prices are process id 0.10, control 0.31, in-step selftest 0.05, registry row 0.005 hosted cpu-s (`.planning/milestones/v0.3.0-phases/15-.../15-RESEARCH.md:922`) |
| `r = cpu_s / build_s` as a decision input | `cpu_s` plus the measured per-step profile; `r` retired | v0.3.0 close (lesson 29, D-55) | `r` may be printed as arithmetic but never as a signal |

**Deprecated/outdated:**
- `INDEX.md`'s line "`.planning/ops/PLAN-LESSONS.md` | Numbered planning lessons 1–25" is stale (31 lessons exist; lesson 32 lands here). Optional but cheap: correct it to 1–32 in the same commit as lesson 32 (it is a quoted figure that disagrees with the file).

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | A zero-word container (8192 zero bytes at base 4000) keeps every harness assertion green **on the hosted Ubuntu dev preset**, which was not exercised in this session (macOS dev and release only) | D3, Code Example 2 | The id fails in CI on a limb unrelated to the join; fall back to Code Example 3's real-program shape |
| A2 | The `count-gate` registry row should point at `slowest-eight` (an existing, currently unregistered case of `ci-summary-selftests.ids`) | D5 | If a plan-checker reads "count gate" strictly, the row points at a rendering case; `extra-file` is the fallback (already registered under `pkg-install`/PKG-01) |
| A3 | Lesson 32 clause 2 (the baseline names the plan's own start commit sha) has no mechanism at all and can only be a planner obligation | D5 | If a mechanism is demanded, the phase needs a new case (0.05 cpu-s) and a reprice |
| A4 | The EV-08 acceptance flip belongs in the plan that adds the first EV-08 row (17-02), not 17-03 | D6 | A §2.7 deviation for one commit, of the shape Phase 14 recorded |

**If this table is empty:** it is not — A1 is the one that needs a CI run to close.

## Open Questions

1. **Does the harness behave identically under the Ubuntu dev preset with a halted guest?**
   - What we know: it is green on macOS on both presets; the drive, the six events, the 13 frame calls and the 806568-tick bound are the same ones `runner-process.host.digests` already runs in CI.
   - What's unclear: nothing measured; the guest program differs from the proven one.
   - Recommendation: land the zero-word shape, and if the first CI run disagrees, switch to Code Example 3's real-program shape in the same plan (it is a 6-line diff, not a redesign).

2. **Which registry row carries the count-gate contract?**
   - What we know: no gate step parses plans (`grep -rln "PLAN[.]md\|-PLAN" tools/check/` is empty), so clauses 1 and 2 have no checker; clause 3's mechanism is `st-pass-line` plus `tools/check/pass-line.sh`.
   - What's unclear: whether "a count gate" means a gate step printing a count token (then `extra-file`/`prefix-files=11` is the fit) or any count derived from a named file (then `slowest-eight` is the fit).
   - Recommendation: `count-gate mutant slowest-eight EV-08` — it registers a mechanism that has no row today, which is the EV-07 precedent's own criterion, and it proves a count is taken from a named file (`step.times`: 12 entries in, exactly the 8 slowest out).

3. **Does 17-02 carry the 16-file flip, or does 17-03?**
   - What we know: the first EV-08 row must land with the flip (D6); §2.7 wants a new check's row in its own commit.
   - What's unclear: whether the planner prefers a heavy 17-02 or a recorded §2.7 deviation.
   - Recommendation: 17-02, split into two commits (documents; then id + inventory line + row + flip). 17-03 then adds only lesson 32 and the `count-gate` row.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|-------------|-----------|---------|----------|
| `sh`, `dash` | Every gate invocation, every verify block | yes (both) | system | none needed |
| `awk`, `sed`, `sort`, `dd`, `od`, `grep`, `diff` | The window recipe, the container, the digest reads, the doc check | yes | POSIX | none needed |
| CMake + Ninja + a C compiler | `build/dev`, `build/release` (both exist with images and tools) | yes | 4.4.3 / Apple clang locally; 3.28.3 / GCC 13 hosted | none needed |
| Docker | `tools/gate-linux.sh` (container gate) | not probed in this session | — | write a visible `not run` line (lesson 24); never a pass |
| The three build-tree executables | The new id | yes, both presets (`build/{dev,release}/{tools/flatwrap/ivs_flatwrap,tests/ivs_host_harness,host/libivs_retro.*,runner/ivs-run}`) | current tree | the id reports `not ok` with the reason, never a skip |

**Missing dependencies with no fallback:** none identified.
**Missing dependencies with fallback:** Docker, if the container gate cannot run (record `not run`).

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | The project's own gate: 38 shell "steps" plus TAP suites run under CTest in two presets (`dev`, `release`) |
| Config file | `tests/CMakeLists.txt` (suite registration), `tools/gate.sh` (step order), `data/expected/tests/*.ids` (id inventories) |
| Quick run command | `sh tools/gate.sh > /tmp/g.out 2>&1; tail -1 /tmp/g.out` and the same with `dash` |
| Full suite command | `sh tools/gate-linux.sh > /dev/null 2>&1; grep -q '^gate: PASS' build/gate-linux.log` |

### Phase Requirements → Test Map
| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| EV-08 (c1) | The window is recomputed from the named file at the named commit | manual-only (a record, not a check) | `awk -F'\t' 'NR>1{print $13}' .planning/ci-runs.tsv \| tail -5 \| sort -n \| sed -n 3p` | n/a — the record is the artifact |
| EV-08 (c1) | The seven rows are present, once each, 118 over 119 | step | `sh tools/check/steps/ci-log.sh` (`ci-runs=<rows>`, header/fields/duplicate/order tokens) | ✅ |
| EV-08 (c2) | The doc lists exactly the eight tokens in ids order | manual verify (dash-safe `diff` of two greps) | `grep '^flatwrap-' data/expected/flat-refusals.ids > /tmp/a; grep '^flatwrap-' docs/formats/ivsc-v1.md > /tmp/b; diff /tmp/a /tmp/b` | ✅ (inventory exists) |
| EV-08 (c2) | The inventory itself is unchanged | step | `tests/pack-tap.sh` `pack.judge.flatwrap-tokens` (the pin against the source's literals) | ✅ |
| EV-08 (c3) | The shim digested the container the tool wrote | suite | `runner-process.judge.flatwrap-shim` (new, one id, both presets) | ❌ Wave 0 (the id itself is the deliverable) |
| EV-08 (c3) | The id can fail | registry | `important-gates` row over the id (a `mutant` row resolving through `data/expected/tests/runner-process.ids`) | ❌ same commit |
| EV-08 (c3) | No new step | step | `ls tools/check/steps/*.sh \| wc -l` = 38 | ✅ |
| EV-08 (c4) | The two documents agree sentence for sentence | manual verify | extract the line from `RELEASE-OPS.md` (i) and require exactly two verbatim occurrences in `docs/packaging.md` | ✅ |
| EV-08 (c5) | Lesson 32 exists with its mechanism | registry | `important-gates` row `count-gate mutant slowest-eight EV-08` | ❌ same commit |
| EV-08 (c5) | The invariants hold | step | `faults=238`, `goldens=29`, `claims=29`, `provisional=28` in the gate token line | ✅ |

### Sampling Rate
- **Per task commit:** `sh tools/gate.sh` and `dash tools/gate.sh`, both ending `gate: PASS` with the plan's predicted token string.
- **Per wave merge:** the same, plus the container gate (`tools/gate-linux.sh`) once per plan that changes `tests/` or `data/expected/`.
- **Phase gate:** full suite green, both shells, before `verify-work`.

### Wave 0 Gaps
- [ ] `tests/runner-process-tap.sh` — the `runner-process.judge.flatwrap-shim` case (one case, after `result runner-process.judge.flatwrap-roundtrip`, currently line 1841).
- [ ] `data/expected/tests/runner-process.ids` — one line after line 241 (`runner-process.judge.flatwrap-roundtrip`).
- [ ] `data/expected/important-gates.ids` — one row for the new id, one row for the count-gate mechanism, plus the Phase 17 phase-table block.
- [ ] `tools/check/steps/important-gates.sh` — the Phase 17 phase-table block (a comment; the step's logic does not change).
- [ ] Framework install: none — the suites, the presets and the harness all exist.

### Predicted token deltas (derive the baseline at the plan's own start commit)

Measured at HEAD (`2f460c0`, clean tree): `steps=38`, `controls=404`, `faults=238/238`, `goldens=29`, `gates=119`, `lines=364`, `tests-expected=5042` (2521 ids × 2 presets), `tests-executed=5038`, `tests-skipped=4`, `prefix-files=11`, `claims=29`, `provisional=28`, `ci-runs=118`, `git ls-files | wc -l` = 2801.

| token | 17-02 | 17-03 | note |
|---|---|---|---|
| `tests-expected` | 5044 (+2) | 5044 | one new id across two presets |
| `tests-executed` | 5040 (+2) | 5040 | the new id runs on both presets |
| `gates` | 119 | 120 (+1) | `count-gate` is the only new gate name; the id's row reuses `flatwrap` |
| `lines` | 365 (+1) | 366 (+1) | registry rows only |
| `files` / `tracked` / `hygiene-files` | + the `.planning/` files the plan commits | likewise | count `git ls-files`, which includes `.planning/` |
| everything else | unchanged | unchanged | `controls`, `faults`, `goldens`, `claims`, `provisional`, `steps`, `docs`, `scanned`, `prefix-files`, `pkg-selftest`, `release-selftest`, `ci-runs` |

## Security Domain

`security_enforcement` is `true` (`.planning/config.json:49`), so this section is included. This phase adds no parsing surface, no network path, no secret and no installed file.

### Applicable ASVS Categories
| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V2 Authentication | no | — |
| V3 Session Management | no | — |
| V4 Access Control | no | — |
| V5 Input Validation | yes (already in place) | `tools/flatwrap` refuses a zero-byte, odd-length, oversized, out-of-base, crossing, in-tree-output, existing-output and out-of-range-word input by token, and its two operator paths are opened without blocking (`tools/flatwrap/flatwrap.c:49-56`) |
| V6 Cryptography | no new use | SHA-256 only through the existing `common/ivs_sha256.h`; no hand-rolled digest |

### Known Threat Patterns for this stack
| Pattern | STRIDE | Standard Mitigation |
|---------|--------|---------------------|
| A test that writes into the work tree (derived material committed by accident) | Tampering | The case writes only under the suite's `mktemp` dir; `flatwrap-output-tree` refuses an in-tree output; `.ivsc` is refused by name in the tree |
| A hung child in a judge case | Denial of service (of the gate) | `run_one` arms the suite's watchdog and reports `hung` as a finding |
| A record that quotes a refused name | Information disclosure (rights) | `rights_scan_naming` (`tools/check/rights-lib.cmake:353-360`) scans `docs/packaging.md` for the refused and owner-local tokens of `data/rights/*.json`; the page is clean today, and the corrected sentence must introduce no new name |
| A test that passes vacuously | Repudiation (of the evidence) | The flipped-input limb plus a `mutant` registry row; `GUARDRAILS-v0.4` §2.7 and §2.8 |

## Sources

### Primary (HIGH confidence)
- `.planning/ci-runs.tsv` (read, 119 lines) — every window figure recomputed in this session with the recipe quoted above.
- `tests/runner-process-tap.sh` (read: header `:60-121`, `run_one` `:250`, `judge` `:282`, `result` `:615`, `op_value` `:1454`, `cart-user` `:1730-1806`, `flatwrap-roundtrip` `:1809-1841`, host block `:2210-2400`) — the case shapes, the helpers and the two existing digests joins.
- `tests/host/ivs_host_harness.c` (read: `:1-60`, `:240-300`, `:400-470`, `:560-619`) — the hardcoded `boot-stub-2.exec` and `lamp-tone.ivsc` names, `HARNESS_CART_MAX_BYTES 131256`, `read_span`, the nine digest lines, the assertions that must stay green.
- `core/src/machine/machine_digest.c:40-52` (read, quoted verbatim) — `slot.cart.sha256` is a field of `IVS_DIGEST_IMAGES`.
- `tools/check/steps/important-gates.sh` (read in full) — the derived accepted set, the `mutant` resolution rule, the tokens, the phase table.
- `data/expected/important-gates.ids` (read `:1-14`, `:500-620`) — the registry's columns, the `st-pass-line` row at `:529`, the `flatwrap`/`cart-role`/`host` rows, the Phase 16 block.
- `data/expected/flat-refusals.ids` (read in full) — the eight tokens, the append-only rule, the `flatwrap-empty` header note.
- `docs/formats/ivsc-v1.md` (read in full) — section boundaries (`## Refusal reasons` at 79, `## Digests` at 111), the 18 reasons, the ASCII-only style.
- `docs/packaging.md` (read in full) — the wrong claim at lines 39 and 123, the eleven-path list, the release-content bullets.
- `.planning/ops/RELEASE-OPS.md` (read `:74-210`) — section (i), (i.1)'s dated quote, section (j)'s `digest=8d73b98c...` corroborating the base-4000 zero wrap.
- `.planning/ops/GUARDRAILS-v0.4.md` (read in full) — W8, W9, W10, §2.4, §2.7, §2.8, §2.12, §2.14, §3, §4, §7.4.
- `.planning/ops/PLAN-LESSONS.md` (read in full) — lessons 1-31; lesson 31 is the one lesson 32 must not repeat.
- `.planning/REQUIREMENTS.md:21`, `:88` — EV-08's text and its `Phase 17 | Pending` traceability row.
- Commit `298be25` (PKG-06 acceptance) — the 16-file shape of a requirement flip, quoted in D6.
- Executed measurements in this session: the harness, the runner and the case on both presets; the flatwrap digests; the `git ls-files` and id counts.

### Secondary (MEDIUM confidence)
- `.planning/milestones/v0.3.0-phases/15-build-only-host-and-the-fake-frontend-harness/15-RESEARCH.md:922` — the frozen unit prices (process id 0.10, control 0.31, in-step selftest 0.05, registry row 0.005).
- `.planning/ops/INDEX.md` — the one-hop index; its PLAN-LESSONS row reads "1–25" (stale).

### Tertiary (LOW confidence)
- A1 (the Ubuntu dev-preset behaviour of a halted guest) — not measured; it is the one claim in this research that a CI run must close.

## Metadata

**Confidence breakdown:**
- The cost window: HIGH — recomputed in this session with the criterion's own recipe; two prose figures corrected.
- The new id's shape: HIGH — executed end to end on both presets, with all four digest limbs quoted.
- The count-gate mechanism: MEDIUM — the mechanism map is solid (no step parses plans; `st-pass-line` is real), but which case the row should point at is a judgement call (A2).
- The documents: HIGH — every section boundary, line number and scan rule read; the `grep '^flatwrap-'` constraint is mechanical.
- The EV-08 flip's blast radius: HIGH — derived from the step's code and confirmed against the PKG-06 commit's file list.

**Research date:** 2026-11-26
**Valid until:** 30 days for the document and registry findings; **the cost window must be recomputed at the plan's own start commit and never reused from this file** (that is the phase's own first deliverable, and a `.planning/ci-runs.tsv` append invalidates every figure here).

## RESEARCH COMPLETE
