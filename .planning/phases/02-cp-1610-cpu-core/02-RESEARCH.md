# Phase 2: CP-1610 CPU Core - Research

**Researched:** 2026-11-09
**Domain:** pure C17 CP-1610 stepper, page-cited vector inventory, named-fault controls, purity gate (CMake/ctest/sh)
**Confidence:** MEDIUM-HIGH (page readings HIGH; hosted-runner/gcc behaviour and four modelling defaults MEDIUM/LOW, listed in the Assumptions Log)

Scope note: design is settled by the 25 decisions in `02-CONTEXT.md`. This file only adds what reduces planning risk: Nyquist validation map, remaining page reads, spot-verification of inferred claims, and sequencing/portability hazards. Page cites are `pdf N` of SRC-gi-cp1600-um-1975 (rendered pages in gitignored `refs/pages/`; extra pages 9-11, 28-31, 33-34, 36-37, 40, 43-45 rendered with `pdftoppm` to scratch this session). Tag `[VERIFIED: pdf N]` = page image opened with `Read` this session.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
D-01 through D-25 in `.planning/phases/02-cp-1610-cpu-core/02-CONTEXT.md` `<decisions>` are locked and incorporated by reference (a second verbatim copy of ~12k words would drift; the planner must read that file first). One-line index: D-01 unit-granular stepper; D-02 12-byte access record + `ivs_bus_fn` + optional log; D-03 32-byte state; D-04 time only as `*_cpu_cycles`; D-05 per-step `lines`; D-06 layout and `ivs_` naming; D-07 UB hygiene; D-08 interrupt policy; D-09 vector supply and reset; D-10 BUSRQ and waits; D-11 SDBD/stack/branch/undefined encodings; D-12 result and sticky faults; D-13 discrepancy and notes work; D-14 JSON vectors via `cmake -P` generator; D-15 independence and blind second reader; D-16 behaviour-class coverage; D-17 exact inventory and TAP; D-18 `#ifdef IVS_FAULT_<ID>` controls; D-19 CMake skeleton; D-20 in-repo harness; D-21 purity by allowlist; D-22 gate steps and budget; D-23 gate hardening; D-24 single ubuntu-24.04 job; D-25 nothing handed to the owner.

### Claude's Discretion (verbatim)
- Exact names of vector files and groups, vector slugs, fault ids, and the order of plans, as long as inventories stay exact.
- Whether ~10 `tests-inventory-*` controls or a few more are needed, and which purity controls to drop (D-21 constraint).
- Struct field order inside the 32 bytes, as long as D-03 holds and the size assert matches.
- Plan split. Suggested order: (1) gate hardening and SPDX/ASCII extension; (2) CMake skeleton, presets, harness, empty core with the inventory gate and its controls; (3) purity step and controls; (4) decode, state, reset and the stepper skeleton with the first vector groups (moves, ALU, flags); (5) remaining vector groups (shifts, branches/jumps, stack, SDBD); (6) interrupt/BUSRQ/halt policies and bus-access vectors; (7) fault controls; (8) blind second-reader pass, discrepancy and notes updates, CI-EFFICIENCY record.

### Deferred Ideas (OUT OF SCOPE, verbatim)
- Per-microcycle or pin-style CPU core, if a hardware trace ever shows mid-instruction acceptance (D-01 reversal).
- Section-based mutable-static check (Phase 3, API-01 isolation).
- MSVC/Windows purity and build, 2-leg CI matrix (Phase 4 or first Apple-only escape).
- MSan, TSan, `-fsanitize=integer`, libFuzzer/AFL++ on Linux or Homebrew LLVM (scheduled job only once a distinct recurring failure appears).
- Project-owned assembler and assembler cross-checks of vectors (D4, Phase 4).
- Gate hardening IN-01, IN-03, IN-05 and the `--tree` control (Phase 1 data checkers).
- Exported-symbols-match-prefix check (low priority).
- Hardware confirmation of timing offsets, BUSRQ release latency and HLT/interrupt behaviour (D10, hard stop; evidence marked missing).
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| CPU-01 | Every opcode: hand-derived register/memory/flag/cycle results with page cite | Per-row cycle/flag pages mapped below (Section 2); flag-semantics page is pdf 45 not 53-58; ambiguity list G1-G8 |
| CPU-02 | Interrupt and BUSRQ acceptance state model | pdf 12, 22, 24, 25, 35, 36 verified; two refinements (BUS_GRANT keeps interruptibility; STPST is an edge) |
| CPU-03 | SDBD behaviour and unsupported-mode policy | pdf 47/49/57/58 re-read; 10-vs-14 resolved by arithmetic; modes 1-3 read count leans to two reads |
| CPU-04 | Stack direction, auto-increment, reset entry | pdf 46/48 (Fig. 3.2.4), 21/27 verified; POL-STK-02 gets page support |
| CPU-05 | Pure stepper with cycle-stamped access list | Bus-sequence figures pdf 27-37 decoded into offsets; ADAR is a distinct access in direct mode (pdf 11, 30) |
| API-05 | No host clock/fs/env/thread/IO in core | Probes: `#pragma GCC poison` ordering trap, `nm -P` output shapes on this host |
| PKG-02 | One local command equals fast PR gate | Baseline gate measured (30 s wall under load 12); hosted image has ninja 1.13.2 |
| RUN-03 | Exact inventory gate | Controls-runner mechanics and `_good/tree` coupling that decide plan atomicity |
</phase_requirements>

## Project Constraints (from CLAUDE.md / AGENTS.md / AUTONOMY.md)

- Prefix `ivs_`; never "intellivision" in package/product/repo names (hard stop). C17 + CMake (D1 delegated). No copying from MAME/jzIntv/other emulators; behaviour agreement with them is not evidence. "Another copy-paste is better than another dependency" (AUTONOMY section 2).
- Rights: no committed manuals/page images/ROMs; `refs/` stays gitignored. A hash is identity, not rights.
- Shift-left verification; CI only for recurring value; measure before changing CI shape; poll CI from the orchestrator only (no `gh` from agents).
- Label observations vs inference; code presence is not a test result. Autonomous operation: nothing in this phase hits a hard stop (D-25).

## Summary

The page images confirm the load-bearing CONTEXT claims: reset is IAB@1 then first BAR@5 (pdf 27), interrupt entry is INTAK@0, DW@2, DWS@3, IAB@5, first BAR@9 (pdf 35), BUSRQ is honoured before INTR (pdf 22/25), NOP/SIN are the GSWD R4-R7 aliases with N as DDD bit 0 (pdf 51/56), and the immediate SDBD figure of 14 equals SDBD 4 plus an instruction that costs 10 (pdf 57/58). No locked decision is flatly contradicted. Six refinements need planner attention because they change a struct field, a vector expectation or a checker rule: (1) `stpst` is an edge-triggered toggle, not a level (pdf 14); (2) after a BUS_GRANT the flow goes to the interrupt test, not to fetch, so `prev_interruptible` must survive a grant (pdf 22, 36); (3) the SDBD modes 1-3 pages lean toward two reads at the same address, the opposite of the D-11 default (pdf 57, Fig. 3.2.6 pdf 49, cycle arithmetic); (4) direct mode reads its address word with an ADAR access, which the D-02 kind list lacks (pdf 11, 30); (5) the branch `7/9` column can never be resolved from the manual, so branch cycle expectations should be `policy` vectors, not permanent expected-skips; (6) D-14's `cite` form contains `§`, which the ASCII-only `data/` integrity rule rejects.

