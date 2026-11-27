---
phase: 19-discriminating-the-display-path
verified: 2026-11-27T09:41:00Z
status: passed
score: 23/23 must-haves verified
covered_files: [
  ".planning/phases/19-discriminating-the-display-path/19-01-PLAN.md",
  ".planning/phases/19-discriminating-the-display-path/19-01-SUMMARY.md",
  ".planning/phases/19-discriminating-the-display-path/19-02-PLAN.md",
  ".planning/phases/19-discriminating-the-display-path/19-02-SUMMARY.md",
  ".planning/phases/19-discriminating-the-display-path/19-03-PLAN.md",
  ".planning/phases/19-discriminating-the-display-path/19-03-SUMMARY.md",
  "cmake/IvsSticTable.cmake",
  "core/src/machine/machine_bus.c",
  "core/src/stic/stic.h",
  "core/src/stic/stic_events.c",
  "core/src/stic/stic_params.h",
  "core/src/stic/stic_regs.c",
  "data/discrepancies.json",
  "data/expected/faults/stic.ids",
  "data/expected/important-gates.ids",
  "data/expected/ivsn-literals.ids",
  "data/expected/profile-fields.ids",
  "data/expected/runner-scenarios.ids",
  "data/expected/tests/runner-process.ids",
  "data/expected/tests/stic.ids",
  "data/profiles/intv-2609-ntsc-family-p1.json",
  "data/scenarios/disc-sweep-probe.ivss",
  "docs/owner-images.md",
  "docs/stic/reference-notes.md",
  "tests/CMakeLists.txt",
  "tests/machine/machine_stic_params.c",
  "tests/runner-process-tap.sh",
  "tests/stic/stic_cases.h",
  "tests/stic/stic_disc.c",
  "tests/stic/stic_main.c",
  "tools/disc-sweep.sh"
]
covered_digest: "v4:sha256:68cdd56e8a36d5fe470f0b5e838043156b1cfad4a265829d75bc0690864a42b4"
behavior_unverified: 0
overrides_applied: 0
re_verification: null
---

# Phase 19: Discriminating the Display Path Verification Report

**Phase Goal:** The four rival display-path rules are separated diagonally by authored
guests, so that the default is either decided by discrimination - both settings pinned
and every dependent expectation retargeted in one commit - or recorded unchanged with
the rivals named.
**Requirement:** VID-07 (`.planning/REQUIREMENTS.md:30` `[x]`, `:91` `| VID-07 | Phase 19 | Complete |`; the only
requirement the traceability table maps to Phase 19, so no orphaned requirement exists)
**Verified:** 2026-11-27T09:41:00Z
**Status:** passed
**Re-verification:** No — initial verification (no previous `19-VERIFICATION.md`)

## Goal Achievement

Every claim below was re-measured on this tree. No SUMMARY.md figure is quoted as evidence.

### Observable Truths

Truths are the ROADMAP's five Success Criteria (non-negotiable contract, `C1`…`C5`) merged with the three plans'
frontmatter truths (`19-01` T1–T9, `19-02` U1–U7, `19-03` V1–V7), deduplicated: a plan truth that restates a criterion is
scored once, under the criterion's wording.

