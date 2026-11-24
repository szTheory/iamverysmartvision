# Phase 9: Registered Evidence and Gate Cost - Research

**Researched:** 2026-11-24
**Domain:** in-repo gate machinery (test inventory, negative controls, requirement registry), plus one cost-only test-suite fold
**Confidence:** HIGH on mechanics (every claim below was read out of the tree this session, with path and line); MEDIUM on the EV-02 mechanism assignment (the ids exist, but "sole non-vacuous evidence" is a judgement the planner must ratify); HIGH on cost arithmetic (computed from `.planning/ci-runs.tsv`).

## Summary

Phase 9 is not a build phase. It is a phase about the *gate's own bookkeeping*, and every
question in it is answered by reading `tools/check/` and `data/expected/` rather than by
choosing a library. Three findings dominate the plan.

**One: EV-04 as specified cannot pass on the good tree.** The phase record says
`important-gates.sh` should "derive the must-be-covered set from the requirement records and
refuse a requirement marked Complete with no registry line". Measured this session: the
archived record marks **50** requirements Complete
(`.planning/milestones/v0.1.0-REQUIREMENTS.md`, traceability table) and the registry covers
**27** (`data/expected/important-gates.ids`, 306 non-comment lines). EV-02 adds 17, reaching
44. The remaining **6** — `API-01 API-05 API-06 RUN-03 PKG-02 REL-01` — have no registry
line and are not in EV-02's 17. A naive "every Complete requirement" derivation therefore
fails closed on the good tree on the day it lands. The derivation needs an explicit,
committed exclusion record, or EV-02 must grow to 23 requirements and 30 lines. The plan
must choose, and the CONTEXT does not say which.

**Two: two of EV-03's five items are already landed, and one of the five cannot be reached
from a control at all.** `glossary-no-files` and `sources-no-files` already exist *with*
controls (`tools/check/controls/gate-hardening.ids`, 4 entries). `gate-step-no-pass-line`
already exists in `tools/gate.sh` but no control directory can reach it: `run-controls.sh`
executes `tools/check/steps/<name>.sh` only, and the good tree carries no `tools/gate.sh`
and no `tools/check/`. The two genuinely missing items are IN-01 (whole-token `SRC-`) and
IN-03 (manifest output anchoring), both still absent, and both cheaply controllable. The
fifth item, the memmap generator, is reachable — `cmake/IvsMemmapProbes.cmake` is a
standalone `cmake -P` script with its own fail-closed messages — but only by adding an
invocation to an existing step.

**Three: the preflight partition's "byte-identical output" is stricter than the CONTEXT
sentence implies.** Steps already run in sorted name order and each step's output is
`cat`'d immediately (`tools/gate.sh:104-124`). Buffering only "tokens and `-- FAIL` lines"
would reorder the *bodies*, and the `step-time` lines and the `tokens` accumulation are both
emitted in execution order today. Byte-identical output requires buffering each step's whole
output plus its `step-time` line and replaying in sorted name order, and it requires building
the summary `tokens` string during the replay rather than during execution.

Cost is settled and small: the last-five median `r` is **7.696** (recomputed below), so wave
A1's row is almost certain to read `discharged-below-budget`; A2's new controls cost about
**1.2 local cpu-s** against a recorded allowance of ≈12.9 local cpu-s.

**Primary recommendation:** plan A1 as three cost-only commits (fold + judgement, trims,
measurement) with the closed file list; plan A2 as five commits in this order — EV-04's
records-and-derivation groundwork *with* EV-02's 24 lines in one commit (they cannot be
separated: the derivation is what makes the 24 lines checkable), then EV-03's two real fixes
plus their controls, then the preflight partition, then bookkeeping. Resolve the 6-exclusion
question and the `judge.variant-static` id question before writing any task.

### Provenance labels used in this document

Two conventions apply and they are made to agree here rather than run in parallel.

- A claim carrying a `path:line` citation and a verbatim quote is an **observed reading made in
  this session** — the project's `[OBS]` (`INDEX.md` rule 2) and the harness's
  `[VERIFIED: <path>:<lines>]`. Every such claim was produced by opening the file, not by
  grepping for a string: the quoted text is beside the citation so it is checkable.
- A count is `[OBS]` only when the command that produced it is printed beside it. Every count
  in this document has its command.
- `[INF]` marks an inference from observed facts — the 24-row mechanism assignment (§Q3.3), the
  A2 spend (§Q8.3), and the buffering requirement of §Q7.3 are the three that matter.
- `[REC]` marks a recommendation (the recommended resolutions in §Q5.1, §Q6 and the Open
  Questions).
- `[UNK]` marks a genuine unknown; the only one of substance is whether
  `runner-process.judge.variant-static` may add a test id in wave A1 (Open Question 1).
- Nothing in this document is `[ASSUMED]` in the sense of "training knowledge presented as
  fact": no claim rests on memory of the repository, and the Assumptions Log below lists the
  *judgement* assumptions (which shape to choose), each with its risk, not factual assumptions.

## User Constraints (from CONTEXT.md)

### Locked Decisions

**Wave A1 — cost-only scope, and the sentence that bounds it**

- **A1 is one push, and its file list is closed.** The plan enumerates every file A1 touches
  in `files_modified` and each task's `<files>`, and the plan carries a hard scope sentence:
  no file outside that list may change. The list is expected to be the runner-scenario
  inventory, the runner-process tap script (and, if the skip vocabulary lives there, the
  runner-process script or inventory), the scenario-pairs inventory, the state restore case,
  the state fuzz case, the test inventory comment lines that name the affected ids, and
  `.planning/CI-EFFICIENCY.md`. `.planning/ci-runs.tsv` is appended by the orchestrator,
  never by an executor. — **Reversibility:** reversible.
- **Lever (3) keeps its judgement, not just its saving.** `demo-lamp-tone-swap` and
  `demo-lamp-tone-shift` become static repeat variants of one mutant: column 5 of
  `data/expected/runner-scenarios.ids` moves from `run` to `release-only`, the scenario pair
  takes the same mode switch, and the tap script gains a derivation-equality judgement — the
  variant's `assert` block must equal, in order, the block between the
  `asserts-begin`/`asserts-end` markers of `docs/video/demo-lamp-tone-1-derivation.md`, its
  `content`/`profile`/`ivss`/`id` lines equal the pass scenario's, and its `input` lines are
  the pass inputs transformed exactly as documented. The judgement is the preserving
  condition: without it the fold is a coverage deletion. One suite self-control
  (`runner-process.judge.variant-static`) lands in the same commit and must reject a variant
  whose assert block differs from the derivation by one value.
- **The trims keep their recorded preserving conditions verbatim.** (a) keep the `zero`
  launch of `runner-process.roundtrip.refuse-range` — it alone proves exit 14, the empty
  report, the first stderr line and the usage text — and make `above-max`, `no-value` and
  `twice` static against the option table; (c) nine digests on every 8th refusal **plus the
  first and last row**, keeping the whole-instance comparison of every row and `produced > 0`
  as the liveness guard; (d) the first value only of each unsealed byte flip, keeping the
  guard that the flip *moves* the digest and the offset set unchanged. Trim (b) is declined
  with its measurement (0.09 local cpu-s against the only sanitized execution of the 65-cut
  loop) and the decline is recorded, not silently skipped.
- **A1 is measured before it is pushed.** Three alternated `/usr/bin/time -p` runs per preset,
  both presets, load average recorded, before and after figures stated. The lever's measured
  ceiling (0.12 local cpu-s) is quoted as a ceiling and not as a saving: it cannot move `r`.
- **Nothing in A1 may weaken a check to go faster.** No dropped control, fault, judge, oracle,
  golden or pinned-default test; no cache, `continue-on-error`, matrix or path filter.

**Wave A2 — EV-02 registration mechanics**

- **The 24 lines are chosen from mechanisms that already run.** Each row is a
  `<gate> <fault|control|mutant> <id> <REQ>` line whose id already exists in
  `data/expected/faults/*.ids`, `data/expected/controls/*.ids`,
  `data/expected/{pkg,ci-summary}-selftests.ids` or a `<suite>.judge.<slug>`/
  `<suite>.mutant.<slug>` test id. The plan states, per requirement, which existing mechanism
  is its sole non-vacuous evidence and why it cannot pass vacuously. No new fault site, no
  new test id, no new control.
- **The registry row count is a prediction, not an observation.** 306 → 330 (24 additions, 0
  deletions), and the plan predicts the `steps`, `controls`, `faults`, `tests-expected`,
  `tests-executed`, `gates`, `lines` and `claims` deltas before the commit. An unexpected
  token is a stop, not a re-bless.
- **The accepted set and all ten toy registries move in the same commit** (GUARDRAILS R2).
  Ten means `tools/check/controls/_good/tree/data/expected/important-gates.ids` plus the nine
  `important-gates-*/overlay/data/expected/important-gates.ids` files. Each gains exactly one
  line per new requirement and zero deletions, proved with `git diff --numstat` before the
  commit. Any `important-gates-bad-req-*` control whose never-valid id became valid is
  retargeted in the same commit.
- **The registry prose moves with the code.** The step's `nreq` split string, its header
  comment and its token prose are three separate edits in the same commit.

**Wave A2 — EV-04, the machine check that makes the gap unrepeatable**

- **Two mechanisms, both inside existing steps, neither a new step.** (i)
  `tools/check/steps/important-gates.sh` stops carrying a hand-written accepted set: it derives
  the must-be-covered set from the requirement records and refuses a requirement marked
  Complete with no registry line. The records it reads are `.planning/REQUIREMENTS.md` and the
  archived `.planning/milestones/v0.1.0-REQUIREMENTS.md`. (ii) `tools/check/run-controls.sh`
  gains a drift check: every toy registry must equal the good tree's registry plus exactly its
  declared deviation.
- **Both are shown able to fail, by a committed scratch stand-in, in the same commit as the
  mechanism.** The stand-ins are controls, not prose.
- **Deriving the set must not shrink coverage.** The derivation is from records outside the
  registry, so deleting a registry line cannot silently delete the requirement from the
  must-be-covered set.

**Wave A2 — EV-03 gate hardening, one control per item**

- **Five items, five controls, five intended reasons, and nothing else.** Whole-token `SRC-`
  match; zero-document failures; per-step pass lines; manifest output anchoring; memmap
  generator controls. Each control's token is pinned in `data/expected/controls/<step>.ids`.
- **A control names the guard that enforces it.** Before writing a control, the plan checks
  that the guard it targets actually lives in the script that would refuse it (lesson 7).
- **The memmap item is the expensive one and is scoped to what a pure step can run.** If it
  cannot be done without a new step, the item is recorded as deferred with the reason and the
  other four land — never a hollow control.

**Wave A2 — the preflight partition**

- **The partition is derived, never hand-maintained.** A step is preflight iff its script
  references neither the gate results directory nor `build/`. The derivation is written into
  `tools/gate.sh`, the partition is printed in the header of the run, and each step's tokens
  and `-- FAIL` lines are buffered and concatenated in sorted name order so the summary line
  and the output order stay byte-identical while execution order changes.
- **The preserving condition is byte-identical output, and it is checked.** The plan requires
  the full `sh` and `dash` gate output captured before and after the change and compared, not
  just the final line.
- **Hosted cost is unchanged by construction** and the plan says so.

**Bookkeeping, settings and the rulesets**

- **The branch ruleset is applied, and the tag ruleset is applied in the only shape a ruleset
  can express.** Branch: PR required with 0 approvals, required status check `gate`, no
  up-to-date requirement, no force push, no deletion, no bypass list. Tag: target `tag`, ref
  `refs/tags/v*`, deletion and non-fast-forward refused. "Created only from a green `main`" is
  **not** expressible in a ruleset and is recorded as such.
- **`main` protection is already real and the record is corrected, not rewritten.**
- **Bookkeeping is one commit of its own.** The stale `D-31` `create_tag` text,
  `RELEASE-OPS.md`'s `create_tag: true` and "ten files", the stale composite-version comment in
  `core/src/machine/machine_digest.c`, the two bench golden commands into `docs/runner.md`,
  and the disposition of the seven draft `VALIDATION.md` files (by citing
  `08-ACCEPTED-DEBT.md` §1 in a live record — never a rewrite of an archive).
- **A disposition is a row, not a sentence.**

**Ordering, the hosted row and who does what**

- **The row is a gate on A2, not a milestone formality.** No test id, control, fault, registry
  line or gate step is committed before a hosted row recorded after the A1 commits exists, and
  the plan proves the ancestry with
  `git merge-base --is-ancestor <row-commit> <first-id-adding-commit>`.
- **Only the orchestrator pushes, opens the PR, records the row and touches repository
  settings.** An executor never calls `gh`, never pushes, and never stages
  `.planning/config.json` or `.planning/state.json`.
- **The row settles one of three words and no others.** `phaseA1: discharged-below-budget`
  (≤7.8), `discharged-within-line` (7.8–8.3) or `lever-required` (>8.3, possible only at the
  second row after `R1`).
