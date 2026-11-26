# Phase 18: The Inspection Surface - Context

**Gathered:** 2026-11-26
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 18 builds the instrument that Phase 19 needs in order to decide a display-path
default by discrimination rather than by fit: a **build-only, scripted, headless
inspection surface** over seams the core already exposes. It delivers four things:
`docs/inspection.md` frozen **before** any code; the tool (`trace`, `break`, `dis`,
`step`) with a cycle-stamped access trace that reconciles with the runner's own
report; breakpoints asserted as a hit/non-hit pair; and disassembly that round-trips
through `ivs_asm`. Its output is an operator-local, rights-bearing artifact, refused
inside the work tree.

Out of scope by construction: no new gate step (38), no new fault (238), no new
golden (29), no new claim (`claims=29`, `provisional=28`), no new installed file
(`pkg-install-files.ids` stays 11), no new core function or sink, and no move of any
profile field, parameter digest, snapshot literal or golden (D-52). Nothing here
reaches `measured-hardware`; the tier ceiling is **none**.

</domain>

<decisions>
## Implementation Decisions

### The frozen oracle
- `docs/inspection.md` is committed **alone in wave 1**, before `tools/inspect/` exists, and the ancestry is proved with `git merge-base --is-ancestor <doc commit> <first tool commit>` (`GUARDRAILS-v0.4` W5). It states the command surface (`trace`, `break`, `dis`, `step`), the record fields, the liveness guards and the refusal tokens.
- When plan prose and the frozen document differ, **the document wins** and the plan sentence becomes a recorded deviation. This is the one place in the milestone where the document is authoritative over the plan, so the document is written as a contract, not as a sketch.
- The document is written from the seams that exist today, not from a wished-for API: `ivs_machine_peek` (public, side-effect-free, `core/include/ivs/ivs_run.h`) and the internal per-unit access trace `ivs_machine_step_unit_trace_internal` (`core/src/machine/machine.h`), which the `psg`, `stic` and `machine` suites already drive through the same header.