Nothing is left unread that blocks vector authoring. Four reads that were "still unread" are now done (Section 2). The rest of the risk is structural: the gate is already 30 s wall on this loaded host, the control runner is sys-time bound by per-control tree copies, new steps/controls must land atomically with their `_good/tree` fixtures, and the first gcc 13 `-Werror` contact happens only on hosted CI.

**Primary recommendation:** make plan 2 a walking skeleton (CMake + harness + generator + a first real vector group + minimal stepper + inventory step and its controls + the fault path with one fault), so no gate state is ever "empty inventory" or "unlisted step"; then add vector groups and their faults together.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Instruction semantics, flags, cycle totals, interrupt/reset entry | Core (`core/src/cpu`) | - | Pure function of (state, lines, replies) (D-05) |
| Access list and per-access wait | Core emits records | Bus port (`ivs_bus_fn`; flat 64Kx16 test bus now, Phase 3 machine later) | Replies supply data and waits; core never knows ticks (D-04) |
| Line sampling point (unit end minus 1 cycle) | Machine layer (Phase 3/6) | Core receives sampled values | POL-LINE-01 |
| Vector truth data | `data/cp1610/vectors/*.json` | `cmake -P` generator to build dir | No JSON parser in C (D-14) |
| Inventory/skip/stale detection | Gate scripts (`tests-inventory`) | Harness TAP output | RUN-03 |
| Purity | Gate (`c-purity`, nm allowlist) | CTest `core_symbols` on shipped artifact | API-05 |
| Fault controls | Compile-time sites in core TUs | `tests/faults.cmake`, `c-faults` | D-18 |

## Standard Stack

No external packages are installed by this phase (copy-paste over dependency; harness, TAP checker and JSON reading are in-repo/CMake built-ins).

| Tool | Local (this host) | Hosted ubuntu-24.04 (image 20261101.360.1) | Note |
|------|------------------|--------------------------------------------|------|
| CMake | 4.4.3 [VERIFIED: `cmake --version`] | 3.31.6 [CITED: github.com/actions/runner-images Ubuntu2404-Readme] | Declared floor 3.25 is untested; effective tested range 3.31-4.4 |
| Ninja | 1.13.2 [VERIFIED: `ninja --version`] | 1.13.2 [CITED: same Readme, "Tools"] | Resolves the D-24 "ninja missing" risk: no apt step needed |
| C compiler | Apple clang 21.0.0 (arm64) [VERIFIED] | gcc 4:13.2.0 (apt), clang 16/17/18 [CITED: same Readme] | Two compilers, two OSes without a matrix |
| nm / ar | llvm-nm (`/usr/bin/nm`, "compatible with GNU nm") [VERIFIED] | binutils 2.42 [CITED] | Output shapes differ, see Pitfalls |
| sh | macOS `/bin/sh`; `/bin/dash` also present [VERIFIED: `ls`] | dash 0.5.12 [CITED] | Run the new scripts once under `dash` locally |
| awk | BWK awk 20200816 [VERIFIED] | mawk (not listed in Readme) [ASSUMED] | Write POSIX-only awk |

Package Legitimacy Audit: not applicable (no packages installed; no `npm`/`pip`/`cargo` surface).

**Alternatives rejected by CONTEXT, not reopened:** third-party test library, hand-written JSON parser, committed generated header, per-fault build dirs, runtime `--fault` switch.

## Section 1 (requested item 2): Remaining primary-source reads

All read this session from page images. `[VERIFIED: pdf N]`.

### 1a. Per-row cycle table coverage (sequencing aid for vector groups)

| pdf | Printed section | Rows with cycles printed | Printed values |
|-----|-----------------|--------------------------|----------------|
| 53 | Register-register, shifts | MOVR, TSTR, JR, ADDR, SUBR, CMPR, ANDR, XORR, CLRR, INCR, DECR, COMR, NEGR, ADCR; SWAP, SLL, RLC, SLLC, SLR, SAR, RRC, SARC | 6 (MOVR "6 *" +1 if DDD is 6/7; TSTR "6 *" +1 if SSS is 6/7; JR 7); shifts 6 and 8 rows per N; header "Add 2 cycles if shift is 2 bits or two bytes"; "Shifts are not interruptable." |
| 54 | Branches | B, NOPP, BC/BLGT, BNC/BLLT, BOV, BNOV, BPL, BMI, BZE/BEQ, BNZE/BNEQ, BLT, BGE, BLE, BGT, BUSC, BESC, BEXT | "7/9" on every row except NOPP "7"; B range "(+1025 to -1024)"; BLGT/BLLT/BEQ/BNEQ are aliases of BC/BNC/BZE/BNZE |
| 55 | Direct and indirect data | MVO 11; MVI/ADD/SUB/CMP/AND/XOR 10; MVO@ 9; PSHR 9; MVI@ and five ALU@ "8 *"; PULR 11 | "*Add 3 to number of cycles if MMM=6." Note on MVO@: "SSS = MMM = 4, 5, 6 or 7 not supported" |
| 56 | Control and jump | GSWD, NOP, SIN, RSWD 6; HLT, EIS, DIS, TCI, CLRC, SETC 4; J, JE, JD, JSR, JSRE, JSRD 12 | "Not Interruptable." printed on EIS, DIS, TCI, CLRC, SETC only; HLT text: "HaLT after next instruction is executed. Resume on control start." |
| 57 | Immediate, SDBD, indirect SDBD | MVOI 9; MVII, ADDI, SUBI, CMPI, ANDI, XORI 8; SDBD 4; SDBD+MVI@/ADD@/SUB@/CMP@/AND@/XOR@ rows 4 then 10 | Separate 4 and 10 rows per pair |
| 58 | Immediate double byte | MVII, ADDI, SUBI, CMPI, ANDI, XORI | 14 each; the SDBD word `0000 000 001` is the first line of every 14-cycle row; MVOI absent |
| 44-45 | Notation and status-bit semantics (section 3.1.1-3.1.2) | - | Only page defining C, OV, Z, S (see 1d) |

Group sequencing by what each group needs from the stepper: (a) control + register-move + register-ALU + shift (no bus replies beyond fetch; pdf 50, 51, 53, 56, 45); (b) direct/indirect/immediate memory (replies, auto-increment, R6; pdf 46, 48, 49, 55, 57); (c) branch + jump (displacement and `ebci`; pdf 47, 52, 54, 56); (d) SDBD (needs (b)); (e) reset/interrupt/BUSRQ/HLT/wait policy (needs the interruptibility column of every earlier row). Encode the per-row `interruptible` value in the decode table in plan 4 so later groups do not retrofit it (vector lists stay literal per D-08).

### 1b. Branch taken / not-taken cycle column - READ, unresolvable from the manual