- **A2 may spend at most the recorded allowance** (about 21 hosted cpu-s for the whole
  milestone, ≈12.9 local cpu-s at the recorded 1.675 factor). The allowance is a ceiling, not
  a target.

### Claude's Discretion

The plan may split A2 into more plans or fewer than the phase record suggests, and may order
the EV-03 items by cost. The plan may decide that the memmap generator control needs a
fixture the phase cannot build without a new step, in which case it records the deferral with
its reason. The plan chooses whether the skip vocabulary for a `release-only` scenario lives
in the tap script or in `tests-inventory.sh`, provided the id still prints one `-- SKIPPED`
line and is counted in `tests-skipped` exactly as before.

### Deferred Ideas (OUT OF SCOPE)

- Installing `ivs-run` or any image — D14, structurally blocked.
- A windowed frontend, a palette table, a second cartridge.
- A new gate step for any of these checks — forbidden this milestone.
- Trim (b) — declined by measurement, recorded in `CI-EFFICIENCY.md`.
- Running negative controls in parallel — a watching item, not landed in A1.

## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| EV-01 | The gate cost lever (3) and trims (a), (c) and (d) land in cost-only commits — with trim (b) declined by measurement — before any new test id, control, fault or gate step, and a hosted row recorded after them settles the cost level. | §Q1 (the fold's exact mechanics and the derivation judgement, verified to pass today on all four scenarios), §Q2 (each trim's exact edit and preserving condition), §Q8 (the recomputed median 7.696 and the allowance) |
| EV-02 | The 17 requirements whose named faults already run in every gate (FND-01..07, CPU-01..05, MEM-01..05) each have at least one cannot-pass-vacuously mechanism registered in `data/expected/important-gates.ids`, with the accepted-requirement set and all ten toy registries extended in the same commit and no new fault or test id. | §Q3 (a verified 24-row proposal), §Q4 (the three prose edits, the ten files, the control-retargeting answer) |
| EV-03 | Each gate-hardening item (`SRC-` whole-token match, zero-document failures, per-step pass lines, manifest output anchoring, memmap generator controls) has a control that fails for its intended reason. | §Q6 (per item: the guard, its file and line, whether it is landed, and whether a control can reach it) |
| EV-04 | A machine check refuses a requirement marked Complete that has no registry line, and refuses a toy registry that drifts from the good tree, and both are shown able to fail. | §Q5 (the records' exact parse, the 50-vs-27 gap, the purity-note amendment, the drift check's measurable one-deviation property) |

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Fold two live scenario launches into static repeats | Test suite (`tests/runner-process-tap.sh`) | Test inventory (`data/expected/tests/runner-process.ids` unchanged) | The mode switch is a suite decision, exactly as `mutant_case` already is (`tests/runner-process-tap.sh:532-563`) |
| Derivation-equality judgement | Test suite, file-only check | Frozen oracle (`docs/video/demo-lamp-tone-1-derivation.md`) | The derivation is the oracle; the judgement compares files and runs nothing, so it is legal on the dev preset |
| Trims (a), (c), (d) | Test suite (runner-process) and C test cases (`tests/state/*.c`) | — | Each trim edits the loop that makes the launches, never the assertion |
| Requirement→mechanism registry | Gate step (`tools/check/steps/important-gates.sh`) | Data (`data/expected/important-gates.ids`) | The step is a pure checker over `$ROOT/data/expected/`; the registry is data |
| Derived must-be-covered set | Gate step (`important-gates.sh`) reading `.planning/` | Requirement records (two files) | Reading `.planning/` from a step is already established by `ci-log.sh` and `glossary.sh` |
| Toy-registry drift check | Control runner (`tools/check/run-controls.sh`) | Toy trees (`tools/check/controls/*/overlay/`) | The runner owns the good tree and every overlay, so it is the only place that can compare them |
| Gate-hardening controls | Control runner + step scripts | `data/expected/controls/*.ids` | A control must name a guard in the script that runs it (lesson 7) |
| Preflight partition | Gate driver (`tools/gate.sh`) | — | One file owns step ordering and the summary line |
| Cost accounting | Orchestrator records (`.planning/ci-runs.tsv`, `CI-EFFICIENCY.md`) | — | Executors never touch these |

---

## Q1 — Lever (3): the fold, the mode switch, the judgement

### 1.1 The current shape of the four columns that matter

`data/expected/runner-scenarios.ids` is 47 lines. The header names six fixed columns
(`data/expected/runner-scenarios.ids:5-10`, verbatim):

```
#   class          deterministic, or host-dependent (wall time, signals): the golden step of plan
#   skip           run, or skip-ok: the scenario needs operator content the gate never has; it is
```

The two lines lever (3) moves are:

```
37:demo-lamp-tone-shift 0 pass deterministic run -
38:demo-lamp-tone-swap 0 pass deterministic run -
```

**Correction to the phase record's "Reusable Assets".** CONTEXT.md says
`data/expected/runner-scenarios.ids` "already has the column-5 mode vocabulary (`run`,
`release-only`)". It does not. Measured this session:
`grep -rn "release-only" tests/ data/ tools/check/steps/` returns **no match**; the only
occurrences in the repo are `tools/release.sh:19,91` and `docs/packaging.md:101`, all about
the release script, plus two `build/plan-08-21/*.md` notes that use the phrase for the
*existing* mutant behaviour. The vocabulary is `run | skip-ok` only
(`tests/runner-process-tap.sh:445`):

```sh
  case $skip in run | skip-ok) ;; *) ok=0; why inventory "skip '$skip'" ;; esac
```

So `release-only` is a **new** column-5 value, and adding it is an edit to that `case` line
(and to the header prose at lines 8-10), not a reuse. This does not change the plan's shape,
but it does mean the plan must not claim the mode already exists, and `tests-inventory.sh`
must not be expected to know it (it does not read column 5 at all — see §1.4).

### 1.2 The mechanism to copy: `mutant_case`

The exact pattern lever (3) needs already exists for mutants, added by plan 08-lever2. Its
docstring (`tests/runner-process-tap.sh:522-531`) and body (`:532-563`) are the template:

```sh
# mutant_case <id> <listing> <cart file name> <scenario id> <addr list> <to list> <exit> <outcome> <detail pattern> [stub] (plan 08-lever2, D-40):
# the arguments of run_mutant, and the one place that decides what a row of any mutant inventory costs. With the build configuration Release
# (the fifth argument of this script) it is run_mutant: the full run and the judgement of outcome, exit code and detail. In any other
# configuration (the sanitized Debug preset) it is the static judgement, which runs nothing: ...
mutant_case() {
  if [ "$config" = Release ]; then
    mut_full=$((mut_full + 1))
    run_mutant "$@"
    return 0
  fi
  mut_static=$((mut_static + 1))
```

`$config` is the script's optional fifth argument (`tests/runner-process-tap.sh:106`: `config=${5:-}`; usage line `:100`: `# Usage: sh runner-process-tap.sh <source root> <ivs-run> <ivs_pack> <images dir> [<build configuration>]`).
That is the *only* switch needed: `release-only` scenarios behave exactly like mutants — full
run on `Release`, static judgement otherwise. The header already states the invariant this
buys (`:88-89`):

```
# scenario: the ids are the same in both presets, no id is skip-ok and tests-skipped does not move (D-40).
```

### 1.3 How a skipped id is counted — and why `release-only` must NOT be a skip

Three separate mechanisms, and conflating them is the trap:

- **`tests-skipped`** counts `ok ... # SKIP <reason>` lines emitted by `result()` when it is
  called with a second argument (`tests/runner-process-tap.sh:420-437`). The suite counts its
  own `skipped=$((skipped + 1))` and writes a trailer; `tests-inventory.sh` re-counts from the
  TAP lines (`tools/check/steps/tests-inventory.sh:100-126`) and requires
  `tests-skipped=` to equal the number of `-- SKIPPED` lines it prints (`:176-182`).
- **`-- SKIPPED <preset>/<suite>/<id>: <reason>`** is one line per skipped id *and preset*,
  printed before the pass line, and its count equals `tests-skipped=`
  (`tools/check/steps/tests-inventory.sh:16-19` and `:176-183`).
- **`skip-ok`** in `data/expected/tests/<suite>.ids` (second token) is what *permits* a skip;
  a skip of an id not marked `skip-ok` is `tests-unexpected-skip`
  (`tools/check/steps/tests-inventory.sh:131-132`).