| # | Truth | Status | Evidence |
| --- | ----- | ------ | -------- |
| 1 | **C1/T1** Four authored guests exist, one per rival rule, as project-owned decle-builder data with the derivation comment, never a committed binary or third-party content | ✓ VERIFIED | `tests/stic/stic_disc.c` (`sd_prog_g1_criterion`, `sd_prog_g2`, `sd_prog_g3`, `sd_prog_g4`, all built from `sc_w_*`), ids `stic.disc.g1-window`/`g2-gating`/`g3-read`/`g4-latch` = `ok 100`/`104`/`105`/`106`; module header `:1-45` states no byte is rights-bearing; `git ls-files` has no binary under `tests/stic/` |
| 2 | **C1/T2** Each guest exposes exactly one registered machine-state observable at the machine seam — never a pixel or frame hash — G1's being the blocked-write counter with the criterion's `displayed` reading carried as an unregistered control | ✓ VERIFIED | `sd_guests[]` (`:1085-1090`): G1/G2 `SD_OBS_BLOCKED` (`report.blocked_writes`), G3 `SD_OBS_DISPLAYED`, G4 `SD_OBS_DISPLAYED3`; reads via `ivs_machine_stic_state`; `:4` "never a pixel and never a hash of a frame"; no pixel/framebuffer symbol in the file |
| 3 | **C1/T3** G1's case asserts the criterion's own reading *and* measures the coupling that forced the re-derivation | ✓ VERIFIED | `sd_g1_window` (`:352-400`): criterion guest `displayed` 0 shipped / 1 at 2900 (limbs `SD_G1_C_*`), witness `blocked` 1/0 (limbs `SD_G1_W_*`), plus two explicit gating-alternate limbs; `ok 100` |
| 4 | **C1/T4** Each guest's default is stated and is what today's bytes produce (G1 1, G2 1, G3 no, G4 yes/no/no); the criterion's `displayed` for G1/G3 is the rival prediction, said so | ✓ VERIFIED | Sweep masks print `default 1`, `default 1`, `default 0`, `default 4` (=1/0/0) with `alternate 0/0/1/7`; guard limbs assert both values; comments at `:352-356`, `:773-777`, `:947-950` |
| 5 | **C1/T5/T8** Every guest ships a liveness guard, a committed stand-in that makes the guard fail, and a judge that is red if the guard passes on the stand-in; the displayed-word equivalence ships the same three | ✓ VERIFIED | Judges `ok 101`, `103`, `107`, `108`, `109`; I mutated each stand-in back into its real guest and each judge went red — `judge-vacuous` on `stic.judge.disc-g1-dead`, `-g2-dead`, `-g3-dead`, `-g4-dead` and `-equivalence-dead` (log below) |
| 6 | **C2/T6** The three settings that cannot move today are reachable as parameter-driven paths whose default reproduces today's behaviour: a gating member, an exact-address READ match that keeps the mode-select limb, a frame-start latch member; and the latch rule is a profile row **before** it is swept | ✓ VERIFIED | `machine_bus.c:88-93`, `stic_regs.c:30-60`, `stic_events.c:55-62`; the latch row lands in `6cffdb4`, the sweep in `2118d71` (ancestor); `git merge-base --is-ancestor 6cffdb4 2118d71` → 0 |
| 7 | **C3/C4/T7** The profile change and the whole `GUARDRAILS-v0.4` **W1** chain land in the causing commit; the field keeps its tag; no claim, golden or rule statement moves; `data/expected/discrepancies.ids` is satisfied by non-movement | ✓ VERIFIED | `6cffdb4` carries field+tag+cite+conflict+revisit (`data/profiles/…json:1424-1433`), `profile-fields.ids:133`, params digest + **all five** digest literals, C82 evidence, `docs/stic/reference-notes.md`, `PROJECT.md` row; `git show --name-only 6cffdb4 \| grep discrepancies.ids` → 0 hits; gate `claims=29 provisional=28 goldens=29 stic-rules=67 discrepancies=110` |
| 8 | **C1/T9** Every guest's access stamp is measured at run time from the machine's own access record, and the case proves the stamp strictly inside (G1) / outside (G2) the bounds computed from the profile's parameters | ✓ VERIFIED | `sd_measure` (`:167-198`) composes `unit_start + offset_cpu_cycles*ticks_per_cpu_cycle + access_strobe_ticks` — byte-identical to `machine_bus.c:85-86`; `sd_guard_g1` `:322-330` and `sd_g2_gating` `:669-678` assert the relations against `p->reg_window_ticks`, `SD_WINDOW_WIDE_CYCLES*ticks_per_cpu_cycle` and `p->gmem_window_ticks` |
| 9 | **C2/V1** The four fields are swept as sixteen cells in one committed id, each cell a stack copy of the generated row, asserting the diagonal as **one** comparison with a per-cell diagnostic | ✓ VERIFIED | `stic.disc.sweep-diagonal` = `ok 110`; `sd_cell` (`:1022-1071`) does `*out = *p` and returns 0 if a field is unreachable; `sd_diagonal` (`:1123-1196`) is one `alternate iff own bit` rule over 16 cells; diagnostic names reason/cell/bits/guest/observed/expected |
| 10 | **C2/V2** The diagonal is falsifiable: a committed stand-in drives the same comparison on G1's entangled reading and is red if the comparison accepts a guest that moves under two fields | ✓ VERIFIED | `stic.judge.disc-diagonal-two-fields` = `ok 111`, printing `the diagonal rejected the entangled guest at cell 2 (bits 0x2), observed 1`; I re-registered G1 by its entangled `displayed` word in the sweep and `ok 110` went red with `reason 1` (`SD_DIAG_FOREIGN`) at cell 2 (log below) |
| 11 | **C2/V3** The criterion's own G1 reading is not lost to the re-derivation | ✓ VERIFIED | The same case asserts `displayed` 0→1 under the window field and 1 under the gating alternate; recorded in `19-DECISION.md` ("The off-diagonal entry the phase already knows about") |
| 12 | **C3/V4** Both exits' commit shapes are written into the phase record **before** the sweep, measured at the start commit, and the final commit's file list is compared against Task 1's blob — never the working tree | ✓ VERIFIED | `git merge-base --is-ancestor 2118d71 9b8c720` → 0; `cmp` of the `## Exit B enumeration` section between `2118d71:19-DECISION.md` and `HEAD:19-DECISION.md` → identical; `git diff 2118d71..HEAD -- 19-DECISION.md \| grep '^-'` → 0 removed lines; the 9 paths of `9b8c720` `diff`-equal to the 9 Exit B bullets (sorted, both directions) |
| 13 | **C3** Exactly one exit is recorded, and the exit taken is the one whose enumeration was honoured | ✓ VERIFIED | `19-DECISION.md` "Exit B taken - … default unchanged, observation appended, rivals named"; `9b8c720` = the Exit B list, and `data/expected/discrepancies.ids`, every profile file, `ivsn-literals.ids`, `data/stic/rules.json`, `data/golden/` and `pkg-install-files.ids` are absent from it |
| 14 | **C4/V6** The four rival rows carry the appended dated observation, the field stays `unknown-default-chosen`, no claim cites the observation, and `claims`/`provisional`/`goldens` are unmoved | ✓ VERIFIED | `data/discrepancies.json` C1/C6/C82/C94 each carry exactly one `Observation, 2026-11-27 (Phase 19 sweep, stic.disc.sweep-diagonal)` sentence (C82 has 2 total because its 2026-11-26 sentence remains), all four `status: open`, all four `unknown-default-chosen:` prefixes and resolvers intact, each says neither setting is preferred; `grep` of the claims registry for the sweep/probe id → 0 hits; gate `claims=29 provisional=28 goldens=29`; `git log 6cffdb4~1..HEAD -- data/golden/ data/stic/rules.json` → empty |
| 15 | **C4/U6** No record, log, id or comment holds a digest, size, path or name of an operator file | ✓ VERIFIED | The probe's table columns are `offset_cpu_cycles outcome displayed`; `runner-process.disc-sweep.output-tree` asserts no 64-hex token in the table (`ok 129`); `binaries-listed=0`, `rights=11`, `rights-cleared=1` unchanged in the gate |
| 16 | **C5/U1** The owner-local sweep is a helper, not a gate step, and refuses CI with one stderr line and exit 2 | ✓ VERIFIED | `steps=38/38`; no `tools/check/steps/*` references `disc-sweep`; `runner-process.disc-sweep.ci-refusal` = `ok 128` (both `CI`+`GITHUB_ACTIONS` and `GITHUB_ACTIONS` alone) |
| 17 | **C5/U2** Its output is refused outside `refs/` on the flatwrap rule verbatim — literal, `..` and symlink spellings all refused with no file left behind, and the same write under `refs/` accepted | ✓ VERIFIED | `runner-process.disc-sweep.output-tree` = `ok 129` (three refusals + one accepted write + six offset rows + no digest) |
| 18 | **C5/U3** It is bounded by a wall-clock limit and a run count; a count above the bound is refused, and an accepted value caps the walk | ✓ VERIFIED | `runner-process.disc-sweep.bound` = `ok 130`; `--max-runs 100000` → exit 2, no output; `--max-runs 2` → two rows (`14000`, `14934`) and header `# disc-sweep: 2 of 6 offsets (run-count cap 2, bound 16)`; `wall_bound=300`, `run_wall_ms=20000` |
| 19 | **C5/U4** It is `skip-ok` in both inventories with `missing-fixture`/exit 13, and the gate prints one visible `-- SKIPPED` line per preset | ✓ VERIFIED | `runner-process.ids:154` (`… skip-ok`) and `runner-scenarios.ids:57` (`disc-sweep-probe 13 missing-fixture deterministic skip-ok -`); gate prints 6 `-- SKIPPED` lines, the probe's one per preset; `tests-skipped=6`, `tests-missing=0` |
| 20 | **C5/U5** Its absence or an inconclusive run leaves criteria 1–4 standing: no criterion, id or verify block reads the probe's output | ✓ VERIFIED | `grep -rn disc-sweep` over the tree resolves only to the helper, its three cases, its two inventory rows, its scenario, `docs/owner-images.md` and the phase records that describe it as not-a-criterion; every other truth above is measured without it |
| 21 | **C5/U7** The three containment limbs are judged as cases of the existing `runner-process` suite against a copy of the helper in a scratch root, on every gate run | ✓ VERIFIED | `tests/runner-process-tap.sh:2363-2471` builds a fresh scratch root with its own `tools/`, `data/scenarios/`, `build/release/runner/ivs-run` symlink and `refs/`; suite `executed=164 skipped=3 failed=0` |
| 22 | **V5** The six registry rows land in the commit that flips VID-07, and the §2.7 split is recorded as a deviation | ✓ VERIFIED | `data/expected/important-gates.ids:702-707` (six `stic-disc mutant` rows, one per stand-in incl. the equivalence one) in `9b8c720` with the `Pending`→`Complete` flip; `gates` 122→123, `lines` 372→378; deviations recorded in `19-01-SUMMARY.md:60` and `19-03-SUMMARY.md:203` |
| 23 | **V7** The cost checkpoint is recomputed by arithmetic from `.planning/ci-runs.tsv` at the plan's start commit, with exactly one verdict | ✓ VERIFIED | Recomputed here from the last five rows: `cpu_s` 368/530/572/580/584 → median **572**, `build_s` 35/49/49/51/51 → **49**, `gate_s` 115/154/164/165/171 → **164**, matching `19-03-SUMMARY.md:104-133`; verdict `reprice`, priced charge `0.65+0.40+0.16=1.21` against the frozen 1.64 |