- pdf 54 prints `7/9` with no label on B, every condition row and BEXT; NOPP (`1000 S0 1000`, the never-true twin of B `1000 S0 0000`) prints a single `7`. No other page labels it; pdf 8 only says full execution times "range from 1.6 to 4.8 microseconds" (4 to 12 cycles at 400 ns); pdf 43 only says timing is "in terms of CPU cycle times".
- The only printed bus sequence is BEXT (pdf 32): BAR@0, NACT, DTB@2, NACT, BAR@4 labelled "OUTPUT PC+1 TO FETCH DISPLACEMENT", then the figure ends. EBCI is sampled at the TS1 that starts that displacement BAR [VERIFIED: pdf 32, "VALID INPUT THROUGHOUT TS1"], so the `ebci(E)` provider is conceptually read at offset 4. E is the low 4 bits of the instruction register [VERIFIED: pdf 11, EBCA0-3].
- Tension to record: every other two-word class (immediate MVII, 8 cycles) follows BAR@4, DTB@6 and ends at 8, but a not-taken branch is printed 7. The 7 cannot be rebuilt from the figure pattern, so offsets after BAR@4 are inference for branches.
- Reading 7 = not taken, 9 = taken stays the best inference (NOPP can only be never-taken and prints 7) [INF, ~85%]; it is POL-BR-01/C29.
- Planner action: branch vectors should be `kind=source` for registers/PC/flags (displacement rule pdf 47) with the cycle expectation carried as a `kind=policy` vector citing pdf 54 and C29. Do not park them in `cp1610-skips.ids`: nothing can un-skip them, so the skip count would never reach zero.

### 1c. SDBD 10-vs-14 split - READ, resolved by arithmetic; modes 1-3 read count leans against D-11

- Immediate: single form 8 (pdf 57), double form 14 with the SDBD word printed as the first line of the row (pdf 58) so 14 = SDBD 4 + 10. Indirect: single 8, double table prints an SDBD row of 4 and an instruction row of 10 (pdf 57), total 14. Both agree only if the printed 10 excludes the 4; the alternative (10 includes SDBD) would make the instruction part cost 6, cheaper than the single form (8), which is implausible. One extra word read adds 2 cycles (8 to 10). C34 stands: 4 + 10. [VERIFIED: pdf 57, 58; arithmetic INF, ~90%]
- Modes 1-3 conflict is real and the weight of the page evidence is 2:1 toward two reads: pdf 47 "only one word is accessed with both bytes of data obtained from the lower eight bits of the same word"; pdf 57 SDBD row "will access the same byte twice resulting in both bytes of data being the same"; Fig. 3.2.6 pdf 49 prints `EA_L <- (MMM)` and `EA_H <- (MMM)` for MMM 1-3 with no increment between; the indirect double-byte table prints one `10` for all MMM (no per-mode cost), which is 8 plus one extra access. Register result is identical either way; only the access list (1 vs 2 reads) and side-effecting peripheral reads differ. D-11 chose one read as [default-chosen]. **Recommendation: switch the default to two reads at the same address, no increment** (still log it; keep the one-read alternative as a profile-style policy only if cheap). Total cycles 10 under either reading. This is a soft contradiction of D-11; the planner decides, but the vector bus list for modes 1-3 must follow whichever is chosen and be tagged `policy`.

### 1d. Flag (S/Z/C/OV) effects - READ, agrees with `opcode-families.json`; semantics gaps recorded

All 74 rows' printed status-change columns on pdf 53-58 match the table's `status_bits` (spot-compared every row group). Notable printed facts: MVO/MVI/MVO@/MVI@/MVII/MVOI/PSHR/PULR change no flag; INCR/DECR/COMR change S,Z only; NEGR and ADCR change S,Z,C,OV; GSWD, NOP, SIN none; RSWD S,Z,C,OV; CLRC/SETC C; branches and jumps none; shifts per N as in the table.

Semantics page is pdf 45 (section 3.1.2), not the instruction pages: C "set if the result of an ADD, SUB, or CMP instruction produces a carry ... SUB and CMP are performed by addition of two's complement of the subtrahend"; OV "arithmetic overflow (two's complement notation)"; Z "all zero result"; S "a one in the high order bit position". Gaps the hand-derivation must record as readings (each is a place a shared misreading can hide):

