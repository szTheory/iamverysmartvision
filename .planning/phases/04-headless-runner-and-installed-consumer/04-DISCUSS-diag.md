# Phase 4 discuss, decision point C: original diagnostic, boot stub, D4, guest-state peek

Labels: [OBS] fetched, read or executed this session; [INF] inference; [REC] recommendation; [UNK] unknown; [BG] background knowledge not re-fetched.
No emulator source was opened. No file other than this one was written (throwaway programs lived in the session scratchpad). No gh calls, no git changes.

## 0. Recommendation in one screen

1. D4 = a hand-assembled, annotated decle listing is the source of truth. No assembler is a build, gate, CI or release dependency.
2. The listing uses the repo's existing vector word form (`<hex4> <bit groups>`), checked by the rules `tools/check/vectors.cmake` already has (hex equals bits, first decle is encoded by the declared opcode row).
3. The oracle is arithmetic and manual tables, not a second interpreter: closed-form values, the GSWD bit layout (pdf 56) and the printed cycle table. One hand-derived predicate (10 memory words, 4 registers).
4. The boot stub is a 22-word text listing at `$1000..$1015`: reset `B` over the vector area, trap handler at `$1004`, `DIS; MVII #$0340,R6; J $5000` at `$1010`.
5. The runner, not the core, owns stub and GROM stand-in supply. The core stays image-free (D-24).
6. Guest state is read through a small public, side-effect-free inspection API (`ivs_machine_cpu_state`, `ivs_machine_peek`).

## 1. Facts found in the repo that shape the decision [OBS]

- Reset reaches `$1000` and interrupt entry reaches `$1004` through the profile (`reset.r7`, `interrupt.iab_value`; both tier4-community, C32 and C42). Only 4 words lie between the two vectors.
- Reset requires BOTH EXEC and GROM: `ivs_machine_reset` returns MISSING_SYSTEM_IMAGE when either `system_images.exec.required` or `system_images.grom.required` slot is empty (machine_run.c, profile). So the stub path also needs a GROM image. Nothing in the task or D-24 supplies one. See D-C6.
- Immediates in ROM are 10-bit decles. `MVII #$5000,R7` is impossible without SDBD; SDBD pairs carry an open conflict (C34: 14 versus 18 cycles). The diagnostic therefore avoids SDBD and builds wide values by arithmetic.
- Footgun found by running the core: `HLT` stops only after the NEXT instruction, and only at an interruptible boundary. A run of zero words (`HLT HLT ...`) therefore does NOT halt: each HLT executes the next one. `HLT; CLRC` ran a further instruction (CLRC is non-interruptible, C19/C56). `HLT; TSTR R0` halts right after TSTR [OBS: 71-unit run, R7 `5036`]. The "unused words are 0x000 (HLT), so they trap" idea in the task sketch is wrong: zero fill is not a trap.
- Existing text-fixture machinery: `data/cp1610/vectors/*.json` hold ~400 `asm` plus `program` pairs in the form `"0310 11 000100 00"`. The `vectors` step already checks (a) the binary groups equal the hex word (`vector-program-mismatch`), (b) the first decle is encoded by the declared row and the row is the most specific leaf of `data/cp1610/opcode-families.json` (`vector-row-mismatch`). Both have negative controls. Those rows were transcribed from the GI manual and read blind by second readers (12/12 agree).
- `ivs_cp1610_result`, the CPU struct and `sysram[]` are internal; there is no public way to read a register or a RAM word. `.ivsc` is already a binary-by-name extension, so a build-tree `.ivsc` can never be tracked by accident.
- No binary can be written by `cmake -P` (NUL bytes). Building `.ivsc` and the 8192-byte EXEC needs a small C tool (the core already has `ivs_sha256.c`).
- A throwaway program (scratchpad, not committed) loaded the stub and the diagnostic below into the Phase 3 machine through the internal step seam. The final memory words equalled my hand derivation, and the cycle total equalled the hand sum of the printed counts. My FIRST draft contained two encoding mistakes (INCR written `0001`, which is SDBD; DECR written `0002`); the row-label check would have caught both mechanically. [OBS]

