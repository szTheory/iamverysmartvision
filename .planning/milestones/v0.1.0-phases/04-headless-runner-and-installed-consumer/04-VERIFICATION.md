---
phase: 04-headless-runner-and-installed-consumer
verified: 2026-11-12T03:27:06Z
status: passed
score: 15/15 must-haves verified (5 roadmap SCs + 10 requirements)
covered_files: [".github/workflows/gate.yml",".planning/ci-runs.tsv",".planning/phases/04-headless-runner-and-installed-consumer/04-01-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-01-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-02-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-02-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-03-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-03-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-04-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-04-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-05-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-05-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-06-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-06-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-07-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-07-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-08-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-08-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-09-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-09-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-10-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-10-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-11-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-11-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-12-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-12-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-13-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-13-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-14-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-14-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-15-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-15-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-16-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-16-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-17-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-17-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-18-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-18-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-19-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-19-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-20-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-20-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-21-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-21-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-22-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-22-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-23-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-23-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-24-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-24-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-25-PLAN.md",".planning/phases/04-headless-runner-and-installed-consumer/04-25-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-LEVER-01-SUMMARY.md",".planning/phases/04-headless-runner-and-installed-consumer/04-LEVER-02-SUMMARY.md","core/include/ivs/ivs_run.h","data/expected/pkg-consumer.golden","data/expected/runner-scenarios.ids","data/golden/diag-cpu-1.expect","runner/exec.c","runner/hostio.c","runner/main.c","runner/outcome.c","runner/report.c","runner/scenario.c","tests/installed-consumer/main.c","tools/check/ci-summary.sh","tools/check/steps/c-faults.sh","tools/gate.sh"]
covered_digest: "v4:sha256:aed81396b330eeacc3d4d79d92bf30efa862b54ccbbe2d98e4885d1c4e1d5e81"
behavior_unverified: 0
overrides_applied: 0
---

# Phase 4: Headless Runner and Installed Consumer (Slice 0) Verification Report

**Phase Goal:** An original, project-authored diagnostic runs from reset through the public API, the headless runner and an independently built installed consumer. Digests are deterministic, failures are reported honestly by kind, and the CI gate mirrors the local command.
**Verified:** 2026-11-12 (local), branch `gsd/phase-4-headless-runner-installed-consumer`, HEAD e9b3d31
**Status:** passed (with warnings and accepted caveats listed below)
**Re-verification:** Re-verified 2026-11-12T03:27:06Z at HEAD 7880979 after one covered-source change: `tools/check/steps/c-faults.sh` watchdog quiet self-test limit 5 s -> 60 s (with comment). `sh tools/gate.sh` re-run: gate: PASS steps=33/33 controls=277/277 faults=149/149; `ivs-run` on `diag-boot-pass.ivss` exit 0 (outcome pass); SC1-SC5 and requirement claims unchanged. Initial verification was 2026-11-12.

Method: I ran `sh tools/gate.sh` myself and wrote probes in a scratch directory outside the repo. I did not treat SUMMARY claims as evidence. Scratch dir: `<scratch-path>`.

Gate result (own run): `gate: PASS steps=33/33 controls=277/277 faults=149/149 tests-expected=2816 tests-executed=2814 tests-skipped=2 tests-missing=0 goldens=4 reports=12 host-dependent=3 gates=26 pkg-selftest=10/10 ci-runs=23`, wall 51 s, about 169 cpu-s, 18 cpus. The local Linux container log `build/gate-linux.log` (22:11, after the last commit at 22:04) also shows `gate: PASS steps=33/33 controls=277/277 cc=GNU`.

MVP-mode note: ROADMAP marks the phase `Mode: mvp`, but the goal is not written as a user story ("As a ..., I want to ..., so that ..."). As the caller instructed, I verified goal-backward against SC1-SC5 and did not refuse. The mismatch between the mode and the goal format is recorded as information only.

## Goal Achievement

### Observable Truths (Roadmap Success Criteria)

