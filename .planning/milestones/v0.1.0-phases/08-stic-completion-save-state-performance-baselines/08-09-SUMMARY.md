---
phase: 08-stic-completion-save-state-performance-baselines
plan: 09
subsystem: Phase 8 wave B, plan 1 - the state oracle before any serializer - the `state` gate step (step 38) over the digest field table text, the 88-row hidden-state checklist, the 18 hard-point cuts, the hand-derived schema literal and its pin, the IVSN v1 container specification with its size derived on paper, the five stand-in programs, and the blind-read request
tags: [B, STA-01, STA-02, oracle, state-step, checklist, hard-points, ivsn-v1, schema-literal, blind-read, D-05, D-11, D-12, D-13, D-16, D-17, D-20, D-38, D-41, D-44, T-08-24, T-08-25]
status: complete

requires:
  - phase: 08-stic-completion-save-state-performance-baselines
    provides: "plan 08-08 end state (STIC state layout settled: ivs_stic_state 528 bytes unchanged, collision bits in mem.stic_reg 0x18 to 0x1F, no new table row; deliver seam non-const); counts at plan start steps=37/37 controls=371/371 faults=226/226 tests-expected=4772 tests-executed=4768 tests-skipped=4 claims=27 provisional=26 (re-measured with a full gate run at the start, identical)"
provides:
  - "tools/check/steps/state.sh (step 38): reads the text of core/src/machine/machine_digest.c and the oracle data files only (no C is built), parses every IVS_DF call and the slot macro (suffix list read from the macro body) with width, count, domain, kind and the optional class token, and reports 29 tokens (row sets, order, kind, domain, shape, class, witness, test, cut use, exemption reasons, pending owners, the schema pin); six controls (state-checklist-row-missing, -line-extra, -witness-unknown, -test-unknown, -exempt-no-reason, state-schema-literal-stale); stats state-rows=88 state-cuts=18"
  - "data/state/schema.literal.txt (88 rows, STATE 51, IMAGE_BEARING 3, BOUND 28, PAD 6), data/expected/state-checklist.ids (88 lines), data/expected/state-cuts.ids (18 cuts), data/state/tests-pending.ids (7 ids, owners 08-11, 08-13, 08-14), data/expected/ivsn-literals.ids (schema digest pin, header 612, trailer 32, seven section lengths and offsets, totals 3,655 and 4,299, the first 24 header bytes as hex)"
  - "docs/formats/ivsn-v1.md (the container: classes, layout, sections, the size derivation, digests informational per D-41, schema digest, profile identity, check order, the 24 planned reasons 58 to 81, derived invariants INV-1 to INV-6 as candidates, rights, worked example, fuzzing), docs/state/hidden-state-checklist.md, docs/state/hard-points.md (the 18 predicates and the programs P-SDBD, P-HLT, P-FAULT, P-LATCH, P-PSG with decles), docs/state/checklist-second-read-request.md (first line pending)"
affects: [08-10, 08-11, 08-12, 08-13, 08-14, 08-15, 08-16, 08-17]

actuals:
  tokens: 30321
  tasks: 3
  commits: 3
plan_head_before: 2decaf176a20cadf42445a0bfb983b2a18037a52
plan_head_after: 02899e1b1814ea678cc4cd78568534779a012873