Therefore a `release-only` scenario must emit an ordinary `ok` line with **no** `# SKIP` on
the dev preset. If it emitted a skip, `tests-skipped` would move 4 → 8 and every recorded
token string in the tree would be stale — which the CONTEXT forbids ("no id is skip-ok and
tests-skipped does not move"). The judgement on the dev preset is a *static* judgement that
passes, exactly as `mutant_case` does.

### 1.4 The id set does not move, and the pair is the hard part

`data/expected/tests/runner-process.ids` lists the two scenario cases and the one pair:

```
60:runner-process.scenario.demo-lamp-tone-shift
61:runner-process.scenario.demo-lamp-tone-swap
137:runner-process.pair.demo-lamp-tone-pass.demo-lamp-tone-shift
```

`tests-inventory.sh` counts `tests-expected` as ids × presets (`:51-54`), and requires the
result id set to equal the inventory set on **both** presets (`:133-134`). So all three ids
must still be emitted on dev. That is fine for the two scenario cases (static judgement), but
the **pair** currently needs two reports (`check_pair`, `:374-380`):

```sh
  if [ ! -s "$_ra" ] || [ ! -s "$_rb" ]; then
    why pair-no-report "no report for $_pa or $_pb"
    return 1
  fi
```

On dev the shift report will not exist. The pair therefore needs the same two-mode switch:
inventory-only judgement on dev (both ids are `deterministic` lines of
`data/expected/runner-scenarios.ids`, neither key list is vacuous, `differ=-`+`equal=-` is
refused — all checks at `:915-931` that need no report), full metamorphic judgement on
`Release`.

The pair line is `demo-lamp-tone-pass demo-lamp-tone-shift differ=- equal=digest.video`
(`data/expected/scenario-pairs.ids:12`) and the loop rejects a fifth column
(`tests/runner-process-tap.sh:922`: `[ -z "$prest" ] || { ok=0; why inventory "more than four columns"; }`).
**Recommendation:** do *not* add a mode column to `scenario-pairs.ids`; derive the pair's mode
from the two scenario lines' column 5 in `$inv` (which the loop already greps, `:930-932`).
That keeps `scenario-pairs.ids` at four columns and satisfies "the scenario pair takes the same
mode switch" without a second vocabulary.

### 1.5 The derivation-equality judgement: it is well-founded and passes today

`docs/video/demo-lamp-tone-1-derivation.md` carries four marker pairs (lines 320/394, 398/434,
438/512, 516/578), each enclosing a fenced ```` ```text ```` block whose first lines are
`# frame: …` literals and whose remainder is the scenario body. The derivation states the
assert counts at `:316`: ``Counts: `pass` 55 asserts, `idle` 29, `shift` 55, `swap` 49.``

I ran the comparison this session. The rule that holds **exactly**, for all four scenarios, is:

> the scenario file's body — every line after the `ivss 1` line, minus the
> `id`/`profile`/`content`/`capture` header lines — equals, in order, the derivation block
> minus its fence lines and minus its **leading** `#` comment lines.

Verified output:

```
demo-lamp-tone-pass: EQUAL (      65 body lines, 55 asserts)
demo-lamp-tone-idle: EQUAL (      30 body lines, 29 asserts)
demo-lamp-tone-shift: EQUAL (      65 body lines, 55 asserts)
demo-lamp-tone-swap: EQUAL (      58 body lines, 49 asserts)
```

The command that produced it (the plan should reuse this shape rather than invent one):

```sh
awk -v id="$id" '/<!-- asserts-begin /{ b=($3==id); next } /<!-- asserts-end /{ if(b) b=0; next } b' \
  docs/video/demo-lamp-tone-1-derivation.md \
  | grep -v '^```' | awk 'BEGIN{s=1} s && /^#/{next} {s=0; print}' > /tmp/der.$id
awk '/^ivss 1$/{h=1; next} h' data/scenarios/$id.ivss \
  | grep -v -E '^(id|profile|content|capture) ' > /tmp/scn.$id
diff -q /tmp/der.$id /tmp/scn.$id
```

**Two things the plan must not get wrong here.** (i) The `id` line cannot "equal the pass
scenario's" — the shift file says `id demo-lamp-tone-shift` and the pass file says
`id demo-lamp-tone-pass`. The CONTEXT sentence is literally unsatisfiable; the workable
reading is "the `ivss`/`profile`/`content` lines equal the pass scenario's and the `id` line
equals `id <its own id>`". Record that as a deviation. (ii) The input transform is checkable
exactly: shift = pass ticks + 2000 (`100000→102000 … 700000→702000`), swap = `c0`/`c1`
exchanged, both as documented at `docs/video/demo-lamp-tone-1-derivation.md:436` and `:514`
(verbatim: `` `demo-lamp-tone-shift` (every event +2,000 ticks, the pass assert lines
unchanged; … `` and `` `demo-lamp-tone-swap` (each event's `c0` and `c1` exchanged; … ``).

**The one-value-differs stand-in** is therefore trivial and file-only: copy the scenario body
to a scratch file with one `assert mem 02F1 == 0003` changed to `== 0004`, run the judgement
on it, and require the finding token. The existing `self_control` helper is the right vehicle
(`tests/runner-process-tap.sh:1539-1552`):

```sh
# self_control <slug> <token> <check...>: runs the check; ok only when it fails with <token>.
self_control() {
  _slug=$1; _tok=$2; shift 2
  : > "$tmp/why"
  if "$@"; then
    ok=0
    : > "$tmp/why"
    why judge-vacuous "the check accepted the fake runner of $_slug"
  elif grep -q -e "^$_tok: " "$tmp/why"; then
```

### 1.6 The cost, and the scope hazard

Recorded ceiling (`CI-EFFICIENCY.md:1635`): "removes two dev launches = **0.12 local cpu-s
≈ 0.005 `r`**". The two launches are `demo-lamp-tone-shift` and `demo-lamp-tone-swap` on the
dev preset; the release preset keeps them, so the true saving is about 0.14 local cpu-s
(release `ivs-run` measured 9.4–9.8 ms). Quote the 0.12 as the ceiling.

**Scope hazard, verified:** no golden and no mutant depends on either scenario.
`grep -rn "demo-lamp-tone-shift\|demo-lamp-tone-swap" data/golden/ data/expected/ tools/ docs/`
returns only the scenario inventory, the tests inventory, the pairs inventory and the
derivation — no `data/golden/*`, and `data/expected/diag-mutations-scn.ids` uses only
`demo-lamp-tone-pass` and `demo-lamp-tone-idle` (13 rows, `awk '{print $1,$2}'`). So the fold
removes no golden and no mutant.

---

## Q2 — Trims (a), (c), (d): the exact edit and the preserving condition

### Trim (a) — `runner-process.roundtrip.refuse-range`

The four launches are `tests/runner-process-tap.sh:906-909`:

```sh
rt_refuse zero 'option value out of range' --roundtrip-at-ticks 0 "$scen/diag-boot-pass.ivss"
rt_refuse above-max 'option value out of range' --roundtrip-at-ticks 9223372036854775809 "$scen/diag-boot-pass.ivss"
rt_refuse no-value 'option without a value' "$scen/diag-boot-pass.ivss" --roundtrip-at-ticks
rt_refuse twice 'option given twice' --roundtrip-at-ticks 600 --roundtrip-at-ticks 700 "$scen/diag-boot-pass.ivss"
result runner-process.roundtrip.refuse-range
```

`rt_refuse` (`:898-904`) judges four things per launch: exit 14 and not hung and an empty
report, the first stderr line `ivs-run: <reason>`, the second line equal to
`usage: ivs-run [options] <scenario.ivss>`, and the usage text naming
`--roundtrip-at-ticks <n>`.

**The trim:** keep `rt_refuse zero …` as the one live launch (it is the only one that can
prove exit 14 + empty report + first stderr line + usage text together); judge `above-max`,
`no-value` and `twice` statically. "Statically against the option table" resolves concretely
to three file-only facts, each of which can fail:

1. the bound `9223372036854775808` is `IVS_RUN_TARGET_MAX 0x8000000000000000u`
   (`core/include/ivs/ivs_run.h:146`), so `9223372036854775809` is one above it — assert the
   literal in the scenario/inventory equals the decimal of that macro (the decimal string is
   already in the tap script; the check is that it parses to `2^63 + 1`);
2. `--roundtrip-at-ticks` requires a value — assert the option appears in the usage text
   (`grep -q -F -e '--roundtrip-at-ticks <n>'`), which the live `zero` launch already checks
   once, and assert the option's arity in `runner/exec.c`'s table (the `no-value` refusal
   comes from the same table);
3. `twice` — assert the "given twice" refusal exists as a row of the option table.

**Preserving condition:** the `zero` launch keeps all four judgements live; the three static
checks keep the *inventory-level* claim (the option is bounded, needs a value, refuses a
repeat) without a process spawn. **Reachability without touching an id:** the id
`runner-process.roundtrip.refuse-range` is one `result` call at `:910`, emitted on both
presets; the trim changes what happens *inside* the block, so the id's identity and its
inventory line (`data/expected/tests/runner-process.ids`) are untouched and
`tests-executed` does not move.

**Cost:** recorded at 0.06 local cpu-s each, 0.24 for the four on the dev preset
(`CI-EFFICIENCY.md:1507`). Trimming three on dev saves 0.18; if the static judgement also
replaces them on release the saving is slightly larger.

### Trim (c) — `state.restore.atomic-every-refusal`

The case is `tests/state/state_restore.c:1584-1596`:

```c
static int rs_atomic_every_refusal(void)
{
  uint32_t bad = 0u;
  size_t i;
  uint32_t produced = 0u;
  for (i = 0u; i < RS_FAMILIES; i++) {
    bad += rs_run_family(&rs_families[i], 1, 1, &produced);
  }
  if (!rs_args(&produced, 1)) {
    bad++;
  }
  return bad == 0u ? IVS_TEST_PASS : IVS_TEST_FAIL;
}
```

The third argument `1` is `digests`; it sets `u.digests = 1`
(`tests/state/state_restore.c:678-686`), which makes `su_call_row` compute the nine digests
after **every** row (`tests/state/state_util.c:482-490`):

```c
  if (u->digests) {
    if (!su_nine_digests(x->m, after) || memcmp(after, u->dig[w->tsel], sizeof after) != 0) {
```

**The trim:** replace the boolean `digests` with a per-row decision — compute the nine digests
for rows 0, `r % 8 == 0`, and `r == t->n - 1` — "every 8th refusal **plus the first and last
row**". The row index is already in scope in `su_run_rows` (`tests/state/state_util.c:496`, loop
at `:505`: `for (r = 0u; r < t->n; r++) {`) and is *not* passed to `su_call_row` (`:432`), so
the edit is one extra parameter (or a `u->row_i` field).

**Preserving condition, both halves, verbatim from the record:** "the whole-instance
comparison of every row stays" — that is the `memcmp` at
`tests/state/state_util.c:476-480`:
```c
  if (memcmp(x->storage, u->ref[w->tsel], IVS_MACHINE_BYTES) != 0) {
```
which is outside the `u->digests` branch (`:482`) and is untouched. "and `produced > 0` as the
liveness guard" — `u->produced |= 1u << (w->reason - 58u)` at `:491`, OR-ed across
families at `state_restore.c:684` and checked in `rs_args`. The trim must keep the `produced` accumulation
unconditional.

**Reachability:** the id is one entry of the case table
(`tests/state/state_restore.c:1767`: `{"state.restore.atomic-every-refusal", rs_atomic_every_refusal, NULL},`)
and one line of `data/expected/tests/state.ids`; the edit is inside the case's own call, so
neither moves and `tests-executed` does not move. **Cost:** about 0.2 local cpu-s per preset,
0.4 together (`CI-EFFICIENCY.md:1446`, `:1507`); the case was measured at 0.46 dev / 0.24
release in isolation (`:1434`).

### Trim (d) — `state.fuzz.byte-flips`

The unsealed sweep is `tests/state/state_fuzz.c:915-936`:

```c
static void fz_run_flips(fz_ctx *c, const fz_plan *p, int sealed)
{
  static const uint8_t masks[2] = {0xFFu, 0x01u};
  uint32_t k;
  for (k = 0u; k < (uint32_t)fz.n; k++) {
    uint32_t j;
    if (!fz_pick(p, k)) {
      continue;
    }
    for (j = 0u; j < 2u; j++) {
      if (sealed && j == 1u && … fz.row_free[fz.pk_row[k]]) {
        continue;
      }
```

**The trim:** for `sealed == 0`, take `j == 0` only (XOR 0xFF), i.e. add
`if (!sealed && j == 1u) { continue; }` beside the two existing `sealed &&` guards. The
offset loop bound (`k < fz.n`), `fz_pick` and `masks[]` are untouched, so "the offset set
unchanged" holds by construction. `fz_byte_flips` (`:937-950`) is called with `sealed = 0`
and `fz_full` (`{0u, 1u, 0}` at `:826`, i.e. stride 1, no fault thinning).

**The "flip moves the digest" guard — an honest note.** No mechanism by that name exists in
the tree; `grep -n "moves the digest" tests/state/state_fuzz.c` returns nothing. The guard
that does the work is the acceptance refusal at `:945-948`:

```c
  if (c.accepted != 0u) {
    ivs_test_diag("%s: a flip with the trailer left as it was was accepted", c.name);
    ok = IVS_TEST_FAIL;
  }
```

which *is* the "the flip moves the digest" property in operational form: with the trailer not
resealed, a payload byte the digest does not cover would be accepted and the id fails. The
plan should say that in one sentence rather than inventing a new guard. **Cost:** about 0.2
local cpu-s per preset, 0.4 together (`CI-EFFICIENCY.md:1507`).

**Reachability:** `state.fuzz.byte-flips` is one case id in `tests/state/state_fuzz.c`'s table
and one line of `data/expected/tests/state.ids`; nothing moves.

### Trim (b) — the decline

Recorded: 0.09 local cpu-s (0.03 % of a 219 cpu-s gate) against "the only sanitized execution
of the 65-cut loop" (`CI-EFFICIENCY.md:1636`). The plan records the decline as one line in
`.planning/CI-EFFICIENCY.md`; it does not appear in any file list.

---

## Q3 — EV-02: a verified 24-row proposal

### 3.1 The inventories, measured

| Inventory | Mechanism token | Ids | Command |
|-----------|-----------------|-----|---------|
| `data/expected/faults/*.ids` (12 files) | `fault` | **235** | `awk '!/^#/ && NF' data/expected/faults/*.ids \| wc -l` |
| `data/expected/controls/*.ids` (37 files) | `control` | **380** | same over `controls/*.ids` (and 380 control dirs, `ls -d tools/check/controls/*/ \| grep -v _good \| wc -l`) |
| `data/expected/pkg-selftests.ids` | `mutant` | 19 | `awk '{print $1}'` |
| `data/expected/ci-summary-selftests.ids` | `mutant` | 11 | same |
| `data/expected/tests/*.ids`, ids matching `^[a-z0-9-]+[.](judge\|mutant)[.][^ ]+$` | `mutant` | 81 | 35 `runner-process.mutant`, 10 `runner-process.judge`, 8 `video.judge`, 8 `psg.judge`, 6 `state.judge`, 4 `stic.judge`, 3 `runner.judge`, 2 `input.judge`, 2 `asm.judge`, and one each of `pack.judge`, `bench.mutant`, `bench.judge` |

The step resolves exactly these five sources — the four `for` loops at
`tools/check/steps/important-gates.sh:57-78`:

```sh
for f in "$exp"/faults/*.ids; do
  awk '/^#/ { next } NF { print "fault", $1 }' "$f" >> "$tmp/known"
...
for f in "$exp"/tests/*.ids; do
  awk '/^#/ { next } $1 ~ /^[a-z0-9-]+[.](judge|mutant)[.][^ ]+$/ { print "mutant", $1 }' "$f" >> "$tmp/known"
```

### 3.2 The 24 rows

Every id below was verified to exist in its inventory this session by rebuilding
`$tmp/known` with the step's own five loops and `grep -qx -F -e "<mech> <id>"` for each row —
50 candidates checked, 0 missing. Distribution: 7 requirements carry two mechanisms, 10 carry
one (7×2 + 10×1 = 24).

| # | `<gate>` | mechanism | `<id>` | REQ |
|---|----------|-----------|--------|-----|
| 1 | `profile-record` | control | `profile-missing-key` | FND-01 |
| 2 | `profile-record` | control | `profile-tier5-tag` | FND-01 |
| 3 | `claim-ledger` | control | `claims-unknown-tier` | FND-02 |
| 4 | `claim-ledger` | control | `claims-overstated-standing` | FND-02 |
| 5 | `discrepancy-log` | control | `discrepancies-missing-c17` | FND-03 |
| 6 | `fixture-manifest` | control | `manifests-digest-mismatch` | FND-04 |
| 7 | `fixture-manifest` | control | `manifests-recipe-source-digest` | FND-04 |
| 8 | `provenance-rule` | control | `denylist-hit` | FND-05 |
| 9 | `cp1610-reference-notes` | control | `cp1610-row-without-cite` | FND-06 |
| 10 | `time-base-vocabulary` | control | `glossary-bare-clock` | FND-07 |
| 11 | `cpu-opcode-expectations` | fault | `ADD_OVERFLOW` | CPU-01 |
| 12 | `cpu-opcode-expectations` | fault | `GEN_EXPECT_MUTATION` | CPU-01 |
| 13 | `cpu-interrupt-model` | fault | `INT_ENTRY_ORDER` | CPU-02 |
| 14 | `cpu-interrupt-model` | fault | `BUSRQ_PRIORITY` | CPU-02 |
| 15 | `cpu-sdbd` | fault | `SDBD_DBD_NOT_CLEARED` | CPU-03 |
| 16 | `cpu-stack-and-reset` | fault | `PUSH_DIRECTION` | CPU-04 |
| 17 | `cpu-bus-trace` | fault | `TCI_STROBE_STAMP` | CPU-05 |
| 18 | `memmap-profile-generated` | fault | `GEN_MAP_MUTATION` | MEM-01 |
| 19 | `memmap-profile-generated` | control | `memmap-literal-orphan` | MEM-01 |
| 20 | `memmap-width-masking` | fault | `MEM_ROM_NO_MASK` | MEM-02 |
| 21 | `memmap-width-masking` | fault | `MEM_RAM8_NO_TRUNC` | MEM-02 |
| 22 | `content-loading` | fault | `LOAD_TABLE_BEFORE_VALIDATE` | MEM-03 |
| 23 | `content-refusal-order` | fault | `CHECK_ORDER_MALFORMED_FIRST` | MEM-04 |
| 24 | `ivsc-import-format` | fault | `IVSC_TRUST_TOTAL_BYTES` | MEM-05 |

All 17 gate names are new: the existing 90 names contain no `cpu*`, no `memmap*` except
`memmap-flip`, and no `machine`/`ivsc` family (`awk '{print $1}' | sort -u`). So the predicted
delta is **`gates=90 → 107`**. If the planner prefers no gate-count delta, reusing existing
family names keeps `gates=90`; state whichever is chosen in the prediction.

### 3.3 Why each cannot pass vacuously — one sentence per requirement

A **fault** cannot pass vacuously because the gate mutates the implementation at a named site
and the suite must observe the mutation (a surviving mutation fails `faults-verdict`). A
**control** cannot pass vacuously because `run-controls.sh` requires the step to *exit
non-zero* and to print the pinned token (`tools/check/run-controls.sh:189-195`).

- **FND-01** — `profile-missing-key` overlays a profile record with a required key absent, and
  `profile-tier5-tag` overlays a tier tag outside the vocabulary; the step must refuse each,
  so a profile record that is not actually read fails.
- **FND-02** — `claims-unknown-tier` names a tier that is not in the evidence-tier vocabulary
  and `claims-overstated-standing` raises a claim above its recorded tier; a ledger that is
  not consulted cannot refuse either.
- **FND-03** — `discrepancies-missing-c17` removes the conflict-resolution link from a
  discrepancy row; a log that is only prose passes and the control fails.
- **FND-04** — `manifests-digest-mismatch` changes a manifest's recorded digest and
  `manifests-recipe-source-digest` breaks the recipe→source digest chain; either makes a
  manifest a decoration rather than a record.
- **FND-05** — `denylist-hit` plants a copied constant in an in-scope file; the provenance rule
  is enforced by a scan, not by review prose, only if this fails.
- **FND-06** — `cp1610-row-without-cite` removes the source citation from one row of the
  CP-1610 table; the reference notes are project-owned only if every row is cited.
- **FND-07** — `glossary-bare-clock` writes a bare `clock` where the time-base vocabulary
  requires a qualified unit; a defined vocabulary that is not enforced is a document.
- **CPU-01** — `ADD_OVERFLOW` removes the overflow flag from ADD and `GEN_EXPECT_MUTATION`
  mutates the generated expectation table; the first proves the ALU is compared, the second
  proves the expectation table is read at all (an unread table makes every opcode case pass).
- **CPU-02** — `INT_ENTRY_ORDER` reorders the interrupt entry sequence and `BUSRQ_PRIORITY`
  inverts the BUSRQ/BUSAK priority; an acceptance model that is not executed cannot observe
  either.
- **CPU-03** — `SDBD_DBD_NOT_CLEARED` leaves the double-byte-data flag set after an SDBD
  immediate; the SDBD behaviour is compared, not asserted in prose.
- **CPU-04** — `PUSH_DIRECTION` reverses the stack growth direction; the vector suite must
  fail, so stack direction is a measured fact.
- **CPU-05** — `TCI_STROBE_STAMP` drops the cycle stamp from the bus accesses the stepper
  reports; a stepper whose trace is not read cannot show this.
- **MEM-01** — `GEN_MAP_MUTATION` mutates the generated map from the profile descriptors and
  `memmap-literal-orphan` adds a literal map file with no profile; the map is generated from
  data and the data is checked, not asserted.
- **MEM-02** — `MEM_ROM_NO_MASK` stops masking 10-bit ROM reads and `MEM_RAM8_NO_TRUNC` stops
  truncating a 16-bit store into an 8-bit cell; both are width behaviour, so a width claim with
  no failing mutation is a claim about code shape.
- **MEM-03** — `LOAD_TABLE_BEFORE_VALIDATE` publishes the cartridge segment table before the
  segment is validated; typed loading is proven by the refusal of an unvalidated load.
- **MEM-04** — `CHECK_ORDER_MALFORMED_FIRST` swaps the malformed-bytes check with a later one;
  the refusal *order* is the requirement, and only a mutation of the order can test it.
- **MEM-05** — `IVSC_TRUST_TOTAL_BYTES` trusts the container's declared total byte count; the
  documented import format is only enforced if a container that lies about its size is
  refused.

### 3.4 The ids that must NOT be proposed

`GEN_EXPECT_MUTATION` and `GEN_MAP_MUTATION` are generator faults, not test ids — they are in
`data/expected/faults/cpu.ids` and `data/expected/faults/memmap.ids` first column, so the
`fault` mechanism resolves them. Do not propose `cp1610-vectors`-suite judge ids for CPU-01:
that suite's controls are `control`-kind (`data/expected/controls/cp1610-vectors.ids`, 40), and
the registry would then name a control whose CHECK is `cp1610-vectors`, which is legal but
duplicates #9's family.

---

## Q4 — EV-02 mechanics: three prose edits, ten files, and which controls break

### 4.1 The three edits in `tools/check/steps/important-gates.sh` (111 lines)

1. **The `nreq` split string**, line 86, verbatim:
   ```
       nreq = split("API-02 API-04 RUN-01 RUN-02 RUN-04 RUN-05 RUN-06 PKG-01 PKG-03 PKG-04 INP-01 INP-02 INP-03 API-03 AUD-01 AUD-02 VID-01 VID-02 VID-03 VID-04 VID-05 VID-06 SCN-01 SCN-02 STA-01 STA-02 PRF-01", reqs, " ")
   ```
   → append ` FND-01 … FND-07 CPU-01 … CPU-05 MEM-01 … MEM-05` (17 ids). This array is both
   `accepted[]` and the must-be-covered set (`:87-88`, and `END` at `:106`).

2. **The header comment**, lines 4-8 and 20-30. Line 4 says the step "covers every Phase 4,
   Phase 5, Phase 6, Phase 7 and Phase 8 requirement", and lines 5-8 enumerate which plan
   added which phase. Both must gain Phase 1/2/3 (FND/CPU/MEM are Phase 1/2/3 requirements
   per `.planning/milestones/v0.1.0-REQUIREMENTS.md`'s traceability table). **This is exactly
   the defect the phase exists to remove**: a header that lists Phases 4–8 and omits 1–3.

3. **The token prose**, line 38, verbatim:
   ```
   #   or a requirement outside the Phase 4, Phase 5, Phase 6, Phase 7 and Phase 8 sets) or <mechanism> <id> line <n> (the id
   ```
   and line 39-40 (`important-gates-uncovered-gate: <REQ> (a Phase 4, Phase 5, Phase 6, Phase 7 or
   Phase 8 requirement with no registry line; also the registry file missing).`). Both
   sentences must name the new phases or drop the enumeration.

Plus the amendment of the purity note (see §Q5.3) — a fourth edit in the same commit if EV-04
lands with EV-02.

### 4.2 The ten toy registries — measured, and the retargeting answer

Exactly ten files, verified:

```
tools/check/controls/_good/tree/data/expected/important-gates.ids            (37 lines, 27 non-comment)
tools/check/controls/important-gates-bad-req-phase5/overlay/...              (35 lines)
tools/check/controls/important-gates-bad-req-phase6/overlay/...              (34 lines)
tools/check/controls/important-gates-req-uncovered-phase5/overlay/...        (33 lines)
tools/check/controls/important-gates-req-uncovered-phase6/overlay/...        (32 lines)
tools/check/controls/important-gates-req-uncovered-phase7/overlay/...        (33 lines)
tools/check/controls/important-gates-uncovered-gate/overlay/...              (34 lines)
tools/check/controls/important-gates-unknown-id-phase5/overlay/...           (35 lines)
tools/check/controls/important-gates-unknown-id-phase6/overlay/...           (34 lines)
tools/check/controls/important-gates-unknown-id/overlay/...                  (36 lines)
```

Every one is a **whole-file overlay**, not a MAKE transform (`ls tools/check/controls/important-gates-*/MAKE`
finds nothing). Each overlay replaces the good tree's 4-line header with its own 2-line header
and then carries the good tree's body.

**`_good` must move, and this is the hard dependency.** `run-controls.sh`'s positive control
requires the step to pass on the pristine good tree and to print its pass line
(`tools/check/run-controls.sh:156-165`):

```sh
  if run_step "$2" "$1" && grep -q -F -e "-- $1 pass" "$2/out"; then
```

so a good-tree toy registry without the 17 lines fails `important-gates-uncovered-gate` for 17
requirements and the whole controls run goes red. Every other toy registry must move with it,
because R2 says so and because the drift check (§Q5.4) will demand it.

**Which `important-gates-bad-req-*` control must be retargeted? None.** The two are:

- `important-gates-bad-req-phase5`, EXPECT `FAIL important-gates-unknown-id: line 19`, added
  line `toy-input fault GOOD_FAULT INP-04`;
- `important-gates-bad-req-phase6`, EXPECT `FAIL important-gates-unknown-id: line 24`, added
  line `toy-video fault GOOD_FAULT VID-99`.

`INP-04` and `VID-99` are outside both the old and the new accepted sets, so neither becomes
valid. **The retargeting clause of R2 is vacuously satisfied — say so in the plan rather than
silently omitting it.** The real hazard is the *line numbers* in those two EXPECT files: they
are only stable if the 17 new lines are **appended at the end** of each toy file, after the
existing last line (`toy-prf01 fault GOOD_FAULT PRF-01`, line 37 of `_good`). Appending at the
end keeps line 19 and line 24 intact and keeps each overlay's 2-line header untouched.

**How a control declares its overlay and token** (`tools/check/run-controls.sh:4-6` and
`:127-148`): a control directory holds `CHECK` (one step name), `EXPECT` (one token per line,
matched as a fixed substring of an output line), optional `FORBID`, optional `overlay/`
(copied over a fresh copy of `_good/tree`) and optional `MAKE` (line transforms). `run_step`
invokes `sh "$CHECK_DIR/steps/$2.sh"` with `ROOT=<work tree>`, `FILELIST=<work tree filelist>`,
`CHECK_DIR=<real tools/check>`, `IVS_GATE_DIR=<work tree>/build/gate`.

### 4.3 The predicted token deltas for the EV-02 commit

Measured baseline from the last recorded local run, `build/g9-final-sh.log:512` (tail):
`steps=38/38 controls=380/380 … gates=90 lines=306 … tests-expected=4958 tests-executed=4954
tests-skipped=4 tests-missing=0`.

| Token | Before | After EV-02 | Why |
|-------|--------|-------------|-----|
| `steps` | 38/38 | 38/38 | no new step (GUARDRAILS §2.1) |
| `controls` | 380/380 | 380/380 | no new control dir |
| `faults` | 235/235 | 235/235 | no new fault site |
| `gates` | 90 | **107** | 17 new gate names |
| `lines` | 306 | **330** | +24, −0 |
| `tests-expected` | 4958 | 4958 | no new test id |
| `tests-executed` | 4954 | 4954 | — |
| `tests-skipped` | 4 | 4 | — |
| `claims` | 28 | 28 | no claim moves |

---

## Q5 — EV-04: the derivation, the purity note, the drift check

### 5.1 What "marked Complete" parses to — and the 50-vs-27 gap

Both records carry the same machine-readable shape: a `## Traceability` table whose rows are
`| <REQ-ID> | Phase <n> | <Status> |`. `.planning/REQUIREMENTS.md` ends with 10 rows, all
`Pending` (verbatim from the file):

```
| EV-01 | Phase 9 | Pending |
…
| FMT-04 | Phase 12 | Pending |
```

`.planning/milestones/v0.1.0-REQUIREMENTS.md` has 50 rows, 49 `Complete` and one
`Complete (stage 2 recorded)` (`:242`: `| REL-01 | Phase 8 | Complete (stage 2 recorded) |`).

**Robust parse:** match `^\| *[A-Z]+-[0-9]+ *\|`, take field 2 as the id and field 4 as the
status, and treat `Complete*` (prefix) as Complete. Do **not** parse the `- [x] **REQ-NN**:`
checkbox lines: they exist in both files but the traceability table is the table the project
already treats as authoritative ("Filled at roadmap creation … verified against the
`.planning/ROADMAP.md` Phase Details on the same date").

**The gap, measured:**

```
$ awk -F'|' '/^\| *[A-Z]+-[0-9]+ *\|/ {gsub(/ /,"",$2); gsub(/ /,"",$4); if ($4 ~ /^Complete/) print $2}' \
    .planning/milestones/v0.1.0-REQUIREMENTS.md | sort > /tmp/complete.txt
$ awk '!/^#/ && NF {print $4}' data/expected/important-gates.ids | sort -u > /tmp/registered.txt
$ comm -23 /tmp/complete.txt /tmp/registered.txt
API-01 API-05 API-06 CPU-01 CPU-02 CPU-03 CPU-04 CPU-05 FND-01 FND-02 FND-03 FND-04 FND-05 FND-06
FND-07 MEM-01 MEM-02 MEM-03 MEM-04 MEM-05 PKG-02 REL-01 RUN-03
$ comm -23 /tmp/complete.txt /tmp/registered.txt | wc -l
      23
```

23 unregistered Complete requirements: EV-02's 17 plus `API-01 API-05 API-06 RUN-03 PKG-02
REL-01`. **A derivation with no exclusion record fails the good tree immediately** and the
whole gate goes red. This is the single most consequential finding in this research.

**Three viable shapes; the plan must pick one and state the cost:**

| Shape | What lands | Cost | Risk |
|-------|-----------|------|------|
| **S1 — exclusion record** (recommended) | `data/expected/important-gates-excluded.ids` with `<REQ> <reason>`, 6 rows; the step refuses a Complete requirement that is neither registered nor excluded | one data file + one awk branch in the step + one control | an exclusion is a visible, reviewable line; the property "deleting a registry line cannot shrink coverage" still holds because the exclusion set is a *different* file |
| **S2 — grow EV-02 to 23 requirements / 30 lines** | 6 more rows: `API-01` (`machine`/`CREATE_MAGIC_BEFORE_PROFILE_CHECK`), `API-05` (`c-purity` control), `API-06` (`api-effects` control), `RUN-03` (`tests-inventory` control), `PKG-02` (`ci-parity` control), `REL-01` (no gate mechanism exists — it is a close-record requirement) | 6 more lines, `lines=336`, plus one requirement with no mechanism | REL-01 has no gate evidence at all, so S2 cannot be completed honestly |
| **S3 — derive only from `.planning/REQUIREMENTS.md`** | 10 Pending rows, so the set is empty and the check is vacuous | 0 | a vacuous check, forbidden by lesson 20 and GUARDRAILS §2.3 |

S1 is the only shape that is both non-vacuous and honest. It also gives EV-04's control
something to point at: overlay `.planning/REQUIREMENTS.md` with one row flipped from `Pending`
to `Complete` and no registry line → the step must fail with the new token.

### 5.2 Where the good tree's `.planning/` copy lives, and what must be added

`tools/check/controls/_good/tree` holds 120 files; its `.planning/` holds exactly two:

```
tools/check/controls/_good/tree/.planning/PROJECT.md
tools/check/controls/_good/tree/.planning/ci-runs.tsv
```

So EV-04 must **add** `.../.planning/REQUIREMENTS.md` and
`.../.planning/milestones/v0.1.0-REQUIREMENTS.md` to the good tree, plus (under S1)
`.../data/expected/important-gates-excluded.ids`. Two consequences the plan must handle:

1. A copied real record in the good tree is a drift hazard. Either the toy records are
   *minimal* (listing exactly the 44 ids the toy registry covers, all Complete) or the drift
   check of §Q5.4 must be extended to them. Recommend minimal toy records — they are then
   consistent by construction with the toy registry's 44 lines.
2. The toy `.planning/` records must be *self-consistent* with the toy registry for the
   `_good` positive control to pass. With minimal records this is automatic; with copies of the
   real records it is not (the toy registry has toy gate names and toy ids but real REQ ids —
   that part is fine, since only the REQ id set matters).

### 5.3 The purity note that must be amended (verbatim)

`tools/check/steps/important-gates.sh:3-4`, verbatim:

```
# Step important-gates: the machine-checked form of RUN-04 (D-46, plan 04-24). Pure checker: reads
# only $ROOT/data/expected/, never builds or runs anything, so the control runner can drive it.
```

After EV-04 the step also reads `$ROOT/.planning/`. The amendment is one clause, and it is
*already true of two other steps* — quote them as precedent:

- `tools/check/steps/ci-log.sh:3` and `:24`:
  ```
  # Step ci-log: format check of the hosted run record .planning/ci-runs.tsv (PKG-04, D-44).
  rec="$ROOT/.planning/ci-runs.tsv"
  ```
- `tools/check/steps/glossary.sh:64-66`:
  ```
  proj="$ROOT/.planning/PROJECT.md"
  …
    bad glossary-d2-unlinked ".planning/PROJECT.md"
  ```

So the amendment reads "reads only `$ROOT/data/expected/` and the requirement records under
`$ROOT/.planning/`, never builds or runs anything". Note that `ci-log.sh` and `glossary.sh`
are both **preflight** steps by the §Q7 predicate, so reading `.planning/` does not disturb the
partition.

### 5.4 The toy-registry drift check in `run-controls.sh`

**The property to enforce, measured:** every overlay's **non-comment** content equals the good
tree's non-comment content plus exactly one line, or minus exactly one line. Verified for all
nine:

```
$ for d in important-gates-*/; do diff <(grep -v '^#' _good/tree/…/important-gates.ids | sort) \
      <(grep -v '^#' $d/overlay/…/important-gates.ids | sort) | grep '^[<>]'; done
bad-req-phase5              > toy-input fault GOOD_FAULT INP-04
bad-req-phase6              > toy-video fault GOOD_FAULT VID-99
req-uncovered-phase5        < toy-log fault GOOD_RUNNER AUD-02
req-uncovered-phase6        < toy-inspect mutant toy-selftest VID-06
req-uncovered-phase7        < toy-scene control toy-control SCN-01
uncovered-gate              < toy-log control toy-control PKG-04
unknown-id-phase5           > toy-input control NO_SUCH_CONTROL_P5 INP-01
unknown-id-phase6           > toy-input control NO_SUCH_CONTROL_P6 VID-01
unknown-id                < > toy-bound fault NO_SUCH_FAULT API-02
```

(exactly one `<` or `>` line per overlay; the last row shows one addition.) **Comments must be
excluded** — each overlay replaces the 4-line header with its own 2-line header by design, so
a comment-inclusive comparison would fire on all nine.

**Recommended declaration format:** a `DEVIATION` file per control, one line per permitted
difference, `<` or `>` followed by the exact line. `run-controls.sh` recomputes
`diff <(sort good-noncomment) <(sort toy-noncomment)` and refuses when any diff line is not
declared, or when a declaration matches no diff line (both directions, so the file cannot rot).
That satisfies "a toy registry that drifts from the good tree is refused" and it enforces R2
mechanically: appending 17 lines to `_good` without appending them to an overlay produces 17
undeclared `<` lines.

**The stand-in** is one more control: an overlay that drops a line the good tree has (e.g. the
`AUD-02` line) with **no** `DEVIATION` entry → the drift check must fire. That is cheaper than
a whole new file and reuses the existing overlay mechanism.

**Where it runs.** The drift check belongs beside the control/inventory consistency block
(`tools/check/run-controls.sh:249-268`), which already does exactly this shape of comparison
(`comm -13 "$tmp/ctl.want" "$tmp/ctl.have"` → `control-not-in-inventory`, and the reverse →
`control-inventory-orphan`). Two new tokens, e.g. `control-toy-drift <control>` and
`control-deviation-unused <control>`, plus their lines in `data/expected/controls/gate-hardening.ids`
(or a new `run-controls.ids`) so the tokens themselves are covered.

---

## Q6 — EV-03: the five items, located, with reachability

The five items are the Phase 1 review's **IN-01 to IN-04** plus the memmap generator. The
mapping is explicit in `.planning/milestones/v0.1.0-phases/01-evidence-profile-and-rights-foundations/01-REVIEW.md:218-266`
and restated in `.planning/STATE.md:505` and
`.planning/milestones/v0.1.0-phases/08-…/08-ACCEPTED-DEBT.md` §2. **Two of the five are
already landed**, and the phase must say so rather than re-doing them.

### Item 1 — whole-token `SRC-` match (IN-01). NOT landed.

Guard: `tools/check/steps/sources.sh:40`:
```sh
  grep -a -o -E 'SRC-[a-z0-9-]+' "$ROOT/$f" >> "$tmp/refs"
```
The review reproduces the defect exactly: `SRC-bsr-2609_nope` truncates to the registered
`SRC-bsr-2609` and passes; `SRC-Nope-Doc` and `SRC-Gi-1980` produce no match at all. Fix:
`SRC-[A-Za-z0-9_.-]*[A-Za-z0-9]` and fail any token that is not in `data/expected/sources.ids`
(the resolution loop already uses `grep -q -x -F`, `sources.sh:43`, so only the extraction changes).

Control: overlay one scanned `docs/*.md` file with `SRC-bsr-2609_nope`; intended reason
`source-unknown-ref: SRC-bsr-2609_nope`. Pure step (`sources` is preflight by the §Q7
predicate) — **reachable**. Pin the token in `data/expected/controls/gate-hardening.ids`.

### Item 2 — zero-document failures (IN-02). **Already landed.**

- `tools/check/steps/glossary.sh:48`: `bad glossary-no-files "README.md docs/"`
- `tools/check/steps/sources.sh:33-35`: `printf -- '-- FAIL sources-no-files: data/\n'`

Both already have controls, in `tools/check/controls/gate-hardening.ids`:
`glossary-no-files` (EXPECT `FAIL glossary-no-files:`) and `sources-no-files` (EXPECT
`FAIL sources-no-files:`). So EV-03's item 2 needs **verification, not construction**: the
plan should state the guard, cite the existing control, and record the item closed — or, if
the intent is the *remaining* scanning steps, extend to the two that still lack a control:
`denylist-no-files` (`tools/check/steps/denylist.sh:43`) and `runner-scan-empty`
(`tools/check/steps/runner-scan.sh:58`) — both currently have **no** control (measured:
`grep -rl -- "denylist-no-files" tools/check/controls/*/EXPECT` → 0; same for
`runner-scan-empty`). Both are reachable with a `MAKE delete` of the in-scope files.

### Item 3 — per-step pass lines (IN-04). **Guard landed; NOT control-reachable.**

Guard: `tools/gate.sh:111-113`:
```sh
    if grep -q -F -e "-- $name pass" "$GATE_TMP/step.out"; then
      passed=$((passed + 1))
      toks=$(sed -n "s/^-- $name pass //p" "$GATE_TMP/step.out" | tr '\n' ' ')
```
with the refusal at `:116`: `bad gate-step-no-pass-line "$name"`.

**A control cannot reach it.** `run-controls.sh` runs `sh "$CHECK_DIR/steps/$name.sh"` where
`CHECK_DIR` is the *real* `tools/check` (`:150-154`), and `CHECK` must name a file in
`tools/check/steps/` (`:168-175`). `tools/gate.sh` is not in `tools/check/steps/`, and the good
tree carries no `tools/gate.sh` and no `tools/check/` at all (verified: `ls
tools/check/controls/_good/tree/tools/` → `asm pcm`). A `CHECK gate` control would also
recurse (gate.sh starts `run-controls.sh`).

Options, with cost:

| Option | Mechanism | Cost | Verdict |
|--------|-----------|------|---------|
| **O1** | a committed scratch stand-in *outside* `tools/check/controls/`: a tiny shell script that pipes a step-like output through the same `grep -q -F -e "-- $name pass"` and requires the refusal | ~0 | not a control; does not satisfy "each item has a control" literally |
| **O2** | harden the *step side* instead: require each step's pass line to be the last non-empty line, and check it in `run-controls.sh`'s `good_one` (which already greps the pass line, `:158`) | ~0.2 local cpu-s (no new run) | gives a reachable guard, but it is a positive-control guard, not a negative control |
| **O3** | add a `CHECK gate` mode to `run-controls.sh` with an overlay carrying a minimal `tools/gate.sh` + a one-step `tools/check/steps/` | a whole second gate shape; recursion; unbounded cost | reject |

**Recommendation: O2, recorded as a deviation** — EV-03 item 3 becomes "every step's pass line
is its last non-empty line, enforced in `run-controls.sh`'s `good_one`", with a control whose
step prints its pass line before a `#` comment. State plainly that the `gate-step-no-pass-line`
branch in `tools/gate.sh` remains uncontrol-reachable and is covered by the definition-of-done
gate run instead. This is the honest resolution; a hollow control is worse.

### Item 4 — manifest output anchoring (IN-03). NOT landed.

Guard today: `tools/check/manifests.cmake:88-93` only checks charset, no leading `/`, no `..`:
```cmake
function(path_ok out p)
  if(p MATCHES "${PATH_RE}" AND NOT p MATCHES "^/" AND NOT p MATCHES "(^|/)[.][.](/|$)")
```
`check_entries` (`:97-131`) calls it for `outputs`, `notices` and `sources` but has no notion of
the manifest's own directory. Fix per the review: require `^fixtures/<dirname>/` for those
three arrays. `dirname` is available at the call site (`check_entries("${lbl}" outputs "${doc}")`,
`:240`; the manifest id/dirname check is `:184-186`).

Control: overlay `fixtures/FX-good/manifest.json` with an output path `docs/other.pdf`; intended
reason `manifest-format: fixtures/FX-good/outputs/path`. `manifests` is a **preflight** step
(§Q7) — **reachable**. Ordering caveat: the anchoring refusal must be raised before the digest
comparison, or a second token fires and the EXPECT is ambiguous.

### Item 5 — memmap generator controls. NOT landed, but **reachable without a new step**.

The generator is `cmake/IvsMemmapProbes.cmake`, a standalone script with a documented
invocation and fail-closed messages (`:5`, `:14-19`), verbatim:

```
# Usage: cmake -DROOT=<repo> -DOUT=<build>/tests/ivs_memmap_probes.inc -P cmake/IvsMemmapProbes.cmake
# Fail closed (message `IvsMemmapProbes: <file>[:<line>]: <reason>`): no literal file; a row that
# does not match the grammar of the literal header; LO above HI; a device outside the literal
# vocabulary; a width above 16; a mirror that is not a power of two; a view that does not tile
# 0000-FFFF exactly once.
```

No gate step runs it: it is invoked at CMake configure time
(`tests/CMakeLists.txt:87`: `set(IVS_MMPROBE_GENERATOR "${PROJECT_SOURCE_DIR}/cmake/IvsMemmapProbes.cmake")`).
`tools/check/steps/memmap.sh` runs a *different* script (`tools/check/memmap.cmake`) and is
documented as writing nothing.

**Reachability:** add one invocation to `tools/check/steps/memmap.sh` writing `-DOUT` into the
step's own `mktemp -d` directory, and map its exit status to one new token (e.g.
`memmap-generator-refused: <reason>`). That is inside an existing step, so no new gate step
(GUARDRAILS §2.1 holds) and `steps=38/38` does not move. `memmap` is a **producer** step by the
§Q7 predicate (it references `build/`), so the added cost does not disturb the preflight
partition. Cost: one `cmake -P` launch per gate ≈ 0.15–0.3 s local, one time (not per preset).

**Fixture:** a control that overlays `data/memmap/<profile>.literal.txt` with a row whose `LO`
is above its `HI`, or a `mirror=3`. The generator's `mp_fail` fires before any build.
**Recommendation: land it, do not defer it.** The CONTEXT's deferral clause is available but
this item does not need it; deferring would be the "hollow control" the CONTEXT forbids when a
real one is two edits away.

### EV-03 summary the plan should carry

| Item | Guard file:line | Landed? | Control today | Reachable from a control? | New tokens |
|------|-----------------|---------|---------------|---------------------------|-----------|
| `SRC-` whole token | `tools/check/steps/sources.sh:40` | no | no | yes (pure step) | 0 (reuses `source-unknown-ref`) |
| zero documents | `glossary.sh:48`, `sources.sh:33-35` | **yes** | **yes** (`gate-hardening.ids`) | yes | 0, or 2 if extended to `denylist`/`runner-scan` |
| per-step pass lines | `tools/gate.sh:139-143` | **yes** | none | **no** (§Q6 item 3) | 1 if O2 |
| manifest anchoring | `tools/check/manifests.cmake:88-94` | no | no | yes (pure step) | 0 (reuses `manifest-format`) |
| memmap generator | `cmake/IvsMemmapProbes.cmake:14-19` | no (nothing runs it) | no | yes, via `memmap.sh` | 1 (`memmap-generator-refused`) |

So the honest count is **three new controls** (SRC-, manifest anchoring, memmap generator),
**one verification** (zero documents), **one mechanism** (per-step pass line, on the step
side). `controls=380/380 → 383/383`. Say that instead of "five controls", or the plan will
double-count two items that already have theirs.

---

## Q7 — The preflight partition: what must be buffered for byte-identical output

### 7.1 How steps are ordered and how output is produced today

`tools/gate.sh:104-124`:

```sh
# Run every step (sorted, never stopping at the first failure).
passed=0
for name in $(cat "$GATE_TMP/steps.have"); do
  t0=$(date +%s)
  if sh "$CHECK_DIR/steps/$name.sh" > "$GATE_TMP/step.out" 2>&1; then
    cat "$GATE_TMP/step.out"
    keep_fails "$GATE_TMP/step.out"
    if grep -q -F -e "-- $name pass" "$GATE_TMP/step.out"; then
      passed=$((passed + 1))
      toks=$(sed -n "s/^-- $name pass //p" "$GATE_TMP/step.out" | tr '\n' ' ')
      tokens="$tokens$toks"
    else
      bad gate-step-no-pass-line "$name"
    fi
  else
    cat "$GATE_TMP/step.out"
    keep_fails "$GATE_TMP/step.out"
    bad gate-step-failed "$name"
  fi
  step_time "$name" "$(date +%s)" "$t0"
done
```

`steps.have` is `sort -u`'d at `:92`. So today the output order is exactly sorted step-name
order, and **five** things are emitted in that order:

1. the step's own stdout+stderr (`cat`);
2. its `-- FAIL` lines into `$GATE_TMP/fail.lines` (job summary only, not stdout);
3. a possible `-- FAIL gate-step-failed|gate-step-no-pass-line` line on **stdout**;
4. its `step-time <name> <s>` line on stdout;
5. its tokens appended to `$tokens`, which becomes the tail of the summary line (`:179`).

A partition that only buffers "tokens and `-- FAIL` lines" reorders (1), (4) and (5). **All
three must be buffered.**

### 7.2 The predicate, mechanically

"A step is preflight iff its script references neither the gate results directory nor
`build/`" resolves to one grep over the step script:

```sh
grep -q -e 'IVS_GATE_DIR' -e 'build/' "tools/check/steps/$name.sh"
```

Verified counts this session (38 scripts):

```
$ ls tools/check/steps/*.sh | wc -l
      38
$ for f in tools/check/steps/*.sh; do grep -q -e 'IVS_GATE_DIR' -e 'build/' "$f" && echo PRODUCER; done | wc -l
       9
$ … || echo PREFLIGHT … | wc -l
      29
```

**Producers (9):** `c-build c-faults c-tests cp1610-vectors faults-verdict golden memmap
pkg-install tests-inventory`.
**Preflight (29):** `api-effects c-purity ci-log ci-parity ci-summary claims cp1610
cp1610-reader denylist discrepancies files glossary hygiene hygiene-history important-gates
input integrity listing manifests profile psg-vectors release-assets runner-scan runner-selftest
sources state text video vocab`.

This matches the recorded 29/9 (`CI-EFFICIENCY.md:1649`). Two soundness notes:
`GATE_TMP` is exported (`tools/gate.sh:26`: `export ROOT CHECK_DIR FILELIST GATE_TMP`) but **no
step reads it** (`grep -ln GATE_TMP tools/check/steps/*.sh` → none), so no step can read
another step's output through it. `IVS_TEST_OUT_DIR` is read by exactly three steps
(`c-faults`, `c-tests`, `pkg-install`), all producers. The predicate is therefore both
necessary and sufficient today; the plan should record it as a *derived* predicate that a
future step could falsify, and add a guard: a preflight step that reads
`$IVS_GATE_DIR`-shaped state would be caught only by the partition itself, so print the
partition in the run header (as CONTEXT requires) so a reader sees the derivation.

### 7.3 What must be buffered, exactly

- Buffer each step's full stdout+stderr in `$GATE_TMP/step.out.<name>` (the file name gains the
  step name; `$GATE_TMP/step.out` is reused today and cannot hold 38 outputs).
- Record per step: `rc`, the presence of the pass line, the token string, and the start/end
  seconds.
- After all steps: replay **in `steps.have` order** — `cat` the buffer, `keep_fails`, emit the
  same `bad` lines, append the same tokens, emit `step-time <name> <end-start>`.
- `setup`, `controls` and `controls-wait` step-time lines are outside the loop
  (`:84`, `:138-139`) and keep their positions.

**The comparison cannot be literally byte-identical**, because `step-time <name> <s>`,
`gate-cpu user= sys=`, `gate-time total=` and `gate-env` carry measurements. The plan's
before/after diff must normalise those four line shapes (`sed -E 's/^(step-time|gate-cpu|gate-time|gate-env) .*/\1 …/'`)
and require the normalised streams to be identical. State that in the plan, or the executor
will chase a diff that can never close.

**Second-order effect the plan must handle:** `keep_fails` appends to `$GATE_TMP/fail.lines`,
which `ci-summary.sh` renders into `GITHUB_STEP_SUMMARY`. Replaying in sorted order keeps that
order identical too — provided `keep_fails` also runs during the replay, not during execution.

**Cost:** hosted cost is unchanged by construction (the same 38 steps, the same evidence). The
local win is time-to-first-signal: the recorded estimate is 21–24 s instead of about 44 s
(`CI-EFFICIENCY.md:1649`). The plan must not claim a hosted saving.

---

## Q8 — Cost: the recomputed window, the allowance, and what A2 will spend

### 8.1 The last-five median, recomputed by arithmetic

`r = cpu_s / build_s` (`CI-EFFICIENCY.md`, the cost rule). The last five rows of
`.planning/ci-runs.tsv` (78 lines: 1 header + 77 rows), with `$8 = build_s` and `$13 = cpu_s`:

| run | event | `build_s` | `cpu_s` | `r` |
|-----|-------|-----------|---------|-----|
| 45334963892 | `push` | 46 | 360 | 7.8261 |
| 45412585701 | `pull_request` | 48 | 356 | 7.4167 |
| 45413049708 | `push` | 46 | 354 | 7.6957 |
| 45414715588 | `push` | 45 | 339 | 7.5333 |
| 45415977805 | `push` | 42 | 352 | 8.3810 |

Sorted: 7.4167, 7.5333, 7.6957, 7.8261, 8.3810 → **median 7.6957**, i.e. **7.696**. That is
0.104 under the 7.8 budget and 0.604 under the 8.3 stop line. Command:

```sh
awk -F'\t' 'NR>1 && $13>0 {r=$13/$8; printf "%s\tbuild=%s\tcpu=%s\tr=%.4f\n",$2,$8,$13,r}' \
  .planning/ci-runs.tsv | tail -5
```

Note the window contains four rows taken after the "next-milestone opening" reading of 7.826,
all on records-only trees — the movement is runner speed, as recorded.

### 8.2 The allowance

Recorded (`CI-EFFICIENCY.md:1634`): at `build_s` 46, `Δr = Δcpu_s / 46`, so the 0.47 `r` of
headroom is about **21 hosted cpu-s**, ≈ **12.9 local cpu-s** at the local→hosted factor 1.675.
Recomputed against the current window the headroom is 8.3 − 7.696 = 0.604 → 27.8 hosted cpu-s to
the stop line, 0.104 → 4.8 hosted cpu-s to the budget. **Use the recorded 21/12.9**, because the
recorded figure is what the phase was priced against; note the recomputation so a reader is not
surprised.

### 8.3 What A1 saves and what A2 spends

A1 (savings, local cpu-s): lever (3) 0.12 (ceiling, recorded) + trim (a) 0.18 (3 of 4 dev
launches at 0.06) + trim (c) 0.4 + trim (d) 0.4 ≈ **1.10**, matching the recorded "about 1.1
together" (`CI-EFFICIENCY.md:1636`).

A2 (spend, from the recorded unit costs at `CI-EFFICIENCY.md:772-775`: control **0.185 local
cpu-s** ≈ 0.07 hosted for a pure step; test id 1.5–3.5 ms per id per preset):

| A2 item | New controls | New test ids | New faults | Local cpu-s |
|---------|--------------|--------------|-----------|-------------|
| EV-02 registry (24 lines) | 0 | 0 | 0 | ~0 (pure step reads 24 more lines) |
| EV-03 (3 controls + 1 step-side mechanism) | 3 | 0 | 0 | 3 × 0.185 = 0.56 |
| EV-04 (2 stand-ins) | 2 | 0 | 0 | 2 × 0.185 = 0.37 |
| EV-03 memmap generator invocation | 0 | 0 | 0 | ~0.2–0.3 per gate (one `cmake -P`) |
| preflight partition | 0 | 0 | 0 | 0 (hosted unchanged; local wall improves) |
| bookkeeping | 0 | 0 | 0 | 0 |
| **Total** | **5** | **0** | **0** | **≈1.2** |

That is **≈1.2 local cpu-s ≈ 0.7 hosted cpu-s ≈ +0.015 `r`** against an allowance of 12.9 local
/ 21 hosted. A2 is comfortably inside the ceiling; the phase's cost risk is in A1's measurement
discipline, not in A2's spend.

**One A1 spend to price if the self-control lands as an id** (see §Open Questions): +1 test id
= `tests-expected` +2, `tests-executed` +2, ≈ 3 ms × 2 = 0.006 local cpu-s. Negligible; the
issue is the *rule*, not the cost.

---

## Q9 — The constraints, measured

| Constraint | Source | Measured now |
|-----------|--------|--------------|
| No new gate step; 38 stays 38 | GUARDRAILS §2.1 | `ls tools/check/steps/*.sh \| wc -l` → **38**; `data/expected/gate-steps/*.ids` → 24 files, 38 non-comment lines |
| No new fault site | GUARDRAILS §2.1 | `awk '!/^#/ && NF' data/expected/faults/*.ids \| wc -l` → **235** |
| No new test id in A1 | GUARDRAILS §2.2 | `awk '!/^#/ && NF' data/expected/tests/*.ids \| wc -l` → **2479** ids × 2 presets = **4958** = the token's `tests-expected=4958` |
| Registry non-comment rows | CONTEXT | `awk '!/^#/ && NF' data/expected/important-gates.ids \| wc -l` → **306**; distinct gates → **90** |
| Control count | CONTEXT | 380 control dirs; token `controls=380/380` |
| No claim moves | CONTEXT tier ceiling | `claims=28 provisional=27` in the last recorded run |
| `.planning/config.json` and `.planning/state.json` never staged | EXEC-RULES | restated; note `git.create_tag` already reads `false` (`:15`), so the D-31 item is prose-only |
| `.planning/ci-runs.tsv` appended only by the orchestrator | CONTEXT | restated; `tools/ci-record.sh` is the only writer |
| No `gh` from an executor | EXEC-RULES | restated |

**The gate summary token from the last recorded local run** (`build/g9-final-sh.log:512`,
tail, verbatim):

```
gate: PASS steps=38/38 controls=380/380 … gates=90 lines=306 … tests-expected=4958 tests-executed=4954 tests-skipped=4 tests-missing=0
```

Every A1 commit must reproduce this token byte for byte except `ci-runs=`, which the
orchestrator's rows move.

### 9.1 The bookkeeping items, each measured

| Item | Recorded claim | Measured | Action |
|------|----------------|----------|--------|
| `D-31` `create_tag` | "carries `git.create_tag: true`" (`PROJECT.md:242`, `RELEASE-OPS.md:13`) | `.planning/config.json:15` reads `"create_tag": false,` | correct the **prose** in `PROJECT.md` and `RELEASE-OPS.md`; the key needs nothing and must not be staged by an executor |
| `RELEASE-OPS.md` "ten files" | P3: "the install list equals the pinned ten files" (`RELEASE-OPS.md:31`) | `data/expected/pkg-install-files.ids` → **11** non-comment lines | change "ten" to "eleven" |
| composite-version comment | "the stale composite-version comment" | `core/src/machine/machine_digest.c:12-14` says "it is version 2 since plan 05-03"; `core/src/machine/machine.h:157` reads `#define IVS_DIGEST_COMPOSITE_VERSION 3u` and `core/include/ivs/ivs_machine.h:352` says "composite version 3, plan 06-05" | amend the file-header sentence to version 3. **Not** line 407 — that comment describes a fault site and is correct in context |
| two bench golden commands | R11 requires "the writing command added to `docs/runner.md`" | `grep -n bench docs/runner.md` → **no match** | add the two `--update-golden`-style commands for the bench goldens |
| seven draft `VALIDATION.md` files | "the seven draft" | 8 files exist; `01` is `status: validated`, `02`–`08` are `status: draft` → **7 draft** | cite `08-ACCEPTED-DEBT.md` §1 (which covers six, Phases 2–7, and says the seventh is "left to the close record of plan 08-21") in a live record; rewrite no archive |
| branch + tag rulesets | applied or recorded as not applied | `main` protection exists (legacy, required check `gate`); no ruleset recorded | orchestrator only; `gh` is forbidden to executors |

---

## Validation Architecture

`workflow.nyquist_validation` is `true` in `.planning/config.json` (`"nyquist_validation": true`), so this section is required.

### Test Framework

| Property | Value |
|----------|-------|
| Framework | CTest via `CMakePresets.json` (`base`/`dev`/`release`, test presets `dev` and `release`) over the in-repo TAP harness (`tests/harness/`), plus the shell gate driver `tools/gate.sh` |
| Config file | `CMakePresets.json` (presets at lines 13, 25, 36, 37, 41, 47); suite registration in `tests/CMakeLists.txt`; expected-id inventories in `data/expected/tests/*.ids` (22 suites, 2479 ids) |
| Quick run command | `cmake --build --preset dev && ctest --preset dev` |
| Full suite command | `sh tools/gate.sh` (and `dash tools/gate.sh`; container: `sh tools/gate-linux.sh >/dev/null 2>&1 && grep -q '^gate: PASS' build/gate-linux.log`) |

### Phase Requirements → Test Map

| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| EV-01 | The two variants are static on dev and live on release; the derivation judgement rejects a one-value change; the trims keep their guards | suite + static | `ctest --preset dev -R '^runner-process$'` then `ctest --preset release -R '^runner-process$'` | ✅ (`tests/runner-process-tap.sh`, ids already in `data/expected/tests/runner-process.ids:60,61,137`) |
| EV-01 (state trims) | `state.restore.atomic-every-refusal` still compares every row's whole instance and still produces ≥1 refusal; `state.fuzz.byte-flips` still refuses every unsealed flip | suite | `ctest --preset dev -R '^state$'` and `--preset release` | ✅ (`tests/state/state_restore.c:1584`, `tests/state/state_fuzz.c:937`) |
| EV-02 | 24 registry lines resolve against existing inventories; all ten toy registries carry 44 lines; the step's prose names Phases 1–8 | gate step + controls | `sh tools/check/steps/important-gates.sh` (needs `ROOT`, `CHECK_DIR`, `FILELIST`), then `sh tools/check/run-controls.sh` and require `controls=383/383` | ✅ step + controls exist |
| EV-03 | Each hardening item's control fails for its intended token | controls | `sh tools/check/run-controls.sh` — every new control must print `control ok` and the step must exit non-zero on it | ✅ runner exists (`tools/check/run-controls.sh`) |
| EV-04 (a) | A Complete requirement with no registry line is refused | gate step | `sh tools/check/run-controls.sh` with the new `important-gates-complete-uncovered` control | ❌ Wave 0 — the control and the good tree's `.planning/` records must be created |
| EV-04 (b) | A toy registry that drifts from the good tree is refused | controls runner | `sh tools/check/run-controls.sh` with the drift control | ❌ Wave 0 — the drift check and its control must be created |
| Preflight partition | Normalised gate output is identical before and after; the summary token is byte-identical | end-to-end | `sh tools/gate.sh >/tmp/after.out 2>&1; dash tools/gate.sh >>/tmp/after.out 2>&1` and diff against the pre-change capture with `step-time`/`gate-cpu`/`gate-time`/`gate-env` normalised | ❌ Wave 0 — the "before" capture must be taken and committed as evidence |

### Sampling Rate

- **Per task commit:** `cmake --build --preset dev && ctest --preset dev` for a task that
  touches C (`tests/state/*`); `sh tools/check/run-controls.sh` for a task that touches a step
  or a control; nothing cheaper for a task that touches only `.planning/` text.
- **Per wave merge:** `sh tools/gate.sh` **and** `dash tools/gate.sh`, both ending `gate: PASS`
  with the predicted token; plus the container gate
  (`sh tools/gate-linux.sh >/dev/null 2>&1 && grep -q '^gate: PASS' build/gate-linux.log`) at
  each wave boundary, one at a time (lesson 18), log copied before anything else runs.
- **Phase gate:** full suite green on the final tree under sh, dash and the container before
  `/gsd:verify-work`; `git status --porcelain` empty except orchestrator state.

### Wave 0 Gaps

- [ ] `tools/check/controls/_good/tree/.planning/REQUIREMENTS.md` — the toy requirement record
      EV-04 reads (does not exist; the good tree's `.planning/` holds only `PROJECT.md` and
      `ci-runs.tsv`)
- [ ] `tools/check/controls/_good/tree/.planning/milestones/v0.1.0-REQUIREMENTS.md` — the
      second record
- [ ] `data/expected/important-gates-excluded.ids` (under the recommended S1 shape) and its
      good-tree copy
- [ ] The EV-04 controls: `important-gates-complete-uncovered` and the drift control, plus their
      lines in `data/expected/controls/important-gates.ids` and a `run-controls.ids`
- [ ] `DEVIATION` files for the nine `important-gates-*/` overlays
- [ ] The pre-change gate capture (`sh` and `dash`) that the partition's diff is taken against
- [ ] The derivation-equality check's scratch stand-in file (a one-value-changed scenario body)
- [ ] Framework install: none — CTest, cmake, ninja and a C17 compiler are already required by
      the gate (`tools/gate.sh:45-47` refuses without them)

---

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Deciding whether a scenario runs on dev | A new skip mode or a new inventory column in `tests/runner-process.ids` | `mutant_case`'s `$config = Release` switch (`tests/runner-process-tap.sh:532-538`) | A new skip would move `tests-skipped` and stale every recorded token |
| Judging a static variant | A second expected-value file for the variants | The derivation's `asserts-begin`/`asserts-end` blocks (`docs/video/demo-lamp-tone-1-derivation.md:438,512`) | The derivation is the frozen oracle; a second file is a second source of truth |
| Comparing the good tree with a toy tree | A bespoke comparison script | `diff` over `sort`ed non-comment lines, beside `comm` in `run-controls.sh:250-262` | The runner already owns both trees and already reports this shape of finding |
| Parsing the requirement records | A YAML/JSON side-car of requirement statuses | The `## Traceability` markdown table (`\| REQ \| Phase n \| Status \|`) | It is the table the project already treats as authoritative and it needs no generator |
| Detecting a preflight step | A hand-maintained list | `grep -q -e 'IVS_GATE_DIR' -e 'build/' tools/check/steps/<name>.sh` | A hand list drifts the first time a step gains a results-dir read |
| Running the memmap generator under test | A new gate step | `cmake -DROOT=… -DOUT=… -P cmake/IvsMemmapProbes.cmake` from inside `memmap.sh` | The generator already exists as a standalone script with fail-closed messages |

**Key insight:** in this repository every mechanism Phase 9 needs already exists as a *mode* of
something else. The phase's whole risk is adding a parallel mechanism that then has to be kept
in step with the original.

---

## Common Pitfalls

### Pitfall 1: the derivation judgement is written against the wrong slice of the file
**What goes wrong:** comparing the whole scenario file (or the whole derivation block,
including its fenced code markers and its leading `# frame:` lines) with the other.
**Why it happens:** the derivation block's first lines are documentation, and the scenario file
has a provenance comment block plus an `ivss`/`profile`/`content`/`capture` header.
**How to avoid:** use the exact rule verified in §1.5 (drop the fence lines and the *leading*
`#` lines from the derivation; drop everything up to and including `ivss 1` and the
`id`/`profile`/`content`/`capture` lines from the scenario). **Warning signs:** the judgement
fails on `demo-lamp-tone-pass` on the first run, which proves the slice is wrong, not the tree.

### Pitfall 2: the `id` line cannot "equal the pass scenario's"
**What goes wrong:** the plan copies CONTEXT's sentence literally and writes a check that the
shift file's `id` line equals the pass file's `id` line.
**Why it happens:** the sentence is unsatisfiable as written.
**How to avoid:** record the deviation and check `id <own id>` instead. **Warning signs:** a
check that can only pass by deleting the `id` line.

### Pitfall 3: a `release-only` scenario is implemented as a skip
**What goes wrong:** `tests-skipped` moves 4 → 8, every recorded token string is stale, and the
`-- SKIPPED` count check in `tests-inventory.sh` fires.
**Why it happens:** a skip is the obvious way to "not run" something, and the suite already has
`skip-ok`.
**How to avoid:** copy `mutant_case` — an ordinary `ok` line, no `# SKIP`. **Warning signs:**
`tests-skipped=8` in a trial run.

### Pitfall 4: the pair id loses its report on dev
**What goes wrong:** `runner-process.pair.demo-lamp-tone-pass.demo-lamp-tone-shift` fails
`pair-no-report` on the dev preset and `tests-missing dev/runner-process/...` if it is dropped.
**Why it happens:** the pair judge needs two reports and only one scenario now runs on dev.
**How to avoid:** give the pair the same two-mode switch and an inventory-only judgement on
dev (§1.4). **Warning signs:** `pair-no-report` on a dev-only ctest run.

### Pitfall 5: the derived must-be-covered set fails on the good tree
**What goes wrong:** EV-04 lands, and `important-gates-uncovered-gate` fires for
`API-01 API-05 API-06 RUN-03 PKG-02 REL-01` on the pristine tree, so `run-controls.sh`'s
positive control fails and the whole gate goes red.
**Why it happens:** 50 requirements are marked Complete and the registry covers 27.
**How to avoid:** land an exclusion record (§Q5.1 S1) or grow EV-02 to 23 requirements. Compute
the set difference in the plan and state the number. **Warning signs:** a first gate run after
EV-04 with six `uncovered-gate` lines.

### Pitfall 6: appending the 17 toy lines in the wrong place breaks two EXPECT line numbers
**What goes wrong:** `important-gates-bad-req-phase5` expects `FAIL important-gates-unknown-id:
line 19` and `-phase6` expects `line 24`; inserting the new lines anywhere but the end shifts
both and the controls fail for the wrong reason.
**Why it happens:** the toy registries are whole-file overlays with positional EXPECT tokens.
**How to avoid:** append after the existing last line in all ten files (§Q4.2). **Warning
signs:** `control-token-missing` on those two controls.

### Pitfall 7: a "control" for the per-step pass line that cannot fail
**What goes wrong:** a control whose `CHECK` is `gate` (no such step) is reported
`control-unknown-step`, or one aimed at `gate-step-no-pass-line` never reaches it.
**Why it happens:** the guard lives in `tools/gate.sh`, which the control runner never executes.
**How to avoid:** harden the step side (O2, §Q6 item 3) and record the `gate.sh` branch as
covered by the definition-of-done run. **Warning signs:** a control that passes on the good
tree and on the bad tree alike.

### Pitfall 8: the preflight partition reorders the summary line's tokens
**What goes wrong:** the summary line's token order changes because `tokens` is accumulated
during execution.
**Why it happens:** `tokens="$tokens$toks"` (`tools/gate.sh:114`) is inside the run loop.
**How to avoid:** build `tokens`, `passed`, the `bad` lines and the `step-time` lines during the
sorted replay (§Q7.3). **Warning signs:** a diff that shows only the tail of the summary line
reordered.

### Pitfall 9: the byte-identical comparison chases timing numbers
**What goes wrong:** the before/after diff never closes because `step-time`, `gate-cpu`,
`gate-time` and `gate-env` carry measurements.
**How to avoid:** normalise those four line shapes before diffing and say so in the plan.
**Warning signs:** a diff whose only hunks are numbers.

### Pitfall 10: the memmap generator control is written against the wrong script
**What goes wrong:** the control overlays a bad literal file and expects a generator message,
but the *step* refuses it first with `memmap-literal-grammar`, so the token is ambiguous.
**Why it happens:** two checkers read the same literals.
**How to avoid:** pick a defect the step does not check (e.g. `mirror=3`, which the generator
refuses as "not a power of two" and `memmap.cmake` may not), verify the intended token is the
only one printed, and pin it. **Warning signs:** two `-- FAIL` lines for one control.

---

## Package Legitimacy Audit

**Not applicable.** Phase 9 installs no external package in any ecosystem. Every mechanism is a
mode of an existing in-repo script, and the only external commands the phase relies on are
already required by `tools/gate.sh`'s own preflight (`cmake`, `ninja`, `cc`;
`tools/gate.sh:45-47`). No `npm view`, `pip index versions` or `cargo search` was run, and none
is needed. If a later plan proposes a dependency for any of these checks, it must be gated
behind a `checkpoint:human-verify` task and a fresh legitimacy run.

