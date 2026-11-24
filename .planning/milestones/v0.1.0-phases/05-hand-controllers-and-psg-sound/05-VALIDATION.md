---
phase: "5"
slug: "hand-controllers-and-psg-sound"
status: draft
nyquist_compliant: true
wave_0_complete: false
created: "2026-11-12"
---

# Phase 5 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution. Decisions: `05-CONTEXT.md` D-01..D-28; code grounding, derived constants and the re-cut plan table: `05-RESEARCH.md`; option analysis: `05-DISCUSS-{ctrl,psg,audio,verify}.md`. Plan numbers below follow the RECOMMENDED 12-plan cut of `05-RESEARCH.md` (05-02 ABI only, 05-03 digest foundation, 05-04 register file, 05-05 input, 05-06 tone and sampling, 05-07 noise and envelope, 05-08 write log, 05-09 runner, 05-10 reference converter, 05-11 fixture, 05-12 close-out); if the planner keeps CONTEXT's ten plans, the rows keep their ids and only the plan column moves. Test, fault, control and token ids are proposals (final names belong to the plans) except where an id already exists in the repo.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | In-repo C harness `tests/harness/ivs_test.{h,c}` (TAP) under CTest, two NEW suites `input` (`tests/input/`) and `psg` (`tests/psg/`) with their own executables, ids files and fault files (V-15: a fault reruns only its own suite); existing suites `run`, `machine`, `memmap`, `runner`, `runner-process`, `headers`, `core-symbols`; pure steps `input` and `psg-vectors` (text formats, no `string(JSON)`), extended steps `listing`, `discrepancies`, `golden`, `api-effects`, `pkg-install`, `important-gates`; named faults (`tests/faults.cmake`, suites registered with `ivs_fault_suite`); negative controls (`tools/check/run-controls.sh` on `tools/check/controls/_good/tree`); in-suite judge self-controls (`input.judge.*`, `psg.judge.*`) counted as mutants by `important-gates.ids` |
| **Config file** | `CMakePresets.json` (`dev`: Debug -O1, ASan+UBSan, Werror, fault controls; `release`: -O2 -DNDEBUG, Werror), `tests/CMakeLists.txt` (suite and fault-suite registry), `data/expected/tests/<suite>.ids`, `data/expected/faults/<suite>.ids`, `data/expected/controls/*.ids`, `data/expected/gate-steps/*.ids`, `data/expected/{input-policies,psg-rules,psg-log-source,d10-measurements,important-gates}.ids` |
| **Quick run command** | `cmake --build --preset dev && build/dev/tests/ivs_tests_input --only input.<family>.` (likewise `ivs_tests_psg --only psg.<family>.`, `ivs_tests_run --only run.<family>.`, `ivs_tests_machine --only machine.digest.`, `ivs_tests_memmap`, `ivs_tests_runner --only runner.<family>.`); one suite: `ctest --preset dev -R '^(input|psg|run|machine|memmap|runner|runner-process)$'` |
| **Full suite command** | `sh tools/gate.sh` (the single local and CI command) and `dash tools/gate.sh` at every plan end; `sh tools/gate-linux.sh` (ubuntu 24.04, gcc 13, dash, cmake 3.28, container proxy for the hosted runner) before pushing plans 05-02, 05-03, 05-06, 05-08, 05-09, 05-10, 05-11, 05-12 |
| **Baseline** | `gate: PASS steps=33/33 controls=277/277 faults=149/149 tests-expected=2816 tests-executed=2814 tests-skipped=2 tests-missing=0 goldens=4 reports=12 host-dependent=3 gates=26 pkg-selftest=10/10 ci-runs=23` (04-VERIFICATION.md); `tests-expected` is ids x 2 presets (tests-inventory.sh:19) |
| **Projected end** | steps 35 (adds `input`, `psg-vectors`); controls about 299 (22 new per V-18, cap 25); faults about 175-179 (V-17 lists 26 and D-16 adds `AUDIO_STATE_DEPENDS_ON_BUFFER`; the ABI and digest plans may add 2-4 more, or use judge self-controls instead: planner decision at the first checkpoint); about 365 new ids, so `tests-expected` about 3,546; `goldens` 4 -> about 6; `gates` 26 -> about 31 (estimates, not measurements) |
| **Gate time budget** | Hosted budget since 04-25: median `gate_s` 75 s and median `cpu_s` 200 as a cost signal; revisit when the median of the last 5 `gate_s` exceeds 90 s, any run exceeds 120 s, or the median of the last 5 `cpu_s` exceeds 200. Last-5 `cpu_s` median today 175 (`.planning/ci-runs.tsv`: 175, 179, 225, 150, 136). V-16 projection +17..28 cpu-s, +4..8 s wall (192-203 cpu-s). Checkpoints (orchestrator, `tools/ci-record.sh`): after 05-04 (first PSG suite and faults: replace estimates by measurements; if above 0.4 cpu-s per fault drop the six lowest-value faults for judge self-controls), after 05-08, at phase end; lever order if a projection or last-5 median reaches 190 cpu-s: (1) batch the `cp1610-vectors` JSON re-parse, (2) fault reruns limited to the suites a fault names, (3) background `c-build` only if `cpu_s / (ncpu x gate_s)` shows idle cores; conditional plan 05-L1 |

