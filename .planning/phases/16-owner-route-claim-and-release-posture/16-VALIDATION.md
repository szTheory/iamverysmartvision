---
phase: "16"
slug: "owner-route-claim-and-release-posture"
# status lifecycle: draft (seeded by plan-phase) → validated (set by validate-phase §6)
# audit-milestone §5.5 distinguishes NOT-VALIDATED (draft) from PARTIAL (validated + nyquist_compliant: false) (#2117)
status: validated
nyquist_compliant: true
wave_0_complete: true
created: "2026-11-26"
---

# Phase 16 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | The project's own gate harness: 38 POSIX `sh` steps + CMake `-P` checkers + 404 negative-control trees. No external test framework. |
| **Config file** | `tools/gate.sh` (runner), `data/expected/gate-steps/*.ids`, `data/expected/controls/*.ids`, `data/expected/*.ids` |
| **Quick run command** | `git ls-files > /tmp/fl && ROOT="$PWD" CHECK_DIR="$PWD/tools/check" FILELIST=/tmp/fl sh tools/check/steps/<step>.sh` |
| **Full suite command** | `sh tools/gate.sh` (the local fast path, same as the PR gate) |
| **Estimated runtime** | ~72 s full gate on this host (`gate-time total` 79 → 72 after the lever); 66 s at the final tree |

---

## Sampling Rate

- **After every task commit:** run the single affected step directly (all < 2 s except `files` and the C steps).
- **After every plan wave:** `sh tools/check/run-controls.sh` (404 controls).
- **Before `/gsd:verify-work`:** `sh tools/gate.sh` full suite green, `steps=38/38`, `faults=238/238`, `goldens=29`, `controls=404/404`.
- **Max feedback latency:** ~72 s (full gate), ~2 s per step.

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 16-01-T1 | 01 | 1 | PKG-06 | T-16-01 | the overlap cannot hide a behavioural difference: each `step.out.<name>` compared, token line byte-identical | integration | `sh tools/gate.sh` twice, per-buffer `cmp` + filtered whole-log `cmp` | ✅ | ✅ green |
| 16-01-T2 | 01 | 1 | PKG-06 | T-16-01 | the record states the measured before/after and what the lever is not | record | `grep -q 'gate-overlap pair=c-faults+c-tests' .planning/CI-EFFICIENCY.md` | ✅ | ✅ green |
| 16-02-T1 | 02 | 2 | PKG-06 | T-16-02 | the route runs from the source archive and refuses a CI environment | integration | `sh tools/release-rehearse.sh --source-route` (exit 0, `124 of 124` compared) | ✅ | ✅ green |
| 16-02-T2 | 02 | 2 | PKG-06 | T-16-02 | the mutation is refused and `exec:` stays `-` | integration + record | transcript in `RELEASE-OPS.md` §(j); `exit=14 status=IVS_E_DIGEST_MISMATCH` | ✅ | ✅ green |
| 16-03-T1 | 03 | 3 | PKG-06 | T-16-03 | the ledger's row count is a stat counted from the file | unit (step) | `-- claims pass claims=29 provisional=28 handoffs=8` | ✅ | ✅ green |
| 16-03-T2 | 03 | 3 | PKG-06 | T-16-03 | a handoff row is `evidence: none`, cited by no claim and no criterion | unit (step) + record | `claims-handoff-row`/`-missing`/`-unreadable` controls; `docs/handoffs.md` sections | ✅ | ✅ green |
| 16-04-T1 | 04 | 4 | PKG-06 | T-16-04 | a **tracked** shared object is refused by name, tree left clean | integration | `files-unlisted-binary-so` on a tracked probe, witness pair, `git status` clean after | ✅ | ✅ green |
| 16-04-T2 | 04 | 4 | PKG-06 | T-16-04 | PKG-06 accepted; package stays eleven files; release line recomputed | unit (step) | `-- files pass … prefix-files=11`; `important-gates gates=119 lines=364` | ✅ | ✅ green |
| 16-05-T1 | 05 | 5 | PKG-06 | T-16-05 | one line on one tree under two shells and once in the container; one verdict | integration | `sh`/`dash` `cmp`-identical tails; `sh tools/gate-linux.sh` exit 0 | ✅ | ✅ green |
| 16-05-T2 | 05 | 5 | PKG-06 | T-16-05 | the validation record is signed and the milestone's claim/handoff posture stated | record | `16-VALIDATION.md` frontmatter `status: validated`; SUMMARY section present | ✅ | ✅ green |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [x] `tools/gate.sh` — the lever's overlap and the inline build lock (plan 16-01)
- [x] `tools/release-rehearse.sh` — the three-command source route (plan 16-02)
- [x] `docs/owner-images.md` — the documented route (plan 16-02)
- [x] `.planning/ops/RELEASE-OPS.md` §(j) — the transcript (plan 16-02)
- [x] `data/expected/handoffs.ids` — the three rows, `evidence: none` (plan 16-03)
- [x] `docs/handoffs.md` — the packet sections (plan 16-03)
- [x] `data/expected/important-gates.ids` — the three `release-posture` rows (plan 16-04)
- [x] `.planning/ops/RELEASE-OPS.md` §(i) — the recomputed release-decision line (plan 16-04)
- [x] `16-VALIDATION.md` — this record, signed (plan 16-05)
- [x] Framework install: none — the harness is the project's own.

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| none | — | — | All phase behaviors have automated verification. The owner's own run is a handoff row (H16-1..H16-3), not a criterion. |

---

## Validation Sign-Off

- [x] All tasks have `<automated>` verify or Wave 0 dependencies
- [x] Sampling continuity: no 3 consecutive tasks without automated verify
- [x] Wave 0 covers all MISSING references
- [x] No watch-mode flags
- [x] Feedback latency < 120s (measured ~72 s full gate)
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** approved 2026-11-26 — `gate: PASS steps=38/38 controls=404/404 faults=238/238 goldens=29 gates=119 lines=364 handoffs=8 claims=29 provisional=28 pkg-selftest=20/20 release-selftest=12/12 tests-expected=5042`, `sh` and `dash` tails `cmp`-identical at 1,520 bytes, `sh tools/gate-linux.sh` exit 0 (`gate-time total=793`, four documented host tokens differing), one checkpoint verdict `reprice`.
