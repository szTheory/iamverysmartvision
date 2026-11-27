---
phase: 18-the-inspection-surface
plan: 03
subsystem: inspection-surface
tags: [build-only-tool, disassembly, round-trip, non-vacuity-guards, refusal-token, registry-row, resume]

requires:
  - phase: 18-the-inspection-surface
    plan: 01
    provides: "`docs/inspection.md` as the frozen contract: the `## Disassembly` section with its pinned derivation (header-key filter, three operand cuts, the `ORG` rule), its oracle (the `ivs_pack --dump-words` word dump of both listings), its non-vacuity guards and its one authorised drop"
  - phase: 18-the-inspection-surface
    plan: 02
    provides: "`tools/inspect/` and `ivs_inspect` under both presets, the output ladder (tree refusal, exists, exclusive create, short write removes), the named-refusal shape `ivs-inspect: <token>: <detail>`, and `inspect-dis-row` already listed in the frozen page's refusal-token block"
  - phase: 07
    provides: "`asm.twin-boot-stub` and the suite's `twin_words` / `dumps_agree` comparator, which this limb reuses instead of writing a second one"
  - phase: 17
    provides: "the count-gate contract (D-56): every count gate names the file it counts and every baseline names its start commit"

provides:
  - "`ivs-inspect dis <listing> --out <file.asm>`: the project's own listing turned into source by the listing's own `ROW` and `ASM` columns, with no decoder table in the tool"
  - "six `asm` ids: the three-oracle word-dump round trip, the two-word form, the immediate, the distinct-mnemonic count, determinism, and the committed `DECLE $0000` constant-mnemonic stand-in"
  - "one `important-gates` row under the new gate name `asm`, so the limb's stand-in is re-run on every gate run"
  - "the round trip measured non-vacuous: 22 words against 16 instruction lines, 7 distinct mnemonics, `MVII #` present, and the stand-in reassembling to 16 words that differ from the input's 22"

affects:
  - "TOOL-02: the disassembly half is now built and proved, so the requirement's remaining sentence about a droppable limb is closed by construction rather than by a drop"
  - "the milestone close: the priced addition is unchanged at 3.59 against the frozen 2.96, and the plan's 0.60 share is spent rather than returned"
  - "any later listing with a new annotation: it fails loudly at `inspect-dis-row` or at the word comparison, not by silently emitting a source that assembles to different words"

tech-stack:
  added: []
  patterns:
    - "the mnemonic is the listing's own leaf row id and the operand text its own `ASM` column cut by three fixed rules, so the tool holds a vocabulary and not a decoder table: nothing in it maps a word to a mnemonic or back"
    - "a round trip whose oracle is the assembler and whose observation is the word dump on both sides, compared by the suite's existing `dumps_agree` - the same comparator `asm.twin-boot-stub` uses, so there is no second comparator to drift"
    - "non-vacuity as four counts on one id set: words against instruction lines, an immediate on both the emitted source and the committed twin, a distinct-mnemonic floor of three, and a committed constant-mnemonic stand-in that must reassemble and then differ"
    - "a fail-closed refusal before any output exists, so a listing form the tool has never seen leaves no file behind"

key-files:
  created:
    - tools/inspect/inspect_dis.c
    - .planning/phases/18-the-inspection-surface/18-03-SUMMARY.md
  modified:
    - tools/inspect/inspect.c, tools/inspect/CMakeLists.txt
    - tests/CMakeLists.txt, tests/asm-tap.sh
    - data/expected/tests/asm.ids, data/expected/important-gates.ids

