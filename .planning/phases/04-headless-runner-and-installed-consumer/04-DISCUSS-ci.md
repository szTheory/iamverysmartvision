# Phase 4 discuss, decision point E: CI shape (D12) and gate evolution

Labels: [OBS] read or ran it this session; [INF] inference; [REC] recommendation; [UNK] unknown;
[BG] background knowledge not re-fetched. Scope: PKG-03, PKG-04, RUN-04 (and the CI cost of RUN-01/05/06, PKG-01).
Read: AGENTS.md, AUTONOMY.md 1-6, CI-EFFICIENCY.md, gate.yml, tools/gate.sh, steps/ci-parity.sh, run-controls.sh,
tests/faults.cmake, CMakePresets.json, core/CMakeLists.txt, REQUIREMENTS/ROADMAP/PROJECT D-rows. No `gh` calls, no emulator source.

## 1. State of play

- [OBS] `gate.yml`: one job `gate`, `ubuntu-24.04`, `timeout-minutes: 10`, triggers `pull_request` and `push: main`,
  top-level `permissions: contents: read`, `concurrency: gate-${{ github.ref }}` with cancel-in-progress for PRs only,
  `actions/checkout` pinned by SHA with `persist-credentials: false`, steps `cmake --version` and `sh tools/gate.sh`. No artifacts, no cache, no matrix, no path filter, no secrets.
- [OBS] `ci-parity` already enforces: only those two `run:` lines, every `uses:` SHA-pinned, no `if:`, no `continue-on-error`, no `pull_request_target`, permissions exactly `contents: read`, no flow-style mappings. It does NOT forbid `actions/cache`, `upload-artifact`, `secrets.`, `strategy:`/matrix, or any other pinned action. PKG-03's "never caches or uploads" is therefore policy-only today.
- [OBS] Hosted timing (CI-EFFICIENCY): gate-time 30-49 s on identical code (36, 44, 30, 33, 39, 38, 49), budget 40 s, trigger 50 s. Cold `c-build` 16-26 s (two presets plus the `ivs_faults` aggregate), `controls` 13-24 s overlapped (controls-wait 0, so steps are the critical path), `cp1610-vectors` 4-5 s, `c-faults` 1-5 s. Queue 2-4 s, checkout 1-2 s (rows of 2026-11-09/10).
- [OBS] The fault build already compiles only the owning TU per fault (plain object copies are shared; `tests/faults.cmake`). The "owning-TU rebuild" lever in CI-EFFICIENCY is therefore largely already structural; its remaining cost is one compile+link per fault (115 today).
- [OBS] Core is a STATIC library only (`add_library(ivs_core STATIC`); no SHARED, no rpath/install_name anywhere in CMake.
- [OBS] `github` context has no run-start or creation timestamp and `job` context has no start time (docs.github.com contexts page, first 100k chars read). In-job queue measurement is not possible from context. `github.run_attempt` exists.
- [OBS] `build/gate-linux.log` (emulated amd64, not a hosted figure): controls 95 s, faults-verdict 34, c-build 32, hygiene-history 26. Ordering only; not used for budgets.
- [UNK] Whether `main` already has a ruleset (no `gh`). [UNK] Hosted CPU saturation (4 vCPU for public-repo standard Linux runners is [BG]); nothing in the gate prints CPU time or core count.

## 2. Lenses (condensed)

