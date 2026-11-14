---
phase: 07-first-visible-original-scenario
plan: 04
subsystem: project mini-assembler ivs_asm (tools/asm) completed: full table, explicit SDBD, DECLE with row DATA, pairs corpus, four twins, pinned refusals; DATA rule of the listing checker; D4 documentation; discrepancy C106
tags: [W2b, assembler, D4, ivs_asm, twins, pairs-corpus, DATA-row, refusal-tokens, D-16, D-17, D-18, D-19, A-01, A-10, A-15, A-18, A-22, T-07-08, T-07-09, T-07-10, V2]
status: complete

requires:
  - phase: 07-first-visible-original-scenario
    provides: "plan 07-03: gate steps=36/36 controls=350/350 faults=222/222 listings=4 listing-words=300 discrepancies=105 tests-skipped=2 tests-missing=0, HEAD 214e6b3 at the start of this plan; ivs_asm tracer of 806 lines with 19 mnemonics and 17 tokens; oracle frozen"
provides:
  - "tools/asm/asm.c: ivs_asm complete, one C17 file of 1,137 lines (hard stop 1,200): 78 spellings covering 73 of the 74 leaf rows (all but MVOI) and the five jump forms of row J, explicit SDBD, DECLE with row DATA, --pairs mode, the 20 refusal tokens"
  - "tests/asm/pairs.txt: 304 hand-derived asm and word pairs in nine row families (301 copied once from the Phase 2 vectors, three hand cases), no JSON at test time"
  - "tests/asm/twins/diag-ctrl-psg-1.asm and diag-stic-1.asm (its pattern table as DECLE): with the two earlier twins they reproduce all 300 hand-typed words (22 + 56 + 89 + 133)"
  - "data/expected/asm-tokens.ids: the 20 pinned refusal tokens, 18 tracked sources under tests/asm/neg, the CR and over-long-line cases built at run time, asm.neg-coverage"
  - "tools/check/listing.cmake: the DATA row rule; controls listing-data-in-operand-slot and listing-data-after-sdbd (350 to 352); the pristine toy listing holds one valid DATA word"
  - "docs/diagnostics.md: the ivs_asm section (decision, grammar, generated listing form, explicit SDBD rule, twin discipline, what reverses the decision, effort data) replacing the text on when an assembler becomes worth writing"
  - "discrepancy C106 (data/discrepancies.json, data/expected/discrepancies.ids), 105 to 106"
affects: [phase-7-plans-07-05-to-07-12, plan-07-05-stub-2-recipe-and-regen, plan-07-06-program]

actuals:
  tokens: 18135
  tasks: 4
  commits: 5
plan_head_before: 214e6b36d4b488ecf8b097103e9d41e3b2ea1a9b
plan_head_after: 06875830c13efa9dbf35982baa31d25e61370150

key-decisions:
  - "asm-imm-range made reachable: the expression evaluator now allows partial results 0 to 16777215 and each use limits its own value (immediate, address, target and ORG at most 65535, data word and direct address at most 03FF); under the old 0 to 65535 rule the token was unreachable, as the 07-03 summary said"
  - "MVOI has no spelling (its operand word is the cell it overwrites, so it carries no value): 73 of 74 leaf rows are assembled, which is every leaf row that has a spelling in the repository dialect"
  - "Header keys are limited to the eight of the listing grammar and each may appear once (asm-header), so a header holds at most 8 lines and the old 32-line limit was dead code and was removed"
  - "The corpus copies every vector whose asm text is plain statements and whose words are all at most 03FF (301), masks the filler upper bits of SDBD immediate operand words to their low byte (the manual marks them Not Used), and leaves out what the dialect refuses by design or the vector uses to test the core"
  - "The pinned tokens equal the tokens asm.c can print (asm.neg-coverage compares the two), one case per token; the CR (asm-syntax) and over-long line (asm-limit) sources are built at run time so no tracked file holds a CR (A-22)"

