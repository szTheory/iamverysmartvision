# Phase 14: Cartridge Route and Real-Content Conformance - Context

**Gathered:** 2026-11-26
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 14 makes the content path real, and fixes the first thing a real run
exposed. Three deliverables:

1. **A build-only wrap tool** `tools/flatwrap <flat> <base> <out>
   [--word-order hi-first|lo-first]` that takes a byte length, a word order and
   an address the operator states, and writes the project's own `IVSC v1`
   container. It parses no third-party format, auto-detects nothing, names no
   other tool's format, is absent from the install list, and produces derived
   material that is never committed, never a golden, never a fixture and never a
   claim artifact. Seven typed refusals, each beside its boundary positive.
2. **A cartridge role in the owner-local route**, with all six derived-output
   refusals and the snapshot rule extended to it, and the pass path exercised on
   project-made files so no owner image is needed to prove it.
3. **The first real-content blocker fixed, not recorded**: the interrupt-enable
   default (`cpu.interrupt_accept_clears_intff`) resolves to `1` from the
   owner-local observation (D-51), with a project-authored test pinning **both**
   settings, the field still tagged `unknown-default-chosen`, the observation in
   the discrepancy log (C41), and the profile parameter digest, every snapshot
   literal and the boot-stub-2 interrupt expectations retargeted in the same
   commit. The next blocker the same run exposes (the display-enable strobe
   never latches, so frames stay black) is recorded with its own row and gate —
   named, not silently carried.

Out of scope: `.cfg` parsing and bank switching (FMT-01/FMT-04 stay `Deferred`),
any host or shared object (Phase 15), any claim (tier ceiling `none`), any
committed content byte.

</domain>

<decisions>
## Implementation Decisions

