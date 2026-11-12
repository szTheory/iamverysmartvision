# Autonomous Operation Policy

**Status:** owner decision, accepted 2026-11-09 (given in the Phase 1 discuss session).
**Applies to:** every GSD step (discuss, plan, execute, verify, ship, milestone) and
every agent working in this repository. `AGENTS.md` points here.

This file records *how* the project is driven. It is policy, not research. Do not
re-ask the owner about anything this file already settles.

## 1. Owner delegation

The owner wants the emulator developed as far and as fast as possible in
autonomous mode, while keeping quality high.

- **Self-unblocking.** When a decision gate (D1–D13) or open question blocks
  progress, research it (section 2), adopt the researched recommendation, and
  record it in the `PROJECT.md` Decision Register as
  `Delegated decision (owner authorization 2026-11-09)`. Record the rationale
  and the evidence that would reverse it. Do not stop and ask.
- **Public GitHub repository.** You may create and push a public repo with the
  `gh` CLI. Before the first push and before each release, run the hygiene
  check (section 6).
- **Releases.** Cut releases often. Each release is the exact commit that
  passed CI on `main`, uses 0.x semver, and is published with `gh release`. A
  release ships only project-authored artifacts. Its notes state claims only
  as profile + evidence tier + artifact (Core Value). GSD milestones and
  software versions stay separate.
- **Keep `main` green.** Fix red `main` before starting new feature work.
  Phase work happens on `gsd/phase-<N>-<slug>` branches, because GSD treats
  `main` as protected. `main` advances only by fast-forward or merge after
  the gate passes. Once the GitHub remote exists, this goes through a PR
  with CI green.
- **Auto-triage PRs and issues.** Label them, then review, merge, close or
  answer. Merge only when CI is green.

### Still hard stops (not delegated)

| Boundary | Why |
|----------|-----|
| Committing or redistributing third-party rights-bearing material (EXEC, GROM, commercial ROMs, MTE-201, manual/datasheet scans, overlays) | Rights. A hash proves identity, not rights. Keep reference PDFs in a gitignored local cache only |
| PII, credentials, tokens or private paths in the repository or its history | Public repo |
| Physical hardware measurement (D10) and runs with the owner's real EXEC/GROM (D11) | These need the owner's hardware or files. Mark the evidence as missing and keep going |
| Irreversible destructive actions: force-push to `main`, rewriting published history, deleting the repo or releases | Cannot be undone |
| "Intellivision" in a package, product or repo name | Third-party trademark (REQUIREMENTS Out of Scope) |
| Paid services beyond free tiers | Cost |

### 2026-11-11 — Branch protection on main (Phase 4 D-47, recommended, not applied by agents)

Recommended shape: a repository ruleset on the default branch (not legacy
branch protection) that requires a pull request with 0 approvals (a solo flow
cannot self-approve), requires the status check `gate` (the job id of
`.github/workflows/gate.yml`; renaming the job or adding a matrix changes the
required context, so such a change must update the ruleset in the same PR),
does not require the branch to be up to date (PR runs already test the merge
ref), and blocks force-push and deletion with no bypass list. This is a
repository-settings action for the owner or orchestrator; executors never
call `gh` and never change settings. It blocks nothing in Phase 4: record
commits go through phase branches (or a `gsd/records-*` PR once it applies),
never straight to `main`.

Exact commands for the owner or orchestrator (run once from the repository
checkout; `{owner}/{repo}` is filled in by `gh` from the current remote):

```sh
gh api --method POST 'repos/{owner}/{repo}/rulesets' --input - <<'EOF'
{
  "name": "main: PR and gate",
  "target": "branch",
  "enforcement": "active",
  "conditions": { "ref_name": { "include": ["~DEFAULT_BRANCH"], "exclude": [] } },
  "bypass_actors": [],
  "rules": [
    { "type": "deletion" },
    { "type": "non_fast_forward" },
    { "type": "pull_request",
      "parameters": {
        "required_approving_review_count": 0,
        "dismiss_stale_reviews_on_push": false,
        "require_code_owner_review": false,
        "require_last_push_approval": false,
        "required_review_thread_resolution": false } },
    { "type": "required_status_checks",
      "parameters": {
        "strict_required_status_checks_policy": false,
        "required_status_checks": [ { "context": "gate" } ] } }
  ]
}
EOF
# one read to confirm (at apply time and at milestone close; never polled)
gh api 'repos/{owner}/{repo}/rulesets' --jq '.[] | [.id, .name, .enforcement] | @tsv'
```

After applying, record the date and the ruleset id here. Reversal: delete the
ruleset (`gh api --method DELETE 'repos/{owner}/{repo}/rulesets/<id>'`) if it
blocks a needed recovery; force-push to `main` stays a hard stop either way.

## 2. Discuss protocol (fan-out by default)

Use this at every discuss-phase and milestone discussion, and for any
non-trivial decision.

1. Enumerate the decision points. Spawn one research subagent per point, in
   parallel, to keep the main context small.