key-files:
  created:
    - tests/asm/pairs.txt
    - tests/asm/twins/diag-ctrl-psg-1.asm
    - tests/asm/twins/diag-stic-1.asm
    - tests/asm/neg/ (18 sources, one per token except asm-syntax and asm-limit)
    - tests/asm/pos/branch-03ff.asm
    - data/expected/asm-tokens.ids
    - tools/check/controls/listing-data-in-operand-slot/
    - tools/check/controls/listing-data-after-sdbd/
  modified:
    - tools/asm/asm.c
    - tests/asm-tap.sh
    - data/expected/tests/asm.ids
    - tools/check/listing.cmake
    - tools/check/controls/_good/tree/data/listings/toy.listing.txt
    - data/expected/controls/listing.ids
    - docs/diagnostics.md
    - data/discrepancies.json
    - data/expected/discrepancies.ids
    - .planning/STATE.md
    - .planning/ROADMAP.md

completed: 2026-11-13
---

# Phase 7 Plan 04: ivs_asm complete Summary

**`ivs_asm` now assembles every leaf row that has a spelling (73 of 74, 78 spellings), with explicit SDBD and DECLE data words; 304 hand-derived pairs and the four twins (all 300 hand-typed words) agree with its hand-written table at the first run, so no hand slip and no table error was found; all 20 refusal tokens are pinned with one case each; the checker accepts the DATA row it emits, with two controls; the D4 decision and the branch-range note (C106) are documented.**

## Commits

| Commit | Task | What |
|--------|------|------|
| `e4f7dab` | 1 (tracer) | full table, SDBD, DECLE with row DATA, twins of diag-ctrl-psg-1 and diag-stic-1, ids |
| `969c516` | 2 | `--pairs` mode, `tests/asm/pairs.txt` (304 pairs), `asm.pairs.<class>` x 9, `asm.judge.mutated-pair`, optional NOP and SIN operand |
| `55bb7bd` | 3 | `asm-tokens.ids`, 18 negative sources, `asm.neg.<token>` x 20, `asm.neg-coverage`, seven `asm.var` cases, `asm.boundary.branch-03ff`, header keys limited to the listing grammar |
| `b05a222` | fix | gcc 13 `-Wsign-conversion` in the pairs result line (see Deviations 1) |
| `0687583` | 4 | DATA rule and two controls, the ivs_asm section of docs/diagnostics.md, C106 |
| the commit that adds this file | | this SUMMARY, STATE.md, ROADMAP.md |

## What was built

