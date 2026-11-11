# Phase 2: CP-1610 CPU Core - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-11-09
**Phase:** 2-CP-1610 CPU Core
**Areas discussed:** Stepper shape and bus-access API; Opcode-vector methodology and inventory; Interrupt/BUSRQ/reset/SDBD policy; Build, gate, CI and named-fault controls; Core purity (API-05)
**Mode:** `--auto` with fan-out (one research agent per area; owner delegation in `.planning/AUTONOMY.md`). No AskUserQuestion was used; the synthesized recommendation was auto-selected for each area.

---

## Stepper shape and bus-access API

| Option | Description | Selected |
|--------|-------------|----------|
| Unit-granular step + synchronous bus port + optional caller log | One step = instruction / interrupt entry / reset entry / bus grant / halt; access record delivered to a bus callback and an optional log | ✓ |
| Per-microcycle pin-style tick | ~4x calls; internal schedule not readable from the manual | |
| Coroutine / yield-pull state machine | Purer, callback-free; transient mid-instruction state enters snapshots | |
| Output-only access list | Cannot serve catch-up scheduling (read data depends on device time) | |

**Selection:** auto-selected recommendation (confidence ~70-75%). **Notes:** the manual tabulates totals and a few bus figures, not intra-instruction offsets, so offsets are tagged inference and totals stay authoritative. Adversarial pass: callbacks hamper FFI and inlining (CPU API is internal; public API is machine-level), and a 12-byte record may feel heavy (logging optional, bounded ~4/unit).

## Opcode-vector methodology and inventory

| Option | Description | Selected |
|--------|-------------|----------|
| JSON vectors + `cmake -P` generator to an uncommitted C table | Reuses Phase 1 record framework; no JSON dependency in C | ✓ |
| Hand-written C JSON parser | Second parser to keep consistent | |
| Committed generated header | Adds a staleness failure class | |
| Line-based text format | Nicer to author, needs two parsers, loses framework | |

**Selection:** auto-selected recommendation (75%). **Notes:** delta-only expectations with an unchanged-everything-else rule and poison memory; blind second reader bound to file sha256 (claims say "second-read", never independent comparison); ~300-360 vectors by behaviour class; three exact inventory lists; measured 0.79 s vs 8.1 s for `string(JSON GET)` with and without sub-extraction. Runtime `--fault` switch (agent 2) overridden by compile-time hooks, see below.

## Interrupt, BUSRQ, reset and SDBD policy

| Option | Description | Selected |
|--------|-------------|----------|
| 19 named policies (POL-*) with vectors and faults; unknowns are named defaults | Refusal-style terminal faults for unsupported SDBD targets; vector via bus-layer IAB provider | ✓ |
| Core-constant vector ($1004) | Breaks INTAK/daisy-chain priority (pdf 24/75) | |
| Treat-as-single for unsupported SDBD modes | Silent; hides bugs; not monotone | |
| Mid-instruction interrupt acceptance | Not supported by any primary evidence (pdf 22, 24, 25 say boundaries only) | |

**Selection:** auto-selected recommendation (85% on IAB provider; ~60% on BUSRQ latency; ~50% on HLT). **Notes:** new primary evidence found at pdf 27-37, 40, 43 (timing figures, wait state W); SDBD pdf 47 vs 57 conflict recorded as a new discrepancy; INTFF-not-cleared-on-accept is a profile switch with vectors for both values.

## Build, gate, CI and named-fault controls

| Option | Description | Selected |
|--------|-------------|----------|
| Compile-time `#ifdef IVS_FAULT_<ID>` per owning TU, checked for intended-reason failure | No production hook; ~1 s per 40 faults measured | ✓ |
| Runtime `--fault NAME` switch in production code | Hot-loop and leakage risk | |
| Build dir per fault | 0.5 s configure each | |
| Anchored source patching (gabbaboy-style) | Anchor drift and sprawl | |

Also decided: dev (ASan+UBSan) and release presets both in the gate; gate absorbs C steps with no workflow change; single `ubuntu-24.04` CI job (no macOS leg; Phase 1 note corrected after reading the workflow); no ccache or cache action. **Selection:** auto-selected recommendation. **Notes:** measured gate 30.3 s / 16.6 s locally under load, 9 s hosted; levers listed in order.

## Core purity (API-05)

| Option | Description | Selected |
|--------|-------------|----------|
| `nm` allowlist (default deny) + include/asm/float greps + poisoned `float double` | Portable on macOS and Ubuntu gcc (executed); catches 13 probe violations by name | ✓ |
| Denylist of banned symbols | Misses anything unlisted | |
| `-ffreestanding -nostdinc` include path | Not portable (gcc fails; clang lacks `<string.h>`) | |
| `-mgeneral-regs-only` | Blocks NEON memcpy; misses float literals | |

**Selection:** auto-selected recommendation (HIGH on allowlist and nm portability; MEDIUM on cost; LOW on MSVC). **Notes:** guards for gcc/clang LTO objects and bitcode (false negatives), Apple `ltmp0` local symbol, optimizer elision (check both -O0 and -O2); no malloc in core, caller-provided storage; `__builtin_trap()` not `assert`.

---

## Claude's Discretion

Vector grouping and slugs, fault ids, struct field order within the 32 bytes, plan split, and which purity/inventory controls to trim (with recorded coverage rationale).

## Deferred Ideas

Micro-stepped CPU core; mutable-static check (Phase 3); MSVC and CI matrix (Phase 4); MSan/TSan/fuzzing; project-owned assembler (D4); gate hardening IN-01/03/05; hardware confirmation of timing and BUSRQ latency (D10).