## 2. D4: how diagnostics are produced

Options (lenses: emulator architect, QA, security/rights, devops, DX, product):

| Option | Pros | Cons / footguns |
|---|---|---|
| A. Annotated hand-assembled listing, checked by the existing row rules (REC) | Zero tools, zero licence surface, byte-exact review in a diff; fixtures are tiny (2 now); reuses vector checker logic by copy-paste; no new gate cost class | Branch/jump displacements are hand-computed; does not scale past a few dozen words per program |
| B. Project mini-assembler (C or cmake, ~300-500 lines: labels, ORG, B/J forms, operand parsing) | Scales; labels | A new program to test; author and emulator share one reading (the exact shared-misreading failure AGENTS.md warns about); premature for 2 programs. Future test corpus is free: the vectors' asm/program pairs |
| C. Pinned as1600 | Purpose built | GPL-2.0-or-later tool and library `.asm` INCLUDEs (CLAUDE.md Part 2); needs install in CI or a committed binary; adds a toolchain pin to a repo whose gate is C + cmake + sh |
| D. libasm (Apache-2.0) as black-box cross-check | Independent encoder written by someone else; permissive | C++ build; CP1600 10-bit decle packaging gap [UNK]; syntax must be learned from docs; not worth a CI dependency |

[REC] A as the rule; D as an optional authoring-time black-box cross-check only if it is already installed or costs under ten minutes. It is not in the gate, and its result is recorded as an executed result with tool, version, command and date (AGENTS.md: a report is not a fresh test). C is rejected. B is deferred with triggers: more than 3 diagnostics, a program needing labels or macros, or a second author. When B is built, the 400 vector asm/program pairs are its acceptance corpus, so deferring costs nothing.

Independence and the shared-misreading limit [INF]: the author derives both the words and the expected values, so the diagnostic cannot detect a misreading shared with the emulator. What it does give is (a) integration evidence (reset path, stub, J, cartridge fetch, RAM, halt) the unit vectors never ran, (b) expected values that come from outside the CPU code: 16-bit addition (`0x3FF*32+31 = 0x7FFF`), the sum 1..10 = 55, the GSWD bit layout printed on pdf 56 (C at bits 4 and 12, OV 5 and 13, Z 6 and 14, S 7 and 15), the stack direction (C17, resolved-primary), and the printed per-instruction cycle counts, (c) a regression anchor. The claim tier is `original-diagnostic`, standing provisional (it depends on `reset.r7`, `cartridge.slots` and the cartridge span C62, all tier4 or default-chosen). It is not accuracy evidence; D10 hardware is the only thing that would be.

Adversarial pass: "a hand listing will rot" (true past about 100 words; trigger above). "Same author on both sides" (true; mitigated by anchoring words to the transcribed family table and expected values to arithmetic; stated in the claim note). "libasm agreement would be stronger" (it checks encoding only, and costs a toolchain; kept optional). "A second reference interpreter in cmake" (about 120 lines duplicating CPU semantics for one 56-word program; rejected, revisit with trigger B).

## 3. Listing format and gate check (D-C2)

One decle per line, in a text file (`.txt`, so the content classifier treats it as text):

```
# comments, derivations
@5000                                  segment origin (hex)
5000 02B8 10 1011 1000  MVII  MVII #$03FF,R0     <- hex4, 10 bits in groups, row id, asm text
5001 03FF 11 1111 1111  -     imm                 <- operand word: row `-`
```

New check `tools/check/listing.cmake` (about 80 lines, copy of the two vector functions `program_word` and the leaf-row match; no dependency): per line the hex equals the bits (token `listing-bits-mismatch`); an instruction line's row is the leaf row of its decle in the family table (`listing-row-mismatch`); the following `words - 1` lines are `-` operand lines (`listing-operand-count`); every word is at most `03FF`, addresses are contiguous inside a segment, segments stay inside the file's declared window (EXEC `1000-1FFF`, cartridge the profile span) (`listing-range`). Stub-only rules are in D-C4. Inventory: `data/expected/listings.ids` (the two files). Negative controls under `tools/check/controls/`: one per token plus `listing-unlisted`. Cost [INF, unmeasured]: one `cmake -P` over about 90 lines, under 0.3 s.