| # | Gap | Evidence | Recommendation |
|---|-----|----------|----------------|
| G1 | CMP direction: CMPR/CMP/CMP@ say "CoMPare ... with ... by subtraction" (operand order unstated); CMPI says "from contents of register SSS" (pdf 57) | pdf 53, 55, 57 | Anchor on the CMPI wording (register minus data) and SUBR's explicit "SUBtract Register SSS from contents of register DDD"; derive carry as the carry-out of register + two's complement of the subtrahend; log as a reading |
| G2 | S after right shifts and SWAP is bit 7 of the result, not bit 15 | pdf 53 header: "Shift Right instructions set the S flip-flop with Bit 7 of the result"; SWAP row "S equals Bit 7 of results of SWAP" | Literal vectors; left shifts use the generic rule (INF) |
| G3 | Link-bit positions differ left vs right: RLC N=1 "C as bit 17 and OV as bit 16"; RRC N=1 "C as bit 16, OV as bit 17"; RLC/RRC N=1 use OV as an input, N=0 preserve OV | pdf 53 | Derive with an 18-bit string per row, write the string in `derivation`; include an initial-OV=1 case |
| G4 | NEGR/ADCR/RSWD set C/OV but 3.1.2 names only ADD/SUB/CMP | pdf 45 vs 53, 56 | Derive by analogy (NEGR = 0 minus x via two's complement addition; ADCR = x + C), tag `inference` in `derivation` |
| G5 | Backward branch: one's complement of the displacement is added to PC+2 (pdf 47); width unstated; the printed range "(+1025 to -1024)" mixes bases (forward end is PC+2+1023; backward end -1024 matches PC+2 minus 1024) | pdf 47, 54 | Use the 16-bit complement of the full word for derivation; do not derive from the range text; log |
| G6 | Direct EA is `aaaaaa AAAAAAAAAA` where the six high bits depend on memory width (pdf 48); jump uses bits 7..2 of word 2 and bits 9..0 of word 3 (pdf 52) | pdf 48, 52 | The "ignore bits 15..10" rule of POL-ILL-01 applies to the first decle only; operand, direct-address and displacement words keep all 16 bits (flat 16-bit test bus); Phase 3 decides what 10-bit ROM upper bits read as. Log as default-chosen |
| G7 | SDBD modes 1-3 | see 1c | - |
| G8 | JSR return address: reference-notes call the last digit smudged on pdf 52; pdf 56 prints "Return address (PC+3)" legibly | pdf 52, 56 | Update reference-notes (D-13) |

R6/R7 cycle bonus is printed only on MOVR (DDD 6/7) and TSTR (SSS 6/7), plus `+3` when MMM=6 on the six `@` read forms; ADDR/SUBR/CMPR/ANDR/XORR/CLRR print a flat 6 even with R6/R7. Add negative vectors (ADDR R0,R7 stays 6) so a blanket "R7 costs +1" fault is caught.

## Section 2 (requested item 3): Spot-verification of CONTEXT claims

| Claim (CONTEXT) | Page evidence | Verdict |
|-----------------|---------------|---------|
| Reset: IAB@1, first BAR@5, 5 cycles (D-09) | pdf 27: NACT (cycle 0), IAB (1), NACT x3, BAR (5) from TS1 "START HERE" [VERIFIED] | Confirmed |
| Interrupt entry 9 cycles; INTAK@0 DW@2 DWS@3 IAB@5 BAR@9 (D-08, C30) | pdf 35 cells: INTAK, NACT, DW, DWS, NACT, IAB, NACT x3, BAR; pdf 10: DW "occurs one machine cycle time prior to DWS" [VERIFIED] | Confirmed |
| BUSRQ before INTR; INTR always; INTRM needs INTFF; samples each TS1 (D-08) | pdf 12, 22 flow diagram, 24, 25 [VERIFIED] | Confirmed |
| Grant only at interruptible boundary, never after interrupt entry (D-10) | pdf 22: interrupt sequence returns to the fetch loop with no test; pdf 36 "END OF CURRENT INTERRUPTABLE INSTRUCTION" | Confirmed; **refinement**: pdf 22 shows the bus-wait loop exiting into the interrupt test, and pdf 36 labels the post-release BAR "PROGRAM COUNTER ... (OR STACK POINTER TO SERVICE INTERRUPT)". `prev_interruptible` must stay true across a BUS_GRANT unit, and a vector must cover BUSRQ+INTR together (grant first, interrupt entry on release) |
| Release latency 2 cycles (D-10, INF ~60%) | pdf 36: BUSRQ sampled at a TS1, BAR starts about two cycle-widths after; the figure has // breaks | Consistent, still inference |
| NOP/SIN N is DDD bit 0; GSWD R4/R5 = NOP, R6/R7 = SIN (D-11) | pdf 51 footnote; pdf 56 rows `0000 110 0DD` GSWD, `10N` NOP, `11N` SIN; SIN "pulse to PCIT * pin"; pdf 13 "acknowledged at the end of the SIN instruction" | Confirmed (SIN interruptible) |
| SDBD 1-3: pdf 47 vs pdf 57 conflict (D-11) | see 1c | Conflict real; evidence leans to two reads |
| TCI/PCIT strobes may be stamped past unit end (D-12) | pdf 33: TCI (4 cycles) pulse begins about 1.2 cycles after the unit end (cycle 5). pdf 34: PCIT output pulse spans about cycles 4-5 of the 6-cycle SIN | Confirmed; the TCI strobe sits at unit end + 1, not "up to" |
| HLT sets a latch evaluated at the next interruptible boundary, non-interruptible itself (D-11) | pdf 56 "HaLT after next instruction is executed. Resume on control start."; pdf 14 STPST "only after the completion of an interruptible instruction"; pdf 74 per notes | Consistent: non-interruptible HLT makes the following instruction run, then stop |
| `lines.stpst` is a per-step input (D-05) | pdf 14: "edge-triggered", "negative transition of STPST will cause the CPU to stop ... The next negative transition ... return to the run mode" | **Contradicted as a level**: define `stpst` as an edge event (caller passes the negative edge once), because a level would toggle every step. Same latch serves HLT and STPST; a second edge before the boundary cancels the pending stop [INF] |
| Wait cap 35 from "under 40 us" (D-10) | pdf 12 "less than 40 microseconds"; 40 us / 1.1175 us per CPU cycle (D2) = 35.8 | Valid only for the D2 cycle; at the manual's 400 ns it would be 100. Keep it a profile-derived field (D-13) |
| Waits on every access (D-02/D-10) | pdf 40: BDRDY sampled "immediately following every micro state in which a BAR or ADAR bus control signal is issued"; pdf 43 "8+2W" | **Refine**: INTAK, IAB, DW, DWS accesses carry no wait; the write's wait belongs to its BAR. One wait value per record, applied after its BAR/ADAR (INF for the direct-mode address word) |
| Kinds FETCH, READ, WRITE, INTAK, IAB, strobes (D-02) | pdf 11: "ADAR ... activated during all instructions which specify direct addressing"; pdf 30: BAR@4, ADAR@6, DTB@8 | **Gap**: direct mode has three reads (instruction, address word via ADAR, operand). Add an ADAR flag or kind so the list is comparable with the figure (SC4). `IVS_CP1610_MAX_ACCESSES`=8 stays ample (real max 3-4) |
| POL-STK-02 default (D-11) | Fig. 3.2.4 pdf 48 orders `EA <- (MMM)` then `(MMM) <- (MMM)+1`; section 3.2.3: second operand register receives the result; MVO@ SSS=MMM 4-7 is "not supported" (pdf 55), which is exactly the case where the output register would already be incremented | Page-supported: result wins over the increment for MVI@; the increment happens before the operation, so ADD@ R4,R4 sees R4+1 as its second operand [INF]; log both |

Machine-readable summary of contradictions to apply: (1) `stpst` edge; (2) keep interruptibility through BUS_GRANT; (3) SDBD 1-3 two reads (soft); (4) ADAR in the access list; (5) branch cycles as policy vectors not skips; (6) vector `cite` must be ASCII (`S3.2.1, pdf 46`, the form in `data/cp1610/opcode-families.json`: `'cite': 'S3.3.4 Fig 3.3.4, pdf 51; control table, pdf 56'`); D-14's `§` fails `integrity-non-ascii` for `data/`.

Not modelled and worth one discrepancy line: PCIT as an input (suppresses PC increment during fetch, pdf 13); the `lines` struct has no field for it.

## Architecture Patterns

### System flow (conceptual)

```
data/cp1610/vectors/*.json --(cmake -P generator, sha256 embedded)--> build/<preset>/vectors.inc
         |                                                              |
 data/expected/cp1610-vectors.ids <-- cp1610-vectors data check          v
                                   tests/harness (ivs_test.c) --> ivs_cp1610_step(state, lines, bus_fn, log)
 lines + bus replies (flat 64Kx16 test bus)  ---------------------^   |  result + ordered access list
                                                                       v
                   TAP file $IVS_TEST_OUT_DIR/<suite>.txt  (nonce, plan, trailer)
                                                                       v
 data/expected/tests/<suite>.ids + NONCE --> tests-inventory step --> executed/skipped/missing
 IVS_FAULT_<ID> TU copies --> ivs_faults exe --> c-faults (must_fail / prefix / sensitivity)
 core sources --> temp -O0/-O2 archives --> core-symbols.sh (allowlist) --> c-purity; ctest core_symbols on shipped lib
```

### Recommended plan shape (amends the Discretion suggestion; reasons in Hazards)

1. Gate hardening and SPDX/ASCII extension for `core|include|tests` (lands before any `.c`).
2. **Walking skeleton**: CMake/presets/warnings, harness, generator, `ivs_cp1610_state_size/validate/reset`, a minimal stepper for the control-class ops (CLRC, SETC, EIS, DIS, TCI, HLT/NOP/SIN, GSWD/RSWD), the first vector file, `c-build`, `c-tests`, `tests-inventory` plus controls, and the fault infrastructure with one fault (`c-faults`). End state: inventory non-empty, every inventory file non-empty, one fault proven end to end.
3. Purity step, symbols script, controls, `_good/tree` stand-in core.
4. Decode table and register/ALU/shift groups, each with its faults.
5. Memory groups (direct/indirect/immediate, R4-R6 auto-increment, stack), then branch/jump, then SDBD, each with faults.
6. Interrupt/BUSRQ/reset/HLT/wait policies and `cpu-bus` figure-based sequences, with their POL faults.
7. Fault sufficiency review (every plausible-bug in D-18 list has a catching vector) and generator-path `expect` mutation control.
8. Blind second-reader pass (vectors frozen), discrepancy/notes/profile-field updates, CI-EFFICIENCY record (including amending its "add a macOS leg when the C build arrives" line, which D-24 supersedes).

### Anti-patterns
- A step script, its `data/expected/gate-steps/*.ids` line, its control directories, its `data/expected/controls/*.ids` lines and its `_good/tree` fixtures landing in different plans (red gate in between).
- Fault sites retrofitted into finished TUs in one late plan (touches stable code; each group plan should add its own sites).
- Branch/cycle uncertainties parked as expected-skips.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Reading JSON vectors | C JSON parser, awk JSON parser | `cmake -P` with `string(JSON ...)` (D-14; extract each vector into a sub-string first) | Second parser rejected; speed measured in CONTEXT |
| sha256 of vector files | `sha256sum` vs `shasum` branching | `file(SHA256 ...)` inside cmake -P (`cmake -E sha256sum` verified locally per project notes) | One portable primitive |
| Record/id inventory comparison | New comparison scripts per checker | `sort`/`comm` under `LC_ALL=C` as in `gate.sh` | Already the repo idiom |
| Multi-file vector schema check | Reuse `records.cmake` unchanged | A dedicated `vectors.cmake` like `coverage.cmake`, or generalise `file` to a list | `records.cmake` schema DSL has one `file` per kind [VERIFIED: tools/check/schema/cp1610-row.txt line `file data/cp1610/opcode-families.json`, records.cmake line 53 `if(d STREQUAL "file")`] |

## Common Pitfalls (verification steps should check these)

1. **`#pragma GCC poison float double` placement.** Placed before `<stddef.h>` it errors on Apple clang 21: `__stddef_max_align_t.h:16:14: error: attempt to use a poisoned identifier` (`typedef long double max_align_t`). Executed locally. Prelude order that compiled clean with the full D-19 warning list plus ASan/UBSan: include `limits.h stdbool.h stddef.h stdint.h string.h` first, poison last. Add a control (`core-purity-header`) that proves the prelude is mandatory, and a one-line check that the prelude is the first include of every core `.c`.
2. **`nm -P` shapes.** Local llvm-nm: `nm -P -u a.o` prints `_ext U 0 0`; `nm -P -g -U` prints `_f T 0 0` and `_g D 20 0`; on an `ar` archive it prints a blank line plus a `a.o:` member header before symbols; ASan objects add `___asan_init`, `___asan_version_mismatch_check_apple_clang_2100` (leading underscore plus `__asan_`) [VERIFIED executed]. The parser must key on `$1`/`$2` only and skip lines with fewer than two fields or ending in `:`. GNU nm documents `-U` as `--defined-only` [CITED: sourceware.org/binutils/docs/binutils/nm.html]; prefer the long form `--defined-only`, which both nms accept (long form verified on llvm-nm only). GNU archive header format `archive[member]:` and the field count of GNU `-P` undefined lines are [ASSUMED] (see log). `-fsanitize` objects only get the `--allow-prefix` list, as D-21 says.
3. **Control-runner coupling.** `run-controls.sh` runs `steps/<CHECK>.sh` with `ROOT=$tmp/tree` copied from `_good/tree`, and its positive control requires every distinct `CHECK` step to pass with `-- <step> pass` on the pristine tree [VERIFIED: read `tools/check/run-controls.sh`]. So `tests-inventory` must read TAP files and `NONCE` from `$ROOT/build/gate` (not `GATE_TMP`), and `_good/tree` must carry a tiny consistent inventory + TAP + NONCE; `c-purity` controls need a stand-in `core/` in `_good/tree`. Controls also require `data/expected/controls/*.ids` parity or `control-not-in-inventory`/`control-inventory-orphan` fire.
4. **Step inventory parity.** `gate.sh` compares `tools/check/steps/*.sh` with the union of `data/expected/gate-steps/*.ids` and prints `gate-step-missing`/`gate-step-extra` [VERIFIED: read `tools/gate.sh`]. New steps must land with their ids line.
5. **Second-reader schema is strict.** `schema/second-reader.txt` fixes `topkeys schema_version reader method c17_reading rows` and per-row `keys`, with parity to `cp1610-rows.ids` [VERIFIED]. D-15's "extend `second-reader.json`" would break `cp1610-reader` and its three controls. Recommend a sibling file/kind (`data/cp1610/vectors-second-reader.json`, own schema and control) mirroring the pattern.
6. **ASCII, SPDX, denylist, path rules.** `integrity.cmake` bans non-ASCII under `data|fixtures|tools|.github` and an empty file anywhere; SPDX is enforced for `tools/*.sh|cmake` and workflows only (D-23 extends to `.c/.h`); the denylist step scans every tracked path except `.planning/`, `.claude/`, `docs/provenance*`, `tools/check/`, `data/sources.json` and fails on `Zbiciak`, `Nathan Woods`, `Palazzolo`, `license:BSD-3-Clause`, and on `jzintv` unless the line carries `[cite-only]` [VERIFIED: read `denylist.sh`, `denylist*.txt`]. Do not commit an empty `.gitkeep` for `core/include/ivs/` (create nothing). Path charset is `[A-Za-z0-9._/-]`: no `@` or `+` in file names (row ids like `MVO@` live only inside JSON and `.ids` contents).
7. **gcc-only warnings.** `-Wconversion` on compound assignment of narrow types (`r += 1` on `uint16_t`) did not warn on Apple clang 21 (probe executed) but gcc is known to warn [ASSUMED]; first gcc 13 `-Werror` contact is hosted PR CI. Route every narrowing store through one explicit-cast helper (`IVS_U16(x)` = `(uint16_t)((x) & 0xFFFFu)`), and budget one CI fix iteration on the phase branch PR. Optional local second compiler: Docker CLI exists at `/usr/local/bin/docker` (daemon not probed); not a gate dependency.
8. **Sanitizers on hosted Linux.** Past ubuntu runner images raised `vm.mmap_rnd_bits` and crashed ASan; the common workaround was `sudo sysctl vm.mmap_rnd_bits=28`, and one later project removed the workaround after an image fix [CITED: web search, mails.dpdk.org 2024-11 thread; unverified against runner-images]. If the first hosted run dies at startup with a DEADLYSIGNAL, add that single step (needs a ci-parity allowlist edit) rather than dropping sanitizers.
9. **Fault copies and sanitizers.** A fault that causes an out-of-bounds access would abort under ASan and show as `fault-wrong-reason`. Apply sanitizer flags per target through an interface library, not global `CMAKE_C_FLAGS`, so fault executables are plain `-O1` (compile+link of a 4-file C exe measured about 0.2 s either way under load 19; 30 faults about 6 s serial, less under ninja -j).
10. **`ctest --preset` with `noTestsAction`** works on CMake 4.4.3 (executed in scratch with two presets: 1.3 s total including configure, build, ctest under load 12); the field belongs to preset schema v6 / CMake 3.25 [CITED: cmake.org/cmake/help/v3.25/manual/cmake-presets.7.html]. Avoid post-3.25 CMake features; the floor is declared only.
11. **STPST and BUS_GRANT state.** Implement the two refinements from Section 2 before writing vectors; adding them later changes the 32-byte layout (D-03 reversibility: costly).
12. **HALTED behaviour is a page gap.** pdf 14 says only STPST edge resumes; it is silent on INTR/BUSRQ while stopped. Default-choose (suggest: HALTED unit ignores INTR/INTRM, still honours BUSRQ because the CPU is off the bus anyway) and log as C36+; give it a vector and a fault.

## Section 3 (requested item 4): Planning risks and sequencing hazards

**Gate-red-between-plans checklist (every plan that adds a gate-visible artifact):** step script + `data/expected/gate-steps/cpu.ids` line; for each control directory a line in `data/expected/controls/*.ids` and `_good/tree` fixtures that make the positive control pass; new `.c/.h/.json/.cmake/.ids/.txt` files pass `integrity`/`text`/`files`/`denylist` (SPDX line, ASCII, no CRLF, no emulator names); `.gitignore` already covers `/build/` (anchored, so `tools/check/controls/_good/tree/build/gate/` fixtures are not ignored [VERIFIED: `.gitignore` line `/build/`]).

**Non-empty inventory bootstrap.** `tests-inventory-empty` fires on an empty inventory (D-17), and `core-symbols-no-definitions` fires on an empty core object (D-21). Hence an "empty core" plan is red by construction. The walking-skeleton plan 2 above avoids it; `ivs_cp1610_state_size()` plus one vector group gives both a defined symbol and a non-empty inventory. Marker rule: the stale-marker check must not fail on an absent second-reader marker until plan 8; print a stat `second-read=<n>/<m>`, fail on stale (file_sha256 mismatch), and let phase verification require n==m.

**Gate runtime (D-22 budget).** Measured now: `sh tools/gate.sh` 29.9 s wall (user 11.0 s, sys 17.3 s) at load average ~12; the control runner alone 17.2 s wall (5.6 s user, 10.3 s sys) for 112 controls, about 0.15 s each, sys-dominated by `cp -R` of the 96 KB, 24-file `_good/tree` [VERIFIED: executed]. Adding ~25 controls costs about +4 s locally. Keep `_good/tree` additions minimal (stand-in core of 1-2 files, ids and TAP of a few lines). Lever 2 of D-22 is not an `xargs` one-liner: `run-controls.sh` shares `$tmp/tree`, `$tmp/out`, `$tmp/filelist` and the `ok/total` counters, so parallelising means per-control temp dirs and result files; schedule it as its own task only if the budget is hit. A two-preset configure/build/test of a tiny project cost 1.3 s under load, so CMake overhead is small; the sanitizer vector run and the faults dominate. Measure idle baseline before plan 2 so the 25 s local budget is judged on idle, not on a loaded host.

**Portability traps.** awk: BWK 20200816 locally vs mawk on hosted [ASSUMED]; stay POSIX (no interval expressions, `gensub`, `\y`, `systime`). `sed`: no `-i`, no `\+`/`\|`. `sort`/`comm` under `LC_ALL=C` as in the repo. `mktemp -d "${TMPDIR:-/tmp}/x.XXXXXX"` pattern already used. Run the new scripts once under `/bin/dash`. `ar rcs` works on both. macOS ASan: do not set `detect_leaks=1`.

**Other sequencing notes.**
- Gate step order is alphabetical in the C locale; `c-build` < `ci-parity` ('-' sorts before 'i'), so C steps run first; `c-faults` runs before `c-tests` (fine, both build first).
- The README line "needs POSIX shell, git and CMake" must change in plan 1 or the plan that adds the ninja/cc precheck.
- `gate-no-ninja`/`gate-no-cc` live in `gate.sh`, which the control runner cannot drive (like `gate-no-cmake` today); cover with a throwaway local PATH probe, not a control.
- Profile-field additions (D-13) must update the profile record and `data/expected/profile-fields.ids` together or the `profile` step fails.
- Discrepancy ids start at C36 (current inventory ends at C35 [VERIFIED: `data/discrepancies.json` C29-C35 read]); new entries must be added to `data/expected/discrepancies.ids` in the same commit.
- Reference-notes updates (D-13) should add: N-bit (pdf 51/56), C34 corroborated, C29 NOPP twin, timing figures pdf 27-37, ADAR (pdf 11), STPST edge (pdf 14), JSR (PC+3) legible on pdf 56.
- Fuzz-style run (D-12) under ASan/UBSan: seed fixed and printed, bounded iteration count, no host randomness in the harness (use an in-repo xorshift); it counts as a TAP suite with a fixed id list.

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | In-repo C harness `tests/harness/ivs_test.{h,c}` (TAP-like) driven by CTest; gate parses TAP files |
| Config file | `CMakePresets.json` (v6), `tests/CMakeLists.txt`, `tests/faults.cmake` (all Wave 0) |
| Quick run command | `cmake --preset dev && cmake --build --preset dev && ctest --preset dev` (D-19); `--only <prefix>` on the harness |
| Full suite command | `sh tools/gate.sh` |

### Suites and inventories (names are recommendations; Discretion)
| Suite (one ctest test each) | Contents | Id inventory |
|------------------------------|----------|--------------|
| `harness` | TAP formatting, bitmap duplicate detection, log_cap refusal plumbing | `data/expected/tests/harness.ids` |
| `cpu-vectors` | all JSON vectors (~300-360), source and policy | `data/expected/tests/cpu-vectors.ids` set-equal to `data/expected/cp1610-vectors.ids` |
| `cpu-sweep` | 1024-decle decode sweep, first-decle ignore of bits 15..10 over 65536 words | `data/expected/tests/cpu-sweep.ids` |
| `cpu-bus` | figure-based access-list/offset contracts (pdf 27-37), state_validate, refusal-never-mutates, no-progress guard | `data/expected/tests/cpu-bus.ids` |
| `cpu-fuzz` | seeded random state/lines/opcodes under ASan/UBSan | `data/expected/tests/cpu-fuzz.ids` |
| `core_symbols` (ctest, not TAP) | `core-symbols.sh` on `$<TARGET_FILE:core>` | list compared with `ctest -N` |
Planning note (2026-11-09, after plan-checker iteration 1): the plans fold the `cpu-bus` contracts into per-vector `bus` lists in `cpu-vectors` (every row needs one, rule `vector-bus-uncovered`) and into `cpu-api`; no `cpu-bus` suite or `cpu-bus.ids` exists. The ctest suite is named `core-symbols` and writes TAP like the others. The fault inventory has four columns (`<id> <must-fail-vector> <allowed-prefixes> <must-pass-vector>`).

Also: `data/expected/cp1610-skips.ids` (id + discrepancy), `data/expected/faults/cpu.ids` (`<id> <must-fail-vector> <allowed-prefix>`), `data/expected/gate-steps/cpu.ids`, `data/expected/controls/{tests-inventory,core-purity,...}.ids`.

### Phase Requirements to Test Map
| Req ID | Behaviour | Automated check | Where it runs | Inventory | Handed to owner |
|--------|-----------|-----------------|---------------|-----------|-----------------|
| CPU-01 | every row's registers/memory/flags/cycles with page cite | exact-match vectors; checker derives coverage classes from table columns (D-16), cite regex, first-decle row-label check, vectors-digest; decode sweep; arithmetic/flag/cycle faults | ctest dev+release (`c-tests`); data checks in gate; `c-faults` | `cp1610-vectors.ids`, `tests/cpu-vectors.ids`, `tests/cpu-sweep.ids`, `faults/cpu.ids` | nothing |
| CPU-02 | INT/BUSRQ acceptance (prev-instr interruptibility, I flag, D/SDBD, BUSRQ priority, BUSRQ+INTR, STPST edge, HLT) | policy vectors per POL-INT/BUS/HLT/LINE; one fault each (I flag ignored, BUSRQ priority, D/SDBD interrupted, JE/JSRE timing, accept-clears-INTFF both values) | `c-tests`, `c-faults` | same files | nothing; hardware timing stays "missing evidence" (D10) |
| CPU-03 | SDBD modes 1-5 and refusal 0/6, MVO@/branches/other targets | vectors for each supported mode and each refusal (`UNSUPPORTED_SDBD_TARGET`, PC stays, DBD set); faults: indirect increment, immediate PC advance, flag change, interruptible SDBD | `c-tests`, `c-faults` | same | nothing |
| CPU-04 | stack direction, R4/R5/R1-3 auto-increment, reset entry | vectors from pdf 46/48/62/21/27 incl. POL-STK-01/02, POL-RST-01 (provider $0000, $FFFF), POL-IAB-01; faults: push/pull direction, increment class, reset entry, hardcoded $1004 | `c-tests`, `c-faults` | same | nothing |
| CPU-05 | pure stepper, ordered cycle-stamped access list | `cpu-bus` suite asserts exact offsets only for classes with a printed figure (fetch, indirect, direct incl. ADAR, MVO@, BEXT up to BAR@4, TCI, SIN, INT entry, reset, BUSRQ); inference-tagged elsewhere; `state_validate`; deterministic repeat; fuzz; log_cap refusal | `c-tests` | `tests/cpu-bus.ids`, `tests/cpu-fuzz.ids` | nothing |
| API-05 | no clock/fs/env/thread/IO/rand/alloc in core | `c-purity` (fresh -O0/-O2 archives + source layer + prelude) and ctest `core_symbols`; 13 `core-purity-*` controls | gate (`c-purity`), ctest | `controls/core-purity.ids`, `tools/check/core-symbols-allow.txt` | nothing |
| PKG-02 | one local command == PR gate | `sh tools/gate.sh` runs the C steps; ci-parity step unchanged; README line updated; ninja/cc prechecks | gate, CI | `gate-steps/cpu.ids` | nothing |
| RUN-03 | missing/duplicate/unexpected-skip/stale test fails; counts reported | `tests-inventory` over TAP + NONCE + ids + `ctest -N`; ~10-12 `tests-inventory-*` controls (missing, extra, duplicate, unlisted skip, stale nonce, truncated, plan mismatch, stale digest, stale review, bad cite, row mismatch, ctest drift) | gate (step), control runner | `controls/tests-inventory.ids` | nothing |

### Success criteria to checks
| ROADMAP criterion | Automated check | Runs in | Evidence file | Owner handoff |
|-------------------|-----------------|---------|---------------|---------------|
| 1 inventory + omission control | `tests-inventory` prints `tests-expected= tests-executed= tests-skipped= tests-missing=`; omission/duplicate/skip/stale controls | gate step + control runner | `tests/*.ids`, `controls/tests-inventory.ids` | none |
| 2 per-opcode vectors + broken opcode fails | `cpu-vectors` suite; `c-faults` proves a deliberately broken opcode fails its vectors with the allowed prefix; generator-path `expect` mutation control | ctest, `c-faults` | `cp1610-vectors.ids`, `faults/cpu.ids` | none |
| 3 INT/BUSRQ/SDBD/stack/auto-inc/reset each pass and each has a fault | policy groups; every POL id has a vector and a fault; set-equality of `IVS_FAULT_*` sites with `faults/cpu.ids` | ctest, `c-faults` | `faults/cpu.ids` | none |
| 4 ordered cycle-stamped access list vs manual sequence | `cpu-bus` suite and per-vector `bus` lists | ctest | `tests/cpu-bus.ids` | none |
| 5 symbol check fails on clock/fs/env/thread/IO + control | `c-purity` + 13 controls | gate | `controls/core-purity.ids` | none |

### Sampling Rate
- Per task commit: dev-preset quick run (`--only <group-prefix>` on the harness) plus the touched check script.
- Per wave merge: `sh tools/gate.sh`.
- Phase gate: gate green on both presets, `second-read=n/n`, then `/gsd-verify-work`.

### Wave 0 Gaps
- [ ] `CMakeLists.txt`, `CMakePresets.json`, `cmake/IvsWarnings.cmake`, `tests/{CMakeLists.txt,faults.cmake,harness/ivs_test.{h,c}}`
- [ ] `core/src/common/ivs_status.h`, core prelude header (poison, included first by every core `.c`)
- [ ] vector generator `cmake -P` script and a multi-file vector checker (`vectors.cmake`)
- [ ] `tools/check/steps/{c-build,c-tests,c-faults,c-purity,tests-inventory}.sh` (+ optional `cp1610-vectors.sh`), `tools/check/core-symbols.sh`, allow/deny lists
- [ ] `data/expected/{cp1610-vectors.ids,cp1610-skips.ids,tests/*.ids,faults/cpu.ids,gate-steps/cpu.ids,controls/*.ids}` and `_good/tree` fixtures
- [ ] Framework install: none (cmake, ninja, cc, nm, ar only; hosted image has all)

## Security Domain

`security_enforcement` is enabled (ASVS level 1). Applicable controls are about robustness of untrusted-shaped inputs, not authentication.

| ASVS category | Applies | Control |
|---------------|---------|---------|
| V5 Input validation | yes | Core refuses bad arguments (`IVS_E_BAD_ARG`, `IVS_E_BAD_STATE`, undersized `log_cap`) without mutating state; `ivs_cp1610_state_validate` rejects bad enums and nonzero reserved bits; guest faults are sticky, never abort; fuzz suite under ASan/UBSan |
| V2/V3/V4/V6 | no | no authentication, sessions, access control or cryptography in scope |
| V14 build/supply chain | yes | no third-party packages; pinned actions unchanged; purity allowlist fails closed; `-Werror=date-time`; no committed rights-bearing bytes |

| Threat pattern | STRIDE | Mitigation |
|----------------|--------|------------|
| Out-of-bounds from hostile bus reply or state | Tampering | all 16-bit math masked through `uint32_t`; no allocation; ASan/UBSan in the dev preset; fuzz |
| Non-terminating run loop on no-progress statuses | DoS | guard test required by D-12; wait cap faults with `BUS_WAIT_LIMIT` |
| Host dependency creeping into the core | Tampering | allowlist symbol check on fresh and shipped artifacts, source-layer grep, poisoned float/double |
| Stale or forged test output | Spoofing | NONCE per gate run, per-id bitmap, digest of vector sources |

## Environment Availability

| Dependency | Required by | Available | Version | Fallback |
|------------|-------------|-----------|---------|----------|
| cmake | all | yes | 4.4.3 local; 3.31.6 hosted | none (gate fails closed `gate-no-cmake`) |
| ninja | presets (D-19) | yes | 1.13.2 local; 1.13.2 hosted [CITED] | none; precheck `gate-no-ninja` |
| cc (clang/gcc) | build, purity | yes | Apple clang 21 local; gcc 13.2 hosted [CITED] | none; precheck `gate-no-cc` |
| nm, ar | purity | yes | llvm-nm local; binutils 2.42 hosted [CITED] | none |
| dash / shellcheck | portability probe | yes (`/bin/dash`, `/opt/homebrew/bin/shellcheck`) | - | optional |
| gcc locally | second-compiler probe | no (`ls /opt/homebrew/bin` has none; `llvm@18`, `llvm@21` only) | - | first gcc contact is hosted PR CI; optional Docker (daemon not probed) |
| refs/ page images | blind second reader | local only (gitignored) | - | CI verifies only the committed marker digest, never the pages |

No missing dependency blocks execution.

## Code Examples

Core prelude that compiled clean on Apple clang 21 with the D-19 warning set and ASan/UBSan (executed in scratch):

```c
/* core/src/common/ivs_prelude.h  -- included first by every core .c */
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#pragma GCC poison float double
```

Counter-example that failed (poison before std headers): `error: attempt to use a poisoned identifier` at clang's `__stddef_max_align_t.h:16:14`.

Parser sketch for `core-symbols.sh` (not executed; shape derived from the observed `nm -P` output above): read `nm -P -u FILE`, skip lines where `NF < 2` or the last field of a one-field line ends in `:`, take `$1`, strip `@ver`, strip one leading `_` on Darwin, subtract defined (`nm -P -g --defined-only`) and the allowlist.

## State of the Art

| Old / assumed | Current finding | Impact |
|---------------|-----------------|--------|
| Phase 1 note: add a macOS CI leg when C arrives | D-24: single ubuntu-24.04 job; clang local, gcc 13 hosted | amend `.planning/CI-EFFICIENCY.md` in plan 8 |
| Ninja may be missing from the hosted image | Listed in image Readme Tools (1.13.2) | drop the apt step from the risk list |
| Branch/SDBD cycle vectors as skips (D-16) | branch is policy-only; SDBD split resolved | zero permanent skips expected; `cp1610-skips.ids` may legitimately stay near empty |

## Assumptions Log

| # | Claim | Section | Risk if wrong |
|---|-------|---------|---------------|
| A1 | Hosted `awk` is mawk and stays POSIX-compatible with the checker | Standard Stack / Pitfalls | checker fails only on hosted CI; fix by simplifying awk |
| A2 | GNU `nm -P` prints archive headers as `archive[member]:` and undefined symbols with fewer fields than llvm-nm | Pitfall 2 | purity step misparses on hosted CI; parse on `$1,$2` only |
| A3 | gcc 13 `-Wconversion` warns on compound assignment to narrow unsigned types | Pitfall 7 | extra or fewer CI fix iterations |
| A4 | `cc` resolves to gcc 13 on ubuntu-24.04 | Environment | different compiler than assumed |
| A5 | The current hosted image no longer needs `vm.mmap_rnd_bits=28` for ASan | Pitfall 8 | first hosted run crashes at startup |
| A6 | HALTED idle ignores INTR/INTRM and still honours BUSRQ | Pitfall 12 | vector and discrepancy entry (C36+); logged default |
| A7 | Reading 7 = not taken, 9 = taken | 1b | branch policy vectors flip; sticky C29 |
| A8 | Wait after the direct-mode address word applies after its BAR/ADAR as one value | Section 2 | direct-mode wait vectors need re-stamping |
| A9 | Branch displacement DTB is at offset 6 (only BAR@4 is on the figure) | 1b | branch bus lists stay inference-tagged |
| A10 | Full 16-bit direct EA and displacement words on the flat test bus | G5/G6 | Phase 3 may mask to 10 bits; policy vectors only |

## Open Questions (RESOLVED)

1. **SDBD modes 1-3: one read or two?** Known: pages lean 2:1 to two reads (1c). Unclear: hardware. Recommendation: two reads, no increment, tagged `policy`, logged in the discrepancy entry D-13 already requires. **RESOLVED:** adopted as CONTEXT A-03.
2. **HALTED handling of INTR/BUSRQ** (A6): decide in plan 6, log as C36+, one vector and one fault. **RESOLVED:** adopted as CONTEXT A-10 (planned as POL-HLT-02).
3. **Vector-data checker placement:** separate `cp1610-vectors` step versus folding into `tests-inventory`. Recommendation: separate (pure cmake -P, 0.8 s class cost, drivable by controls) and list it in `gate-steps/cpu.ids`. **RESOLVED:** separate step, CONTEXT A-07.

## Sources

### Primary (HIGH)
- SRC-gi-cp1600-um-1975 page images, opened this session: pdf 9-14, 22, 24, 25, 27-37, 40, 43-58 (local `refs/pages/` and scratch renders; not committed).
- Repo files read: `02-CONTEXT.md`, `AGENTS.md`, `.planning/AUTONOMY.md`, `tools/gate.sh`, `tools/check/run-controls.sh`, `tools/check/steps/{cp1610,cp1610-reader,denylist}.sh`, `tools/check/{integrity.cmake,files.cmake}` (head), `tools/check/schema/{cp1610-row,second-reader}.txt`, `tools/check/kinds/second-reader.cmake`, `tools/check/{denylist,denylist-cite,text-ext,binary-ext}.txt`, `data/cp1610/opcode-families.json`, `data/discrepancies.json`, `docs/cp1610/reference-notes.md`, `.planning/CI-EFFICIENCY.md`, `.github/workflows/*.yml`, `.gitignore`.
- Executed probes (scratch only): `sh tools/gate.sh` and `tools/check/run-controls.sh` timings; llvm-nm output shapes; poison-prelude compile with ASan/UBSan and the full warning set; two-preset CMake/Ninja/CTest run.

### Secondary (MEDIUM)
- https://raw.githubusercontent.com/actions/runner-images/main/images/ubuntu/Ubuntu2404-Readme.md (CMake 3.31.6, Ninja 1.13.2, gcc 13.2.0, binutils 2.42, dash 0.5.12, image 20261101.360.1).
- https://sourceware.org/binutils/docs/binutils/nm.html (`-U` = `--defined-only`).
- https://cmake.org/cmake/help/v3.25/manual/cmake-presets.7.html (`noTestsAction`; version 6 = CMake 3.25).

### Tertiary (LOW)
- Web search on ASan and `vm.mmap_rnd_bits` on GitHub Ubuntu images (e.g. https://mails.dpdk.org/archives/dev/2024-November/308348.html); unverified against runner-images.

## Metadata

**Confidence breakdown:**
- Page readings (cycle/flag columns, timing figures): HIGH - images opened this session.
- Interpretations (branch 7/9, SDBD 1-3, wait scope, HALTED): MEDIUM/LOW - inference, logged.
- Gate/CI hazards: MEDIUM - local probes executed; gcc and mawk paths unverified until hosted CI.

**Research date:** 2026-11-09
**Valid until:** 2026-12-09 (stable tooling; re-check runner image Readme if the first hosted C run fails)
