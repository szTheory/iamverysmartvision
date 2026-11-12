---
phase: 05-hand-controllers-and-psg-sound
plan: 01
subsystem: data, docs, tools/check
tags: [oracle-first, sources, controller-literal, psg-rules, psg-vectors, discrepancies, d10, listing, D-23, INP-01, INP-02, INP-03, API-03, AUD-01, AUD-02]
status: complete

requires:
  - phase: 04-headless-runner-and-installed-consumer
    provides: "the gate (steps=33/33 controls=277/277 faults=149/149 tests-expected=2818), the record-kind driver, the literal-file idiom (memmap), the vector-file idiom, the listing step, the discrepancy log"
provides:
  - "docs/psg/reference-notes.md: the AY-3-8910 page-reading verdict (four literal lines), the octal register numbering note, the rule table and ONE allocation table (C69..C80, CL-0009..CL-0015)"
  - "data/input/intv-2609-ntsc-family-p1.literal.txt: 31 controller rows (pins, cleared mask, wiki value, byte), transcribed from the cached sources before any planning-note table was opened; data/expected/input-policies.ids pins the D-03 result classes and tags"
  - "data/psg/rules.json (PSG-R01..R21), 120 vectors in 11 files under data/psg/vectors/ (95 source, 25 policy, 12 policy ids), data/psg/intv-2609-ntsc-family-p1.regmap.literal.txt (16 rows, octal and decimal register numbers)"
  - "gate steps `input` (9 tokens, 6 controls) and `psg-vectors` (7 tokens, 7 controls); listing rule listing-branch-target (2 controls); discrepancies d10-measurements cross-check (2 controls)"
  - "discrepancies C69..C80 with updates to C11, C12, C14, C15, C16, C23, C60, C63..C67; data/expected/d10-measurements.ids (32 items); docs/input/d10-procedure.md"
  - "docs/time-base.md corrected: one PSG step = 8 PSG chip clocks = 16 master ticks = 4 CPU cycles"
  - "docs/input/diag-ctrl-psg-1-derivation.md (hand derivation of the second fixture) and pending blind second-read requests"
affects: [05-02, 05-03, 05-04, 05-05, 05-06, 05-07, 05-08, 05-09, 05-10, 05-11, 05-12]

actuals:
  tokens: 61000
  tasks: 5
  commits: 5
plan_head_before: 7857ebfec388e1575be3d9036c815a715244b4a5
plan_head_after: 14a19043e503c58dcefcb50020e3b32f8a731727

tech-stack:
  added: []
  patterns:
    - "Oracle-first plan: every expectation later plans test against is committed from the sources, with a self-consistency gate, before any code exists"
    - "Pure text gate step with a pinned class table inside the checker (input) next to a `.ids` inventory that must equal it"
    - "Rule inventory records with a closed vector grammar, both-direction coverage and per-token controls (psg-vectors)"

key-files:
  created:
    - docs/psg/reference-notes.md
    - docs/input/controller-notes.md
    - docs/input/d10-procedure.md
    - docs/input/diag-ctrl-psg-1-derivation.md
    - docs/input/second-reader-protocol.md
    - docs/psg/second-reader-protocol.md
    - docs/input/second-read-request.md
    - docs/psg/second-read-request.md
    - data/input/intv-2609-ntsc-family-p1.literal.txt
    - data/psg/rules.json
    - data/psg/intv-2609-ntsc-family-p1.regmap.literal.txt
    - data/psg/vectors/regs.txt, reset.txt, reset-engine.txt, tone.txt, noise.txt, env.txt, mix.txt, amp.txt, ampenv.txt, port.txt, dac.txt
    - data/expected/d10-measurements.ids, input-policies.ids, psg-rules.ids, psg-policies.ids, psg-vector-files.ids
    - tools/check/steps/input.sh, tools/check/input.cmake, tools/check/steps/psg-vectors.sh, tools/check/psg-vectors.cmake
    - tools/check/schema/psg-rule.txt, tools/check/kinds/psg-rule.cmake
  modified:
    - data/sources.json, data/expected/sources.ids
    - data/discrepancies.json, data/expected/discrepancies.ids
    - tools/check/kinds/discrepancy.cmake, tools/check/listing.cmake
    - docs/time-base.md, docs/diagnostics.md
    - fixtures/FX-boot-stub/boot-stub.listing.txt, fixtures/FX-diag-cpu-1/diag-cpu-1.listing.txt and both manifests