2. Each agent covers breadth and depth through every relevant role lens:
   emulator/CPU/video/audio architect, hardware/EE, QA, security/rights,
   devops/CI, API/integrator, product, DX. It weighs pros/cons/trade-offs,
   patterns, anti-patterns, footguns and lessons learned from other
   emulators and ecosystems, and runs an adversarial pass against its own
   recommendation. It prefers reputable primary sources, labels claims
   `[OBS]/[INF]/[REC]/[UNK]`, and stops at diminishing returns.
3. Synthesize one coherent, opinionated recommendation per point that fits
   the other points and the project goals. Auto-select it and record it in
   CONTEXT.md and the DISCUSSION-LOG.
4. Dependency preference: **"another copy-paste is better than another
   dependency."** Keep the dependency tree small and flat. Add a dependency
   only for a concrete capability whose maintenance cost is clearly
   justified. Third-party code is still subject to the no-copy and licence
   rules.

## 3. Verification protocol (shift left; aim for zero human UAT)

- Automate verification as early as it gives **useful** feedback: unit, seam,
  integration, end-to-end, smoke, installed-consumer and negative controls.
- Throwaway verification is fine: a one-off script or probe that answers a
  question does not have to live in CI.
- **Put a check in CI only when it has recurring value** (it catches a
  distinct recurring failure class at a useful cost). Otherwise keep it
  local or throwaway.
- Do not over-build. Do not paint the project into a corner with
  verification machinery that gives no feedback.
- Hand off to the owner only what tools truly cannot judge: subjective
  look/feel/sound, physical hardware, owner-held files. In each
  VERIFICATION/UAT record, list what was automated, what was handed off,
  and why.
- `workflow.human_verify_mode = end-of-phase`: no mid-flight human
  checkpoints.

## 4. CI efficiency review (after every CI run)

After every CI run, or when one finishes, look for low-hanging fruit that
improves wall time, iteration speed or Actions cost **without weakening
evidence**:

- queue/setup/build/test breakdown
- caching
- `concurrency` with cancel-in-progress
- path filters
- merging tiny jobs
- removing redundant matrix legs
- artifact size

Measure before changing. Append findings and actions to
`.planning/CI-EFFICIENCY.md`. Create that file on the first CI run.

## 5. CD / release steps

1. `main` is green on the release commit.
2. Run the hygiene check (section 6).
3. Version bump and changelog. Claims are evidence-bound.
4. Tag `vX.Y.Z` on that exact commit.
5. Run `gh release create` with the tested artifacts.
6. Record the release in STATE.md.

Run the CI efficiency review (section 4) on the release workflow as well.

### 2026-11-11 — Release workflow design (Phase 4 D-47, recorded; implemented at the first release)

Not implemented in Phase 4; the release PR implements it and updates
`ci-parity` in the same change.

- `.github/workflows/release.yml`, `on: push: tags: ['v*']`; `permissions:
  contents: write` at job level only (the top level stays `contents: read`);
  the same SHA-pinned checkout of the tag, `persist-credentials: false`.
- The job re-runs `sh tools/gate.sh` on the tag commit. That proves the
  released commit is the tested commit without `actions: read` or API polling.
- Assets: a `git archive --format=tar.gz --prefix=ivs-vX.Y.Z/` source tarball
  (deterministic), a tarball of the install tree from the installed-consumer
  step's stage prefix, and `SHA256SUMS` written with `cmake -E sha256sum`.
- `gh release create vX.Y.Z` with those assets; the notes state claims only as
  profile + evidence tier + artifact (Core Value).
- Assets come from an allowlist of redistributable project-authored files:
  tracked files, fixture manifests with `redistributable: yes`, plus the
  install tree; a `release-assets` check refuses any asset that is untracked or
  matches the binary-extension list. No EXEC, GROM or commercial bytes, ever.
- `ci-parity` gets a deliberate extension in that PR (job-level `contents:
  write` allowed only in `release.yml`, a `run:` allowlist for that file). A
  tag ruleset (`v*` created only from a green `main`, no update or deletion)
  is a settings action like the branch ruleset in section 1. Attestations are
  deferred until someone consumes them.

## 6. Public-repo hygiene check

Run this before the first push and before every release:

- **Author emails.** Every author email in history must be a GitHub noreply
  address (`git log --format='%ae' | sort -u`).
- **Secrets.** No secret patterns in tracked files or history (`gho_`, `ghp_`,
  `github_pat_`, `AKIA`, `-----BEGIN`, `password=`, `.env`).
- **Private paths.** No absolute home paths (`/Users/<name>`) in tracked
  project files. `.planning/` research may cite sibling projects by relative
  name only.
- **Third-party binaries.** No `.rom/.bin/.int/.itv/.cc3/.pdf` or images
  unless listed as project-authored in the fixture manifest with
  `redistributable: yes`.

## 7. GSD config mapping

`mode: yolo`, `workflow.auto_advance: true`, `workflow.human_verify_mode:
end-of-phase`, `workflow.skip_discuss: false` (discuss runs the fan-out
protocol above and auto-selects), `workflow.research: true`,
`workflow.verifier: true`. Discuss-phase runs in advisor mode (the
USER-PROFILE exists) with auto-selection of the synthesized recommendation.