key-decisions:
  - "The operand text comes from the `ASM` column by the page's three pinned cuts, not from the `BITS` groups: the cuts proved deterministic on the corpus on the first try, so the plan's authorised BITS-group fallback was not taken and no guard was weakened."
  - "The row vocabulary is the assembler's accepted opcode spellings and nothing else - names only, no encoding, no operand shape, no word - so `DATA`/`DECLE` are deliberately outside it and a listing carrying one is refused fail-closed instead of emitted as a guess. Every row the boot-stub corpus uses is inside it."
  - "The stand-in replaces each instruction line with the whole line `DECLE $0000` rather than swapping the mnemonic, because keeping the operand text yields `DECLE #$0001, R0` (`asm-operand-kind`) and `DECLE $1010` (`asm-decle-range`): a mnemonic-only substitution never assembles, so it would fail for the assembler's reason instead of the word difference the criterion names."
  - "The three oracles are compared with the suite's existing `twin_words` and `dumps_agree`, so `asm.dis.roundtrip-boot-stub` and `asm.twin-boot-stub` judge with the same function; the case's comment says the equality is over word dumps, so a reader does not look for a listing-byte equality this limb never asserts."
  - "Determinism is `asm.dis.deterministic` alone and is not asserted a second time inside the round-trip id, because a duplicate assertion of the same property is an id that can only ever fail with the one that already covers it."
  - "The limb was built, not dropped: it round-tripped the corpus containing a two-word form and an immediate inside the priced line, which is the frozen page's only drop condition, so no appended note was needed and `docs/inspection.md` is untouched."

requirements-completed: [TOOL-02]

actuals:
  tokens: 11076
  tasks: 1
  commits: 1
  plan_head_before: dc14599151f2a804cf499589701b16d9ab3633b9
  plan_head_after: b639319

# Metrics
duration: 71min
completed: 2026-11-27
status: complete
---

# Phase 18 Plan 03: The Disassembly Limb, Round-Tripped Through the Project's Own Assembler Summary

**One build-only `ivs-inspect dis` that reads the project's own boot-stub listing's `ROW` and `ASM` columns and
emits source `ivs_asm` reassembles into the same 22 words - proved by the `ivs_pack --dump-words` dump agreeing
with the hand listing and with the committed twin, and kept from being vacuous by 22 words against 16
instruction lines, 7 distinct mnemonics, a present immediate, and a committed `DECLE $0000` stand-in that
reassembles to 16 words and then fails the equality.**

## Baselines

start-commit: dc14599151f2a804cf499589701b16d9ab3633b9

`dc14599` is the commit this plan's task began from (the previous dispatch's `HEAD` and the orchestrator's
stated `current HEAD`); it is `18-02`'s record commit, so the tree at that sha is exactly the tree `18-02`
measured its own final line on. The baseline below is that line, carried forward with its provenance named
rather than re-run: **this dispatch did not run the gate before its first edit**, because the first edit had
already been made by the dispatch that was stopped, and a gate started now would measure a half-committed
tree. The figures are the ones `18-02` recorded at `dc14599`, and every predicted token below reads from them.

```
baseline (dc14599, = 18-02's final line): tests-expected=5070 tests-executed=5066 tests-skipped=4
  tests-missing=0 controls=405/405 steps=38/38 faults=238/238 goldens=29 gates=121 lines=371
  pkg-selftest=21/21 release-selftest=13/13 prefix-files=11 rights=11 rights-cleared=1
  claims=29 provisional=28 claim-refs=27 docs=65 doc-files=64 listings=7
  tracked=2831 files=2831 hygiene-files=2831
```