---

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | The 17 gate names in the 24-row proposal are the right names, and `gates=90 → 107` is the intended delta | §Q3.2, §Q4.3 | The token prediction is wrong and the executor stops on an unexpected token. Low cost: the names are free-form (`^[a-z0-9][a-z0-9-]*$`, `important-gates.sh:93`) and only the count matters |
| A2 | "Sole non-vacuous evidence" per requirement is the assignment in §Q3.3; the planner may prefer different mechanisms for FND-01/02/04 | §Q3.3 | The registry would be defensible but not the owner's intended mapping. Reversible: a registry line is one line |
| A3 | S1 (an exclusion record) is the right resolution for the 6 unregistered Complete requirements | §Q5.1 | A different shape changes the file list and possibly the line count. This is the highest-risk assumption in the phase |
| A4 | The per-step pass-line item is resolved on the step side (O2) rather than by a control | §Q6 item 3 | EV-03 would then be "four controls and one deviation" rather than "five controls". Recorded, not hidden |
| A5 | `cmake/IvsMemmapProbes.cmake` can be invoked from `memmap.sh` without breaking its "writes nothing" contract (writing to a `mktemp -d`) | §Q6 item 5 | The step's header prose and its partition class would need amending. Reversible |
| A6 | The 0.185 local cpu-s control unit cost still holds for these five controls | §Q8.3 | The spend could be 2–3× higher and still fit the 12.9 local allowance |
| A7 | `runner-process.judge.variant-static` is a new test id and therefore contradicts A1's "no new test id" | Open Questions | If the owner reads it as allowed, the token gains `tests-expected +2`, `tests-executed +2` |