- **Emulator/QA**: CI's job is "same evidence as local, visibly". Silent skips are the failure class to fear (RUN-02 "missing fixture" will exist as a *normal outcome* for owner-local EXEC runs, D11). A skip that exits 0 and is not counted is a false green.
- **Security/rights**: no EXEC/GROM ever exists in CI by construction (refs/ is gitignored; Phase 3 tests use synthetic stand-ins). The enforceable rule is structural: no workflow step can cache/upload/download anything. Fork PRs run attacker-controlled gate code with a read-only token and no secrets; the residual risk is abuse of compute, bounded by `timeout-minutes` and the first-time-contributor approval setting [BG].
- **DevOps**: public repo means standard-runner minutes are free [BG]; the scarce resources are wall time and review attention, so "evidence-driven" is about latency, not money. 4 vCPUs shared by the cold C build and the 4 control lanes means more overlap has diminishing returns [INF].
- **API/integrator, DX**: the PR check list should explain itself: a reviewer should see skips, missing and slowest steps without opening the log. Local `sh tools/gate.sh` stays the single command (PKG-02), so any CI-only logic must live inside gate.sh/helpers, not in the workflow.
- **Product**: Core Value says claims are bound to evidence; "CI green" must mean "inventory complete, zero missing", already encoded in tokens (`tests-missing=0`).

## 3. Sub-decisions with options

### E1. Triggers, concurrency, path filters (keep)
Options: (a) keep as is; (b) add `paths-ignore` for docs; (c) add `workflow_dispatch`/merge_group.
- Path filters: reject. A skipped required check by path filter stays "Expected - waiting" and blocks merge on docs-only PRs [BG], and this gate validates planning/docs/data too (CI-EFFICIENCY already says so). Anti-pattern: a filter that a required-check ruleset cannot satisfy.
- Keep `push: main`: AUTONOMY section 5 releases "the exact commit that passed CI on main"; the main run is the receipt for that SHA even though the PR ran the merge ref. Duplicate ~30 s of free compute is the price. Never cancel main runs (already so).
- Keep `cancel-in-progress` for PRs only. A cancelled superseded run is not a required-check state that blocks (the newer run supplies the check).
- No `workflow_dispatch`, no merge queue: re-run exists in the UI/gh; solo flow. [REC] unchanged.
- `timeout-minutes: 10` -> 5 (worst gate 49 s; hung runner/consumer should die early). The runner's own cycle-bound and wall limits make hangs typed failures first; the job timeout is only the backstop.

### E2. macOS leg (keep D-24: none)
Evidence for: the only two hosted-only escapes were gcc/Linux-only; macOS is exactly what the owner runs locally, so Apple-only escapes are caught pre-push. Core is static-only, so "relocated prefix" has no rpath/install_name/dylib semantics that differ by OS [OBS]. Cost of a leg: a second cold build (+20-40 s of wall only if parallel, plus its own queue; macOS runners queue longer [BG]).
- [REC] No macOS leg in Phase 4. Reversal triggers (any one): (1) first Apple-only failure that reaches `main`; (2) a SHARED library or installed dylib target is introduced (rpath/install_name); (3) a macOS-only contributor path becomes supported. If added: a second job named `gate-macos`, same `run: sh tools/gate.sh`, separate required check, no matrix (names stay stable).
- `tools/gate-linux.sh` stays local, opt-in, NOT a CI step (CI is already that environment).

### E3. Job summary (decide: yes) and what is rendered
Options: (a) log only (today); (b) extra workflow step that parses the log; (c) gate.sh writes `$GITHUB_STEP_SUMMARY` itself; (d) `::warning`/`::error` annotations.
- (b) rejected: needs `if: always()` (banned by ci-parity) and a second `run:` (banned), and parsing a log is fragile.
- (c) chosen: `gate.sh` already accumulates everything. Add two append-only files in `$GATE_TMP` (`fails`, `times`, fed by `bad()` and `step_time()`), and in the existing EXIT trap run `sh "$CHECK_DIR/ci-summary.sh"` only when `GITHUB_STEP_SUMMARY` is non-empty, writing nothing to stdout (the final line stays the `gate:` summary line, SKELETON.md section 1) and never changing the exit status (`|| true`, stderr to /dev/null). Zero workflow change, so the ci-parity "one gate run" invariant and local/CI equality hold; early `exit 1` paths still summarise because the trap runs.
- Rendered content (markdown, bounded to ~60 lines; 1 MiB per step cap [BG]):
  1. headline `gate: PASS|FAIL`, `gate-time`, `steps p/w`, `controls`, `faults`, `tests expected/executed`, compiler id, `ncpu`, CPU seconds (see E5);
  2. "Skips and missing": every summary token whose name matches `skipped|missing|pending|unknown|unsupported`, nonzero rows first; an explicit line "none" when all zero (a visible absence, not a silent one);
  3. "Slowest steps": top 8 from `step-time` plus `controls`/`controls-wait`;
  4. "Failures": first 40 `-- FAIL` lines inside a code fence (backticks stripped, since paths come from PR content: markdown-injection hygiene).
  Claims are generated from tokens only; nothing hand-written like "no system images used" (not a measured fact). The rights statement is carried by the existing `binscan`/`hygiene` tokens, which the summary lists like any other.
