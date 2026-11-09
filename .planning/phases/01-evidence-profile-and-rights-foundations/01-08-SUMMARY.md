---
phase: 01-evidence-profile-and-rights-foundations
plan: 08
subsystem: time-base
tags: [time-base, glossary, decision-d2, gate-step, negative-controls, fnd-07]
requires:
  - phase: 01-evidence-profile-and-rights-foundations
    provides: "gate, step and control contracts (plan 01-01); profile field inventory data/expected/profile-fields.ids (plan 01-01); SRC registry (plan 01-14)"
provides:
  - "docs/time-base.md: D2 decision record with rationale and reversibility, NTSC nominal numbers with tags, nine glossary entries, units-in-records rule, Phase 2 queue, integrator note"
  - "data/expected/glossary-terms.ids: exact nine-term inventory"
  - "glossary gate step: term inventory, bare-unit tripwire (R2) over README.md and docs/, D2 link check, unit-suffix rule over profile field paths"
  - "Four negative controls: glossary-missing-term, glossary-bare-clock, glossary-d2-unlinked, glossary-unit-suffix"
  - "Decision records aligned: PROJECT.md D2 row links docs/time-base.md; ROADMAP and REQUIREMENTS gate tables show D1, D2, D3, D9 as delegated decisions"
affects: [every later doc, trace, golden and API (one time unit), plan 01-09 onward (R2 now enforced over all docs), Phase 2 (queued unit-swap control and frame check)]
actuals:
  tokens: 17800
  tasks: 2
  commits: 2
plan_head_before: f9f477eb0a4491ea72b029f514ab9cbe394d1981
plan_head_after: 3b59fca569994fc26c103aa81deb25af6c7de915
tech-stack:
  added: []
  patterns:
    - "Inventory terms with spaces are read as whole lines (not first token) and matched as fixed-string **term**"
    - "R2 scan uses the anchored patterns ^README[.]md$ and ^docs/.*[.]md$ so control overlays under tools/check/controls are never scanned on the real tree"
    - "Unit-suffix rule matches whole underscore-delimited tokens by padding the segment with underscores, so address_windows is not a window token"
key-files:
  created:
    - docs/time-base.md
    - data/expected/glossary-terms.ids
    - data/expected/gate-steps/glossary.ids
    - data/expected/controls/glossary.ids
    - tools/check/steps/glossary.sh
    - tools/check/controls/_good/tree/docs/time-base.md
    - tools/check/controls/_good/tree/data/expected/glossary-terms.ids
    - tools/check/controls/_good/tree/.planning/PROJECT.md
    - "tools/check/controls/glossary-* (4 directories)"
  modified:
    - .planning/PROJECT.md
    - .planning/ROADMAP.md
    - .planning/REQUIREMENTS.md
    - tools/check/controls/text-no-files/MAKE
key-decisions:
  - "D-10/D-11 unit-suffix resolution (orchestrator-decided): D-11's suffix rule applies to durations and offsets; ratio and count keys named ticks_per_X or X_per_Y name their units and comply; crystal_hz is a rational frequency read only by host adapters. Enforced mechanically by glossary check (d) and stated in docs/time-base.md. All 23 real profile paths pass; the control proves the rule can fail."
  - "PSG step wording follows corrected D-11: 16 PSG chip clocks, 32 master ticks under ticks_per_psg_clock = 2, to be checked in Phase 5; the line carries only qualified unit words"
  - "The glossary success line uses doc-files=<n>, not docs=<n>, because the text step already emits docs=<n> and the gate summary does not de-duplicate keys"
requirements-completed: [FND-07]
duration: about 25min
completed: 2026-11-09
status: complete
---

# Phase 1 Plan 08: Time-base glossary and D2 record Summary

**`docs/time-base.md` fixes the time vocabulary (master tick, CPU cycle, CPU input clock, time slot, STIC clock, PSG chip clock, PSG step, frame, crystal) and records D2, and a `glossary` gate step enforces the term inventory, the bare-unit ban and a unit-suffix rule on profile keys, each with a control that proves it can fail.**

## Performance

- **Duration:** about 25 min
- **Tasks:** 2 of 2 (tracer, then cross-links and remaining controls)
- **Files:** 24 changed in two commits (counting control files individually)

## Accomplishments

- Tracer: `docs/time-base.md`, `data/expected/glossary-terms.ids`, the `glossary` step and the `glossary-missing-term` control, green end to end before the cross-links task began (`-- glossary pass terms=9`).
- D2 record: master tick = one crystal period, `uint64_t` from reset; CPU cycle = profile ratio (NTSC 4, inference); `run(target_tick)` returns `now_tick`, `overshoot_ticks` and a stop reason; targets above 2^63 refused; Option B catch-up scheduling, no lockstep; one-way once a Phase 2 API ships; reversed only by a device timing source that is not an integer divisor of the crystal.
- NTSC nominal numbers with tags: 262 x 57 = 14,934 CPU cycles (tier 4, C7) = 59,736 master ticks (inference); a frame is a timeline event, never derived by modulo.
- Glossary check (d) unit-suffix rule: 23 profile field paths examined, none flagged; `reset.hold` in the control overlay fails as `glossary-unit-suffix: reset.hold`.
- Decision records: PROJECT.md D2 row links `docs/time-base.md` (single D2 row, no duplicate); D1, D3 and D9 rows were verified against D-01, D-02 and D-04 and needed no correction; one sentence added for D5-D8, D12 (decided by fan-out when needed) and D10/D11 (hard stops, evidence missing). ROADMAP Decision Gate Timeline and REQUIREMENTS Decision Gates show "Delegated decision 2026-11-09 (PROJECT.md)" for D1, D2, D3, D9; all other rows untouched.

