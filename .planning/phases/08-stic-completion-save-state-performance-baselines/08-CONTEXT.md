# Phase 8: STIC Completion, Save State and Performance Baselines - Context

**Gathered:** 2026-11-14
**Status:** Ready for planning
**Mode:** `--auto` with the fan-out protocol (`.planning/AUTONOMY.md` section 2). Four parallel research agents, one per decision point: STIC completion (`08-DISCUSS-stic.md`, S1-*), snapshot and state (`08-DISCUSS-state.md`, S2-*), benchmarks (`08-DISCUSS-perf.md`, S3-*), milestone close and release (`08-DISCUSS-release.md`, S4-*). This file adopts one recommendation per point and resolves the conflicts. The notes carry the evidence; this file carries the decisions. Labels in the notes: [OBS] read or measured, [INF], [REC], [UNK], [BG].

<domain>
## Phase Boundary

Phase 8 closes the milestone: (1) the remaining STIC features (MOBs, border, horizontal and vertical delay, collision registers) each get a single-feature fixture with an independently derived expected result, or sit on an explicit `unknown-default-chosen` list; (2) one canonical serializer yields both snapshots and digests, and snapshot, restore and run equals the uninterrupted run at hard points; (3) named workloads get a benchmark tool that asserts the digest before reporting throughput; (4) the milestone-close record and a release-readiness assessment for the exact tested commit, followed by the delegated release flow.

Not in scope: any measured-hardware claim (D10), any real EXEC/GROM run (D11), PAL, ECS, a reference host frontend, 192-line video publication (D8 stays), performance optimization beyond what the baselines justify, D13.
</domain>

<decisions>
## Implementation Decisions

### STIC completion (S-1, VID-05)

- **D-01:** Evidence reality: no tier 1-3 page prints any MOB, collision, delay or border-extension bit; all bit-level semantics come from one tier-4 lineage. Every expected value is `source-expectation`, claims are provisional, nothing above that tier. Plans re-read the raw pages before freezing values (the research used a summarising fetch tool).
- **D-02:** Each feature is split into a derivable core (MOB placement with the 8-pixel field offset, zoom, 8x16 mode, flips, colour, MOB-over-MOB by lower index, PRIO clear, sticky MOB/MOB and MOB/background collision bits, delay shifting cards and MOBs together, 8-pixel border-extension strips) and a pinned `unknown-default-chosen` remainder (PRIO-set pixel semantics, collision read/clear/X=0/border-bit/offscreen/display-off, exposed-area content and top-strip unit, delay range, delay versus bus windows, half-height pixels). Every pinned default has a `default-pinned` test, a one-field flip control, `-` cells in the oracle frames and a claim exclusion.
- **D-03:** Geometry: keep 159x96 (no D8 reversal, no re-bless). The MOB model is line-resolved internally and published by a labelled even-line sampling default. Frames are derived only for sampling-invariant zooms (2x, 4x, 8x) plus two pinned sampling frames.
- **D-04:** One cartridge program `ivs-diag-stic-2` with a controller-word selector (as `diag-stic-1`), five single-feature scenarios (MOB, border, hdelay, vdelay, collision) plus an idle control. Sub-cases are in-process table tests of a pure renderer against hand frames.
- **D-05:** Oracle first, Phase 7 method, no C in the oracle plan: derivation with a blind-spec block, frames `F-M*`, a collision truth table, pair-region declarations, then a blind second read before the program and renderer plans. Oracle sections freeze after the blind-read commit (same rule as Phase 7).
- **D-06:** Collision bits stay in the register file, ORed in at FRAME_END, with or without a video buffer. A CPU write clears; a read has no side effect. This adds no hidden state. The `const ivs_memsys *` in `ivs_stic_frame_deliver` changes as needed.
- **D-07:** No change to the bus-window schedule for vertical delay. "No dependence" is pinned as a C92 update plus an owner hand-off. No assert depends on machine time.
- **D-08:** Guest programs mask collision reads with `03FF`. The "bits 15:14 read 0, other unimplemented read 1" note is logged as a C81 update only. The planner allocates the next free C numbers (C107 and up).
- **D-09:** About 5 owner-hand-off D10 lines (collision semantics, priority per pixel kind, delay range/exposed area/bus windows, extension units, half-height structure). None performed. Claims CL-0025 (pictures), CL-0026 (collision), CL-0027 (diagnostic), each provisional with explicit exclusions.
- **D-10:** Cost cap for wave A: at most 6 plans, 4 core faults, 6 mutants (release preset only), 4 controls, no new consumer replay.