### The wrap tool states bytes, never a format
- Signature and defaults are frozen by D-47 and scoping R-2: `tools/flatwrap
  <flat> <base> <out>` plus `--word-order hi-first|lo-first`, default
  `hi-first` (the project's one wire order). Input is a byte length and a word
  order only; nothing is auto-detected and no other implementation is described.
  — **Reversibility:** one-way — the tool's refusal tokens and its `IVSC v1`
  output are a published operator contract that `data/expected/flat-refusals.ids`
  pins append-only; changing a token or the wire order later re-opens every row
  and every test that cites one.
- Build-only: no install rule, not a gate step, absent from
  `data/expected/pkg-install-files.ids` (which stays eleven paths). It is a
  `tools/` binary like `ivs_pack`, not a member of the package.
- Seven typed refusals, pinned append-only in `data/expected/flat-refusals.ids`,
  each beside its boundary positive: odd↔even length; a word above `$03FF`↔
  `$03FF`; 65,537↔65,536 words; base `$3FFF`↔`$4000`; an image crossing
  `$10000`↔ending at it; an output inside the work tree↔outside; an existing↔new
  output.
- The output is derived material of rights-bearing content: never committed
  (`*.ivsc` is already in `.gitignore` and `binary-ext.txt`), never a golden,
  never a fixture with `committed:true`, never created by CI, never a claim
  artifact, and no digest of it except on stdout.
- `.cfg` is refused **by name** — the refusal surface already exists in
  `binary-ext.txt` and `.gitignore` and must not be removed.

### The cartridge role in the owner-local route
- The route gains a cartridge role alongside the EXEC and GROM images. All six
  derived-output refusals and the snapshot rule fire for it, one line each and
  no file, and `docs/owner-images.md` stops saying the route has no cartridge
  role.
- The pass path is proved with project-made files: zero-filled 8192/2048-byte
  images are shape-legal, so the route's pass path runs end to end; a one-byte
  EXEC mutation proves the id can fail, and the `exec:` label stays `-` so
  nothing reads as a real-EXEC result.
- No new binary extension is tracked, and `binary-ext.txt` and `.gitignore` are
  unchanged by this phase — the existing refusal surfaces already cover `ivsc`
  and `cfg`.

### The interrupt-enable default (D-51, CPU-06)
- `cpu.interrupt_accept_clears_intff` becomes `1`, informed by the owner's own
  images through the D11 route. The field **stays** tagged
  `unknown-default-chosen`: an observation is not a hardware probe and mints no
  claim.
- The same commit carries: the profile parameter digest, **every** snapshot
  literal in `data/expected/ivsn-literals.ids`, the boot-stub-2 interrupt
  expectations (`run.stub2.jd-as-j`), the `state.read.describe-ok` and
  `state.literal.params-digest` expectations, and the C41 discrepancy note
  recording the observation. Measured by the milestone freeze; re-measure before
  landing.
- A project-authored CPU test pins **both** settings, so the not-cleared case
  stays exercised rather than being deleted with the default.
- The next blocker (the STIC register-window / strobe rule; the display-enable
  strobe never latches) gets its own row and its own gate. It is named, not
  fixed here and not silently carried.

### Gate cost and evidence discipline
- Priced addition for this phase: **1.3 hosted cpu-s** against the 21 cpu-s
  allowance. No new gate step, fault or golden: `steps=38`, `faults=238`,
  `goldens=29` are invariants.
- Same-commit retargets per GUARDRAILS V5 (`tools/flatwrap/*.c` → the `c-build`
  module list, the `ivs-flatwrap-sources.txt` writer, the toy source lists, an
  unlisted-source control, and the `docs/packaging.md` "not installed" line),
  V12 (the profile field → its tag and cite, the parameter digest, every
  snapshot literal, every expectation the behaviour moves, and the discrepancy
  note), and V7 (a test id → the pending line deleted, the toy registries
  extended, an orphan site failing).
- Every new equivalence gets a control whose stand-in fails it: the round trip's
  digest equality is proved live by flipping one input byte, and the pass path's
  id is proved able to fail by the one-byte EXEC mutation.
- No case may assert only `exit 0` / `outcome pass`.
- Cost checkpoint at the phase boundary: recompute the last-five medians from
  `.planning/ci-runs.tsv` and record exactly one of
  `discharged-within-allowance` / `reprice` / `lever-required`.

### Decided autonomously
- [The wrap tool] Q: "Where does the flat image's word order default come from?" → Selected: "`hi-first`, the project's one wire order, with `lo-first` accepted and nothing auto-detected" — a flag default is one line plus its test.
- [The wrap tool] Q: "Does the refusal list live in a new file or an existing inventory?" → Selected: "a new `data/expected/flat-refusals.ids`, pinned append-only, mirroring `faults/*.ids`" — an inventory file plus its reading id; undo is local.
- [The wrap tool] Q: "How is the output's 'never committed' rule enforced?" → Selected: "the existing `*.ivsc` entry in `.gitignore` and `binary-ext.txt`, plus a `files` id that would refuse a tracked one" — the surfaces already exist; no new mechanism.
- [The cartridge role] Q: "Which images does the pass path use?" → Selected: "zero-filled 8192/2048-byte files, shape-legal, so no owner image is needed" — test data, not a contract.
- [The interrupt-enable default] Q: "Where is the both-settings test pinned?" → Selected: "in the existing CPU test suite as a project-authored vector pair, not a new suite" — an id inside a suite that exists.
- [The interrupt-enable default] Q: "Is the next blocker's row a phase criterion or a record?" → Selected: "a record with its own gate, not a criterion — criterion 7 says named, not fixed" — a record edit.
- [Gate cost] Q: "Does the phase add a measurement run for the cost checkpoint?" → Selected: "no — recompute from the committed `.planning/ci-runs.tsv` rows" — the file is the arithmetic input.

### Claude's Discretion
None — every question above was frozen upstream or decided autonomously under the reversible-technical rule.

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- `tools/pack/` (`ivs_pack`) already writes `IVSC v1` from tracked listings; the wrap tool is a second, smaller writer of the same container, so the header and slot layout live in one place.
- `tools/owner-run.sh` already runs the owner-local route with EXEC and GROM roles; the cartridge role extends it.
- `data/expected/pkg-install-files.ids` (eleven paths) and `data/expected/pkg-selftests.ids` (19) are the packaging baselines that must not move.
- `data/profiles/intv-2609-ntsc-family-p1.json` carries `cpu.interrupt_accept_clears_intff` with its tag and cite; `data/memmap/<profile>.literal.txt` holds the map rows.
- `data/expected/ivsn-literals.ids` holds every snapshot literal; `data/discrepancies.json` holds C41.
- `tools/check/steps/{c-build,files,pkg-install,runner-process,state}.sh` are the steps the retargets land in.

### Established Patterns
- A refusal token is `<surface>-<rule>`; its positive case sits beside it in the same commit.
- A new test id deletes its pending line, extends the toy registries, and leaves an orphan site failing (GUARDRAILS V7).
- A profile field that moves behaviour moves its tag, cite, digest, every literal and every expectation in one commit (V12).
- Records are append-only; a live document gets an inline pointer plus an appended amendment.

### Integration Points
- `tools/flatwrap/` → `c-build`'s module list → an unlisted-source control.
- `data/expected/flat-refusals.ids` → the `pack` suite's refusal ids.
- `tools/owner-run.sh` → the cartridge role → `docs/owner-images.md`.
- `data/profiles/…json` → the parameter digest → `data/expected/ivsn-literals.ids` → `state` and `runner` expectations.

</code_context>

<specifics>
## Specific Ideas

- The upstream freeze is the spec: `.planning/MILESTONE-CONTEXT.md` §3 (D-47, D-51), §4 (verification strategy) and `.planning/research/v0.3.0-scoping-3-rights-format-packaging.md` R-2/R-5 are cited by row, not re-argued.
- The first real-content run's record (`.planning/MILESTONE-CONTEXT.md` §1a) is why the conformance work joined this phase: a host built before the fix is a black screen with a window around it.
- The observation mints no claim and no digest of any owner image is recorded anywhere.

</specifics>

<deferred>
## Deferred Ideas

- `.cfg` parsing and bank switching: FMT-01/FMT-04 stay `Deferred` with their written reversal conditions.
- The STIC register-window / strobe rule: recorded with its own row and gate, fixed in a later phase.
- Any host, shared object or libretro shim: Phase 15.
- Any real EXEC/GROM/ROM digest or committed content byte: never.

</deferred>
