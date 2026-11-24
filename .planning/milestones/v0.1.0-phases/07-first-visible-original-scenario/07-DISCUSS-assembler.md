# Phase 7 discuss: S-3, D4 assembler re-evaluation (research agent C)

Scope: decision point S-3 of the Phase 7 discuss (sub-decisions S3-1 to S3-11). Written for the orchestrator's synthesis; nothing here is an owner decision and nothing in the repository other than this file was changed.
Labels: [OBS] read, measured or computed this session; [INF] inference; [REC] recommendation; [UNK] unknown; [BG] background knowledge not re-fetched.
Method: the repository itself first (AGENTS.md, AUTONOMY.md, PROJECT.md D4 rows and notes, `docs/diagnostics.md`, the three diagnostics and the boot stub, `tools/check/listing.cmake`, `tools/pack/pack.c`, `tests/pack-tap.sh`, `data/cp1610/`, `docs/cp1610/reference-notes.md`, the Phase 2 vectors, the gate and CI-EFFICIENCY records, fixture-manifest rules), then lenses (CPU/emulator architect, hardware/EE, QA, security and rights, devops/CI, integrator, product, DX) and an adversarial pass (S3-2 and S3-11). No third-party assembler source was read; no hardware claim is made.

## 0. Recommendation on one screen

1. **The D4 trigger fires on three independent counts.** Phase 7's program is the fourth project-authored program (D4 says "above 3"), it needs labels (an interrupt routine entered from the boot stub, loop heads, a table address used as an immediate) and it needs a data directive (the Phase 6 finding). The 06-13 crossing rule (more than 170 words or more than 8 branches) very likely fires too, but that depends on S-1's size [INF]. No second author exists. [S3-1]
2. **Build a project mini-assembler, `ivs_asm`, as a listing generator, not a new source of truth for images.** Input: a project-syntax `.asm`. Output: the existing committed listing grammar (`ADDR HEX4 BITS ROW ASM`) plus one new row token `DATA`. The listing stays committed, checked by the existing `listing` step and `pack.words-*` suite, and consumed by an unchanged `ivs_pack`. [S3-2]
3. **C17, one file, `tools/asm/asm.c`, built beside `ivs_pack`, never installed or shipped.** Not sh/awk (no portable hex or bit arithmetic), not CMake script (the checker alone is 676 lines). Registration follows the `tools/pcm` precedent. Budget 700 to 900 lines, stop at 1,200. [S3-3]
4. **Encoding source: a hand-written table inside `asm.c` from `docs/cp1610/reference-notes.md` and the manual pages it cites, policed from outside by three independent checks:** the listing checker's decle-to-leaf-row check against the second-read `data/cp1610/opcode-families.json`, a copied corpus of 317 hand-derived, blind-second-read asm/word pairs from the Phase 2 vectors, and byte-for-byte reproduction of the 300 hand-typed words of the existing listings. No GPL input, nothing copied from another assembler or emulator. [S3-4]
5. **Dialect = the repository's own** (`MVII #$0006, R0`, `MVI $01FE, R0`, `MVO@ R0, R5`, `BZE $500C`) plus labels, `ORG`, `EQU`, `DECLE`, `+` and `-`. No macros, includes, conditionals, linker, auto-anything. [S3-5]
6. **Explicit SDBD, canonical spellings, strict ranges.** No auto-SDBD (sizes then depend on syntax only: no size fixpoint). Immediate above `03FF` without a prefix, direct address above `03FF`, displacement above `03FF`, a spelling whose leaf row differs from its mnemonic, and a gap between segments are named errors. Branch displacement: forward `target - (pc + 2)`, backward `(pc + 1) - target`, the formula the checker already uses. [S3-6]
7. **Two committed files per assembled fixture:** `<name>.asm` (source of truth) and `<name>.listing.txt` (generated, header says so), both in the manifest `outputs`, recipe filled in. A test regenerates each listing and requires byte equality. Run goldens do not depend on the assembler. [S3-7]
8. **Migration: leave the boot stub contract 1 and the three diagnostics untouched** (hand listing stays source of truth; hashes, manifests, mutants, goldens unchanged). Write `.asm` twins as test data; `ivs_asm` must reproduce their words, the best independent corpus it will ever have. [S3-10]
9. **Tests, new suite `asm`:** pairs corpus, four twins, regen equality, determinism, a judge self-control, about 22 negative sources pinned to tokens in `data/expected/asm-tokens.ids`; three new pure controls for `DATA`. [S3-8]
10. **Gate and CI: no new step, job or dependency.** The suite rides `c-tests`; estimated 1.5 to 3 cpu-s against a hosted median of 315 to 335 (about 1 percent). No new JSON parse (2.25 s user measured for the existing vector generator): the corpus is a one-time flat copy. [S3-9]
11. **Sequence it first in Phase 7** (after the queued CI lever check), tracer first, tests first; it runs beside the program's derivation. [S3-11]
12. **Reverse on evidence:** two run-found encoding escapes, a source above 1,200 lines or any wish for macros, or a scenario program under about 150 words with under 6 branches and a trivial interrupt routine (then a `DATA` row alone is right and the assembler waits for Phase 8). [S3-11]

## S3-1. Does the D4 trigger fire for a fourth program with an interrupt routine, labels and data?

