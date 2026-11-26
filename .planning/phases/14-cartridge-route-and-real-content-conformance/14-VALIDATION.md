---
phase: "14"
slug: "cartridge-route-and-real-content-conformance"
# status lifecycle: draft (seeded by plan-phase) → validated (set by validate-phase §6)
# audit-milestone §5.5 distinguishes NOT-VALIDATED (draft) from PARTIAL (validated + nyquist_compliant: false) (#2117)
status: validated
nyquist_compliant: true
wave_0_complete: true
created: "2026-11-26"
---

# Phase 14 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.
> Filled by the planner from the four plans' own `<verify>` blocks; `status`, `nyquist_compliant`,
> `wave_0_complete` and the sign-off were completed by plan 14-04 Task 2, which is where the
> measurements exist. Amended in place at that sign-off (GUARDRAILS 2.10): the tables below keep the
> planner's rows and each row's status is signed off with the command that produced it, and the one
> figure measurement refuted (the plan-time 14 s quick-run latency) is amended with its measured
> value rather than re-blessed.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | CTest over 22 registered suites plus the 38-step gate (`tools/gate.sh`); 18 in-process TAP binaries, 4 shell TAP drivers (`pack-tap.sh`, `asm-tap.sh`, `runner-process-tap.sh`, `headers-tap.sh`) |
| **Config file** | `CMakePresets.json` (presets `dev`/`release`), `tests/CMakeLists.txt`, `data/expected/tests/*.ids` (one expected-id file per suite) |
| **Quick run command** | `ctest --preset dev -R '^(pack|runner-process|state|run|stic|cpu-vectors)$' --output-on-failure` |
| **Full suite command** | `sh tools/gate.sh` (`sh tools/gate-linux.sh` for the Ubuntu substance) |
| **Measured runtime** | the quick run is 29.2 s cold / 37.6 s warm on this host (six suites, `runner-process` 29.5 s of it); the full gate measured `gate-time total=` **86, 105 and 147** across three `sh` runs on this host (147 at the sign-off measurement, the spread being host load, not tree size) and **487** inside the Ubuntu 24.04 container (`linux/amd64`, emulated). The plan-time 14 s quick-run estimate was stale by measurement; see the plan defect recorded in `14-04-SUMMARY.md` |

---

## Sampling Rate