| # | Truth | Status | Evidence (own runs) |
|---|-------|--------|---------------------|
| SC1 | A scripted scenario on fresh state reports machine time, stop reason, raw outputs and guest-state assertions. The original diagnostic reaches an independently derived predicate, and its boot path is labelled "project boot stub, not an EXEC". | VERIFIED | `build/release/runner/ivs-run --content-dir build/release/images data/scenarios/diag-boot-pass.ivss`: exit 0, `outcome pass`, and the report has `exec: project boot stub, not an EXEC sha256=2b7cfb09...`, `grom: zero image, not a GROM`, `step.1.now_ticks 2084`, `stop_reason halted`, 18/18 asserts pass, 8 digest lines and `output.video.bytes 0` / `output.audio.bytes 0`. I recomputed the predicate in Python from the listing's instruction semantics (scratch probe), not from the core: R0-R6 = 03C5, 8080, 0000, 0037, 0201, 0037, 0340; [0200..0208] = 7FFF, 8080, A0A0, 7070, 0037, 0037, 0340, 0201, 7FFF; end marker [020F] = 03C5. The loop sum 1..10 = 0x37 matches `data/golden/diag-cpu-1.expect`. I also hand-checked the listing encodings (MVII 2B8, ADDR R2,R3 0D3, CLRR = XORR 1DB, BNZE 22C with disp 3 -> 501C, PSHR = MVO@ R6 273, PULR = MVI@ R6 2B5, HLT 000), and they agree. The installed consumer (SC4) shows the same state: `step.2.mem ... 0204=0037 ... 020F=03C5`. In-process `run.diag.predicate` and `run.diag.slice` are ok. |
| SC2 | Harness failure, guest failure, timeout, cancellation and missing fixture each give a distinct runner outcome, with one scenario each. Important gates have named-fault controls. | VERIFIED | I ran all 15 lines of `data/expected/runner-scenarios.ids` by hand through the real `ivs-run` (script `runall.sh`, with a staged content dir and a truncated cart). Every exit code matched: pass 0 (`diag-boot-pass`), guest-fail 10 (`guest-assert-mismatch`, `assert=13 mem 0204 == 0036 got 0037`), timeout 11 (`timeout-budget` budget; `timeout-wall` wall cap-ms=1), cancelled 12 (`cancel-signal`, `cancel-signal-twice`), missing-fixture 13 (`missing-cart-file`, `needs-user-exec`), harness-fail 14 (`bad-syntax`, `unexpected-refusal`, `dead-after-refusal`, `no-run-to`). Gate step `important-gates pass gates=26 lines=102`, `faults-verdict faults=149/149`, controls 277/277. |
| SC3 | A run to an absolute cycle target reports overshoot. Slicing gives the same per-domain digests and repeated runs match. Goldens carry tier/oracle headers. An undersized output buffer is refused before any state change. | VERIFIED | Diag report byte-equal apart from `host.calls` for `--slice-ticks` 1, 7, 100, 333, 1500 and the default; a repeated run is byte-identical. Scratch scenario `os.ivss` (`run-to 1001 ticks`, `run-to 1003 ticks`, `run-to 16000 ticks stop halted`) gives `overshoot_ticks 19`, then `17` (target already passed), then `0` halted, and is equal across slice sizes 1, 5 and 333. A public-API probe linked against the installed package (`bprobe/probe.c`) gives: target 1000 -> now 1020, overshoot 20; NULL video with capacity 64 -> `IVS_E_BAD_ARG reason=42`, whole instance storage `memcmp`-unchanged; buffer inside instance storage -> `IVS_E_BAD_ARG reason=43`, unchanged; the run then completes to halt. `BUFFER_TOO_SMALL` cannot be reached through the public API in Phase 4, because the layout is NONE with 0 bytes per frame (D-06). It is proven through the internal env seam: `run.buffers.too-small-video` and `-audio` are ok, and the `bf_refused` helper compares a whole-instance snapshot. All 4 goldens and `pkg-consumer.golden` have `# tier:` / `# oracle:` headers. |
| SC4 | A separate consumer project, built only against the installed package in a relocated prefix, runs load, refusal, run and digest. | VERIFIED | Fresh `cmake --preset release -B <scratch>/rel`, full build, `cmake --install --prefix <scratch>/pfx-orig` (10 files). Moved to `<scratch>/moved dir/pfx` (path with a space) and confirmed the original is gone. Configured a copy of `tests/installed-consumer` with only `CMAKE_PREFIX_PATH` = the moved prefix: `ivs_DIR` resolves inside `moved dir`, and `ninja -t deps` shows 0 references to the source tree. The output equals `data/expected/pkg-consumer.golden` without its `#` lines (`CONSUMER-GOLDEN-EQUAL`), and all six digests equal `data/golden/diag-input-pass.digests` (cross-path agreement with the runner). The only source-tree name in the prefix is the copyright line of the installed LICENSE. |
| SC5 | CI runs the same fast gate as local on every PR, shows skips and missing items, never caches or uploads images or rights-unknown fixtures, and records queue/setup/build/test per run. | VERIFIED | `.github/workflows/gate.yml` triggers on `pull_request` and push to main, runs one job with only `actions/checkout@<sha>` (persist-credentials false), `cmake --version` and `sh tools/gate.sh`, which is the local command. It has no cache or upload action; `ci-parity` bans these and its controls are green. I rendered `tools/check/ci-summary.sh` with GITHUB_STEP_SUMMARY from my own gate's lines: headline `gate: PASS`, a "Skips and missing" table (tests-skipped 2, tests-missing 0, plus pending-resolvers/unknown-fields noise) and the slowest steps. `.planning/ci-runs.tsv` has 23 rows with queue_s, setup_s, build_s, test_s, gate_s, total_s, ncpu and cpu_s for each recorded hosted run. |

**Score:** 5/5 SCs verified (0 present, behavior-unverified)

### Requirements Coverage

