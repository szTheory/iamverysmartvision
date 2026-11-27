---
phase: "19"
slug: "discriminating-the-display-path"
status: complete
nyquist_compliant: true
wave_0_complete: true
created: "2026-11-27"
---

# Phase 19 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

**Completed after execution.** This file was written before the phase started and still read `status: draft`,
`nyquist_compliant: false`, every task `⬜ pending` and every Wave 0 box unchecked while the three plans were already
green (review finding IN-07). It now describes the seven tasks that shipped, and **every row is green because its
command was run on this tree during the Phase 19 review-and-fix pass** — not inherited from a SUMMARY.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | the project's own TAP shell suites under `ctest` (`dev`, `release` presets); the `stic` suite carries the discrimination ids |
| **Config file** | `tests/CMakeLists.txt`; `data/expected/tests/stic.ids` and `data/expected/tests/runner-process.ids` are the id inventories |
| **Quick run command** | `IVS_TEST_OUT_DIR=<tmp> ctest --preset dev -R '^stic$' --output-on-failure` — the ids are read from `<tmp>/stic.txt`, whose grammar is `ok <n> <id>` with **no dash** (`tests/harness/ivs_test.h`); a passing suite is not echoed by ctest, so its stdout is never the source |
| **Full suite command** | `sh tools/gate.sh` **then** `dash tools/gate.sh` — **sequentially, never concurrently** (lesson 33: both write `build/`, and a concurrent pair reports a phantom `c-build-failed`) |
| **Measured runtime** | `stic` 0.38 s and `runner-process` 23.8 s on this host (`dev`); the two gate shells are timed in the disposition appended to `19-REVIEW.md` |

---

## Sampling Rate

- **After every task commit:** `ctest --preset dev -R '^stic' --output-on-failure`
- **After every plan wave:** the two gate runs, sequentially, `cmp`-ing their summary lines
- **Before `/gsd:verify-work`:** the full gate plus one `sh tools/gate-linux.sh` (copy `build/gate-linux.log` aside first); if the daemon is unavailable, a visible `not run` line, never a pass
- **Max feedback latency:** 300 s

---

## Per-Task Verification Map