## 4. Boot stub (D-24 contract, D-C4/C5/C6)

Contract v1 (everything else is not promised: R0-R5 and flags are whatever reset leaves; the diagnostic never relies on them, which keeps it independent of C32):

```
$1000 B $1010            200 00E     reset entry (R7 = reset.r7); skips the vector area. disp 0E: PC+2=1002, +0E
$1002 -- -- (zero, unreachable)
$1004 MVII #$0001,R0     2B8 001     interrupt entry (R7 = interrupt.iab_value): unexpected-interrupt trap
$1006 MVO R0,$035F       240 35F     records [035F]=1
$1008 HLT                000
$1009 TSTR R0            080         the follower that makes HLT stop (see section 1)
$1010 DIS                003         reset code
$1011 MVII #$0340,R6     2BE 340     stack in system RAM
$1013 J $5000            004 350 000 hand-off to the cartridge entry (BB=11, addr bits 15..10 = 010100)
```

- The cartridge entry is the fixed project convention `$5000` (the stub constant; the span `4000-FFFF` is itself a chosen default, C62). Not a hardware claim. It is one equate in the listing header.
- The trap at `$1004` is unreachable in Slice 0 (no device drives INTR). It is still tested: [OBS] a throwaway pulse of INTR after the J reached `$1004`, wrote `[035F]=1`, halted at `$100A` (interrupt entry 9 cycles, matching the derived figure). A held INTR level re-enters forever (INTR is always honored); a test must pulse, not hold. Phase 5/6 replaces the trap; each revision is a new contract number, new manifest note and a new golden.
- Source: `fixtures/FX-boot-stub/boot-stub.listing.txt`, manifest `FX-boot-stub` (kind `original-source`, committed, MIT, `author-attested`, redistributable yes, note "project boot stub, not an EXEC; never used as proof of EXEC compatibility"). The 8192-byte raw EXEC image (big-endian, zero tail) is built into the build tree by `tools/pack` and never committed.
- Label: the runner derives the label only from the symbolic name `boot-stub` in the scenario (not from file contents or a hash database, D-27) and prints `exec: project boot stub, not an EXEC sha256=<hex>`. A gate test greps that exact line in the runner output. Claims and the ledger say "project boot stub", never "EXEC".

Why it cannot drift into a high-level-emulated EXEC (D-C4), all gate-resident:
1. Size: the listing's addresses stay in `$1000-$101F` and non-zero words are at most 16 (`bootstub-size`).
2. Row allowlist `data/expected/bootstub-rows.ids`: HLT, DIS, EIS, MVII, MVO, TSTR, B, J. No JSR family, no MVI, no arithmetic, no indirect modes, so no callable routine, no table, no data read from GROM or cartridge (`bootstub-row`).
3. Profile binding: the check reads `reset.r7` and `interrupt.iab_value` from the profile JSON and requires a non-zero first word at each (`bootstub-vector`).
4. Path test (suite machine, both presets): after reset the R7 values at unit starts are `1000, 1010, 1011, 1013, 5000`, derived by hand; a pulse test covers the trap. Controls: a listing with a `JSR` line, one with 40 words, and one with a moved vector each fail for their own token.
5. Review rule in `docs/diagnostics.md`: the stub may initialise hardware to the profile's documented power-on state and hand off; it provides no entry point, service or data to a cartridge. Phase 6 and 7 changes to the stub must cite this rule.