| Requirement | Status | Evidence |
|-------------|--------|----------|
| API-02 bounded run, overshoot, slicing invariance | SATISFIED | SC3 probes (runner and public API); `run.target.*`, `run.slice.{halt,fault}.*` ok in `ivs_tests_run` |
| API-04 caller-owned buffers, refusal before state change | SATISFIED (seam for TOO_SMALL, by D-06) | Public probe: NULL/inside-instance buffers refused with storage unchanged; `run.buffers.too-small-*` through the env seam |
| RUN-01 scripted scenario reports time, stop, outputs, assertions | SATISFIED | diag-boot-pass report (SC1) |
| RUN-02 distinct outcomes | SATISFIED | 15/15 scenarios matched exits 0/10/11/12/13/14 (SC2) |
| RUN-04 important gates have named-fault controls | SATISFIED | `important-gates gates=26`, faults 149/149, controls 277/277 |
| RUN-05 per-domain digests, repeat match, golden headers | SATISFIED | Repeat byte-equal; slice-equal digests; headers on all goldens. Digests are repeatability pins (self-recorded), not accuracy |
| RUN-06 original diagnostic from reset via project boot path | SATISFIED (provisional claim CL-0008) | SC1; independent recomputation of the arithmetic |
| PKG-01 installed consumer in a relocated prefix | SATISFIED | SC4 own build |
| PKG-03 CI fast gate, visible skips, no caches/uploads | SATISFIED | SC5 |
| PKG-04 per-run critical path recorded | SATISFIED (orchestrator-recorded) | 23 rows in ci-runs.tsv; gate emits `step-time`, `gate-env`, `gate-cpu`, `gate-time` on every run |

No orphaned requirements: REQUIREMENTS.md maps exactly these ten to Phase 4.

### Claim wording

The goldens use `original-diagnostic / independent-derivation` (the .expect) and `deterministic-repeatability / self-recorded` or `regression pin, emulator is oracle` (the digest goldens and consumer golden). `docs/diagnostics.md:44,109` and `docs/packaging.md:77,89` state that nothing is a hardware claim and name the shared-misreading risk. A grep for hardware-validated, cycle-accurate, "matches jzIntv/MAME" or "runs Intellivision games" wording in docs, README, runner, goldens and the consumer found nothing. Note: the oracle label "independent-derivation" means independent of the emulator's execution, not of the author. The file and the docs say this explicitly.

### Anti-Patterns

There are no unreferenced TBD, FIXME or XXX markers in the 346 non-planning files changed since the phase CONTEXT commit (3e5c356..HEAD).

### Warnings (not blocking)

1. **c-faults watchdog self-test "quiet" can flake on loaded or suspended hosts (confirmed by reading the code).** In `tools/check/steps/c-faults.sh:120-146`, `run_limited 5 ... true` passes only if the parent shell reaps `true` and sends TERM to the watchdog within 5 s of wall time after the watchdog's `sleep 5` starts. The test runs concurrently with 4 fault lanes and the `controls` step. A scheduling stall longer than 5 s writes `wd-quiet.mark` and fails the gate, and so does a laptop sleep or a VM pause in that window. The signal-ordering paths (TERM before the trap, TERM before `_s` is set) all exit without the mark, so the only failure mode is timing. The one observed failure (load 14, dash) fits this. Suggested fix: raise the quiet limit (for example to 30 s, since `true` needs no budget), or run the self-test before the lanes start.
2. **The hosted gate has no recorded run at HEAD.** The last row in ci-runs.tsv is c1cf910, which is 19 commits behind HEAD; the commits after it include the review fixes, among them glibc-specific WR-04. Local sh, dash and gcc-container gates pass at HEAD. Recording hosted rows is the orchestrator's job, and I did not call gh.
3. **`BUFFER_TOO_SMALL` cannot be reached through the public API in Phase 4 (D-06, by design).** It is proven only through the internal env seam until Phases 6-7 give the output layout real sizes.
4. **ROADMAP `Mode: mvp` with a goal that is not a user story** (format discrepancy only).

### Accepted caveats (listed, not failed)

- Five deferred info items in the 04-REVIEW Disposition (IN-04, IN-06, IN-09, IN-10, IN-11).
- CL-0008 is provisional: the diagnostic was hand-derived from the manual by the same model family, so the shared-misreading risk stands, and the libasm cross-check was not run.
- Input applies at CPU-unit-start granularity (C67).
- The INTERNAL stop reason cannot be tested through the public API (define pins only).
- The hosted gate wall time is 39-75 s with host-speed variance, and the budget was re-baselined to median gate_s 75 s and cpu_s 200.
- The `hygiene --history` scan fails on already-public planning commits that contain a scratch path (an owner decision).
- The job-summary skip table lists pending-resolvers and unknown-fields (noise).
- Branch protection is recommended, not applied.
- One dash gate run under load failed the c-faults watchdog "quiet" self-test (see warning 1).
- Totals in planning prose are stale; for example, 04-25 says controls 276, while the gate now has 277 after IN-07.

### Human Verification Required

None for the phase goal. Branch protection is an owner or orchestrator settings action and does not block this phase.

### Gaps Summary

There are no blocking gaps. Every SC and requirement was reproduced with my own runs: the gate, the real runner over every scenario, slicing and overshoot probes, an independent recomputation of the predicate, a fresh relocated install with the consumer built against it, and a public-API refusal probe. The warnings above are reliability and recording items, not goal failures.

---

_Verified: 2026-11-12_
_Verifier: Claude (gsd-verifier)_