---

## Sampling Rate

Nyquist reasoning: the quantity that can change between two feedback samples is "one task's diff"; every task therefore has an automated sample that fails for the intended reason, and no three consecutive tasks go without one.

- **After every task commit:** the quick command for the touched family (`--only <prefix>`, under 5 s), or the touched pure step through `sh tools/gate.sh` when a step or inventory changed, or `sh tools/check/run-controls.sh` (`controls=N/N` last line) when controls changed.
- **After every plan (waves are sequential; a plan is its own wave):** `sh tools/gate.sh` and `dash tools/gate.sh`, both ending `gate: PASS` with the plan's exact `steps`, `controls`, `faults`, `tests-expected` counts, and a recorded measured plain-run time per preset for any new suite (SUMMARY habit from Phase 4).
- **Before pushing plans 05-02, 05-03, 05-06, 05-08, 05-09, 05-10, 05-11, 05-12:** `sh tools/gate-linux.sh` green (hosted-only classes: gcc evaluation order, `-Wconversion`, PIE and `_GLOBAL_OFFSET_TABLE_`, GNU `nm` shapes, glibc names).
- **Golden-touching plans (05-03, 05-06, 05-07, 05-09, 05-11):** the last task re-runs `ivs-run --update-golden` for the affected goldens and records the CLASSIFIED diff in the SUMMARY (only `digest.instance`, `digest.composite` and, later, `output.audio.bytes`/`digest.audio` lines may change; `digest.cpu`, `digest.mem`, `digest.images` and every `step.*`, `slot.*`, `cpu.*` line byte-identical); `pkg-consumer.golden` follows; never hand-edited.
- **Oracle-first ordering:** every implementation SUMMARY names the oracle commit (05-01) that precedes the implementation commit; the verifier checks `git log` order.
- **Before `/gsd-verify-work`:** gate green under sh, dash and gate-linux with the final counts; hosted `gate_s` and `cpu_s` rows recorded by the orchestrator; D-45 rule evaluated.
- **Cost discipline:** judge self-controls and controls before faults; one in-process loop per matrix (the 62-control matrix is NOT 62 scenarios); fuzz schedules at most 100,000 ticks per seed and no noise period 1 in fuzz (every sample would be a level change and one SHA-256 block); runner-process variants (slice 1, slice 7) only for the two end-to-end scenarios; no `sleep` or wall-clock in any new test.
- **Max feedback latency:** 75 s hosted median `gate_s` (budget), local quick samples under 5 s.

---

## Per-Task Verification Map

"gate" = `sh tools/gate.sh` with its final line checked for `gate: PASS` and the listed tokens; "dash" = the same with `dash tools/gate.sh`; "linux" = `sh tools/gate-linux.sh`; "quick" = `cmake --build --preset dev && build/dev/tests/<suite exe> --only <prefix>`; "controls" = `out=$(sh tools/check/run-controls.sh) && test "$(printf '%s\n' "$out" | tail -1)" = "controls=N/N"`. Counts move by plan (estimates): steps 33 -> 35 (05-01); controls +17 (05-01), +2 (05-02), +3 (05-12), other plans 0-1; faults +1..3 (05-02), +1..3 (05-03), +3 (05-04), +7 (05-05), +6 (05-06), +6 (05-07), +6 (05-08), +2..3 (05-09).