- **Table** (D-17): 78 spellings written by hand from `docs/cp1610/reference-notes.md` and the manual pages it cites (pages 53, 54, 55, 56 were read as page images from the local cache for the shift, branch, indirect and jump tables, which the notes defer to the manual). 73 leaf rows plus the five jump forms (`JE JD JSR JSRE JSRD`, all row `J`). Not spelled: `MVOI`. `JD` is there for stub contract 2 of plan 07-05. Entries are placed by 18 operand kinds with one letter-per-operand shape table (`r # a n`, upper case optional): shifts take an optional count, `NOP` and `SIN` an optional 0 or 1.
- **SDBD** (D-17): explicit prefix only; after it an immediate row (`MVII ADDI SUBI CMPI ANDI XORI`) writes a low byte word and a high byte word, or an indirect read row (`MVI@ ADD@ SUB@ CMP@ AND@ XOR@`) through R1 to R5; anything else, an SDBD at the end, or an SDBD before `ORG`, `DECLE` or `EQU` is `asm-sdbd-target`. Pass 1 counts an SDBD immediate as three words, so there is no size fixpoint; a forward label above 03FF without the prefix is `asm-imm-sdbd` in pass 2.
- **DECLE** (D-16, A-10): data words at most 03FF (`asm-decle-range`), row `DATA`, text `DECLE $XXXX`; up to 127 values per line.
- **Refusals**: spelling rules `MOVR R1,R1` (TSTR), `MOVR R1,R7` (JR), `XORR R1,R1` (CLRR), `MVI@ R6` (PULR), `MVO@ ..., R6` (PSHR), pointer R7; also `MVO@` with the source equal to the pointer 4 to 7 (the manual marks it unsupported, pdf 55), `JSR` register outside R4 to R6, a branch or jump target above FFFF (no wraparound), a shift count other than 1 or 2, a `BEXT` code above 15, an `NOP` or `SIN` operand above 1, `ORG` above FFFF.
- **`--pairs`** mode: one process over a flat corpus, one result line per pair (`ok CLASS LINE` or `bad CLASS LINE EXPECTED GOT`); statements of a pair are separated by a semicolon, so `SDBD; ANDI ...` is one pair.
- **Corpus** `tests/asm/pairs.txt`: 304 lines in nine families (alu 76, branch 96, control 24, imm 7, jump 10, move 26, sdbd 15, shift 43, stack 7). 301 copied by a throwaway script (not committed) from the `asm` and `program` fields of `data/cp1610/vectors/*.json` with the start address `init.r[7]`; 94 vectors with a program were left out (58 asm texts that are not plain statements, 17 with a program word above 03FF, 11 SDBD before a non-target, 2 SDBD alone or twice, 2 NOPP with a displacement word, 2 `MVO@` source equal to the pointer, 1 wrapping program, 1 wrapping branch); the header names the source files, the date and the rules. Three hand cases: `XORI #$0155, R2` (03FA, 0155), `SDBD; ANDI #$1234, R1` (0001, 03B9, 0034, 0012) and `NOPP` (0208, 0000), each derived in its comment from the notes. First run: 304 of 304 ok.
- **Twins**: `diag-ctrl-psg-1.asm` (89 words, labels `poll`, `port1`, `test`, `delay`) and `diag-stic-1.asm` (133 words: 101 of code with eight SDBD immediates, 32 data words as four `DECLE` lines) typed from the ASM columns of the hand listings. Both equal the hand listings word for word through `ivs_pack --dump-words` at the first run.
- **Tests** (suite `asm`, 45 cases under the existing `c-tests`, no new step, job or dependency): 4 twins, `asm.repeat`, `asm.judge.mutated-twin`, 9 `asm.pairs.<class>`, `asm.judge.mutated-pair`, 20 `asm.neg.<token>`, `asm.neg-coverage`, 7 `asm.var.<slug>` (five spelling causes, SDBD at the end, SDBD before a stack-mode indirect read) and `asm.boundary.branch-03ff` (displacement 03FF accepted: words 0200 and 03FF; 0400 is `asm.neg.asm-branch-range`). A refusal must exit 1, print nothing on stdout, print exactly one `ivs_asm: <token>: ...` stderr line and leave no `.tmp` or listing. `asm.neg-coverage` fails if the pinned tokens differ from the `"asm-..."` strings of `tools/asm/asm.c`, a token is pinned twice, or a file under `tests/asm/neg` names no pinned token.
- **Checker** (A-10, T-07-08): `listing.cmake` skips the leaf-row label for row `DATA` and nothing else changes: the existing paths already refuse a DATA word while operands are pending (`listing-operand-count`) and directly after SDBD (`listing-sdbd`), and bits and range still apply. The two controls were run in a scratch copy and each prints exactly one finding (`MVII lacks 1 operand line(s)`, `SDBD before DATA, which is not an SDBD target`); the pristine toy tree with a DATA word passes.
- **Docs**: `docs/diagnostics.md` states the decision (the trigger fired on three counts, break-even on Phase 7 alone, adopted for labels and the stub-to-program reference), the grammar, the generated-listing form (`# generated by` line, `# sym:` lines, DATA rows), the explicit-SDBD rule, the twin discipline, what reverses the decision and the effort data; the two statements at its top were reworded to stay true (no third-party assembler is a dependency; the project tool is build-only and never shipped). C106 records the manual's printed +1025/-1024 against the decle arithmetic (forward `pc + 1025`, backward `pc - 1022`); the assembler follows the decle limit; no hardware claim.

