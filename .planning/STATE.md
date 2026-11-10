---
gsd_state_version: "1.0"
current_phase: 2
current_phase_name: CP-1610 CPU Core
status: executing
stopped_at: Completed 02-10-PLAN.md
last_updated: "2026-11-10T08:09:22.072Z"
last_activity: 2026-11-10
last_activity_desc: Phase 2 execution started
state_head: 45d35af2915ca76dbfc34b380bba12994a6a5869
progress:
  total_phases: 8
  completed_phases: 1
  total_plans: 32
  completed_plans: 24
  percent: 13
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-11-09)

**Core value:** A deterministic, observable machine core whose every compatibility claim is bound to an exactly named profile and independent evidence — never an unqualified "it runs Intellivision games" label.
**Current focus:** Phase 2 — CP-1610 CPU Core

## Current Position

Phase: 2 (CP-1610 CPU Core) — EXECUTING
Plan: 11 of 18
Status: Ready to execute
Last activity: 2026-11-10 — Phase 2 execution started

Progress: [█░░░░░░░░░] 13%

## Performance Metrics

**Velocity:**
- Total plans completed: 14
- Average duration: -
- Total execution time: -

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 1 | 14 | - | - |

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

### Pending Todos

None yet.

### Blockers/Concerns

Gates resolved as delegated decisions (owner authorization 2026-11-09, `.planning/AUTONOMY.md`; see PROJECT.md Decision Register):
- D1 C17 + CMake; D2 master tick (u64); D3 `intv-2609-ntsc-family-p1`; D9 MIT + public repo after hygiene check.

Still open, decided by fan-out protocol when their phase needs them: D4 (Phase 4), D5 (Phase 3/4), D6 (Phase 4/5), D7 (Phase 5), D8 (Phase 6), D12 (Phase 4), D13 (deferred).
Hard stops (not delegated): D10 physical measurement, D11 owner-local EXEC/GROM runs — evidence marked missing, work continues.


Other open questions: cartridge import format (OQ4, Phase 3), first scenario title/program (OQ9, Phase 7), persistence policy (OQ8, Phase 8).

Concerns:
- 8 phases exceeds `standard` granularity guidance (4-6); justified by five high-uncertainty subsystems (PIT-18). Revisit at milestone discussion.
- No tier-1 AY-3-8914 datasheet found; STIC timing constants disputed (C1-C4); controller matrix from one LOW source (C16).

## Deferred Items

| Category | Item | Status | Deferred At | Milestone |
|----------|------|--------|-------------|-----------|
| Gate hardening | IN-01 whole `SRC-` token match; IN-02 fail glossary/sources on zero documents; IN-04 require each step's pass line; IN-03 anchor manifest outputs to their own fixture dir; IN-05 runner-selftest coverage of remaining tokens; denylist grep status 2 read as no-hit (`denylist.sh`); `--tree` grep-error control | Open, low severity (01-REVIEW.md, 01-SECURITY.md residual risks) | Phase 1 | Fold into a gate-hardening quick task when the Phase 2 C build adds steps |
| Gate speed | Run negative controls in parallel if the hosted gate step passes ~30 s | Watching (9 s hosted, 2026-11-10) | Phase 1 | Phase 2 |
| Evidence | C11 AY-3-8914 datasheet not found; CP-1600 vs CP-1610 inference (C31); same-model second reader | Open, carried by the discrepancy log | Phase 1 | Phase 5 / measurement gate D10 |

## Session Continuity

Last session: 2026-11-10T08:09:22.014Z
Stopped at: Completed 02-10-PLAN.md
Resume file: None