---

## Open Questions (RESOLVED)

All five are answered by amendments A-01 to A-09 in `09-CONTEXT.md`; each question carries its
answering amendment inline. Nothing in this section is open at plan time.

1. **Does `runner-process.judge.variant-static` add a test id in wave A1, and is that allowed?**
   — RESOLVED: A-03. No new test id in A1; the judgement ships as an assertion inside the
   existing variant case, with its negative half run in-case.
   - What we know: CONTEXT's A1 section says both "A1 adds no test id, no control, no fault, no
     gate step and no registry line" **and** "One suite self-control
     (`runner-process.judge.variant-static`) lands in the same commit". GUARDRAILS §2.2 is
     explicit: "No test id, control, fault or registry line may be committed before the hosted
     row recorded after lever (3) and trims (a)–(c)." A judge self-control is a test id — every
     existing one is a line of `data/expected/tests/runner-process.ids` (10 of them), so adding
     one moves `tests-expected` and `tests-executed` by 2.
   - What's unclear: whether the owner intends the self-control to be exempt (it *removes* a
     live launch and *adds* a static check, so it is not "new gate content" in the sense
     GUARDRAILS means) or whether the judgement must be folded into an existing id.
   - Recommendation: **fold it into an existing judge id** and keep A1 id-free. The cheapest
     fold is to extend `runner-process.judge.body-differs` (already the self-control for
     "a comparison that accepts a wrong body", `:1591`) with the variant case, or to extend
     `runner-process.judge.pair-video-equal`. If the owner prefers a named id, the plan must
     amend GUARDRAILS §2.2 for this one id and predict `tests-expected=4960
     tests-executed=4956`, and record the amendment — an unrecorded exception is the defect.

