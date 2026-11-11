# Phase 4: Headless Runner and Installed Consumer - Discussion Log

> **Audit trail only.** Decisions are in 04-CONTEXT.md.

**Date:** 2026-11-11
**Mode:** `--auto` with fan-out (one research agent per area; owner delegation in `.planning/AUTONOMY.md`). No AskUserQuestion; the synthesized recommendation was auto-selected per area.
**Areas:** run API and digests; headless runner; diagnostic, boot stub and D4; packaging and installed consumer; CI shape D12; D6 controller input.

Per-area option tables, adversarial passes and sources are in the `04-DISCUSS-*.md` files in this directory.

| Area | Selected | Rejected alternatives |
|------|----------|-----------------------|
| Run bound | One stateless `ivs_machine_run` with an absolute master-tick target, overshoot derived | Relative cycle budget with carry; per-microcycle ticking; stateful set-target |
| Observation | Public read-only `cpu_state` + `peek` | Internal-only hooks; callbacks; access logs in the public API |
| D6 | Logical switch state in, electrical derived in-core (Phase 5) | Electrical per-port bytes as the core input; both input paths |
| Scenario format | Line-based IVSS v1, hand-written parser | JSON (dependency or large parser); Lua/INI |
| D4 | Hand-assembled annotated listings + listing checker | Pinned as1600 (GPL); libasm gating; project assembler now |
| Boot path | Project boot stub through the EXEC slot, anti-HLE guards | Built-in EXEC; HLE; fetched image |
| Packaging | Static `ivs::core`, relocated-prefix consumer, `pkg-install` step with in-step selftests | Shared library now; per-case control-runner controls (a full build each) |
| D12 | One ubuntu job, step summary, no cache/artifacts, run record TSV | macOS leg; path filters; two-job split; ccache |

**Confidence:** about 70% overall; weakest points: unit-start input granularity (goldens change when refined), tier-4 control table, zero-valued speculative API fields before video/audio exist.