| Task | Plan | Wave | Requirement | Test Type | Automated Command | Status |
|------|------|------|-------------|-----------|-------------------|--------|
| 05-01-T1 | 01 | 1 | all (D-23, D-28) | docs and sources: page reading result recorded, hash-verified fetch into `refs/`, `data/sources.json` entries, `docs/psg/reference-notes.md`, `docs/input/controller-notes.md`, C-id and CL-id allocation table | gate (`sources`, `discrepancies`, `hygiene`, `text`, `glossary`); `git status` shows no file under `refs/` | ⬜ pending |
| 05-01-T2 | 01 | 1 | INP-01, INP-02 | pure step `input` (literal grammar, pins -> bits = cleared mask = wiki value, tiling of 31 controls, tag and cite checks) with controls (about 6) | gate (`input-rows=31`, `steps=34/34`); controls | ⬜ pending |
| 05-01-T3 | 01 | 1 | AUD-01 | pure step `psg-vectors` (vector grammar, rule inventory coverage, cite and tier per line, unknown-default parameter needs a tag) with controls (about 7), `data/psg/rules.json`, `psg-rules.ids` | gate (`psg-rules=`, `steps=35/35`); controls | ⬜ pending |
| 05-01-T4 | 01 | 1 | AUD-01, AUD-02 | `d10-measurements.ids` + `discrepancies` D10 cross-check (2 controls); listing branch-target rule `listing-branch-target` (2 controls) with `-> ADDR` annotations added to the two existing listings and manifest hashes updated; `docs/time-base.md` PSG-step correction | gate (`listings=2`); controls (+17 total) | ⬜ pending |
| 05-01-T5 | 01 | 1 | API-03, INP-01 | hand derivation of `ivs-diag-ctrl-psg-1` written as a doc (not `data/golden/*.expect`), blind second-read requests recorded, discrepancy and profile-field plan | gate + dash | ⬜ pending |
| 05-02-T1 | 02 | 2 | API-03, AUD-02 | v1-size pins and rebase: `IVS_RUN_REQUEST_V1_SIZE` 64, `IVS_RUN_REPORT_V1_SIZE` 112, request v2 and report v2 fields, tail rule on the core's own size; `run.struct.*` extended (tracer) | quick `run.struct.`, `run.refuse.`; gate | ⬜ pending |
| 05-02-T2 | 02 | 2 | AUD-02 | ladder stage for `psg_log` through the env seam (NULL, overlap with instance, report, video, audio, events; capacity), reasons 55..57 pinned in `api-reasons.ids`, `api-effects.json` rows, D-17 NULL plus 0 "not requested" rule with the Phase 4 `run.buffers.*` cases updated | quick `run.buffers.`, `run.prec.`; gate (`api-rows=`, `pinned-defines=`) | ⬜ pending |
| 05-02-T3 | 02 | 2 | AUD-02 | `ivs_info` v3 fields (zero until 05-06), log-source define class pin (`psg-log-source.ids`) + 2 controls, `core-symbols` and `headers` unchanged | quick `run.info.`; controls; gate + dash; linux | ⬜ pending |
| 05-02-T4 | 02 | 2 | AUD-02 | 1-3 faults for the grown structs (for example report tail unguarded) | gate + dash (`faults=`); linux | ⬜ pending |
| 05-03-T1 | 03 | 3 | API-03, AUD-02 | FIRST: re-record the v1 composite (`md_fresh_composite`) and stepped literals (`md_stepped`) from the unmodified tree (repeatability labels) | quick `machine.digest.fresh-composite`, `machine.digest.stepped-literals` | ⬜ pending |
| 05-03-T2 | 03 | 3 | AUD-02 | instance layout (PSG state and register rows tagged INSTANCE, wider audio row, domain 19 rows, pad words), `IVS_MACHINE_BYTES`, composite v2, `ivs_digest_domain_known`, reset clears new rows, domain-19 refusal retargeted to 20 | quick `machine.digest.`, `machine.reset.`, `machine.refuse.digest.`; gate | ⬜ pending |
| 05-03-T3 | 03 | 3 | AUD-02 | hand-framed domain-4 literal stream extended, coverage and sensitivity per new row, composite-includes-PSG and domains-1..3-unchanged tests, 1-3 faults | quick `machine.digest.`; gate + dash (`faults=`) | ⬜ pending |
| 05-03-T4 | 03 | 3 | AUD-02 | golden re-bless #1 with classified diff (instance and composite lines only), `pkg-consumer.golden` | gate (`goldens=`, `pkg-selftest=10/10`); linux | ⬜ pending |
| 05-04-T1 | 04 | 4 | AUD-01 | `psg8` device kind, generator changes (`MM_DEVICES`, `MM_STUB_PHYS`, `MM_CAP_psg`), `IvsPsgTable.cmake` (regmap and masks) in `core-generated.ids` and `ivs_core_gen` | gate (`c-purity` generated-include proof); quick `memmap` | ⬜ pending |
| 05-04-T2 | 04 | 4 | AUD-01 | register file through the machine bus wrapper (no engine): writes stored masked, reads masked, R10 read-back default, power-on and reset | quick `psg.reg.`, `psg.reset.`; gate | ⬜ pending |
| 05-04-T3 | 04 | 4 | AUD-01 | memmap literal split by width, `psg` rule rows and descriptor row, `memmap.row.*` ids updated, CL-0006 artifact refreshed | `ctest -R '^memmap$'`; gate (`literal-rows=`) | ⬜ pending |
| 05-04-T4 | 04 | 4 | AUD-01 | suite `psg` created (ids, ctest, fault suite), 3 faults, `psg-vectors` cross-check with profile fields + controls; measurement checkpoint 1 | quick `psg.`; gate + dash (`faults=`) | ⬜ pending |
| 05-05-T1 | 05 | 5 | INP-01, INP-03 | `IvsCtrlTable.cmake` fail-closed generator, 34 profile fields (tier4, C16), `GEN_` mutation fault registration, `input` step cross-check extension | gate (`input-rows=`); controls | ⬜ pending |
| 05-05-T2 | 05 | 5 | INP-01, INP-02 | pull-down setter in apply (instance-wide), take-back restores pull-down, `psg8` read path, peek equals guest read; suite `input` created: `input.single.*`, `input.idle.*`, `input.anchor.*`, `input.struct.*` | quick `input.single.`, `input.anchor.`; gate | ⬜ pending |
| 05-05-T3 | 05 | 5 | INP-02 | `input.combo.*` (about 24), pairwise properties (930 pairs in 4 ids), `input.port.*` policy rows, `input-policies.ids` | quick `input.combo.`, `input.port.`; gate | ⬜ pending |
| 05-05-T4 | 05 | 5 | API-03 | flip `run.input.guest-invisible` to `run.input.guest-visible`, fuzz B and C, C67 boundary test, invariant `port_pulldown == derive(words)`, `INPUT_EXTRA_CONTROLLER_UNCHECKED` | quick `run.input.`, `input.boundary.`, `input.fuzz.`; gate | ⬜ pending |
| 05-05-T5 | 05 | 5 | INP-01..03, API-03 | 6-7 faults, goldens unchanged classification re-checked | gate + dash (`faults=`) | ⬜ pending |
| 05-06-T1 | 06 | 6 | AUD-01 | engine params struct and generator rows (clocks per step, sample period, period-zero fields, mixer and amplitude fields), tone and mixer and amplitude, `psg.tone.*`, `psg.mix.*`, `psg.amp.*` (tracer) | quick `psg.tone.`, `psg.mix.`, `psg.amp.`; gate | ⬜ pending |
| 05-06-T2 | 06 | 6 | AUD-02 | machine bus wrapper catch-up, eager seam catch-up, per-call sink and counting sink, sample record `level_A, level_B, level_C, 0`, `psg.sample.*`, `psg.slice.seam-equals-run` | quick `psg.sample.`, `psg.slice.`; gate | ⬜ pending |
| 05-06-T3 | 06 | 6 | AUD-02 | audio capacity with slack, `audio_first_index`, real audio layout in `ivs_info` (flips `run.buffers.real-layout-zero`, `run.info` audio asserts and the fault line `data/expected/faults/run.ids:54`), `psg.buffers.*`, domain 17 chain | quick `psg.buffers.`, `run.buffers.`, `run.info.`; gate | ⬜ pending |
| 05-06-T4 | 06 | 6 | AUD-02 | golden re-bless #2 (adds `output.audio.bytes` only when a buffer is requested; instance and composite lines), 6 faults, linux | gate + dash (`faults=`); linux | ⬜ pending |
| 05-07-T1 | 07 | 7 | AUD-01 | noise LFSR (profile object), period-zero fields, `psg.noise.*` incl. `psg.noise.lfsr-maximal-period` | quick `psg.noise.`; gate | ⬜ pending |
| 05-07-T2 | 07 | 7 | AUD-01 | envelope 16 shapes (`psg.env.shape-0` .. `-15`), restart on shape write, rate bounds, DAC table as data | quick `psg.env.`; gate | ⬜ pending |
| 05-07-T3 | 07 | 7 | AUD-01 | profile perturbation tests `psg.profile.<field>` (8), property guards, 6 faults, golden re-bless #3 | quick `psg.profile.`; gate + dash (`faults=`) | ⬜ pending |
| 05-08-T1 | 08 | 8 | AUD-02 | log record 16 bytes (byte-wise, pinned order), strictly increasing ticks, stamps from the wrapper, `psg.log.framing`, `psg.log.window` | quick `psg.log.`; gate | ⬜ pending |
| 05-08-T2 | 08 | 8 | AUD-02 | `W_log` and the property test of g = 36 and U (real bus and synthetic maximal waits), `psg.buffers.worst-case-writer`, third buffer wired, domain 19 chain | quick `psg.bounds.`, `psg.buffers.`; gate | ⬜ pending |
| 05-08-T3 | 08 | 8 | AUD-02 | replay equivalence, log versus CPU bus trace, effect boundary, slicing of log and samples, `psg.fuzz.seed01..08`, `psg.iso.*`, `psg.reset.*` | quick `psg.log.`, `psg.slice.`, `psg.fuzz.`; gate | ⬜ pending |
| 05-08-T4 | 08 | 8 | AUD-02 | 6 faults, measurement checkpoint 2 | gate + dash (`faults=`); linux | ⬜ pending |
| 05-09-T1 | 09 | 9 | AUD-02, API-03 | `main.c` allocates buffers from `ivs_info`, exec three-way split, accumulate and re-hash the audio chain (`audio.chain.check`), `ivsr 2` and new keys in report.c, tests/runner, runner-process, docs | quick `runner.exec.`; `ctest -R '^runner-process$'`; gate | ⬜ pending |
| 05-09-T2 | 09 | 9 | AUD-02 | golden key list in `main.c` and `golden.sh`, `# domains:` grammar for `psglog`, toy reports in `_good/tree`, golden controls | controls; gate (`goldens=`) | ⬜ pending |
| 05-09-T3 | 09 | 9 | AUD-02 | IVSS `capture psglog|audio`, `assert psglog ...`, `assert audio ...`, `assert digest audio=|psglog=`, parser table and fuzz rows, scenario `psg-log-assert-mismatch` (guest-fail 10), options `--audio-raw`, `--psg-log-out`, `--audio-out-max-bytes` | quick `runner.parse.`, `runner.exec.psglog-assert`; gate + dash | ⬜ pending |
| 05-09-T4 | 09 | 9 | AUD-02 | 2-3 faults (`RUNNER_PSGLOG_ASSERT_IGNORED`, `RUNNER_MASK_IGNORED`, audio check), golden re-bless #4 (key list only), linux | gate + dash (`faults=`); linux | ⬜ pending |
| 05-10-T1 | 10 | 10 | AUD-02 (D7) | `ivs_refpcm` pure integer converter, no I/O, no allocation, no globals; tests in suite `runner`; location decision applied (tools/pcm extension of four guards, or `runner/refpcm`) | quick `runner.refpcm.`; gate | ⬜ pending |
| 05-10-T2 | 10 | 10 | AUD-02 (D7) | `--audio-wav` and `--wav-rate` wiring (exclusive create in `hostio.c` only), `refpcm.*` report keys only with the option, `.refpcm` golden labelled repeatability, pitch within 1 percent of `crystal/(32*TP)` | quick `runner.refpcm.pitch`; `ctest -R '^runner-process$'` | ⬜ pending |
| 05-10-T3 | 10 | 10 | AUD-02 (D7) | proofs that no PCM, resample, wav or DAC-table symbol is in `ivs_core`: exact install inventory (`pkg-install-files.ids`) and public-symbol rule; deny-list labels extended | gate (`prefix-files=`, `public-symbols=`); linux | ⬜ pending |
| 05-11-T1 | 11 | 11 | INP-01, API-03, AUD-01, AUD-02 | `ivs-diag-ctrl-psg-1` listing (about 100 words, no SDBD), manifest (`fixtures.ids`, `listings.ids`), `.expect` with the derivation from 05-01, scenarios `diag-ctrl-psg-pass`, `-nokey`, `-order` | `ctest -R '^runner-process$'`; gate (`listings=3`, `goldens=`) | ⬜ pending |
| 05-11-T2 | 11 | 11 | API-03, AUD-02 | in-process `run.diag.ctrl-psg.*`, five program mutants (`diag-mutations.ids`), runner slices 1 and 7 variants | quick `run.diag.ctrl-psg.`; `ctest -R '^runner-process$'` | ⬜ pending |
| 05-11-T3 | 11 | 11 | API-03, AUD-02 | installed consumer replays `diag-ctrl-psg-pass` (events, audio and log buffers through the installed headers), `pkg-consumer.golden`, digests equal the runner golden | gate (`consumer-lines=`, `pkg-selftest=`); linux | ⬜ pending |
| 05-11-T4 | 11 | 11 | all | claims CL-0014 and CL-0015, discrepancy and claims records | gate (`claims=`) | ⬜ pending |
| 05-12-T1 | 12 | 12 | all | claims CL-0009..CL-0013, discrepancy status updates, D10 inventory finalised | gate (`claims=`, `discrepancies`) | ⬜ pending |
| 05-12-T2 | 12 | 12 | all | `important-gates.ids` lines for every new gate, 3 controls, REQUIREMENTS and ROADMAP traceability | gate (`gates=`, `lines=`); controls | ⬜ pending |
| 05-12-T3 | 12 | 12 | all | D-45 evaluation, CI-EFFICIENCY entry, VERIFICATION three tables (automated, handed off, accepted caveats), final counts | gate + dash; linux | ⬜ pending |