2. **How are the 6 unregistered Complete requirements resolved?** — RESOLVED: A-01.
   Shape S1: a six-row exclusion ledger `data/expected/important-gates-excluded.ids`, and the step
   refuses a Complete requirement that is neither registered nor excluded.
   - What we know: 50 Complete, 27 registered, 17 added by EV-02, 6 left over (§Q5.1).
   - What's unclear: whether an exclusion record is acceptable (it is a new data file and a new
     file type) or whether EV-02's scope should grow.
   - Recommendation: S1. It is the only shape that is non-vacuous, keeps `lines=330`, and keeps
     the 6 visible.

3. **Does the good tree's `.planning/` carry real requirement records or minimal toy ones?**
   — RESOLVED: A-09. Minimal toy records listing exactly the ids the toy registry covers.
   - What we know: the good tree currently carries `.planning/PROJECT.md` and
     `.planning/ci-runs.tsv` only (§Q5.2).
   - What's unclear: whether a copy of a real record inside the control tree counts as an
     archive duplication the project's "write it once" rule forbids.
   - Recommendation: minimal toy records listing exactly the 44 ids the toy registry covers,
     plus the exclusion file. They are consistent by construction and cannot drift silently
     because the drift check compares the toy registry, not the records.

4. **Is `memmap-generator-refused` a new token in an existing step, or does the generator stay
   uncontrolled?** — RESOLVED: A-02. The item is landed, not deferred: `memmap.sh` gains one
   `cmake -P` invocation and two tokens, with no new gate step.
   - What we know: it is reachable (§Q6 item 5) at ~0.2–0.3 local cpu-s.
   - What's unclear: whether adding a token to `memmap.sh` counts as "new gate content" for the
     A1/A2 ordering rule. It is A2 content either way, so the ordering rule is satisfied.
   - Recommendation: land it in A2; the CONTEXT's deferral clause is available but unnecessary.