- (d) annotations: emit `::error title=gate::<token>` for FAIL lines (token only, never the path/detail, to keep the command-injection surface at zero; GitHub shows ~10 per step [BG]) and one `::warning title=gate-skips::<tokens>` when any skip/missing token is nonzero, only when `GITHUB_ACTIONS=true`. Cheap, optional; do it in the same helper.
- Footguns: PR-from-fork summaries are visible to anyone; they hold only tokens, step names and FAIL lines (no secrets exist). Do not print environment variables. A summary writer that fails must not fail the gate.

### E4. Artifacts, caches, rights (decide: none, enforced)
Options: (a) policy only (today); (b) upload gate log on failure; (c) structural ban.
- [REC] (c). Upload nothing: the job log already carries the gate output; summary carries the digest; build trees are reproducible and may contain test-generated stand-in images. (b) needs `if: failure()` (banned) and buys nothing.
- Extend `ci-parity` with an **action allowlist**: the only permitted `uses:` is `actions/checkout@<40hex>`; token `ci-parity-action-not-allowed: <file>:<line>`. This makes cache, upload-artifact, download-artifact, setup-*, docker actions and any future action a deliberate edit of the step plus its inventory. Also new tokens: `ci-parity-secrets` (any `secrets.` reference), `ci-parity-matrix` (any `strategy:` key; D-24 "no matrix" becomes checked), and `runs-on` must equal `ubuntu-24.04` (`ci-parity-runs-on`). Each gets a control under `tools/check/controls/` (expected tokens: the new token; the existing eight ci-parity-* controls remain). Update the step's header comment and `data/expected/controls/`.
- Repo settings to record in AUTONOMY.md (orchestrator applies once via `gh api`; agents never poll): default workflow token permissions read-only; fork-PR approval required for first-time contributors (or all outside collaborators); Actions allowed-actions limited to "this repo + actions/checkout" [BG].
- Pin hygiene: no dependabot (a PR stream for one action is triage cost); bump `checkout` at releases and record in CI-EFFICIENCY. The `ubuntu-latest` re-evaluation after 2026-11-19 stays open.

