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
  physical/external evidence and decisions tools cannot make. **Shift left by
  default**: integration, seam, end-to-end, smoke and installed-consumer checks,
  aiming for zero human UAT. Throwaway verification is legitimate and should not
  be pushed into CI; CI takes a check only for a distinct recurring failure at a
  useful cost. Every new equivalence or round-trip check needs a liveness guard
  and a stand-in proving the guard can fail. Run cheap consistency checks before
  the long suites. Hand off only what tools truly cannot judge, record it as a
  ledger row with `evidence: none`, and never re-ask it in a later milestone.
- Keep the local fast path aligned with the fast PR gate. Add CI checks only
  when they catch a distinct recurring failure at a useful cost. After every
  CI run, inspect measured opportunities to reduce wait or cost without
  weakening evidence. Reproduce CI locally first (GitHub `act` for workflow
  shape, `tools/gate-linux.sh` for the Ubuntu substance) before spending hosted
  minutes. Name every docker image, container and volume `ivs-<purpose>` and
  remove it when the run is done, and clean up after yourself generally: every
  process, background job, fuzzing run, temp tree and open connection you start
  is terminated and removed before you hand off, so nothing runs away with the
  owner's CPU, memory or disk. Details in `.planning/AUTONOMY.md` sections 3, 4
  and 9.
- **Every new milestone runs the standing ritual** in
  `.planning/ops/MILESTONE-RITUAL.md`, unasked: refresh the rolling roadmap
  (near / mid / long, prioritized, every row naming its gate) → inspect state
  and recompute every figure → fan out the milestone choice by lens and
  auto-select one recommendation → fan out a separate **upstream scoping pass**
  (phase decomposition, gate cost and evidence durability, rights/format/
  packaging policy, verification strategy, downstream guardrails) so the whole
  milestone's downstream phase work is directionally correct → freeze the
  upstream artifacts and run the normal GSD cycle → hand the owner one
  `/gsd-autonomous` command. Re-triage the roadmap every time; never append to
  it.
- At milestone close, keep main green, triage open PRs, update roadmap and
  lessons, and target a release from the exact tested commit whenever content
  rights, package readiness, qualification and authority allow it. GSD
  milestones and software versions are separate. **The close ends in a clean
  state**: green `main` with the push run recorded, no dirty worktree, one
  worktree, no stashes, PRs and issues triaged, rulesets applied or explicitly
  recorded as not applied, records reconciled, one release-decision line.
- **Never re-ask what a record already answers.** `.planning/ops/INDEX.md` is
  the one-hop index of durable policy, decisions and lessons; read it before
  asking. When a decision is made, record it in the same commit so it is not
  re-litigated.
- **Recompute, never quote.** Cost medians, counts and coverage are arithmetic
  over committed files. A quoted figure that disagrees with the file is a
  defect to correct, not a fact to reuse.
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
