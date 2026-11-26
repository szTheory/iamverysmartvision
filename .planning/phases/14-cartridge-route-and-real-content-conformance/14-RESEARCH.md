# Phase 14: Cartridge Route and Real-Content Conformance - Research

**Researched:** 2026-11-26
**Domain:** an in-tree C17 build-only writer of the project's own `IVSC v1` container; a shell route's third content role; a profile-field retarget with its digest and literal consequences
**Confidence:** HIGH for every retarget figure (measured on this tree this session); HIGH for the container layout and the runner's refusals (read from source); MEDIUM for two open design choices (the work-tree refusal mechanism and the `ivs_cp1610_config_default` question), both of which change the delta and must be decided in the plan.

## Summary

Three deliverables, and the second one carries the risk. The wrap tool is small and its
inputs are three facts the operator states; the container it must write already has a
single source of truth in the tree (`core/src/content/ivsc.h`) and a single digest
implementation (`common/ivs_sha256.h`), both reachable from a build-only tool through the
existing `ivs_core_internal` interface target — so no layout is duplicated and no installed
header moves. The cartridge role is mostly *proving* what already fires: the runner already
accepts `--cart` and already refuses all six derived outputs for **any** `content ... user`
role, measured below. The real work is `tools/owner-run.sh` (two images today, a
two-role lock, and a test that asserts the role `cart` is *refused*) and the doc line that
says the route has no cartridge role.

The third deliverable is the one that must not be guessed. Flipping
`cpu.interrupt_accept_clears_intff` to `1` and rebuilding the dev preset produced **nine**
failing test ids, not the three the frozen guardrail names (V12 names
`state.read.describe-ok`, `state.literal.params-digest`, `run.stub2.jd-as-j`). Two more
(and five more again, if `ivs_cp1610_config_default` moves with the profile) appear in
`stic`, `run` and `cpu-vectors`. Every literal that must move was measured, not quoted:
the new profile parameter digest is `efba85009a57e91ad21060742dadc34949b976829525bb5e18d6b0897af1425c`
and the four snapshot literals are pinned below. `run.stub2.jd-as-j` does not merely change
an expectation — its *control premise* ("a plain J is a trap") is what the flip removes, so
the retarget must be a re-expression of the control, not a number edit.

**Primary recommendation:** build the writer against `content/ivsc.h` + `ivs_core_internal`,
land the seven refusals as seven `pack` ids that each assert the refusal *and* its boundary
positive in one case, prove the cartridge role with one `runner-process` id that asserts all
six derived-output refusals plus one round-trip id, retarget the profile field with the
measured nine-id list (plus five if the C default moves), and keep `ivs_cp1610_config_default`
at `0` only if the plan can say why the library default may disagree with the one registered
profile.

---

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|---|---|---|---|
| Reading the operator's flat file, pairing bytes, checking the word range | Build-only tool (`tools/flatwrap/`) | — | It is a file-to-file converter with no machine state; the core never sees a path |
| Writing the `IVSC v1` container bytes | Build-only tool, over the core's **internal** layout header | Core (`content/ivsc.h`, `common/ivs_sha256.h`) | One definition of the header and slot layout; the writer reads constants, the parser reads the same ones |
| Deciding whether a container is legal | Core parser (`core/src/content/ivsc.c`) | — | Already specified and fuzzed; the writer must not re-implement a checker |
| Refusing an output inside the work tree | Build-only tool | — | A staging hazard, not a machine rule; `owner-run.sh` is the precedent |
| Accepting a cartridge from the operator | Runner CLI (`runner/main.c`) | Core load (layer 2, `machine_load.c`) | The runner already has `--cart` and `content cart user`; the route only forwards a path |
| Refusing derived outputs for user content | Runner (`runner/main.c`, one loop over all roles) | — | Already role-generic: measured to fire for `content cart user` with no code change |
| Resolving the interrupt-enable default | Profile data (`data/profiles/*.json`) → generated memmap table → parameter digest | CPU layer (`cp1610.c` reads `config`) | The value is profile data; the C default is a separate library surface |
| Recording the next blocker | Record (`data/discrepancies.json`) + `STATE.md` Deferred row | — | The observation is not a claim and mints no tier |

---

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

**The wrap tool states bytes, never a format**
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

**The cartridge role in the owner-local route**
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

**The interrupt-enable default (D-51, CPU-06)**
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

**Gate cost and evidence discipline**
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

### Claude's Discretion
None — every question above was frozen upstream or decided autonomously under the reversible-technical rule.

### Deferred Ideas (OUT OF SCOPE)

- `.cfg` parsing and bank switching: FMT-01/FMT-04 stay `Deferred` with their written reversal conditions.
- The STIC register-window / strobe rule: recorded with its own row and gate, fixed in a later phase.
- Any host, shared object or libretro shim: Phase 15.
- Any real EXEC/GROM/ROM digest or committed content byte: never.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| TOOL-07 | wrap a flat image at a stated address into the cartridge container with one build-only command; every malformed input refused by a typed token with a positive case beside it; the output is derived material, never committed/golden/fixture/claim artifact | The layout constants to reuse (§Pattern 1), the seven tokens and their boundary arithmetic incl. the one pair that cannot be an acceptance (§Pattern 2, §Pitfall 2), the exclusive-create requirement (§Pitfall 3), the measured round-trip container (§Code Examples 1–2), and the rights-token scopes the tool's own source is scanned by (§Pitfall 6) |
| SCN-03 | the owner-local route accepts a cartridge image alongside EXEC and GROM; every derived-output refusal extended to it; the pass path exercised on project-made files | The route's current two-image shape and its lock grammar (§Pattern 4), the measured six refusals that already fire for `content cart user` (§Code Example 3), the measured zero-filled pass-path outcome (§Code Example 4), and the lock-role test that a third role would break (§Pitfall 5) |
| CPU-06 | the first real-content blocker resolved from an owner-local observation; a project-authored test pins both settings; the value stays `unknown-default-chosen` with the observation in the log; the digest, every snapshot literal and the boot-stub-2 interrupt expectations move in the same commit | The exact field record and its cite (§Pattern 5), the measured nine-id failure set and the four new literals (§Pattern 5, §Pitfall 1), the five extra ids if the C default moves (§Pitfall 4), the C41 note shape (§Pattern 6) and the two existing both-values pins (§Pattern 7) |
</phase_requirements>

## Standard Stack

**No new dependency, and no package to install.** This phase adds one C17 translation unit
inside the tree and edits shell/CMake/data files. There is nothing to fetch from a
registry, so the Package Legitimacy Audit is recorded as not applicable rather than empty.

### Core

| Library | Version | Purpose | Why Standard |
|---|---|---|---|
| `ivs_core_internal` (INTERFACE target, `core/CMakeLists.txt:138-139`) | in-tree | exposes `core/src` as an include dir so a tool can read the internal headers | the same target `tools/pack` already links; it installs nothing |
| `content/ivsc.h` (`core/src/content/ivsc.h:20-31`) | in-tree | the container's own constants: magic length, prefix/entry/trailer sizes, per-segment and total word limits, version, width, word max, address-space words | one definition shared by the parser and both writers; not installed, not an ABI |
| `common/ivs_sha256.h` (via `ivs_core`) | in-tree | the trailer digest | `ivs_pack` already links the core archive for exactly this |
| `ivs_warnings`, `ivs_san` | in-tree | the project's warning set and sanitizer flags | every tool target links them |

### Supporting

| Library | Version | Purpose | When to Use |
|---|---|---|---|
| `cmake -E sha256sum` | host CMake ≥ 3.25 | an implementation *other than the core's* for the trailer check | the suite's trailer check and the printed-hash comparison; never inside the tool |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|---|---|---|
| `content/ivsc.h` constants | a private copy of the 8-byte magic and the offsets inside `tools/flatwrap/` | duplicates the layout the phase exists to share; the round-trip test would catch drift only after the fact |
| a new installed header under `core/include/ivs/` | — | moves `data/expected/pkg-install-files.ids` from eleven paths to twelve; forbidden by D-49 and by success criterion 1 |
| a new `docs/formats/*.md` page for the wrap | — | the tool "names no other tool's format"; the container page already exists (`docs/formats/ivsc-v1.md`) and the tool adds no format |

**Installation:**
```bash
# nothing to install: the phase adds no dependency.
cmake --preset dev && cmake --build --preset dev --target ivs_flatwrap
```

**Version verification:** not applicable — no third-party package is added. The in-tree
symbols were read this session and are quoted verbatim in §Pattern 1.

## Package Legitimacy Audit

