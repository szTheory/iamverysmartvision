---
phase: "13"
slug: "rights-ledger-and-enforced-claim-ceiling"
# status lifecycle: draft (seeded by plan-phase) → validated (set by validate-phase §6)
# audit-milestone §5.5 distinguishes NOT-VALIDATED (draft) from PARTIAL (validated + nyquist_compliant: false)
status: validated
nyquist_compliant: true
wave_0_complete: true
created: "2026-11-26"
---

# Phase 13 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | The project's own gate harness: 38 POSIX `sh` steps + CMake `-P` checkers + negative-control trees. No external test framework. |
| **Config file** | `tools/gate.sh` (runner), `data/expected/gate-steps/*.ids` (step inventory), `data/expected/controls/*.ids` (control inventory), `data/expected/*.ids` (per-suite baselines) |
| **Quick run command** | `git ls-files > /tmp/fl && ROOT="$PWD" CHECK_DIR="$PWD/tools/check" FILELIST=/tmp/fl sh tools/check/steps/<step>.sh` |
| **Full suite command** | `sh tools/gate.sh` (the local fast path, same as the PR gate) |
| **Estimated runtime** | ~120 s full gate (`gate_s` last-five median 121); each single step < 2 s except `files` (~2 s) |

---

## Sampling Rate

- **After every task commit:** run the single affected step directly (all < 2 s).
- **After every plan wave:** `sh tools/check/run-controls.sh` — the fastest way to catch a control-overlay mistake.
- **Before `/gsd:verify-work`:** `sh tools/gate.sh` full suite green, `steps=38/38`, `faults=238/238`, `goldens=29`.
- **Max feedback latency:** ~120 seconds (full gate); ~2 seconds per step.

---

## Per-Task Verification Map