### The tool
- **One binary, four subcommands** (`trace`, `break`, `dis`, `step`), built as a target under the existing presets, never installed, outside the eleven-path prefix. No new preset, no new step, no new dependency.
- It **reuses the runner's scenario parsing and observation layer by linking the same sources**, so its report is not a second implementation of the report: its `digest.*` lines must be **byte-identical** to `ivs-run`'s for the same scenario run untraced. A second report writer would be a second truth, and the criterion would be unfalsifiable.
- The trace is written as **one record per line, in a text format with named fields** (tick first, then the access kind, address, value and the unit's own stamps), because the ids assert domains — `records > 0`, ticks non-decreasing, last tick equal to the report's own `step.<k>` end tick — and a text line is what a shell id can assert without a parser.
- **No core change.** No new core function, no new sink, no file I/O in the core, no new installed file (D-52). The tool opens its own output file and writes it; the core never sees a path.
- The trace file extension is **`ivst`**, refused by name in `tools/check/binary-ext.txt` and `.gitignore` in the same commit as the rights row.

### Breakpoints
- `break --pc <addr>` prints a **hit count and a non-hit count in one report**, and the id asserts the **pair**: hit count 1 at a unique PC and 0 at an unreached PC, in the same id. A matcher that always hits must fail the 0 case — that is the committed stand-in.
- A breakpoint never alters machine state, log or samples: the report's `digest.*` lines under a breakpoint equal the same run's lines with no breakpoint. This is the same non-interference property the trace has, asserted separately because it is a separate seam.

### Disassembly
- `dis` reads the project's **own** listings (`data/expected/listings.ids`) and the words `ivs_pack --dump-words` already produces; it invents no decoder table beyond the one the assembler and the listings already pin.
- The round trip lives in the existing `tests/asm-tap.sh` suite: disassemble, re-assemble with `ivs_asm`, bytes equal to the input, with `words > 0`, **at least one two-word form and one immediate present** so the case cannot pass on a corpus of single-word instructions, and a constant-mnemonic stand-in failing byte equality.
- **The limb is built, not dropped.** `18-03` is pre-authorized as droppable, and the reversal condition is recorded here: it is dropped only if it cannot round-trip a corpus containing a two-word form and an immediate inside the priced line, and then `docs/inspection.md`'s disassembly section reads "not built; a reader uses the committed listings named in `data/expected/listings.ids`" and TOOL-02 exits with the reduced surface named. Dropping it for schedule reasons alone is not authorized.

### Rights
- `data/rights/inspection-output.json` with `disposition: owner-local-only`, `source: null`, `taken: nothing` and tokens `trace-file`, `disassembly-dump`, `inspection-dump`; plus its `data/expected/rights.ids` line and its `docs/RIGHTS.md` row, all in **one commit** with the `ivst` entries in `tools/check/binary-ext.txt` and `.gitignore`.
- The tool **refuses an output path inside the work tree** with the token `inspect-output-tree`, allows `refs/` as the documented exception, and refuses an existing file. `git ls-files | grep -E '\.ivst$'` is empty at every commit, whatever the file contains.
- Nothing is ever committed from the tool's output: not a golden, not a fixture, not a CI artifact, not a claim artifact.

### Build-only, proved
- `data/expected/pkg-install-files.ids` stays **11** and a `pkg-install` selftest proves `ivs_inspect` is absent from a copy of the installed prefix; one `release-assets` case refuses it as an archive member; `ls tools/check/steps/*.sh | wc -l` reads **38**; `goldens=29`, `faults=238`, `claims=29`, `provisional=28` are unchanged.
- The `c-build` module line, the `ivs-inspect-sources.txt` writer, the toy source lists and the unlisted-source control move with `tools/inspect/*` in one commit (`GUARDRAILS-v0.4` W4), and `docs/packaging.md`'s "not installed" line names the tool.

### Decided autonomously
- [Frozen oracle] Q: "what does the document pin, and how is its priority enforced?" → Selected: "the command surface, the record fields, the liveness guards and the refusal tokens, frozen alone in wave 1 with an ancestry proof, and the document wins over plan prose" — prose in one new file, no code depends on its layout beyond the section names.
- [The tool] Q: "one binary or four, and does it re-implement the report?" → Selected: "one binary with four subcommands, reusing the runner's scenario and observation sources so its `digest.*` lines are byte-identical to `ivs-run`'s" — a build target and a source list; the alternative (a second report writer) is the one choice that would be expensive to undo, and it is refused on that ground.
- [Trace format] Q: "what shape are the records?" → Selected: "one text record per line, tick first, then kind, address, value and the unit's stamps" — a writer function in a build-only tool, invisible to the core and to every digest.
- [Breakpoints] Q: "hit count only, or hit and non-hit?" → Selected: "both, asserted as a pair in one id, with an always-hits stand-in failing the zero case" — one report field and one id.
- [Disassembly limb] Q: "build `18-03` or take the pre-authorized drop?" → Selected: "build it, with the drop's reversal condition recorded" — a suite case in an existing suite; taking the drop later costs a document paragraph and a TOOL-02 note, both cheap, so the choice is reversible either way.
- [Rights] Q: "one row or several, and which tokens?" → Selected: "one `inspection-output` row with three tokens, plus the `ivst` refusals in the same commit" — a JSON row, an ids line and a document row, all revertible together.

### Claude's Discretion
- The exact record field names and their order after the tick, the subcommand flag spellings beyond the four names, and the internal module split under `tools/inspect/`.
- How much of `runner/` the tool links: the planner picks the smallest set that makes the `digest.*` equality true rather than approximate, and states the set in the plan.

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- `core/include/ivs/ivs_run.h` — `ivs_machine_peek(const ivs_machine *, uint16_t addr, uint32_t count, uint16_t *out)`, the side-effect-free read path the tool may use for inspection without advancing time.
- `core/src/machine/machine.h:275-280` — `ivs_machine_step_unit_trace_internal(m, out, sink, ivs_cp1610_access *trace, size_t trace_cap)`, the per-unit access trace the `psg`, `stic` and `machine` suites already drive through `"machine/machine.h"`; the tool reuses this seam rather than adding one.
- `runner/` — scenario parsing, observation and report emission, whose `digest.*`, `step.<k>.*`, `slot.*` and `output.*` keys the golden `data/golden/demo-lamp-tone-pass.digests` already pins.
- `tests/asm-tap.sh` — the `ivs_asm` suite with `asm.twin-<id>`, `asm.pairs.<class>`, `asm.neg.<token>` and the `asm.judge.mutated-twin` self-control pattern the disassembly round trip follows.
- `data/rights/*.json` + `data/expected/rights.ids` + `docs/RIGHTS.md` — the three-place rights row shape (`tools/check/rights-lib.cmake` refuses an id with no row and a row with no id).
- `tools/check/binary-ext.txt` — one list feeding the `files` step, the release member check and the hygiene binary-by-name rule; `ivst` joins it.
- `data/expected/pkg-install-files.ids` (11 paths) and the `pkg-selftests.ids` cases (`host-so-absent` is the precedent for "the build-only object is absent from the prefix").

### Established Patterns
- A build-only tool has no install rule, is not a release member, and is proved absent by a `pkg-install` selftest plus a `release-assets` case.
- A new capability lands as ids in a suite that already exists; every new check ships a must-fail registry row, a liveness guard and a committed stand-in showing the guard fail, in one commit.
- `data/expected/listings.ids` names the seven committed listings and their word counts; `ivs_pack --dump-words` is the existing way to read words out of a listing.
- `tools/check/steps/c-build.sh` owns the module lines for every built target; a new source file that is not listed is a `c-build` finding.

### Integration Points
- `tests/CMakeLists.txt` registers the suites; `tests/runner-process-tap.sh` and `tests/asm-tap.sh` take the built binaries as arguments, so the new tool's path reaches them the same way `ivs_flatwrap`, `ivs_retro_shim` and `ivs_host_harness` do.
- `docs/packaging.md` carries the "not installed" line for build-only targets; `docs/inspection.md` is the new page.
- `.gitignore` and `tools/check/binary-ext.txt` are the two places `ivst` must appear in the same commit as the rights row.

</code_context>

<specifics>
## Specific Ideas

The discuss boilerplate of `GUARDRAILS-v0.4` §5, answered in writing before planning:

1. **W rows and rights rows touched.** `GUARDRAILS-v0.4` **W3** (a trace or disassembly artifact: `data/rights/<slug>.json`, `data/expected/rights.ids`, `docs/RIGHTS.md`, `tools/check/binary-ext.txt` with token **and** scope, `.gitignore`, the `files` id that refuses it, one `files-unlisted-*` control, `docs/inspection.md`), **W4** (`tools/inspect/*`: the `c-build` module line, the `ivs-inspect-sources.txt` writer, the toy source lists, the unlisted-source control, the `docs/packaging.md` "not installed" line, `pkg-install-files.ids` unchanged at 11 plus one `pkg-install` selftest, one `release-assets` case) and **W5** (`docs/inspection.md` frozen before any code, ancestry proved). One new rights row: `data/rights/inspection-output.json` (`owner-local-only`, `source: null`, nothing taken).
2. **Rival rule pair separated.** None directly — the tool is the instrument Phase 19 uses. What this phase separates is the *observation*: traced vs untraced (`digest.*` equality) and breakpoint vs no breakpoint (the same equality), each asserted as its own id.
3. **Count delta and its price.** `steps` **38** (unchanged), `controls` **+1** (the unlisted-source control W4 requires), `faults` **238**, `goldens` **29**, `claims` **29**, `provisional` **28**, install list **11**, `tests-expected` grows by the new `runner-process` and `asm` ids across both presets (the plan states the exact number before execution), `gates` and `lines` unchanged unless a registry row is added — if one is, the plan states it. Priced **2.96 hosted cpu-s** against the 21 allowance, the milestone's largest line, measured locally and multiplied by 1.675 (D-55); the hosted row is recorded to monitor, not to decide.
4. **Oracle, freeze commit, liveness guard, stand-in.** Oracle: `docs/inspection.md`, frozen in wave 1 with `git merge-base --is-ancestor` proving the ancestry. Liveness guards: the trace ids' last-tick equality against the report's own `step.<k>` end tick, the breakpoint pair's zero case, the disassembly round trip's `words > 0` plus a two-word form and an immediate. Stand-ins: an emitter returning zero records, an emitter repeating one tick, a "tracing" build that pokes a register, a matcher that always hits, and a constant-mnemonic disassembler — each failing its own id for its own stated reason.
5. **Rights posture in one line.** The tool's output is operator-local and rights-bearing: one `owner-local-only` row with nothing taken, refused inside the tree except under `refs/`, `ivst` refused by name in two lists, and **no trace or disassembly ever committed, whatever its content**.
6. **What is deferred, and where.** The disassembly limb's drop (reversal condition recorded above, in `docs/inspection.md` and the plan); any real frontend (D-49 stays); the owner-local sweep (Phase 19). Recorded in `docs/inspection.md` and the plan that takes it.
7. **Evidence tier.** **None.** The tool is build-only and its output is repeatability information, never a claim: `claims=29` and `provisional=28` are invariants, and `CL-0002` remains the only `established` claim.

No "I want it like X" reference was raised: the frozen criteria and `GUARDRAILS-v0.4` W3/W4/W5 name every artifact, token, id and count this phase produces.

</specifics>

<deferred>
## Deferred Ideas

- A windowed or interactive inspection surface — refused: the tool is scripted and headless, and a window belongs to a host, not to this repository (D-49).
- Reading third-party or operator content through the tool — allowed only as an operator-local artifact under the rights row, never in CI, and never committed.
- Any second decoder table for disassembly — refused: the assembler and the committed listings already pin one, and a second table is a second truth.

</deferred>