**Not applicable.** This phase installs no external package: the writer is one C17 file
inside `tools/flatwrap/`, linking two in-tree targets (`ivs_core`, `ivs_core_internal`) that
already exist and are already linked by `tools/pack` (`tools/pack/CMakeLists.txt:27-30`).
No `npm`/`PyPI`/`crates` package name appears anywhere in this research, so there is nothing
to run the legitimacy gate over. If a plan ever proposes a third-party dependency for this
phase, that proposal contradicts D-47 and the project's dependency rule
(`AGENTS.md`: "Prefer a small dependency tree … Add a dependency for a concrete capability
whose maintenance cost is justified").

## Architecture Patterns

### System Architecture Diagram

```
                     operator states three facts, nothing is detected
   ┌──────────────┐   <flat> bytes   <base> $4000..$FFFF   --word-order hi-first|lo-first
   │ operator's   │───────────────────────────────┐
   │ flat file    │                               │
   └──────────────┘                               ▼
                                    ┌──────────────────────────────┐
                                    │ tools/flatwrap (build-only)  │
                                    │  1 length: odd? -> refuse    │
                                    │  2 words: > 65,536 -> refuse │
                                    │  3 base in $4000..$FFFF      │
                                    │  4 addr+words > $10000       │
                                    │  5 output dir inside tree    │
                                    │     (except refs/) -> refuse │
                                    │  6 output exists -> refuse   │
                                    │  7 a word > $03FF -> refuse  │
                                    └───────────────┬──────────────┘
                       constants from content/ivsc.h│  digest from common/ivs_sha256.h
                                                    ▼
                                     <out>.ivsc  (24 prefix + 8 entry + 2n words + 32 trailer)
                                     stdout: one 64-hex digest of the whole file
                                                    │
                       ┌────────────────────────────┴───────────────────────────┐
                       ▼                                                        ▼
   ┌──────────────────────────────────┐                    ┌──────────────────────────────┐
   │ suite pack (ids)                 │                    │ runner / owner-local route   │
   │  refusal token + boundary posit. │                    │  --cart <file>               │
   │  round-trip parse by the core?   │                    │  content cart user           │
   │  no: the load is proved by       │                    │  slot.cart.sha256 == printed │
   │  runner-process instead          │                    │  six derived outputs refused │
   └──────────────────────────────────┘                    └──────────────────────────────┘
```

### Recommended Project Structure

```
tools/flatwrap/
├── CMakeLists.txt        # IVS_FLATWRAP_SOURCES + the ivs-flatwrap-sources.txt writer
└── flatwrap.c            # the whole tool: one read, six checks, one write, one stdout line
data/expected/
├── flat-refusals.ids     # the seven tokens, append-only, one per line
└── tests/pack.ids        # + one id per refusal, each asserting its boundary positive
docs/
├── packaging.md          # the "not installed" line gains the tool
└── owner-images.md       # the cartridge role, the template, the snapshot rule
tests/
├── pack-tap.sh           # + flatwrap argv, + the refusal/positive cases
└── runner-process-tap.sh # + the cartridge role, the six refusals, the round trip
tools/owner-run.sh        # + an optional --cart <file>
```

### Pattern 1: a second writer shares the layout through the core's *internal* header

**What:** `core/src/content/ivsc.h` already carries every constant a writer needs, and
`ivs_core_internal` already publishes `core/src` as an include directory. `tools/pack`
proves the shape: it links `ivs_core` (for the digest body) and `ivs_core_internal` (for the
header) and includes `"common/ivs_sha256.h"`.

**When to use:** for the wrap tool, and only for the wrap tool. It writes exactly one
segment, so it needs: the prefix size, the entry size, the trailer size, the version, the
width, the word max, the address-space words, and the magic.

**Example — the constants, verbatim (`core/src/content/ivsc.h:20-31`):**

```c
#define IVS_IVSC_MAGIC_BYTES 8u
#define IVS_IVSC_PREFIX_BYTES 24u
#define IVS_IVSC_ENTRY_BYTES 8u
#define IVS_IVSC_TRAILER_BYTES 32u
#define IVS_IVSC_MAX_SEGMENTS 16u
#define IVS_IVSC_MAX_SEGMENT_WORDS 65535u
#define IVS_IVSC_MIN_BYTES 66u     /* prefix 24 + one entry 8 + one word 2 + trailer 32 */
#define IVS_IVSC_MAX_BYTES 131256u /* prefix 24 + 16 entries 128 + 65536 words 131072 + 32 */
#define IVS_IVSC_VERSION 1u
#define IVS_IVSC_WIDTH_BITS 10u
#define IVS_IVSC_WORD_MAX 0x03FFu
#define IVS_IVSC_ADDRESS_SPACE_WORDS 0x10000u
```
[VERIFIED: core/src/content/ivsc.h:20-31]

**The one gap:** the 8 magic **bytes** are a `static const` inside `ivsc.c`
(`core/src/content/ivsc.c:17`), not in the header:

```c
static const uint8_t ivsc_magic[IVS_IVSC_MAGIC_BYTES] = {0x89u, 0x49u, 0x56u, 0x53u, 0x43u, 0x0Du, 0x0Au, 0x1Au};
```
[VERIFIED: core/src/content/ivsc.c:17]

Two honest options, both one-line: (a) restate the eight bytes in `flatwrap.c` with a
comment pointing at the spec — the round-trip test fails immediately if they drift; or
(b) move the array to a declaration in `content/ivsc.h` and keep the definition in
`ivsc.c`. (b) is the "one place" the CONTEXT asks for, and it changes no installed header
and no symbol visibility rule (`ivs_` prefix, `core-symbols` allow list). Recommend (b),
and note that `tools/pack`'s own `pk_ivsc()` restates the same eight bytes today
(`tools/pack/pack.c:298`), so (b) also retires one existing duplication.

**The layout, verbatim from the spec (`docs/formats/ivsc-v1.md`, "Layout"):**

| Offset | Size | Field | Rule |
|---|---|---|---|
| 0 | 8 | magic | exactly `89 49 56 53 43 0D 0A 1A` |
| 8 | 2 | version | 1 |
| 10 | 2 | flags | 0 |
| 12 | 4 | required_features | bit mask; v1 defines no bit |
| 16 | 4 | total_bytes | the size of the whole container, trailer included |
| 20 | 2 | segment_count | 1 to 16 |
| 22 | 2 | reserved | 0 |
| 24 | 8 per segment | segment table | one entry per segment, in table order |
| 24 + 8n | 2 per word | data | the words of every segment, segment after segment |
| total_bytes - 32 | 32 | trailer | SHA-256 of every byte before the trailer |

and the 8-byte entry: `addr` (2), `len_words` (2, 1 to 65,535), `width` (1, 10), `attr`
(1, 0), `reserved` (2, 0). [CITED: docs/formats/ivsc-v1.md §Layout]

### Pattern 2: a refusal token plus its boundary positive, in one id, with a documented check order

**What:** the project's refusal-surface pattern is: a `<surface>-<rule>` token, one case per
token, a coverage case that diffs the pinned list against the tokens the source can print,
and a *boundary positive* beside the refusal. `ivs_asm` is the model
(`data/expected/asm-tokens.ids`, `tests/asm-tap.sh:273-330`, `asm.neg-coverage`,
`asm.boundary.branch-03ff` with `tests/asm/pos/branch-03ff.asm`).

**When to use:** for all seven refusals, and read the order as part of the contract: the
container's own parser documents a fixed check order and states that "the first failing
check decides the refusal" (`docs/formats/ivsc-v1.md`, "Check order"). flatwrap must state
its order too, because two of its rules overlap (see Pitfall 2).

**The seven rules and their arithmetic, with the base range `$4000`–`$FFFF`:**

| # | Rule | Refused | Boundary positive |
|---|---|---|---|
| 1 | odd byte length | 1 byte (and any odd count) | 2 bytes → one word |
| 2 | a word above `$03FF` | bytes `04 00` | `03 FF` |
| 3 | more than 65,536 words | 65,537 words (131,074 bytes) | 65,536 words is **not** refused by this rule — see Pitfall 2 |
| 4 | base outside `$4000`–`$FFFF` | `3FFF` | `4000` |
| 5 | `base + words > $10000` | 49,153 words at `$4000` | 49,152 words at `$4000` (ends at `FFFF`) |
| 6 | output inside the work tree (except `refs/`) | `$ROOT/build/x.ivsc` | `$TMPDIR/x.ivsc` |
| 7 | output already exists | an existing path of any kind | a new path |

The arithmetic for rows 3 and 5 is the load-bearing part: `$10000 - $4000 = $C000 =
49,152` words is the largest image any accepted base can hold, so rule 5 always fires
before rule 3 can be *satisfied*; and `IVS_IVSC_MAX_SEGMENT_WORDS` is `65535`, so a
one-segment writer can never emit a 65,536-word segment anyway.

### Pattern 3: an in-tree tool's CMake shape (the source-list writer)

**What:** each build-only tool writes its own repo-relative source list to the build tree so
the `c-build` step can find a tracked `tools/<x>/*.c` the build does not compile. The block
is identical in five directories; copy it.

**Example, verbatim (`tools/asm/CMakeLists.txt`, tail):**

```cmake
set(listed "")
foreach(src IN LISTS IVS_ASM_SOURCES)
  string(APPEND listed "tools/asm/${src}\n")
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/ivs-asm-sources.txt.tmp" "${listed}")
file(COPY_FILE "${CMAKE_BINARY_DIR}/ivs-asm-sources.txt.tmp" "${CMAKE_BINARY_DIR}/ivs-asm-sources.txt"
  ONLY_IF_DIFFERENT)
```
[VERIFIED: tools/asm/CMakeLists.txt, tail]

**The full same-commit set for a new `tools/flatwrap/*.c`** (GUARDRAILS V5, corrected
against the tree — see Pitfall 8 for the item V5 does not name):

1. `CMakeLists.txt` — `add_subdirectory(tools/flatwrap)` beside the five existing tool
   subdirectories (`CMakeLists.txt:36-42`).
2. `tools/flatwrap/CMakeLists.txt` — the target, the link line
   (`ivs_core ivs_core_internal ivs_warnings ivs_san`) and the source-list writer above.
3. `tools/check/steps/c-build.sh:52-55` — the `for mod in …` list gains
   `'^tools/flatwrap/.*[.]c$ ivs-flatwrap-sources.txt'`; the header comment's enumeration
   (lines 3-9) gains the directory.
4. `tools/check/controls/_good/tree/CMakeLists.txt` — one more
   `file(WRITE "${CMAKE_BINARY_DIR}/ivs-flatwrap-sources.txt" "")` (empty, like `pack` and
   `bench`), plus the comment listing the six lists.
5. `tools/check/controls/c-build-unlisted-flatwrap-source/` — `CHECK` = `c-build`,
   `EXPECT` = `FAIL c-build-unlisted-source: tools/flatwrap/extra.c`, and
   `overlay/tools/flatwrap/extra.c` (SPDX header, no refused spelling in the comment).
6. `data/expected/controls/` — the control's id registered in the file that owns its step
   (`data/expected/controls/rights.ids` is where the two `files-*` and the `c-build-*`
   entries do not live; check which file owns `c-build-unlisted-asm-source` before
   appending).
7. `docs/packaging.md:38` — the "not installed" sentence gains the tool.
8. `tools/check/integrity.cmake:70-72` — the SPDX alternation
   `^tools/(pack|cart|pcm|asm|bench)/` gains `flatwrap`, plus its control (Pitfall 8).

### Pattern 4: the route's role shape and its lock grammar

**What:** `tools/owner-run.sh` takes exactly two positional image paths, checks CI, then
arguments, then the skip, then location, then shape, then the lock, then the runner and the
template.

**The two lines the phase must change, verbatim (`tools/owner-run.sh`):**

```sh
usage='usage: owner-run.sh [--preset dev|release] [--record] <exec-image> <grom-image>'
...
    *) die 'bad lock line: the role is not exec or grom' ;;
```
[VERIFIED: tools/owner-run.sh, usage line and the lock role arm]

and the shape check:

```sh
sized() { [ -f "$2" ] && [ "$(wc -c < "$2" | tr -d ' ')" = "$3" ] || die "the $1 image is not a regular file of exactly $3 bytes"; }
sized exec "$ex" 8192
sized grom "$gr" 2048
```
[VERIFIED: tools/owner-run.sh, `sized` and its two calls]

**Recommendation:** add `--cart <file>` as an *optional* option (never a third positional:
nine existing ids pass exactly two positionals), forward it as `ivs-run --cart <file>`, and
**do not** add `cart` to the lock. Reasons, each checkable:

- The lock's grammar is asserted against a `cart` role today, and that assertion is a live
  id: `tests/runner-process-tap.sh:1930`
  `or_badlock 'owner-run: bad lock line: the role is not exec or grom' "$or_z  exec" "$or_z  cart"`
  [VERIFIED: tests/runner-process-tap.sh:1930]. Extending the lock to three roles
  *invalidates the case's example*, so the phase would have to re-point it at another bogus
  role — an extra edit for no capability.
- The container's identity is already in the report (`slot.cart.sha256`), and SCN-03 asks
  for the role, not for a lock entry.
- The template's marker insertion asserts `grep -c '^expect-sha256 ' == 2`; leaving the lock
  at two roles keeps that check intact and the awk line unchanged.
- No route-level size check for the cartridge: the core already refuses a short container
  (reason 1) and a long one (reason 2) at load with a precise report line, so a second
  shape rule would be a second registration of one refusal. The exec/grom exact sizes exist
  because those shapes are exact and the core's own messages are coarser there.

### Pattern 5: the profile-field retarget (V12) — the measured set

**What:** the field record, verbatim (`data/profiles/intv-2609-ntsc-family-p1.json:174-183`):

```json
    "cpu.interrupt_accept_clears_intff": {
      "value": 0,
      "tag": "unknown-default-chosen",
      "cite": "SRC-gi-cp1600-um-1975 pdf 22, pdf 56: the pages do not say that accepting an interrupt clears the interrupt enable; not-cleared is the chosen default",
      "conflict": "C41",
      "resolving_evidence": "measurement: D10 repeated-interrupt probe on owner hardware",
      "revisit_trigger": "when a source documents the enable state after acceptance or a hardware probe exists",
      "decision": null,
      "note": "0 = false, 1 = true. Stored as a number because the profile schema has no boolean value type."
    },
```
[VERIFIED: data/profiles/intv-2609-ntsc-family-p1.json:174-183]

**How the field reaches the digest.** It is a *registry scalar* of the memory-map table, not
a map row:

```cmake
  set(MM_CTX "${rel}: cpu.interrupt_accept_clears_intff")
  mm_uint(intff "${doc}" "value" 1 fields cpu.interrupt_accept_clears_intff value)
```
[VERIFIED: cmake/IvsMemmapTable.cmake:302-303]

so it lands in `ivs_mm_profiles[]` (the `${reg_c}` block that is part of `mm_body`) and
therefore in the memmap table body digest, which is one of the four inputs of the parameter
digest:

```c
  d[0] = ivs_memmap_table_digest(profile_index);
  d[1] = ivs_psg_table_digest(profile_index);
  d[2] = ivs_stic_table_digest(profile_index);
  d[3] = ivs_ctrl_table_digest(profile_index);
  ...
  ivs_sha256_digest(pre, sizeof pre, out);
```
[VERIFIED: core/src/machine/machine_snapshot.c:99-121, the four table digests and the
134-byte `IVSP`+be16(1) preimage]

**Measured retarget table (this session; method in §Code Example 5):**

| Artifact | Where | Now | After the flip |
|---|---|---|---|
| `value` | profile JSON:175 | `0` | `1` |
| `cite` | profile JSON:177 | "…not-cleared is the chosen default" | must state the owner-local observation as the basis and still say no source exists (the tag does not change) |
| `note` | profile JSON:182 | "0 = false, 1 = true…" | should record which value is chosen and why, still without a claim |
| memmap body digest | generated (`IvsMemmapTable.cmake:763`) | `b16e3672c67dbb224dac624f6b7eb42c12f3b683e5ea2dc8917008256c172771` | `c6e522ee0e42bebdb7db37858355511cade9ad29db6af05617b88608212413ed` |
| `params-digest-sha256` | `data/expected/ivsn-literals.ids:49` | `89ad65073a3e357acbdd687f6a29016457864a1666b953e9327b7559dc0c0522` | `efba85009a57e91ad21060742dadc34949b976829525bb5e18d6b0897af1425c` |
| `stepped-snapshot-sha256` | `ivsn-literals.ids:57` | `087a15d3fce834b1d944bbbd74385d029e42eb4a4f9f0fbb322d754df0755b6b` | `95dd98cbb5221c99f189173c54d72bde17409ea98ce05616c01a6b1d5a363bcc` |
| `cut-snapshot-sha256.hp-frame-mid` | `ivsn-literals.ids:61` | `9dc4ae7e20a746b4b8d6638cb14305ce3a8d4ec9bd4da872916353272c68567f` | `82fd4438b78390cea19056722bb30cbe1759aae9f7150c288088262b9818478b` |
| `cut-snapshot-sha256.hp-latch` | `ivsn-literals.ids:62` | `63507cb48fa4bbf42cfebb99e07fa5d722db44797641c3404118c1a9826cee1d` | `1eb9c2cd15f6a7da80d5b88138c04cf1db323bd278a60a823a7d13ea58f8662b` |
| `cut-snapshot-sha256.hp-mob` | `ivsn-literals.ids:63` | `27e98b02b605d6f201d2745ebf35c3e2a137d62cc9d892b9d0eb7322f09bc768` | `ffdf3d0e4b8793e953cb3911f77bf2314267e32c7409d92d05bedc250eb9bc55` |

[VERIFIED: measured — the current column reproduces the pinned literals exactly, which is
the check that the method is right; the "after" column is the same computation with the
field at 1 and a rebuild of the dev preset]

**What does *not* move** (also measured, and worth writing into the plan so the commit
diff is defensible):

- `data/memmap/intv-2609-ntsc-family-p1.literal.txt` — the file "names no profile field"
  and holds map rows only (`data/memmap/…literal.txt:10-13`, and the file has no
  `intff`/`reset`/`iab` line). V12's "if the field is a map row" does not apply.
- `data/expected/profile-fields.ids` — it is a list of field *paths*; the path does not
  change (`cpu.interrupt_accept_clears_intff` is line 28).
- `data/expected/digest-v1-literals.ids`, `digest-v2-literals.ids` — those pin older
  *report* digest values for cross-version comparison; no report digest moves.
- every `data/golden/*.digests` and `data/expected/pkg-consumer.golden` — the `runner`,
  `machine` and `runner-process` suites all stayed green with the field at 1, so no report
  digest line moves (the snapshot's parameter digest is not a report key).
- `data/expected/state-checklist.ids`, `state-cuts.ids` — cut ticks and row classifications
  are unchanged; only the snapshot *bytes* change.

**The failure set, measured twice (field at 1, dev preset rebuilt, `ctest --preset dev`):**

| Suite | id | Why it fails | Disposition |
|---|---|---|---|
| state | `state.literal.params-digest` | the core's digest ≠ the pinned literal | re-pin |
| state | `state.literal.header-structure` | "comparison 6 failed … the parameter digest differs from params-digest-sha256" | collateral: passes again after the re-pin |
| state | `state.read.describe-ok` | `memcmp(rep.r.profile_params_sha256, params, …)` at `tests/state/state_read.c:1212` | collateral on the re-pin |
| state | `state.save.canonical-stream-literal` | the stepped snapshot digest moved | re-pin |
| state | `state.restore.roundtrip-fixed-point` | it re-checks the same `stepped-snapshot-sha256` (`tests/state/state_restore.c:1262-1282`) | collateral on that re-pin |
| state | `state.judge.header-literal-corrupt` | its damaged copies now trip comparison 6 before 7…10 | collateral on the params re-pin |
| state | `state.cut.fixed-point-and-canonical` | three cut snapshot digests moved | re-pin (3 literals) |
| run | `run.stub2.jd-as-j` | `check failed at line 315: f.entries > 1u` | **re-express the control**, not a number edit (Pitfall 1) |
| stic | `stic.irq.level-held-reentry-c41-0` | `1 entries in frame 0 (closed form 251 …)`, i.e. the re-entry storm is gone | re-target: it is the C41=0 profile pin, now false at the profile level |

The frozen V12 row names three of these nine. The plan must carry all nine, and the
`13-…`/`14-…` summary must say that V12's measured list was incomplete and was recomputed —
"recompute, never quote" (`AGENTS.md`).

### Pattern 6: recording an observation in a discrepancy row without closing it

**What:** C41 already carries the observation in its `note`, appended without touching
`status` or `default_policy`:

> "Observation, 2026-11-26 (owner-local, the D11 route; no digest of any owner image is
> recorded). With the owner's own EXEC, GROM and an 8K cartridge loaded through the public
> API, the default 0 produced an interrupt storm … This is an observation, not a claim: the
> value stays unknown-default-chosen until the hardware probe above exists…"
> [VERIFIED: data/discrepancies.json, C41 `note`]

**The row's own rules** (`tools/check/schema/discrepancy.txt`): keys are exactly
`id topic sources status default_policy resolver evidence note`, `status` ∈
`open|partial|resolved-primary|withdrawn`, `resolver` must match `^(test|measurement|pending):.+`,
and an `open` or `partial` row's `default_policy` must start `unknown-default-chosen: `.

**Therefore:** keep C41 `status: "open"`, keep the `unknown-default-chosen:` prefix on
`default_policy`, and edit `default_policy` because it currently states the value:
"…profile field cpu.interrupt_accept_clears_intff = 0, with vectors for both values"
[VERIFIED: data/discrepancies.json, C41 `default_policy`]. The value in that sentence must
become 1 while the row stays open, otherwise the log contradicts the profile. The `note`
then gets an appended amendment recording that the field is now 1 and why, exactly as the
existing observation paragraph did. `evidence` may be extended the same way C41's was.

### Pattern 7: pinning both settings of a CPU config bit

**What:** the CPU vector corpus already parameterises this exact field. The harness applies
an override only when the vector's `config_mask` bit 0 is set:

```c
  ivs_cp1610_config_default(&cfg);
  if (v->config_mask & 1u) {
    cfg.interrupt_accept_clears_intff = v->config.interrupt_accept_clears_intff;
  }
```
[VERIFIED: tests/cpu/cpu_vectors.c:153-156]

and the harness struct documents the bit:

```c
  uint8_t config_mask;       /* bit 0: interrupt_accept_clears_intff, 1: release latency, 2: wait limit */
```
[VERIFIED: tests/harness/ivs_vec.h:44]

**Both settings already exist in the corpus:**

- `cpu.UNIT-INT.accept-clears-intff-config` — `config: {"interrupt_accept_clears_intff": 1}`,
  `init.intff = 1`, `expect.intff = 0`.
- `cpu.J.je-then-intrm-immediate`, `cpu.EIS.then-intrm-after-next`, `cpu.DIS.defers-intr`,
  `cpu.UNIT-RESET.*` — `expect.intff` values that rest on the *default* 0.

**So the "both settings" pin has two possible expressions, and the choice is the phase's
one real decision here:**

- **(a) leave `ivs_cp1610_config_default` at 0** (measured cost: 0 extra ids). The
  not-cleared case stays exercised by the five default-0 vectors. But the library default
  then contradicts the only registered profile, and its comment says so out loud
  (`core/src/cpu/cp1610.c:231-233`: "Profile intv-2609-ntsc-family-p1 (plan 02-01):
  cpu.interrupt_accept_clears_intff = 0") [VERIFIED], so the comment must be re-written to
  say "the C default, which the machine overrides from the profile" — and a reader of the
  public header `ivs_cp1610_config_default` gets a value the profile does not use.
- **(b) flip `ivs_cp1610_config_default` to 1** (measured cost: **5 extra failing ids**:
  `cpu.EIS.defers-intr`, `cpu.EIS.then-intrm-after-next`, `cpu.GSWD.intff-unchanged`,
  `cpu.J.je-then-intrm-immediate`, `cpu.UNIT-INT.intrm-intff-set-accepted`) and add
  `"config": {"interrupt_accept_clears_intff": 0}` to those vectors so the not-cleared case
  is pinned *explicitly* rather than by an implicit default. This is the reading that
  matches criterion 6's "pins **both** settings so the not-cleared case is still
  exercised": under (a) the 0 case would silently follow whatever the C default becomes.

Recommend (b), and register the five vectors' `covers` gain (`config:interrupt-accept-clears-intff`)
only if the plan wants the tag; `vprove` returns "not understood" (empty) for an unknown tag
prefix, so a new covers tag is inert, not an error.

**The STIC suite already has the both-values idiom**, which is the pattern to copy if the
plan prefers a suite-level pin:

```c
      uint32_t got = si_rig_entries(c41, dis, &t0);
      uint32_t expect = (c41 == 0u && dis == 0u) ? si_reentry_closed_form(p, t0, 15u * (uint64_t)p->ticks_per_cpu_cycle) : 1u;
```
[VERIFIED: tests/stic/stic_irq.c, `si_c41_both_values`, which builds a configuration copy
per (c41, dis) pair]

### Anti-Patterns to Avoid

- **Writing a second container *checker* in the tool.** The parser is specified, fuzzed and
  fault-covered; the tool's job is to refuse its own malformed *inputs*, not to validate the
  container it just wrote.
- **Auto-detecting the word order from the input.** Forbidden by D-47; a heuristic here is a
  claim about someone else's dump.
- **A third positional argument on `owner-run.sh`.** It breaks nine ids and the documented
  usage string.
- **Editing `data/expected/ivsn-literals.ids`'s frozen sections.** The file's own convention
  is that earlier sections are frozen oracle data and later plans *append*; a re-pin edits a
  value line deliberately, with the recipe in the commit message — not a rewrite.
- **Adding a gate step, fault or golden.** 38 / 238 / 29 are invariants.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---|---|---|---|
| the container header and slot layout | a private copy of offsets and sizes | `content/ivsc.h` via `ivs_core_internal` | one definition; the parser and both writers then cannot disagree |
| SHA-256 | any digest code | `common/ivs_sha256.h` (already linked by `ivs_pack`) | audited, fault-covered, and the trailer must match the core's own computation |
| an independent check of the trailer | the core's own SHA-256 again | `cmake -E sha256sum` on the container minus its last 32 bytes | the `pack` suite already does exactly this for `pack.trailer-<id>` |
| an exclusive-create output file | `fopen(path,"wb")` then a `stat` race | `open(path, O_CREAT|O_EXCL…)` as `runner/hostio.c:137` does, or C11 `fopen(path,"wbx")` | "existing ↔ new output" is one of the seven refusals; a truncating open destroys the file it was supposed to refuse |
| an "am I inside the tree" test in shell | a new helper script | the pattern in `tools/owner-run.sh` (physical directory, `cd -P`, prefix compare) | it is already the project's answer, and its own ids cover the `..` and `-x` spellings |
| a both-settings CPU test from scratch | a new suite | `config_mask` bit 0 in the vector corpus, or `si_rig_entries(c41, dis, …)` in the STIC suite | both already exist and are already parameterised |

**Key insight:** in this phase almost every mechanism already exists; the work is wiring one
new file into five lists and *measuring* one field's blast radius. Anything built fresh here
is a second registration of an existing proof, which the project counts as cost without
evidence.

## Runtime State Inventory

Not applicable: this is not a rename, rebrand or migration phase. The one name-shaped
change is a profile *value* (0 → 1) plus its cite, and the state it lives in is tracked
(`data/profiles/`, `data/expected/`, `data/discrepancies.json`); no database, service
configuration, OS registration, secret name or installed artifact carries it. The only
runtime state the phase creates is `build/` output (the tool binary and its source list),
which is regenerated and gitignored.

## Common Pitfalls

### Pitfall 1: `run.stub2.jd-as-j` is a control whose *premise* the flip removes

**What goes wrong:** the id fails at `f.entries > 1u` (measured). Its stated purpose is
"the control STUB2_JD_AS_J (A-17) … shows more than one interrupt entry in the frame,
interrupts still on after the vector, and R6 above 0341 at the end of the frame: a plain J
is a trap (D-10)" (`tests/run/run_stub2.c:17-21`) [VERIFIED]. With acceptance clearing the
enable, one entry is all the frame gives, `vec_intr` is 0 and the storm is gone — so a plain
J is *no longer a trap by itself*. Editing the expectation to "one entry" would leave a
control that asserts the same thing as `run.stub2.handoff` and proves nothing.

**Why it happens:** the control was written to detect the *consequence* of C41=0, and the
flip deletes that consequence rather than changing a number.

**How to avoid:** re-express the control so it still discriminates. The measured facts
(one entry, enable clear after acceptance, R6 back at 0341) mean the discriminating case is
now a handler that **re-enables with EIS and never disables**: with `JD` the vector's own
disable still wins for the rest of the frame, with `J` the handler's EIS re-arms and the
level is taken again. So the mutant should stay `J` but the probe must re-arm *before* its
wait (the existing probe already does `EIS` at `$5000`), and the assertion becomes a
comparison *against* the `JD` run in the same frame rather than an absolute count. Decide
the shape by measuring, not by editing the failing line.

**Warning signs:** a retarget whose new expectation equals another id's expectation; a
control whose name still says "trap" while nothing traps.

### Pitfall 2: the 65,536-word boundary cannot be an acceptance

**What goes wrong:** criterion 2 asks for "65,537↔65,536 words" beside the other six pairs,
and every other pair is refuse↔accept. For this one, an acceptance is impossible twice over:

- `IVS_IVSC_MAX_SEGMENT_WORDS` is `65535u` and `len_words` is a 2-byte field, so a
  one-segment writer cannot encode 65,536 words at all;
- `$4000 + 65536 = $14000` crosses `$10000`, so the crossing rule refuses it first anyway.

**Why it happens:** the number 65,536 is the *container's total* word limit
(`IVS_IVSC_MAX_BYTES 131256u /* prefix 24 + 16 entries 128 + 65536 words 131072 + 32 */`),
not a one-segment limit, and the base range `$4000`–`$FFFF` caps any accepted image at
49,152 words.

**How to avoid:** make the pair a *token* pair and say so in the plan: 65,537 words is
refused with the word-count token; 65,536 words is refused with the **crossing** token, not
the word-count token. That is a real boundary witness — it pins the count rule's threshold
at exactly 65,536 and proves the tool never writes a wrapped or zero `len_words` — and it is
the only expression that keeps both numbers in criterion 2 truthful. The true acceptance
boundary is rule 5's pair (49,152 words at `$4000` ends at `FFFF`).

**Warning signs:** a test that expects exit 0 for a 131,072-byte input; a refusal id that
passes only because a *different* rule fired.

### Pitfall 3: a truncating open silently defeats the "existing output" refusal

**What goes wrong:** `fopen(path,"wb")` on an existing file succeeds and destroys it, so the
"existing↔new output" refusal is untested and unenforced. `ivs_pack` uses exactly that
(`tools/pack/pack.c`, `pk_write`), which is fine for a build-tree tool but wrong for one
whose output is derived rights-bearing material.

**How to avoid:** exclusive create. `runner/hostio.c:137` is the in-tree precedent
(`open(path, access | O_CREAT | O_EXCL | O_NOCTTY, …)`); C11 `fopen(path, "wbx")` is the
portable short form. The boundary positive must then be "a path that does not exist is
created", not "an existing path is overwritten".

**Warning signs:** a refusal case that passes when the tool writes first and checks second.

### Pitfall 4: the C default and the profile can disagree silently

**What goes wrong:** `ivs_cp1610_config_default()` is a *public* library surface
(`core/include/ivs/ivs_run.h`-level callers use it), and its body is a copy of the profile's
values:

```c
  /* Profile intv-2609-ntsc-family-p1 (plan 02-01): cpu.interrupt_accept_clears_intff = 0,
   * bus.release_latency_cpu_cycles = 2, bus.wait_limit_cpu_cycles = 35. */
  out->interrupt_accept_clears_intff = 0u;
```
[VERIFIED: core/src/cpu/cp1610.c:231-233]

Nothing in the gate compares these two. Measured: flipping only the profile leaves
`cpu-vectors` green (5 ids), flipping both breaks 5 more (see Pattern 7).

**How to avoid:** decide explicitly and record the decision in the plan and in the C41 note.
If the default stays 0, the comment must stop claiming to mirror the profile and must say
which value a standalone CPU caller gets; if it moves to 1, the five vectors gain an explicit
`config` override so the 0 case is pinned rather than inherited.

**Warning signs:** a green `cpu-vectors` after the profile flip, read as "nothing to do".

### Pitfall 5: the lock's role arm is asserted *against* `cart`

**What goes wrong:** adding `cart` as a legal lock role silently weakens
`runner-process.owner-run.bad-lock-line`, whose `cart` example is
`tests/runner-process-tap.sh:1930` [VERIFIED]. The case would keep passing only if the
example changed, and the change is easy to miss because the id name does not mention the
role.

**How to avoid:** keep the lock at two roles (Pattern 4). If the plan extends it, the same
commit must change that case's example to a role no code will ever accept (e.g. `rom`) and
say so in the summary.

**Warning signs:** an `owner-run` diff that touches the lock grammar without touching
`bad-lock-line`.

### Pitfall 6: the rights naming limb scans `tools/` and six doc pages

**What goes wrong:** Phase 13's naming check refuses the tokens of every `refused` **and**
`owner-local-only` rights row inside `core/src/`, `runner/`, `tools/` (except
`tools/check/controls/`) and exactly six pages — `README.md`, `docs/running.md`,
`docs/owner-images.md`, `docs/packaging.md`, `docs/diagnostics.md`, `docs/RIGHTS.md` — with a
character-bounded match `(^|[^A-Za-z0-9_-])TOKEN([^A-Za-z0-9_-]|$)` and a per-`(token, path)`
pin list [VERIFIED: tools/check/rights-lib.cmake:352-380].

The live tokens include `flat-image` (from `data/rights/flat-cartridge-image.json`), plus
`exec-image`, `grom-image`, `cfg-container`, `.cfg`, `bank-switch`, `commercial-rom`,
`game-rom`, `cartridge-dump`, `screenshot`, `rendered-frame`, `page-render`, `libretro`.

**How to avoid:** the new `tools/flatwrap/flatwrap.c` comments, its usage string, and the new
prose in `docs/owner-images.md` and `docs/packaging.md` must say "a flat cartridge image",
"the operator's flat file", "the operator's cartridge file" — never the hyphenated
`flat-image`. `flatwrap` itself is safe (the match is character-bounded, so `flatwrap` and
`flat-image-converter` are not hits). The four existing pins stay: they are
`exec-image`/`grom-image` × `docs/owner-images.md`/`tools/owner-run.sh`
(`data/expected/rights-pinned.ids`), and a pin that stops matching is a **failure**, so
removing the spellings from those two files would break the gate.

**Warning signs:** a gate run whose only new failure is `rights-token: flat-image tools/…`.

### Pitfall 7: the claim-word tripwire covers the operator pages

**What goes wrong:** the `text` step fails any line in a scanned doc that contains
`supports|compatible|passes|accurate|verified|runs|plays|playable|looks|sounds` without a
claim tag or the `[nc]` waiver on the same line
[VERIFIED: tools/check/steps/text.sh:6-11,37-61]. New prose about a cartridge route invites
all of them ("the route runs", "the picture looks right").

**How to avoid:** write the new prose without claim words, or with `[nc]` on the line, as the
existing pages do.

### Pitfall 8: GUARDRAILS V5 does not name the SPDX alternation

**What goes wrong:** the integrity step's SPDX rule is anchored to five tool directories and
enumerates them explicitly:

```cmake
     OR p MATCHES "^runner/.*[.](c|h|sh|cmake)$" OR p MATCHES "^tools/(pack|cart|pcm|asm|bench)/.*[.](c|h)$")
```
[VERIFIED: tools/check/integrity.cmake:70-72]

A new `tools/flatwrap/flatwrap.c` is therefore **outside** the SPDX rule and would escape it
silently — the exact "a tracked C file the gate does not check" failure the rule exists to
prevent. V5 lists the c-build module list, the sources writer, the toy source lists, the
unlisted-source control and the packaging line; it does not list this.

**How to avoid:** add `flatwrap` to that alternation, and note the project's own convention:
each directory in the alternation has exactly one control
(`integrity-missing-spdx-tools-c` → `write tools/pack/x.c x`,
`-tools-asm` → `tools/asm/x.c`, `-tools-pcm` → `tools/pcm/x.c`), so the convention implies a
fourth control `integrity-missing-spdx-tools-flatwrap`. That control costs 0.31 cpu-s of the
1.3 priced for the whole phase — the cost table in §Validation Architecture prices the
alternatives.

**Warning signs:** a new `.c` file with an SPDX header and no gate rule that would notice its
absence.

## Code Examples

### 1. The writer's shape (one read, six checks, one write, one line)

```c
/* tools/flatwrap/flatwrap.c — shape only; every constant comes from content/ivsc.h */
#include "content/ivsc.h"      /* IVS_IVSC_PREFIX_BYTES, _ENTRY_BYTES, _TRAILER_BYTES, _WORD_MAX, … */
#include "common/ivs_sha256.h" /* ivs_sha256_digest */

/* check order is part of the published contract (docs/formats/ivsc-v1.md, "Check order"):
 * length -> words -> base -> crossing -> output directory -> output exists -> word range */
static int fw_words(uint8_t *b, size_t n) { /* hi-first: word = b[2i]<<8 | b[2i+1]; lo-first swaps */ }
static int fw_write(const char *out, const uint8_t *words, uint32_t base, uint32_t n_words) {
  size_t total = IVS_IVSC_PREFIX_BYTES + IVS_IVSC_ENTRY_BYTES + 2u * n_words + IVS_IVSC_TRAILER_BYTES;
  /* prefix: magic, IVS_IVSC_VERSION, flags 0, required_features 0, total, segment_count 1, reserved 0
   * entry : addr = base, len_words = n_words, width = IVS_IVSC_WIDTH_BITS, attr 0, reserved 0
   * trailer: ivs_sha256_digest(bytes, total - IVS_IVSC_TRAILER_BYTES, bytes + total - IVS_IVSC_TRAILER_BYTES) */
  /* one stdout line: the 64 lower-case hex of ivs_sha256_digest(bytes, total, d) */
}
```

The stdout line is what success criterion 3 compares with `slot.cart.sha256`, and the
loader's `file_sha256` is documented as "the SHA-256 of the whole container, trailer
included (what `sha256sum` of the file prints, D-25)"
[VERIFIED: core/src/content/ivsc.h:74-79]. So one line, the whole-file digest, nothing else.

### 2. The round trip, measured

A hand-written container of the same shape as the writer's output — 8,192 zero bytes at
`$5000`, i.e. `24 + 8 + 2*4096 + 32 = 8256` bytes — was run through the release runner:

```
$ build/release/runner/ivs-run --content-dir … --exec <8192 zero> --grom <2048 zero> \
      --cart wrapped.ivsc cartwrap.ivss
outcome pass
exit.code 0
slot.cart.loaded 1
slot.cart.length_bytes 8256
slot.cart.sha256 23db1e4931fade388ae28ee4a1d1d80e9a575b5be4d4609add1fd1884fd65857
slot.cart.status ok
exec: -
```
[VERIFIED: measured this session; `23db1e49…` is the `sha256sum` of the 8,256-byte file, so
the equality the criterion asks for holds end to end]

The live-equality stand-in: flip one byte of the *input* (e.g. `00 00` → `00 01`, still
≤ `$03FF`), wrap again, and require the printed digest and `slot.cart.sha256` to *differ*.
A judge that accepted the same digest twice would be vacuous.

### 3. The six derived-output refusals already fire for a cartridge role

```
--frame-ppm   rc=14  stderr="ivs-run: --frame-ppm refused: content user"    file=no
--video-raw   rc=14  stderr="ivs-run: --video-raw refused: content user"    file=no
--audio-raw   rc=14  stderr="ivs-run: --audio-raw refused: content user"    file=no
--psg-log-out rc=14  stderr="ivs-run: --psg-log-out refused: content user"  file=no
--audio-wav   rc=14  stderr="ivs-run: --audio-wav refused: content user"    file=no
--update-golden rc=14 stderr="ivs-run: golden refused: content user"        file=no
```
[VERIFIED: measured this session with a scenario carrying `content exec user`,
`content grom user`, `content cart user`; no report on stdout in every case]

The code is already role-generic — one loop over `IVS_SCN_ROLES` per output family
(`runner/main.c:762-793`) [VERIFIED] — so this part of criterion 4 is **proof, not
implementation**.

### 4. The route's pass path on project-made files, measured

A scratch root outside the tree, a copy of the helper, the template, a symlink to the
release runner, two zero-filled files and `--record`:

```
rc=10
OWNER-LOCAL RUN (D11): inputs are the owner's, not redistributable, not run in CI, not evidence, no claim
outcome guest-fail
outcome.detail stop step=1 line=23 expected=target got=halted
exit.code 10
slot.exec.loaded 1
slot.exec.sha256 9f1dcbc35c350d6027f98be0f5c8b43b42ca52b7604459c0c42be3aa88913d47
slot.grom.sha256 e5a00aa9991ac8a5ee3109844d84a55583bd20572ad3ffcd42792f3c36b183ad
exec: -
grom: -
```
[VERIFIED: measured this session; the two digests are the ones the suite already pins for
the zero-filled files at `tests/runner-process-tap.sh` (the `or.want` block)]

So the pass-path id **pins the outcome the zeros give**: `guest-fail`, exit 10, detail
`stop step=1 line=23 expected=target got=halted`, `exec: -`. The stand-in that proves the id
can fail is the existing mechanism: flip one EXEC byte *after* `--record`, and the core
refuses at load (`IVS_E_DIGEST_MISMATCH`, exit 14) so the id's expected outcome is not met.

### 5. The digest arithmetic, reproducible

```python
# mm body digest: run the generator twice into a temp tree (cmake -DROOT=<tmp> -DOUT=… -P cmake/IvsMemmapTable.cmake),
# once with the field at 0 and once at 1, and read ivs_mm_body_digest[0].
pre = b"IVSP" + struct.pack(">H", 1) + mm_digest + psg_digest + stic_digest + ctrl_digest  # 134 bytes
params = sha256(pre).hexdigest()
# with the field at 0 this reproduces 89ad65073a3e357acbdd687f6a29016457864a1666b953e9327b7559dc0c0522
# exactly — that equality is the check that the method is right.
```
[VERIFIED: measured this session, both the reproduction and the new value]

### 6. The route's cartridge role, shape

```sh
# tools/owner-run.sh — the option, not a third positional
usage='usage: owner-run.sh [--preset dev|release] [--record] [--cart <cart-image>] <exec-image> <grom-image>'
...
    --cart) [ "$#" -ge 2 ] || die "$usage"; cart=$2; shift 2 ;;
...
[ -z "$cart" ] || [ -f "$cart" ] || die "the cart image is not a regular file: $cart"
...
printf '%s\n' "OWNER-LOCAL RUN (D11): …"
rc=0
if [ -z "$cart" ]; then "$run" --exec "$ex" --grom "$gr" "$tmp/scn.ivss" || rc=$?
else "$run" --exec "$ex" --grom "$gr" --cart "$cart" "$tmp/scn.ivss" || rc=$?; fi
exit "$rc"
```

and the template gains one line, which does **not** change the tracked scenario's expected
outcome because roles load exec → grom → cart and the exec is the one the gate never has:

```
content cart user
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|---|---|---|---|
| a cartridge must be a project listing → `ivs_pack` | an operator's flat image → `tools/flatwrap` → the same container | this phase | the first non-project content path; no third-party format is parsed |
| the owner route takes two system images | the route takes an optional cartridge too | this phase | SCN-03; the lock stays two roles |
| `cpu.interrupt_accept_clears_intff = 0` chosen from the manual's silence | `1`, from the owner's own run through the D11 route | this phase (D-51) | nine test ids and four literals move; the tag does not |

**Deprecated/outdated:**
- The V12 sentence "measured for `cpu.interrupt_accept_clears_intff`: `state.read.describe-ok`,
  `state.literal.params-digest` and `run.stub2.jd-as-j`" — incomplete against this tree; the
  measured set is nine ids (plus five if the C default moves). Record the recomputation.
- The `13-…` note that `.cfg` "is refused by name" is unchanged and must stay: the refusal
  surface is `binary-ext.txt:9` (`cfg`) and `.gitignore:14` (`*.cfg`)
  [VERIFIED: tools/check/binary-ext.txt:9, .gitignore:14], and both files must be
  byte-identical after this phase.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | The work-tree refusal is implemented by baking the source root at configure time (`target_compile_definitions`) and comparing the output's *physical parent directory* against it, accepting `<root>/refs/` | Pattern 2 row 6, Open Question 1 | The one genuinely new mechanism in the phase; a different mechanism (a `.git` walk, or a `--allow-inside` flag) changes the tool's contract and the test shape |
| A2 | The executable is named `ivs_flatwrap` (the `ivs_` convention) while the source directory is `tools/flatwrap` | Pattern 3, Open Question 2 | Criterion 1 spells the command `tools/flatwrap`; a verifier may read the criterion literally and expect a binary named `flatwrap` |
| A3 | The base argument is four bare hex digits (`4000`), the `ivs_pack --patch ADDR=HEX` convention, not `$4000` | Pattern 2 | A `$`-prefixed argument would need a different refusal detail and a different positive; docs spell `$4000` |
| A4 | `--cart` is an option, not a third positional, and the lock keeps two roles | Pattern 4 | Changing either breaks nine existing ids (`usage`, `bad-lock-line`) in ways the plan must then absorb |
| A5 | The seven refusal tokens are `flatwrap-<rule>` and the count rule's threshold is 65,536 words | Pattern 2, Pitfall 2 | The tokens are a one-way published contract (CONTEXT: "one-way"); renaming later re-opens every row |
| A6 | The next blocker (STIC strobe) is recorded by appending to C82's `note` plus a `STATE.md` Deferred row, with no new discrepancy id | Pattern 6, Open Question 3 | A new `C111` row would move `discrepancies=110`/`open=101` and need a `d10-measurements.ids` line; the opposite of the cost posture |
| A7 | The cost unit "process-launch id 0.10 cpu-s" is per id across both presets, so 13 ids ≈ the priced 1.3 | Validation Architecture | If it is per id *per preset*, the same id list costs ~2.6 and the phase must record `reprice` |

## Open Questions

1. **How does the tool know where the work tree is?**
   - What we know: R-2 says "an output inside the work tree except under the ignored `refs/`
     (as `owner-run.sh`)"; `owner-run.sh` derives its root from `$0` (`here=${0%/*}`,
     `root=$(cd -P -- "$here/.." && pwd -P)`), which a C tool cannot do portably. No tool in
     the tree currently resolves a path or a root (`grep -rn "realpath\|getcwd" tools/*/*.c`
     returns nothing), so there is no precedent to copy.
   - What's unclear: whether baking `${PROJECT_SOURCE_DIR}` into a build-only binary is
     acceptable to the project's "no host path in an output" rule (it is not an *output*,
     and the tool is never installed) or whether a root-free rule is preferred.
   - Recommendation: bake the root (`target_compile_definitions(ivs_flatwrap PRIVATE
     "IVS_FLATWRAP_TREE_ROOT=\"${PROJECT_SOURCE_DIR}\"")`) and resolve the output's parent
     with `realpath` — the same physical-directory comparison `owner-run.sh` performs, one
     line of CMake, no directory walk, and both halves of the boundary pair testable against
     the real tree and `$TMPDIR`. Record the alternative (walk up for `.git`) in the plan's
     summary if the reviewer prefers it.

2. **Is the tool called `flatwrap` or `ivs_flatwrap`?**
   - What we know: every existing tool is `ivs_<x>` in `tools/<x>/` (`ivs_pack`, `ivs_asm`,
     `ivs_bench`, `ivs_refpcm`), and `docs/packaging.md:38` names them that way; success
     criterion 1 writes the *path* `tools/flatwrap <flat> <base> <out>`.
   - What's unclear: whether the criterion's spelling is a path or a command name.
   - Recommendation: `ivs_flatwrap`, documented as `tools/flatwrap/` + the build-tree path;
     state the reading in the plan so the verifier does not have to guess.

3. **Is the STIC strobe blocker C82's note or a new row?**
   - What we know: C82 already owns "display-latch persistence and strobe semantics, with
     strobes at swept offsets" and already has its gate `D10-VID-08 | C82` in
     `data/expected/d10-measurements.ids:46`; the observation is that the strobe never
     latches.
   - What's unclear: whether criterion 7's "its own row" means a new id.
   - Recommendation: C82's note + a `STATE.md` Deferred row whose "Milestone" column names
     the phase that will take the tier-4 STIC default. A second row would register one
     proof twice — the reasoning Phase 13 used for EV-07's six control rows — and would move
     two gate stats for no new evidence.

4. **Which ids does the cost allow?**
   - What we know: 1.3 cpu-s is priced; the freeze's units are 0.10 per process-launch id,
     0.065 per in-process id, 0.31 per control.
   - What's unclear: the exact per-preset multiplication (A7).
   - Recommendation: see the three candidate packages in §Validation Architecture and pick
     one *before* writing cases; re-measure at the phase boundary and record one of the
     three checkpoint words.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|---|---|---|---|---|
| CMake | build, presets, `cmake -E sha256sum` | ✓ | ≥ 3.25 (presets + `file(COPY_FILE)`) | — |
| Ninja + a C17 compiler | `c-build` in both presets | ✓ | build tree present and incremental (a full dev rebuild of the touched targets took ~2 s) | — |
| POSIX shell (`sh`, `mktemp`, `dd`, `awk`, `cmp`, `od`) | the two suites and the helper | ✓ | — | — |
| `git` | `hygiene --tree` uses `git rev-parse --show-toplevel` | ✓ | — | not needed by this phase |
| Python 3 | *not* used by the gate; used only to recompute the digest prediction in this research | ✓ | — | hand arithmetic from the generator output |

**Missing dependencies with no fallback:** none.
**Missing dependencies with fallback:** none.

## Validation Architecture

`workflow.nyquist_validation` is `true` in `.planning/config.json`.

### Test Framework

| Property | Value |
|---|---|
| Framework | CTest over 22 registered suites; 18 are in-process TAP binaries (`tests/harness`), 4 are shell TAP drivers (`pack-tap.sh`, `asm-tap.sh`, `runner-process-tap.sh`, `headers-tap.sh`), plus the 38-step gate (`tools/gate.sh`) that judges the TAP files |
| Config file | `CMakePresets.json` (presets `dev`/`release`), `tests/CMakeLists.txt`, `data/expected/tests/*.ids` (one file per suite, the expected id set) |
| Quick run command | `ctest --preset dev -R '^(pack\|runner-process)$' --output-on-failure` |
| Full suite command | `sh tools/gate.sh` (or `sh tools/gate-linux.sh` for the Ubuntu substance) |

Baseline to compare the delta against (the last full Linux gate, `build/gate-linux.log`):
`steps=38/38 controls=401/401 faults=238/238 goldens=29 tests-expected=4990
tests-executed=4986 tests-skipped=4 tests-missing=0 tracked=2728 rights=10 rights-cleared=1
rights-pinned=4/4`. `tests-expected` recomputes from the tree as 2,495 ids × 2 presets =
4,990, and `tests-skipped` as the two `skip-ok` *scenario* ids (`needs-user-exec`,
`owner-exec-title`) × 2 presets = 4. Every new id adds 2; no planned id is `skip-ok`, so the
predicted delta is `tests-expected = 4990 + 2N`, `tests-skipped = 4` unchanged.

### Phase Requirements → Test Map

| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|---|---|---|---|---|
| TOOL-07 | odd length refused / even accepted | integration (process) | `ctest --preset dev -R '^pack$'` — `pack.flatwrap.length` | ❌ Wave 0 (new case in an existing suite) |
| TOOL-07 | word above `$03FF` refused / `$03FF` accepted | integration | `pack.flatwrap.word-range` | ❌ Wave 0 |
| TOOL-07 | 65,537 words refused / 65,536 not refused by that token | integration | `pack.flatwrap.words` | ❌ Wave 0 |
| TOOL-07 | base `$3FFF` refused / `$4000` accepted | integration | `pack.flatwrap.base` | ❌ Wave 0 |
| TOOL-07 | crossing `$10000` refused / ending at it accepted | integration | `pack.flatwrap.cross` | ❌ Wave 0 |
| TOOL-07 | output inside the tree refused / outside accepted | integration | `pack.flatwrap.output-tree` | ❌ Wave 0 |
| TOOL-07 | existing output refused / new accepted | integration | `pack.flatwrap.output-exists` | ❌ Wave 0 |
| TOOL-07 | `--word-order lo-first` reverses each word; default equals hi-first | integration | `pack.flatwrap.word-order` | ❌ Wave 0 |
| TOOL-07 | pinned tokens ≡ printable tokens, every token has a case | in-step (grep diff) | `pack.flatwrap.coverage` (the `asm.neg-coverage` idiom) | ❌ Wave 0 |
| SCN-03 | `content cart user` + all six derived outputs refused, one line each, no file | integration (process) | `ctest --preset dev -R '^runner-process$'` — `runner-process.cart-user.refuse-derived` (or six ids) | ❌ Wave 0 |
| SCN-03 | the snapshot rule fires: `--roundtrip-at-ticks` allowed, nothing written | integration | `runner-process.cart-user.roundtrip` | ❌ Wave 0 |
| TOOL-07/SCN-03 | wrapped image loads as `content cart user`; `slot.cart.sha256` == printed; one flipped input byte changes it | integration (process, 2 runs) | `runner-process.flatwrap.roundtrip` | ❌ Wave 0 |
| SCN-03 | the route's cartridge role: pass path on project-made files, outcome pinned, one-byte mutation fails it | integration (process) | `runner-process.owner-run.cart-pass` | ❌ Wave 0 |
| CPU-06 | the field is 1 and both settings are pinned | in-process | `ctest --preset dev -R '^cpu-vectors$'` — the five retargeted vectors + `cpu.UNIT-INT.accept-clears-intff-config` | ✅ exists, needs the override edit |
| CPU-06 | the digest and every snapshot literal are the new values | in-process | `ctest --preset dev -R '^state$'` — `state.literal.params-digest`, `state.save.canonical-stream-literal`, `state.cut.fixed-point-and-canonical` | ✅ exists, needs the re-pin |
| CPU-06 | the boot-stub-2 interrupt control still discriminates | in-process | `ctest --preset dev -R '^run$'` — `run.stub2.jd-as-j` | ✅ exists, needs re-expression |
| CPU-06 | the C41=0 profile pin is gone from the STIC suite | in-process | `ctest --preset dev -R '^stic$'` — `stic.irq.level-held-reentry-c41-0` | ✅ exists, needs re-target |
| CPU-06 | the observation is in the log, the row stays open, the tag stays `unknown-default-chosen` | registry (gate step) | `sh tools/gate.sh` — `discrepancies` + `profile` steps | ✅ exists |
| TOOL-07 | no tracked `.ivsc`, `binary-ext.txt` and `.gitignore` unchanged | registry (gate step) | `sh tools/gate.sh` — `files` step (`files-unlisted-binary` already proves the rule for a listed extension) | ✅ exists |
| TOOL-07 | the tool is not installed and the package stays eleven paths | registry (gate step) | `sh tools/gate.sh` — `pkg-install` step (`prefix-files=11`) | ✅ exists |
| TOOL-07 | the new C file is inside the SPDX rule | registry (gate step) | `sh tools/gate.sh` — `integrity` step (+ Pitfall 8) | ✅ exists, needs the alternation edit |

### Sampling Rate

- **Per task commit:** `cmake --build --preset dev && ctest --preset dev -R '^(pack|runner-process|cpu-vectors|run|stic|state)$' --output-on-failure`
- **Per wave merge:** `sh tools/gate.sh` (the 38-step gate; `steps=38/38` is an invariant)
- **Phase gate:** the full gate green, `tests-expected` equal to the predicted value, and the cost checkpoint recorded

### Wave 0 Gaps

- [ ] `tools/flatwrap/{CMakeLists.txt,flatwrap.c}` — the tool itself, with SPDX headers (ASCII only)
- [ ] `tests/pack-tap.sh` — a third argv for the flatwrap binary and the nine cases; its `usage` line (`tests/pack-tap.sh:24`) changes from `-eq 2` to `-eq 3`
- [ ] `tests/runner-process-tap.sh` — a sixth argv, the cartridge-role cases, and the round-trip case
- [ ] `tests/CMakeLists.txt:708,738-740` — pass `"$<TARGET_FILE:ivs_flatwrap>"` to both suites
- [ ] `data/expected/flat-refusals.ids` — the seven tokens (new file)
- [ ] `data/expected/tests/pack.ids` and `data/expected/tests/runner-process.ids` — the new ids
- [ ] `tools/check/controls/c-build-unlisted-flatwrap-source/` and its registry line
- [ ] `tools/check/controls/integrity-missing-spdx-tools-flatwrap/` (if Pitfall 8's convention is followed)
- [ ] `tools/check/controls/_good/tree/CMakeLists.txt` — the seventh toy source list
- [ ] `data/expected/ivsn-literals.ids` — four re-pinned literals
- [ ] No new framework, no new suite, no new gate step: all of the above are ids, cases and registry lines inside what exists

**Cost packages (pick one before writing cases).** At the freeze's units (0.10 per
process-launch id, 0.31 per control), and assuming A7 (per id, both presets):

| Package | ids | controls | ≈ cpu-s | Fits 1.3? |
|---|---|---|---|---|
| **A (recommended)** 8 `pack` + 1 `cart-user.refuse-derived` + 1 `cart-user.roundtrip` + 1 `flatwrap.roundtrip` + 1 `owner-run.cart-pass` | 12 | 1 (c-build) | 1.2 + 0.31 = 1.51 | over by 0.21 → record `reprice` or drop the second control |
| **B** package A without `pack.flatwrap.word-order` and `cart-user.roundtrip` (fold both into neighbours) | 10 | 1 | 1.0 + 0.31 = 1.31 | yes, exactly at the price |
| **C** package B without the `integrity` control (extend the alternation, no new control: the rule is already proved three times) | 10 | 1 | 1.31 | yes, with the Pitfall 8 convention deviation recorded in the summary |

Measured context for the price: the `pack` suite runs 14 ids in 1.51 s wall and
`runner-process` 135 ids in 16.67 s wall on this machine, so the marginal cost of a handful
of small process launches is small in wall time and the cpu-s figure is the frozen unit, not
a measurement of these particular cases. Recompute at the boundary.

### Cost checkpoint (recomputed this session from `.planning/ci-runs.tsv`, 99 usable rows)

| Figure | Frozen upstream | Recomputed now | Note |
|---|---|---|---|
| last-five `cpu_s` median | 361 | **363** (357, 360, 363, 438, 368) | two more rows since the freeze |
| last-five `build_s` median | 34 | **35** (33, 34, 35, 39, 35) | |
| last-five `gate_s` median | 121 | **121** (121, 120, 119, 135, 122) | matches |

The phase boundary must recompute again and record exactly one of
`discharged-within-allowance` / `reprice` / `lever-required`.

## Security Domain

`security_enforcement` is `true` and `security_asvs_level` is `1`.

### Applicable ASVS Categories

| ASVS Category | Applies | Standard Control |
|---|---|---|
| V2 Authentication | no | no principal, no credential, no session anywhere in this phase |
| V3 Session Management | no | — |
| V4 Access Control | yes (weakly) | the output-path rule (inside the tree refused, `refs/` excepted) and the exclusive create; the route's existing "not in CI" refusal and its `refs/`-only location rule |
| V5 Input Validation | yes | the seven typed refusals with a fixed check order; a bounded read (cap the input at 131,074 bytes so an oversized file reaches the count rule instead of the heap); every field length checked before use; the container's own parser as the second line |
| V6 Cryptography | yes | SHA-256 only, and only through `common/ivs_sha256.h`; never hand-rolled, never a second implementation in the tool |
| V7 Error Handling and Logging | yes | one `flatwrap: <token>: <detail>` stderr line and exit 1, no partial output file (remove on a failed write, as `pk_write` does) |
| V8 Data Protection | no | no personal data |
| V12 Files and Resources | yes | the output is derived rights-bearing material: never committed, never a fixture, never a CI artifact; the tool writes only the one file it was told to write |

### Known Threat Patterns for a C file-to-file tool

| Pattern | STRIDE | Standard Mitigation |
|---|---|---|
| integer overflow in `2 * n_words` or `total_bytes` | Tampering | check the word count against 65,536 **before** any multiplication; every size fits `size_t` on any supported host, and the container's `total_bytes` is a 4-byte field |
| a short read treated as a complete input | Tampering | read with a cap of `2 * 65537 = 131074` bytes plus one, and refuse anything above it; a read error is its own refusal |
| symlink/`..` escape of the output-path rule | Tampering / Elevation | resolve the **parent directory** physically (`realpath` on the directory, as `owner-run.sh` does with `cd -P`) before comparing against the root, and compare prefixes with the trailing slash included |
| a write into a path the operator did not intend (truncating an existing file) | Tampering | exclusive create (`O_CREAT|O_EXCL`), plus the "existing↔new" refusal |
| a partial container left after a failed write | Tampering | write, check `ferror`, and `remove()` on failure (`pk_write`'s shape) |
| a rights-bearing byte reaching the tree | Information disclosure (of third-party material) | the output-path rule + the gitignored `*.ivsc` + `binary-ext.txt`'s `ivsc`; the tool prints only a digest of derived material and stores nothing else |
| a refused spelling reaching `tools/` or an operator page | — (policy) | Phase 13's naming limb; see Pitfall 6 |

## Sources

### Primary (HIGH confidence — read from this tree this session)

- `core/src/content/ivsc.h:20-31`, `:74-79`, `:99-103` — the container constants, the
  whole-file digest definition, the parse contract
- `core/src/content/ivsc.c:17` — the magic array
- `docs/formats/ivsc-v1.md` — layout, entry table, limits, check order, refusal reasons,
  worked example (66-byte minimum container)
- `tools/pack/pack.c` — `pk_ivsc()` as the existing writer; `pk_write()` as the
  write-then-remove shape; `tools/pack/CMakeLists.txt:27-30` for the link line
- `core/CMakeLists.txt:138-139` — `ivs_core_internal` exposes `core/src`
- `tools/asm/CMakeLists.txt` (tail) — the source-list writer idiom
- `tools/check/steps/c-build.sh:52-55` — the six-module loop
- `tools/check/controls/_good/tree/CMakeLists.txt` — the toy source lists
- `tools/check/integrity.cmake:55-72` — the ASCII and SPDX rules and their directory list
- `tools/check/rights-lib.cmake:352-380` — the naming limb's scopes and bounded match
- `tools/check/steps/text.sh:6-11,37-61` — the claim-word tripwire
- `tools/check/schema/discrepancy.txt`, `tools/check/kinds/discrepancy.cmake` — the log's
  shape and its rules
- `tools/owner-run.sh` — the whole route, quoted in Pattern 4
- `runner/main.c:565-793` — the CLI, the six derived-output refusals and their role loop
- `tests/runner-process-tap.sh:1755-1960` (the owner-run block, including the `cart` lock
  case at :1930), `tests/pack-tap.sh:24`, `tests/asm-tap.sh:273-330`
- `tests/cpu/cpu_vectors.c:153-156`, `tests/harness/ivs_vec.h:44`,
  `core/src/cpu/cp1610.c:226-235`, `tests/stic/stic_irq.c` (`si_c41_both_values`)
- `data/profiles/intv-2609-ntsc-family-p1.json:174-183`,
  `data/memmap/intv-2609-ntsc-family-p1.literal.txt:1-30`,
  `data/expected/profile-fields.ids:28`
- `cmake/IvsMemmapTable.cmake:302-303, 763`,
  `core/src/machine/machine_snapshot.c:92-121`
- `data/expected/ivsn-literals.ids:47-63`, `data/discrepancies.json` (C41, C82, C110)
- `data/expected/pkg-install-files.ids` (eleven paths, quoted below), `docs/packaging.md:38,121`
- `data/rights/*.json`, `data/expected/rights-pinned.ids`, `tools/check/binary-ext.txt:5,9`,
  `.gitignore:14,16`

The eleven installed paths, verbatim (`data/expected/pkg-install-files.ids`, body):
```
include/ivs/ivs.h
include/ivs/ivs_machine.h
include/ivs/ivs_run.h
include/ivs/ivs_state.h
include/ivs/ivs_status.h
lib/cmake/ivs/ivsConfig.cmake
lib/cmake/ivs/ivsConfigVersion.cmake
lib/cmake/ivs/ivsTargets-release.cmake
lib/cmake/ivs/ivsTargets.cmake
lib/libivs_core.a
share/doc/ivs/LICENSE
```
[VERIFIED: data/expected/pkg-install-files.ids, eleven non-comment lines]

The packaging sentence that must gain the tool, verbatim (`docs/packaging.md:38`):
> "The development tools `ivs_pack` (tools/pack) and `ivs_asm` (tools/asm, the project
> mini-assembler, D-17) are build-only: they have no install rule, are never installed or
> shipped, and are not part of the core."
[VERIFIED: docs/packaging.md:38]; and `docs/packaging.md:121` lists the same set again
("Neither the runner (`ivs-run`), nor `ivs_pack`, nor `ivs_asm` is installed or shipped…")
— both lines must mention the wrap tool or the page contradicts itself.

### Secondary (MEDIUM confidence — project records read, not re-measured)

- `.planning/MILESTONE-CONTEXT.md` §1a, §3 (D-47, D-51), §4, §5
- `.planning/ops/GUARDRAILS-v0.3.md` V5, V7, V12, §2
- `.planning/research/v0.3.0-scoping-3-rights-format-packaging.md` R-2, R-5
- `.planning/research/v0.3.0-scoping-4-verification.md` (the flat-wrap and cartridge-role rows)
- `.planning/STATE.md:700-742` (the binding rules, including item 8 on the first real run)

### Tertiary (LOW confidence — not used for any decision)

- None. This phase's domain is entirely in-tree; no web search was needed or used, which is
  also why the Package Legitimacy Audit is not applicable.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — no dependency; the two in-tree targets and the header were read.
- Architecture: HIGH for the container/layout and the runner's refusals (read and, for the
  refusals, executed); MEDIUM for the work-tree refusal mechanism and the tool's name, which
  are design choices with no in-tree precedent.
- Pitfalls: HIGH — the nine-id failure set, the four new literals, the pass-path outcome and
  the six refusals were all measured on this tree this session, and the two extra
  guardrail gaps (SPDX alternation, lock-role test) were found by reading the code.

**Research date:** 2026-11-26
**Valid until:** the next commit that touches `data/profiles/`, `data/expected/ivsn-literals.ids`,
`tools/owner-run.sh`, `runner/main.c` or `tools/check/integrity.cmake` — the measured figures
are keyed to this tree (`bd0e89f` + the uncommitted build state), and every one of them is
cheaper to recompute than to trust.