- **After every task commit:** the quick run above (the closing task ran the full gate after its commit, which is stronger than the quick run).
- **After every plan wave:** `sh tools/gate.sh`.
- **Before `/gsd:verify-work`:** the full gate green under both `sh` and `dash`, plus one container run - done at this sign-off.
- **Max feedback latency:** 37.6 s, the measured warm quick-run wall time (29.2 s cold), dominated by the `runner-process` suite at 29.5 s. The plan-time 14 s figure was stale by measurement and is amended here rather than re-blessed.

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 14-01-T1 | 01 | 1 | TOOL-07 | T-14-01 / T-14-02 / T-14-03 / T-14-04 / T-14-05 / T-14-06 | the size arithmetic is bounded before any multiplication, the output parent is resolved physically, the open is exclusive, and a failed write leaves no file | integration (process) | wrap 8192 zero bytes at `5000`, load as `content cart user`, `grep -x -F "slot.cart.sha256 <printed>"` | ✅ exists (`tools/flatwrap/flatwrap.c`, `tools/flatwrap/CMakeLists.txt`, the `ivs_flatwrap` target) | ✅ green |
| 14-01-T2 | 01 | 1 | TOOL-07 | T-14-01 / T-14-03 / T-14-05 | each refusal exits 1 with one token line and no output file; the existing-output case leaves the file byte-identical | integration (process) | `ctest --preset dev -R '^pack$'` → 23 ids, no `not ok` | ✅ exists (`data/expected/tests/pack.ids`, 23 ids) | ✅ green |
| 14-02-T1 | 02 | 1 | CPU-06 | T-14-07 / T-14-08 / T-14-09 / T-14-10 / T-14-11 | the profile scalar governs the machine, both settings are explicit, and the literals are recomputed by a second path | in-process | `ctest --preset dev -R '^(state\|run\|stic\|cpu-vectors)$'` green; the five pre-flip literals gone; `"config": {` count 7 | ✅ exists (`data/profiles/intv-2609-ntsc-family-p1.json`, the five vector `config` keys, `core/src/cpu/cp1610.c`) | ✅ green |
| 14-02-T2 | 02 | 1 | CPU-06 | T-14-10 / T-14-11 | an observation moves a default, never a tier; the log's shape and its `unknown-default-chosen: ` prefix hold | registry (gate step) | `sh tools/check/steps/discrepancies.sh` → `discrepancies=110 pending-resolvers=20 open=101` | ✅ exists (`data/discrepancies.json` C41/C82, `data/expected/d10-measurements.ids`) | ✅ green |
| 14-03-T1 | 03 | 2 | SCN-03 | T-14-12 / T-14-13 / T-14-16 | all six derived outputs refused with one line each and no file; the snapshot hook is accepted, cuts, and writes nothing | integration (process) | `ctest --preset dev -R '^runner-process$'` → 136 ids, no `not ok` | ✅ exists (`tests/runner-process-tap.sh`, its sixth argument, 136 ids) | ✅ green |
| 14-03-T2 | 03 | 2 | SCN-03 | T-14-14 / T-14-16 | the digest equality is live: one flipped input byte changes the printed digest and `slot.cart.sha256` | integration (process, 2 runs) | same suite → 137 ids | ✅ exists (`data/expected/tests/runner-process.ids`, 137 ids) | ✅ green |
| 14-03-T3 | 03 | 2 | SCN-03 | T-14-13 / T-14-15 / T-14-16 | the cartridge is an option, the lock keeps two roles, and the pass path is shown able to fail by a one-byte EXEC mutation | integration (process) | same suite → 138 ids; `files` step `rights-pinned=4/4 binaries-listed=0` | ✅ exists (`tools/owner-run.sh --cart`, `data/expected/tests/runner-process.ids`, 138 ids) | ✅ green |
| 14-04-T1 | 04 | 3 | TOOL-07 / SCN-03 / CPU-06 | T-14-17 / T-14-21 | a flip and its row move together in both directions; the toy tree's derived set stays closed | registry (gate step) | `important-gates` → `gates=117 lines=357`; toy tree `gates=46 lines=49`; `run-controls.sh` → `controls=402/402` | ✅ exists (`data/expected/important-gates.ids`, the thirteen toy files) | ✅ green |
| 14-04-T2 | 04 | 3 | TOOL-07 / SCN-03 / CPU-06 | T-14-18 / T-14-19 / T-14-20 | the closing line is measured, not quoted; one cost verdict with its arithmetic; one container gate at a time | integration (two shells + container) | `sh`/`dash` `tail -1` byte-identical, the predicted token string, `gate: PASS` in the container log | ✅ exists (`.planning/STATE.md`, this file, the three gate logs) | ✅ green |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [x] `tools/flatwrap/{flatwrap.c,CMakeLists.txt}` — the tool and its target, with the SPDX header and the source-list writer (plan 14-01). **Satisfied by** `tools/flatwrap/flatwrap.c`, `tools/flatwrap/CMakeLists.txt`, the root `add_subdirectory`, the `ivs-flatwrap-sources.txt` writer, the `c-build` module list and the `tools/check/integrity.cmake` SPDX alternation
- [x] `data/expected/flat-refusals.ids` — the seven pinned tokens, append-only (plan 14-01). **Satisfied by** `data/expected/flat-refusals.ids` (seven lines), minted into one case per token by `tests/pack-tap.sh`
- [x] `tests/pack-tap.sh` third argument and `tests/CMakeLists.txt`'s `pack` line (plan 14-01). **Satisfied by** `tests/pack-tap.sh`'s `$3` (the tool path) and `tests/CMakeLists.txt`'s `pack` line passing `$<TARGET_FILE:ivs_flatwrap>`
- [x] `data/expected/tests/pack.ids` — nine ids (plan 14-01). **Satisfied by** `data/expected/tests/pack.ids` (23 ids: 14 existing plus the nine; 23 `ok`, no `not ok`)
- [x] `tools/check/controls/c-build-unlisted-flatwrap-source/` and its registry line (plan 14-01). **Satisfied by** that directory's `CHECK`, `EXPECT` and `overlay/tools/flatwrap/extra.c`, plus its line in `data/expected/controls/runner-hygiene.ids` (`controls` 401 to 402, measured `controls=402/402`)
- [x] `tools/check/controls/_good/tree/CMakeLists.txt` — the seventh toy source list (plan 14-01). **Satisfied by** the toy tree's seventh list, and the thirteen toy files this phase extends (39 added lines: 30 registry lines plus 9 requirement rows)
- [x] `data/expected/ivsn-literals.ids` — five re-pinned values (plan 14-02). **Satisfied by** the five re-pinned literals in `data/expected/ivsn-literals.ids`, the profile field at 1, the five explicit vector `config` overrides and the C default at 1
- [x] `tests/runner-process-tap.sh` sixth argument and `tests/CMakeLists.txt`'s `runner-process` line (plan 14-03). **Satisfied by** the sixth argument (the wrap tool) and the `runner-process` line passing `$<TARGET_FILE:ivs_flatwrap>`
- [x] `data/expected/tests/runner-process.ids` — three ids (plan 14-03). **Satisfied by** that file (138 ids: 135 existing plus three), `ctest --preset dev -R '^runner-process$'` green with no `not ok`
- [x] No new framework, no new suite, no new gate step: every item above is a file, a case or an id inside what exists. **Measured at the sign-off:** `steps=38/38` (`ls tools/check/steps/*.sh | wc -l` is 38), `faults=238/238`, `goldens=29`, `controls=402/402`, `pkg-selftest=19/19` - no framework, no suite and no gate step was added