### Snapshot and state (S-2, STA-01, STA-02)

- **D-11:** The digest field table and walker in `core/src/machine/machine_digest.c` become the single serializer. Each row gets a snapshot class in the unused `reserved` u16 (STATE, IMAGE_BEARING, BOUND, PAD). A behaviour-neutral refactor lands first; the existing digest literals are the tripwire.
- **D-12:** Big-endian, the project's single wire order (IVSC, digest rows, video slot header), so the snapshot payload is byte-identical to the digest row encoding.
- **D-13:** Container "IVSN v1": header (magic, format version, flags, total length, schema version, schema digest over the field-table descriptors, profile id, profile parameter digest, three image slot identities, composite digest, section table), 7 sections (domains 1, 2, 4, 16-19), SHA-256 trailer over all earlier bytes. About 4.3 KB. Never carries image bytes.
- **D-14:** New header `ivs/ivs_state.h` with four calls: `snapshot_size`, `snapshot_save` (caller buffer), `snapshot_restore` (validate everything, then commit), instance-free `ivs_snapshot_describe`; report struct follows the `struct_size` rule; reasons appended from 58. One new status `IVS_E_STATE_MISMATCH` (-24) for a well-formed snapshot that does not belong to the target. Restore works on any instance of the same profile whose slots match; phase becomes POWERED; save is refused while OFF; snapshot points are unit boundaries.
- **D-15:** Validate before mutating, using the existing CPU and STIC state validators on local decodes plus provably-true derived invariants (each with a "reachable states satisfy it" test).
- **D-16:** STA-02 oracles are independent of the shared table: restore into a poisoned target and compare the whole instance with memcmp; compare video, audio and log streams concatenated across the cut; compare all nine digests; a runtime row-omission sweep built from `ivs_machine_digest_field()` plus about 9 compile-time named faults; a machine-checked `data/expected/state-checklist.ids` (class, witness cut, test id per row).
- **D-17:** Hard points via a witness scan over the step seam, ticks pinned and re-proved by a witness test: SDBD, bus steal (3 variants), interrupt open and closed, interrupt entry, HLT, halted, faulted, reset-pending, mid-frame, PSG and envelope mid-phase, input, log, a VID-05 MOB cut. HP-LATCH is a designed program that stores into an already-fetched BACKTAB row.
- **D-18:** Host surface: `ivs-run --roundtrip-at-ticks` in-process hook (existing scenarios must reproduce their goldens), a fifth installed-consumer pass on Lamp Tone. File options (`--snapshot-out`/`--snapshot-in`) are an optional last plan, runner-only, explicit path, refused for `content user`. The core does no file IO.
- **D-19:** Rights: the writer is tested for no embedded image windows; a snapshot of a real-EXEC run is derived rights-bearing material; `.ivsn` goes into `binary-ext` and `.gitignore`. Open question 8 (persistence policy) is decided here as a delegated decision: bytes only from the core, explicit-path files in the runner, exact-build non-archival format.
- **D-20:** Ordering: STATE waves come after the VID-05 STIC state layout lands. Corruption and fuzz family includes a libFuzzer target outside the gate. Split trigger below applies.

### Benchmarks (S-3, PRF-01)