Counts read directly from the files they come from, at the baseline and after: `data/expected/tests/asm.ids`
50 data lines → 56; `data/expected/important-gates.ids` 121 distinct gate names over 371 data lines → 122 over
372. Both files also gained comment lines (the phase's convention of documenting an id block above the ids),
which move neither count: the gate counts data lines, not `wc -l`.

## The measured delta

Measured on the committed tree with `sh tools/gate.sh`, and byte-identical under `dash`:

| token | before (dc14599) | after (b639319) | why |
|---|---|---|---|
| `tests-expected` | 5070 | **5082** | six ids, counted across both presets |
| `tests-executed` | 5066 | **5078** | none of the six is skip-ok; a missing fourth argument reports `not ok`, never a skip |
| `gates` | 121 | **122** | the row introduces the gate name `asm` |
| `lines` | 371 | **372** | one registry row |
| `tracked`/`files`/`hygiene-files` | 2831 | **2832** | one new tracked file, `tools/inspect/inspect_dis.c`; this record is the 2833rd at its own commit |
| `steps` | 38/38 | **38/38** | no new file under `tools/check/steps/` |
| `controls` | 405/405 | **405/405** | the stand-in is a case in an existing suite, not a control directory |
| `faults`, `goldens` | 238/238, 29 | **same** | invariants |
| `claims`, `provisional`, `claim-refs` | 29, 28, 27 | **same** | this phase mints no claim |
| `prefix-files` | 11 | **11** | no install rule |
| `rights`, `rights-cleared` | 11, 1 | **11, 1** | the `.ivst` row landed in `18-02` |
| `docs`, `doc-files` | 65, 64 | **65, 64** | the page is frozen and was not amended, because the limb was built |
| `tests-skipped`, `tests-missing` | 4, 0 | **4, 0** | nothing is skipped and nothing is missing |
| `pkg-selftest`, `release-selftest` | 21/21, 13/13 | **same** | no packaging surface is touched |

Every predicted token matched the plan's verify list exactly and nothing else moved. No `.ivst` is tracked
(`git ls-files | grep -E '\.ivst$'` is empty), and `sh tools/hygiene.sh --tree` passes with no finding.

## What the two commits ship

**`b639319` — the limb.** `tools/inspect/inspect_dis.c` (466 lines, `SPDX-License-Identifier: MIT` on line 1)
implements `ivs_inspect_dis_emit`: it copies the listing's leading run of `# key: value` lines whose key is one
of the eight `ivs_asm` accepts, finds each word line's row by consuming the all-`0`/`1` `BITS` groups, treats a
`-` row as an operand word of the instruction above it, cuts the `ASM` text by the page's three rules in the
page's order, refuses a row outside the assembler's mnemonic vocabulary with `inspect-dis-row` naming the row
and the address, and emits `ORG $HHHH` before the first instruction and at every gap. `tools/inspect/inspect.c`
gains the `dis` subcommand - the transform runs to completion before anything is created, then the output goes
through the same ladder `trace` and `break` use, and a short write removes what the run created.
`tests/asm-tap.sh` gains the fourth argument, the `-ge 3 -le 4` arity guard and the six cases; `tests/CMakeLists.txt`
passes `$<TARGET_FILE:ivs_inspect>`; six ids land in `asm.ids` and one `asm mutant` row in `important-gates.ids`.

**This record — the baselines, the measured figures, the deviations.** The plan's step (e) asked for the record
in the limb's own commit; this dispatch committed the limb first and the record second, which is deviation 2
below. `start-commit:` above is read back by the plan's verify from this file, and it names a 40-hex ancestor of
`HEAD`.

## The round trip, measured rather than asserted

Measured this dispatch on the committed tree, `build/dev` binaries:

| observation | value |
|---|---|
| emitted source, header | the eight accepted keys, including `# window: 1000-101F`; the listing's bare `#` and prose lines dropped |
| emitted source, instruction lines | **16** (`ORG` excluded, operand-less `HLT`/`DIS` counted) |
| words in the input listing's dump | **22** |
| words in the reassembled source's dump | **22**, byte-identical to the input's |
| words in the committed twin's dump | **22**, byte-identical to both |
| margin over the two-word-form floor | **6** (22 − 16), against a required 2 |
| distinct mnemonics in the emitted source | **7** - `B DIS HLT J MVII MVO TSTR` - against a required 3 |
| immediate form | `MVII #$0001, R0` present in the emitted source and in the committed twin |
| `dis` run twice | byte-identical (`cmp` clean) |
| unknown row `NOSUCHROW` at `1000` | exit 1, `ivs-inspect: inspect-dis-row: the row NOSUCHROW at 1000 is not one of the mnemonics ivs_asm accepts`, **no output file** |
| constant-mnemonic stand-in | assembles (rc 0) to **16** words, dump **differs** from the input's 22 - the comparison is shown able to fail |

The emitted source's instruction sequence is the plan's predicted derivation, row for row: `B $1010`, eight
`HLT` fill rows, `MVII #$0001, R0`, `MVO R0, $035F`, `TSTR R0`, `DIS`, `MVII #$0340, R6`, `J $5000`. No line
repeats its mnemonic (`B B ...`, `HLT HLT`), which is the cut-3 invariant the plan's guard mutation names.

## Deviations from Plan

Six, each recorded rather than hidden.

### 1. [Rule 2 - missing critical wiring] `tools/inspect/inspect.c` is modified although `files_modified` does not list it

- **Found during:** the resumed dispatch's reconciliation of the stopped dispatch's tree.
- **Issue:** the plan's `files_modified` names `inspect_dis.c` and the build/test files but not `inspect.c`, yet
  `dis` cannot be reachable without a dispatch entry and an output ladder in the command file: `main`'s
  subcommand chain and the tree/exists/exclusive-create ladder live there.
- **Fix:** `inspect.c` gains `ix_cmd_dis` (+122/−7): the transform runs to completion before any output is
  opened, then the same `ix_open_output` ladder, then a short write that removes what the run created, and a
  `dis` entry in `main`'s chain. The alternative - a fourth source file to hold the subcommand - is refused by
  the split the file's own comment states, and would have been a `c-build` finding.
- **Files modified:** `tools/inspect/inspect.c`.
- **Commit:** `b639319`.

### 2. [Recorded] the record is its own commit, not the limb's

- **Found during:** the dispatch's time discipline, stated in its own instructions.
- **Issue:** the plan's step (e) asks for one commit carrying `18-03-SUMMARY.md`, but the record's measured
  figures only exist after the gate runs, and this dispatch was instructed to commit the limb as soon as the
  `asm` suite was green and to run the gate afterwards.
- **Fix:** two commits - the limb (`b639319`), then this record. Nothing the plan's verify reads is weakened:
  `start-commit:` is a 40-hex ancestor of `HEAD` and the delta table is measured against it.
- **Files modified:** none beyond the record itself.
- **Commit:** the record's own commit.

### 3. [Recorded, observation] the frozen page's `## Disassembly` names two operand derivations

- **Found during:** reading the page against the plan's action, as the plan's `read_first` requires.
- **Issue:** the page's opening paragraph says "the operands are the following `-` rows plus the field groups
  of the `BITS` column", while its own pinned derivation list two paragraphs later says the operand text is the
  `ASM` column cut by three rules. The two sentences describe different implementations.
- **Fix:** the implementation follows the **pinned derivation list**, which the page introduces as pinned
  precisely because "a cut the implementer has to guess is a cut that silently emits a source assembling to
  different words", and which is the one the plan's action spells out step by step. The page is frozen and was
  **not** edited (the only sanctioned edit is the drop note, and no drop was taken). The BITS-group fallback the
  plan authorised as a contingency was not needed: the cuts proved deterministic on the corpus on the first
  attempt, so no guard was weakened. Flagged here so the phase record shows the discrepancy rather than leaving
  it to a later reader of the page.
- **Files modified:** none.
- **Commit:** this record.

### 4. [Recorded] the row vocabulary is names only, and `DECLE`/`DATA` are outside it on purpose

- **Found during:** writing the refusal's known set.
- **Issue:** the plan says the known set is "the assembler's accepted mnemonic vocabulary", which could be read
  as including the data directive `ivs_asm` writes for a `DECLE` word.
- **Fix:** the table holds the assembler's opcode spellings only - no encoding, no operand shape, no word, so it
  cannot become a second truth about the instruction set - and the file header states that a data row is refused
  fail-closed rather than emitted as a guess. Every row the boot-stub corpus uses is inside it, which is the
  corpus the frozen page names.
- **Files modified:** `tools/inspect/inspect_dis.c` (the header comment says so).
- **Commit:** `b639319`.

### 5. [Recorded] the ids and registry files also gain comment lines

- **Found during:** the first gate run's token comparison.
- **Issue:** the plan's `fails_when` states the `asm.ids` line count as 50 → 56 and the registry's as 371 → 372,
  while `wc -l` on either file moves by more than that.
- **Fix:** nothing to fix - the counts the plan names are **data lines**, which is what the gate counts
  (`asm.ids` 50 → 56, `important-gates.ids` 371 → 372), and both files' new comment blocks document the id group
  above the ids, which is the phase's own convention. Recorded so the discrepancy between `wc -l` and the gate's
  count is not read later as a miscount.
- **Files modified:** `data/expected/tests/asm.ids`, `data/expected/important-gates.ids`.
- **Commit:** `b639319`.

### 6. [Recorded] the container gate was not run

- **Found during:** the dispatch's own time discipline, and measured rather than assumed.
- **Issue:** the docker daemon on this host does not answer.
- **Fix:** the line below is written as a visible `not run`, never a pass. The orchestrator attempts the
  container gate separately.
- **Files modified:** none.
- **Commit:** this record.

```
gate-linux: not run — docker daemon unresponsive on this host (retried at 2026-11-27T01:57:58Z, `docker info` timed out after 20s with no answer)
```

## Gate evidence

Both shells, once each, on the committed tree at `b639319`, with the last line of each log compared by `cmp`:

```
sh   tools/gate.sh : rc 0, gate: PASS steps=38/38 controls=405/405 faults=238/238 goldens=29
                     gates=122 lines=372 tests-expected=5082 tests-executed=5078 tests-skipped=4
                     tests-missing=0 prefix-files=11 claims=29 provisional=28 claim-refs=27
                     rights=11 rights-cleared=1 docs=65 doc-files=64 pkg-selftest=21/21
                     release-selftest=13/13 listings=7 tracked=2832 files=2832 hygiene-files=2832
dash tools/gate.sh : rc 0, byte-identical summary line (`cmp` clean)
```

`grep -c '^-- FAIL '` is 0 in both logs. `ctest --preset dev -R '^asm$'` and `ctest --preset release -R '^asm$'`
both pass (10.6 s and 1.4 s), and the suite's own TAP output shows `ok 6` through `ok 11` for the six new ids.
`sh -n` and `dash -n` on `tests/asm-tap.sh` are both clean.

## Known Stubs

None. The emitter writes no label and no placeholder: every emitted line is either a header line copied from the
listing, an `ORG` directive, or an instruction line whose mnemonic and operand both come from the listing's own
columns. The row refusal is the only path that produces no source, and it produces no file either.

## Threat Flags

None. The limb adds no network surface, no auth path, no schema and no new file-access pattern: it reads one
listing the caller names and writes one source through the ladder `18-02` already proved, and the `.ivst`
refusal plus the no-tracked-`.ivst` assertion are unchanged.

## Self-Check: PASSED

- `tools/inspect/inspect_dis.c` exists, carries `SPDX-License-Identifier: MIT` on line 1, and is in
  `IVS_INSPECT_SOURCES`.
- `b639319` is an ancestor of `HEAD`; `dc14599151f2a804cf499589701b16d9ab3633b9` is an ancestor of `b639319`.
- The six ids exist in `data/expected/tests/asm.ids` **and** as cases in `tests/asm-tap.sh`, and all six report
  `ok` in the suite's own output.
- The `asm mutant asm.judge.dis-constant-mnemonic TOOL-02` row resolves in `data/expected/important-gates.ids`.
- The round trip is non-vacuous by measurement: 22 words, 16 instruction lines, 7 distinct mnemonics, an
  immediate present, and the stand-in reassembling to 16 words that differ.