**Score:** 23/23 truths verified (0 present, behavior-unverified; 0 overrides)

No truth asserts a state transition or cancellation/ordering invariant that no test exercises: every behaviour-dependent
claim above is exercised by a committed id I ran on this tree (the `stic`, `state` and `runner-process` suites, plus the
mutation experiments recorded under "Behavioral Spot-Checks"). `behavior_unverified: 0` is therefore a measured 0, not a
default.

### Deferred Items

None. Every Success Criterion is discharged by Phase 19 itself; the one item that cannot be discharged on this host
(the container gate) is outstanding evidence, not a deferred criterion — see "Outstanding evidence".

### Advisory (New Scope, Unevidenced)

Not applicable: this is an initial verification, not a re-verification, so the #3304 evidence gate does not run.

### Required Artifacts

| Artifact | Expected | Status | Details |
| -------- | -------- | ------ | ------- |
| `tests/stic/stic_disc.c` | four guests, four guards, five stand-ins, five judges, the equivalence id, the sweep and its judge | ✓ VERIFIED | 1274 lines; 12 ids = `ok 100`–`ok 111`; in `IVS_STIC_TEST_SOURCES` (`tests/CMakeLists.txt`), so also compiled into `ivs_fault_suite(stic …)` with no new fault site (`faults=238/238`) |
| `core/src/machine/machine_bus.c`, `core/src/stic/stic_regs.c`, `core/src/stic/stic_events.c` | three parameter-driven paths, default = today's behaviour | ✓ VERIFIED | branches at `:88-93`, `:30-60`, `:55-62`; each short-circuits at the shipped value |
| `core/src/stic/stic_params.h`, `cmake/IvsSticTable.cmake` | two policy members emitted from the profile row the generator already validates | ✓ VERIFIED | `access_gating_when_off`, `display_latch_cleared_each_frame`; range-checked in `tests/machine/machine_stic_params.c` |
| `data/profiles/intv-2609-ntsc-family-p1.json`, `data/expected/profile-fields.ids` | the latch rule as a profile row | ✓ VERIFIED | value `1`, tag `unknown-default-chosen`, cite `none-found:`, conflict `C82`, `decision: null`; inventory line `:133` |
| `data/expected/ivsn-literals.ids` | params digest + every snapshot literal retargeted in the causing commit | ✓ VERIFIED | exactly five digest-carrying values moved in `6cffdb4` (params-digest, stepped-snapshot, three cut-snapshots); the 22 size/offset/schema/prefix rows did not move |
| `data/discrepancies.json` | C82 evidence extension, four rival rows' observation, `discrepancies.ids` unmoved | ✓ VERIFIED | `discrepancies=110`; `data/expected/discrepancies.ids` untouched by `6cffdb4` and by `9b8c720` |
| `data/expected/important-gates.ids` | six `mutant` rows, one per stand-in | ✓ VERIFIED | `:702-707`; `gates=123 lines=378` |
| `data/expected/tests/stic.ids`, `data/expected/tests/runner-process.ids`, `data/expected/runner-scenarios.ids`, `data/expected/faults/stic.ids` | inventories | ✓ VERIFIED | `stic` 111 ids == result-file id set; `runner-process` 167 ids; the probe `skip-ok` in both; fault collateral only (`faults=238/238`) |
| `tools/disc-sweep.sh`, `data/scenarios/disc-sweep-probe.ivss`, `docs/owner-images.md` | the contained probe | ✓ VERIFIED | `sh -n`/`dash -n` clean; three containment cases green |
| `19-DECISION.md` | both enumerations before the measurement, then the matrix and the exit | ✓ VERIFIED | 253 lines; Exit B section byte-identical between `2118d71` and `HEAD`; no removed line |