---

## Requirement-to-Check Map

| Req | Behaviour | Tests (exact ids to create unless marked existing) | Steps and scenarios | Independent oracle | Tier recorded |
|-----|-----------|-----------------------------------------------------|---------------------|--------------------|---------------|
| INP-01 | each of 31 controls on both ports reads its documented byte; idle `$FF` | `input.single.p0.<control>` and `input.single.p1.<control>` (62, both read paths: guest read and `ivs_machine_peek`), `input.idle.p0`, `input.idle.p1`, `input.anchor.wiki-idle`, `input.anchor.wiki-key1` (`$7E`), `input.anchor.wiki-key1-9` (`$5A`), `input.struct.*` (6: code widths, decodability properties), `input.table.matches-literal`, `run.observe.psg-ports` (peek is side-effect free for the ports) | step `input` (literal grammar, pins-to-bits, tiling, tags); runner-process `diag-ctrl-psg-pass` (8-control subset through the guest) | wiki anchors; pin-table arithmetic in the literal file; BSR code widths as properties; second read of the pin page | `source-expectation` (tier4 LOW), `provisional`; guest subset `original-diagnostic` |
| INP-02 | combinations produce the union of lines; policy rows labelled | `input.combo.<row>` (about 24: key+key, disc+side, disc adjacent, disc opposite, side+side, three sides, key+disc, key+side, all closed, port independence), `input.pairs.union`, `input.pairs.commutative`, `input.pairs.monotone-clear`, `input.pairs.decodable-claims` (930 pairs per port inside), `input.port.direction-output-reads-latch`, `input.port.upper-byte`, `input.port.unplugged-is-idle` | `data/expected/input-policies.ids` (result classes and tags) | wiki anchor `$5A`; wiki decodability statements; algebraic properties | `source-expectation` for anchors and decodability; policy rows carry no tier claim |
| INP-03 | core input is machine-level, no host mapping or discovery | existing: `core-symbols` ctest suite, step `c-purity`, `headers`; new: `input.profile.fields-are-lines-only` | steps `c-purity`, `api-effects` (`IVS_CTRL_*` pin), `profile` | pinned bit table (`input-bits.ids`), symbol allowlist | `deterministic-repeatability` plus structural |
| API-03 | input applied at an explicit machine-time boundary; same script, same results whatever the slicing | `input.slice.guest-visible`, `input.boundary.start-minus-1`, `input.boundary.start`, `input.boundary.start-plus-1` (observed unit starts, C67), `input.fuzz.seed01`..`seed08`, `input.fuzz.cases-covered`, `input.iso.interleaved-instances`, `input.reset.clears-pulldown`, `input.invariant.pulldown-derives-from-words`, `run.input.guest-visible` (renamed from existing `run.input.guest-invisible`), existing `run.input.*`, `run.slice.*`, `run.fuzz.*` stay green on content A | runner-process `diag-ctrl-psg-pass` at default slice, `--slice-ticks 1`, `--slice-ticks 7`; installed consumer replay | slicing invariance; reference `run(now + 1)` per unit; schedule-computable oracle for fuzz content B | `deterministic-repeatability`, `installed-host-behavior`; C67 is a contract, not a hardware claim |
| AUD-01 | register file and tone, noise, envelope, mixer, amplitude follow the documented rules; unknowns are tagged profile data | `psg.reg.*` (28), `psg.tone.*` (16, `psg.tone.rate-*`), `psg.noise.*` (12, `psg.noise.lfsr-maximal-period`, `psg.noise.rate-*`), `psg.env.*` (22, `psg.env.shape-0`..`-15`, `psg.env.rate-*`), `psg.mix.*` (10), `psg.amp.*` (10), `psg.port.*` (10), `psg.profile.<field>` (8 perturbation tests: noise taps, period-zero fields, DAC levels, mirrors, clocks per step, mixer bits, envelope select, reset), `psg.judge.*`, `memmap.dump.matches-literal` and `memmap.row.*` (existing family, rows renamed) | step `psg-vectors` (every rule has a vector, a cite and a tier; orphan and missing rule tokens); `data/psg/vectors/*.txt` | AY-8910 page equations by hand (rate and ordering only, tier from plan 05-01's page reading); closed-form rate bounds; LFSR period and DAC monotonicity as property guards | `source-expectation`; behaviour phase and unknown parameters `unknown-default-chosen`; claims `provisional` |
| AUD-02 | cycle-stamped write log and integer chip-rate samples, independent of PCM | `psg.sample.*` (10: count formula, grid stamp `k * 16`, byte order, record layout), `psg.log.framing` (hand-framed bytes + `cmake -E sha256sum`), `psg.log.replay-equals-run`, `psg.log.vs-cpu-trace`, `psg.log.effect-boundary`, `psg.log.window`, `psg.log.strictly-increasing`, `psg.slice.*` (8), `psg.fuzz.seed01`..`seed08`, `psg.buffers.*` (6: too small, exact fit, NULL, overlap, worst-case writer, sentinel tail), `psg.bounds.g-and-u` (property test), `psg.reset.*`, `psg.iso.*`, `machine.digest.composite-v2-framing`, `machine.digest.psg-rows-in-instance-domain`, `machine.digest.domains-1-3-unchanged`, `run.buffers.audio-real` | runner `runner.exec.audio-chain-check`, `runner.exec.psglog-assert`; scenario `psg-log-assert-mismatch`; `diag-ctrl-psg-pass`; `pkg-install` consumer | replay equivalence; bus-trace cross-check; hand-derived write list of the project listing; `cmake -E sha256sum` framing | `deterministic-repeatability`; write list `original-diagnostic` |
| AUD-02 (D7) | PCM and resampling only outside the core, outside the evidence path | `runner.refpcm.*` (exact integer rational filter, DC block, rate 44,100 and 48,000, `runner.refpcm.pitch-within-1pct`) | `pkg-install` exact inventory and public-symbol rule; no `refpcm`, `resample`, `wav`, `pcm` symbol in `libivs_core.a`; `important-gates` | self-contained integer arithmetic; datasheet pitch formula | `deterministic-repeatability`, labelled "reference conversion v1, converter repeatability only" |
| API-03, AUD-02 | structs grow append-only; old callers keep working | `run.struct.v1-sizes`, `run.struct.v1-sizes-lp64` (existing, updated), `run.struct.request-v1-caller`, `run.struct.report-v1-caller`, `run.info.v1-size-caller` (existing), `run.refuse.request-tail-nonzero` (existing family) | installed consumer prints the v1 sizes from the installed headers | pinned numeric sizes | structural |

### Success criteria to checks (zero human UAT)

| SC | Claim | Automated evidence (all in `sh tools/gate.sh`) |
|----|-------|-----------------------------------------------|
| SC1 | Each of 12 keys, 16 disc positions, side-action inputs, each player, individually reads its documented value with a tier tag per value | `input.single.*` (62) and `input.idle.*` with both read paths, `input.anchor.*`, step `input` (rows tagged, cited, tiling), `diag-ctrl-psg-pass` subset |
| SC2 | Combinations, opposite directions and ambiguous electrical combinations produce a documented tested result; nothing host-specific in the core | `input.combo.*`, `input.pairs.*`, `input-policies.ids`, `core-symbols`, `c-purity`, `api-effects` |
| SC3 | The same input script gives the same guest-visible results and digests in one call or many slices | `input.slice.guest-visible`, `input.boundary.*`, `input.fuzz.seed*`, `run.slice.*`, runner-process slice variants, `golden` step dev == release, consumer replay equal to the runner golden |
| SC4 | Each PSG register, tone, noise and envelope rule has a passing vector; behaviours without a primary source are tier-tagged profile data | `psg.*` families, `psg.profile.*` perturbation tests, step `psg-vectors`, `psg-rules.ids`, profile checker (`unknown-fields`) |
| SC5 | The core emits a cycle-stamped write log and integer chip-rate samples; any PCM sits where D7 puts it and outside the raw evidence path | `psg.log.*`, `psg.sample.*`, `psg.slice.*`, `runner.exec.audio-chain-check`, `runner.refpcm.*`, install inventory and symbol proofs, goldens carry no refpcm line except the `.refpcm` repeatability golden |

---

## Wave 0 Requirements

Files and facts that must exist BEFORE the implementation plan that consumes them (each is created by the plan named; plan 05-01 creates the oracle set and contains no C):

- [ ] 05-01: one allocation table for discrepancy ids (C69..C77 and the verification note's extras) and claim ids CL-0009..CL-0015, written before any profile field cites them
- [ ] 05-01: `data/input/intv-2609-ntsc-family-p1.literal.txt`, `data/psg/rules.json`, `data/psg/vectors/*.txt`, `data/expected/psg-rules.ids`, steps `input` and `psg-vectors` with controls and toy files in `tools/check/controls/_good/tree/`, `data/expected/d10-measurements.ids`, listing branch-target rule, `docs/time-base.md` PSG-step correction, derivation doc for the fixture
- [ ] 05-01: record whether the AY-8910 pages (printed 5-23..5-29, pdf about 204-210) print the equations, shapes and amplitude behaviour BEFORE 05-06 starts; if not, SC4 vectors fall back to `unknown-default-chosen` defaults with structural and rate properties only and C12 stays open
- [ ] 05-03 T1: the v1 composite and stepped literals re-recorded from the unmodified tree before the digest table changes
- [ ] 05-04: `memmap` literal split and probe rules land in the same plan as the `psg8` device (never earlier)
- [ ] 05-05: `ivs_ctrl_lines` generator and its entry in `core-generated.ids` before any test reads the table
- [ ] 05-09: golden key list changed in `runner/main.c` and `tools/check/steps/golden.sh` together, with the toy reports in `_good/tree` and the `ivsr 2` header in `tests/runner/runner_exec.c` and `tests/runner-process-tap.sh`
- [ ] Each new suite (`input` 05-05, `psg` 05-04) lands with its ids file, ctest registration, `ivs_fault_suite` entry and TAP in both presets in the same plan
- [ ] Each new step (`input`, `psg-vectors` 05-01) lands with its gate-step inventory line, controls inventory lines and toy fixtures; every distinct `CHECK` step must pass on the pristine toy tree
- [ ] Framework install: none (cmake, ninja, cc, dash, docker for the container gate, pdftotext for plan 05-01; no assembler, no new package)

---

## Manual-Only Verifications

All phase behaviours have automated verification; nothing blocks the phase goal. Recorded as missing evidence and handed off (counted from `data/expected/d10-measurements.ids` so the list cannot shrink silently):

| Item | Why no tool can judge it | Discrepancy | Owner action (non-blocking) | Claim stays |
|------|--------------------------|-------------|------------------------------|-------------|
| Subjective sound of the operator WAV (D-22) | listening is subjective | C12, C23 | listen to one local WAV (sha256 recorded by the helper), note pass or concern | provisional |
| Controller bytes for all 31 controls on both ports, which connector is port 0 | physical measurement (D10) | C16, C63, C64, C65, C66 | procedure in `docs/input/d10-procedure.md` (a reader diagnostic is a Phase 7 deliverable) | provisional |
| PSG tone and noise capture, envelope shapes, DAC levels, period 0, register read-back, power-on state, clock ratio, address map probe | physical measurement (D10) | C11, C12, C69..C76 | same procedure document | provisional |
| Controller and audio behaviour of a real EXEC | owner-held rights-bearing image (D11) | C20, C32 | none in Phase 5 (SCN-02 is Phase 7) | provisional |
| A tier-1 AY-3-8914 data sheet, if the owner holds one | no source found | C11 | scan into the local cache only | provisional |

Hosted gate times are read by the orchestrator (`tools/ci-record.sh`); agents never call `gh`. Branch protection stays a recorded recommendation (AUTONOMY.md) and blocks nothing.

---

## Validation Sign-Off

- [ ] All tasks have an `<automated>` verify or a Wave 0 dependency
- [ ] Sampling continuity: no 3 consecutive tasks without an automated verify
- [ ] Wave 0 covers all MISSING references
- [x] No watch-mode flags
- [ ] Feedback latency within the hosted budget (median `gate_s` 75 s, median `cpu_s` 200; checkpoints after 05-04 and 05-08, evaluated in 05-12)
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