key-decisions:
  - "magic is BOUND, not PAD (the plan's decision, recorded): restore zeroes the PAD rows and must never touch the handle magic; counts STATE 51, IMAGE_BEARING 3, BOUND 28, PAD 6 (the research listed 27 BOUND and 7 PAD/MAGIC)"
  - "The witness map departs from the plan's examples in six places because the plan's own step token state-cut-unused needs every one of the 18 cuts to be the witness of a row, and the examples left HP-STEAL-C, HP-CLOSED, HP-INT-ENTRY and HP-RESETPEND without one: cpu.r at HP-INT-ENTRY (R6 pushed, R7 the vector), mem.gate_open at HP-RESETPEND (all gates open, a zero target blocks the first strobe), stic.grants_in_frame at HP-STEAL-C, stic.late_grants at HP-STEAL-B, stic.display_latch at HP-CLOSED (the boundary after the first strobe of the interrupt routine), cpu.intff at HP-FRAME-MID (armed for the next frame; the predicate adds intff 1), the blocked counters at HP-LATCH (the only program that touches the registers outside the window); mem.scratchpad at HP-SDBD and mem.gram and mem.sysram at HP-LATCH"
  - "Sweep rule for plan 08-14, written into the checklist narrative: the sweep value of a row is the byte-wise complement of its own bytes at the cut (and the other member of the set for a 0 or 1 flag), never the 0xA5 fill alone, so a row whose reference value equals the fill, or a flag tested as nonzero, cannot hide"
  - "The state step reads the constants of the table text (counts, domain ids) from define lines of the table source and every core header outside fault ifdef branches, so a width or count edit in a header moves the table row and breaks the schema literal (state-shape-drift); the toy table already carries class tokens, so the seventh-field branch of the step runs in the positive control (plan 08-10 expected to add it)"
  - "HP-CLOSED is defined as the closed boundary after the first display strobe of the interrupt routine (MVO is not interruptible) with INTRM low, not as 'prev_interruptible 0 with intff set': in the lamp run the vector is JD, so intff is 0 after the first jump and a closed boundary with the enable set exists only right after the entry, which is HP-INT-ENTRY"
  - "The five programs are mine to define (plan 08-14 builds them from the document): every decle derived from the printed patterns of opcode-families.json and the vectors, then cross-checked word for word against the listings that ivs_asm (built from tools/asm/asm.c in a scratch directory, never tracked) produced from the same instruction sequences; cycle arithmetic gives HP-SDBD tick 112, HP-HLT 372, HP-FAULTED 540 as expectations, not pins"

key-files:
  created:
    - tools/check/steps/state.sh
    - data/expected/gate-steps/state.ids
    - data/expected/state-checklist.ids
    - data/expected/state-cuts.ids
    - data/expected/ivsn-literals.ids
    - data/expected/controls/state.ids
    - data/state/schema.literal.txt
    - data/state/tests-pending.ids
    - docs/formats/ivsn-v1.md
    - docs/state/hidden-state-checklist.md
    - docs/state/hard-points.md
    - docs/state/checklist-second-read-request.md
    - tools/check/controls/_good/tree/core/src/machine/machine_digest.c
    - tools/check/controls/_good/tree/data/state/schema.literal.txt
    - tools/check/controls/_good/tree/data/state/tests-pending.ids
    - tools/check/controls/_good/tree/data/expected/state-checklist.ids
    - tools/check/controls/_good/tree/data/expected/state-cuts.ids
    - tools/check/controls/_good/tree/data/expected/ivsn-literals.ids
    - tools/check/controls/state-checklist-row-missing/
    - tools/check/controls/state-checklist-line-extra/
    - tools/check/controls/state-checklist-witness-unknown/
    - tools/check/controls/state-checklist-test-unknown/
    - tools/check/controls/state-checklist-exempt-no-reason/
    - tools/check/controls/state-schema-literal-stale/
    - .planning/phases/08-stic-completion-save-state-performance-baselines/08-09-SUMMARY.md
  modified:
    - tools/check/controls/_good/tree/CMakeLists.txt
    - .planning/STATE.md
    - .planning/ROADMAP.md

requirements-completed: []

completed: 2026-11-14
---

# Phase 8 Plan 09: the state oracle Summary