### Key Link Verification

The `verify.key-links` verb reports `all_verified: false` for two plans because it only tests whether the target *path
string* appears in the source file — a proxy that cannot see a mechanism link. Each link was therefore checked by hand:

| From | To | Via | Status | Details |
| ---- | -- | --- | ------ | ------- |
| `tests/stic/stic_disc.c` | `core/src/machine/machine_bus.c` | the stamp composition | ✓ WIRED | `sd_measure` `:186-187` and `machine_bus.c:85-86` are the same expression (unit start + offset×ticks_per_cpu_cycle + access_strobe_ticks); a guest deriving its stamp from instruction cycles would disagree by both terms |
| `tests/stic/stic_disc.c` | `tests/stic/stic_profile.c` | stack copy, never a global edit | ✓ WIRED | `sd_cell` `:1034` `*out = *p`; precedent `stic_profile.c:131/136/162` (`q = *p;`, the `stic.profile.window-close` mechanism); no write to the profile index or a global in the module |
| `core/src/stic/stic_regs.c` | `data/discrepancies.json` (C84) | exact-address READ match, register-index match kept for mode selects | ✓ WIRED | executed proof (log below): READ `$4021`/`$8021`/`$C021` → `MODE_CS`, READ `$0040` → `NONE`, READ `$0020` → `NONE`, WRITE `$0020` → `ARM`; `stic.alias.read-4021-switches-mode` = `ok 12` |
| `data/expected/ivsn-literals.ids` | `tests/state/state_literal.c` | the params digest in the container header | ✓ WIRED | `state.literal.params-digest` = `ok 7`, state suite `executed=61 failed=0`; only digest-carrying literals moved, sizes did not |
| `data/expected/tests/stic.ids` | `data/expected/important-gates.ids` | `<suite>.judge.<slug>` resolution | ✓ WIRED | `important-gates.sh` builds its `known` set from `tests/*.ids` lines matching `^[a-z0-9-]+[.](judge\|mutant)[.][^ ]+$`; the six rows resolve and the step passes (`gates=123`) |
| `data/expected/important-gates.ids` | `.planning/REQUIREMENTS.md` | the derived accepted set | ✓ WIRED | `important-gates.sh:186-200` derives it from the traceability table's `Complete` ids; VID-07 `Complete` at `:91`, six rows present, step green |
| `.planning/phases/…/19-DECISION.md` | `data/discrepancies.json` | the four rows' appended observation | ✓ WIRED | C1/C6/C82/C94 each name `stic.disc.sweep-diagonal` in their 2026-11-27 sentence |
| `data/expected/ivsn-literals.ids` | `tests/state/state_literal.c` (Exit A limb) | a decided value would move all five again | — N/A | Exit B decided no value, so this link is not exercised; the record states the reason |

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
| -------- | ------------- | ------ | ------------------ | ------ |
| `sd_guests[]` → `sd_observe_guest` | `blocked_writes`, `windows_active`, the three-frame vector | `ivs_machine_stic_state` on a live `ivs_machine` after the guest's *measured* accesses are replayed through `ivs_machine_bus` | yes — the values move under the field they should and not under others (masks `0xAAAA`/`0xCCCC`/`0xF0F0`/`0xFF00`) | ✓ FLOWING |
| `sd_cell` → cell rows | the sixteen `ivs_stic_params` copies | `*out = *p` from `ivs_stic_params_get(SC_PROFILE)`, then per-bit perturbation | yes — `sd_cell` returns 0 for an unreachable field, so a dead field is a red case, not a silent no-op | ✓ FLOWING |
| probe table | `offset_cpu_cycles outcome displayed` | the runner's own report for each bounded run | yes — the negative half records `-` below the 14,934-cycle frame end and a word above it (case `ok 129` asserts six rows and no digest) | ✓ FLOWING |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
| -------- | ------- | ------ | ------ |
| `stic` suite, all Phase 19 ids | `IVS_TEST_OUT_DIR=<tmp> ctest --preset dev -R '^stic$'` | `# suite=stic executed=111 skipped=0 failed=0`; `ok 100`–`ok 111` | ✓ PASS |
| the diagonal, in one comparison | `./build/dev/tests/ivs_tests_stic` stdout | `# sweep-diagonal: G1 field 0 mask 0xAAAA default 1 alternate 0` … `G2 … 0xCCCC` … `G3 … 0xF0F0` … `G4 … 0xFF00 default 4 alternate 7` | ✓ PASS |
| the diagonal's own mutant | same | `# judge: the diagonal rejected the entangled guest at cell 2 (bits 0x2), observed 1` (`ok 111`) | ✓ PASS |
| `state` suite (the params-digest holder) | `ctest --preset dev -R '^state$'` | `ok 7 state.literal.params-digest`, `executed=61 skipped=0 failed=0` | ✓ PASS |
| `runner-process` suite, the probe's three cases | `ctest --preset dev -R '^runner-process$'` (23.2 s) | `ok 36 …disc-sweep-probe # SKIP …`, `ok 128 ci-refusal`, `ok 129 output-tree`, `ok 130 bound`; `executed=164 skipped=3 failed=0` | ✓ PASS |
| the mode-select limb, executed | scratch program linked against `build/dev/core/libivs_core.a` calling `ivs_stic_effect` on the shipped row | `READ 0021/4021/8021/C021 -> effect 3 (mode_cs) OK`; `READ 0020 -> 0 (none)`; `READ 0040 -> 0 (none)`; `WRITE 0020 -> 1 (arm)`; defaults `gating=1 latch=1 reg_window=2000/8000` | ✓ PASS |
| **each guard can fail** (mutation, restored afterwards) | `stic_disc.c` stand-ins replaced by their real guests; rebuilt; ran the binary | `not ok 107` / `108` / `109` / `101` each with `judge-vacuous: the guard passed on the stand-in`; `not ok 103` with `judge-vacuous: the guard accepted a pair read at 59736 and 59736` | ✓ PASS (mechanism falsifiable) |
| **the registered sweep can fail** (mutation, restored) | G1 re-registered by its entangled `displayed` word | `# sweep-diagonal: reason 1 at cell 2 (bits 0x2) guest G1 observed 1 expected 0` → `not ok 110` | ✓ PASS (two-field guest rejected) |
| restore check | `sha256sum` + rebuild + re-run | hash back to `f606a197…6273`, `git status --porcelain` empty, suite `executed=111 failed=0` | ✓ PASS |