## Measurements and scratch evidence (none is a hosted figure)

- `wc -l tools/asm/asm.c` = **1,137** (budget 700 to 900, hard stop 1,200; 63 lines of headroom). The forecast of the 07-03 summary was about 975; the table block with its page cites (49 lines), the pairs mode (102 lines: `as_pair` and `as_run_pairs`), the 18-kind encode switch with its spelling refusals, `DECLE` and the header-key table account for the difference. Table packing and comment trimming are the levers if later plans need room.
- Rows covered: 73 of 74 leaf rows (not `MVOI`); 78 spellings; 77 mnemonic spellings are exercised by at least one pair, the 78th (`NOPP`) by a hand case.
- Hand slips found by the twins: **0**. Table errors found by the corpus: **0**. Encoding escapes (an error found by a run only): **0**. `asm-imm-sdbd` hits while writing a real source: **0** (the stic twin writes every SDBD by hand). Refusals met while writing the two new twins: none.
- Executed scratch results, not committed: (1) the listing checker (`listing.cmake` in dump mode, scratch ROOT, the four generated twin listings registered as cart or boot-stub) accepted all four with no finding, 22 + 56 + 89 + 133 = 300 words, so the independent row, bits, operand-count, SDBD and branch-target recomputation agrees with every word the tool wrote, including the DATA rows of the stic twin (before the checker change the 32 DATA lines were the only findings); (2) a sweep of 2,000 random one to three byte mutations of the four twins through the ASan and UBSan dev binary gave 873 accepted and 1,127 refused sources, no crash, no sanitizer report, every refusal exactly one `ivs_asm:` line, no output or `.tmp` file left; (3) gcc 13.3 (Ubuntu 24.04, amd64) compiled `asm.c` with the dev flags and sanitizers, at -O2 and at -O3, with `-Werror`.
- Suite `asm` alone (45 cases, three runs each, laptop load 10 to 13): dev 2.90 to 2.98 s wall, release 0.39 to 0.43 s wall (07-03: 4 cases, 0.63 s and 0.05 s). The dev figure is above the 1.5 to 3 cpu-s estimate of S3-9 for the whole suite on a loaded machine; the hosted effect on `r` is not measured (no `gh`); the S3-9 reversal (batch the negatives into one process) applies if the first hosted row after this plan raises `r` by more than 0.2.
- Gate times: `sh tools/gate.sh` 53 s, `dash tools/gate.sh` 58 s, container 236 s (laptop loaded).

## Deviations from Plan