key-decisions:
  - "Page-reading verdict of the 1980 catalog pages 5-23..5-29: shapes-printed yes; equations-printed, amplitude-printed and dac-labels-printed no. Vectors that need only the printed divide chains, field widths and shapes stay kind=source; the DAC rows and everything needing an unprinted fact are kind=policy (D-23 fallback applied per vector, not to every vector); C12 stays partial"
  - "The AY-3-8910 reprint (SRC-gi-ay8910-manual) and the 1998 pin chart (SRC-tilton-intv-pinouts) could not be obtained; both are registered reference-only and no vector or row cites them"
  - "The controller literal cites the re-read ConsoleMods pin matrix for pins and the wiki key-table image for the wiki value; all 31 rows agree, none was corrected"
  - "vector `read:` in families reg and reset evaluates the register store (ports are stores there); the pin-following read is family port only"
  - "A third module-level expect kind `table` (DAC table) was added beside gate and index, so the DAC vectors have a grammar slot"

duration: about 40 min of edits plus six gate runs (first commit 01:33, last 01:59)
completed: 2026-11-12
---

# Phase 5 Plan 01: sources and oracles Summary

**The Phase 5 oracle set is committed with no C: sources and page-reading verdict, the 31-row controller literal, the PSG rule inventory with 120 vectors and the register-address literal, one allocation table (C69..C80, CL-0009..CL-0015), the D10/D11 inventory, the branch-target listing rule and the hand derivation of the fixture. The gate stays green at `steps=35/35 controls=294/294 faults=149/149`.**

## What was built

Commits (oracle commit = the last one, `14a1904`; every later Phase 5 SUMMARY names it):

- Task 1 (`bf70e31`): both catalogs fetched into the gitignored cache and hash-verified (`refs-verified=3/4`); the AY-3-8910 pages re-read; three sources registered reference-only; `docs/psg/reference-notes.md` (verdict, numbering) and `docs/input/controller-notes.md`.
- Task 2 (`9237dc4`): allocation table, discrepancy records C69..C80 and the updates of D-06/D-12/P5-C5, `d10-measurements.ids` (32 items) with the `discrepancies` cross-check and 2 controls, `docs/input/d10-procedure.md`, the `docs/time-base.md` correction (`controls=279/279`).
- Task 3 (`dfd7efe`): the controller literal, `input-policies.ids`, the `input` step with 6 controls (`steps=34/34 controls=285/285`, `input-rows=31`).
- Task 4 (`57aa5b4`): rules, vectors, register-address literal, the `psg-vectors` step with 7 controls (`steps=35/35 controls=292/292`, `psg-rules=21 psg-vectors=120`).
- Task 5 (`14a1904`): listing branch-target rule with 2 controls, annotated listings and refreshed manifests, fixture derivation, second-reader protocols and requests (`controls=294/294`).

## Page-reading verdict (decides SC4's tier)

Read on the 150 and 300 dpi renders of pdf 204-210 of the 1980 catalog (printed 5-23..5-29), and the same text on pdf 299-305 of the 1982 catalog:

```
equations-printed: no
shapes-printed: yes
amplitude-printed: no
dac-labels-printed: no
```