**Facts.**
- [OBS] D4 (PROJECT.md): "A diagnostic count above 3, labels/macros, or a second author: build a project mini-assembler." Phase 4 D-25 and `docs/diagnostics.md` repeat it: "more than 3 project diagnostics exist, a listing needs labels or macros, or a second author writes listings", and "Phase 7 re-examines the trigger: a fourth diagnostic, or any listing that needs labels or macros, is the point to weigh the project assembler".
- [OBS] The 06-13 crossing rule (derivation of `ivs-diag-stic-1`, recorded in 06-13-SUMMARY and the diagnostics doc): any one of more than 170 words or more than 8 branches, a hand-assembly error that survives the checker and is found only by a run, a second STIC cartridge, a second author. None fired for the third diagnostic (133 words, 5 listed branches).
- [OBS] Effort data of the three diagnostics plus stub: 22 + 56 + 89 + 133 = 300 words; listings of 4,828, 7,044, 10,992 and 14,629 bytes; derivation documents of 146 lines (ctrl-psg) and 126 lines (stic); zero errors caught by the listing checker on the third listing, two slips caught by hand first (an opcode constant, `03F8` for CMPI where the family table says XORI, and an annotation punctuation slip), zero escapes found by a run. The second listing's one correction was semantic (auto-increment of R1 to R3), not an encoding slip.
- [OBS] Phase 6 hand-offs to Phase 7: a data directive for listings, a project interrupt routine, the D4 re-evaluation, a reader cartridge for the D10-VID items.
- [OBS] Phase 8 success criterion 1 needs a single-feature fixture each for MOBs, border, horizontal and vertical delay and collision registers (or an explicit unknown entry): up to about five more programs soon after Phase 7. [INF] The D10-VID reader cartridge is another.

**Evaluation against the trigger.**
- Count above 3: yes if a scenario program counts as a diagnostic (D-25 says "project diagnostics"); the stub is not counted and a scenario program is not a test-suite stand-in.
- Labels: yes. The stub's interrupt entry must reach the program's service routine, and a table address immediate is today a hand-computed constant that moves with every edit above it.
- Data directive: yes (a row label per decle, one accidental three-decle `J` in the third listing).
- Crossing rule size or branch count: likely [INF]. An interrupt routine (register save, strobe, input read, PSG writes, restore, return), a main loop, input decode and tables is 200 to 350 words and 10 to 20 branches; depends on S-1.
- Escape found only by a run: not yet. Second author: no (all sessions are "project contributors"; 06-13 treated this as unfired).

**Recommendation [REC].** The trigger is met by its letter on at least three counts. Do not re-argue the rule; decide the shape (S3-2). The rule's purpose (hand assembly is cheap while programs are tiny and unlabeled) no longer holds for a program that is iterated for an audible, visible result, referenced from a second fixture and followed by a run of single-feature fixtures.

**Reversal evidence.** The scenario program (S-1) is small and static enough that none of: ISR, labels, tables, more than 150 words, more than 6 branches is true. Then only the `DATA` row is needed (S3-2 option O1).

**Risks.** The "diagnostic" wording could be read as excluding a scenario program; the answer is that labels and data independently fire, so the reading does not matter.

## S3-2. Option space: extend the checker, or a project mini-assembler

| Option | What it is | Cost [INF] | What it removes | What it leaves |
|---|---|---|---|---|
| O1 | Status quo plus a `DATA` row in `listing.cmake` | 0.5 h, about 25 lines of CMake, 2 to 3 controls | the data-as-program wart | every address, displacement, SDBD split and BITS column typed by hand |
| O2 | O1 plus labels in the checker (`# label: name ADDR` lines, branch lines name the label, checker resolves) | 1.5 h, about 60 more lines | unreadable numeric targets | all hand arithmetic; checker still only validates what a human typed |
| O3 | **`ivs_asm`: `.asm` to existing listing grammar, listing committed and checked** | 3 to 5 h executor time, 700 to 900 lines C plus tests [INF from 06-13's 3 h and pack.c's 507 lines] | address churn, displacement arithmetic, SDBD splits, BITS and ROW columns, absolute-address constants | the author still writes the program and derives expected values by hand |
| O4 | O3 but `ivs_pack` reads `.asm` directly and the listing disappears | O3 plus a rewrite of `pack.c` and `listing.cmake` | one artifact fewer | loses the audit artifact (words, bits, rows) and the independent checker; rejected |
| O5 | Adopt a third-party assembler (`libasm`, Apache-2.0, has a CP1600 target) as a gating build step | glue for the 10-bit packaging, a pinned C++ build in CI | encoding work | a new CI dependency D4 forbids and the AGENTS.md dependency rule discourages; 10-bit packaging and syntax unverified [UNK]; `libasm` is not installed here [OBS: not on PATH] |
| O6 | `as1600` | n/a | | GPL-2.0-or-later and SDK `.asm` includes; rejected by D-25 |
| O7 | Compile from IntyBASIC or C | n/a | | hides instruction behaviour, adds runtime code; rejected in the bootstrap research |

