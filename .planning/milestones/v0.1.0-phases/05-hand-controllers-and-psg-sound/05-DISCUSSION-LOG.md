# Phase 5: Hand Controllers and PSG Sound - Discussion Log

> **Audit trail only.** Decisions are in 05-CONTEXT.md.

**Date:** 2026-11-12
**Mode:** `--auto` with fan-out (one research agent per area; owner delegation in `.planning/AUTONOMY.md`). No AskUserQuestion; the synthesized recommendation was auto-selected per area.
**Areas:** controller read path and electrical model; PSG chip rules; raw audio, write log and D7; verification and evidence strategy.

Per-area option tables, adversarial passes and sources are in the `05-DISCUSS-*.md` files in this directory.

| Area | Selected | Rejected alternatives |
|------|----------|-----------------------|
| Controller byte | 31 tier-4 line masks as profile data, `byte = 0xFF XOR OR(masks)`, generated table plus source-only literal | Hard-coded table; electrical bytes as host input; resolving ambiguous combinations in the core |
| Port read path | R14/R15 via R7 direction bits, latch in output mode, upper byte `$00` (tagged default), pulldown in the memory system | Per-access input visibility; raw port injection |
| PSG rules | 8910 rules tagged `inference` plus community register map, every rule in `data/psg/rules.json`, unsourced behaviours as profile fields | Hard-coded constants; adopting an emulator's tables |
| PSG time chain | 8 PSG clocks = 16 master ticks per step (corrects `docs/time-base.md`) | 16 clocks per step as written in the glossary |
| Raw audio | 4-byte per-channel level samples at 16 master ticks, change-record chain, write log as a third buffer | Packed u16 samples; mixed or PCM samples in the core; per-frame quantized streams |
| Digests | Composite v2 including PSG state; domains 17 and 19 outside it | Leaving PSG state out of the composite |
| D7 | No PCM in the core or the install; integer reference converter in `tools/pcm`, runner `--audio-wav` only | PCM in the core; an installed refpcm target now; floating-point resampling |
| Verification | Oracle-first plan with no C, one new original fixture, ten plans in eight waves | Self-authored expectations in each plan; 62 scenarios; 25 plans |

Conflicts between the four notes and how they were resolved are in 05-CONTEXT.md section "Reconciliation".