GROM (D-C6) [INF]: the profile marks GROM required, so the runner synthesizes 2048 zero bytes, symbolic name `zero-grom`, labelled `grom: project stand-in (2048 zero bytes), not a GROM`. Nothing is committed or cached. The diagnostic never reads GROM; Phase 6 will need a real stand-in pattern. Alternative rejected: relaxing `grom.required` per profile (changes a Phase 3 matrix and weakens the refusal evidence).

## 5. The first diagnostic cartridge: `ivs-diag-cpu-1` (D-C7)

Source `fixtures/FX-diag-cpu-1/diag-cpu-1.listing.txt`; one IVSC v1 segment at `$5000`, 56 words, built as `diag-cpu-1.ivsc` in the build tree by `tools/pack`. It sets every register it uses before using it, uses no SDBD, no shifts, no interrupts, no GROM or STIC address. Sections:

- S1 flags: `R0=$3FF`, five `ADDR R0,R0` (doubling: `$7FE0`), `ADDI #$1F` gives `$7FFF` (store). `INCR` gives `$8000` with S only, OV and C unchanged, so `GSWD R1` = `$8080`. `DECR`, then `ADDI #1` gives `$8000` with S and OV: `GSWD R2` = `$A0A0`. `ADDR R0,R0` gives `$0000` with Z, C and OV: `GSWD R3` = `$7070`. This separates INCR (S, Z only) from ADD (all four).
- S2 loop: `R2=10`, `CLRR R3`; loop `ADDR R2,R3; DECR R2; BNZE back` (disp 3: PC+2=`5020`, target = `5020 + ~3` = `501C`). Sum 1..10 = `$0037`.
- S3 stack and indirect: `PSHR R3; PULR R5` round trip (stack `$0340`, grows up, C17), `MOVR R6,R0` stores the restored pointer; `MVI@ R4` from `$0200` with auto-increment (R4 ends `$0201`).
- End: `[020F]=$03C5` written last, then `HLT; TSTR R0 x3` (three followers make the predicate independent of the unresolved HLT and interruptibility policies: C40, C45, C56).

Words (address, word, asm); operand words follow their instruction:
`5000 02B8 MVII #$03FF,R0 | 5001 03FF | 5002-5006 00C0 x5 ADDR R0,R0 | 5007 02F8 ADDI #$001F,R0 | 5008 001F | 5009 0240 MVO R0,$0200 | 500A 0200 | 500B 0008 INCR R0 | 500C 0031 GSWD R1 | 500D 0241 MVO R1,$0201 | 500E 0201 | 500F 0010 DECR R0 | 5010 02F8 ADDI #$0001,R0 | 5011 0001 | 5012 0032 GSWD R2 | 5013 0242 MVO R2,$0202 | 5014 0202 | 5015 00C0 ADDR R0,R0 | 5016 0033 GSWD R3 | 5017 0243 MVO R3,$0203 | 5018 0203 | 5019 02BA MVII #$000A,R2 | 501A 000A | 501B 01DB CLRR R3 | 501C 00D3 ADDR R2,R3 | 501D 0012 DECR R2 | 501E 022C BNZE 501C | 501F 0003 | 5020 0243 MVO R3,$0204 | 5021 0204 | 5022 0273 PSHR R3 | 5023 02B5 PULR R5 | 5024 0245 MVO R5,$0205 | 5025 0205 | 5026 00B0 MOVR R6,R0 | 5027 0240 MVO R0,$0206 | 5028 0206 | 5029 02BC MVII #$0200,R4 | 502A 0200 | 502B 02A0 MVI@ R4,R0 | 502C 0244 MVO R4,$0207 | 502D 0207 | 502E 0240 MVO R0,$0208 | 502F 0208 | 5030 02B8 MVII #$03C5,R0 | 5031 03C5 | 5032 0240 MVO R0,$020F | 5033 020F | 5034 0000 HLT | 5035-5037 0080 x3 TSTR R0`