**Why O3 and not O1/O2.** [INF] The hand cost is the edit cycle of a program that will be tuned, not the first 133 words. An inserted instruction shifts every later address, every crossing branch (displacement and `-> ADDR` note), every absolute immediate naming code or data (`MVII #$5065, R4`; in Phase 7 the stub's jump to the interrupt routine), the address-keyed mutant inventory and the derivation's references. The checker detects these slips (hence zero escapes) but each costs a fix cycle, and zero escapes at 133 words does not extrapolate to 300 words with a shared stub. Both slips of the third listing (opcode constant, annotation syntax) are kinds an assembler removes by construction, and [OBS] its BITS column came from a scratch formatter, so `listing-bits-mismatch` was never exercised honestly.

**Why O3 and not O4.** The generated listing is what a reviewer and the independent checker read, and it keeps three mechanisms alive free: `listing.cmake` re-derives rows from the JSON, `ivs_pack --dump-words` and `listing.cmake -DDUMP` parse the same file twice, `pack.trailer-*` checks the seal. O4 deletes evidence the project already pays for.

**Why not O5.** "Another copy-paste is better than another dependency"; D-25 forbids an assembler as build or CI dependency; a C++ build plus a decle converter costs more than 700 to 900 lines. [REC] Keep `libasm` as an optional non-gating cross-check (S3-8 T8).

**Adversarial pass against O3.**
- *"Defer to Phase 8; do O1 now."* The strongest counter. Rebuttal: the assembler's value is front-loaded on the flagship program (iterated, shipped in releases, referenced by the stub), its best oracle (300 frozen hand-typed words) exists now, and building it first makes the Phase 7 program and the Phase 8 fixtures cheaper. If S-1 picks a tiny program the counter wins (S3-1 reversal).
- *"A same-author assembler shares the core's misreadings."* True, and the project already says so for every diagnostic. The assembler is never used to produce expected values, only words; its table is checked by independent sources (S3-4) and a wrong encoding that matters changes program behaviour against hand-derived predicates, which is a visible failure. It does not enlarge the shared-misreading surface beyond what hand assembly had (the hand assembler was the same author reading the same pages).
- *"Two files per fixture will drift."* Regen equality in `asm.regen-<id>` makes drift a test failure in both presets.
- *"Scope creep."* Hard non-goals (S3-5) and a line budget with a stop (S3-11).
- *"CI budget is tight (`r` median 8.02 against a trigger of 8.3)."* About 1 percent added, no new job (S3-9).
- *"Derivation-before-listing is weakened."* Not if the order holds: the program derivation (expected state, cycle margins) precedes the `.asm`; only the encoding lines vanish, and the corpus checks the table.

**Reversal evidence.** See S3-11.

## S3-3. Language, location and build registration

Alternatives: sh/awk (no portable hex parse or bit ops in POSIX awk, BWK awk on macOS against mawk on the hosted image, dash quirks), CMake script (slow and unreadable at this size: the checker alone is 676 lines), another language (a new toolchain, against D1). C17 matches `ivs_pack` (`ivs_warnings`, `ivs_san`: ASan and UBSan in the dev preset) and has native bit operations; its costs are UB discipline (the project's integer rules) and the hosted gcc/dash lessons (PLAN-LESSONS 11).

[REC] C17, one translation unit, copy-paste helpers from `pack.c` (hex digit, tokenizer) instead of a shared library. [OBS] `pack.c` compiles in 0.14 s user at -O2 on this laptop, so an 800-line tool adds about 0.25 s user per preset.

Registration (each a precedent in the tree [OBS]): `tools/asm/CMakeLists.txt` modelled on `tools/pack` and `tools/pcm`; `add_subdirectory(tools/asm)` in the root `CMakeLists.txt`; `tools/asm/asm.c` listed in a generated `ivs-asm-sources.txt`; one entry in the `mod` loop of `tools/check/steps/c-build.sh` (`'^tools/asm/.*[.]c$ ivs-asm-sources.txt'`) with a control like `c-build-unlisted-pcm-source`; never installed (the `pkg-install` step already asserts no tool enters the package); `// SPDX-License-Identifier: MIT` header. The tool reads one input file and writes only the one output path it is given, no time, no path or host-dependent byte in the output, no includes, so it keeps the packer's determinism contract.

**Reversal evidence.** The hosted gcc 13 / dash leg produces a recurring tool-specific failure that C cannot avoid cheaply; unlikely.

## S3-4. Encoding source of truth (project-owned notes, no GPL input)

**What exists [OBS].** `docs/cp1610/reference-notes.md` (every statement cited to a manual page); `data/cp1610/opcode-families.json` (74 rows, page-image transcription, second-read: 71 agreed, 3 resolved, no table change); the Phase 2 vectors (412 with an `asm` string and a `program`; blind second reading agreed on 100 percent of control 27/27, register 62/62, immediate 24/24, shift 43/43, branch 98/98, jump 11/11; the disagreements were direct-mode bus labels, SDBD cycle totals and interrupt cases, not encodings). The `asm` strings are project-authored and the file note says none comes from an assembler. The manual says SDBD is "normally supplied by the assembler".

**Options.**
- E1: generate the table from `opcode-families.json` at build time. One source, but operand fields are only letters in a `printed` string, syntax classes and aliases are not structured, the row check of the listing step becomes tautological for the table, and a JSON parse costs build time (2.25 s user measured for the existing generator) against the queued lever to batch JSON re-parsing.
- **E2 [REC]: a hand-written table in `asm.c`**, about 80 spellings each with a class (implied, one register, two registers, shift register plus count, direct, immediate, indirect, branch, branch external, jump family, status), a base decle and a leaf row id. Each entry cites its manual page in a comment, as the existing listing headers do. It is independent of the JSON, and the JSON polices it from outside.
- E3: a third data file of syntax: a fourth copy, rejected.

**The three outside checks (they are why E2 is safe) [INF design, each mechanism OBS in the repo].**
1. *Row check on every generated line.* `ivs_asm` writes the row id its mnemonic names; `listing.cmake` independently recomputes the leaf row of each decle from the JSON and fails `listing-row-mismatch` on any difference. That catches a wrong base opcode (the class of the one hand slip of the third listing), a wrong group or a swapped pair, for every assembled word in every fixture, forever, at no new cost. It cannot see operand fields (the checker never parses the ASM text, [OBS] header of `listing.cmake`).
2. *Corpus of hand-derived pairs* (S3-8 T1) for the operand fields: register numbers, MMM modes, shift N bits, branch direction and condition, JSR register choice, BEXT codes.
3. *Reproduction of the 300 hand-typed words* of the four legacy listings (T2).

**Canonical spelling rule.** Where one decle belongs to two mnemonics with a leaf refinement ([OBS] `TSTR` refines `MOVR` with `JR` also encoded; `CLRR` refines `XORR`; `PSHR` refines `MVO@`; `PULR` and `MVII` refine `MVI@`), the assembler refuses the generic spelling that would land on the refined row (`MOVR R1,R1`, `MOVR R2,R7`, `XORR R3,R3`, `MVO@ R2,R6`, `MVI@ R6,R1`, mode 7 spelled as indirect) with token `asm-spelling` and the message names the canonical spelling. This keeps the emitted ROW honest without teaching the assembler the whole refinement graph. Pairs mode (S3-8) writes words only and does not apply this rule.

**Rights [REC].** Author `asm.c` from the reference notes, the family table and the cited manual pages only; the plan states in its first task that no assembler or emulator source was opened; the file header says so. Mnemonics and encodings are facts of the GI manual (tier 1, rights-bearing document not redistributed); dialect names come from the repository's own listings. The vector `asm` strings are project-authored, so copying the pairs is a project-internal copy, not third-party input. AGENTS.md "no code, table or comment of another implementation" stays true; the denylist step (emulator project names in text) applies to `.asm` sources and the tool.

**Reversal evidence.** A wrong table row found by a run and not by T1, T2 or the row check (a genuine escape) means the corpus must grow first (add the case), and two such escapes mean generating the table from the JSON after all.

## S3-5. Source language: dialect, labels, directives, expressions, non-goals

**Dialect [REC]: the repository's.** [OBS] The vector corpus and the listings already share one operand grammar: `Rn`; `#expr` immediate; `$expr` or a bare expression as a direct address; `MVI@ Rn, Rd` and `MVO@ Rs, Rn` for indirect (the `@` is part of the mnemonic, so there is no operand-level indirect syntax); `SLL R1, 2` and `SLL R1` (count one); `JSR R5, target`; `BEXT target, code`; `NOP 0`, `SIN 1`. Hex is `$` plus digits, decimal digits plain. 79 spellings occur in the clean vector subset. Reusing this grammar means the corpus defines the syntax and the generated ASM column re-parses.

**Statements.**
```
; comment to end of line
label:  MNEMONIC operands        ; label is optional, one statement per line
        ORG   $5000              ; sets the word address; forward only within a segment (S3-6)
NAME    EQU   $0340              ; or: EQU NAME, expr (one form only; pick one in the plan)
tab:    DECLE $00FF, $0000, 3    ; 10-bit data words, each at most $03FF
```
- Header: the leading `# key: value` block of the source (`id`, `kind`, `profile`, `window`, `label`, `entry`, `stack`, plus `contract` for a stub) is copied to the listing unchanged, because the listing grammar and the checker already own those keys. The assembler also checks `window` against the actual extent (an error `asm-window` if it is exceeded). No new header keys, so `HEADER_KEYS` in `listing.cmake` is untouched.
- Labels: `[A-Za-z_][A-Za-z0-9_]*`, at most 31 characters, case-sensitive, global, defined once, reserved words (mnemonics, `R0` to `R7`, directives) refused (`asm-symbol-reserved`). No local labels in v1.
- Expressions: terms are number, symbol or `*`-free; operators `+` and `-` only, left to right, 16-bit unsigned results, no parentheses. [INF] This is enough for `table_end - table`, `tab + 2`, `base + 8`. If the program wants `20 * 12`, precompute it in a comment (the derivation already holds such arithmetic).
- Immediates: `#expr`. A value above `$03FF` requires the explicit `SDBD` line above the instruction (S3-6).
- Branch and jump targets: a label or an absolute expression; the assembler computes displacements.
- Comments are not copied into the listing line by line; the listing carries a canonical ASM text plus the label name as a note (S3-7). Long narrative stays in the `.asm` and the derivation document.

**Explicit non-goals (a written list, enforced by review and the line budget).** Macros, `INCLUDE`, conditional assembly, multiple segments per source beyond forward `ORG` zero-fill, relocatable output, a linker, local labels, expression precedence or parentheses, string directives, listing-file options, auto-SDBD, auto-spelling of aliases, cycle counting, symbol export to other files. Cross-fixture addresses (the stub reaching the program's interrupt routine) are plain `EQU` constants repeated in both sources and tied by a header or manifest check chosen in S-2 (see Open items).

**Pros/cons.** Reviewers already read this grammar and the corpus doubles as the grammar test; the cost is repetitive `SDBD; MVII; MVO` triples (six in the third diagnostic), acceptable because explicitness is the project's value.

**Reversal evidence.** A program whose size is dominated by a repeated idiom that a three-line macro would remove, with review evidence that the repetition hid an error. Then add one fixed-form pseudo-instruction (for example `SETW addr, value` expanding to the triple) as a named, tested builtin, not a macro system.

## S3-6. SDBD, branch offsets, relative branches and the other encoding hazards

**SDBD [OBS rules in the repo, REC handling].**
- `SDBD` is a mnemonic on its own line (one word). It applies to the next instruction only. Targets: the six immediate rows (`MVII ADDI SUBI CMPI ANDI XORI`) and the six indirect read rows (`MVI@ ADD@ SUB@ CMP@ AND@ XOR@`) with registers R1 to R5 as the indirect register (modes 1 to 5, the set `listing.cmake` and the core accept; modes 0 and 6 are unsupported per the manual, [OBS] reference notes). Anything else after `SDBD` is `asm-sdbd-target`, which mirrors the checker's `listing-sdbd` token so the two refusals agree.
- An immediate form after `SDBD` takes a 16-bit value and emits two operand words: low byte then high byte, each at most `00FF` (matches the third listing: `MVII #$5065, R4` gives `0065`, `0050`). A value above `$FFFF` is `asm-imm-range`.
- **No auto-SDBD [REC].** If the assembler inserted the prefix when the value exceeds `$03FF`, the size of an instruction would depend on an expression that may name a forward label, which turns assembly into a size fixpoint problem (a known classic for variable-length assemblers). Explicit prefix means sizes depend only on syntax, so pass 1 assigns all addresses and pass 2 resolves values. A forward label that resolves above `$03FF` in an immediate with no prefix is caught in pass 2 as `asm-imm-sdbd` ("needs SDBD"). The cost is that a source line count shows the prefix, which is what the listing shows too.

**Direct address.** [OBS] The direct mode takes one operand decle (the third listing states "every operand address below 0400, so each address word is one decle"). `MVI $5000, R0` is not encodable in a 10-bit ROM: `asm-direct-range`. Large addresses go through `SDBD; MVII #addr, Rn` then an indirect access, as the third diagnostic does for the graphics RAM.

**Conditional branches and `B`.**
- Encoding [OBS: reference notes, `docs/diagnostics.md`, vector `B $1001` at `$1000` gives `0220 0000`]: first decle `1000 S 0 cccc` (or `1000 S 1 EEEE` for `BEXT`), second the displacement decle. Forward (`target >= pc + 2`): `S = 0`, `d = target - (pc + 2)`. Backward (`target <= pc + 1`): `S = 1`, `d = (pc + 1) - target`. So `target = pc + 2` is forward with `d = 0`, `target = pc + 1` is backward with `d = 0`, `target = pc` is backward with `d = 1` (a branch to itself). This is the formula the checker uses for `-> ADDR`, so the assembler's output is checked by an implementation that did not write it.
- Range: the displacement is one decle, so `d <= $03FF`, otherwise `asm-branch-range`. [INF] The vector note prints "+1025 to -1024" for the manual's range; arithmetic with a 10-bit displacement gives forward `pc + 2 + 1023 = pc + 1025` but backward `pc + 1 - 1023 = pc - 1022`, so the printed backward limit is not reproduced by the formula; the assembler follows the decle limit and the discrepancy is a note for the log, not an assembler matter. [UNK] whether a wider displacement exists on 16-bit memory; irrelevant to 10-bit cartridge ROM.
- No wraparound: the assembler refuses a target outside the 16-bit address space and never relies on the core's modulo-65536 rule (C50). A branch whose displacement word would be `SDBD`-prefixed is refused (the core faults on that, [OBS] checker rule).
- `NOPP` and the 16 conditions take the same form; `BEXT target, code` fills the low four bits and bit 4. Spellings with aliases (`BEQ` for `BZE` in the family table) are not accepted unless the corpus uses them [INF].

**Jumps.** `J target`, `JE`, `JD`, `JSR Rr, target`, `JSRE`, `JSRD` with `Rr` in R4 to R6; three decles: `0004`, then `BB AAAAAA EE`, then the low ten address bits ([OBS] the boot stub's `J $5000` gives `0004 0350 0000`). `JR Rs` is a single word. The hand-off and the interrupt routine entry use these.

**Data and fill.** `DECLE` values above `$03FF` are `asm-decle-range` (other assemblers truncate silently; the listing rule is stricter). The listing requires contiguous addresses, so `ORG` may only move forward and the gap is emitted as explicit zero words with row `HLT` and text `fill`, exactly as the boot stub does ([OBS] zero words decode as `HLT`, which is not a trap, docs "Endings need a follower"). A backward `ORG`, an overlap or a second window is `asm-org`. [REC] Carts are one contiguous segment.

**Endings and hygiene.** The assembler inserts no followers (an ending lint belongs in the checker, open item 4). ASCII and LF only (CR is `asm-syntax`), line and word limits (`asm-limit`) so the tool is total on hostile input, no `getenv`, `time`, locale or files other than input and output.

**Reversal evidence.** A real need for `SDBD` on a forward-referenced immediate that makes the explicit form error-prone in practice (two or more `asm-imm-sdbd` slips in Phase 7). Then add auto-SDBD with a two-pass fixpoint, test-first.

## S3-7. Listings, manifests and golden digests: staying reviewable

**Generated listing format [REC].** Exactly the existing grammar so nothing downstream changes: header lines copied; one comment line `# generated by ivs_asm from <repo-relative .asm path>; edit the source, not this file`; one comment line per label in source order (`# label: loop 5026`) so authors and mutant inventories can read addresses; then one word line per word.
- Instruction lines: `ADDR HEX4 BITS ROW ASM`, BITS grouped as the row's printed fields (as in the vector `program` strings and the listings: `1010 111 000`; operand words 2+4+4; SDBD byte words as in the third listing), ROW the leaf row, ASM the canonical text (`MVII #$0006, R0`, `BNZE $505A -> 505A`), then the label or note after at least two spaces. The ASM column is chosen so that it re-parses as source (S3-8 T7).
- Branch and `J` lines carry `-> ADDR` as the `listing-branch-target` rule demands.
- Data lines: ROW `DATA`, ASM `DECLE $00FF`. New rule in `listing.cmake` (about 25 lines): a `DATA` word skips leaf-row labelling and branch-target decoding, is refused while operands are pending (`listing-operand-count`) and after an `SDBD` (`listing-sdbd`), must still satisfy `listing-bits-mismatch` and `listing-range`. 2 to 3 controls in `tools/check/controls/` (`listing-data-in-operand-slot`, `listing-data-after-sdbd`, optionally `listing-data-row-unknown`) plus `data/expected/controls/listing.ids` lines; `docs/diagnostics.md` grammar paragraph. `pack.c` ignores ROW, so no change. [INF] The legacy third listing keeps its decoded-as-code data lines unchanged; a mixed regime is fine because the checker accepts both.

**Manifest [OBS rules in `docs/fixture-manifest.md`, REC use].**
- `kind: original-source`, `committed: true`, `path` = the `.asm`, `outputs` = the `.asm` and the listing, both with `sha256` and `size_bytes` (outputs are re-hashed, [OBS] `manifests.cmake` hashes every listed output; the files step refuses any unlisted file inside a fixture directory).
- `recipe` filled (the schema already has the slot, and legacy manifests carry `null`): `tool: ivs_asm`, `tool_version: "1"` (a version string printed by `ivs_asm --version`, bumped when output could change), `tool_sha256` = sha256 of `tools/asm/asm.c` at generation time, `tool_licence: MIT`, `command: "ivs_asm <asm> <listing>"`, `sources[{asm path, sha256}]`. [OBS] only the shape of `tool_sha256` and `sources` is checked; the outputs are. [INF] A stale tool digest is harmless because `asm.regen-<id>` fails if a tool change alters any listing; do not build a "tool digest freshness" rule.
- `licence: MIT`, `copyright`, `redistributable: yes`, `rights_review: author-attested`, `derived_from: []`, `allowed_use: [local-test, ci, repo]` (`release` where E agrees), exactly as the legacy manifests. The note records the order: derivation committed before the `.asm`; no assembler other than the project's; nothing copied from another implementation.
- `ivs_asm` is not a fixture and not shipped (like `ivs_pack`).

**Golden digests.** [OBS] The `.expect` files are hand-derived from arithmetic and printed tables and committed before the first run; `.digests` goldens are self-recorded by `ivs-run --update-golden`; the IVSC identity hash is pinned by the golden. None depends on how the words were produced, so the assembler does not touch the evidence protocol. [REC] For the Phase 7 program the derivation document still precedes the `.asm` (A-04 order, checked by commit order); what the derivation no longer lists are encodings.

**Reviewing a PR.** The diff shows the `.asm` change (intent) and the listing change (exact words, mechanical). The gate guarantees equality; a reviewer who wants to check an encoding reads the listing line against the opcode row, as today.

**Mutants.** [OBS] The mutant inventories name absolute addresses with an explicit `from` word checked against `ivs_pack --dump-words`, so a moved instruction fails loudly. [REC] Keep the format; the `# label:` comment lines make addresses readable; add `label+N` addresses only if two label-driven re-derivations occur (open item 3).

**Reversal evidence.** The listing and the source disagree in review often enough that two files confuse authors: then make the listing a build-tree artifact (O4) and keep only the source committed, accepting that the static `listing` step moves to a post-build check.

## S3-8. Test strategy and negative controls

New suite `asm` (shell TAP in the shape of `tests/pack-tap.sh`: nonce header, plan, one result per case, `SKIP_RETURN_CODE 77`), ids in `data/expected/tests/asm.ids`, wired next to the `pack` suite in `tests/CMakeLists.txt`.

| ID | Test | What it proves | Independence |
|---|---|---|---|
| T1 `asm.pairs.<class>` | `ivs_asm --pairs tests/asm/pairs.txt`: one process, per-class ids | operand fields and condition codes against hand-derived, blind-second-read pairs | copied from Phase 2 vectors; [OBS] 317 pairs (about 250 unique asm and address combinations) with every word at most `03FF`, covering 77 of the 79 spellings; absent: `ANDI` and `XORI` (their vector values exceed `03FF`; `ANDI` is covered by the listings, add one hand case for `XORI`) |
| T2 `asm.twin-<id>` | assemble each twin under `tests/asm/twins/` and compare words with `ivs_pack --dump-words` of the committed hand listing | 300 hand-typed words incl. SDBD, backward branches, `J`, indirect, shifts | hand-typed by a person reading the manual, checked twice already |
| T3 `asm.regen-<id>` | regenerate each assembled fixture's listing, byte-equal to the committed file | no drift between source and listing; determinism | n/a |
| T4 `asm.listing-<id>` (rides existing machinery) | add the fixture to `data/expected/listings.ids`: the `listing` step and `pack.words-<id>` and `pack.trailer-<id>` cover it | rows, bits, ranges, branch targets recomputed, two parsers agree | existing independent checker |
| T5 `asm.neg.<token>` | about 22 tiny bad sources, each must exit non-zero with exactly its token on one `ivs_asm:` stderr line, no output file | refusal behaviour, totality on hostile input | tokens pinned in `data/expected/asm-tokens.ids`; the suite fails if a token has no case or a case names an unpinned token |
| T6 `asm.judge.mutated-twin` | change one register in a twin source and require the T2 comparison to fail | the comparison can fail (same device as `pack.judge.patched-word`) | n/a |
| T7 `asm.roundtrip` | re-assemble the ASM column of a generated listing and compare words | the canonical text is faithful; the first real check that a listing's text matches its words | stretch; drop if it costs more than half a day |
| T8 optional `libasm` cross-check | when `libasm` is on PATH, assemble the Phase 7 source's twin and compare words | a third, permissively licensed implementation | executed result only, never gating, labelled skipped when absent [OBS: not installed here] |

Token list [INF] for T5: `asm-syntax`, `asm-mnemonic`, `asm-operand-count`, `asm-operand-kind`, `asm-register` (shift on R4, SDBD register range), `asm-symbol-undefined`, `asm-symbol-duplicate`, `asm-symbol-reserved`, `asm-expr`, `asm-imm-range`, `asm-imm-sdbd`, `asm-sdbd-target`, `asm-direct-range`, `asm-branch-range`, `asm-decle-range`, `asm-org`, `asm-window`, `asm-spelling`, `asm-header`, `asm-limit`.

Negative cases to write test-first, one per token, including the boundary pair (displacement `$03FF` accepted, `$0400` refused), a forward label above `03FF` used as an immediate with no `SDBD`, `SDBD` before a branch, `MVO` and `MVI@ R6`, `MOVR R1, R1`, `SLL R4, 1`, a CR line ending and an over-long line.

Additional properties: determinism (twice and in both presets: identical bytes); diagnostics only on stderr; ASan/UBSan clean in the dev preset (free fuzz-lite via T5 and the corpus); optionally a seeded one-character mutation sweep of one twin that must never crash or hang (about 0.2 s).

The existing machinery gives the rest: a wrong word that matters fails the scenario predicates derived independently (`halted`, memory and frame asserts); the program mutants (`ivs_pack --patch`) still work on the generated listing.

**Reversal evidence.** T1 or T2 passing while a run finds an encoding error: add the case, and if it recurs, regenerate the table from the JSON (S3-4 E1) or revisit.

## S3-9. Does it join the gate, and what does it cost in CI?

**Placement [REC].** No new gate step and no new CI job (D12 stands). The suite is a ctest suite (`asm`) in both presets, which `c-tests` already runs and `tests-inventory` already counts (`tests-expected` grows by the ids times two presets). The generated listings are covered by `listing`, `manifests` and `pack`. One new `c-build` source list entry and its control. Three `DATA` controls and one `c-build` control, all pure. [OBS] Gate steps today: 36 and 339 controls (Phase 6 close).

**Cost [INF, inputs OBS].** Compile: `pack.c` (507 lines) takes 0.14 s user at -O2 here, so an 800-line tool is about 0.5 s for both presets. Run: about 30 process starts per preset (4 twins, 4 regen, 22 negatives, 1 pairs run), 0.2 s release and 0.6 s with sanitizers. Inventory: about 45 ids times two presets times 0.0034 cpu-s, 0.3 s. Controls: about 4 times 0.07, 0.3 s. Total 1.5 to 3 cpu-s against a hosted last-five median of 315 to 335: under 1 percent. [OBS] The 06-14 record puts `r = cpu_s / build_s` at a median of 8.02 against a revisit trigger of 8.3 and says Phase 7 plan 1 starts with a trigger check and runs the queued cheap lever (batching the `cp1610-vectors` JSON re-parse, about 5 cpu-s) first if the last-five median is 8.0 or above. [REC] Keep that order; the lever saves more than this work costs. Do not read the vector JSON from the new suite (a third parse); the corpus is a one-time flat copy (`tests/asm/pairs.txt`, about 317 lines, 15 to 25 KB) whose header names its provenance. The vectors are pinned by `tests-vectors-digest`, so a deliberate vector change is a visible diff a reviewer can mirror; a drift check is a possible later addition.

**Beyond tests?** No. The realistic risk is a hosted-only failure (gcc 13, glibc, dash; PLAN-LESSONS 11): warning-clean code, POSIX test script, container gate before push.

**Reversal evidence.** The first hosted row after the assembler plan raises `r` by more than 0.2; investigate before adding anything else (unlikely at this size).

## S3-10. Migration of the existing listings

Keep as hand-assembled source of truth, each with a twin and no manifest, hash or golden change: the boot stub contract 1 (22 words), `FX-diag-cpu-1` (56), `FX-diag-ctrl-psg-1` (89) and `FX-diag-stic-1` (133; its data table becomes `DECLE` in the twin while the legacy listing keeps its decoded-as-code data lines). New: the Phase 7 program, and a boot stub contract 2 if S-2 adds one, as `.asm` fixtures with the S3-7 manifest shape.

**Why not convert now.** It would rewrite four listings and four manifests' output hashes and drop the hand-written derivation comments in the headers, for no behavioural gain, and it destroys the frozen independent evidence (the twin comparison is independent only while the hand listing stays hand-made). The images would be word-identical either way (an image is built from words only), so goldens would not move; that makes a later conversion safe, not necessary.

**Twin discipline.** The twin lives under `tests/asm/twins/` (test data, MIT, no manifest because it is not a fixture; the `files` step restricts only fixture directories and ROM, PDF and image types, [OBS]). A twin whose words differ from its listing fails T2; if a legacy listing ever changes (a stub contract 2 is a new contract number, not an edit), its twin changes in the same commit or is promoted to source of truth at that point (the manifest then gets the S3-7 recipe).

**Stub contract.** [INF] If S-2 changes the stub to jump to the program's interrupt routine, write contract 2 as a new `.asm` fixture (kind `boot-stub`, header `contract: 2`) and keep contract 1 alive for the three diagnostics (their tests assert contract 1's trap). The anti-HLE guards run on the generated listing unchanged (addresses, at most 16 non-zero words, an allowed row list that may need `data/expected/bootstub-rows.ids` extended deliberately).

**Docs to edit [REC].** `docs/diagnostics.md`: replace "When a project assembler becomes worth writing" with the decision and the new grammar (`DATA`, generated-listing header); PROJECT.md D4 row and a Phase 7 note; `docs/packaging.md` mention that `ivs_asm` like `ivs_pack` is never installed.

**Reversal evidence.** T2 finds a legacy listing word that disagrees with its twin: that is a hand slip found by the assembler, a valuable result recorded in the D4 effort data, and the fix goes to whichever is wrong after checking the manual page.

## S3-11. Effort data, sequencing, exit criteria and the evidence that reverses the decision

**Effort inputs [OBS unless marked].**
- Hand cost: 300 words and about 37 KB of listing text hand-typed in Phases 4 to 6; the third listing's plan took about 3 h elapsed with the derivation, scenarios, mutants and installed-consumer replay; the listing itself was a fraction of that.
- Tool cost [INF]: 700 to 900 lines of C (pack.c is 507, the bootstrap research estimated 500 to 800), a 150 to 200 line test script (pack-tap.sh is 187), a 15 to 25 KB pairs file, about 250 lines of twins, 22 tiny negative sources, 25 lines of checker, 3 controls, docs. Executor time 3 to 5 h by analogy with 06-13 and the Phase 5 `tools/pcm` addition (191 lines, one plan task).
- Break-even [INF]: not established by effort data for the Phase 7 program alone; it is roughly neutral for one program of 200 to 350 words, and positive from the next fixture on (Phase 8's single-feature fixtures, the reader cartridge). The case for doing it now is structural (labels and a stub-to-program reference), not arithmetic. State this plainly in the Decision Register note.

**Suggested plan shape (not binding).** (0) the queued CI trigger check and, if the last-five `r` median is 8.0 or above, the cheap lever first; (1) tracer, tests first (PLAN-LESSONS 1: gates satisfiable at the task's own commit): `tools/asm` skeleton, registration, `c-build` control, `asm` suite with T2 for the stub and `diag-cpu-1` twins and T6, only the classes those need; (2) the rest of the set, SDBD, `DATA`/`DECLE`, T1, the other twins, negatives with `asm-tokens.ids`, the `DATA` row and controls; (3) docs and the D4 note; then the program as in 06-13 (derivation, `.asm`, listing, manifest, scenario, goldens). The assembler tasks do not depend on S-1's content.

**Exit criteria and what Phase 7 must record (so the decision is reviewable at Phase 8):** assembler lines (budget 700 to 900, stop 1,200); number of source edits that shifted addresses; number of tool-found hand slips (T2) and run-found escapes (target zero); number of `asm-imm-sdbd` and other token hits during authoring; the twin result; gate and `r` deltas of the first hosted row.

**Reversal evidence (consolidated).** One word-level escape (a run, not T1, T2, the row check or the listing checker, finds an encoding error): fix, add the case, record it in the D4 effort data. Two: rebuild the table from the family JSON (E1) or hand-assemble that fixture with `ivs_asm` as a cross-check only. `asm.c` above 1,200 lines or a request for macros, includes or conditionals: stop and decide whether the program is too large for this approach. S-1 picks a program under about 150 words, under 6 branches, an interrupt routine of 12 words or fewer and no table: choose O1 and carry the assembler to the start of Phase 8, recording that the trigger fired and was overridden by size. The owner picks a third-party assembler and licence path: reopen O5; `ivs_asm` stays valid as twin and corpus checker. A hosted `r` rise above 0.2 attributable to the suite: batch the negatives into one process, then reassess.

**Risks.** Shared misreading (same author for table, corpus, listings, predicates and core) is the main one: mitigated by the three outside checks, by never deriving expected values from the assembler, limited by the same-model-family readers already noted in the second-reader pass; the residual is recorded, not removed. A generated listing could be accepted unread: keep it committed and reviewed. Scope creep: the non-goal list and the line budget. Hosted-only failures (gcc 13, dash): warning-clean build, POSIX script, container gate before push. Cross-fixture constants repeated in two sources can disagree (open item 1). Rights: low, the tool and sources are project-authored MIT and the encoding facts are already held by the repository.

## Assumptions from other decision points

- **A (S-1, program).** The program has a main loop, at least one interrupt routine, at least one table, at least one branch per input class, and lands at 200 to 350 words and 10 to 20 branches [INF]; it lives at the cartridge entry convention (`5000`) in a single segment; it needs 14-bit values (STIC words) so uses `SDBD; MVII; MVO` triples; its RAM markers are plain memory addresses below `0400`. If A picks something much smaller, see S3-1 reversal.
- **B (S-2, runtime).** The boot stub gains a contract 2 whose interrupt entry hands off to the program's service routine at a project-convention address (a plain constant in both sources, like `5000` and `0340` today), or the interrupt routine lives inside the stub (then the stub, already guarded as a hand-off with no services, is the wrong place for a routine; I assume B keeps services out and hands off). Contract 1 stays for the three diagnostics. The stub allowed-row list may grow deliberately. Sustained display is by the program's own interrupt routine (per-frame strobe inside the STIC register window) and the program polls inputs; none of that needs an assembler feature beyond `J`, `JE`/`JD`, `PSHR`/`PULR`, `EIS`/`DIS`, which the table already has.
- **D (S-4, verification).** Scenarios, `.expect` and `.digests` are unchanged in kind; the mutant inventory stays address-keyed with explicit `from` words (S3-7); installed-consumer replay compares against the runner golden as today; D's new CI cost is separate and adds to this one: the two together should be projected against the `r` budget before Phase 7 plans are written.
- **E (S-5, rights and manifest).** The manifest of the assembled fixture follows S3-7 (two outputs, recipe filled); licence MIT per D9 and `LICENSE`; the program name is trademark-safe (no "Intellivision" in a package or product name, AUTONOMY hard stop); a release ships only the project-authored `.asm` and listing, never the tool binary.

## Open items for planning

1. **Cross-fixture address tie (B plus C).** If the stub's interrupt entry jumps to a program address, decide how the gate ties the two numbers: a new optional header key `isr` in the cart listing checked by `listing.cmake` against the stub's `J` target, or a documented convention constant plus one scenario-level assertion. Cheapest: a convention plus a test that fails if the stub's target differs from the program's `entry`-style header value.
2. **Directive spelling.** `EQU` form (`NAME EQU expr` versus `EQU NAME, expr`) and `DECLE` versus another name: pick one; [BG] `DECLE` is common community usage for 10-bit data, the repository already says "decle".
3. **Mutant addresses by label.** Defer unless two label-driven re-derivations occur; if adopted, `runner-process-tap.sh` learns `label+N` from the `# label:` comment lines.
4. **Checker lint for endings.** `HLT` followed by a zero word or by fewer than one interruptible instruction (the `docs/diagnostics.md` footgun); belongs to `listing.cmake`, optional.
5. **ASM-text re-parse check on legacy listings (T7).** A cheap extension: assemble the ASM column of each instruction line of the four hand listings and compare with the typed words. It is the first check that register and operand fields of the ASM text match the words. Skip lines that begin with `data:` (the third listing's pseudo-instruction data rows). Timebox it.
6. **`XORI` hand case** for T1 (no usable vector pair) and one `ANDI` with a value above `03FF` under `SDBD` from the twins.
7. **Branch range note.** Log the manual's printed "+1025 to -1024" against the decle arithmetic as a discrepancy candidate (no hardware claim; C50 neighbourhood).
8. **Manifest recipe fields.** Confirm with E that `tool_version` is the string printed by `ivs_asm --version` and that the `files` step needs no change for a `.asm` extension (classified by content, not listed in `tools/check/text-ext.txt`; [INF] fine, verify in the tracer commit).
9. **Where the corpus copy and `asm-tokens.ids` are registered** (`data/expected/tests/asm.ids`, `data/expected/asm-tokens.ids`, `data/expected/controls/listing.ids` for the `DATA` controls, `tests/CMakeLists.txt` second source list per PLAN-LESSONS 5).
10. **Decision Register wording** for the D4 row (proposed): "Fixture assembler policy: project mini-assembler `ivs_asm` (C, `tools/asm`, never shipped) generates the committed listing from a project-syntax source; listing stays checked by the `listing` step and the pack suite; legacy listings stay hand-assembled and are reproduced by twins; as1600 rejected (GPL), libasm optional non-gating cross-check; reversal: two run-found escapes, a source above 1,200 lines, a request for macros, or a tiny scenario program (Phase 7 D4 re-evaluation, Delegated decision, owner authorization 2026-11-09)."