- **D-21:** `ivs-bench` in `tools/bench/` (two files, one holding the POSIX clock), calling `ivs_runner_execute` in-process. No timing in the core, in `runner/` or in `ivs-run` flags. No new dependency (no hyperfine).
- **D-22:** Iteration 0 is an untimed verification run against the committed `.digests` golden (`ivs_runner_golden_keep`); only an exact match starts warmup and timing; every timed iteration re-checks the composite digest. A mismatch prints `bench-refuse` and no timing fields.
- **D-23:** Workloads are ordinary `bench-*` scenarios registered in `data/expected/runner-scenarios.ids` with `.digests` goldens (verified by existing `runner-process` and `golden` steps): `bench-lamp-idle` (120 frames), `bench-lamp-input` (64 events), `bench-lamp-cold` (the existing pass scenario), `bench-lamp-snapshot` (after the state waves; digest equals idle). No "cpu-bound" labels. A `bench-cpu` fixture is optional.
- **D-24:** TSV output, one row per workload, three separate groups: throughput (cycles/s, frames/s on wall and CPU time, ns per guest cycle), latency (cold first and median wall), CI time (pointer to `.planning/ci-runs.tsv`, never measured here, never `gh`). Never gate on absolute throughput; fixed warmup and iteration counts, median; optional local `--max-regress` outside `gate.sh`; committed baseline is a person-written record taken on an idle host. Provenance fields (`build_id`, `optimized`, `sanitizers`).
- **D-25:** Negative control: a mutated core or golden makes `ivs-bench` exit non-zero with no `ns` field. No optimization in Phase 8 (the profile is SHA-256 over every frame, about 43%, which is the evidence design).
- **D-26:** The gate runs only the digest-asserting scenarios plus a smoke of the tool; the clock never runs in the gate. Two plans plus an optional fixture plan; land after STIC and the behaviour-changing state plans.

### Milestone close and release (S-4, REL-01)

- **D-27:** The close record is `08-VERIFICATION.md` plus `08-MILESTONE-CLOSE.md` with sections A-K (identity, main green, PR/issue triage, roadmap and lessons, requirement coverage with a caveat column, claim table generated from `data/claims.json`, rights and package audit, qualification, authority, hygiene, decision). A read-only `tools/release-readiness.sh` generates the computable parts.
- **D-28:** REL-01 completes in two stages: stage 1 inside the phase PR; stage 2 a `gsd/records-m1-close` PR after the merge (the green push run on the merge commit exists only then). REL-01 is marked complete after stage 2.
- **D-29:** Release: publish `v0.1.0` as a GitHub pre-release at milestone close, gated by checks P1-P8 (AUTONOMY sections 1 and 5 delegate `gh release`; no hard stop applies; the ROADMAP "owner gate D9" wording is stale). If any check fails, record "ready, not published: blocked by P<n>". The release is created as a draft, then published by the orchestrator after read-only checks (deleting a published release is a hard stop). Tag the artifact commit (the Phase 8 merge commit), not the records commit.
- **D-30:** Version policy keeps three separate numbers: GSD milestone label, software version (`0.1.0` in the `ivs.h` macros, no bump), format versions. A check requires tag == macros == `ivs_VERSION`.
- **D-31:** Footgun: `config.json` has `git.create_tag: true`; `gsd-complete-milestone` would create a `v1.0` tag. Set it to false and use a strict `v[0-9]*.[0-9]*.[0-9]*` release trigger. Do not stage `.planning/config.json` in an unrelated commit; make the change deliberately and separately, or pass an equivalent flag to the milestone tool.
- **D-32:** Release machinery is a wave-1 plan: `release.yml`, `ci-parity` extension, content-based `release-assets` check (not extension-based; the `.tar.gz` assets would trip the binary-ext rule), `CHANGELOG.md`, a rehearsal script, the text claim scan extended to `CHANGELOG.md` and release bodies, the release job runs the consumer against the very tarball it ships.
- **D-33:** History: six old planning commits on public main trip `hygiene --history` on a `a scratch-directory path` shape. A rewrite is an owner-only hard stop and would not purge `refs/pull/*`. Delegated decision: accept without rewrite, pinned by sha; a release check fails on any new finding; do not move the baseline or widen the rule; the readiness record prints FAIL-with-disposition, never "hygiene pass".
- **D-34:** At least nine stale statements are fixed before the tag (ROADMAP Phase 8 gates and SC5 D9 wording, REQUIREMENTS open D6/D7/D8/D12 list, STATE.md, PROJECT.md "Validated: none yet", README "No release has been published", and others the sweep finds). Backfill `requirements-completed` frontmatter in SUMMARYs (missing for Phases 4-7) so the milestone audit does not mark requirements partial; the six `draft` VALIDATION.md files are recorded as accepted tech debt.
- **D-35:** Lifecycle after the phase: milestone audit, then complete-milestone on a records branch (irreversible archive), then cleanup.

### Plan structure and cost

