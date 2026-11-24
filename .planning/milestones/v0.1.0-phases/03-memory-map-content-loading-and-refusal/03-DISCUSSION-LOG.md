# Phase 3: Memory Map, Content Loading and Refusal - Discussion Log

> **Audit trail only.** Decisions are in 03-CONTEXT.md.

**Date:** 2026-11-11
**Mode:** `--auto` with fan-out (one research agent per area; owner delegation in `.planning/AUTONOMY.md`). No AskUserQuestion; the synthesized recommendation was auto-selected per area.
**Areas:** public API and instance lifecycle; memory map and width masking; content loading and D5; cartridge import format; refusal matrix, atomicity and state digest.

Per-area option tables, adversarial passes and sources are in `03-DISCUSS-api.md`, `03-DISCUSS-memmap.md`, `03-DISCUSS-content.md`, `03-DISCUSS-cart.md` and `03-DISCUSS-refusal.md`.

| Area | Selected | Rejected alternatives |
|------|----------|-----------------------|
| Instance storage | Caller-provided storage, opaque handle | Core-side malloc; allocator callback; global arena |
| Load model | One typed `load`, validate-then-commit, OFF-only, no replace | Per-class calls; stage-in-copy; undo logs; replace-on-duplicate |
| Memory map | Profile descriptors + generated tables + independent literal map probed over 65,536 addresses | Hand-written C map; descriptors as their own oracle |
| D5 | User-supplied only + project boot stub via the EXEC slot | Built-in image; fetch; HLE EXEC |
| Cartridge format | Project container IVSC v1 | `.rom`; `.bin+.cfg`; raw binary + profile metadata |
| Digest | In-repo SHA-256 (FIPS 180-4), per-domain state digest | Third-party library; CRC; raw struct hashing |
| Byte order | Big-endian everywhere on the wire | Little-endian IVSC (research proposal) |
| Missing system image | Refused by `reset` | Refused by load |

**Confidence:** about 70% overall; weakest points: system-image byte order [BG], tier-4 cartridge windows (C62), writable-data symbol check portability.