**Seven tasks shipped, not eight**: plan 19-01 has three, plan 19-02 has two (landed in one commit — the probe and its
containment cannot be separated, see `19-02-SUMMARY.md`), and plan 19-03 has two.

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 19-01-T1 | 01 | 1 | VID-07 | T-19-01 | G1 moves under its own field only, and its guard can fail | suite | **V1** — `ctest --preset dev -R '^stic$'` → `ok 100`–`ok 103` (`stic.disc.g1-window`, `stic.judge.disc-g1-dead`, `stic.disc.displayed-equivalence`, `stic.judge.disc-equivalence-dead`), `executed=111 skipped=0 failed=0` | ✅ | ✅ green |
| 19-01-T2 | 01 | 1 | VID-07 | T-19-02 | the latch rule is a profile field before it is swept, and the default reproduces today's bytes | step | **V3** — `profile.sh` → `-- profile pass records=155 fields=155 unknown-fields=48`; `state` suite → `ok 7 state.literal.params-digest`, `executed=61 failed=0`; the field row is `stic.display_latch_cleared_each_frame = 1`, `tag: unknown-default-chosen`, `decision: null`, with `data/expected/profile-fields.ids:133` in the same commit `6cffdb4` | ✅ | ✅ green |
| 19-01-T3 | 01 | 1 | VID-07 | T-19-01 | the remaining three guests move under their own fields only | suite | **V1** — `ok 104`–`ok 109` (`stic.disc.g2-gating`, `stic.disc.g3-read`, `stic.disc.g4-latch` and their three judges), no profile row, literal or golden moved by plan 19-01's third task | ✅ | ✅ green |
| 19-02-T1 | 02 | 2 | VID-07 | T-19-03 | the probe cannot leave `refs/`, refuses CI, and is bounded | suite | **V2** — `ctest --preset dev -R '^runner-process$'` → `ok 128`–`ok 130` (`disc-sweep.ci-refusal`, `disc-sweep.output-tree`, `disc-sweep.bound`), each run against a copy of the helper in a scratch root outside the work tree; `disc-sweep.bound` now also proves `--max-runs 2` is honoured as a cap (two rows, the first two offsets, a header naming the cap) | ✅ | ✅ green |
| 19-02-T2 | 02 | 2 | VID-07 | T-19-09 | the probe is a visible skip and nobody's exit criterion | suite + gate | **V2**/**V5** — `ok 36 runner-process.scenario.disc-sweep-probe # SKIP disc-sweep-probe: needs operator content (ended missing-fixture, exit 13, as expected)`, and the gate prints exactly two `-- SKIPPED <preset>/runner-process/runner-process.scenario.disc-sweep-probe: ...` lines, one per preset | ✅ | ✅ green |
| 19-03-T1 | 03 | 3 | VID-07 | T-19-04 | the exit is decided by measurement, and the record cannot be improvised | verify | **V4** — the 16-cell matrix is diagonal (masks `0xAAAA`, `0xCCCC`, `0xF0F0`, `0xFF00`, one per guest) with `ok 111 stic.judge.disc-diagonal-two-fields` shown able to fail (`judge: the diagonal rejected the entangled guest at cell 2 (bits 0x2), observed 1`), and both enumerations were committed in `2118d71`, an ancestor of the measurement commit `9b8c720` | ✅ | ✅ green |
| 19-03-T2 | 03 | 3 | VID-07 | T-19-05 | no claim, tier or golden moves, and the enumeration is honoured | step | **V4**/**V5** — the gate token line (`claims=29 provisional=28 goldens=29 faults=238/238 steps=38/38 prefix-files=11 controls=405/405`), the field still `unknown-default-chosen` (D-51), and `9b8c720`'s nine paths equal the Exit B enumeration in `19-DECISION.md` | ✅ | ✅ green |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

**Continuity:** all seven tasks have an automated verify, so no three consecutive tasks lack one and no row needs a
manual fallback.

---

## The commands this pass ran

### V1 — the `stic` suite and the matrix (`19-01-T1`, `19-01-T3`, `19-03-T1`)

```sh
IVS_TEST_OUT_DIR=/tmp/g19-fix ctest --preset dev -R '^stic$' --output-on-failure   # rc 0
grep -c '^ok ' /tmp/g19-fix/stic.txt                                              # 111
grep -c 'not ok' /tmp/g19-fix/stic.txt                                            # 0
tail -1 /tmp/g19-fix/stic.txt       # # suite=stic executed=111 skipped=0 failed=0
```

The id set of the result file is byte-identical to `data/expected/tests/stic.ids` (`diff` clean), so the suite's
`executed=111` is the inventory's 111 and no id was added or removed. The twelve Phase 19 ids are `ok 100`–`ok 111`.
The binary's own stdout reproduces the measured matrix of `19-DECISION.md`:

```
# sweep-diagonal: G1 field 0 mask 0xAAAA default 1 alternate 0
# sweep-diagonal: G2 field 1 mask 0xCCCC default 1 alternate 0
# sweep-diagonal: G3 field 2 mask 0xF0F0 default 0 alternate 1
# sweep-diagonal: G4 field 3 mask 0xFF00 default 4 alternate 7
# judge: the diagonal rejected the entangled guest at cell 2 (bits 0x2), observed 1
```

### V2 — `runner-process` and the probe's containment (`19-02-T1`, `19-02-T2`)

```sh
IVS_TEST_OUT_DIR=/tmp/g19-rp ctest --preset dev -R '^runner-process$' --output-on-failure   # rc 0, 23.8 s
grep 'disc-sweep' /tmp/g19-rp/runner-process.txt
#   ok 36 runner-process.scenario.disc-sweep-probe # SKIP disc-sweep-probe: needs operator content (ended missing-fixture, exit 13, as expected)
#   ok 128 runner-process.disc-sweep.ci-refusal
#   ok 129 runner-process.disc-sweep.output-tree
#   ok 130 runner-process.disc-sweep.bound
tail -1 /tmp/g19-rp/runner-process.txt   # # suite=runner-process executed=164 skipped=3 failed=0
```

167 `ok` lines, 0 `not ok`, and the id set is identical to `data/expected/tests/runner-process.ids` (167 ids), so the
cap limb added to `disc-sweep.bound` in the fix pass added no id. The cap limb's own measurement, read from the table
the helper wrote: two offset rows (`14000`, `14934`), and the header
`# disc-sweep: 2 of 6 offsets (run-count cap 2, bound 16), ...` — so the option now changes what the sweep does
instead of only refusing.

### V3 — the profile chain (`19-01-T2`)

```sh
ROOT=$PWD CHECK_DIR=$PWD/tools/check sh tools/check/steps/profile.sh
#   -- profile pass records=155 fields=155 unknown-fields=48
IVS_TEST_OUT_DIR=/tmp/g19-st ctest --preset dev -R '^state$'                        # rc 0, 3.06 s
#   ok 7 state.literal.params-digest
#   # suite=state executed=61 skipped=0 failed=0
grep -n display_latch_cleared_each_frame data/expected/profile-fields.ids            # :133
```

The row itself, read from `data/profiles/intv-2609-ntsc-family-p1.json`:
`stic.display_latch_cleared_each_frame` value `1`, `tag: unknown-default-chosen`, `conflict: C82`,
`resolving_evidence: test: stic.disc.g4-latch and stic.disc.g1-window (plan 19-01) pin the default and its one-field
reversal; measurement: D10-VID-08`, `decision: null` — so the exit appended an observation and decided no default
(D-51, D-54).

### V4 — the exit record, the registry and the field (`19-03-T1`, `19-03-T2`)

```sh
git merge-base --is-ancestor 2118d71 9b8c720                     # rc 0: the enumerations precede the measurement
git show --name-only --format= 9b8c720                           # 9 paths, equal to the Exit B enumeration
grep -n 'stic-disc mutant' data/expected/important-gates.ids     # :702-:707, six rows, one per committed stand-in
grep -n 'VID-07' .planning/REQUIREMENTS.md                       # :30 [x]; :91 `| VID-07 | Phase 19 | Complete |`
```

`9b8c720`'s paths are `.planning/PROJECT.md`, `.planning/REQUIREMENTS.md`, `.planning/ROADMAP.md`,
`.planning/STATE.md`, `19-03-SUMMARY.md`, `19-DECISION.md`, `data/discrepancies.json`,
`data/expected/important-gates.ids` and `docs/stic/reference-notes.md` — the same nine the record's
`## Exit B enumeration` lists, in both directions.

### V5 — the two gate shells (`19-02-T2`, `19-03-T2`)

`sh tools/gate.sh` then `dash tools/gate.sh`, one after the other and never concurrently (lesson 33), the last line of
each compared whole with `cmp`. The measured line and the two probe skip lines are quoted in the disposition appended
to `19-REVIEW.md`.

---

## The measured state of the phase

Every token this phase is bound to, read from the gate's own summary line in the fix pass (both shells byte-identical):

| token | value | |
|---|---|---|
| `steps` | `38/38` | ✓ no new gate step |
| `controls` | `405/405` | ✓ no new control |
| `faults` | `238/238` | ✓ no new fault site |
| `goldens` | `29` | ✓ no golden moved |
| `claims` / `provisional` | `29` / `28` | ✓ no claim minted |
| `prefix-files` | `11` | ✓ no installed file |
| `tests-expected` | `5114` | ✓ the twelve ids across two presets |
| `tests-executed` | `5108` | ✓ (`tests-skipped=6`, `tests-missing=0`) |
| `gates` / `lines` | `123` / `378` | ✓ the six `stic-disc` rows under one new gate name |
| `profile-keys` / `fields` / `records` | `155` | ✓ the one field |
| `unknown-fields` | `48` | ✓ the field stays `unknown-default-chosen` |
| `discrepancies` | `110` | ✓ no C id minted |
| `stic-rules` | `67` | ✓ no rule statement moved |

---

## Wave 0 Requirements

Every reference is present on the tree, checked item by item:

- [x] four guests authored as the suite's own decle-builder data in `tests/stic/stic_disc.c` (the `stic_progs.c`
      convention, no binary, nothing rights-bearing) — `sd_prog_g1_criterion`, `sd_prog_g2`, `sd_prog_g3`, `sd_prog_g4`
      in `IVS_STIC_TEST_SOURCES`, with the four committed stand-ins and the diagonal sweep in the same module
- [x] the latch rule as a new profile field row with its tag and cite, and C82's `evidence` extended in the same
      commit `6cffdb4` (`stic.display_latch_cleared_each_frame`, `data/expected/profile-fields.ids:133`)
- [x] the `W1` chain retargeted in the same commit (parameter digest `ff4eb16a...` and all five digest-carrying
      literals of `data/expected/ivsn-literals.ids`), held by `ok 7 state.literal.params-digest`
- [x] the 16-cell matrix and the diagonal assertion with its `mutant` row (`ok 110`, `ok 111`,
      `data/expected/important-gates.ids:707`)
- [x] the bounded `refs/`-only sweep with its `skip-ok` markers (`tools/disc-sweep.sh`,
      `data/scenarios/disc-sweep-probe.ivss`, the `skip-ok` lines in both inventories)
- [x] the C-row observation append in `data/discrepancies.json` (C1, C6, C82, C94 each carry one new
      `Observation, 2026-11-27 (Phase 19 sweep, stic.disc.sweep-diagonal)` sentence, all four still `open`)

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| none | — | — | The four observables are machine state read at the seam; the sweep is a matrix of parameter rows; the exit is a recorded comparison. The one act no tool can perform — the D10-VID-08 hardware probe — is a handoff row with `evidence: none`, never a phase criterion. |

*No `data/expected/handoffs.ids` row is added by this phase: D10-VID-08 already exists and Phase 20 writes its packet.*

---

## Outstanding evidence (recorded as `not run`, never as a pass)

| Item | State | Measured reason | What would discharge it |
|------|-------|-----------------|-------------------------|
| `sh tools/gate-linux.sh` (the container gate, `GUARDRAILS-v0.4` §4) | **not run** | the docker daemon is unresponsive on this host (measured in Phase 18: `timeout 20 docker info` → rc 124, twice), so the container gate is recorded as not run and no `ivs-*` image, container or volume was created | one container gate on a host whose daemon answers, ending `gate: PASS` in `build/gate-linux.log` |

This is **outstanding evidence, not a pass**. It is a local pre-push helper and not a gate step (`steps=38/38`), so its
absence weakens no id this phase added: every id above ran under the local gate on this host.

---

## Validation Sign-Off

- [x] All tasks have `<automated>` verify or Wave 0 dependencies — seven tasks, five automated command groups
      (`V1`–`V5`), each run during this pass
- [x] Sampling continuity: no 3 consecutive tasks without automated verify — none of the seven lacks one
- [x] Wave 0 covers all MISSING references — every Wave 0 item is present on the tree, checked item by item above
- [x] No watch-mode flags — none in either suite or in the gate
- [x] Feedback latency < 300s — the `stic` suite is 0.38 s and the full gate pair is measured in the `19-REVIEW.md`
      disposition
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** complete — audited 2026-11-27 in the Phase 19 review-and-fix pass. Every row green because its command
was run on this tree; one item handed off as outstanding evidence (the container gate) and recorded as `not run`,
never as a pass.