Filled by the planner/executor as plans land. Every row's automated command must be a real, runnable command with a stated failing direction in its plan task.

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 13-01-01 | 13-01 | 1 | RTS-01 | T-13-01, T-13-02 | The ledger row's key and type are checked and a `cleared` row must cite a resolvable `SRC-` id | unit (step) | `ROOT="$PWD" CHECK_DIR="$PWD/tools/check" FILELIST=/tmp/fl13 sh tools/check/steps/manifests.sh` → `-- manifests pass manifests=11 effective-yes=8 effective-no=3 rights=1 rights-cleared=1` | ✅ | ✅ green |
| 13-02-01 | 13-02 | 2 | RTS-01 | T-13-17, T-13-19 | Every milestone question has a written disposition, and a `cleared` row added to look answered moves `rights-cleared` | unit (step) | `ROOT="$PWD" CHECK_DIR="$PWD/tools/check" FILELIST=/tmp/fl13 sh tools/check/steps/manifests.sh` → `-- manifests pass manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1` | ✅ | ✅ green |
| 13-03-01 | 13-03 | 3 | RTS-02 | T-13-06, T-13-07, T-13-08 | The refused-spelling match is character-bounded per scope, a stale rights pin is a failure and no glob widens a scope | unit (step) | `ROOT="$PWD" CHECK_DIR="$PWD/tools/check" FILELIST=/tmp/fl13 sh tools/check/steps/files.sh` → `-- files pass tracked=2702 binaries-listed=0 rights-pinned=4/4` (the `tracked=` token is that task's own commit-time count; `rights-pinned=4/4` is the invariant) | ✅ | ✅ green |
| 13-03-02 | 13-03 | 3 | EV-06 | T-13-09 | The D-50 ceiling cannot pass vacuously: two named findings, three must-fail controls and one legal positive | unit (step) | `ROOT="$PWD" CHECK_DIR="$PWD/tools/check" FILELIST=/tmp/fl13 sh tools/check/steps/claims.sh` → `-- claims pass claims=28 provisional=27` | ✅ | ✅ green |
| 13-04-01 | 13-04 | 4 | EV-07 | T-13-11 | A `mutant` row resolves through a committed case inventory the step must actually run, never a hard-coded name | selftest inventory | `ROOT="$PWD" CHECK_DIR="$PWD/tools/check" FILELIST=/tmp/fl13 sh tools/check/steps/runner-selftest.sh` → `-- runner-selftest pass cases=14` | ✅ | ✅ green |
| 13-04-02 | 13-04 | 4 | EV-07 | T-13-12, T-13-14 | The sixth control directory exists under the name its registry row cites, and its FORBID list proves it failed for that reason alone | control directory | `sh tools/check/run-controls.sh` → `controls=401/401` and `control ok   important-gates-excluded-stale` | ✅ | ✅ green |
| 13-04-03 | 13-04 | 4 | EV-07 | T-13-13, T-13-15 | The row's requirement is inside the derived accepted set, and the baseline is re-derived by running the step rather than quoted | unit (step) | `ROOT="$PWD" CHECK_DIR="$PWD/tools/check" FILELIST=/tmp/fl13 sh tools/check/steps/important-gates.sh` → `-- important-gates pass gates=114 lines=352` | ✅ | ✅ green |
| 13-05-01 | 13-05 | 5 | RTS-01, RTS-02, EV-06, EV-07 | T-13-20 | The cost medians are recomputed by arithmetic from committed rows and the middle value is the recorded one, so no quoted median can certify a number nobody measured | full gate, two shells | `sh tools/gate.sh` and `dash tools/gate.sh` → `gate: PASS steps=38/38 controls=401/401 ... faults=238/238 ... goldens=29`, byte-identical; plus `grep -c -e '^- \[Phase 13\]: 13-05: checkpoint: ' .planning/STATE.md` → `1` | ✅ | ✅ green |
| 13-05-02 | 13-05 | 5 | RTS-01, RTS-02, EV-06, EV-07 | T-13-21 | A hosted row comes from `sh tools/ci-record.sh <run-id>` for a run id `gh run list` reported, and its absence is a visible `not-recorded` line, never a fabricated row | unit (step) | `ROOT="$PWD" CHECK_DIR="$PWD/tools/check" FILELIST=/tmp/fl13 sh tools/check/steps/ci-log.sh` → `-- ci-log pass ci-runs=96`; `gh run list --commit 10f66a1 --json databaseId,event,conclusion,headSha` → `[]` | ✅ | ✅ green (hosted row `not-recorded`, visible in `13-05-SUMMARY.md`) |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [x] `data/rights/*.json` (10 rows) — every rights id depends on these existing. Landed as the ten files under `data/rights/`; the ledger row count is the gate's own `rights=10`.
- [x] `data/expected/rights.ids`, `data/expected/rights-pinned.ids` — membership and pins. Both present; `rights-pinned=4/4` is the gate's own count and a stale pin is `rights-pinned-stale` (a failure, not a NOTE).
- [x] The ledger checker — landed as `tools/check/rights-lib.cmake`, included by `tools/check/manifests.cmake:34` (the RTS-01 limb) and `tools/check/files.cmake:41` (the RTS-02 naming limb), in place of the `schema/rights.txt` / `kinds/rights.cmake` pair this row guessed at. No new gate step: `steps` stays 38 (GUARDRAILS 2.1).
- [x] `tools/check/controls/_good/tree/data/rights/*.json` + its `rights.ids` / `rights-pinned.ids` — the toy tree must pass the new checks. Present as `toy-owner-image.json` and `toy-reference-doc.json` plus both inventory files.
- [x] `data/expected/runner-selftest.ids` — the four selftest case names (EV-07 deviation limb). Present; `cases=14` is the step's own count and `tools/check/selftest-inventory.sh` refuses an inventory name the step did not run.
- [x] `data/expected/controls/rights.ids` + `data/expected/controls/claims.ids` — inventory ids for the new control dirs. Both present; `controls=401/401` is the runner's own count.
- [x] Framework install: none — the harness is the project's own. Satisfied by construction: this phase installed nothing, and no package manager was invoked.

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| none | — | — | All phase behaviors have automated verification. |

---

## Validation Sign-Off

- [x] All tasks have `<automated>` verify or Wave 0 dependencies — nine of nine rows above carry one
- [x] Sampling continuity: no 3 consecutive tasks without automated verify
- [x] Wave 0 covers all MISSING references — all seven bullets ticked, each naming the file that satisfies it
- [x] No watch-mode flags
- [x] Feedback latency < 120s — measured on this host at the closing commit: `gate-time total=97` under `sh` and `106` under `dash`
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** validated by plan 13-05 on 2026-11-25 (the date the closing commits carry; this file's `created:` field is the planner's 2026-11-26). The phase's closing gate run, byte-identical under `sh` and `dash` (`tail -1 /tmp/g13.out` compared with `cmp` against `tail -1 /tmp/g13d.out`), reads:

```
gate: PASS steps=38/38 controls=401/401 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=96 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=686 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2722 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=63 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2722 history-commits=1 gates=114 lines=352 input-rows=31 input-policies=11 input-profile-fields=31 files=2722 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=4990 tests-executed=4986 tests-skipped=4 tests-missing=0 docs=64 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

The Ubuntu 24.04 container gate (`sh tools/gate-linux.sh`, gcc 13.3.0, cmake 3.28.3) also ends `gate: PASS steps=38/38 controls=401/401 ... faults=238/238 ... goldens=29` with `gate-linux: exit 0`. Comparing the two summary lines token by token, exactly four differ and each is a recorded host difference, not a movement of the phase's line: `cc=GNU` for `cc=AppleClang` (D-24 keeps two compilers and two operating systems without a second job), `core-imports=5` for `7` (the Darwin `-O2` build imports `__stack_chk_fail` and `__stack_chk_guard`; `c-purity` reports the count as `core-imports`), `archive=sha256` for `archive=members` (plan 04-22: CMake re-runs `ranlib` at install on Darwin, so the archive is compared member-wise there and whole-file on Linux), and `refs-verified=0/4` for `3/4` (the sources registry's cached PDFs are not sent into the container, so the registry verifies no reference hash there). Every invariant the phase freezes — `steps=38/38`, `controls=401/401`, `faults=238/238`, `goldens=29`, `rights=10 rights-cleared=1`, `gates=114 lines=352`, `cases=14` — is identical in both.