**Wave B opens oracle first and with no C. A new pure gate step `state` (step 38) reads the text of the digest field table and holds it together with a hand-derived canonical descriptor text of the 88 rows (`data/state/schema.literal.txt`, pinned by SHA-256 in `data/expected/ivsn-literals.ids`), a machine-checked hidden-state checklist (one line per row: class, witness cut, test id, sweep), the 18 named hard-point cuts and the list of test ids that no suite registers yet; it fails in 29 ways and six controls prove six of them. The container is specified before any serializer: IVSN v1 with a 612-byte header, seven sections of 35, 2,761, 615, 51, 63, 79 and 51 bytes (3,655), a 32-byte trailer and a total of 4,299 bytes, derived on paper and recounted by a scratch awk over the schema literal, with the first 24 header bytes pinned as hex; the eight machine-level digests of the header are informational (D-41) and named as the only exception of the canonical re-encode rule. The hidden-state narrative, the 18 cut predicates with five stand-in programs (decles derived from the printed patterns and cross-checked against an independent assembler build), and the request for a blind second read of the checklist (first line `pending:`) are written; the reader is built and run by the orchestrator, nothing of it is committed here. No file under `core/`, `runner/`, `tests/`, `fixtures/` or `data/scenarios/` changed. The research figures (88 rows, 79/2/1/6 by kind, 10/12/25/28/2/5/4/2 by domain, 4,299 bytes) were re-derived by reading the table before the step ran, and they agree: no difference. The final gates pass under sh, dash and the Ubuntu container with `steps=38/38 controls=377/377 faults=226/226 tests-expected=4772 tests-executed=4768 tests-skipped=4 tests-missing=0 state-rows=88 state-cuts=18`. STA-01 and STA-02 stay Pending.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `06d4820` | 1 (tracer) | the state step, the checklist (88 lines), the cut inventory (18), the schema literal and its pin, the pending list, the toy table and toy copies, control state-checklist-row-missing (steps 37 to 38, controls 371 to 372) |
| `ae78cb8` | 2 | docs/formats/ivsn-v1.md, the size and header literals, five controls (372 to 377) |
| `02899e1` | 3 | the hidden-state narrative, the hard-point document with the five programs, the blind-read request; two witness lines of the checklist moved (cpu.intff, stic.display_latch) |
| the commit that adds this file | | this SUMMARY, STATE.md and ROADMAP.md |

## The re-derived figures (Task 1)

Read from the table text before any run: 88 rows = 67 literal `IVS_DF` rows + 3 slot-macro uses of 7 rows (kind, loaded, length_bytes, length_units, unit_bits, base_addr, sha256). By kind UINT 79, PROFILE 2 (`profile`, `mem.profile`), MAGIC 1, PAD 6. By domain CPU 10, MEM 12, IMAGES 25 (4 literal + 21 slot), INSTANCE 28, VIDEO 2, AUDIO 5, INPUT 4, PSGLOG 2. By class STATE 51, IMAGE_BEARING 3, BOUND 28, PAD 6 (51 + 3 + 28 + 6 = 88). Differences from the research (08-RESEARCH 2.1): none in rows, kinds, domains and sizes; the research's class split 27 BOUND and 7 PAD/MAGIC differs from the plan's only by the `magic` decision (BOUND), which is the plan's and is recorded here. The step parses the table with awk and measures `state-rows=88`; the hand derivation and the parse agree on every name, width, count, domain and kind (the step compares them row by row).

Container figures (Task 2), derived by hand and recounted by a scratch awk over the schema literal (not tracked): CPU 35, MEM 2,761, INSTANCE 615, VIDEO 51, AUDIO 63, INPUT 79, PSGLOG 51; sum 3,655; header 612; trailer 32; total 4,299; offsets 612, 647, 3,408, 4,023, 4,074, 4,137, 4,216; trailer at 4,267. The research figures were the same (3,655 and 4,299, INSTANCE 615, MEM 2,761, CPU 35, VIDEO 51, AUDIO 63, INPUT 79, PSGLOG 51): no difference. The first 24 header bytes are `894956534E0D0A1A00010000000010CB0000000100000007`.

## The state step (Task 1)