- Printed: the divide chains in words (input clock by 16 then the 12-bit tone period; by 16 then the 5-bit noise period; by 256 then the 16-bit envelope period, then 16 states), the field widths, Fig. 3 (ten shape classes for R15 B3..B0), Fig. 4/5 (sixteen states labelled 0..15), the amplitude register diagram (mode bit and 4-bit fixed level), the R7 diagram with overbars on the enable labels, pull-ups on every port pin, reset of all registers to 0, the 1.0 to 2.0 MHz input range.
- Not printed: closed-form frequency equations, the period range sentences, the worked envelope example, the sense of the amplitude mode bit, any DAC voltage label, the sense of the port direction bits (the overbar on that cell was not resolvable at 300 dpi), any statement about the 8914.
- Effect: shape vectors and the printed-chain rate vectors stay `kind=source`; DAC vectors, noise period zero, LFSR, port and generator-reset rows are `kind=policy`; C12 stays `partial`, C23 `partial`. The 1980-catalog dac and equation topics that exist only in the unobtainable reprint are cited nowhere.

## Counts

- Start: `steps=33/33 controls=277/277 faults=149/149 tests-expected=2818 tests-missing=0`.
- End: `steps=35/35 controls=294/294 faults=149/149 tests-expected=2818 tests-executed=2816 tests-skipped=2 tests-missing=0`; controls 277 + 2 (discrepancies d10) + 6 (input) + 7 (psg-vectors) + 2 (listing) = 294; `discrepancies=80 open=72 pending-resolvers=13 d10-items=32 input-rows=31 input-policies=11 psg-rules=21 psg-vectors=120 psg-policies=12 sources=12 refs-verified=3/4 listings=2`.
- Open discrepancies whose resolver starts `measurement:` (open or partial): 26 (C1 C2 C3 C4 C6 C9 C16 C21 C32 C59 C60 C61 C62 C63 C64 C65 C66 C69 C70 C71 C72 C73 C74 C75 C76 C77), all named by `d10-measurements.ids` (32 lines: 12 system, 7 input, 11 PSG, 2 D11).
- Vectors: regs 28, reset 2, reset-engine 2, tone 16, noise 12, env 22, mix 10, amp 6, ampenv 4, port 10, dac 8; 95 source and 25 policy.
- The expectations of the 120 vectors were re-checked with a throwaway second model written afterwards (all agree; a shared-misreading risk, not an independent implementation). No file under `core/`, `runner/`, `tools/pack/`, `tools/pcm/` or a test `.c` changed; `git ls-files refs` is empty.

## Deviations from Plan

**1. [Rule 1 - Inaccuracy] `docs/psg/reference-notes.md` cites the catalog, not the reprint.** The plan expects a hash-verified reprint under `refs/`; it could not be obtained (public CDN address not recorded; a different 8-page scan only). `SRC-gi-ay8910-manual` is reference-only with the discuss-recorded hash in its note, and no row cites it.

**2. [Rule 1] The 1998 pin chart could not be re-fetched** (archive address unknown, rate-limited, search denied): `SRC-tilton-intv-pinouts` is reference-only, no row cites it. The ConsoleMods capture fetched is 35,364 bytes with prefix 82ad0ff1, not 36,791 bytes and 984d548c: recorded as a difference in its note, not accepted as a match. The wiki page html differs byte for byte between fetches (per-request tokens); only the image hash (f789cc5a..., 7,498 bytes) identifies a copy and it matches.

**3. [Rule 1] D-23 fallback applied per vector.** Taken literally, `equations-printed: no` would turn every rate vector into policy. The divide chains and field widths are printed, so the rate vectors stay source with the chain as their cite and the equation form and 50 percent duty named as inference; only vectors needing an unprinted fact are policy (see verdict).

**4. [Rule 3] Rule file shape.** The plan says a top-level array; the record driver requires an object with an array member, so `data/psg/rules.json` is `{schema_version, note, rules: [...]}`.

