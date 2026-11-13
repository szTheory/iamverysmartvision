# Phase 7: First Visible Original Scenario - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-11-13
**Phase:** 7 - First Visible Original Scenario
**Mode:** `--auto` with the fan-out protocol (AUTONOMY.md section 2); no interactive questions, the synthesized recommendation was auto-selected per point.
**Areas discussed:** scenario program, boot/runtime architecture, D4 assembler, verification and evidence, rights/manifest/SCN-02

---

## Scenario program (S-1) - research note `07-DISCUSS-scenario.md`

| Option | Description | Selected |
|--------|-------------|----------|
| Lamp Tone (interrupt-driven lamp board, one PSG voice per port) | 132-136 words, 10 branches, no text/GROM/MOBs | ✓ |
| Polling main loop, same output | 114 words, stub contract 1 unchanged, loses counter and interrupt evidence | fallback |
| Third-party homebrew | Rights conflict (Geese C28, IntyBASIC notices, 4-Tris) | |

**Choice:** Lamp Tone. **Notes:** spike agreed with an independent model; seven of eleven mutants are visible to one channel only, so all three channels are asserted.

## Boot/runtime architecture (S-2) - `07-DISCUSS-runtime.md`

| Option | Description | Selected |
|--------|-------------|----------|
| Second stub image (contract 2), `JD $5004` vector | Zero re-bless | ✓ |
| Replace contract 1 in place | 6 `.digests` + consumer golden move | fallback |
| Plain `J` vector | Measured trap (re-entry 178-181x per frame) | |

**Notes:** re-arm band measured; two forms (in-ISR pad, main-loop re-arm) left to the oracle plan.

## D4 assembler (S-3) - `07-DISCUSS-assembler.md`

| Option | Description | Selected |
|--------|-------------|----------|
| `ivs_asm` generating listings (C17, `tools/asm/asm.c`) | Trigger fires on three counts | ✓ |
| Extend the listing checker only (DATA row), defer assembler to Phase 8 | Own adversarial pass: break-even now | |
| sh/awk or CMake-script assembler | Rejected | |

## Verification and evidence (S-4) - `07-DISCUSS-verify.md`

| Option | Description | Selected |
|--------|-------------|----------|
| Freeze IVSS v1 / `ivsr 3`, literals + gate-side region check | No re-bless | ✓ |
| New `assert audio changes` / region subjects | Grammar change, re-bless | fallback (audio only) |

## Rights, manifest, SCN-02 (S-5) - `07-DISCUSS-rights.md`

| Option | Description | Selected |
|--------|-------------|----------|
| New original-source manifest (MIT), owner-images doc, skip-ok template scenario | Documented-route completion for SCN-02 | ✓ |
| Record known EXEC/GROM digests | Rights and identity risk | |

---

## Claude's Discretion

Lamp layout, `EQU`/`DECLE` spelling, mutant addressing, gate tie of stub target to program address, consumer id prefix.

## Deferred Ideas

MOB sprites (Phase 8), reference host (v2), legacy listing promotion, reader cartridge, known EXEC/GROM digests, installable runner.

## Conflicts resolved by the orchestrator

Audio exactness (constant-level oracle), halt vs never-halt (never halt), re-arm form (wider measured margin), scenario ids (`demo-lamp-tone-*`), assembler timing (adopted).