### E5. Measuring queue/setup/build/test per run (PKG-04) without agents polling gh
Options: (a) status quo: orchestrator `gh run view` and hand-written table row; (b) in-job self-measurement only; (c) `workflow_run` recorder workflow; (d) scripted orchestrator pull, one call per run, into an append-only machine-readable file.
- Queue cannot be measured in-job [OBS, no timestamp in context]. Only run `createdAt` vs job `startedAt` from the API gives it.
- (c) rejected: a second workflow with `actions: read`, triggered by `workflow_run`, is the classic privilege-escalation shape and breaks ci-parity's single-workflow, read-only contract.
- [REC] (d)+(b). In-job, gate.sh prints extra measurement lines (not tokens, nothing parses them in-gate): `gate-env ncpu=<n> cc=<id>` and `gate-cpu user=<s> sys=<s>` from the POSIX `times` builtin (cumulative children CPU; zero dependency). `step-time` lines already give setup/build/test: **build** = `step-time c-build`; **test** = gate-time minus build minus overlapped controls (record `controls` and `controls-wait` separately); setup = checkout step (API). CPU seconds / (ncpu x wall) answers the open [UNK] "are the hosted cores saturated", which decides whether more overlap can help.
- Orchestrator-only helper `tools/ci-record.sh <run-id>` (not a gate step, not in CI): two calls per run, `gh run view <id> --json createdAt,startedAt,updatedAt,conclusion,headSha,event,jobs --jq ...` (gh embeds a jq-compatible engine, `fromdateiso8601` [BG]) and `gh run view <id> --log` filtered for `gate-time`, `step-time`, `gate-env`, `gate-cpu`. Prints one TSV row. Rate cost: 2 calls per PR run, well under the shared 5000/hour.
- Record: new append-only `.planning/ci-runs.tsv`, columns `date run_id sha event conclusion queue_s setup_s build_s test_s gate_s total_s ncpu cpu_s`. CI-EFFICIENCY.md keeps prose findings and the existing legacy table (not rewritten). A cheap step `ci-log` (awk, no dependency) checks the TSV: exact header, 13 fields, numeric, unique run ids, dates non-decreasing; controls for each token (`ci-log-header`, `ci-log-fields`, `ci-log-duplicate`, `ci-log-order`). It cannot check that every run was recorded (needs gh); that stays the orchestrator's AUTONOMY section 4 duty. Cost <0.2 s.
- Decision rule (replaces eyeballing single runs; identical code ranged 38-49 s): **apply the next lever when the median of the last 3 hosted gate-times on one branch exceeds 40 s; revisit the budget itself when any run exceeds 60 s or the median of the last 5 exceeds 50 s.** The old single-run 50 s trigger fires on noise. [INF] from the 38 vs 49 s same-code pair.

### E6. Wall-time budget and levers (hosted 30-49 s; budget 40, trigger 50)
Phase 4 additions, estimated hosted [INF]: `consumer` producer 2-3 s (reuse `build/release`: `cmake --install` of the already-built tree, no third build; one-TU consumer, Ninja; the step name sorts after `c-build`), `runner` scenarios 1-2 s, ~20 new faults +1-2 s, ~30 new controls +1 s CPU, `ci-summary`/`ci-log` selftests <0.5 s. Net +5-8 s on a 30-49 s base: expect median ~38-42 s, i.e. the budget WILL be tested in Phase 4.
Levers, in the order to apply (each only after the E5 rule fires and `step-time` names the step):
1. **Keep Phase 4 steps cheap by construction** (no new cold builds, consumer reuses the release tree, runner scenarios in-process or one process per scenario, no sleeps, no wall-clock waits). This is a design constraint, not a lever.
2. **Background C build** (lever listed since D-38): start `c-build` at the top of gate.sh beside the controls, run the non-C steps (`hygiene-history`, `denylist`, `hygiene`, `discrepancies`, `text`, records: roughly 10-15 s hosted) while it builds, then `wait` before the build-dependent steps. Largest candidate (up to ~10 s) but only if cores are idle during the cold build: decide from `gate-cpu`/`ncpu`. Footguns: hidden build dependencies and hosted flakiness; mitigations: an explicit inventory `data/expected/gate-steps/needs-build.ids`, a step-inventory check that every step is listed in exactly one of {needs-build, independent}, and a control that runs the independent steps in a tree with no `build/` (must pass). Also two cold compilers + 4 control lanes on 4 vCPU may lengthen `c-build` (16 vs 26 s spread may partly be this contention [INF]); if `gate-cpu` shows saturation, instead *delay* the control lanes until `c-build` ends, because total CPU is fixed and ordering then helps nothing, so the real lever becomes less CPU.
3. **Less CPU**: control runner is sys-dominated by per-control tree copies (CI-EFFICIENCY); shrink `_good/tree` copies (hard links/cp -l is not portable to macOS; copy only files a step reads) before touching anything else. Build `dev` and `release` presets concurrently only for the link/configure tail (small, ~1-3 s [INF]).
4. **ccache / actions/cache**: still banned by policy while within budget; the E4 allowlist makes adopting it a visible two-file change. Safe w.r.t. rights (caches only compiler outputs) but PR cache scoping and a poisoned-cache story must be written first.
5. **Two parallel jobs** (needs a different command per job): breaks "no arguments" (SKELETON section 1) and ci-parity's single `run: sh tools/gate.sh`; last resort and a redefinition of PKG-02, owner-visible in the Decision Register.
Not levers: dropping controls, merging checks, weakening the refusal oracle (D-12), splitting jobs without the rule.

