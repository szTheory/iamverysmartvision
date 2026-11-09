# iamverysmartvision project instructions

This is the working project name for an Intellivision emulator. Use this file
and project-owned GSD records as current policy. Bootstrap notes are dated
research input; verify material claims before turning them into requirements.

## Owner direction and workflow

- Aim for a high-quality, accurate and useful Intellivision emulator. A narrow
  first delivery is sequencing, not a permanent ceiling on the goal.
- Reuse relevant lessons from sibling emulator projects and LibFuturo, but do
  not transfer their hardware assumptions, APIs, performance budgets, stale
  gates or licenses.
- Automate repeatable verification as early as it gives trustworthy feedback.
  Keep test inventories exact, make missing evidence visible and use meaningful
  negative controls. Reserve human checks for subjective judgment,
  physical/external evidence and decisions tools cannot make.
- Keep the local fast path aligned with the fast PR gate. Add CI checks only
  when they catch a distinct recurring failure at a useful cost. After every
  CI run, inspect measured opportunities to reduce wait or cost without
  weakening evidence.
- At milestone discussion, inspect current project state and relevant evidence;
  fan out by decision-relevant technical, product, security, QA and DevOps
  lenses; compare alternatives and failure modes; include an adversarial pass;
  stop when more research stops changing the recommendation. Keep a rolling
  roadmap.
- At milestone close, keep main green, triage open PRs, update roadmap and
  lessons, and target a release from the exact tested commit whenever content
  rights, package readiness, qualification and authority allow it. GSD
  milestones and software versions are separate.
- Proceed autonomously on routine reversible work using the configured GSD
  workflow. Stop for material owner choices, unresolved rights, external
  publication, physical qualification or another explicit safety boundary.
- **Autonomous operation (owner decision, 2026-11-09):** the owner has
  delegated decision gates, public GitHub publication via `gh`, frequent
  releases, keeping `main` green and PR triage. Unblock yourself by following
  your own researched recommendations. The remaining hard stops, the fan-out
  discuss protocol, the shift-left verification protocol, the CI efficiency
  review and the release/hygiene steps are in `.planning/AUTONOMY.md`. Where
  that file conflicts with the stop rule above, that file takes precedence.

## Engineering and evidence

- Keep deterministic machine behavior separate from filesystems, wall clocks,
  physical controller discovery, presentation, audio devices and storage
  policy. State ownership, input timing, execution bounds, outputs, lifecycle
  and refusal effects explicitly.
- Keep machine time/progress, logical input, raw video, raw audio and host
  presentation as distinct observations. Repeated hashes establish
  repeatability, not accuracy; shared-core agreement is not an independent
  oracle.
- Prefer a small dependency tree and understandable components. Add a
  dependency for a concrete capability whose maintenance cost is justified.
- Treat binaries, diagnostic ROMs, EXEC/GROM images, manuals, overlays and
  other third-party material as rights-bearing. Do not commit or redistribute
  without documented permission. Record fixture source, recipe, hashes,
  license, notices and allowed use. A hash establishes identity, not rights.
- Label owner decisions, source observations, historical reports, executed
  results, inference, recommendation and unknowns separately. Code presence or
  a project report is not a fresh test result.

## Kickoff boundary

Read .planning/bootstrap/START-HERE.md, PROJECT-SEED.md and EMULATOR-DNA.md
before initialization. GSD should create the formal project, requirements,
rolling roadmap and state. Do not start phase planning or implementation as
part of bootstrap setup. Preserve owner decisions and explicit stops in
project-owned records.