- Table parse: line reader (preprocessor lines skipped, the slot macro definition captured with its continuation lines, block and line comments removed, newlines to blanks), then a scan for IVS_DF( and IVS_DF_SLOT( calls with balanced-paren argument splitting (string-aware), name expressions with the macro parameter substituted, widths, counts and domains evaluated as sums of products of numbers and defined names (the names come from define lines of the table source and every core header, outside fault ifdef branches, the else branch counting), kind and class tokens mapped. The seventh argument is the class token (IVS_SNAP_STATE, IVS_SNAP_IMAGE, IVS_SNAP_BOUND, IVS_SNAP_PAD); the current real table has six arguments, so the source comparison is skipped for it and runs in the toy tree.
- The 29 tokens are listed in the header of `tools/check/steps/state.sh`; the plan's 14 are all present (`state-row-missing`, `state-line-extra`, `state-kind-drift`, `state-domain-drift`, `state-class-unknown`, `state-class-drift`, `state-witness-unknown`, `state-cut-unused`, `state-test-unknown`, `state-pending-owner`, `state-pending-stale`, `state-exempt-no-reason`, `state-schema-stale`, `state-literals-missing`).
- POSIX awk only (no intervals, no gensub, no gawk extensions), `sh` and `dash`, `cmake -E sha256sum`; measured at 0 s per step in the gate.
- Toy tree: `core/src/machine/machine_digest.c` (valid C, the local macros IVS_DF, a slot macro and the IVS_SNAP_* constants, a static const table of seven rows with one row of each class and a count written as a product, a sizeof assertion and no external definition: an earlier version defined one and broke the existing control core-purity-hollow, which needs the archive to have no definition once good.c is hollowed), the toy copies, and the toy CMakeLists source list.

### Checks shown able to fail (22 probes on copies of the real inputs outside the tree, plus the six controls)

Each probe ended with exactly the expected token: a checklist line removed (`state-row-missing: stic.row_latch (checklist)`), an extra line, an unknown witness (also `state-cut-unused`), an unknown test, an empty and a two-word exemption, a class word edited in the schema (`state-class-drift` and `state-schema-stale`), the same with the pin re-made (class drift alone), a kind edited, a width and count edited (`state-shape-drift: mem.sysram`), a domain edited, two schema rows swapped (`state-schema-order`), an unused cut, a registered id also pending (`state-pending-stale`), a pending owner outside 08-11 to 08-17, a PAD-kind row classed STATE (`state-class-kind`), the pin line deleted (`state-literals-missing`), the slot suffix edited in the macro body (the step sees the changed row names, so the suffix list is read from the macro), a new literal row added to the table, a constant edited in a header (`state-shape-drift: mem.sysram (table 2x513 ...)`), a class token in the source that differs (`state-class-drift: cpu.r (source IMAGE_BEARING ...)`) and an unresolved name (`state-table-unresolved`). The tokens beyond the plan's have probes and no controls (the plan fixes the control count at 377).

## The checklist, the cuts and the witnesses (Tasks 1 and 3)

- 88 lines; 51 STATE rows with a witness cut, a cut test and sweep `ROW`; 3 IMAGE_BEARING rows (test `state.save.no-image-bytes`); 28 BOUND rows (`state.restore.refuse-identity-profile` for magic, profile and mem.profile, `state.restore.refuse-identity-slots` for the other 25); 6 PAD rows (`state.restore.poisoned-target-equals-source`). `grep -c 'ROW$'` is 51 and `grep -c 'EXEMPT:'` is 0: no exemption.
- Cuts per witness (rows named): HP-SDBD 2, HP-STEAL-A 1, HP-STEAL-B 1, HP-STEAL-C 1, HP-OPEN 1, HP-CLOSED 1, HP-INT-ENTRY 1, HP-HLT 2, HP-HALTED 1, HP-FAULTED 1, HP-RESETPEND 1, HP-FRAME-MID 8, HP-LATCH 7, HP-MOB 1, HP-PSG-MID 10, HP-ENV-MID 5, HP-INPUT 5, HP-LOG 2 (51). The cut set is 18: no cut was added.
- The `hard-points.md` witness lists equal the checklist (a scratch comparison of the 51 pairs).
- Pending lines (7): `state.cut.cpu-points`, `state.cut.stic-points`, `state.cut.device-points` (08-14), `state.save.no-image-bytes` (08-11), `state.restore.refuse-identity-profile`, `state.restore.refuse-identity-slots`, `state.restore.poisoned-target-equals-source` (08-13).

## The container specification (Task 2)

`docs/formats/ivsn-v1.md` holds, in the order of the cartridge-container specification: Conventions, Classes (the four definitions between marker comments, so the reader can be given exactly that range), Layout (offsets 0 to 612 and the trailer), Sections, Size derivation (the table above, row by row), Digests (the eight informational digest fields), Schema digest, Profile identity (the IVSP preimage of four body digests, emitted by plan 08-11), Check order (nine steps with their reasons), Refusal reasons (all 24 names with number, status and meaning; the status `IVS_E_STATE_MISMATCH` -24 is plan 08-11's), Derived invariants (INV-1 to INV-6, each candidate), Rights, Evolution, Worked example and Fuzzing. `grep -c '4,299'` is 5; all 24 reason names are present.

## The hard-point document and the programs (Task 3)

The 18 cuts are defined with program, family, predicate, occurrence rule with arithmetic, what is non-default, and the rows witnessed. The five programs (P-SDBD 13 words, P-HLT 14, P-FAULT 6, P-LATCH 35, P-PSG 60) are listed with decles and cycles; every decle was derived by hand from the printed patterns (checked against the vector files for BZE, BNZE, MVI, MVO, MVO@, XORI, ADDR, CLRR, INCR) and then compared with the output of the project assembler built in a scratch directory outside the tree: all 128 words agree. Arithmetic expectations: HP-SDBD 28 cycles (tick 112), HP-HLT 93 cycles (tick 372), HP-FAULTED 135 cycles (tick 540), P-PSG's second rewrite near cycle 3,246 (tick about 13,000); the other ticks are windows of frame 1 (HP-STEAL-A about tick 74,900, HP-FRAME-MID about 98,100, HP-LATCH 75,400 to 75,700), all pinned by plan 08-14.

## The blind-read request

`docs/state/checklist-second-read-request.md` (first line `pending: blind read not yet run`) gives the reader the five blinded header copies and the extracted class definitions and nothing else; `grep -c 'machine_digest\|state-checklist\|schema.literal'` counts 2 lines, the sentence of the reader's description that says what it never sees and the Forbidden sentence, both exclusions. No `docs/state/checklist-second-read-record.md` exists. SHA-256 of the Classes section of `docs/formats/ivsn-v1.md` (the sed range between the two marker comments, 6 lines): `e23d78160147e71915eca6c0b34da888ad34fe432652de18f36003a09b3eb179`; plan 08-10 can prove with `sed -n '/^<!-- classes-begin -->$/,/^<!-- classes-end -->$/p' docs/formats/ivsn-v1.md | sha256sum` that the definitions were not edited after the request.

## Deviations from Plan

**1. [Interpretation] Witness assignments (Rule 1, plan text contradicts its own check).** The plan's examples name HP-OPEN for both cpu.prev_interruptible and cpu.intff, HP-HALTED for cpu.run_state, HP-STEAL-B for all four blocked and grant counters and no row for HP-STEAL-C, HP-CLOSED, HP-INT-ENTRY and HP-RESETPEND, while the step token `state-cut-unused` requires each cut to be the witness of a row. I kept one witness per row (the grammar and plan 08-14's `$5=="ROW"` count) and chose the assignments of the key-decisions above; each is argued per row in `docs/state/hidden-state-checklist.md`. Two were revised in Task 3 after the lamp run's vector turned out to be JD (interrupts off after the first jump) and acceptance leaves the enable set in this profile (HP-CLOSED and the cpu.intff witness); the checklist and the cut definitions were edited in the same commit, as the plan allows.

**2. [Interpretation] The step is richer than the plan.** More tokens (29 against 14), the constant evaluator over header defines, the schema order and shape checks and the kind-class consistency. The reason is lesson 7 and T-08-25: a hand-derived 88-line literal with widths and counts is checked against the table text now and not at plan 08-11. No control was added for the extra tokens (the count is fixed); the probes above show each fails.

**3. [Interpretation] The toy table already carries class tokens (plan 08-10 expected to add them).** The seventh-field branch of the step is thus exercised by the positive control now; the plan's later control `state-class-source-drift` needs only an overlay.

**4. [Information] Files.** Task 3 also edited `data/expected/state-checklist.ids` (two witness lines), as its step 4 allows. No file beyond the plan's lists. No change to core, runner, tests, fixtures or scenarios in any of the three commits (`git show --stat` filter prints 0 for each).

No auth gates, no package installs (T-08-SC), no `gh` call, no push, no tag, no `tools/ci-record.sh`; `.planning/config.json` and `.planning/state.json` were never staged.

## Decisions Made

- One witness per row and every cut used: the coverage rule of the step is the discipline that stops a cut from being decorative.
- Predicates are over fields of the instance, and a cut that never holds in its program is a finding of plan 08-14 (appended note), not a silent change: the document names the lines of the lamp run it expects (JD vector, EIS at frame-0 cycles 6,564 to 6,631) so the scan has a target.
- The sweep value rule is in the oracle now, because a constant fill can coincide with a flag's reference value.

## Cost (laptop)

| Item | Value |
|------|-------|
| estimate of the plan | 150,000 tokens (raw, confidence low), 3 tasks |
| actuals | 30,321 tokens (chars/4 over the 37 files changed by the three task commits), 3 tasks, 3 commits (measured from the ledger of the plan head) |
| gate total, sh, at plan start | 76 s (gate-cpu user 137.51 sys 111.48) |
| gate total, sh and dash, Task 1 tree | 82 s and 80 s, identical summary line |
| gate total, sh and dash, Task 2 tree | 80 s and 84 s, identical summary line |
| gate total, sh and dash, Task 3 tree | 85 s and 83 s, identical summary line |
| container gate, Task 1 tree | 316 s (gcc 13.3.0, cmake 3.28.3, x86_64 emulated, state step on mawk) |
| container gate, Task 3 tree | 320 s (gcc 13.3.0, cmake 3.28.3, x86_64 emulated), exit 0, 0 FAIL lines |
| `state` step in isolation | 0 s of the gate's whole-second timer; the 22 probes ran in a few seconds in total |

The plan adds one gate step and six controls and no test id, fault, scenario, golden or consumer line: the estimated hosted cost is the six controls at about 0.07 local cpu-s each plus the step at under 0.05, about 0.5 local cpu-s in all (D-37 charged wave B 12 to 16 hosted cpu-s, almost all of it later plans). The stop rule of checkpoint A is not in force (resolved no-lever); no id, fault or control of waves B and C was added here beyond the plan's six controls.

## Open Items

- **Orchestrator: the blind read (D-05).** Build the six blinded files under `build/review/blind/` with the commands of `docs/state/checklist-second-read-request.md`, give one fresh isolated reader those six files and the output form of that request only, compare its member list with the 88 rows and its classes with the checklist, write `docs/state/checklist-second-read-record.md` (head `state: closed`, `oracle-commit: <40 hex>` of the last commit of this plan, `corrections: <n>`, the `correction:` lines, the prose) and commit it. A member the reader finds that no row covers is a defect of the table itself (the checklist has one line per table row): decide it before closing the record. Plan 08-10 starts only after that commit and applies the corrections as its first commit.
- **Plan 08-14: three cuts may not occur as defined in the lamp run.** HP-OPEN, HP-CLOSED and HP-INT-ENTRY assume the vector JD behaviour and the first EIS at frame-0 cycles 6,564 to 6,631; if the scan finds none, the plan edits the program or the predicate in an appended note and reports.
- **Plan 08-14: `stic.late_grants`** is non-default only after a frame with an ungranted window; the document gives the construction that forces one (a non-interruptible stretch of at least 1,026 cycles) or the exemption path.
- **Plan 08-14: the sweep value** (the complement rule) is written in the narrative, not in the checklist grammar.
- **Plan 08-10:** the toy table already has class tokens; the step's source branch is live as soon as the real table gets a seventh argument.
- **Information.** D10 and D11 are not performed; no claim above provisional exists; no hosted-only check was run; STA-01 and STA-02 stay Pending until the serializer, restore and the hard-point suite exist; REL-01 stays Pending (D-28).

## Interfaces for the next plans

- The step name and number: `state`, step 38; gate counts after this plan `steps=38/38 controls=377/377 faults=226/226 tests-expected=4772 tests-executed=4768 tests-skipped=4 tests-missing=0 state-rows=88 state-cuts=18` (plan 08-10 adds control 378, plan 08-14 pins the cut ticks without changing `state-cuts`).
- The checklist grammar `<row> <class> <witness> <test> <sweep>`, the cut grammar `<cut-id> <program> <pinned-tick> <family>` (the token `pending-08-14` until plan 08-14), the pending grammar `<test-id> <owner-plan>` (owners 08-11 to 08-17; each owner deletes its lines in the commit that registers the ids).
- Plan 08-10 renames the unused `reserved` member of `ivs_digest_field` to `snap` and writes the seventh argument of the rows; the step compares it with the checklist.
- Plan 08-11 must produce the schema digest `6cd4932f5ebf0b0fc2377555ca079d0313bb27a4df1a4fda9e15f37c6eb1d193` from the table descriptors and sizes 35, 2,761, 615, 51, 63, 79, 51 (3,655) and 4,299 from the writer; a difference is a finding about the hand derivation first.
- Threat ids T-08-24 and T-08-25 are used here (T-08-SC throughout); the next state plans continue from T-08-26 or the numbering their plans assign.

## Final gate

```
sh tools/gate.sh:      gate: PASS steps=38/38 controls=377/377 ... faults=226/226 ... tests-expected=4772 tests-executed=4768 tests-skipped=4 tests-missing=0 ... state-rows=88 state-cuts=18 (0 FAIL lines)
dash tools/gate.sh:    the identical summary line (0 FAIL lines)
GATE_LINUX_PLATFORM= sh tools/gate-linux.sh: exit 0 on the Task 3 tree, build/gate-linux.log ends gate: PASS (gcc 13.3.0, cmake 3.28.3, x86_64 emulated), the plan-end counts, 0 FAIL lines
```

After the Task 1 tree: sh and dash `steps=38/38 controls=372/372`, container exit 0 (316 s). After the Task 2 tree: `controls=377/377` under sh and dash. On the closing tree (this SUMMARY, STATE.md and ROADMAP.md added or edited; `tracked=2428`): sh (82 s) and dash (81 s) `gate: PASS` with the identical summary line (md5 38fe52f73a2d1f8ae67ee4a2eeac091e), 0 `FAIL` lines, and the container gate was run again (`GATE_LINUX_PLATFORM= sh tools/gate-linux.sh`): exit 0, `gate: PASS` with the plan-end counts, 0 `FAIL` lines, gate-time total=324 s (gcc 13.3.0, cmake 3.28.3, x86_64 emulated); `build/gate-linux.log` holds that run. The first gate run of every task passed except one: the first Task 1 run showed `controls=371/372` because the toy table defined an external function and the existing control `core-purity-hollow` (a hollowed `good.c`) then no longer produced an archive without definitions; the toy table was changed to a `sizeof` assertion and the next run passed (the only retry).

## Known Stubs

None.

## Threat Flags

None new. T-08-24 (a field classed PAD or BOUND that carries state): one line per row checked against the table text, classes agree between the schema literal and the checklist (and, from plan 08-10, the source), every STATE row has a witness cut and a named test, the kind-class consistency is checked, the blind read of the member list is requested; the runtime memcmp and the row sweep follow in plans 08-13 and 08-14. T-08-25 (an awk reader that misses a row hidden behind a macro): the suffix list is read from the macro body, the toy tree exercises literal, macro and product-count rows, 22 probes and six controls show each failure, and the C writer of plan 08-11 must equal the pinned digest. T-08-SC: no installs.

## Self-Check: PASSED

Verified before the final commit: `.planning/phases/08-stic-completion-save-state-performance-baselines/08-09-SUMMARY.md` and the 13 created files of the plan exist (14 FOUND); commits `06d4820`, `ae78cb8` and `02899e1` are ancestors of HEAD and `git rev-list --count 2decaf176a20cadf42445a0bfb983b2a18037a52..HEAD` printed 3 before this file's commit; `.planning/config.json` and `.planning/state.json` are not staged; `git ls-files` lists no `.ivsc`, `.bin`, `.rom`, `.exec`, `.ivsn`, `.wav`, `.ppm` or `.vraw` file; none of the three task commits touches `core/`, `runner/`, `tests/`, `fixtures/` or `data/scenarios/` (the filter of the acceptance criterion prints 0 for each) and none deletes a file; `head -1 docs/state/checklist-second-read-request.md` is `pending: blind read not yet run` and `docs/state/checklist-second-read-record.md` does not exist; `grep -c 'ROW$' data/expected/state-checklist.ids` 51, `grep -c 'EXEMPT:'` 0, `grep -c 'schema-digest-sha256' data/expected/ivsn-literals.ids` 1, `grep -c '^state-' data/expected/controls/state.ids` 6, `grep -c '4,299' docs/formats/ivsn-v1.md` 5, all 24 reason names present; no home, scratch or temp path and no at sign before a dotted word in any file of the plan.