### E7. Branch protection on main (decide: yes, record)
- [REC] GitHub ruleset on `main` (not legacy branch protection): require a pull request (0 approvals; solo flow cannot self-approve), require status check `gate` (the job name; changing the job name or adding a matrix changes the required context, so renames must update the ruleset), block force-push and deletion (matches the AUTONOMY hard stop), not "require up to date" (PR runs test the merge ref already; strict mode would force a re-run per base move). No bypass list for non-fast-forward/deletion.
- Consequence to plan for: planning/record commits can no longer be pushed straight to `main` (current history shows direct docs commits after the merge, e.g. 8a3dc3c [OBS in git log]). Orchestrator rows go into the next phase branch or a `gsd/records-*` PR; this is also why path filters must stay off (E1).
- Orchestrator applies the ruleset once with `gh api` and records the JSON shape plus the date in AUTONOMY.md section 1. Verification: one `gh api repos/:owner/:repo/rulesets` read at milestone close, not polled. [UNK] current state.

### E8. Release readiness (design now, implement at the first release; not Phase 4 work)
- Shape [REC]: `.github/workflows/release.yml`, `on: push: tags: ['v*']`, job-level `permissions: contents: write` (only there), SHA-pinned checkout of the tag, runs `sh tools/gate.sh` on the tag commit again (free; removes the need for `actions: read` and API polling to prove "tested commit"), builds assets with `git archive --format=tar.gz --prefix=ivs-vX.Y.Z/` (deterministic) and the installed-package tarball from the Phase 4 consumer step's stage dir, writes `SHA256SUMS` via `cmake -E sha256sum`, then `gh release create` with generated notes holding profile + evidence tier + artifact lines (Core Value). Asset allowlist = tracked files filtered by fixture manifests (`redistributable: yes`) plus the install tree; a release control is a `release-assets` check that every asset path is tracked and not in `binary-ext.txt`.
- ci-parity needs a deliberate extension then (permissions `contents: write` allowed only job-level in release.yml; `run:` allowlist for that file). Do it in the release PR, not now: the current strictness protects PKG-03.
- Tag ruleset: tags `v*` creatable only after `main` is green; no deletion/update. Attestations (`id-token`/`attestations: write`) deferred to when someone consumes them. CD review of the release run per AUTONOMY section 5.
- Phase 4 obligation only: the consumer step leaves a reproducible install tree and the gate records `cmake`/compiler versions so a release can cite them.

### E9. Security posture (summary; most already enforced)
`pull_request` only (never `_target`), `permissions: contents: read` top-level, SHA-pinned single action, `persist-credentials: false`, no secrets (now banned), no `if:`/`continue-on-error`, no untrusted `${{ }}` in `run:` (only constants run), no artifacts/caches (now structurally banned), `timeout-minutes: 5`. Residual: a fork PR can alter gate.sh itself; read-only token and no secrets cap the blast radius; approval gate for outside contributors. Summary/annotations print tokens and FAIL lines only. Known gap: ci-parity is line-based and does not interpret YAML anchors or block scalars (documented fail-closed limits).

## 4. RUN-04 policy: named-fault controls

**Definition [REC]. An important gate is a check whose silent pass would allow a requirement, claim or evidence tier to be asserted without evidence.** Operationally a gate is important when at least one holds: (1) it is the only automated evidence for a REQUIREMENTS id or a `claims` entry; (2) it is of a kind that can pass vacuously (an inventory/expected-vs-actual comparison, a digest or golden comparison, an exit-code or outcome classifier, a refusal/no-state-change check, a verdict over producer output); (3) its failure class has already escaped once (the two hosted-only gcc bugs). Pure formatting/hygiene lints are not important gates unless (3).