- **D-36:** One phase, about 22 plans (range 19-25), waves in order: D1 release machinery (and the CI-cost checkpoint line), A STIC (6 plans), B snapshot/state (about 7 plans), C benchmarks (2-3 plans), D2/D3 stale-text sweep and close-out. A precedes B because collision, MOB and delay state belong on the hidden-state checklist. Pre-committed split trigger: wave A over 9 plans, or a `lever-required` cost checkpoint, then Phase 9 = B+C+close.
- **D-37:** CI cost: no new job, matrix or cache. Estimated additions: wave A +8.7 hosted cpu-s, wave B +12 to 16, bench about +0.4 local; combined median r from 7.78 to about 8.1-8.2 (trigger 8.3, budget 7.8). Mitigations: mutants and the snapshot point sweep on release only (count by arithmetic), one checkpoint line after the restore plan, lever (2) already queued (mutant inventories on release preset only; apply it early if the checkpoint projects over 8.0).

### Conflict resolutions

- Ordering conflict: S4 puts release machinery first, S2/S3 say after behaviour changes. Resolution: release machinery first (it changes no machine behaviour and the release rehearsal needs it early), then A, B, C.
- Endianness: S2 chose big-endian; the requirement says "endianness-independent". Resolution: the encoding is explicitly specified big-endian field by field, independent of host byte order and struct layout.
- Plan count: the S1+S2+S3 sums (6+7+3) plus S4 machinery and close-out reach about 22; S4's split trigger is adopted as the escape valve.
- REL-01 timing: S4's two-stage completion is adopted over marking it complete inside the phase PR.

### Hard stops that apply here

No rights-bearing bytes; no "Intellivision" in names; no absolute home or scratch paths in tracked files; executors never use `gh` or push; never stage `.planning/config.json` or `.planning/state.json` (except the deliberate D-31 change by the orchestrator); D10 and D11 not performed; no history rewrite; commit trailer exactly `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`.
</decisions>

<canonical_refs>
## Canonical References

- `.planning/phases/08-stic-completion-save-state-performance-baselines/08-DISCUSS-{stic,state,perf,release}.md` (evidence)
- `AGENTS.md`, `.planning/AUTONOMY.md`, `.planning/PROJECT.md`, `.planning/REQUIREMENTS.md`, `.planning/ROADMAP.md` Phase 8, `.planning/CI-EFFICIENCY.md`, `.planning/ops/EXEC-RULES.md`, `.planning/ops/PLAN-LESSONS.md`
- Phase 6 and 7 artifacts for the oracle-first, blind-read and frame-pair machinery (`07-CONTEXT.md`, `07-VERIFICATION.md`, `docs/video/*`)
</canonical_refs>

<deferred>
## Deferred Ideas

- 192-line video publication (would add about 2 plans and a re-bless of every frame digest).
- Snapshot file options if the plan budget is exceeded (optional last plan in wave B).
- libFuzzer target runs in CI (stays outside the gate).
- Optimization of SHA-256 frame hashing (evidence design; revisit only with a measured need).
- D10, D11, history rewrite, name clearance: owner items.
</deferred>

## Planning resolutions (orchestrator, after research)

- **D-38:** A new `state` gate step (steps 36 to 37) holds the checklist and hard-point verification; no existing checker is stretched.
- **D-39:** `bench-lamp-snapshot` is a row in a `workloads.tsv` (workload table read by `ivs-bench`), not a runner scenario. The other bench workloads stay ordinary scenarios (D-23).
- **D-40:** "Release-only" sweeps do not become `skip-ok` ids (that would raise `tests-skipped`). The point set shrinks inside one test id on the dev/sanitizer preset and runs fully on release.
- **D-41:** Restore verifies the SHA-256 trailer and structure before commit; per-section digests stay a test oracle (composite digest compared after restore).
- **D-42:** The `git.create_tag` change in `.planning/config.json` is an orchestrator-only commit. Executors never stage `.planning/config.json` or `.planning/state.json`.
- **D-43:** Seven gate checks break at Phase 8 commits and each gets its same-commit retarget (api-effects status band, important-gates requirement list, video checker grammar for `-` cells and MOB rule ids, ci-parity per-file branch for `release.yml`, text.sh scope for `CHANGELOG.md`, c-build tools/bench inventory, pkg-install header set); see `08-RESEARCH.md` section on registration ledger.
- **D-44:** Plan numbering: 08-01..08-02 release machinery (D1), 08-03..08-08 STIC (A), 08-09..08-15 state (B), optional 08-16 file options, then benchmarks (C), then stale-text sweep and close-out (D). Cost checkpoints at the end of A, mid B and the end of B.