Every mutation above was reverted before the gate re-run; the working tree is clean and the suite green at HEAD.

### Probe Execution

| Probe | Command | Result | Status |
| ----- | ------- | ------ | ------ |
| `tools/disc-sweep.sh` (the owner-local sweep itself) | not run — needs operator EXEC/GROM content (D11) | the scenario case is `ok 36 … # SKIP … (ended missing-fixture, exit 13, as expected)`; the gate prints one such line per preset | SKIPPED (declared) |
| its containment (the three cases that can run without content) | `ctest --preset dev -R '^runner-process$'` | `ok 128`, `ok 129`, `ok 130` | ✓ PASS |
| any other `scripts/*/tests/probe-*.sh` | `find scripts -path '*/tests/probe-*.sh'` | none — this project has no such convention | N/A |

The probe's absence is exactly what criterion 5 permits, and no truth above depends on its output.

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
| ----------- | ----------- | ----------- | ------ | -------- |
| VID-07 | 19-01, 19-02, 19-03 (all three declare `requirements: [VID-07]`) | the four rival display-path rules separated by four authored guests, diagonal across the 16-cell sweep, each with a liveness guard a stand-in can fail, then either the decided default pinned with the whole dependent chain in one commit **or** "default unchanged, observation appended, rivals named" | ✓ SATISFIED | Criterion 1: truths 1–5, 8. Criterion 2: truths 6, 9–11. Criterion 3 (Exit B branch): truths 12–13. Criterion 4: truths 7, 14–15. Criterion 5: truths 16–21. Requirement recorded `Complete` at `.planning/REQUIREMENTS.md:30` and `:91`, and the flip landed in the same commit as the six registry rows |