Predicate (hand-derived, written into the scenario, not generated by the core):
`[0200]=7FFF [0201]=8080 [0202]=A0A0 [0203]=7070 [0204]=0037 [0205]=0037 [0206]=0340 [0207]=0201 [0208]=7FFF [020F]=03C5`, registers `R0=03C5 R1=8080 R2=0000 R3=0037 R4=0201 R5=0037 R6=0340`, stop reason `halted`. Excluded on purpose: R7 and flags at the halt (policy dependent: HLT, interruptibility) and machine time.

Cycles [OBS]: hand sum of the printed counts (reset entry 5 [INF from pdf 27], stub 33, S1 134, loop setup 14, loop 208 = 10 x 12 + 9 taken x 9 + 1 not taken x 7, S3 and end 126 including HLT 4 and one follower 6) = 520 CPU cycles = 2080 ticks at entry to HALTED; the core gave 520 (+1 for the HALTED unit, 521 total, 2084 ticks). Whether a run stops at 2080 or 2084 is the run API's choice (decision point A); the golden records the choice. The golden stores time as regression data, not as an independent predicate, because the reset-entry 5 is an inference and no wait states exist in this profile.

Evidence and golden (RUN-05):
- `data/golden/diag-cpu-1.golden` (text; header lines `evidence: original-diagnostic (predicate), deterministic-repeatability (digests)`, `oracle: hand-derived from SRC-gi-cp1600-um-1975 pdf 55, 56 and arithmetic; digests are core output`, profile id, contract numbers). Body: input digests (stub EXEC raw, zero-grom, cart whole-file sha256 via `cmake -E sha256sum`, an implementation independent of the core), `stop`, `time_ticks`, per-domain digests cpu and mem plus composite, `video: absent (no STIC device in this phase)`, `audio: absent`, `input: empty schedule`. Do not hash "nothing" and call it a digest. A re-bless is an explicit command and a reviewed diff.
- Manifest `FX-diag-cpu-1` (kind `original-source`, committed listing, MIT, author-attested). The built `.ivsc` is identified by the golden's digest; no binary is committed or manifested (`FX-ivsc-vectors` precedent). [UNK] whether the manifests step wants a second `derived-output` record for build products; default is no.
- Ledger: new claim CL-0008, tier `original-diagnostic`, profile `intv-2609-ntsc-family-p1`, artifact the listing, depends_on `reset.r7`, `cartridge.slots`, provisional. Text: "a project boot stub and project-authored diagnostic reach the stated guest state"; no "runs Intellivision games" wording.

Scenario predicate encoding (the grammar the scenario carries, independent of file syntax chosen at point B): clauses `halted`, `reg R3=0037`, `mem 0200=7FFF`, `mem 0200..0203=7FFF,8080,A0A0,7070`, `digest cpu=<64 hex>`. All hex upper case, exact compare, evaluated at the stop. First failing clause is reported as `expected` versus `actual` (at most 64 asserted words are ever printed; never a range dump, so a user-supplied EXEC cannot leak through diagnostics). A failed clause is outcome `guest-failure`; a missing predicate is `harness-failure`.

## 6. Guest-state access (D-C8)

Options: (a) public read-only inspection, (b) runner-internal access through `core/src` headers, (c) digests only.
[REC] (a). Reasons: the installed consumer (PKG-01) is built from the public package only, so with (b) it could not assert guest state and RUN-06 would not be exercised through the installed path; (c) can only compare a golden, which is a regression check, not an independently derived predicate; Phase 6 needs side-effect-free inspection anyway. Emulator debuggers commonly separate a debug read from a bus read for this reason (side effects, wait states) [BG]. Keep it to two calls:

```c
typedef struct ivs_cpu_state { uint32_t struct_size; uint32_t run_state;   /* IVS_CPU_RUNNING/HALTED/FAULTED/UNRESET */
  uint32_t fault_reason; uint16_t r[8]; uint8_t s, z, ov, c, intff, reserved[3]; } ivs_cpu_state;
IVS_API ivs_status ivs_machine_cpu_state(const ivs_machine *m, ivs_cpu_state *out);
IVS_API ivs_status ivs_machine_peek(const ivs_machine *m, uint32_t addr, uint32_t count, uint16_t *out, uint32_t out_cap);
```