5. **Which recorded figure governs the allowance — 21 hosted cpu-s (recorded) or 27.8
   (recomputed from the current window)?** — RESOLVED: A-07. The recorded 21 hosted / 12.9 local
   stands as the ceiling; the recomputation (median 7.696) is printed beside it.
   - What we know: the recorded figure was computed at `build_s` 46 against 0.47 of headroom;
     the current window's headroom is 0.604.
   - Recommendation: use the recorded 21/12.9 as the ceiling (it is the more conservative) and
     print the recomputation beside it, so the next phase boundary can re-read it.

---

## Sources

### Primary (HIGH confidence — read out of the tree this session)

- `tools/gate.sh` (181 lines) — the whole step loop, the token accumulation, the summary line,
  the partition predicate's inputs.
- `tools/check/run-controls.sh` (319 lines) — `build_tree`, `run_step`, `good_one`, `ctl_one`,
  the inventory consistency block, `apply_make`.
- `tools/check/steps/important-gates.sh` (111 lines) — the whole step: the five inventories,
  the `nreq` split string, the purity note, the token prose.
- `tools/check/steps/tests-inventory.sh` (198 lines) — `tests-expected`/`tests-executed`/
  `tests-skipped`, `-- SKIPPED`, `skip-ok`, the per-preset id-set equality.