No requirement mapped to Phase 19 is missing from the plans' `requirements` field, and no requirement mapped to another
phase appears in these plans.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
| ---- | ---- | ------- | -------- | ------ |
| — | — | `TBD` / `FIXME` / `XXX` in any file this phase modified | — | none: the only `XXX` hits are `$XXXX` address placeholders and `mktemp …XXXXXX` templates (`IvsSticTable.cmake:23/136/139/484/643`, `runner-process-tap.sh:178/2097/2377`, `disc-sweep.sh:109`) |
| — | — | `TODO` / `HACK` / `PLACEHOLDER` / "not yet implemented" | — | none in the changed files |
| — | — | empty implementation (`return null/{}`/`[]`, `=> {}`) or hardcoded empty props | — | none in the changed core paths; the three branches are guarded, not stubbed |
| `tools/disc-sweep.sh` | 88-95 | a bounded probe whose sweep never runs in the gate | ℹ️ Info | by design and by criterion 5; its *containment* is what the gate judges, and it is judged by three cases |
| `tests/stic/stic_disc.c` | 138, 181 | `sd_hits.cycle[]` is written and never read (review IN-02) | ℹ️ Info | dead field, no behavioural effect; recorded open with a reason in `19-REVIEW.md`'s disposition |
| `cmake/IvsSticTable.cmake` | 378-383 | the gating select's `else()` is unreachable (review IN-03) | ℹ️ Info | dead branch that cannot run; recorded open with a reason |

No 🛑 Blocker and no ⚠️ Warning anti-pattern was found. `19-REVIEW.md` raised 0 Critical, 3 Warning and 7 Info findings;
the disposition below accounts for all ten.

## The gate, re-run on this tree

`GUARDRAILS-v0.4` §6 applied literally: `sh tools/gate.sh` then `dash tools/gate.sh`, **sequentially** (lesson 33), each
to its own log, then the two summary lines compared with `cmp`.

```
sh  tools/gate.sh  -> rc 0, 61 s   gate: PASS …
dash tools/gate.sh -> rc 0, 64 s   gate: PASS …
cmp <(sh summary line) <(dash summary line) -> identical
```

The whole line was re-run a second time after the mutation experiments above; its summary line is byte-identical to the
first pair as well.

Token-by-token against the phase's contract:

| token | required | measured | |
|---|---|---|---|
| `steps` | 38/38 | 38/38 | ✓ no new gate step |
| `controls` | 405/405 | 405/405 | ✓ |
| `faults` | 238/238 | 238/238 | ✓ no new fault site |
| `goldens` | 29 | 29 | ✓ no golden moved |
| `claims` | 29 | 29 | ✓ |
| `provisional` | 28 | 28 | ✓ |
| `prefix-files` | 11 | 11 | ✓ |
| `discrepancies` | 110 | 110 | ✓ no C id minted |
| `profile-keys` | 155 | 155 | ✓ |
| `records` | 155 | 155 | ✓ |
| `fields` | 155 | 155 | ✓ |
| `unknown-fields` | 48 | 48 | ✓ the field stays `unknown-default-chosen` |
| `gates` | 123 | 123 | ✓ one new gate name |
| `lines` | 378 | 378 | ✓ +6 registry rows |
| `tests-expected` | 5114 | 5114 | ✓ the twelve Phase 19 ids across two presets |
| `tests-executed` | 5108 | 5108 | ✓ |
| `tests-skipped` | 6 | 6 | ✓ (`tests-missing=0`) |

`grep -- '-- FAIL'` returns exactly two lines under each shell, both inside `runner-selftest proof` — the gate's own
negative controls (`control-pass-line-not-last: profile`, `runner-selftest-missing: st-no-such-case`), printed as the
proof that those selftests *can* fail. Zero `-- FAIL` lines from any step. `stic-rules=67`, `open=101`,
`pending-resolvers=20`, `d10-items=47`, `rights-pinned=4/4`, `binaries-listed=0` also unmoved.

## The five Success Criteria, judged one by one

**Criterion 1 — four guests, one machine-state observable each, a liveness guard a stand-in fails.** MET. Four guests
exist in `tests/stic/stic_disc.c` as decle-builder programs, each read through `ivs_machine_stic_state`, each with a
two-limb guard over both settings and a committed degenerate stand-in. I did not take the judges' green at face value:
each stand-in was swapped back into its real guest, the suite rebuilt, and every judge went red for the intended reason
(`judge-vacuous`), then restored. The one deviation — G1's registered observable is the blocked-write counter rather than
the criterion's parenthetical `frame 0 displayed` — is judged below.

**Criterion 2 — sixteen cells, diagonal, latch rule in the inventory before the sweep, a `mutant` over the check shown
able to fail.** MET. One id (`ok 110`) asserts `alternate iff own bit` over sixteen stack-copied cells; the masks are
exactly `0xAAAA`, `0xCCCC`, `0xF0F0`, `0xFF00`; `stic.judge.disc-diagonal-two-fields` (`ok 111`) rejects the entangled
guest at cell 2; and I confirmed the registered sweep itself rejects a two-field guest (`reason 1`, `SD_DIAG_FOREIGN`, at
cell 2) by re-registering G1 the entangled way. The latch rule is a `profile-fields.ids` row in `6cffdb4`, an ancestor of
the sweep's `2118d71`.