Rules: `peek` returns what a CPU read at that address would return now (decode, width mask, undriven fill, gates included), changes no state and takes no machine time; valid in OFF and POWERED; refusal order BAD_ARG (NULL, count 0), BAD_HANDLE, OUT_OF_RANGE (`addr+count` above `$10000`), BUFFER_TOO_SMALL (`out_cap < count`), all before any write to `out`; the out buffer must not overlap the instance (the Phase 3 rule). Gate pieces: rows in `data/api/api-effects.json` (the `api-effects` step then demands a test per refusal), tests in suite `machine`, one new named fault `PEEK_ADVANCES_TIME` (a peek moves `time_ticks`; must fail a state-digest-before-and-after test) and `PEEK_GATE_IGNORED`. The headers suite and the symbol allowlist gain the two names. Risk: public API growth; mitigated by pre-1.0 `struct_size` rules and keeping the surface at two calls. Stop reason and the CPU halted state also reach the host through `ivs_cpu_state.run_state`, so the run result need not duplicate it (decision point A).

## 7. Controls and tests (RUN-04) (D-C9)

Core faults already show the instruction rules are tested (`ADD_OVERFLOW`, `INCR_TOUCHES_C`, `GSWD_LAYOUT`, `BRANCH_BACKWARD_SIGN`, `PUSH_DIRECTION`, `AUTOINC_R4R5_MISSING`, `JUMP_TARGET_BITS` all sit on the diagnostic's path), but the fault registry allows an id in exactly one file, so the diagnostic cannot re-register them. Its own controls therefore mutate inputs, which is cheaper than a fault build (a run is about 520 cycles):
- Program mutations through `ivs_pack --patch ADDR=HEX`, listed in `data/expected/diag-mutations.ids` as `<ID> <addr> <word> <must-fail-clause>`: `ADDI-TO-SUBI` (5007 to `0338`, S1 words), `BNZE-TO-BZE` (501E to `022C`'s sibling `0224`, loop: sum word or timeout), `PSHR-TO-MVO@-R4` (5022, stack words), `INCR-TO-DECR` (500B to `0010`, `[0201]`). Each must end `guest-failure` naming that clause, not pass and not a harness failure.
- Outcome controls for RUN-02: wrong expected word gives `guest-failure`; run bound 100 cycles gives `timeout` with stop time reported; absent `--fixture-dir` file gives `missing-fixture`; edited golden gives `golden-mismatch`; malformed scenario gives `harness-failure`. Exit codes are distinct (point B).
- Gate cost [INF, unmeasured]: one tiny C tool build, about 12 runner invocations per preset, three cmake checks: about 1-2 s locally, which fits the 40 s hosted budget only if the cold build stays near 20 s; measure on the first plan (D-38 habit).
- Write all new shell for dash and cmake 3.25 (no `pushd`, no `[[`); run `sh tools/gate-linux.sh` before pushing C changes (two hosted-only failures in Phase 3).

## 8. Risks, reversal, unknowns

Top risks: (1) shared misreading between listing, expected values and core (bounded by section 2, claim stays `original-diagnostic`, provisional); (2) HLT and non-interruptible-follower semantics are policy (C40, C45, C56) and any stub or diagnostic ending relies on followers; (3) the `$5000` hand-off and `$0340` stack are project conventions that Phase 6 and 7 (STIC init, interrupts) will change, so stub versioning and the HLE guard must be enforced from now.
Would reverse: D4 to mini-assembler on triggers in section 2; peek to internal only if the owner prefers a minimal public API before Phase 6 (then PKG-01 asserts digests only); stub-in-core only if the owner wants a built-in image (a separately labelled HLE, D-24 reversal condition).
Blocking [UNK]: none for planning. Non-blocking: whether the manifests step needs a record for build products; run-time stop boundary (2080 versus 2084 ticks) is decision point A; the tier of the stub `$5000` entry convention (no source; default chosen, to be logged as a discrepancy next free number).
