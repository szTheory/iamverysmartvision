---
gsd_state_version: "1.0"
milestone: v0.2.0
milestone_name: Registered Evidence, Reachable Runs, Replayable Input
status: Awaiting next milestone
stopped_at: All four phases resolved - 9, 10 and 11 delivered and verified; 12 dropped at its entry gate
last_updated: "2026-11-25T17:27:31.209Z"
last_activity: 2026-11-25
last_activity_desc: Milestone v0.2.0 completed and archived
state_head: dea1c13
progress:
  total_phases: 4
  completed_phases: 4
  total_plans: 20
  completed_plans: 20
  percent: 100
current_phase: none
current_phase_name: none
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-11-24)

**Core value:** A deterministic, observable machine core whose every compatibility claim is bound to an exactly named profile and independent evidence — never an unqualified "it runs Intellivision games" label.
**Current focus:** Phase 11 — Input Record and Replay (complete)

## Current Position

Phase: Milestone v0.2.0 complete
Plan: —
Status: Awaiting next milestone
Last activity: 2026-11-25 — Milestone v0.2.0 completed and archived

### Planning overrides recorded by the autonomous run

| Gate | Result | Disposition |
|---|---|---|
| Decision coverage (`check.decision-coverage-plan`) | `could-not-parse` | **Proceeded.** The gate parses `D-NN` bullets; smart-discuss CONTEXT.md carries no ids by design, so the gate is inapplicable rather than unsatisfied. Verified by hand instead: amendments A-01 to A-09 are each cited by at least one plan (A-01→09-05, A-02→09-06/09-07, A-03/A-04→09-01, A-05→09-08, A-06→09-09, A-07→09-03, A-08→09-04..09-09, A-09→09-06) and every decision topic appears in a plan. No decision is dropped. Re-surfaced at verify-work. |
| Plan-checker iteration cap | Reached at 3, plus two guided passes | **Accepted after the fifth pass.** The loop found 24 real defects (a non-existent option table, a stale count baseline 31 files out of date, a stand-in that could not fire, a generator invocation that would have broken six controls, a regression that would have reclassified `runner-selftest` and made 09-08 unsatisfiable). Each fix was verified against the tree; the two final properties were proved directly by the orchestrator (`grep -e IVS_GATE_DIR -e 'build/' tools/check/steps/runner-selftest.sh` empty; the `memcmp`-before-`su_nine_digests` ordering awk exits 0 at HEAD). No open blocker. |

## Releases

| Version | Date | Commit | Assets | Notes |
|---------|------|--------|--------|-------|
| `v0.1.0` (pre-release) | 2026-11-24T05:02:08Z | `3fe5373` (the Phase 8 merge commit, the artifact commit) | `ivs-v0.1.0-source.tar.gz` (5,295,098 B), `ivs-v0.1.0-install-linux-x86_64.tar.gz` (83,468 B), `SHA256SUMS` (196 B) | Source-only; claims provisional and nothing measured; D10 and D11 not performed. Created as a draft by the tag-triggered workflow (run `45328905701`, `success`), read with `gh release view --json`, both archives verified against `SHA256SUMS` (`shasum -a 256 -c`, both OK), then published. No EXEC, GROM, ROM, manual or overlay of any kind. |

The release is a **pre-release**: the first 0.x release, published through the delegated flow (D9, D29). Deleting a published release remains an owner-only hard stop.

## Performance Metrics

**Velocity:**
- Total plans completed: 119
- Average duration: -
- Total execution time: -

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 1 | 14 | - | - |
| 2 | 18 | - | - |
| 3 | 14 | - | - |
| 4 | 27 | - | - |
| 5 | 12 | - | - |
| 6 | 14 | - | - |
| 9 | 9 | - | - |
| 10 | 6 | - | - |
| 11 | 5 | - | - |

*Updated after each plan completion*
**Per-Plan Metrics:**

| Plan | Duration | Tasks | Files |
|------|----------|-------|-------|
| Phase 1 P01 | 30min | 2 tasks | 90 files |
| Phase 1 P14 | 6min | 2 tasks | 1 files |
| Phase 1 P02 | 35min | 3 tasks | 1 files |
| Phase 01 P03 | 5min | 3 tasks | 1 files |
| Phase 01 P04 | 15min | 2 tasks | 18 files |
| Phase 1 P05 | 20min | 2 tasks | 26 files |
| Phase 1 P06 | 35min | 2 tasks | 60 files |
| Phase 1 P07 | 25min | 2 tasks | 41 files |
| Phase 1 P08 | 25min | 2 tasks | 24 files |
| Phase 1 P13 | 15min | 2 tasks | 12 files |
| Phase 1 P09 | 25min | 2 tasks | 15 files |
| Phase 1 P01-11 | 40min | 2 tasks | 16 files |
| Phase 1 P10 | 40min | 3 tasks | 11 files |
| Phase 1 P12 | 15min | 2 tasks | 1 files |
| Phase 02 P01 | 12 min | 3 tasks | 23 files |
| Phase 02 P02 | 14min | 3 tasks | 72 files |
| Phase 02 P03 | 6min | 2 tasks | 5 files |
| Phase 02 P04 | 15min | 2 tasks | 114 files |
| Phase 02 P05 | 16min | 2 tasks | 89 files |
| Phase 02 P06 | 8min | 2 tasks | 8 files |
| Phase 02 P07 | 14min | 3 tasks | 50 files |
| Phase 02 P08 | 19min | 3 tasks | 10 files |
| Phase 02 P09 | 13min | 3 tasks | 10 files |
| Phase 02 P10 | 12min | 3 tasks | 11 files |
| Phase 02 P11 | 9min | 3 tasks | 9 files |
| Phase 02 P12 | 10min | 2 tasks | 13 files |
| Phase 02 P13 | 18min | 3 tasks | 16 files |
| Phase 02 P14 | 18min | 3 tasks | 7 files |
| Phase 02 P15 | 15min | 3 tasks | 14 files |
| Phase 02 P16 | 15min | 3 tasks | 39 files |
| Phase 02 P17 | 7min | 3 tasks | 6 files |
| Phase 02 P18 | 15min | 3 tasks | 3 files |
| Phase 03 P01 | 8min | 2 tasks | 22 files |
| Phase 03 P02 | 15min | 3 tasks | 13 files |
| Phase 03 P03 | 8min | 3 tasks | 31 files |
| Phase 03 P04 | 15min | 3 tasks | 14 files |
| Phase 03 P05 | 8min | 2 tasks | 40 files |
| Phase 03 P06 | 12min | 3 tasks | 29 files |
| Phase 03 P07 | 15min | 3 tasks | 16 files |
| Phase 03 P08 | 14min | 3 tasks | 14 files |
| Phase 03 P09 | 16min | 3 tasks | 16 files |
| Phase 03 P10 | 12min | 3 tasks | 16 files |
| Phase 03 P11 | 20min | 3 tasks | 21 files |
| Phase 03 P12 | 15min | 3 tasks | 22 files |
| Phase 03 P13 | 10min | 3 tasks | 46 files |
| Phase 03 P14 | 12min | 3 tasks | 4 files |
| Phase 04 P01 | 45min | 3 tasks | 36 files |
| Phase 04 P02 | 15min | 2 tasks | 19 files |
| Phase 04 P03 | 40min | 2 tasks | 24 files |
| Phase 4 P04 | 30min | 2 tasks | 28 files |
| Phase 4 P05 | 45min | 2 tasks | 8 files |
| Phase 4 P06 | 30min | 2 tasks | 12 files |
| Phase 4 P07 | 55min | 3 tasks | 12 files |
| Phase 04 P08 | 20 min | 2 tasks | 20 files |
| Phase 04 P09 | 45 min | 2 tasks | 16 files |
| Phase 04 P10 | 40 min | 2 tasks | 14 files |
| Phase 04 P11 | 95 min | 3 tasks | 18 files |
| Phase 04 P12 | 35 min | 2 tasks | 13 files |
| Phase 04 P13 | 35 min | 3 tasks | 18 files |
| Phase 04 P14 | 16 min | 2 tasks | 14 files |
| Phase 04 P15 | 8 min | 3 tasks | 45 files |
| Phase 04 P16 | 15 min | 3 tasks | 21 files |
| Phase 04 P17 | 20 min | 3 tasks | 12 files |
| Phase 04 P18 | 19 min | 3 tasks | 10 files |
| Phase 04 P19 | 17 min | 3 tasks | 14 files |
| Phase 04 P20 | 14 min | 3 tasks | 17 files |
| Phase 04 P21 | 20 min | 3 tasks | 47 files |
| Phase 04 P22 | 15 min | 3 tasks | 9 files |
| Phase 4 P23 | 14min | 3 tasks | 8 files |
| Phase 04 P24 | 35 min | 3 tasks | 15 files |
| Phase 4 P25 | 40min | 3 tasks | 5 files |
| Phase 05 P01 | 40 min | 5 tasks | 118 files |
| Phase 5 P02 | 35 min | 4 tasks | 33 files |
| Phase 5 P03 | 70 min | 4 tasks | 21 files |
| Phase 5 P04 | 150 min | 5 tasks | 54 files |
| Phase 5 P05 | 6h | 5 tasks | 85 files |
| Phase 5 P06 | 5h | 5 tasks | 60 files |
| Phase 5 P07 | 4h | 5 tasks | 37 files |
| Phase 05 P08 | 45min | 5 tasks | 35 files |
| Phase 5 P09 | 3h | 5 tasks | 54 files |
| Phase 5 P10 | 4h | 5 tasks | 56 files |
| Phase 05 P11 | 40min | 5 tasks | 44 files |
| Phase 05 P12 | 55min | 4 tasks | 30 files |
| Phase 6 P01 | about 5 hours | 5 tasks | 96 files |
| Phase 6 P02 | 2h | 3 tasks | 2 files |
| Phase 06 P03 | 3h | 3 tasks | 22 files |
| Phase 06 P04 | about 4 hours | 4 tasks | 24 files |
| Phase 6 P05 | 3h | 4 tasks | 24 files |
| Phase 06 P06 | 9h | 5 tasks | 60 files |
| Phase 06 P07 | 2h | 4 tasks | 14 files |
| Phase 06 P08 | 8h | 5 tasks | 30 files |
| Phase 06 P09 | 6h | 5 tasks | 50 files |
| Phase 06 P10 | 2h | 4 tasks | 20 files |
| Phase 06 P11 | 180min | 3 tasks | 28 files |
| Phase 6 P12 | about 6 hours | 4 tasks | 40 files |
| Phase 6 P13 | about 3 hours | 5 tasks | 38 files |
| Phase 6 P14 | about 3 hours | 4 tasks | 22 files |
| Phase 7 P01 | 66 min | 3 tasks | 3 files |
| Phase 7 P02 | about 50 min measured, plus reading | 4 tasks | 47 files |
| Phase 7 P03 | about 32 min measured, plus reading | 3 tasks | 20 files |
| Phase 7 P04 | about 45 min measured to the last task commit, plus reading | 4 tasks | 41 files |
| Phase 7 P05 | about 1 h measured to the last task commit (a machine restart inside), plus reading | 4 tasks | 35 files |
| Phase 7 P06 | 25min | 3 tasks | 14 files |
| Phase 7 P07 | 40min | 4 tasks | 14 files |
| Phase 7 P08 | 40min | 3 tasks | 5 files |
| Phase 7 P09 | 35min | 3 tasks | 7 files |
| Phase 7 P10 | 32min | 4 tasks | 34 files |
| Phase 7 P11 | 38min | 3 tasks | 7 files |
| Phase 7 P12 | 58min | 3 tasks | 19 files |
| Phase 8 P01 | about 55 min measured, plus reading | 3 tasks | 42 files |
| Phase 8 P02 | 31min | 3 tasks | 16 files |
| Phase 08 P03 | about 3h | 3 tasks | 41 files |
| Phase 08 P04 | about 45 min | 3 tasks | 26 files |
| Phase 08 P05 | about 50 min | 3 tasks | 22 files |
| Phase 08 P06 | about 45 min | 3 tasks | 19 files |
| Phase 08 P07 | about 75 min | 3 tasks | 35 files |
| Phase 08 P08 | about 55 min | 3 tasks | 25 files |
| Phase 8 P09 | about 50 min | 3 tasks | 37 files |
| Phase 8 P10 | about 80 min | 3 tasks | 17 files |
| Phase 8 P11 | about 70 min | 3 tasks | 28 files |
| Phase 8 P12 | about 55 min | 2 tasks | 17 files |
| Phase 8 P13 | about 60 min | 3 tasks | 25 files |
| Phase 8 P14 | about 150 min | 3 tasks | 22 files |
| Phase 8 P15 | about 65 min | 3 tasks | 10 files |
| Phase 8 P16 | about 75 min | 3 tasks | 17 files |
| Phase 8 P17 | about 40 min | 3 tasks | 30 files |
| Phase 8 P18 | about 120 min | 3 tasks | 22 files |
| Phase 8 P19 | about 150 min (two sessions, the baseline input taken between them) | 3 tasks | 22 files |
| Phase 8 P20 | about 45 min | 3 tasks | 51 files |
| Phase 8 P21 | about 65 min (estimated: no start time was taken) | 3 tasks | 10 files |
| Phase 09 P03 | 15m | 2 tasks | 2 files |
| Phase 09 P04 | ~70 min | 3 tasks | 13 files |
| Phase 9 P05 | 47m | 2 tasks | 11 files |
| Phase 9 P6 | 41m | 2 tasks | 11 files |
| Phase 09 P07 | 13 | 3 tasks | 11 files |
| Phase 09 P08 | 32m | 3 tasks | 3 files |
| Phase 9 P09 | 22m | 3 tasks | 8 files |
| Phase 10 P01 | 7m40s | 2 tasks | 3 files |
| Phase 10 P02 | ~28m | 2 tasks | 1 files |
| Phase 10 P03 | 24 min | 2 tasks | 2 files |
| Phase 10 P04 | 18m | 2 tasks | 3 files |
| Phase 10 P6 | ~55 min wall clock | 3 tasks | 12 files |
| Phase 11 P01 | 14m | 2 tasks | 1 files |
| Phase 11 P02 | 27m | 3 tasks | 8 files |
| Phase 11 P03 | 24m | 3 tasks | 2 files |
| Phase 11 P4 | 1h 02m | 2 tasks | 4 files |
| Phase 11 P5 | 55m | 3 tasks | 20 files |

## Accumulated Context

### Decisions

Decisions live in the PROJECT.md Decision Register (owner decisions /
recommendations / GSD defaults / open questions are kept separate).