**Criterion 3 — exactly one exit recorded, with the enumeration honoured.** MET on **Exit B**. The record reads
"default unchanged, observation appended, rivals named"; the Exit B enumeration section is byte-identical between Task 1's
blob and HEAD, the diff removes no line, and `9b8c720`'s nine paths equal the enumeration in both directions. The four
rival rows were checked individually (not as a set): C1 names the window field and G1's counter, C6 names the sustain rule
and G4's vector, C82 names both the `$0020` READ row (G3) and the persist alternate (G4), C94 names the gating alternate
and G2's counter — each carries the dated observation, stays `open`, keeps its `unknown-default-chosen:` policy and its
resolver, and says neither setting is preferred. Exit A is not taken, so "both settings pinned and every dependent
expectation retargeted" is not owed by this phase; the Exit A enumeration exists only as the pre-committed alternative.

**Criterion 4 — the four rival rows carry the observation, the field stays `unknown-default-chosen`, no claim cites it,
`claims=29`/`provisional=28`, `goldens=29`, no operator-file identity.** MET. Measured above; the observation sentences
each end "this observation mints no claim", the claims registry contains no reference to the sweep or the probe, and the
gate's claim, golden, rights and binary tokens are unmoved.

**Criterion 5 — the owner-local sweep is contained and is nobody's exit criterion.** MET. Not a gate step
(`steps=38/38`), `refs/`-only with the flatwrap rule proved in three spellings plus a positive limb, bounded by a wall
clock and a run cap, `skip-ok` in both inventories with `missing-fixture`/exit 13, and no criterion, id or verify block
reads its output. Its absence is visible as a `-- SKIPPED` line per preset, never a pass.

## Recorded deviations, judged

1. **G1's observable re-derived (criterion 1's parenthetical).** The plan registers the blocked-write counter as G1's
   observable and keeps the criterion's `frame 0 displayed` as an asserted, unregistered control, on the authority of
   criterion 2's own sentence ("A guest that moves under two fields is re-derived"). **Judged a recorded, owned reading,
   not a criterion left unmet.** The criterion's substance — one machine-state observation per guest, measured, no pixel
   and no human judgement — holds; the parenthetical names a reading that criterion 2's own rule forbids registering,
   and the reading is still asserted in the same case with its coupling measured. The re-derivation was made before the
   sweep and recorded in `19-DECISION.md` and `19-01-SUMMARY.md`, not after a result.
2. **The exit-trigger reading appears only after the measurement (review IN-05).** `19-DECISION.md` interprets "exactly
   one field discriminates" as "exactly one field yields evidence that prefers one of its settings" inside the appended
   section. **Judged a record-ordering defect that leaves no criterion unmet**: the interpretation is not a fit — under
   either parse, §2.6 and D-54 give the same exit for a diagonal with no preference, and the enumerations (the part that
   could have been fitted) were committed before the run. It is recorded as IN-05 `open, append-only` with the remedy
   owed to the next plan carrying an exit choice. Not a gap.