---

## Manual-Only Verifications

All phase behaviors have automated verification. The phase's ceiling is no claim (D-50, unchanged by this
phase), so nothing is handed off, no row carries `evidence: none`, and the four `14-04` verify blocks plus the
nine per-task rows above are the whole contract. The one behavior no automated check can judge - whether an
operator's own cartridge looks right on a screen - is D10 (physical measurement) and D11 (owner-local
EXEC/GROM runs), already recorded as hard stops in `.planning/STATE.md`, and it is not re-asked here.

---

## Validation Sign-Off

- [x] All tasks have `<automated>` verify or Wave 0 dependencies - nine tasks, nine automated commands, every one run and green
- [x] Sampling continuity: no 3 consecutive tasks without automated verify - the quick run after every task and the full gate after every wave, and the closing task ran the full gate after its own commit
- [x] Wave 0 covers all MISSING references - the ten bullets above, each naming the file that satisfies it
- [x] No watch-mode flags - no `--watch`, no `-j`-style long-poll and no interactive flag in any of the nine commands
- [x] Feedback latency measured, and the plan-time bound amended rather than re-blessed: the quick run is 29.2 s cold and 37.6 s warm on this host (`runner-process` 29.5 s of it), so the plan-time "14 s" was stale by measurement (recorded as a plan defect in `14-04-SUMMARY.md`); the bound is the measured 37.6 s and it is under the full gate's measured 86-147 s
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** 2026-11-26 - `sh tools/gate.sh` and `dash tools/gate.sh` both exit 0 with byte-identical last
lines (`cmp` clean), and one Ubuntu 24.04 container gate (`gate-linux: exit 0`, four documented host tokens
differing: `cc=GNU`, `core-imports=5`, `archive=sha256`, `refs-verified=0/4`). The measured line, quoted once
from `/tmp/g14sh.tail`:

```
gate: PASS steps=38/38 controls=402/402 api-calls=19 api-statuses=15 api-rows=61 pinned-defines=120 cc=AppleClang c-presets=2 fault-runs=238 core-imports=7 purity-selftest=7/7 ctest-runs=2 ci-runs=100 ci-uses=2 ci-workflows=2 ci-summary-selftest=11/11 claims=28 provisional=27 cp1610-rows=74 patterns=1024 unclaimed=0 ambiguous=0 coincidences=1 agree=71 resolved=3 vectors=412 source=291 policy=121 rows-pending=0 policies-pending=0 bus-figure-pages=35 blind-files=12 second-read=12/12 scanned=689 waived=0 discrepancies=110 pending-resolvers=20 open=101 d10-items=47 faults=238/238 tracked=2745 binaries-listed=0 rights-pinned=4/4 terms=9 doc-files=63 profile-keys=154 goldens=29 reports=30 host-dependent=3 release-only=2 mode=tree hygiene-files=2745 history-commits=1 gates=117 lines=357 input-rows=31 input-policies=11 input-profile-fields=31 files=2745 listings=7 listing-words=1386 manifests=11 effective-yes=8 effective-no=3 rights=10 rights-cleared=1 literal-rows=80 views=2 dump-lines=160 prefix-files=11 archive=members public-symbols=19 defined=141 hdr-alone=10/10 consumer-lines=164 cxx-link=ok pkg-selftest=19/19 records=154 fields=154 unknown-fields=47 psg-rules=21 psg-vectors=120 psg-policies=12 release-selftest=11/11 runner-files=18 cases=14 sources=13 refs-verified=3/4 state-rows=88 state-cuts=18 tests-expected=5014 tests-executed=5010 tests-skipped=4 tests-missing=0 docs=64 claim-refs=26 stic-rules=67 stic-rows=64 video-frames=26 video-hash-ties=18 video-frame-pairs=9 tags=10 tiers=8
```

---

## Amendment, 2026-11-26 (the phase's code review round)

Appended after the signature, never rewritten: the Approval block above records the line measured at this
validation's own commit and stays as it was measured there. Three of its tokens are stale at the fix head,
each with a named cause rather than a re-bless:

- `gates=117 lines=357` -> `gates=117 lines=356`: review WR-03 deleted the vacuous `SCN-STUB-JD-AS-J`
  registry row at `fc25ce9` (a `pass 0` row cannot fail, so it is not a must-fail mechanism; the case still
  runs from its inventory row). `gates` does not move because it counts distinct gate names. The row
  `14-04-T1` above therefore reads `lines=356` at the fix head.
- `tests-expected=5014 tests-executed=5010` -> `5016`/`5012`: review CR-01's `pack.flatwrap.neg.flatwrap-empty`
  (`bf3215d`), one id counted once per preset.
- `tracked=2745 files=2745 hygiene-files=2745` -> `2747`: the two record files written after the measured
  line (`14-04-SUMMARY.md` in `2e27267` and `14-REVIEW.md`).

`steps=38/38`, `faults=238`, `goldens=29` and `controls=402/402` are unchanged, so this amendment adds no
step, fault, golden or control. The measured line at the fix head (`sh tools/gate.sh`, exit 0, no `^-- FAIL`,
`gate-time total=97`) is quoted in `14-REVIEW.md`'s Dispositions section. The review also fixed WR-01
(`4efc806`, both operator paths opened without blocking, proved under `timeout` with a real FIFO) and WR-02
(`94300ec`, C41's three present-tense statements given the inline pointer GUARDRAILS 2.10 asks for), and
deferred IN-01..IN-03 with their reasons. No evidence tier moved and no claim was raised: the phase's
ceiling is still no claim (D-50).