- `tools/check/steps/sources.sh`, `glossary.sh`, `memmap.sh`, `manifests.sh`,
  `tools/check/manifests.cmake`, `cmake/IvsMemmapProbes.cmake`.
- `tests/runner-process-tap.sh` (1733 lines) — the scenario loop, `result`, `mutant_case`,
  `run_mutant`, `patch_changes_image`, `check_pair`, `check_roundtrip`, `rt_refuse`,
  `self_control`.
- `tests/state/state_restore.c` (`rs_atomic_every_refusal` at 1584-1596, `rs_run_family` at
  669-692), `tests/state/state_util.c` (`su_call_row` at 432-493, `su_run_rows` at 496-541),
  `tests/state/state_fuzz.c` (`fz_run_flips` at 915-936, `fz_byte_flips` at 937-950).
- `data/expected/runner-scenarios.ids`, `scenario-pairs.ids`, `tests/runner-process.ids`,
  `important-gates.ids`, `controls/important-gates.ids`, `controls/gate-hardening.ids`,
  `pkg-install-files.ids`, `faults/*.ids`, `controls/*.ids`.
- `docs/video/demo-lamp-tone-1-derivation.md` — the four `asserts-begin`/`asserts-end` blocks
  (320/394, 398/434, 438/512, 516/578) and the counts line at 316.
- `.planning/REQUIREMENTS.md`, `.planning/milestones/v0.1.0-REQUIREMENTS.md` (traceability
  tables), `.planning/ci-runs.tsv` (78 lines), `.planning/config.json`.
- `.planning/milestones/v0.1.0-phases/01-…/01-REVIEW.md:214-266` (IN-01 to IN-05) and
  `08-…/08-ACCEPTED-DEBT.md` §1–§2.
- `.planning/ops/GUARDRAILS-v0.2.md` (R1–R11, §2, §3), `EXEC-RULES.md`, `PLAN-LESSONS.md`
  (lessons 7, 15, 19, 20, 22), `INDEX.md`.
- `.planning/CI-EFFICIENCY.md:1600-1671` (the next-milestone opening, the trims with preserving
  conditions, the preflight rule, the allowance) and `:770-775` (the measured unit costs).
- `build/g9-final-sh.log:512` — the last recorded local summary token.

### Secondary (MEDIUM confidence — inference from the above)

- The 24-row mechanism assignment (§Q3.3): every id was verified to exist; the *judgement* that
  it is each requirement's sole non-vacuous evidence is mine, not the owner's.
- The preflight partition's buffering requirements (§Q7.3): derived by reading the loop, not by
  running a partitioned gate.
- The ≈1.2 local cpu-s A2 spend (§Q8.3): arithmetic over recorded unit costs, not measured.

### Tertiary (LOW confidence)

- None. No claim in this document rests on training knowledge alone.

## Metadata

**Confidence breakdown:**
- Lever (3) mechanics and the derivation judgement: **HIGH** — the judgement was executed this
  session and passes on all four scenarios; the `mutant_case` template is read in full.
- Trims (a), (c), (d): **HIGH** for the edit sites; **MEDIUM** for trim (d)'s "moves the digest"
  guard, which has no named mechanism in the tree (the operational guard is
  `c.accepted != 0`).
- EV-02's 24 rows: **HIGH** that every id exists; **MEDIUM** on the assignment.
- EV-03: **HIGH** — every guard located by file and line, and two items measured as already
  landed.
- EV-04: **HIGH** on the 50-vs-27 gap and the drift property (both computed); **MEDIUM** on the
  recommended shape.
- Preflight partition: **HIGH** on the counts (29/9 computed) and the predicate; **HIGH** on the
  buffering requirement.
- Cost: **HIGH** — arithmetic from committed rows.

**Research date:** 2026-11-24
**Valid until:** the next change to `tools/gate.sh`, `tools/check/steps/important-gates.sh`,
`tools/check/run-controls.sh`, `tests/runner-process-tap.sh` or the requirement records —
whichever comes first. The cost window should be recomputed at every phase boundary (7 days is
the practical horizon given runner-speed variance).