**5. [Rule 2] Extra inventory `data/expected/psg-vector-files.ids`** (not in the plan's file list): the step's "file list" for `psg-vector-unlisted-file` has to be per tree for the toy-tree controls. Also the hard-coded D-03 class table lives in `input.cmake`, with `input-policies.ids` as the pinned inventory the step compares to it.

**6. [Rule 2] New expect kind `table`** for the DAC vectors (the plan's kind list has no slot for a table comparison); it is module level like `gate` and `index`.

**7. Plan figures corrected from the derivation.** One poll loop is 81 CPU cycles = 324 ticks (not about 30 cycles); the program has 89 words and halts near tick 16,700 (not 16,200); both are estimates and below the 20,000-tick cap. Inventory baseline is `tests-expected=2818` (the plan text says 2816).

**8. [Rule 3] Toy-tree growth for the listing rule.** The good tree needed a toy branch row `B` (family table, rows inventory, pending list, second-reader record) and the `sources-no-files` control needed `delete data/psg/rules.json` so the new toy data file is removed with the others.

**9. Source registry entry for the reprint uses the 1980 catalog URL in its `url` field** (the schema requires a non-blank url; the note explains).

## Decisions Made

- Allocation table first: C69..C80 and CL-0009..CL-0015 are reserved in `docs/psg/reference-notes.md`; the next free ids are C81 and CL-0016.
- Policy ids POL-PSG-01..12 are pinned in `data/expected/psg-policies.ids`; a policy no vector names fails the step.
- Register-address literal rows carry both numberings (`gi=R13 n=11`); a bare register number in another table is a defect (C80).

## Open Items

- **Pending blind second reads (orchestrator):** `docs/input/second-read-request.md` (1 reader, the controller literal) and `docs/psg/second-read-request.md` (11 readers, one per vector file, the register literal read with `regs`). State `pending: orchestrator-run blind reader`; no blind-copy tool exists yet (the requests describe the projection).
- **Not obtained:** the AY-3-8910 reprint (its equations, ranges, worked example and DAC labels are unconfirmed by any page read), the 1998 pin chart; no schematic of the controller.
- **Later plans:** 05-04 compares the profile `psg.regmap` with the register literal and creates `fields-pending.ids` and `vectors-pending.ids`; 05-05 adds the profile-versus-literal comparison to the `input` step; 05-11 turns the derivation into the golden and the listing; claims CL-0009..CL-0015 are written by 05-11 and 05-12.
- **D10/D11:** 32 inventory items are handed off, nothing performed; the reader cartridge is Phase 7.
- Carried from Phase 4: IN-04, IN-06, IN-09, IN-10, IN-11 stay open; `sh tools/hygiene.sh --history` still fails on earlier planning commits.
- `gate-linux` was not run (the plan does not require it for this plan).

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=35/35 controls=294/294 ... faults=149/149 ... d10-items=32 ... input-rows=31 input-policies=11 ... psg-rules=21 psg-vectors=120 psg-policies=12 ... sources=12 refs-verified=3/4 tests-expected=2818 tests-executed=2816 tests-skipped=2 tests-missing=0 ...`, `gate-time total=31` (load moderate).
- `dash tools/gate.sh`: the same line, `gate: PASS`, `gate-time total=31`.
- `sh tools/gate-linux.sh`: not run.

## Threat Flags

None. Records, docs and checkers only; no network endpoint, no file access pattern, no schema at a trust boundary.

Mitigations from the plan's threat model: T-05-01 (PDFs, images and fetched pages only under `refs/`, `git ls-files refs` empty), T-05-02 (literals and vectors written from the cached sources before any generator exists; the `input` step cross-checks pins, cleared mask, wiki value and byte; blind second reads requested), T-05-03 (no home, scratch or temp path in a tracked file; `hygiene` green), T-05-04 (13 controls on the two new steps plus 4 on the extended checkers; toy-tree positive controls), T-05-SC (no package installs).

## Self-Check: PASSED

- Files exist: docs/psg/reference-notes.md, docs/input/controller-notes.md, docs/input/d10-procedure.md, docs/input/diag-ctrl-psg-1-derivation.md, data/input/intv-2609-ntsc-family-p1.literal.txt, data/psg/rules.json, data/psg/intv-2609-ntsc-family-p1.regmap.literal.txt, data/psg/vectors/*.txt (11), data/expected/d10-measurements.ids, tools/check/input.cmake, tools/check/psg-vectors.cmake.
- Commits on HEAD: bf70e31, 9237dc4, dfd7efe, 57aa5b4, 14a1904.