**Mechanisms (three, all existing patterns; choose by where the defect lives):**
- **Fault** (`data/expected/faults/<suite>.ids`, `IVS_FAULT_<ID>` site, `ivs_fault_<ID>` exe, `faults-verdict`): behaviour in C code (core or runner library). Must-fail vector, allowed reason prefixes, must-pass vector make "fails for the intended reason" machine-checked.
- **Control** (`tools/check/controls/<name>/`, EXPECT/FORBID tokens): gate scripts and pure verdict steps; the token is the intended reason.
- **Mutant** (new, step-internal): for CMake/install/process behaviour no fault site can hold (the installed consumer), the producer step runs named broken variants and records `mutant <name> failed-with <token>` lines to `build/gate/<step>/` with the nonce; a pure `*-verdict` step (control-driven, like `faults-verdict`) judges them. Copy-paste of the c-faults/faults-verdict split, no dependency.

**Registry**: `data/expected/important-gates.ids`, one line per (gate, mechanism, id, requirement): `<gate> <fault|control|mutant> <id> <REQ>`. A cheap awk step `important-gates` (<0.2 s) checks each id exists in the matching inventory (`faults/*.ids`, `data/expected/controls`, mutant list) and that every REQ listed as Phase 4 in this file has at least one line. Controls for it: `important-gates-unknown-id`, `important-gates-uncovered-gate`. This is the machine-checked form of RUN-04; without it "each important gate has a fault" is a claim.

Cost note: faults ~0.02-0.3 s each in 4 lanes plus one TU compile+link each in the cold build; a control ~0.1-0.4 s CPU. Prefer controls/mutants over faults wherever the defect is in a script or CMake.

**Phase 4 inventory (ids proposed, final names belong to the plans):**

| Gate (requirement) | Mechanism | Named faults / controls (intended reason in token) |
|---|---|---|
| Run bound: absolute cycle target, overshoot reported, slicing invariance (API-02) | fault, suite `run` | `RUN_TARGET_RELATIVE` (target treated as per-call delta), `RUN_OVERSHOOT_DROPPED`, `RUN_SLICE_LOSES_CARRY`; vectors: slicing-invariance cases fail with `run.slice.` prefix |
| Output buffers refused before any state change (API-04) | fault, suite `run` (extends api-effects table) | `OUTBUF_CHECK_AFTER_STEP` (digest differs after refusal), `OUTBUF_PARTIAL_WRITE` |
| Per-domain digests and repeat-run equality (RUN-05) | fault + control | `DIGEST_DOMAIN_SWAPPED`, `INPUT_LOG_NOT_DIGESTED`, `NONDETERMINISM_SEAM` (a fault-only tick source varying per run: repeat-run gate must fail `digest.repeat.`); control `golden-header-missing-tier`, `golden-header-missing-oracle`, `golden-mismatch` |
| Runner outcome classification (RUN-01/02): five outcomes distinct | fault, suite `runner` (outcome mapping is a pure function in a runner library so a fault site can sit in it) | `OUTCOME_GUEST_AS_HARNESS`, `OUTCOME_TIMEOUT_AS_GUEST`, `OUTCOME_MISSING_FIXTURE_EXIT0`, `OUTCOME_CANCEL_AS_TIMEOUT`; control `runner-exit-code-merge` |
| Guest predicate end-to-end from reset, independent expectation (RUN-06) | fault + control | fault `BOOT_STUB_WRONG_ENTRY` (predicate not reached: outcome must be guest failure, not pass), control `diag-fixture-mutated-decle` (one decle changed: predicate fails with the guest-failure token), `diag-predicate-vacuous` (predicate true on an untouched machine must be refused), `fixture-hash-mismatch` (manifest) |
| Installed consumer in relocated prefix (PKG-01) | mutant + control | mutants `consumer-empty-prefix` (find_package fails, token `consumer-find-package`), `consumer-prefix-not-moved` (old stage path must fail after the `mv`), `consumer-missing-exercise` (consumer without refusal/run/digest lines fails against `data/expected/consumer.ids`); check `consumer-abs-path` (installed CMake files contain no `$ROOT` or stage path); verdict-step controls for each token |
| CI parity extensions (PKG-03) | control | `ci-parity-action-cache`, `ci-parity-action-upload`, `ci-parity-secrets`, `ci-parity-matrix`, `ci-parity-runs-on` |
| CI summary (PKG-03) | control | `ci-summary-skips-section-missing`, `ci-summary-failures-missing`, `ci-summary-token-escape` (backticks stripped), `ci-summary-unwritable-keeps-exit` (unwritable path does not change gate status) |
| CI log TSV (PKG-04) | control | `ci-log-header`, `ci-log-fields`, `ci-log-duplicate`, `ci-log-order` |
| Registry itself (RUN-04) | control | `important-gates-unknown-id`, `important-gates-uncovered-gate` |