- [Init]: Roadmap order follows research (foundations → CPU → memory/refusal → Slice 0 runner+consumer → controllers+PSG → STIC first pass → visible scenario → STIC completion+state+perf). GSD default, reversible.
- [Init]: Visible scenario (Phase 7) placed directly after the first STIC pass, ahead of full STIC coverage (conflict C26). GSD default, reversible.
- [Init]: Variant profiles and peripherals (the research summary's proposed variants stage) are v2 and are not in this roadmap.
- [Phase 1]: 01-01: gate summary steps=<passed>/<expected>; vocab step prints its own pass line; duplicate JSON keys stay a documented undetectable limit
- [Phase 1]: 01-14: sources registry keeps community wiki and docs tree reference-only (no stable single-document hash); Blue Sky page re-hash matches research
- [Phase 1]: C17 recorded resolved-primary from page-image reading of pdf 24, 46, 48, 62 (01-02)
- [Phase 1]: Interrupt-entry (9 microcycles) and reset sequence counts in CP-1610 notes are inferred from timing-diagram time slots, not printed tables (01-02)
- [Phase 01]: 01-03: opcode family table has 74 rows (below D-19's about 90: SDBD as attributes, aliases on one row, one jump row); NOPP transcribed as printed 7 cycles not 7/9; interruptible is no only where the page prints it, else manual-silent
- [Phase 01]: 01-04: C17 resolved-primary, C19 partial; notes candidates became C29-C35 (device-supplied reset PC is C32); log holds 35 entries
- [Phase 01]: 01-04: open/partial default-policy prefix is checked in the discrepancy hook because the records DSL cannot hold a space in a regex
- [Phase 1]: Claim ids are CL-NNNN, strictly ascending; only CL-0002 is established, claims resting on tier4-community, inference or unknown-default-chosen fields stay provisional
- [Phase 1]: text step exempts only the literal placeholder CL-NNNN from claim-reference resolution
- [Phase 1]: 01-06: files step reports tracked=<n> (integrity already owns files=<n>); unknown derived_from ids and cycles resolve to effective-no
- [Phase 1]: 01-07: denylist stat named scanned= (not files=) to avoid clashing with the integrity step key; reference-only registry entries must carry null digests and a note mentioning hashing
- [Phase 1]: 01-08: D-11 unit-suffix rule applies to duration/offset profile keys; ticks_per_X and X_per_Y ratio keys name their units and comply; enforced by the glossary step
- [Phase 1]: 01-08: glossary success line uses doc-files= (text step already emits docs=)
- [Phase 1]: 01-09: cite and sdbd regexes checked in the cp1610-row hook (DSL cannot hold spaces); no cycle value required so NOPP 7 passes; cp1610 step measured 0.7 s so decode stays in cmake -P
- [Phase 1]: 01-11: hygiene success line uses key hygiene-files (not files) to avoid a duplicate gate stat key; allow patterns anchored on grep -o output
- [Phase 1]: 01-11: cmake line budget measured 2154 vs plan limit 1500; not shrunk, recorded as deviation for owner/orchestrator
- [Phase 1]: 01-10: page-internal figure-vs-table differences (GSWD, RSWD, XOR@ printed form) are recorded as resolved second-reader entries; the family table is unchanged
- [Phase 1]: 01-12: single CI job, no cache/matrix/path filter; macOS leg with the C build; revisit ubuntu-24.04 pin after 2026-11-19
- [Phase 02]: C58 (JSR return address) resolved-primary: PC+3 legible on pdf 56; A-06 gap mapping G1-G6 = C46-C51, G7 = C37, G8 = C58
- [Phase 02]: Profile flag fields cpu.interrupt_accept_clears_intff and cpu.ebci_default are JSON numbers (0 = false); the schema has no boolean value type
- [Phase 02]: 02-02: the CPU access log carries read data and the applied wait; the bus port sees the request (wait 0, read data 0)
- [Phase 02]: 02-02: a wait above wait_limit_cpu_cycles abandons the unit with sticky BUS_WAIT_LIMIT, elapsed = offset + 1 + wait (inference)
- [Phase 02]: 02-02: step refuses lines outside 0/1 and an invalid config (IVS_E_BAD_ARG)
- [Phase 02]: 02-02: vector units and events asserted only when listed; full state and 64K memory always asserted
- [Phase 02]: 02-03: harness self-test compiles ivs_test.c directly and links only ivs_warnings and ivs_san, so it cannot depend on core or test-bus code
- [Phase 02]: 02-03: refuse-small-log covers log_cap 0..7 (0 is below the 8-record minimum); log_cap 8 is the accepted control
- [Phase 02]: 02-03: bus-order-matches-log compares kind, flags, addr, offset between port call and log record; data only for WRITE/DW; reads must show port data 0 and wait 0
- [Phase 02]: 02-04: Fault executables (tests/faults.cmake) recompile only the owning TU with IVS_FAULT_<ID> and link plain core and harness copies with ivs_warnings only; c-faults runs them, faults-verdict judges them as a pure checker
- [Phase 02]: 02-04: vector-program-mismatch compares a program word's binary with the whole hex word (generator rule), not only the low 10 bits
- [Phase 02]: 02-04: vector-row-mismatch accepts only leaf rows for a decle; the 0BF TSTR/JR coincidence accepts TSTR or JR, never MOVR (plan 02-05 shows the MOVR R7 bonus with SSS != 7)
- [Phase 02]: 02-04: mame lives only in tools/check/vector-emulator-names.txt; widening the shared denylist-cite.txt stays open for plan 02-18 (6 in-scope lines today)
- [Phase 02]: cp1610-vectors counts a coverage tag only after the vector's own data proves it; a row or policy leaves cp1610-coverage-pending.ids only with all its derived needs (flags, cycles, required addr/branch tags, bus, and for policies a fault naming them)
- [Phase 02]: MVI@ cycles:mod is witnessed by a single-step PULR cycles:base vector of 11 cycles (cp1610-modifier-witness.ids, A-13); the branch direction bit is bit 5 (S on pdf 54, meaning on pdf 47)
- [Phase 02]: Strobes are access records through the bus port (addr = data = 0): STROBE_TCI at elapsed_cpu_cycles + 1 (pdf 33), STROBE_PCIT at cycle 4 of SIN (pdf 34); HLT reports HALT_LATCHED when it sets halt_pending
- [Phase 02]: core-symbols external= counts imports not defined inside the input; c-purity reports the -O2 count as core-imports (2: __stack_chk_fail, __stack_chk_guard)
- [Phase 02]: c-purity source layer is a C preprocessing-number tokenizer (comments/strings/char literals removed); float-literal rule also scans core headers; float/double words .c-only
- [Phase 02]: core-purity-lto is a purity-selftest case, not an overlay control: c-purity forces -fno-lto (D-21 allows the drop since the class stays covered)
- [Phase 02]: Sanitizer prefixes reach the core-symbols ctest only via IVS_SYMBOL_PREFIX_ARGS when IVS_SAN is ON; release archive gets the strict check
- [Phase 02]: 02-08: The decode table holds all 74 rows, most specific first; rows without semantics carry OP_NONE and report UNIMPLEMENTED; decle $0BF decodes to TSTR (TSTR/JR/MOVR give the same result)
- [Phase 02]: 02-08: A register read of R7 sees the PC incremented by the fetch (inference, pdf 28, POL-STK-01); every register-group write goes through ivs_wreg, where an R7 write overrides the PC
- [Phase 02]: 02-08: The MOVR/TSTR/JR coincidence decle is $0BF, not $2FF as plan text says; the vector is cpu.MOVR.coincidence-0bf and plan 02-13 should rename its high-bits vector to match
- [Phase 02]: Immediate reads/writes are figure-basis bus records at offset 4 (pdf 29 read, pdf 31 write) with EA = PC+1 (Fig 3.2.5, pdf 49)
- [Phase 02]: S after right shifts and SWAP is bit 7 (C47); RLC/RRC N=1 use the C48 18-bit link strings with OV as input; N=0 forms keep OV
- [Phase 02]: SARC N=1 takes the RRC link layout (bit 0 to C, bit 1 to OV): inference, the page names no positions
- [Phase 02]: 02-10: direct reads record FETCH, READ of the address word at 4 and ADAR of the effective address at 6 (pdf 30, pdf 11); direct MVO writes at the ADAR phase (inference)
- [Phase 02]: 02-10: pull reads (PULR, @ rows with MMM=6) put the READ at offset 7, after the 3 extra cycles (inference, C36)
- [Phase 02]: 02-10: POL-STK-02 pointer moves before the operation; result wins; ADD@ R4,R4 adds the word to R4+1
- [Phase 02]: 02-10: POL-MVO-01 refuses $264 $26D $276 $27F as sticky UNSUPPORTED_MVO_SSS_EQ_MMM before any effect
- [Phase 02]: 02-11: branch condition = pdf 54 test of bits 2..0 xor bit 3 (negate); NOPP (1000) is B negated and never branches
- [Phase 02]: 02-11: branch displacement read through ivs_imm_read (one PC-advance site); backward target = PC+2 + 16-bit one's complement (C50); taken adds 2 to the base 7 (POL-BR-01)
- [Phase 02]: 02-11: branch bus lists are FETCH at 0 and the displacement READ at 4, figure basis pdf 32; nothing after the displacement BAR is listed
- [Phase 02]: 02-12: BEXT asks port->ebci(ctx, decle & $F) after the displacement READ (pdf 11, pdf 32); NULL provider is false (C57); vector ebci key absent = NULL provider, present (even empty) = provider via has_ebci
- [Phase 02]: 02-12: Jump target = bits 7..2 of word 2 << 10 | bits 9..0 of word 3 (pdf 52); JSR saves R7 after both operand reads (PC+3, C58) in R4+BB; DE 01/10 set/clear INTFF; bus READ at 4 and 8 is inference (C36)
- [Phase 02]: 02-12: Jump DE=11 stops after word 2 with R7 at the offender, 8 cycles, temporary UNIMPLEMENTED (02-13 renames); cpu.J.target-ffff uses JSR R4 so JUMP_TARGET_BITS can fail
- [Phase 02]: 02-13: SDBD double byte data is read inside the shared immediate and indirect readers; every other target after SDBD is refused with UNSUPPORTED_SDBD_TARGET before execution (SDBD check before POL-MVO-01); the high byte READ offset 7 is an inference (C36)
- [Phase 02]: 02-13: The temporary unimplemented fault reason and OP_NONE are removed; jump DE = 11 faults UNSUPPORTED_JUMP_DE; FAULT_LAST bounds validation
- [Phase 02]: 02-13: Suite cpu-sweep compares every decle (with and without DBD) and all 65536 words against the Phase 1 family table via cmake/IvsSweepTable.cmake; no readings differed
- [Phase 02]: 02-14: Interrupt acceptance runs at the start of a RUNNING step (prev_interruptible open, DBD clear; INTR, or INTRM with INTFF; levels never latched); the INT_ENTRY unit is INTAK@0 (R6), DW@2 of R7, R6+1, IAB@5 into R7, 9 cycles, prev_interruptible 0 after it
- [Phase 02]: 02-14: One IAB helper serves reset and interrupt entries (no vector constant); acceptance leaves INTFF unless config interrupt_accept_clears_intff = 1; all 39 interrupt vectors are kind policy
- [Phase 02]: 02-14: Fault prefixes grow only by exact ids: MVO_INTERRUPTIBLE (row-flag site) also fails 12 existing MVO-family vectors, JE_AFTER_BOUNDARY also fails cpu.J.je-sets-intff and cpu.J.jsre-r5; LINES_LATCHED uses a fault-only static latch, which 02-15 HLT vectors holding INTR may leave set
- [Phase 02]: 02-15: Step start order is STPST edge (toggle in RUNNING, resume in HALTED), then a held grant (hold 1 cycle / release = config latency), then at an interruptible boundary stop > BUSRQ > INTR > INTRM; grants never change prev_interruptible (A-02)
- [Phase 02]: 02-15: HALTED units are 1 cycle and ignore INTR/INTRM while BUSRQ is granted and released; the stop entry reports STOPPED; a STPST edge does not report HALT_LATCHED
- [Phase 02]: 02-15: The BUS_WAIT_LIMIT fault unit ends at access offset + 1 + limit + 1 (bounded by config, not the device reply); release latency 0 is refused as a bad config
- [Phase 02]: 02-16: blinded vector copies use opaque v-indices and opaque policy tokens p01..p19 (sha256 order); the gate projects all 12 files every run with a leak scan; the last HANDOFF line for a stem is authoritative
- [Phase 02]: 02-17: all 30 second-reader disagreements resolved for the committed value from the page (pdf 23/30 ADAR labels, pdf 33 TCI order, pdf 48 increment order, pdf 58 SDBD pair total kept under open C34); no vector or core change; second-read=12/12
- [Phase 02]: 02-17: reader protocol and p07 policy statement completed where their wording caused disagreements (bus kind meanings, per-unit record order, increment before operation)
- [Phase 02]: 02-18: D-22 lever 2 applied (4-lane control runner, byte-identical output, runner 73.0 s -> 32.3 s under load); levers 1 and 3 not applied (warm c-build 0.5 s, c-faults 3.2 s); host never idle so the 25 s local budget stays unjudged
- [Phase 02]: 02-18: hosted gate budget (30 s) stays an open item for the final 190-control gate; D-24 supersedes the CI-EFFICIENCY macOS-leg line; C43 independent re-adjudication of the 30 second-read disagreements recommended
- [Phase 03]: 03-01: fault suites registered once in tests/CMakeLists.txt (ivs_fault_suite, ivs_fault_gen); faults .ids files stay pure inventory with a suite header
- [Phase 03]: 03-01: site faults of a suite share one plain object library ivs_fault_suite_obj_<suite> (W7 lever applied)
- [Phase 03]: 03-01: fault ids unique across all faults files (also within a file); site checks use the union of non-GEN_ ids (H3)
- [Phase 03]: 03-02: SHA-256 context has an explicit reserved word: 112 bytes, no implicit padding, assignment is a snapshot (A-04)
- [Phase 03]: 03-02: IvsShaVectors re-derives every vectors.txt line, including the 1,000,000 x a rep line, with string(SHA256); build fails closed on mismatch
- [Phase 03]: 03-02: c-faults runs fault executables in at most 4 lanes: projected phase-end c-faults 9.4 s serial exceeded the 8 s A-07 trigger
- [Phase 03]: 03-03: ivs_core exports only core/include; tests reach core/src through the interface target ivs_core_internal (D-06)
- [Phase 03]: 03-03: a generated core include is accepted only via data/expected/core-generated.ids (read from ROOT); c-purity runs the generator, scans and compiles its output (A-06)
- [Phase 03]: 03-03: writable-data rule (nm b B d D c C) runs only on c-purity's own -fno-pie archives; ctest core-symbols stays imports-only (D-07)
- [Phase 03]: 03-04: the only mutable decode state is a committed segment table of at most 16 {addr, len_words} entries scanned by a bounded loop; it replaces D-20's per-256-word page table because A-01 allows one-word segments at any address (A-08 c)
- [Phase 03]: 03-04: ivs_memsys backing arrays hold 16-bit words for every device; install and raw-poke store whole words and the width formula masks on read (A-05a)
- [Phase 03]: 03-04: a region's physical device is its name up to the first underscore; regions sharing one share its backing and must agree on size_words and device kind
- [Phase 03]: 03-04: write-only alias regions carry conflict C61; C21 stays on stic_regs, stic_alias_read and gram
- [Phase 03]: 03-04: GEN faults on a core-consumed table use ivs_fault_gen CONSUMER core:<src>; an -DMUTATE= in ARGS replaces the default must-fail id
- [Phase 03]: 03-05: literal map mirror column = backing words when the row is an alias or exceeds its backing, - for the sole full-extent home of a device; write-view width = bits a store keeps (0 when dropped)
- [Phase 03]: 03-05: memmap step adds memmap-literal-unknown-tag and memmap-literal-orphan with controls (controls=212; plan 03-06 expects 215); tier-5 refused by tag and by source authority tier
- [Phase 03]: 03-06: profile alias gates follow the literal (STIC aliases stic-regs, GRAM aliases gmem; inference, C61); describe reports the shared backing as an alias's mirror
- [Phase 03]: 03-06: MEM-01/MEM-02 complete by gate evidence (decode dump equals literal in dev and release; row probes at every literal boundary; faults 84); CL-0006 provisional
- [Phase 03]: 03-07: IVSC v1 reasons have stable numbers 1..18, each mapped to one status (spec table in docs/formats/ivsc-v1.md); the parser checks the table size before reading any entry so a lying segment_count cannot over-read
- [Phase 03]: 03-07: long IVSC fixtures (P05, B07, B10, B12) are JSON build records with typed cmake -E sha256sum trailers; any data/cart/*.json edit refreshes FX-ivsc-vectors via tools/cart/manifest-outputs.sh in the same commit
- [Phase 03]: 03-07: SHA fault prefixes not widened (sha256 faults run suite machine only); the loader plan 03-11 still owes the widening; MEM-05/MEM-04 stay open until 03-08/03-11
- [Phase 03]: 03-08: IVSC_TRUST_TOTAL_BYTES drops only the table-smaller half of the implied-size check (memory-safe in the plain fault build); new fixture ivsc.N34-table-short pins that half, which no fixed fixture pinned before
- [Phase 03]: 03-08: IVSC_COUNT_17 lets count 17 pass the count rule and reads the table as 16 entries so the plain fault build stays in bounds
- [Phase 03]: 03-08: CL-0007 is source-expectation and provisional (cartridge.slots unknown-default-chosen); IVSC conformance is to the project's own spec
- [Phase 03]: 03-08: optional libFuzzer target sanitizer list is the cache string IVS_LIBFUZZER_SANITIZERS (default fuzzer,address,undefined; fuzzer,undefined where the ASan runtime hangs)
- [Phase 03]: 03-09: create zeroes exactly the instance bytes (113,456; 16-aligned); calls taking a handle check BAD_ARG before BAD_HANDLE; create order BAD_ARG, UNSUPPORTED, UNKNOWN_PROFILE, BUFFER_TOO_SMALL
- [Phase 03]: 03-09: state digest = per-domain SHA-256 from one 49-row field table (u32 count before arrays, profile index serialized as the id, magic/padding declared not serialized) and the IVSSTATE composite; SHA faults K_WRONG/ROTATE_WRONG/LENGTH_LE widened to machine.digest.
- [Phase 03]: 03-10: load ladder per A-08 (BAD_ARG, BAD_HANDLE, UNSUPPORTED options, BAD_STATE, DUPLICATE_LOAD, OVERSIZED, wrap-proof overlap BAD_ARG, content, expect DIGEST_MISMATCH); refused report writes detail only, image zero
- [Phase 03]: 03-10: IVS_REASON_* 1..18 equal the IVSC reason table, 19..33 are load reasons (span, features, system-image size/width, expect digest, argument, handle, option, state, slot)
- [Phase 03]: 03-10: parser computes trailer check and whole-file SHA-256 in one pass (view.file_sha256); layer-2 span check is the internal seam ivs_machine_cart_span_check
- [Phase 03]: 03-10: no requirement marked complete; MEM-03 system-image shape refusals are tested by 03-11
- [Phase 03]: 03-11: out parameters (load report, info out, digest out, create out) overlapping the instance storage are refused BAD_ARG with nothing written (orchestrator addition; fault REPORT_INSIDE_UNCHECKED); faults now 110
- [Phase 03]: 03-11: refusal rows run per ladder stage: argument BAD_ARG/UNSUPPORTED on priors f,s,c; later classes on f,x; DUPLICATE_LOAD only on occupied slots; lying lengths in heap and inside placements
- [Phase 03]: 03-11: load-level cartridge fixtures name segments with span-relative addresses resolved from the literal map; MEM-03 and MEM-05 complete, MEM-04 waits for 03-12 reset rows, API-06 for 03-13
- [Phase 03]: 03-12: machine time stays in master ticks: the step seam adds elapsed CPU cycles times the registry's ticks_per_cpu_cycle (new registry field in the former reserved u16)
- [Phase 03]: 03-12: reset checks BAD_ARG (kind), BAD_HANDLE, then MISSING_SYSTEM_IMAGE (EXEC before GROM) before any write; power cycle from POWERED; nothing returns to OFF (A-08 d)
- [Phase 03]: 03-12: one shared out-parameter check (ivs_machine_out_overlaps) for create, info and state_digest, guarded by fault OUT_OVERLAP_UNCHECKED
- [Phase 03]: 03-13: data/api/api-effects.json is the D-06 symbol list and D-15 effect table (10 calls, 27 rows, effect none); step api-effects checks it against ivs_status.h, api-statuses.ids, IVS_API lines and the test inventories; OQ-5 residual kept (cited id exists, not that it asserts the status)
- [Phase 03]: 03-13: ninth token/control api-effects-format for table shape; controls=224/224
- [Phase 03]: D5 recorded as decided by D-24 in reset.r7, C32 and C25; hardware read-back stays D10, boot stub stays Phase 4
- [Phase 03]: Hosted gate budget re-baselined 30 s -> 40 s, revisit trigger 50 s (D-38, A-07); no lever applied at phase end
- [Phase 04]: 04-01: gate.sh prints gate-env/gate-cpu (POSIX times via 'command times' in the main shell after the controls wait); job summary only through the EXIT-trap helper tools/check/ci-summary.sh; ci-parity bans any action but actions/checkout, secrets, strategy, runs-on other than ubuntu-24.04 and a gate timeout other than 5
- [Phase 04]: 04-02: ci-record.sh converts ISO times with awk civil-date arithmetic in the shared parser (offline --from-files tests it); ci-log also requires digit run ids and 40-hex shas
- [Phase 04]: 04-02: PKG-04 stays Pending until hosted rows exist in .planning/ci-runs.tsv; orchestrator runs tools/ci-record.sh after each hosted run and evaluates D-45 after 04-10 and 04-19
- [Phase 04]: 04-03: Fault owner TUs are repo-relative (core/ and runner/); file stems unique across both, checked at configure (D-24)
- [Phase 04]: 04-03: Runner outcome values equal their exit codes; F_WALL 0x100 separates wall from budget timeout; non-outcome values map to harness-fail 14
- [Phase 4]: 04-04: runner-scan confines POSIX headers and feature macros to runner/hostio.c (also refusing feature macros as runner CMake compile definitions); no runner file, hostio.c included, may include core internals
- [Phase 4]: 04-05: observation rows (obs.video/audio/input, 144 bytes) sit before slot[] after an explicit obs_pad word; instance 113,600 bytes with no tail padding (asserted); domains 16..18 live, never in the version 1 composite over 1..4
- [Phase 4]: 04-06: ivs_cpu_state_report is 36 bytes; IVS_CPU_STATE_*/IVS_CPU_FAULT_* mirror the CPU internals, pinned by static asserts; observation calls use ivs_machine_span_overlaps directly; run/observation tests live in suite run
- [Phase 4]: 04-07: ivs_machine_run loop tests fault then halt (flag) before each unit; any zero-progress unit ends the loop (INTERNAL unless FAULTED); refusal reasons IVS_REASON_RUN_* 34..40 (BAD_HANDLE reuses IVS_REASON_HANDLE); refusal detail never written into the bytes the handle names
- [Phase 04]: 04-08: generic define pin in api-effects (class table PREFIX|PIN|FORM|EXCL); IVS_STOP_ and IVS_REASON_ pinned; a missing pin file or an empty class fails closed as api-define-unpinned
- [Phase 04]: 04-09: ivs_info grows to 256 bytes (timebase, nominal_frame_ticks, output layout NONE/0); minimum struct_size stays the v1 size 216 (IVS_INFO_V1_SIZE), copy is min(struct_size, sizeof) (A-02); the slicing reference is seam-stepped so a run-path fault cannot move its own oracle (D-07)
- [Phase 04]: 04-10: ivs_machine_run checks caller-owned output buffers in the D-05 slots (BAD_ARG after UNSUPPORTED, BUFFER_TOO_SMALL after OUT_OF_RANGE) and wraps the internal env seam ivs_machine_run_env_internal with the profile layout; a report overlapping a buffer gets no refusal detail
- [Phase 04]: 04-11: the input floor is set by the step seam when the CPU accepts a unit, so run-stepped and seam-stepped twins stay byte-identical
- [Phase 04]: 04-11: per-event validation is two whole-array passes after BUFFER_TOO_SMALL (BAD_ARG order/range/floor, then UNSUPPORTED content); lowest failing index reported
- [Phase 04]: 04-11: an event_count above 65536 names no span for the report rule (found by gate-linux under gcc)
- [Phase 04]: 04-12: input.controllers (profile, tier2-first-party-adjacent, SRC-bsr-2609, C14, D6) = 2; a nonzero controller word at or above it is UNSUPPORTED (IVS_REASON_INPUT_EXTRA_CONTROLLER 54), last check per event in pass 2
- [Phase 04]: 04-13: IVS_CTRL_ bits are pinned by bit index through a new bit value form ((1u << n) read as n) of the generic define pin
- [Phase 04]: 04-13: v1 sizes frozen (request 64 on LP64, report 112, event 32, cpu_state_report 36, ivs_info 216/256); A-02 applies to them from here
- [Phase 04]: 04-13: C14 stays open with the D6 policy; C63-C67 record the D10 hardware questions and the unit-start granularity; next discrepancy id C68
- [Phase 04]: 04-14: stub listing zero fill carries row HLT with text fill ('-' only for operand words); bit groups are exactly 10 digits; header keys are '# key: value' lines
- [Phase 04]: 04-14: tools/pack writes images only to build/<preset>/images (IVS_IMAGES_DIR); the C include of image bytes is generated in tests/CMakeLists.txt and is GENERATED for the run fault suite
- [Phase 04]: listing step copies the vector checker's row loader, decle_leaves and program_word; operand count from the family table's words column; SDBD not modelled (fails closed)
- [Phase 04]: suite pack: ivs_pack --dump-words vs listing.cmake -DDUMP parse, byte for byte; never the core
- [Phase 04]: 04-16: the diag-cpu-1 predicate (data/golden/diag-cpu-1.expect) was derived by hand and committed before the program first ran; run.diag.predicate reads it through a generated include (tests/expect-inc.cmake)
- [Phase 04]: 04-16: run.diag.slice reuses family slice's oracle via ivs_run_slice_halting; it also detects RUN_STOP_BEFORE_OVERSHOOT, RUN_BUDGET_CARRY and INPUT_STATIC_CARRY (allowed collateral)
- [Phase 04]: 04-16: CL-0008 provisional (original-diagnostic); C68 records the stub's $5000 entry and $0340 stack as project conventions; next ids C69 and CL-0009
- [Phase 04]: 04-17: IVSS v1 parser in runner/scenario.c (pure, fixed-size, 42 reason tokens, line 0 = whole file); id and profile required; one unit for all run-to lines and one for all input lines; inputs nondecreasing, run-to strictly increasing, times at most 2^62
- [Phase 04]: 04-17: mem A..B asserts mean every word of the inclusive range equals the value; digest domains by name (composite cpu mem images instance video audio input), 64 lower-case hex
- [Phase 04]: 04-17: -P cmake scripts must start with cmake_minimum_required(VERSION 3.25): cmake 3.28 runs them with CMP0054 OLD and a quoted string in if() can dereference a same-named variable (found by gate-linux)
- [Phase 04]: 04-18: the executor runs every run-to with IVS_RUN_F_STOP_ON_HALT; halted where target was expected is guest-fail, the bound reached where halted/faulted was expected is timeout budget, IVS_STOP_INTERNAL is harness-fail (ivs_runner_stop_observation, pure)
- [Phase 04]: 04-18: report key names pinned (slot.*, exec:/grom: labels, step.<k>.*, step.<k>.assert.<j> with scenario-wide j, cpu.*, input.*, digest.<8 domains>, output.*, host.calls, report.end); host.calls counts read_file/read_user/now_ms/cancel_requested and is the only host-dependent key
- [Phase 04]: 04-18: host vtable read_file(relpath, cap, buf[cap+1]) plus optional read_user(role) for user roles; a matching expect-load refusal ends as pass, a wrong reason or an unrefused load is harness-fail
- [Phase 4]: 04-19: every run-to runs as slices (min(limit, now + slice_ticks), default 1<<22) with the unconsumed input suffix; cancel_requested then now_ms once before every slice; ivs_runner_options {flags, wall_timeout_ms, slice_ticks, max_ticks}, 0 = default
- [Phase 4]: 04-19: --max-ticks (default 2^40) cutting a run-to is timeout budget (detail max-ticks=<m>); the wall cap is timeout 'wall step=<k> cap-ms=<n>' (never golden); scenario wall-timeout-ms only lowers; cancel+wall at one check resolves to cancelled
- [Phase 4]: 04-19: ivs-run: POSIX only in runner/hostio.c (+hostio.h); --content-dir defaults to the scenario's directory; usage errors, unreadable or refused scenarios are exit 14 with no report; --self-signal-after-slices N raises SIGINT in the cancel check after slice N
- [Phase 04]: 04-20: scenario expectations beyond the six inventory columns live in the scenario as # expect-detail: / # expect-stderr: comments; the judge pins outcome.detail exactly and requires each outcome/exit.code pair in data/expected/runner-outcomes.ids
- [Phase 04]: 04-20: D-32 mutants are <FROM>-<TO> ids checked against the listing row; PSHR to MVO@ is MVO@ R3 via R1 (R1=8080), so R6 stays 0340
- [Phase 04]: 04-21: golden key list (scenario.id/profile, outcome, detail, exit.code, slot.*, step.*, cpu.*, input.*, digest.cpu/mem/images/instance/composite, output counts, live-domain digests only when observed); written by ivs-run --update-golden in C and re-derived by the golden step in awk
- [Phase 04]: 04-21: step golden judges dev vs release byte equality apart from host.calls, excluding host-dependent ids (column 4); a live domain is observed only when its report count is nonzero, else none with no digest line
- [Phase 04]: 04-22: archive equality is whole-file sha256 on Linux, member-wise on Darwin (CMake re-runs ranlib at install); pass line archive=sha256|members
- [Phase 04]: 04-22: version-refused also refuses an older minor (SameMinorVersion vs AnyNewerVersion) and requires the built major.minor found under the moved prefix
- [Phase 4]: 04-23: the installed consumer (tests/installed-consumer) replays diag-input-pass from the moved prefix only; its digest lines must equal the runner golden; independence is read from ninja -t deps, the link line and ivs_DIR; pkg-selftest=10/10
- [Phase 04]: 04-24: RUN-04 is gate-checked: data/expected/important-gates.ids (102 lines, 26 gates) and the pure step important-gates; mutant ids also resolve as <suite>.judge.* / <suite>.mutant.*; gap fills api-stop-orphan, pack.judge.patched-word, runner-process.judge.outcome-uncovered, no new fault
- [Phase 4]: Hosted gate budget re-baselined (04-25): 30/40 s wall budgets superseded by median gate_s 75 s plus median cpu_s 200; revisit at last-5 gate_s median > 90 s, any run > 120 s, or last-5 cpu_s median > 200
- [Phase 05]: 05-01: oracle-first plan done with no C; page-reading verdict equations no, shapes yes, amplitude no, DAC labels no; shape and printed-chain rate vectors are source, DAC and unprinted topics are policy; C69..C80 and CL-0009..CL-0015 allocated
- [Phase 05]: 05-01: controller literal (31 rows) transcribed from the cached pin matrix and wiki image before any planning-note table; input and psg-vectors gate steps, 13 controls, steps 35/35 controls 294/294
- [Phase 5]: 05-02: request v2 (psg_log, psg_log_capacity) and report v2 (audio_first_index, psg_log_bytes_written, psg_log_first_index) grow behind pinned minimums IVS_RUN_REQUEST_V1_SIZE 64 and IVS_RUN_REPORT_V1_SIZE 112; the report is written as min(struct_size, 136) bytes and its overlap span is the written byte count
- [Phase 5]: 05-02: write log is the third buffer after video and audio (reasons 55..57); a NULL buffer with capacity 0 means channel not requested (D-17); psg_log versus events is RUN_PSGLOG_OVERLAP decided in stage 4b; steps 35/35 controls 296/296 faults 152/152 under sh, dash and the Ubuntu container
- [Phase 5]: 05-03: PSG storage joins the instance (psg_reg and port_pulldown in MEM, 72-byte ivs_psg_state in INSTANCE, audio rows emitted and prev_levels, domain 19 PSGLOG); composite version 2 with the v1 literals recorded first (5114d8a); IVS_MACHINE_BYTES 113760; goldens moved only on digest.mem, digest.instance and digest.composite, digest.cpu and digest.images equal v1
- [Phase 5]: 05-04: PSG region is device kind psg8 (upper byte 0, C64) with masks from psg.regmap through IvsPsgTable.cmake; every guest access goes through ivs_machine_bus (forwards unchanged until 05-06/05-08); shape and port registers are declared exception rows (x=reads-zero, x=pins) observed in psg_reg; suite psg 40 ids, steps 35/35 controls 300/300 faults 158/158, goldens unchanged; checkpoint-1: lever-required (projection 188.7-195.5 cpu-s vs trigger 190, 0.144 cpu-s per fault)
- [Phase 5]: 05-05: ivs_input_rederive is the only setter of port_pulldown (apply and the refused-step take-back call it); the port read is pure and lives in memsys.c
- [Phase 5]: 05-05: the memmap read rows of the two port registers have w=0 x=pins; the input-visible fuzz compares whole-instance snapshots instead of MEM digests
- [Phase 5]: Time chain: one PSG step = 8 chip clocks = 16 master ticks = 4 CPU cycles; step k stamped k x 16; first chip step after reset is step 1 (05-06)
- [Phase 5]: PSG caught up to the write stamp before the masked store and to the unit end at every unit end, never to a call target (05-06)
- [Phase 5]: Audio layout real: 4-byte samples, 15,060 bytes per frame with slack ceil(U/P)+1; U = 468 ticks derived by exhaustive probe, g = 28 ticks overall (36 instruction only) (05-06)
- [Phase 5]: A layout whose audio format is not PSG levels, and a NULL buffer, use the counting sink: same code, same state (05-06)
- [Phase 5]: Noise register is a profile-driven right-shifting Galois LFSR; shifts every 2 NP steps from the reload; reset loads the seed (05-07)
- [Phase 5]: Envelope is the 16-state machine of the 16 shapes: one state per 2 EP steps, restart on a shape write through the shared write path (ivs_psg_after_write), held at level 0 after reset (05-07)
- [Phase 5]: The DAC table psg.dac.levels is profile data the core never reads: not an ivs_psg_params member, byte-identical core parameters proven at build time (05-07)
- [Phase 5]: Every tagged noise, envelope, period-zero and mixer parameter is proven to be data by a stack-copy perturbation test (psg.profile.*), faults hard-coding one fail exactly its test (05-07)
- [Phase 05]: 05-08: the write log record is appended after the masked store with the catch-up stamp; domain-19 index is the record count; ivs_info publishes W_log from the env so the cut fault is visible to callers
- [Phase 5]: 05-09: audio.chain.check is - unless an audio buffer was delivered on every slice; a mismatch is harness-fail audio-chain; the golden observed rule is count-based for video, audio, input and psglog
- [Phase 5]: 05-09: operator raw outputs are vtable hooks plus options (--audio-raw, --psg-log-out, --audio-out-max-bytes), created exclusively in hostio.c; a capture or stream assert without output buffers is a harness failure
- [Phase 05]: 05-11: fixture pointers use R4 and R5 (only they auto-increment); derivation corrected in dated section (g), original text kept
- [Phase 05]: 05-11: .refpcm golden written from a runner report by a documented recipe; golden step compares it with dev and release refpcm reports
- [Phase 05]: 05-12: C73 text aligned to the profile tag unknown-default-chosen; the D-22 listening check recorded as deferred (no listenable tone), not handed off; AUD-01 confirmed complete with the C11 caveat
- [Phase 6]: 06-01: page-reading verdict: BACKTAB address and 14-bit use are tier 1 (partial), registers, windows and bus hold are tier 4 or defaults; allocation table fixes C81..C105 and CL-0016..CL-0022
- [Phase 6]: 06-01: the diagnostic reads one BACKTAB word set in both modes and is derived against the frame-0 timeline before its listing (133 words, 4 branches, margins in docs/video/diag-stic-1-derivation.md)
- [Phase 6]: 06-02: -g0 on fault-only targets applied (paired cold fault build minus 35.6 container cpu-s, about 18 hosted, no evidence change); r = cpu_s / build_s recorded beside raw triggers (budget 7.8, revisit above 8.3)
- [Phase 06]: 06-03: v3 report and v4 info tails are zeroed locals copied min(struct_size, sizeof); IVS_RUN_F_STOP_ON_FRAME stays refused until 06-09
- [Phase 06]: 06-04: STIC window, interrupt and hold figures tagged tier4-community per the timing-notes evidence table; first_start, fetch_offset and policies stay unknown-default-chosen
- [Phase 6]: 06-05: STIC storage joins the instance (stic_reg[64] in MEM, 528-byte ivs_stic_state in INSTANCE, 12 row latches); composite version 3 with the v2 literals recorded first (21b1ad5); IVS_MACHINE_BYTES 114416; goldens moved only on digest.mem, digest.instance and digest.composite, digest.cpu and digest.images equal v2
- [Phase 06]: 06-06: ADAR (the operand read of a direct-addressed instruction) is a read-like kind for the STIC hook; the plan text listed read, fetch, write and DW only
- [Phase 06]: 06-07: ivs_machine_stic_state reports all 64 stored register words (gate independent); peek stays the bus view; effects only in the bus wrapper
- [Phase 06]: 06-08: stand-in interrupt stack in 16-bit system RAM; release-latency fault drops one cycle (stepper refuses 0)
- [Phase 06]: 06-09: the core renders one card row at a time (the platform stack probe of a 15 KB plane is a forbidden import); ivs_stic_render_plane is a caller-buffer entry point
- [Phase 06]: 06-09: a flagged frame-stop call needs exactly one video slot; the pre-unit INTERNAL guard fires only when a frame could complete in the unit
- [Phase 06]: Plan 06-10: renderer is row-granular and pure (graphics bytes only via ivs_memsys_stic_gfx_byte_internal); Color Stack interim pinned by video.frame.cs-not-decoded for 06-11
- [Phase 06]: 06-11: Color Stack pointer carried through ivs_stic_render_cards_row (cs_pos); frame event stores the final pointer of a display-on frame in cs_ptr; Colored Squares implemented, cut line documented not applied
- [Phase 6]: 06-12: golden keep list moved into the library (ivs_runner_golden_keep); ivsr 3 video row live-gated by the output.video.bytes count rule; operator video outputs mode 0600 with Phase 5 cleanup rules
- [Phase 7]: 07-01: the queued premise (every member lookup parses the whole file) did not hold; the lever was re-aimed at lookups in the vector object, the file-sized element fetch and per-object command overhead, with identical findings (vectors.cmake -46 percent, vector-blind.cmake -50 percent instructions retired)
- [Phase 7]: 07-01: phase7-start: lever-applied; last-five hosted r median 8.02; expected hosted saving about 2 to 3 cpu-s (not 5), headroom about 14 hosted cpu-s against the Phase 7 estimate of 10 to 19; checkpoints C-B after 07-08, C-C after 07-09, close in 07-12
- [Phase 7]: 07-02: oracle first and no C; re-arm form B (main-loop wait) with N = 500 in a band N 183 to 808, mutants N = 100 and 1,150, negative control N = 300; first EIS at frame-0 cycle 6,564 to 6,631 (frame-1 band)
- [Phase 7]: 07-02: channel C is the constant-level gate (mixer 003C) with the tone amplitude deferred one frame through pending words 02F6/02F7; PSG init is 6 writes; 30 PSG records for pass; exact audio windows at ticks 100000, 140000, 160000, 690000, 740000, 760000
- [Phase 7]: 07-02: the settling rule is a time distance (events at least 16,832 ticks from a frame end), so frames 3, 5, 7, 9, 11 and 12 are asserted in pass; SCN-01 accepted by important-gates; the blind second read is requested and plan 07-06 waits for its record
- [Phase 7]: 07-03: ivs_asm is one C17 file of 806 lines (hard stop 1,200) with a shape-driven operand parser, 19 mnemonics and 17 of the 20 refusal tokens; registered like tools/pcm, never installed; the three tokens left (asm-imm-range, asm-sdbd-target, asm-decle-range) arrive with SDBD and DECLE in 07-04
- [Phase 7]: 07-03: the twins of the boot stub (22 words) and diag-cpu-1 (56 words) reproduce the hand listings word for word and column for column; no hand slip, no escape; the comparison is shown able to fail (asm.judge.mutated-twin); suite asm costs 0.05 s release, 0.63 s dev
- [Phase 7]: 07-03: file and usage errors carry no token; the source path must be relative; the header must carry a window; forecast asm.c size after 07-04 is about 975 lines, lever is table packing
- [Phase 7]: 07-04: ivs_asm is complete at 1,137 lines (hard stop 1,200): 78 spellings cover 73 of 74 leaf rows (not MVOI), explicit SDBD, DECLE with row DATA, 20 pinned refusal tokens with one case each; hand slips found by the twins 0, table errors found by the corpus 0
- [Phase 7]: 07-04: tests/asm/pairs.txt has 304 pairs (301 copied from the Phase 2 vectors with SDBD immediate operand words masked to the low byte, three hand cases), all agree at the first run; the four twins reproduce all 300 hand-typed words; asm-imm-range made reachable by widening the evaluator to 0 to 16777215 with per-use limits
- [Phase 7]: 07-04: listing.cmake gains the DATA row rule with two controls (350 to 352) and the toy listing holds one DATA word; docs/diagnostics.md documents the decision, grammar, generated listing form, explicit SDBD and twin discipline; discrepancy C106 records the printed branch range against the decle arithmetic (105 to 106)
- [Phase 7]: 07-04: a gcc-only -Wsign-conversion error in the pairs result line was found by the container gate and fixed in b05a222; suite asm has 45 cases (dev about 3 s, release about 0.4 s on a loaded laptop)
- [Phase 7]: 07-05: boot stub contract 2 (FX-boot-stub-2) is assembled by ivs_asm from boot-stub-2.asm (ISR EQU 5004, JD ISR at 1004) with the reset path word for word that of contract 1; assumption A6 holds (a JD word decodes as row J, so the stub guards and bootstub-rows.ids are unchanged); contract 1, the three diagnostics, every golden and scenario are byte-identical
- [Phase 7]: 07-05: the manifests step now checks recipe.sources digests and tool_sha256 (token manifest-recipe-digest-mismatch, table RECIPE_TOOL_SOURCES for ivs_asm and ivs_pack, control manifests-recipe-source-digest, 352 to 353); asm.c and pack.c are frozen by their recorded digests until a plan refreshes the manifests that name them; asm.regen-<id> regenerates every fixtures asm source and requires byte equality
- [Phase 7]: 07-05: runner content token boot-stub-2 (kind 5, reserved file boot-stub-2.exec, exec only) labelled project boot stub contract 2, not an EXEC from the symbolic kind only; ivsr stays 3; the refusals are runner.parse.edge.* because reasons-named allows one refuse case per reason; one fuzz base names boot-stub-2 so the seeded run reaches kind 5
- [Phase 7]: 07-05: run.stub2.path, run.stub2.handoff and run.stub2.jd-as-j (control STUB2_JD_AS_J, no core fault) count interrupt entries by single-unit stepping: JD gives one entry and R6 0341, a plain J gives more than one (179 measured once) and R6 above 0341; the prefix run.stub2. joins RUN_STOP_BEFORE_OVERSHOOT and RUN_BUDGET_CARRY, the only faults under which the family fails (faults 222/222)
- [Phase 7]: Phase 7 plan 06: Lamp Tone written from the derivation after the oracle and blind record; form B with N=500, render reloads the last byte (+19 cycles, A8 bounds hold)
- [Phase 7]: Phase 7 plan 06: SCN-REARM-LONG uses N=1023 (largest one-word MVII patch); section (g) stays frozen
- [Phase 7]: Phase 7 plan 06: files-trademark-name rule scoped to fixtures, data/scenarios, data/golden, tools, tests; control writes its toy file directly under fixtures/
- [Phase 7]: Phase 7 plan 07: Lamp Tone scenarios and .expect files are mechanical copies of the derivation blocks (pass 55, idle 29, shift 55, swap 49 asserts); the pass .expect is the first playable-guest-progress file and passes golden-shape
- [Phase 7]: Phase 7 plan 07: the pair judge (runner-process.pair.<idA>.<idB>, data/expected/scenario-pairs.ids) relates two reports of one run; idle is decided from input.events 0 in the report; three self-controls reject fake pairs for pair-not-different, input-not-consumed and idle-audible
- [Phase 7]: Phase 7 plan 07: 12 runner-process cases per preset added (71 to 83 executed); local suite time dev 12.0 to 14.4 s, release 6.2 to 6.8 s; repeat, slices and cwd cover the pass scenario only
- [Phase 7]: Phase 7 plan 08: 12 Lamp Tone mutants and one negative row judged on the first clause (with the value read pinned where the derivation states it) and, on the isolation rows, on the complete failing set (fails= column); stub-window rows patch boot-stub-2.exec through ivs_pack --exec
- [Phase 7]: Phase 7 plan 08: SCN-HLT-IDLE patches both words of the two-word idle branch (a halt takes effect after the next instruction; a left-over displacement word ran as SDBD and faulted); SCN-REARM-LONG is N=1023
- [Phase 7]: Phase 7 plan 08: checkpointB-07: lever-required (projection 8.02 + 25 x 0.7 / 41 = 8.45 above 8.3); lever (2), mutant inventories on the release preset only, for the orchestrator; no further test case until it lands
- [Phase 7]: Phase 7 plan 09: the installed consumer replays demo-lamp-tone-pass (prefix scn., six image arguments) and agrees with the runner golden on all nine digest lines and the observed set; it checks slot headers, six lamp pictures, seven declared pair diffs, audio windows and 30 write-log records with its own code, and a 14-call STOP_ON_FRAME loop must end with the same streams and digests
- [Phase 7]: Phase 7 plan 09: the audio window starts are the latest-write bounds of specification A8 (gate before cycle 1,300, deferred amplitude before cycle 250), because the as-built gate write is 14 cycles after the reference figures of derivation section (d); the oracle is untouched
- [Phase 7]: Phase 7 plan 09: checkpointC-07: no-lever (last-five median 8.02, projection 8.04; pkg-install step cost within load noise, at most +0.33 local cpu-s); forward bound for the SCN-02 cases about 8.30 at the S-4 upper unit cost, re-read at the close
- [Phase 7]: Phase 7 plan 10: owner-exec-title is the template of the owner-local route (content exec user and grom user, no cartridge, no assert, run-to 600 frames); skip-ok rows take tests-skipped 2 to 4 and tests-inventory prints one -- SKIPPED line per skipped id and preset; SCN-02 stays Pending until the helper of plan 07-11 and the 07-12 close-out
- [Phase 7]: Phase 7 plan 10: the golden step ties user content to skip-ok in both directions (golden-user-content-not-skip-ok, golden-skip-ok-without-user-content, controls 354 to 356); SCN-02 is an accepted and covered important-gates requirement; FX-owner-exec and FX-owner-grom are uncommitted redistributable-no manifests (manifests 8 to 10, effective-no 1 to 3); no digest and no image enter the tree, D11 not performed
- [Phase 7]: Phase 7 plan 11: ivs-run refuses --audio-raw, --psg-log-out and --audio-wav for a scenario with user content (exit 14, one line, no file, before any storage), so all six derived outputs are refused; tools/owner-run.sh (80 lines) refuses CI, an in-tree path outside refs/, a wrong size and a missing or malformed lock (exit 2), prints the SKIPPED line with exit 77 when no image exists, and hands the runner a temp copy of the template with expect-sha256 lines from the owner's lock (--record writes the lock and refuses to replace one)
- [Phase 7]: Phase 7 plan 11: nine runner-process.owner-run.* cases and the self-control runner-process.judge.owner-run-refusal test every helper refusal, the skip and the identity mismatch on a copy of the helper in a scratch root outside the work tree with zero-filled files only (tests-expected 4598 to 4624, gates 77, lines 264); SCN-02 stays Pending for the 07-12 close-out; cost about 3.0 local cpu-s, projection at most 8.25 under the 8.3 trigger, lever (2) still queued; D11 not performed
- [Phase 7]: Phase 7 plan 12: CL-0023 (playable-guest-progress) and CL-0024 (installed-host-behavior) are provisional by the weakest-field rule, the only allowed wording is 'playable guest progress, for this original program only', and plays and playable are text tripwire claim words (the tier identifier is exempt; control text-untagged-playable, controls 356 to 357)
- [Phase 7]: Phase 7 plan 12: SCN-01 and SCN-02 close on 07-VERIFICATION.md (V1 to V20 automated, H7-1 to H7-5 handed off); SCN-02 is complete on documented-route terms only, D11 stays open and not performed, no claim and no tier; README and packaging state the source-only release posture; D4 records ivs_asm
- [Phase 7]: Phase 7 plan 12: phase7-close: no-lever (last-five hosted median r 7.78 against budget 7.8 and trigger 8.3; gate 146 s and cpu median 335 fire on the slowest build_s of the file, a runner-speed event); final gates sh, dash and container PASS with controls=357/357 faults=222/222 claims=24 provisional=23 tests-skipped=4
- [Phase 8]: 08-01: the new gate step release-assets is step 37 and the later state step becomes 38 (08-CONTEXT D-38 numbered it 37); the step scans FILELIST for binary extensions and selftests tools/check/release-assets.sh on five synthetic-archive cases (four must fail for their token, outer-name-pass must pass)
- [Phase 8]: 08-01: the release asset check is content-based: archive members are judged (prefix, path, extra, missing, binary extension from binary-ext.txt), never the outer name, and the binary rule does not depend on the list (a listed .rom member is still refused)
- [Phase 8]: 08-01: release.yml is held by a per-file ci-parity exemption table (strict tag glob, job-level contents write only there, two run lines, GITHUB_TOKEN only, timeout 15) with nine new controls; tools/release.sh refuses --publish outside GitHub Actions and with --no-gate, and gh is reached only after every check
- [Phase 8]: 08-01: phase8-start: no-lever (last-five median r 7.78 against line 8.3; projection about 7.84 with the measured additions: release-assets costs 0.33 local cpu-s per run and runs three times in a gate); the GNU tar path of release.sh is rehearsed only in an emulated Ubuntu container, the Darwin install archive is not byte-reproducible (ranlib restamp)
- [Phase 8]: 08-02: version policy is three separate numbers; tag == IVS_API_VERSION macros == installed PACKAGE_VERSION is checked by a mode of tools/check/release-assets.sh that the release-assets gate step proves able to fail
- [Phase 8]: 08-02: history findings are pinned by (token, short sha) pair in data/expected/hygiene-history-pinned.ids; a new pair fails the release check, no pass verdict is printed, the baseline file is untouched
- [Phase 8]: 08-02: any commit that changes data/claims.json regenerates the CHANGELOG claim table in the same commit; --release and --publish halt on release-changelog-section-missing until plans 08-20 and 08-21 write the 0.1.0 section
- [Phase 8]: 08-02: the release rehearsal accepts one named Darwin host effect (ranlib restamp of the first member header of lib/libivs_core.a) and requires byte-equal install archives on Linux
- [Phase 08]: 08-03: the oracle derives the delayed frames with shift-tolerant card cells (no grammar construct beyond the plan); F-M4 to F-M6 are hashed under the pinned defaults PD-8 and PD-9, carried by the claim exclusions
- [Phase 08]: 08-03: collision truth table reads sticky bits in the routines of frames 3 and 4 and writes only zero to a collision register; no row depends on a read side effect or a nonzero write
- [Phase 08]: 08-04: the MOB overlay gives a position to the lowest index with a set pixel and only then tests the owner's priority bit against the card layer (a hidden owner still blocks a higher index); part of the pinned priority behavior, nothing asserts it
- [Phase 08]: 08-04: the composition is one path (ivs_stic_compose_card_row) shared by the streamed slot and the whole plane; with every MOB register zero it returns the card row untouched, and the vertical-delay carry of plan 08-06 must not be a stack buffer in the deliver frame (the chain deliver, compose, overlay is 3,040 bytes at -O0)
- [Phase 08]: 08-05: collision is evaluated per television line over the 167 x 105 object grid (ivs_stic_collide_lines per card row inside the one composition path, ivs_stic_collide_edges for the grid lines above and below the plane), accumulated in a local uint32_t[8] and ORed into the register file at the frame end by ivs_memsys_stic_collision_or_internal; the grid has no column right of the plane, so the border bit never comes from the right edge
- [Phase 08]: 08-05: the own bit of a collision register is never set by the frame writer, but a CPU write of it is still stored (write mask 03FF kept as the plan and the Phase 6 tests say); rule STIC-R44's own-bit write rule is an open item for the owner of the regmap literal
- [Phase 08]: 08-06: the plane rows of a frame, streamed or whole, come out of one row generator (ivs_stic_rows): source card rows composed in order into the one card-row buffer, output row y is source row y - v finished in place (delay shift, exposed area, MOB margin, strips painted last); no carry buffer or re-render exists, the deliver frame is 2,528 bytes at -O0 and the chain deliver, rows_next, compose, collide is 3,536
- [Phase 08]: 08-06: collision is evaluated on the undelayed scene (an unpinned choice, stated in stic_frame.c and the reference notes); the three delay selectors video.border_follows_delay, video.delay_exposed_fill and video.delay_mob_in_exposed are exempted by name from the glossary duration-suffix rule (the word names the picture delay registers, not a time): the orchestrator confirms the exemption or renames the fields
- [Phase 08]: 08-07: the scene words of ivs-diag-stic-2 are ten tables of 27 words (two decles per word) written by a loop of eight groups of three plus three straight words, so the longest routine (selector BD, frame 3) ends its register work at frame cycle 1,634 (bound 1,900, window close 2,000); a table word is two decles, so a mutant patches the decle that carries the change (STIC2-MOB-X-ZERO leaves X word 0600, X coordinate 0)
- [Phase 08]: 08-07: the six STIC2 program mutants run their scenario only on a Release build (fifth argument of runner-process, a genex) and are judged statically otherwise, same ids and no skip-ok (D-40); the program follows A5 to the letter, so the frame counter relies on the zero power-on RAM default (open item for the owner: an explicit store would be a new oracle commit)
- [Phase 8]: 08-08: the own bit of a collision register on a CPU write is an explicit unknown-default-chosen pin (the source says it reads 0, the default keeps the write mask 03FF; frozen Phase 6 literal and tests unchanged, PD-7 amended by an addition to the derivation); collision on the undelayed scene and the zero power-on RAM frame counter are claim exclusions and assumptions, not oracle changes
- [Phase 8]: 08-08: checkpoint A reads lever-required (median r 7.78, projection 8.06 to 8.18 against the 8.0 rule of the orchestrator): no further test id, fault or control in waves B and C until the orchestrator answers; lever (2) is queued as the first task of the next wave; the D-36 split trigger is the orchestrator's decision
- [Phase 8]: 08-09: wave B opens oracle first and with no C: the state gate step (38) holds the digest field table text, the 88-row hidden-state checklist (STATE 51, IMAGE_BEARING 3, BOUND 28, PAD 6; magic is BOUND), the 18 hard-point cuts and the hand-derived schema literal with its SHA-256 pin; IVSN v1 is 612 + 3,655 + 32 = 4,299 bytes and its eight machine-level digests are informational (D-41); one witness per row and every cut used (the plan examples left four cuts without a row)
- [Phase 8]: 08-09: the blind read of the checklist (member list from the struct headers, class definitions only) is requested in docs/state/checklist-second-read-request.md with first line pending; the orchestrator builds the blinded files and runs the reader, plan 08-10 starts only when the record docs/state/checklist-second-read-record.md says state: closed; STA-01 and STA-02 stay Pending
- [Phase 8]: 08-10: the digest field table is the single serializer: every one of the 88 rows carries its snapshot class in the former reserved member (now snap, same size), written as the seventh argument of the row macro and held equal to the checklist by the state step; one filter line after the domain test and a state-only sink flag give ivs_machine_state_preimage (lengths 35, 2761, 615, 51, 63, 79, 51 as derived on paper); the digest path leaves the flag 0, so no digest literal, golden or DIGEST fault site moved (8 sites before and after)
- [Phase 8]: 08-10: ivs_psg_state_validate carries every bound derived from the chip step functions (three more than the plan listed: the noise counter, the running-envelope level relation, held implies counter 0), refuses the all-zero state of an OFF instance (save is refused while OFF), and is shown true of the grid, 52 engine vectors and the nine stand-in programs; the Lamp Tone runs reach it only in the round trip of plan 08-12
- [Phase 8]: 08-11: the IVSN v1 writer reproduces the oracle of plan 08-09 in every figure: header 612, sections 35, 2761, 615, 51, 63, 79, 51 at 612, 647, 3408, 4023, 4074, 4137, 4216, trailer at 4267, total 4299, and the schema digest 6cd4932f...b193 built from the live table descriptors equals the hand-pinned literal; the profile parameter digest 89ad6507...0522 hashes four per-profile constants over generator table text with the comments removed (a cite edit moved none, a value edit moved exactly one)
- [Phase 8]: 08-11: ivs/ivs_state.h adds ivs_machine_snapshot_size and ivs_machine_snapshot_save (refusal order arguments, handle, overlap, OFF, capacity; nothing written into the buffer on a refusal; out_written inside the buffer and a capacity past the address space are BAD_ARG); no image byte is ever written (state.save.no-image-bytes, fault SNAPSHOT_IMAGE_BYTES_EMBEDDED); the stepped boot snapshot is pinned (5000 units) and identical in the dev and release presets; IVS_E_STATE_MISMATCH stays with plan 08-13; STA-01 and STA-02 stay Pending
- [Phase 8]: 08-12: ivs_snapshot_describe is the instance-free face of a separate reader (machine_snapshot_read.c, the layout constants moved into machine.h without changing a byte): it checks the container in the order of the specification up to the trailer and the form of the profile id (the length bound before any read, magic, versions, schema, total against the length and the derived size, section count, the section table entry by entry, the trailer, then the id form), with the 14 reasons 58 to 71 on five statuses; where the plan prose read differently the specification won: the id form is judged after the trailer and the section lengths are not compared with the derived lengths by the reader (restore does, reason 80, so it stays reachable)
- [Phase 8]: 08-12: a refusal writes only reason, byte_offset, section_index and the header facts the supplied length let it read (versions from 12 and 24 bytes, total from 16) and keeps struct_size and every other member, a success writes the 360 bytes and keeps a larger caller's tail; the matrix of 406 rows (381 refusals and 25 accepted rows) is written from the specification, each row runs twice into a 400-byte report poisoned 0xA5 and 0x5A, the sweep of every length from 0 to 4300 uses exact heap buffers under the dev sanitizer (a fault build adds a zeroed tail), every reason is expected and produced by a row, and a self-control corrupts one expectation per family; faults SNAPSHOT_TRAILER_UNCHECKED and SNAPSHOT_LENGTH_TRUSTED fail exactly their ids; STA-01 and STA-02 stay Pending
- [Phase 8]: 08-13: ivs_machine_snapshot_restore validates everything into a probe (the CPU, PSG and STIC structs, the input row and the few scalars the invariants read, about 800 bytes) and then commits with a table-driven pass that cannot refuse: STATE rows decoded, PAD rows zeroed, no BOUND or IMAGE row written, the phase becomes POWERED; the check order is the specification's (arguments, handle, overlap 81, the reader 60 to 71, profile 72 73 74, slots 78 75 76, lengths 80, counts 77, frames and ranges 78, derived invariants 79); IVS_E_STATE_MISMATCH (-24) is added in one change with the reserved band narrowed to -25..-31, the ten reasons 72 to 81 are pinned and the call has 9 refusal rows
- [Phase 8]: 08-13: the matrices are written from the specification (180 table rows against four targets and 20 argument rows, each run twice into a poisoned 400-byte report compared whole, the whole target instance byte-identical afterwards, repeated against poisoned non-trivial targets with the nine digests), a restore into a poisoned target equals the source byte for byte at 13 cuts, three faults fail the id named for them, and the invariants were held against 411,075 unit boundaries of 23 programs: INV-2 corrected (the reset state), INV-3 strengthened (previous levels three four-bit levels and a byte of 0), INV-6 kept (decided true), none dropped
- [Phase 8]: 08-13: cost checkpoint B reads the median of the last five hosted rows, 7.98 (row c57328a contains plans 08-09 to 08-12): checkpointB-08 no-lever, with the margin to 8.0 (0.02) and the reading about 8.16 to 8.27 at the end of wave B stated; the wave B additions are priced by measurement (suite state 2.17 dev and 1.05 release cpu-s, 8 fault runs 5.87, the nine digests of the atomicity sweep the first trim); STA-01 and STA-02 stay Pending until the hard-point suite of plan 08-14
- [Phase 8]: 08-14: the hard-point suite: 18 cuts found by a witness scan over the step seam, pinned in data/expected/state-cuts.ids through a generated include and re-proved by three witness ids (first occurrence equals the pin, the public run call ends exactly on it with the scan's bytes, the instance part of the predicate holds); the comparison restores each cut into a 0xA5-poisoned target and compares the whole instance and the nine digests at once, then the concatenated video, audio and PSG log streams, the first indexes, the run and public reports and the final instance with the uninterrupted run; every cut agrees; the designed latch program discriminates a restore that recomputes stic.row_latch (8 of 15,296 bytes differ); the fault SNAPSHOT_STIC_ROWS_OMITTED is caught by state.cut.stic-points (its must-pass id is state.witness.stic-points); the snapshot at every cut is a fixed point and independent of slicing and the SHA-256 at three cuts is pinned and equal in the dev and release presets
- [Phase 8]: 08-14: the runtime row-omission sweep puts each of the 51 STATE rows back at its witness cut with other values (the complement first, each candidate judged by the restore's validators and invariants): 39 rows diverge in behaviour, 3 in a public report only, 9 are exempt (four chains, obs.input.count, obs.input.words, psg.env_level, stic.cs_ptr, stic.late_grants) with an argument each; it found weak witnesses and the cuts were corrected (HP-STEAL-A the boundary before the first grant 74,928, HP-STEAL-B the boundary before a release 79,008, P-PSG a triangle envelope so env_dir is non-default, HP-ENV-MID 9,504) while the frozen checklist was left untouched: the amendments are the appended notes data/expected/state-sweep-notes.ids (an edit of the checklist file was refused by the permission system); the cost stop rule did not fire (about 3.2 hosted cpu-s added, the nine fault runs sum to 6.7 local cpu-s); STA-02 stays Pending until plans 08-16 and 08-17
- [Phase 8]: 08-15: the corruption and fuzz family (state_fuzz.c, 12 ids, no core change, no fault, no control): every case runs through one checker (a documented status and reason pair, a refusal leaves the whole 114,416-byte target equal to a baseline copy and writes only a poisoned 400-byte report, an acceptance writes only STATE and PAD rows, zeroes the PAD rows and re-encodes every changed section, on a bounded subset all seven sections, a full save with the eight informational digest fields and the trailer taken from the input, the report of describe and a 6,000-tick run, two attempts identical) and its expectation is written from docs/formats/ivsn-v1.md: a position classifier of 19 classes for a single flipped byte (reason, offset, section and slot kind) and a model of the container layer for several bytes or a lie about the length; the classifier agreed with the code on 8,598 unsealed, 8,598 resealed and 9,792 header-bit flips at the first run and the model on 4,484 describe cases
- [Phase 8]: 08-15: truncations 4,299 and byte flips 8,598 all refused; resealed flips 3,190 (1,258 refused, 1,932 accepted), header bits 3,968, 126 length and table lies, 28 semantic lies, saturated counters and the clock at its largest (the machine reaches 2^63), 1,200 seeded random cases, 4,484 describe cases on heap buffers of exactly the input size, 10 handle cases, 623 cases twice, and a judge of 49 controls; the plan's literal sets (8,598 resealed attempts and 9,792 header bit cases) were run once in a scratch edit and pass, and are not built (+1.3 dev and +0.8 release cpu-s); two acceptances are pinned and recorded, not closed (the STIC register words are not checked against their write masks, machine time is not tied to a unit boundary); the libFuzzer target fuzz_snapshot.c was built with Homebrew LLVM 21 and ran 100,000 times from one valid snapshot without a violation; the cost stop rule did not fire (suite +1.55 dev, +1.00 release, fault runs +1.0 to 1.4: about 6.2 hosted cpu-s, the projection about 8.13 to 8.21 against the line 8.3); STA-01 and STA-02 stay Pending until plan 08-17
- [Phase 8]: 08-16: the round-trip hook of ivs-run (ivs_runner_options.roundtrip_ticks with the flag IVS_RUNNER_OPT_ROUNDTRIP, ivs-run --roundtrip-at-ticks <n>): at the first unit boundary at or after every multiple of n the executor saves the machine into a buffer of its own, resets it to power-on and restores the bytes, and the report is the plain report apart from host.calls for five scenario families in both presets (diag-boot-pass 600: 3 cuts, diag-ctrl-psg-pass 5,000: 3 with input events, diag-stic-fgbg-pass 20,000: 4 in a displayed frame, demo-lamp-tone-pass 90,000: 8 across twelve frames and six events, diag-stic2-collision-pass 150,000: 4 with the collision registers of frame 1 carried through the cut that the scenario's own asserts read out in frame 3); no cut at the end of a run-to, more than 64 cuts is the harness failure roundtrip-cut-cap, and machine time is read after the reset and the restore so a cut that did not happen is a harness failure and not a silent no-op; the process comparison has a host.calls vacuity guard and a self-control with two fake runners; a core that skips the STIC register words on restore fails exactly the two STIC process cases; no report key, no golden, no file, no core file
- [Phase 8]: 08-16: the fifth installed-consumer replay (snap_session, no new image argument, 29 snap. lines, consumer-lines 135 to 164, pkg-selftest 17 to 19): Lamp Tone saved mid-frame (tick 450,032, frame 7, three events applied) through ivs_machine_snapshot_size and _save, described without an instance, restored into a machine another run dirtied after a damaged copy was refused by its trailer, equal to the source through nine digests and the CPU and STIC reports, a save of the target is the snapshot, and the end digests are those of the straight run, compared with data/golden/demo-lamp-tone-pass.digests through the prefix snap; the public API has no call reporting the applied-event count, so the host keeps it beside the snapshot (open item for 08-17); the file options of the runner stay deferred (D-18); 10 test ids (tests-expected 4912 to 4932), no fault, no control; the cost stop rule fired on the upper end before the cost commit (3.2 local cpu-s, 8.25 to 8.33) and not after it (1.6 local cpu-s: the 65 cuts of the cap id run whole in the normal executables only, reported under the 19 runner fault executables): the projection about 8.18 to 8.28 against the line 8.3; STA-01 and STA-02 stay Pending until plan 08-17
- [Phase 8]: 08-17: the state wave closes on recorded terms: ivsn is a rights-bearing extension by name (tools/check/binary-ext.txt and .gitignore, the control files-unlisted-binary-ivsn shown to fail without the list line, controls 378 to 379), docs/owner-images.md states that a snapshot of an owner-image run is derived rights-bearing material and that the core writes no file and the runner has no snapshot file option in this phase; the persistence policy (open question 8) is decided as a delegated decision: bytes only from the core, files are a host's policy, no default location, exact-build and not archival, reversible by adding sections and raising the schema version (text in the 08-17 SUMMARY for the Decision Register)
- [Phase 8]: 08-17: CL-0028 (provisional, tier deterministic-repeatability, artifact tests/state/state_cut.c, 51 profile members) states for one build and one profile that snapshot, restore into a poisoned or dirtied target and run equals the uninterrupted run at the 18 hard points and that the named classes of bad snapshots are refused with the instance unchanged; the note carries the oracles, the same-model-family caveat, the 9 exemptions of plan 08-14 (data/expected/state-sweep-notes.ids), the two known acceptances and the Excluded on purpose list (no cross-build, hardware, timing or file claim; informational digests; trailer is integrity not authenticity; no big-endian host; D10 and D11 not performed); claims 28, provisional 27, CHANGELOG table regenerated
- [Phase 8]: 08-17: important-gates.sh accepts STA-01 and STA-02 and the registry names 22 mechanisms over six gates (nine faults, six judges, seven controls; gates 83 to 89, lines 282 to 304) with one toy line per requirement in each of the ten toy registries and no CHECK, EXPECT or FORBID file changed; STA-01 and STA-02 are marked complete in REQUIREMENTS on stated terms (provisional claim, one build and one profile, same-family blind reader, nine exempt rows, informational digests, no file persistence, file options deferred, applied-event count without a getter, nothing measured); PRF-01 and REL-01 stay Pending
- [Phase 8]: 08-18: ivs-bench (tools/bench, target ivs_bench, two C17 files: bench.c in ISO C and bench_host.c, the only file with the monotonic clock and the process CPU time) verifies iteration 0 of every workload against the committed golden through ivs_runner_golden_keep and against the final tick of the workload table before any clock is read, and refuses a mismatch with `bench-refuse`, exit 15 and a row with every numeric column empty; TSV `bench-format 1` in three groups plus the ci_time pointer, tier repeatability-only; --verify-only reads no clock and is the only mode the gate runs; workloads bench-lamp-idle (120 frames), bench-lamp-input (64 events: the six of demo-lamp-tone-pass verbatim, 58 self-recorded, 32 moved by the settling rule) and bench-lamp-cold (the existing demo-lamp-tone-pass) in data/bench/workloads.tsv with two new scenarios and goldens (goldens 27 to 29); suite bench with 8 ids, shown able to fail against two broken stand-ins of the tool; control c-build-unlisted-bench-source (controls 379 to 380); no file under core/ or runner/ changed, no claim, no threshold; PRF-01 stays Pending until plan 08-19
- [Phase 8]: 08-18: cost: the bench suite adds about 1.94 local cpu-s (dev 1.69, release 0.25) and the two scenario ids 0.38, in all about 2.3 local (3.9 hosted, about +0.075 in r): the checkpoint C bound of about 0.4 local for the remaining plans is exceeded, the projection reads about 7.94 by the last-five median 7.86 and stays under 8.3; the cost is process start-up under the sanitizers (15 launches of about 0.09 cpu-s); trims are named in the 08-18 SUMMARY and not applied
- [Phase 8]: 08-19: bench-lamp-snapshot is a row of data/bench/workloads.tsv over the idle scenario (cadence 1,792,080 ticks, three cuts in the 120-frame run) and ivs-bench hands the cadence to every execution through IVS_RUNNER_OPT_ROUNDTRIP, so its digest precondition is the idle golden: a save, reset and restore every 30 frames must reproduce the idle report before any figure is printed; a run in which the hook made no cut is a harness failure of the tool; ids bench.verify.bench-lamp-snapshot, bench.workloads.table-consistent (with three broken scratch tables) and bench.baseline.record-format (with four broken scratch records); docs/bench.md says what a figure is and is not (repeatability-only, never evidence, never a claim; lat_first_ms is the first timed iteration of a warm process; about 43 percent of a verified run is SHA-256); important-gates.sh accepts PRF-01 with two lines under the gate prf01-digest-first (gates 89 to 90, lines 304 to 306) and each of the ten toy registries covers it; no claim row for a benchmark
- [Phase 8]: 08-19: the baseline record data/bench/baseline-darwin-arm64.tsv is the orchestrator's release-preset input unchanged under a three-line comment block, host class darwin arm64 18 cpus, 2026-11-15, and it was taken on a host that was NOT idle (load average 5.96 on 18 cpus, kept verbatim on its idle line), so it is informational and an idle-host record replaces it by a reviewed change; only its format is checked and no number is compared with a threshold; PRF-01 is marked complete on stated terms (named workloads verified before any figure, refusal control, separate groups, CI time a pointer, no threshold, no optimization in Phase 8, no claim, repeatability-only); REL-01 stays Pending; cost of the plan about 0.33 local cpu-s (+0.011 in r), projection about 7.95 by the median, under 8.3
- [Phase 8]: 08-17: cost checkpoint C reads the median of the last five hosted rows, 7.98 (rows 8.17, 7.98, 7.28, 7.86, 8.29; the newest row 1a27cdd contains wave B as far as plan 08-16): checkpointC-08 no-lever by the rule, with the readings at the lines stated (newest row plus the remaining additions 8.30, median row counted 8.18 to 8.29), wave B measured at about 21.4 hosted cpu-s (suite state 3.30 dev and 1.94 release, nine fault runs 5.50 under the target of about 6), the remaining plans bound to about 0.4 local cpu-s, the lever order (3) next and the first trims named
- [Phase 8]: 08-20: the stale-text sweep fixed false statements before the tag: the README sentence that no release exists (now a tense-free statement that releases are pre-releases of an exact tested commit and that publication follows the delegated release flow), the installed file counts and the Release content of docs/packaging.md, the CHANGELOG section [0.1.0] (the former Unreleased body, eleven files and five headers), the digest composite version 3 in one header comment and one call record, the ROADMAP, REQUIREMENTS, STATE and PROJECT statements that showed D1 to D9 and D12 open, the Phase 8 plan list, the PROJECT validated list on recorded terms (REL-01 excluded) and the register rows OQ8 (persistence) and OQ11 (performance targets), and Phase 7 lessons 13 to 18 in PLAN-LESSONS; no count, test id, fault, control or golden moved (steps=38/38 controls=380/380 faults=235/235 goldens=29 tests-expected=4958 tests-executed=4954 tests-skipped=4)
- [Phase 8]: 08-20: requirements-completed was backfilled into the 39 SUMMARY files of Phases 4 to 7 that lacked it (the plan's own requirements field verbatim, one added line each); the three-source check over Phases 1 to 7 reads 45 requirements and 0 findings (11 requirements of Phases 5 and 6 had no SUMMARY before); 08-ACCEPTED-DEBT.md records the six draft validations, the STATE deferred items, the open review items of Phases 4 to 7 and the items deferred out of Phase 8 (the snapshot file options, 192-line video publication, libFuzzer in CI, SHA-256 frame-hash optimization, D10, D11, a history rewrite, name clearance); it changes no requirement mark and no claim; REL-01 stays Pending (D-28)
- [Phase 8]: 08-21: stage 1 of REL-01 is recorded and REL-01 stays Pending: 08-MILESTONE-CLOSE.md holds the eleven sections A to K with A, F, G and J pasted from tools/release-readiness.sh (the full history scan: six findings, all six on the pinned list and printed with their recorded disposition, none new, never recorded as a pass; 253 s of wall time on the head of plan 08-20), 08-VERIFICATION.md follows the Phase 7 layout (status human_needed: SC1 to SC4 hold on recorded terms, SC5 at stage 1; 20 V-rows with the commits read from git log), every fact that exists only after the merge is the literal placeholder (the push run, the open pull request and issue counts, the ruleset read, the artifact commit and the release decision), and the release is not started: paused by the owner's decision after Phase 8, relayed by the orchestrator, so no tag, draft, release, push or gh call exists
- [Phase 8]: 08-21: the Decision Register gained four rows (the draft pre-release D-29, the three version numbers D-30, the milestone tool's tag step D-31 and D-42, the history disposition D-33 with the owner-veto note), PLAN-LESSONS gained lessons 19 to 24, the STATE deferred items were refreshed from 08-ACCEPTED-DEBT.md, and RELEASE-OPS.md item h holds the grep that proves no placeholder remains and the order of the orchestrator's stage 2 actions; cost close phase8-close no-lever (the last-five median r is 7.80 against the budget 7.8 and the line 8.3; the five rows contain wave B and the newest contains every plan through 08-20; both raw triggers fire and r sits at the budget, read as runner speed with the stated caveat)
- [Phase 09]: Wave A1's cost record recomputes the last-five window from the committed rows (median 7.696) and prints the allowance as priced with its recomputation beside it, never instead of it
- [Phase 09]: Lever (3) is recorded as a 0.12 local cpu-s ceiling that cannot move r; trim (b) is declined in writing; the lever and trim (a) alternated before/after stays an open item with its reason rather than a figure that was not taken
- [Phase 09]: The header's phase enumeration was replaced by one phase-to-plan table rather than extended in place: a prose list of phases is what rotted (it said Phase 4 to Phase 8 and omitted Phase 1 to Phase 3), and the two token sentences now say 'the accepted set above' instead of re-listing phases.
- [Phase 09]: The real registry is counted over non-comment rows (24/0 filtered) and the ten toy registries over git diff --numstat (17/0 each); the 68-vs-24 numstat gap on the real registry is its # Plan 09-04 and 17 # <REQ>: comment blocks, which lines= never counted.
- [Phase 9]: 09-05: the important-gates must-be-covered set is derived from the requirement records minus a seven-row exclusion ledger; EV-02 joined the ledger because 09-04 marked it Complete (51 - 7 = 44 = registered)
- [Phase 9]: The toy-registry drift check excludes comments and runs in both directions, so an undeclared drift and a stale DEVIATION declaration are both refused; the nine declarations were copied from measured comm output (44/43/45 rows, not the plan's stale 27/26/28)
- [Phase 9]: The per-step pass line is required to be the last non-empty line in good_one, proved by a copied-CHECK_DIR case in runner-selftest.sh because no control directory can reach a token the runner itself emits (A-02/O2)
- [Phase 09]: 09-07: the SRC- class is [A-Za-z0-9_-]+ with no dot, measured (11 tokens, 0 unregistered; the research's dot class extracts 3 unregistered .pdf tokens)
- [Phase 09]: 09-07: manifest outputs are anchored to two roots, fixtures/<dirname>/ or data/, because FX-ivsc-vectors writes data/cart/*.json and a fixtures-only rule refuses the pristine tree
- [Phase 09]: 09-07: the memmap generator's refusal is additive, not a short-circuit, so the six existing memmap controls keep their pinned tokens; memmap-generator-missing is proved by a committed selftest case
- [Phase 09]: The preflight class is derived by grep over step source text (neither IVS_GATE_DIR nor build/) and printed in the run header, so a reclassification is visible rather than silent
- [Phase 9]: Plan 09-09: every bookkeeping correction is an appended note beside the measured value, the seven draft VALIDATION.md files are rows citing 08-ACCEPTED-DEBT.md section 1, and the EV-01/EV-03/EV-04 flip lands with its three exclusion rows so 54 - 10 = 44 = 44 holds
- [Phase 10]: The documented route is the source archive plus the release preset; nothing downloadable is runnable, so the route is build-from-source and no install claim is made (A-01)
- [Phase 10]: docs/running.md quotes the measured nine-line tail with host.calls qualified by slice size (11 default, 1599 at --slice-ticks 1000) rather than dropped
- [Phase 10]: The golden comparison is the committed data/golden/demo-lamp-tone-pass.digests body (124 of 124 lines) against a 131-line report; no second expectation was invented
- [Phase 10]: docs/presentation.md cites tools/check/core-symbols.sh:163-173 and its committed stand-in instead of adding a second colour mechanism (A-02); the mechanism (51515f1) is an ancestor of the policy (b5ad01d), an inversion of GUARDRAILS 2.9 recorded rather than claimed away
- [Phase 10]: the release rehearsal gains --source-route: the archive a release carries is extracted, built cold, run on the demo and compared with the committed golden by a verbatim copy of the gate's filter; release-time only, no gate step, no control, no claim
- [Phase 10]: Amendment A-04 predicted lines=335; the mechanism's stats line counts registry rows only, so the measured value is 333 (three registry rows; the two exclusion rows live in a different file). gates 107->109. The rule stands, the figure is corrected by measurement.
- [Phase 10]: The three requirement flips land with their five ledger rows in one commit: EV-05 gets three registry rows (two claims controls, one text control), PKG-05 and TOOL-04 get exclusion rows. Identity 57 - 12 = 45 = 45.
- [Phase 11]: Nine refusal tokens, not seven (A-11-02): arity carries the line-shape triggers and trailing the file-shape triggers
- [Phase 11]: The ivsl v1 document is committed before any code; the seven-field table cannot express a digest, a name, a length, a byte or a path, and the absence is enforced by the writer and runner.record.no-content-digest
- [Phase 11]: the document's content-value trigger is fed to arity, where the document and the parser both put it, not to unknown-directive
- [Phase 11]: a 20-digit at_tick field is arity, not tick-bound: the decimal reader refuses the field's width before the bound is judged
- [Phase 11]: the tick bound is pinned as IVS_SCN_TIME_MAX - 1 accepted and IVS_SCN_TIME_MAX refused, because the parser refuses tick >= IVS_IVSL_TICK_MAX
- [Phase 11]: hex runs are counted in both cases in runner.record.no-content-digest: the project's digests are lower case, so an upper-case-only scan would miss the stand-in's value
- [Phase 11]: no-content-digest lands in 11-03 rather than 11-04: both halves live in the record section of tests/runner/runner_exec.c
- [Phase 11]: the log is deliberately absent from the three content-user refusal blocks (D15) and a static assertion keeps it so; the no-log witness of the stripped demo copy ends guest-fail, not pass

### Pending Todos

None yet.

### Blockers/Concerns

Decision gates (PROJECT.md Decision Register; owner authorization 2026-11-09, `.planning/AUTONOMY.md`):
- Delegated decisions, recorded: D1 C17 + CMake; D2 master tick (u64); D3 `intv-2609-ntsc-family-p1`; D4 fixture assembler policy (Phase 4, re-evaluated in Phase 7: `ivs_asm`); D5 system images user-supplied only with a project boot stub (Phase 3); D6 logical controller input (Phase 4); D7 reference PCM outside the core (Phase 5); D8 159 x 96 framebuffer (Phase 6); D9 MIT, public repository, publication through the delegated release flow; D12 one hosted job, no matrix (Phase 4).
- Hard stops (not delegated), open and not performed: D10 physical measurement and D11 owner-local EXEC/GROM runs; evidence is marked missing and work continues.
- Deferred: D13 (LibFuturo adapter or shared-contract change), not before a real consumer exists.
- The persistence policy (open question 8) was decided in Phase 8 (plan 08-17) as a delegated decision: bytes only from the core, files are a host's policy, exact-build and not archival. Performance targets (open question 11): PRF-01 sets none.

Open questions that remain: unusual cartridge memory maps and further import formats (OQ4; IVSC v1 is the one format, others are v2), IntelliVoice, ECS, Intellivision II and clones (OQ6; a later roadmap decision), the first consumer and any LibFuturo adapter (OQ10; D13 deferred). The first scenario program (OQ9) was decided in Phase 7 (Lamp Tone).

Concerns:
- 8 phases exceeds `standard` granularity guidance (4-6); justified by five high-uncertainty subsystems (PIT-18). Revisit at milestone discussion.
- No tier-1 AY-3-8914 datasheet found; STIC timing constants disputed (C1-C4); controller matrix from one LOW source (C16).
- The release flow after Phase 8 is **done as far as the release itself**: pull request 7 merged as `3fe5373`, REL-01 stage 2 recorded, `v0.1.0` tagged on the artifact commit and published as a pre-release after P1 to P8 held. `git.create_tag` was set to `false` in its own commit (`6f800cc`) before the milestone tool was used. What remains of the milestone is the lifecycle (audit, complete, cleanup) and, as settings actions, the branch and tag rulesets recommended in `.planning/AUTONOMY.md` section 1, which are still **not applied**.

## Deferred Items

Refreshed by plan 08-21 from `.planning/phases/08-stic-completion-save-state-performance-baselines/08-ACCEPTED-DEBT.md`. Phase 8 resolved none of the four earlier rows, so none is removed; each keeps its status and gets the milestone it now waits for. Refreshed again by plan 09-09 (2026-11-25): the gate-hardening row (IN-01 to IN-04), the memmap-gen row and the machine-digest comment row are closed, the seven draft `VALIDATION.md` files of Phases 2 to 8 are now one row each (cited to `08-ACCEPTED-DEBT.md` section 1, never rewritten), and the two items Phase 9 declined plus the nine deferred registrations of D-09-06 have rows of their own. The "Milestone" column says when the item is next looked at, and "next milestone" means the one that follows the first.

| Category | Item | Status | Deferred At | Milestone |
|----------|------|--------|-------------|-----------|
| Gate hardening | IN-01 whole `SRC-` token match; IN-02 fail glossary/sources on zero documents; IN-04 require each step's pass line; IN-03 anchor manifest outputs to their own fixture dir; IN-05 runner-selftest coverage of remaining tokens; denylist grep status 2 read as no-hit (`denylist.sh`); `--tree` grep-error control | **IN-01 to IN-04 closed by Phase 9** (plan 09-09 update): IN-01 `sources-src-token-whole`, IN-02 the glossary/sources zero-document controls and their verification (09-07 items 1 and 2), IN-03 `manifests-output-anchor` (09-07 item 4), IN-04 `control-pass-line-not-last` (09-06) plus the three 09-07 controls. IN-05, the denylist status-2 reading and the `--tree` grep-error control stay open, low severity (01-REVIEW.md, 01-SECURITY.md residual risks) | Phase 1 | IN-05 and the two remaining items: next milestone, a gate-hardening quick task |
| Gate speed | Run negative controls in parallel if the hosted gate step passes ~30 s | Watching; now governed by the cost rule of `.planning/CI-EFFICIENCY.md` (median `r` 7.80 at the Phase 8 close against the budget 7.8 and the line 8.3; the hosted gate step reads 113 s on run 43732465524); the lever order is recorded there. **Phase 9 (plan 09-09) did not land it**: the phase priced three controls plus one verification plus one step-side mechanism for EV-03 (amendment A-02) and no more, and running negative controls in parallel moves no figure this phase measured, so the idea stays a watching item, governed by the cost rule | Phase 1 | Next milestone: the cost rule decides |
| Gate hardening | memmap-gen-* gate-level controls of cmake/IvsMemmapTable.cmake (needs a dedicated toy memmap profile fixture and a way for a pure step to run the generator; 03-04, 03-05, 03-06 open item) | **Closed by 09-07 item 5**: the generator now runs under the gate, `memmap-generator-refused` is the control and `memmap-generator-missing` its liveness guard (plan 09-09 update) | Phase 3 | Closed 2026-11-25 |
| Evidence | C11 AY-3-8914 datasheet not found; CP-1600 vs CP-1610 inference (C31); same-model second reader (the two Phase 8 blind reads were the same model family too) | Open, carried by the discrepancy log | Phase 1 | Owner gate D10, or a primary AY-3-8914 data sheet, or an independent reader |
| Review items | the open review items of Phases 4 to 7 by name (IN-04, IN-06, IN-09 to IN-11 of Phase 4; IN-01, IN-03, IN-05, IN-06 of Phase 5; IN-02, IN-03 of Phase 6; the owner-run helper's symbolic-link gap and two wording items of Phase 7) and the residuals of the Phase 5 and 6 security records | Open, low severity, carried (`08-ACCEPTED-DEBT.md` section 3) | Phases 4 to 7 | Next milestone |
| Validation | `.planning/milestones/v0.1.0-phases/02-cp-1610-cpu-core/02-VALIDATION.md` | Accepted tech debt, status `draft`; dispositioned by `08-ACCEPTED-DEBT.md` section 1 (every check in its map ran in its own plan and runs again in every gate), cited here rather than rewritten (GUARDRAILS 2.10) | Phase 2 | Next milestone, or never (plan 09-09, 2026-11-25) |
| Validation | `.planning/milestones/v0.1.0-phases/03-memory-map-content-loading-and-refusal/03-VALIDATION.md` | Accepted tech debt, status `draft`; dispositioned by `08-ACCEPTED-DEBT.md` section 1, cited not rewritten (GUARDRAILS 2.10) | Phase 3 | Next milestone, or never (plan 09-09, 2026-11-25) |
| Validation | `.planning/milestones/v0.1.0-phases/04-headless-runner-and-installed-consumer/04-VALIDATION.md` | Accepted tech debt, status `draft`; dispositioned by `08-ACCEPTED-DEBT.md` section 1, cited not rewritten (GUARDRAILS 2.10) | Phase 4 | Next milestone, or never (plan 09-09, 2026-11-25) |
| Validation | `.planning/milestones/v0.1.0-phases/05-hand-controllers-and-psg-sound/05-VALIDATION.md` | Accepted tech debt, status `draft`; dispositioned by `08-ACCEPTED-DEBT.md` section 1, cited not rewritten (GUARDRAILS 2.10) | Phase 5 | Next milestone, or never (plan 09-09, 2026-11-25) |
| Validation | `.planning/milestones/v0.1.0-phases/06-stic-video-first-pass/06-VALIDATION.md` | Accepted tech debt, status `draft`; dispositioned by `08-ACCEPTED-DEBT.md` section 1, cited not rewritten (GUARDRAILS 2.10) | Phase 6 | Next milestone, or never (plan 09-09, 2026-11-25) |
| Validation | `.planning/milestones/v0.1.0-phases/07-first-visible-original-scenario/07-VALIDATION.md` | Accepted tech debt, status `draft`; dispositioned by `08-ACCEPTED-DEBT.md` section 1, cited not rewritten (GUARDRAILS 2.10) | Phase 7 | Next milestone, or never (plan 09-09, 2026-11-25) |
| Validation | `.planning/milestones/v0.1.0-phases/08-stic-completion-save-state-performance-baselines/08-VALIDATION.md` | Accepted tech debt, status `draft`; dispositioned by `08-ACCEPTED-DEBT.md` section 1, cited not rewritten (GUARDRAILS 2.10). `01-VALIDATION.md` is `status: validated` and is not one of these rows | Phase 8 | Next milestone, or never (plan 09-09, 2026-11-25) |
| Snapshots | the file options `--snapshot-out` and `--snapshot-in` (runner-only, explicit path, refused for user content) | Deferred (D-18 optional last plan; policy decided in the register row OQ8) | Phase 8 | Next milestone, when a consumer needs a file; owner scope choice H8-7 |
| Video | 192-line video publication (D8 stays at 159 by 96) | Deferred | Phase 8 | Next milestone, only on evidence that reverses D8 |
| Fuzzing | libFuzzer runs of `tests/fuzz/fuzz_snapshot.c` in CI | Deferred outside the gate (TOOL-05) | Phase 8 | When a failure class appears that no gate step catches |
| Performance | optimization of the SHA-256 frame hashing; an idle-host benchmark baseline (the committed record is informational); the timed path run on macOS arm64 only | Open, informational. The two benchmark golden commands are no longer missing: plan 09-09 added the `bench-lamp-idle` and `bench-lamp-input` writing commands to `docs/runner.md` (GUARDRAILS R11) | Phase 8 | When a consumer states a frame-rate need; the baseline by owner choice H8-8 |
| State | the applied-event count has no public getter; two pinned acceptances (a STIC register word with a bit outside its write mask, machine time not tied to a unit boundary); a non-default witness for `stic.late_grants` | Open (`08-ACCEPTED-DEBT.md` section 6) | Phase 8 | Next milestone (an API addition and a designed program) |
| Hygiene | an internal comment in `core/src/machine/machine_digest.c` that still names composite version 2 | **Closed by plan 09-09** (2026-11-25): the header comment now says version 3 since plan 06-05, matching `IVS_DIGEST_COMPOSITE_VERSION 3u` (`core/src/machine/machine.h:157`); it was a comment and only a comment (`08-ACCEPTED-DEBT.md` section 5) | Phase 8 | Closed |
| Gate hardening | the two uncontrolled zero-document tokens `denylist-no-files` (`tools/check/steps/denylist.sh:43`) and `runner-scan-empty` (`tools/check/steps/runner-scan.sh:58`) | **Declined, not silently dropped** (plan 09-07, dispositioned by plan 09-09): amendment A-02's honest count for EV-03 is three controls plus one verification plus one step-side mechanism, and two more controls is scope the phase did not price. They stay uncontrolled and are visible here | Phase 9 | Next milestone, if the cost rule prices two controls |
| Evidence | GUARDRAILS 2.3's registration limb for the **ten** mechanisms Phase 9 landed with no `data/expected/important-gates.ids` row: `important-gates-complete-uncovered`, `important-gates-excluded-stale`, `control-toy-drift`, `control-deviation-unused`, `control-pass-line-not-last`, `sources-src-token-whole`, `manifests-output-anchor`, `memmap-generator-refused`, `memmap-generator-missing` (the liveness guard for the one before it), and `important-gates-excluded-malformed` (added by the review fix WR-01, `0d84484`) | **Registration deferred, visibly** (deviation D-09-06, `09-05-SUMMARY.md` is the canonical list): the registry row count is frozen at `lines=330` by ROADMAP Phase 9 criterion 3 and amendment A-08, so any row added after 09-04 makes the phase end at a count its own criterion forbids. The other two limbs of 2.3 are met: each mechanism has a liveness guard and a committed, re-runnable stand-in showing the guard fail. Reversal condition: a later milestone re-derives the `lines=330` baseline, then adds these ten rows under EV-03/EV-04 with the non-vacuity sentence every other row carries | Phase 9 | A later milestone that re-derives the registry baseline |
| Gate hardening | the four low-severity findings of the Phase 9 security audit (`09-SECURITY.md`): **F1** `GATE_TMP` is exported with deterministic `step.out.<name>` buffers, so a step can overwrite another step's buffer (the verdict cannot be flipped — `rc.<name>` is the driver's own file — but a step's body, `-- FAIL` lines and summary tokens can be mis-attributed); **F2** an exclusion-ledger reason is validated for shape, not for truth; **F3** three stale pointers inside the mechanisms' own prose (the ledger header says "seven rows" where it holds ten, the step header says "six requirements", and `state_fuzz.c:945-948` is cited where the refusal is at `:954`); **F4** `manifests.cmake:136` interpolates the fixture directory name into a CMake regex, so a `.` in a directory name widens one character, and `^data/` is broader than the two measured files need | **Recorded, not fixed** (plan 09-09 close, security audit `09-SECURITY.md`): none is high or medium, none can flip a verdict, and each fix would move the gate after the phase's verification record was taken. F1 is one line (stop exporting `GATE_TMP`; no step reads it — verified by `grep -ln GATE_TMP tools/check/steps/*.sh` returning nothing) | Phase 9 | Next milestone's gate-hardening pass |
| Owner items | D10 physical measurement (44 hardware items), D11 owner-local real EXEC and GROM runs, a history rewrite veto, name clearance, the immutable-release setting | Owner hard stops or gates, none performed; no claim above provisional | Phases 1 to 8 | Owner (section K of the close record) |
| Release | the flow after Phase 8: the merge, stage 2 of REL-01, the `git.create_tag` change, the tag, the draft and its publication | **Done**: merged as `3fe5373`, REL-01 complete, `create_tag` false, `v0.1.0` published as a pre-release | Phase 8 | Closed |
| Milestone lifecycle | the milestone audit, the milestone completion and the cleanup of Phase 8 | Next: the lifecycle sequence after the records pull request | Phase 8 | Now |
| Settings | the branch ruleset (PR + `gate`, no force push, no deletion) and the tag ruleset (no update, no deletion, from a green `main`) recommended in `.planning/AUTONOMY.md` section 1 | **Not applied** (the close read returned no ruleset) | Phase 4 / Phase 8 | Orchestrator, at the next repository-settings action |

**Appended by plan 11-05 (2026-11-25), Phase 11's five deferred items.** Each is recorded here
because a later run reads this table, not the plan's prose. None was silently dropped, and none is
a new mechanism: `lines` moves only for the four TOOL-01 registry rows this plan added.

| Category | Item | Status | Deferred At | Milestone |
|----------|------|--------|-------------|-----------|
| Input log | the `scenario`/`profile` **mismatch** refusals (research O-1, O-2): a log whose `scenario` or `profile` line disagrees with the scenario it is replayed into is currently accepted, because the replay substitutes the schedule and the two header fields are descriptive. Stated as the format's own limit in `docs/formats/ivsl-v1.md` `## Identity`, not as a defect | Deferred, reader-side change: refusing it is a new `ivsl` refusal token and therefore a version decision, and no run has yet wanted it | Phase 11 | Next milestone that opens the format (a v2 `ivsl`), with a token and a control of its own |
| Input log | a **content-identity field** in the log (a content digest, image name or length) so a log names the content it was recorded against | Deferred by D15 and by the format's posture: the log is an input artifact and carries no content bytes or digest, which is what makes it rights-clean and what `runner.record.no-content-digest` checks. The supported route is `expect-sha256` on the report | Phase 11 | Never as an `ivsl` v1 field; a v2 decision only if a consumer needs the binding |
| Evidence | promoting the **process case** (`runner-process.record.command-line`) from the script-driven suite to a real-executable case inside the gate | Deferred: the process case already runs the built `ivs-run` binary end to end on `demo-lamp-tone-pass` (319-byte log, byte-identical report, no-log witness), and a gate-level copy would be a new step, which GUARDRAILS 2.1 forbids. Reversal trigger: a defect found only at the process boundary and invisible to the in-process equality | Phase 11 | Next milestone, only on that trigger |
| Snapshots | the file options `--snapshot-out` and `--snapshot-in` | Still deferred from Phase 8 (the row above); Phase 11 neither needed nor touched them | Phase 8 | Unchanged: when a consumer needs a file (owner scope choice H8-7) |
| Gate shape | a **new gate step** for the record/replay evidence | Forbidden by GUARDRAILS 2.1 and by ROADMAP Phase 11's own cost gate (`steps=38/38` never moved); the two faults run inside the existing `c-faults` step and the registry rows inside the existing `important-gates` step | Phase 11 | Not a deferral with a trigger: a step is added only by a milestone that re-derives the step count |

**Note on the Owner items row above (plan 10-06, 2026-11-25):** the 44 in that row's
D10 figure and the 47 in Recorded Constraints item 4 below are one measurement read two
ways - 44 `D10-*` rows plus 3 `D11-*` rows, 47 in total - so neither sentence is wrong on
its own and neither should be quoted alone. Measured with
`awk '!/^#/ && NF {print $1}' data/expected/d10-measurements.ids | sed 's/-.*//' | sort | uniq -c`.
The row itself is left as recorded and this note sits under it.

**Milestone close record:** `.planning/milestones/v0.1.0-phases/08-stic-completion-save-state-performance-baselines/08-MILESTONE-CLOSE.md` (sections A to K; every stage-2 placeholder is filled by the records commit) and `08-VERIFICATION.md` in the same directory. The milestone archive moved both here on 2026-11-24; this line is the pointer that does not move.

**Milestone archive:** `.planning/milestones/v0.1.0-ROADMAP.md`, `v0.1.0-REQUIREMENTS.md`, `v0.1.0-MILESTONE-AUDIT.md` (status `tech_debt`) and `v0.1.0-phases/` (the eight phase directories). `MILESTONES.md` holds the shipped entry.

## Session Continuity

Last session: 2026-11-25T15:03:19.670Z
Stopped at: Phase 11 complete, ready to plan Phase 12
Resume file: None

## Recorded Constraints for the Next Milestone

Binding on v0.2.0, not suggestions. Each came from a recorded decision, a
recomputed figure or an owner hard stop.

1. **The gate cost rule, with its reading corrected.** The close-out quoted a
   last-five median `r` of 8.34; recomputed by arithmetic from the committed
   `.planning/ci-runs.tsv` the values are 7.327, 7.521, 7.826, 8.233, 8.338, so
   the median is **7.826** — 0.47 under the 8.3 stop line and 0.026 over the 7.8
   budget. The stop line did **not** fire. The rule still stands because it is
   recorded policy: **lever (3) and trims (a), (c), (d) land in cost-only commits
   before any new test id, control, fault or gate step, and a hosted row recorded
   after them settles the level.** Trim (b) is declined by measurement. The
   headroom is worth about **21 hosted cpu-s** for the whole milestone — a
   ceiling, not a target. After one new row the maximum possible median is 8.233,
   so a single row cannot re-fire the rule. Entry: "next-milestone opening" in
   `.planning/CI-EFFICIENCY.md`.
2. **Evidence durability first (audit W1).** 17 requirements (FND-01..07,
   CPU-01..05, MEM-01..05) have named faults that run in every gate but no lines
   in `data/expected/important-gates.ids`. Register them without adding a fault or
   a test id, extend the accepted-requirement set and all ten toy registries in
   the same commit (`GUARDRAILS-v0.2.md` R2), and add the machine check that makes
   the gap unrepeatable.
3. **Self-referential step inventory (audit W2).** No gate step reads the
   requirement records, so a commit deleting a step and its inventory line shrinks
   coverage silently. v0.2.0 adds **zero new gate steps**; any future step must be
   provable against something outside the inventory itself.
4. **Owner hard stops, not performed:** D10 (47 hardware items) and D11
   (owner-local real EXEC/GROM runs). No claim may exceed `provisional` until they
   are performed. Every human-only item is a row in `data/expected/handoffs.ids`
   with `evidence: none`.
   Note (plan 10-06, 2026-11-25): the 47 above is the whole owner-item count, not a
   second D10 figure. Measured on this tree, `data/expected/d10-measurements.ids`
   holds 44 `D10-*` rows and 3 `D11-*` rows, 47 in total, which is why the Deferred
   Items owner row says 44 hardware items and this item says 47. The sentence above
   is left as recorded and this note sits under it.
5. **Settings still not applied:** the branch ruleset (PR + `gate`, no force push,
   no deletion) and the tag ruleset (no update, no deletion, only from a green
   `main`) recommended in `.planning/AUTONOMY.md` section 1. Apply them, or record
   "not applied" with a reason — never silently skip.
6. **The milestone ritual and the guardrails are standing policy.**
   `.planning/ops/MILESTONE-RITUAL.md`, `.planning/ops/GUARDRAILS-v0.2.md` and
   `.planning/ops/INDEX.md`. Read the index before asking the owner anything.

## Operator Next Steps


- Start the next milestone with /gsd-new-milestone
