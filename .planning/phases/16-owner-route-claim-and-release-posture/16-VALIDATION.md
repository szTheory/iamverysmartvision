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
| **Quick run command** | `git ls-files > /tmp/fl && ROOT="$PWD" CHECK_DIR="$PWD/tools/check" FILELIST=/tmp/fl sh tools/check/steps/claims.sh` (the step the phase's own `handoffs=` stat lives in; any step can be named the same way) |
| **Full suite command** | `sh tools/gate.sh` (the local fast path, same as the PR gate) |
| **Measured runtime** | full gate 66–147 s on this host across five `sh` runs and 172–204 s under `dash` (the host was not idle); the lever's own measured effect was `gate-time total` 79 → 72 on a quiet tree. The bound is 180 s; a loaded host exceeds it and the figure is quoted as a range, not a promise |

---

## Sampling Rate

- **After every task commit:** run the single affected step directly by naming it in the quick-run command above (all < 2 s except `files` and the C steps).
- **After every plan wave:** `sh tools/check/run-controls.sh` (404 controls).
- **Before `/gsd:verify-work`:** `sh tools/gate.sh` full suite green, `steps=38/38`, `faults=238/238`, `goldens=29`, `controls=404/404`.
- **Max feedback latency:** the full gate measured 66-147 s on a loaded host; a single step is < 2 s except `files` and the C steps. The 180 s bound is the budget, not the expectation.

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 16-01-T1 | 01 | 1 | PKG-06 | T-16-01 | the overlap cannot hide a behavioural difference: each `step.out.<name>` compared, token line byte-identical | integration | `sh tools/gate.sh` twice, per-buffer `cmp` + filtered whole-log `cmp` | ✅ | ✅ green |
| 16-01-T2 | 01 | 1 | PKG-06 | T-16-02 | the record states the measured before/after and what the lever is not | record | `grep -q 'gate-overlap pair=c-faults+c-tests' .planning/CI-EFFICIENCY.md` | ✅ | ✅ green |
| 16-02-T1 | 02 | 2 | PKG-06 | T-16-05 | the route runs from the source archive and refuses a CI environment | integration | `sh tools/release-rehearse.sh --source-route` (exit 0, `124 of 124` compared) | ✅ | ✅ green |
| 16-02-T2 | 02 | 2 | PKG-06 | T-16-06 | the mutation is refused and `exec:` stays `-` | integration + record | transcript in `RELEASE-OPS.md` §(j); `exit=14 status=IVS_E_DIGEST_MISMATCH` | ✅ | ✅ green |
| 16-03-T1 | 03 | 3 | PKG-06 | T-16-10 | the ledger's row count is a stat counted from the file | unit (step) | `-- claims pass claims=29 provisional=28 handoffs=8` | ✅ | ✅ green |
| 16-03-T2 | 03 | 3 | PKG-06 | T-16-11 | a handoff row is `evidence: none`, cited by no claim and no criterion | unit (step) + record | `claims-handoff-row`/`-missing`/`-unreadable` controls; `docs/handoffs.md` sections | ✅ | ✅ green |
| 16-04-T1 | 04 | 4 | PKG-06 | T-16-14 | a **tracked** shared object is refused by name, tree left clean | integration | `files-unlisted-binary-so` on a tracked probe, witness pair, `git status` clean after | ✅ | ✅ green |
| 16-04-T2 | 04 | 4 | PKG-06 | T-16-15 | PKG-06 accepted; package stays eleven files; release line recomputed | unit (step) | `-- files pass … prefix-files=11`; `important-gates gates=119 lines=364` | ✅ | ✅ green |
| 16-05-T1 | 05 | 5 | PKG-06 | T-16-19 | one line on one tree under two shells and once in the container; one verdict | integration | `sh`/`dash` `cmp`-identical tails; `sh tools/gate-linux.sh` exit 0 | ✅ | ✅ green |
| 16-05-T2 | 05 | 5 | PKG-06 | T-16-20 | the validation record is signed and the milestone's claim/handoff posture stated | record | `16-VALIDATION.md` frontmatter `status: validated`; SUMMARY section present | ✅ | ✅ green |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [x] `tools/gate.sh` — the lever's overlap and the inline build lock. **Satisfied by** `tools/gate.sh`'s `overlap_bg`/`overlap_fg` declarations and the byte-identical `# build-lock begin/end` blocks in `tools/check/steps/c-tests.sh` and `c-faults.sh` (plan 16-01, `f9fe6a9`).
- [x] `tools/release-rehearse.sh` — the three-command source route. **Satisfied by** the `--source-route` build → wrap → run sequence and its eight report assertions (plan 16-02, `14f81ed`).
- [x] `docs/owner-images.md` — the documented route. **Satisfied by** "The route from the source archive, in three commands" (plan 16-02, `14f81ed`).
- [x] `.planning/ops/RELEASE-OPS.md` §(j) — the transcript. **Satisfied by** the 49-line section quoting the archive prefix, the wrap digest, the run's `exit=10 outcome=guest-fail exec-label=-` and the mutation's `exit=14 status=IVS_E_DIGEST_MISMATCH` (plan 16-02, `6b57148`).
- [x] `data/expected/handoffs.ids` — the three rows, `evidence: none`. **Satisfied by** `H16-1`/`H16-2`/`H16-3` at `handed-off evidence: none`, with `D10`/`D11` still `not-performed` (plan 16-03, `43b4394`).
- [x] `docs/handoffs.md` — the packet sections. **Satisfied by** one section per row, each naming its command, artefact, deterministic identities and what a person judges (plan 16-03, `43b4394`).
- [x] `data/expected/important-gates.ids` — the three `release-posture` rows. **Satisfied by** the rows at `gates=119 lines=364`, with the re-derived baseline in the same commit (plan 16-04, `298be25`).
- [x] `.planning/ops/RELEASE-OPS.md` §(i) — the recomputed release-decision line. **Satisfied by** "the release ships the static package and the source archive; no shared object, no host shim, no content" (plan 16-04, `298be25`).
- [x] `16-VALIDATION.md` — this record, signed. **Satisfied by** this file's `status: validated` frontmatter and the Approval line below (plan 16-05).
- [x] Framework install: none — the harness is the project's own. **Satisfied by** the absence of any dependency in `tools/gate.sh`; `cmake`/`sh`/`awk`/`grep`/`git` are the host's.

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Physical hardware measurement (`D10`, 47 items) | PKG-06 | Needs the owner's hardware and instruments; no `measured-hardware` tier exists and none is claimed | `docs/handoffs.md` D10 — a handoff row, `not-performed`, never a criterion |
| The owner's own EXEC/GROM/cartridge run (`D11`) | PKG-06 | Needs the owner's files (D11) | `docs/handoffs.md` D11 — a handoff row, `not-performed`, never a criterion |
| A real frontend loads the shim (`H16-1`) | PKG-06 | Needs a real libretro frontend, which the project does not ship or run in CI | `docs/handoffs.md` H16-1 — recorded, never a criterion |
| An owner image's picture and sound look right (`H16-2`) | PKG-06 | Needs the owner's own EXEC/GROM/cartridge and a human eye and ear (D11) | `docs/handoffs.md` H16-2 |
| The owner's flat image needs no word-order guess (`H16-3`) | PKG-06 | Needs an owner image whose layout the project cannot know | `docs/handoffs.md` H16-3 |

All three are **handoff rows with `evidence: none`**, cited by no claim and by no phase exit criterion. They
are recorded here so the table does not claim the phase has no manual surface; they are not verification
items and they are never re-asked in a later milestone.

---

## Validation Sign-Off

- [x] All tasks have `<automated>` verify or Wave 0 dependencies
- [x] Sampling continuity: no 3 consecutive tasks without automated verify
- [x] Wave 0 covers all MISSING references
- [x] No watch-mode flags
- [x] Feedback latency < 120s (measured ~72 s full gate)
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** approved 2026-11-26. The measured closing line, quoted verbatim:

```
gate: PASS steps=38/38 controls=404/404 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=104 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=29 provisional=28 handoffs=8 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=695 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2787 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=63 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2787 history-commits=1 gates=119 lines=364 input-rows=31 input-policies=11 input-profile-fields=31 files=2787 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=20/20 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=12/12 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5042 tests-executed=5038 tests-skipped=4 tests-missing=0 docs=64 claim-refs=27 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

`sh` and `dash` final lines are `cmp`-identical at **1,520 bytes**; `sh tools/gate-linux.sh` exited 0 (`gate-time total=793`) differing in exactly the four documented host tokens (`cc`, `core-imports`, `archive`, `refs-verified`) and nothing else; one checkpoint verdict, `reprice`, recomputed from the last five `.planning/ci-runs.tsv` rows.