## Task Commits

1. Task 1 (tracer): `6284b2a` feat(01-08): time-base glossary, D2 record and glossary step with missing-term control
2. Task 2: `3b59fca` feat(01-08): unit-suffix rule, decision-record cross-links and three glossary controls

## Verification (executed)

- `sh tools/gate.sh` and `dash tools/gate.sh`: `gate: PASS steps=11/11 controls=68/68 claims=4 provisional=3 scanned=36 waived=0 discrepancies=35 pending-resolvers=18 open=33 tracked=342 binaries-listed=0 terms=9 doc-files=7 profile-keys=23 files=342 manifests=1 effective-yes=0 effective-no=1 records=23 fields=23 unknown-fields=12 sources=9 refs-verified=1/4 docs=7 claim-refs=2 tags=10 tiers=8`.
- `sh tools/check/run-controls.sh | tail -1` is `controls=68/68`; the four glossary controls and `_good/glossary` report ok.
- Task 1 acceptance greps all matched: `terms=9`; one `| D2 |` row containing `docs/time-base.md`; nine `**Term**:` entries; `59,736` and `14,934` present; the PSG step line carries `16 PSG chip clocks`, `ticks_per_psg_clock` and `32 master ticks`; R2 and R1 greps over `docs/time-base.md` print nothing; `^docs/` anchor present in the step.
- Task 2 acceptance greps: four D1/D2/D3/D9 rows with the delegated status in each of ROADMAP and REQUIREMENTS; `profile-keys=23` on the gate line.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] `text-no-files` control broke once `docs/time-base.md` joined the good tree**
- **Found during:** Task 1 gate run (`control-token-missing: text-no-files`, controls 64/65).
- **Issue:** the plan 01-04 control deletes `docs/evidence-tiers.md` to leave zero scanned docs; the new good-tree `docs/time-base.md` meant one doc remained, so `text-no-files` no longer fired.
- **Fix:** added the line `delete docs/time-base.md` to `tools/check/controls/text-no-files/MAKE`. The control still expects `FAIL text-no-files:` and nothing was weakened.
- **Files modified:** `tools/check/controls/text-no-files/MAKE`
- **Commit:** `6284b2a`

### Changed from the plan wording (no check weakened)

- **Stat key:** the plan named the doc-count stat `docs=<n>`; the step prints `doc-files=<n>` because the text step already emits `docs=<n>` and the gate summary does not de-duplicate keys (same reason as plans 01-06 and 01-07). `terms=<n>` and `profile-keys=<n>` are as planned.

### Notes (not deviations)

- FND-07 is marked complete for the Phase 1 scope the plan states: vocabulary enforced in docs (R2) and in profile keys (inventory plus unit-suffix rule). The plan's flagged assumption stands: "used consistently in code" cannot be checked while no code exists, and the code-level check (unit-swap named-fault control, 59,736-tick frame check, header freshness) is queued for Phase 2 in `docs/time-base.md` under `## Queued for Phase 2`. FND-01 was already complete.
- The R2 scan now covers every `docs/**/*.md` file in the real tree, not only `docs/time-base.md`; the existing docs (RIGHTS, evidence-tiers, fixture-manifest, provenance, cp1610 notes) already passed it unchanged.
- The CPU input clock derivation referenced by the profile (`derivation in docs/time-base.md`) is in the CPU input clock glossary entry: crystal divided by 2, about 1.79 MHz, one CPU cycle about 1.1175 microseconds [INF].
- No claim ids are cited in `docs/time-base.md` because the claim ledger lands in parallel; the only source id cited is `SRC-gi-cp1600-um-1975`.
- The `.planning/config.json` modification predates this plan and was not staged.
- The records DSL whitespace limitation noted by plans 01-04 and 01-05 does not affect this plan: the glossary step is plain sh, not a schema kind.

## Known Stubs

None.

## Threat Flags

None. T-01-21: term inventory, bare-unit tripwire and unit-suffix rule each have a control. T-01-22: the D2 link is gate-checked (`glossary-d2-unlinked`) and the table statuses are verified by acceptance greps. No package installs occurred.

## Issues Encountered

None beyond the auto-fixed control above. A full gate takes about 3 to 7 seconds.

## Next Phase Readiness

Docs written by later plans must avoid the bare unit word (use "crystal", "master tick", "CPU cycle", "CPU input clock", "STIC clock" or "PSG chip clock", or `[nc]` on a line that must name it). New profile fields holding a duration or offset must end in `_ticks` or `_cpu_cycles`. Phase 2 inherits the three queued items in `docs/time-base.md`.

## Self-Check: PASSED