Rules: a fault/control must name the reason (EXPECT token or allowed prefix), not merely "fails"; a new important gate lands with its entry in the same plan; a gate that escapes (hosted-only or later regression) adds a fault/control in the fix commit (class (3)). Host-wall-clock behaviour (runner wall timeout) is tested with a deterministic hook (zero or tiny budget, scripted cancel point), never with sleeps or signals racing the gate (flaky-gate footgun; core stays clock-free).

## 5. Adversarial pass

- "Summary in gate.sh bloats the critical file." It is a ~40-line helper behind an `EXIT` trap, off by default, with its own controls; the alternative (workflow step) needs `if: always()` and a second `run:`, both banned for good reasons.
- "The action allowlist is rigid." That is the point: PKG-03 is "never caches/uploads"; one pinned action stays reviewable. Cost to reverse: one allowlist line plus a control.
- "Median-of-3 rule hides a real regression." A step that jumps by more than 5 s shows in `step-time` immediately and AUTONOMY's per-run review still applies; the rule only governs when to *apply* a lever, and the 60 s single-run revisit catches cliffs.
- "ruleset blocks the orchestrator's record commits." True; accepted cost (E7), mitigated by batching into phase branches; reversal is a bypass entry, which is a visible policy change.
- "No macOS leg risks Apple-only escapes." Escapes are caught on the owner's laptop first; trigger list in E2 makes the reversal evidence-based, and a leg is an additive job.
- "Mutants might pass vacuously." Hence the split producer/verdict with nonce and the control-driven verdict, the same design that protects faults-verdict.
- "Consumer reuse of `build/release` is not an independent build." Independence for PKG-01 is the install + relocation + separate project + no access to repo headers (consumer compiled with only the prefix on its include path; abs-path check), not a re-compile; a rebuild would add ~10 s cold and test nothing more for a static library.
- "Recorder script is untested." It runs only for the orchestrator; its output format is gate-checked via the TSV step. A bad row is a visible validation failure, not silent drift.

## 6. Risks / reversal / unknowns

Top risks: (1) Phase 4 steps push hosted median past 40 s and the cold build is CPU-saturated, so overlap levers underdeliver (mitigation: `gate-cpu`, steps reuse trees, rule in E5); (2) ruleset friction for record commits and a renamed job silently unrequiring the check; (3) summary/annotation code leaking untrusted PR text or altering exit status (mitigated: tokens only, fenced, `|| true`, controls).
Reverse if: hosted median >50 s for 5 runs despite levers (then two-job split, owner-visible); a macOS-only failure reaches main (add `gate-macos`); a legitimate need for an artifact/cache arises with a rights story (extend allowlist deliberately).
[UNK] blocking planning: none. Non-blocking: current ruleset state; hosted vCPU count/saturation (resolved by the first run with `gate-env`/`gate-cpu`); exact `gh --jq` support for `fromdateiso8601` (verify when writing `tools/ci-record.sh`; fall back to awk civil-date conversion).
