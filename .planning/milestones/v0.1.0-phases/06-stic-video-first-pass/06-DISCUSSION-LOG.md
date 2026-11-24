# Phase 6: STIC Video First Pass - Discussion Log

> **Audit trail only.** Decisions are in 06-CONTEXT.md.

**Date:** 2026-11-12
**Mode:** `--auto` with fan-out (one research agent per area; owner delegation in `.planning/AUTONOMY.md`). No AskUserQuestion; the synthesized recommendation was auto-selected per area.
**Areas:** STIC registers, windows and inspection; frame timing, VBLANK and bus steal; raw framebuffer and D8; card rendering; verification strategy.

Per-area option tables, adversarial passes and sources are in the `06-DISCUSS-*.md` files in this directory.

| Area | Selected | Rejected alternatives |
|------|----------|-----------------------|
| Evidence posture | Read the STIC pages first (oracle plan, no C); every register, card and window fact is tier 4 or default, claims provisional | Treating community register maps as primary; skipping the page reading |
| STIC device | Real `stic14` memory-system device, side effects in the bus wrapper, a const inspection call | Side effects in memory-system reads; a peek that switches mode |
| Access windows | Pure function of frame position with tagged community figures, applying with display off | A copy-mode event machine as the mechanism (kept as the alternative) |
| Timeline | 59,736-tick constant frame, frame index as state, INTRM level for 3,791 cycles, 13 bus windows of 114 cycles via the existing grant units | Deriving frames by modulo; a CPU-cycle budget per frame |
| Rendering | Row-granular BACKTAB fetch latch, frame-granular render at the frame end | Pure frame-granular render at one instant; scanline-accurate rendering now |
| D8 geometry | 159 x 96 display rows, width from `video.last_card_column_px`, color numbers 0..15, `0xFF` not displayed | 160 columns; a palette or RGB in the core; the 167 x 105 nominal grid |
| Frame API | `IVS_RUN_F_STOP_ON_FRAME`, `IVS_STOP_FRAME`, video capacity `+2`, per-frame digest chain | Leaving the flag reserved; the Phase 4 `+1` formula; per-call chain framing |
| Cards | Pure renderer, FG/BG plus Color Stack (Colored Squares cut line), hand-written literal frames, GRAM-only fixtures | A second renderer; real GROM glyphs; generated expected frames |
| Verification | Oracle-first plan, third original diagnostic with input-selected mode, small lever pass first, eleven plans | A second cartridge; project ISR in the stub now; a mini-assembler now |

Conflicts between the five notes and how they were resolved are in 06-CONTEXT.md section "Reconciliation".