**1. [Rule 1 - Bug] A gcc-only compile error in code of Task 2.** The pairs result line built its hex digits with an `int` shift count (`(words[k] >> d) & 15u`); clang and both local gates accepted it, gcc 13.3 under `-Werror=sign-conversion` refused it, so the first container gate failed at `c-build` (steps 31 of 36) with that one error. Commits `969c516` and `55bb7bd` are therefore red under gcc and green under clang; the fix is `b05a222` (unsigned arithmetic, two arrays initialised for gcc's maybe-uninitialized analysis), which also compiles clean at -O1 with sanitizers, -O2 and -O3 under gcc 13.3 in a separate compile run. No push happened in between, and the container gate of the final tree passes. Cause: the PLAN-LESSONS 11 class (hosted-only), found by the step the rules require.

**2. [Interpretation] `asm-imm-range` reachable by widening the evaluator.** The plan pins the token and its 07-03 premise ("a value above FFFF is `asm-expr` today") makes it unreachable. Partial results of an expression now range over 0 to 16777215 and each use limits its own value, so `SDBD` then `MVII #$FFFF+1, R0` is `asm-imm-range`; the same widening needs `asm-org` for an ORG above FFFF and `asm-branch-range` for a target above FFFF, both added. No 07-03 test or twin changed. The alternative (dropping the token from the inventory with a reason) was not taken.

**2a. [Interpretation] Leaf rows with a spelling.** `MVOI` has no spelling in the repository dialect (its operand word is the cell it overwrites), so the table covers 73 of 74 rows. The plan text says "every leaf row ... that has a spelling in the repository dialect"; the vector files do spell `MVOI Rn, $N`, but giving it a defined operand word would be an invention.

**3. [Interpretation] Header limit removed.** With the header keys limited to the eight of the listing grammar and each at most once (needed for the plan's "header key outside the listing grammar's set" case), a header has at most 8 lines, so the 32-line `asm-limit` check of 07-03 became dead code and was removed; `asm-limit` is still reachable by source bytes, lines, line length, words and symbols, and the pinned case is the over-long line.

**4. [Interpretation] Corpus content.** The plan (from S3-8) counted 317 pairs and said "a one-time copy ... whose program words are all at most 03FF"; the copy has 301 (the count of S3-8 included lines the dialect refuses by design) plus three hand cases, and the SDBD immediate pairs, which the filter would drop for filler in the upper bits of the operand words, are kept with those words masked to their low byte (pdf 58 marks the upper bits Not Used). The header records both rules. All six SDBD immediate forms are therefore covered by vector pairs that a plain copy would have lost.

**5. [Design choice] More cases than the plan listed.** `asm.judge.mutated-pair` (the pairs comparison shown able to fail), `asm.neg-coverage`, seven `asm.var.<slug>` and `asm.boundary.branch-03ff` are added to the 4 twin, 9 pairs and 20 negative ids; the verify command's equality (pinned tokens equal `asm.neg.` ids, 20 = 20) holds, and `tests-expected` moved 4440 to 4522 (41 ids in two presets).

**6. [Design choice] Further refusals not in the plan.** `MVO@` source equal to the pointer 4 to 7, a JSR register outside R4 to R6, a shift count outside 1 and 2, a `BEXT` code or an `NOP` operand out of range, ORG and targets above FFFF; each maps to an already pinned token (`asm-register`, `asm-operand-kind`, `asm-org`, `asm-branch-range`).

**7. [Interpretation] CR and long line at run time (A-22).** The plan's list gives "a CR line ending" and "an over-long line" as negative cases; both are built by `tests/asm-tap.sh` at run time (`printf` and `awk`), so `tests/asm/neg` has 18 files for 20 tokens and `asm.neg-coverage` fails if a file duplicates a run-time case.

**8. [Process] No committed checker run over the generated twin listings.** Plan text D-18 (2) says the row check polices every generated listing; no generated listing is registered in `listings.ids` before plan 07-05, so the run over the four twin listings is a scratch result (above), not a suite case: a committed one costs four `cmake -P` runs per preset, about as much as the rest of the suite. Plan 07-05 registers the first generated listing, after which the `listing` step covers it every run.

No Rule 2 or Rule 3 fixes; no auth gates; no package installs (T-07-SC).

## Decisions Made

- Keep the shape-driven table: all new rows are data of the same switch (18 kinds); no per-row code except the spelling refusals.
- A refusal is judged by token, exit status, one stderr line and no leftover file; the inventory of tokens is the set `asm.c` can print, so a new refusal cannot be added without its case.
- The hand cases of the corpus (XORI, SDBD ANDI above 03FF, NOPP) stay as the plan asked even though the masked vector pairs also cover SDBD ANDI.
- No requirement is closed: SCN-01 needs the program, scenarios and consumer replay (07-06 to 07-09) and stays Pending.

## Open Items

- Plan 07-05 inputs: `ISR EQU $5004`, `JD ISR` at `$1004`, regen cases `asm.regen-<id>` for tracked `fixtures/*/*.asm`, and a manifest `tool_sha256` equal to the digest of `tools/asm/asm.c` (so `asm.c` should not change after that plan starts without a new digest). The first generated listing registered there brings the `listing` step's row check to the tool's real output.
- Line budget: 1,137 of 1,200 (63 lines). Levers: pack table entries three per line, trim the table comments. A request for macros or includes stops the tool (D-16).
- The limits other than line length (131,072 bytes, 4,096 lines, 4,096 words, 256 symbols) have no committed case; the 2,000-mutation scratch sweep and `asm.neg.asm-limit` cover the refusal path, not each limit.
- Suite cost: 45 cases, dev about 3 s wall on a loaded laptop; measure at the first hosted row and the cost checkpoint after the scenario wave; the lever is one process for all negatives.
- `MVOI` has no spelling; add one only with a defined operand-word rule.
- Carried unchanged: Phase 4 and 5 review deferrals, `sh tools/hygiene.sh --history` on earlier planning commits, the orchestrator-owned hosted CI record (this executor ran no `gh`).

## Final gate

```
sh tools/gate.sh:     gate: PASS steps=36/36 controls=352/352 faults=222/222 ... listings=4 listing-words=300 discrepancies=106 ... tests-expected=4522 tests-executed=4520 tests-skipped=2 tests-missing=0 ... (0 FAIL lines; gate-time total=53)
dash tools/gate.sh:   the identical summary line (same md5 54bce532514b072c3d377a760bb8f133; 0 FAIL lines; gate-time total=58)
sh tools/gate-linux.sh: exit 0, build/gate-linux.log ends gate: PASS (x86_64, gcc 13.3.0, cmake 3.28.3), steps=36/36 controls=352/352 faults=222/222 listings=4 listing-words=300 discrepancies=106 tests-expected=4522 tests-executed=4520 tests-skipped=2 tests-missing=0, 0 FAIL lines (gate-time total=236)
```

The first container run of this plan (tree of `55bb7bd` plus the Task 4 edits) failed at `c-build` with the gcc error of deviation 1 (`gate: FAIL steps=31/36`); the run above is the tree after `b05a222`.

`git diff --quiet 903276757b0dbd901b15dbc00b9813089c1b578e -- fixtures data/golden data/scenarios` exits 0 (hand listings, manifests, goldens and scenarios untouched), and `git diff --quiet 1621dae -- docs/video` exits 0 (the frozen oracle is untouched).

## Known Stubs

None. `tests/asm/neg` sources and the pairs corpus are test data; no file in this plan holds a placeholder that reaches a UI.

## Threat Flags

None new. T-07-08 is mitigated as planned (DATA refused while operands are pending and after SDBD by the existing tokens, bits and range still checked, two controls, the pristine toy tree with a valid DATA word). T-07-09 (the corpus header names the source, the date and the rules; the vectors are pinned by `tests-vectors-digest`; the twins and the listing step stay independent of the corpus). T-07-10 (one pinned case per token including the over-long line and the CR, no output file on refusal, the 2,000-mutation sanitizer sweep with no crash). T-07-SC: no installs. The tool still writes only one file beside its target and renames it.

## Self-Check: PASSED

The files named under `provides` exist (`tools/asm/asm.c`, `tests/asm/pairs.txt`, the two new twins, `data/expected/asm-tokens.ids`, 18 files under `tests/asm/neg`, `tests/asm/pos/branch-03ff.asm`, the two control directories); commits `e4f7dab`, `969c516`, `55bb7bd`, `b05a222` and `0687583` are ancestors of HEAD (`git rev-list --count 214e6b36d4b488ecf8b097103e9d41e3b2ea1a9b..HEAD` printed 5 before this file's commit); `grep -v '^#' data/expected/asm-tokens.ids | grep -c .` prints 20, equal to `grep -c '^asm[.]neg[.]' data/expected/tests/asm.ids`; `grep -c '^asm.twin-' data/expected/tests/asm.ids` prints 4; `grep -v '^#' tests/asm/pairs.txt | grep -c .` prints 304; `grep -c '^C106' data/expected/discrepancies.ids` prints 1; `wc -l tools/asm/asm.c` prints 1137.