3. **The §2.7 splits.** Six registry rows and ten case ids landed one task after their cases, because
   `important-gates.sh` derives its accepted set from `Complete` requirement ids and refuses a row naming a `Pending`
   one. **Judged a recorded, owned deviation with a structural cause** (the same one Phase 18's O4 recorded): each id
   shipped its guard and its stand-in in the same commit as the id; only the registry row moved, and it moved into the
   commit that flips the requirement, which is the earliest commit the checker accepts. §2.7 is a guardrail, not a phase
   criterion, so no Success Criterion is unmet.
4. **Lesson 34 — the unsatisfiable verify predicate.** Task 2's predicate (`… \(Phase 19 sweep[^.]*\.` plus "the matched
   text contains `stic.disc.sweep-diagonal`") cannot match, because `[^.]` cannot cross the dot in `stic.`. The executor
   ran an equivalent two-limb form and recorded the defect as lesson 34 rather than rewording four discrepancy notes.
   **Judged a predicate defect, not a record defect**: I re-measured the intent directly (one *new* dated sentence per
   row, the count of `Observation, ` up by exactly one in each, the sweep id inside it), so the criterion is met and the
   workaround did not change any note's content.

## The ten review findings, judged

`19-REVIEW.md` disposition, re-checked against the code — a "fixed" finding whose mechanism I cannot demonstrate would be
a gap. None is.

| Finding | Disposition claimed | Mechanism I demonstrated |
| ------- | ------------------- | ------------------------ |
| WR-01 the `ivs_stic_effect` declaration comment described the removed READ rule | fixed (`48c7ba4`) | `core/src/stic/stic.h:251-255` now states the two-limb READ match (mode select → register index, so `$4021` still selects Color Stack mode; every other READ → exact address, so `$0020` is reachable and `$0040` inert) and the WRITE/DW register-index match. Comment only: no count moved |
| WR-02 the bus-replay loop was copied five times | fixed (`812b838`) | `grep -c 'unit_start_tick =' tests/stic/stic_disc.c` → **1** (in `sd_replay_one`, `:216`); the access kind comes from the hit, never a literal; `sd_replay` is the filter; the five former copies call it |
| WR-03 `--max-runs` was inert above the refusal | fixed (`f24ad2c`) | `disc-sweep.sh:88-95` caps the walk and writes `cap` into the header; `disc-sweep.bound` asserts two rows, the first two offsets and the header `# disc-sweep: 2 of 6 offsets (run-count cap 2, bound 16)` — `ok 130` |
| IN-04 the second run-count check was always true | fixed as a side effect | `grep -n max_bound disc-sweep.sh` → one list-length assertion at `:89` next to the list; the constant comparison is gone |
| IN-06 `PROJECT.md`'s status line said "after Phase 18" | fixed (`12a9cdd`) | `PROJECT.md:459` reads `*Last updated: 2026-11-27 after Phase 19 — Discriminating the Display Path: Exit B, the diagonal matrix, and the display-path field still unknown-default-chosen.` |
| IN-07 `19-VALIDATION.md` described an unstarted phase | fixed (`fb94092`) | frontmatter `status: complete`, `nyquist_compliant: true`, `wave_0_complete: true`; seven task rows green against V1–V5; container gate a visible `not run` row |
| IN-01 two file headers omit the deciding parameter | open, out of scope | accepted: both branches carry accurate inline comments, `stic_params.h` names both members; cosmetic |
| IN-02 `sd_hits.cycle[]` unread | open, out of scope | confirmed dead (written `:181`, no reader); no behavioural effect |
| IN-03 the gating select's `else()` is unreachable | open, out of scope | confirmed unreachable: `st_policy` fails fatally for any value but `apply`; dead branch |
| IN-05 the exit trigger is read only post-measurement | open, append-only | judged above as a record-ordering defect with no criterion unmet |

## Tier ceiling

**None** (`GUARDRAILS-v0.4` §3, Phase 19: "No claim. The observation stays `unknown-default-chosen` even when a default
is decided"). Measured: `unknown-fields=48`, the latch row `tag: unknown-default-chosen` with `decision: null`, the four
rival rows `status: open`, `claims=29` and `provisional=28` unmoved, `claim-refs=27`. Nothing in this phase reaches
`measured-hardware`; the D10-VID-08 hardware probe remains a handoff row (`d10-items=47`, `handoffs=8`) and is not a
criterion.

## What was automated, thrown away, handed off, and what no tool can judge

**Automated (recurring, in the gate).** Twelve Phase 19 ids inside the existing `stic` suite (four guests, five judges,
the equivalence, the sweep and its judge), three `runner-process` containment cases for the probe, six `important-gates`
registry rows, the `skip-ok` rows in both inventories, and the `state.literal.params-digest` id that holds the params
digest. Every one of them runs on both presets in every gate run. The chain is one command: `sh tools/gate.sh` then
`dash tools/gate.sh`.

**Thrown away (correctly not in CI).** The three mutation experiments in this report (stand-ins restored to real guests;
G1 re-registered by its entangled reading; the equivalence stand-in collapsed to one instant) and the scratch
`ivs_stic_effect` proof program. Each answered "can this fail?" once; none is a distinct recurring failure, so none
belongs in the gate. All were reverted and the tree re-verified.

**Handed off.** One item: the container gate `sh tools/gate-linux.sh`. It is a local pre-push helper and not a gate step
(`steps=38/38`), and it is recorded as `not run` below — never as a pass.

**What no tool can judge.** The hardware question the phase exists *not* to answer: which of the two settings the real
STIC used. The sweep proves separability inside the engine's own table, and an engine agreeing with itself is not an
oracle (D-51). That judgement is the D10-VID-08 owner probe, already a handoff row with `evidence: none`; it is not a
criterion of this phase and is not re-asked here.

## Outstanding evidence (recorded as `not run`, never as a pass)

| Item | State | Measured reason | What would discharge it |
| ---- | ----- | --------------- | ----------------------- |
| `sh tools/gate-linux.sh` (the container gate, `GUARDRAILS-v0.4` §4) | **not run** | `timeout 20 docker info` → rc 124 (no answer) on this host; `build/gate-linux.log` exists but is 0 bytes, so no `gate: PASS` line exists to read. No `ivs-*` image, container or volume was created | one container gate on a host whose daemon answers, ending `gate: PASS` in `build/gate-linux.log` |

This is outstanding evidence, not a pass, and it weakens no id this phase added: every id above ran under the local gate
on this host, and the container gate is a pre-push helper rather than a gate step. Phase 18's verification recorded the
same item the same way.

## Gaps Summary

No gaps. The phase goal is achieved on Exit B's own stated terms: the four rival display-path rules are separated by four
authored guests whose registered observables are diagonal across sixteen cells, the diagonal is falsifiable (proved twice,
once by its committed stand-in and once by my own mutation of the registered sweep), the exit was decided by a measurement
taken after both enumerations were committed, the exit's file list equals the enumeration in both directions, the four
rival rows carry the observation with the rivals named, and the display-path fields stay `unknown-default-chosen` with
`claims`, `provisional`, `goldens`, `faults`, `steps` and `prefix-files` unmoved.

Two items are recorded and owned rather than closed: the §2.7 registry-row split (forced by the derived accepted set) and
IN-05's post-measurement statement of the exit-trigger reading (append-only record). Neither leaves a Success Criterion
unmet. Four review findings stay `open` as cosmetic or record-ordering items with recorded reasons.

---

_Verified: 2026-11-27T09:41:00Z_
_Verifier: Claude (gsd-verifier)_
