---
phase: 07-first-visible-original-scenario
recorded: 2026-11-13
recorded_by: executor of plan 07-12 (not an independent verifier run)
status: passed
requirements: [SCN-01, SCN-02]
---

# Phase 7: First Visible Original Scenario Verification Record

**Phase goal:** An original, rights-clear program visibly responds to controller input with video and sound, run in CI through the installed consumer. The result is stated as playable guest progress, for this original program only, and nothing stronger.
**Recorded:** 2026-11-13, by the executor of plan 07-12 from the gate output of the final tree, the inventories and `git log`. It is the first version of the record; a verifier run may replace its figures with its own, as in `06-VERIFICATION.md`. Every command named here was run by the executor, and the executor is the same model family that wrote the derivation, the program and the core.
**Standing of the evidence:** the two Phase 7 claims, CL-0023 (tier `playable-guest-progress`) and CL-0024 (tier `installed-host-behavior`), are `provisional` by the weakest-field rule (the controller byte table, the STIC rules and the PSG rules are tier 4 or inference fields), and no claim uses the `measured-hardware` tier (`data/claims.json`: 24 claims, 23 provisional, the one established claim is the Phase 1 CL-0002). `status: passed` means the automated evidence exists, was run on the final tree and holds, with the handed-off items and the accepted caveats below; it does not mean that any console behaves as modelled, that a person looked at the picture or listened to the sound, or that any real EXEC or GROM image was run.
**Human gate:** none blocks the phase goal. The goal and the four success criteria ask for documented, tested, tier-tagged behaviour of an original program, not for measured hardware or an owner-local EXEC run; the owner items in table 2 are non-blocking missing evidence (D10, D11), as in the Phase 4 to 6 records.

## Gate and suite results (final tree of plan 07-12)

| Command | Result |
|---------|--------|
| `sh tools/gate.sh` | `gate: PASS steps=36/36 controls=357/357 faults=222/222 claims=24 provisional=23 tests-expected=4624 tests-executed=4620 tests-skipped=4 tests-missing=0`, `gates=77 lines=264`, `goldens=15`, `manifests=10 effective-yes=7 effective-no=3`, `listings=6 listing-words=520`, `consumer-lines=135 pkg-selftest=17/17`, `video-frames=16 video-hash-ties=11 video-frame-pairs=7`, `tracked=2252`, 0 `FAIL` lines, four `-- SKIPPED` lines; `gate-time total=287` s on a laptop under load with the container gate running at the same time |
| `dash tools/gate.sh` | the identical summary line (md5 of the line with the time fields removed equal for both shells), 0 `FAIL` lines, `gate-time total=386` s under the same load |
| `sh tools/gate-linux.sh` (container, Ubuntu 24.04, gcc 13.3.0, cmake 3.28.3, emulated amd64) | `gate: PASS steps=36/36 controls=357/357 faults=222/222 claims=24 provisional=23 gates=77 lines=264 tests-expected=4624 tests-executed=4620 tests-skipped=4 tests-missing=0`, 0 `FAIL` lines, four `-- SKIPPED` lines, `gate-time total=674` s (emulated amd64) |
| the scenario `demo-lamp-tone-pass` through `ivs-run` (dev and release, inside the gate) | outcome pass, 55 of 55 asserts, 6 input events consumed of 6 |

## Goal achievement

The goal is met in the sense the gate can test. A project-authored program ("Lamp Tone", 198 words, assembled by the project's own mini-assembler from a tracked source and committed with its generated listing) is entered through the project boot stub contract 2, shows each controller port as a row of eight lamps and gives each port one PSG voice. Under six scripted controller events, the guest memory words, the delivered raw index frames, the PSG write log and the raw sample levels all equal values that were derived by hand before the program existed; the same cartridge with no input stays at the idle picture with an init-only write log and silence. An independently built consumer replays the same run through the installed headers and archive and reproduces the runner's digests, the lamp pictures, the region differences, the write-log records and the audio levels. The owner-local route for user-supplied EXEC and GROM images is documented with identity layers, labelled non-redistributable, kept out of CI and visible as a counted SKIPPED line, and an owner-run helper refuses every unsafe case under test.

What the goal does not include, and the record does not claim: that any console, real EXEC or GROM image, or commercial program behaves this way (D11 is open and not performed); that any timing figure (input latency, frame pacing, tone pitch or phase) is right; that a person saw the frames or heard the sound; or that the controller byte table, the STIC rules and the PSG rules match hardware (D10). Moving objects, collisions and delay registers are Phase 8.

## 1. Automated

### Success criteria of ROADMAP Phase 7

| # | Criterion | Evidence (V-rows of table 1b, ids and steps) | Result |
|---|-----------|-----------------------------------------------|--------|
| SC1 | The program's source, recipe, digests and licence are in the fixture manifest as `redistributable: yes`, and CI builds and runs it through the installed consumer | V1 (`fixtures/FX-demo-lamp-tone-1/manifest.json`, `redistributable: yes`, recipe sources and tool digests hashed by the `manifests` step, control `manifests-recipe-source-digest`), V11 (`pkg-install`: the fourth replay, prefix `scn.`, equal to `data/expected/pkg-consumer.golden` and to the runner golden) | pass |
| SC2 | A scripted input sequence changes guest state, and the run asserts a guest-state progress predicate, not a title screen or hash alone; the tier is playable guest progress, for this original program only | V4, V5, V9, V10, V14 (`demo-lamp-tone-pass`: `mem 02F0..02F5` at four steps with `halted == no`; the shape lint of the `golden` step; CL-0023) | pass |
| SC3 | The same run shows the input changing raw video (the index framebuffer differs in the expected region) and raw audio (the PSG write log and samples change) | V6, V7, V8 (frame hashes tied to F-S1 to F-S6, seven pair-region counts, `psglog` asserts, `audio at` levels, the pass and idle pair judge, the consumer's region diff) | pass |
| SC4 | The owner-local route for user-supplied EXEC and GROM is documented with digest identity checks, labelled non-redistributable and kept out of CI; any job that refers to it prints a visible SKIPPED line, and the inventory gate counts it | V15, V16, V17, V18, V19 (`docs/owner-images.md`, the two `redistributable: no` manifests, `owner-exec-title` as a counted `skip-ok` scenario, `tests-skipped=4`, the `-- SKIPPED` lines, the consistency rule, `tools/owner-run.sh` with its tests, the runner refusal of six derived outputs) | pass, documented route only (D11 not performed) |

### 1b. Automated evidence per V-row

The failing direction is what the check does when the thing it guards breaks. Commits are the commits that landed the check; every commit subject has the form `<type>(07-NN): ...`.

| Row | Requirement or decision | Check, id or step | Fails when | Landed in |
|-----|-------------------------|-------------------|------------|-----------|
| V1 | SCN-01, D-28 | `manifests` step with recipe-source and tool digests (control `manifests-recipe-source-digest`), `files`, `listing`, `pack.words` and trailer checks; manifests `FX-boot-stub-2` and `FX-demo-lamp-tone-1` (`effective-yes=7`) | a listing, `.asm`, `asm.c` or `pack.c` is edited without refreshing the digests; a row, branch or word mismatch | `7e16b2d` (07-05), `949407e` (07-06) |
| V2 | D-16 to D-19 | suite `asm` (47 ids): 304 hand-derived pairs (`asm.pairs.*`), four twins reproducing the 300 legacy words (`asm.twin-*`), `asm.regen-boot-stub-2` and `asm.regen-ivs-demo-lamp-tone-1`, 20 pinned refusals (`asm.neg.*` with `data/expected/asm-tokens.ids`), judges `asm.judge.mutated-twin` and `asm.judge.mutated-pair`; the `DATA` row rule with controls `listing-data-in-operand-slot` and `listing-data-after-sdbd` | a wrong opcode, field or displacement; a refusal that changes; an escape found only by a run (none occurred) | `be7aaa9`, `5d317f6` (07-03), `e4f7dab`, `969c516`, `55bb7bd`, `0687583` (07-04) |
| V3 | D-09, D-10 | `runner.parse.accept.boot-stub-2` and siblings, `runner.exec.boot-stub-2`, `run.stub2.path`, `run.stub2.handoff`, `run.stub2.jd-as-j`; mutants `SCN-STUB-JD-AS-J` and `SCN-ISR-TIE` judged by `frame 3 displayed == yes` | the stub vector loses its `JD` (the vector re-enters, no frame is displayed, though the counter still reads right) or its target moves | `7a26dcf`, `4a97001` (07-05), `e2c8b3b`, `647365a` (07-08) |
| V4 | D-04, D-05, D-14 | `runner-process.scenario.demo-lamp-tone-pass`: 55 asserts, `mem` at four `run-to` steps together with `halted == no`; `data/golden/demo-lamp-tone-pass.expect` | a wrong store address, a stuck counter, an input that is not decoded (mutants `SCN-INPUT-PORT`, `SCN-PROGRESS-STORE`, `SCN-NO-EIS`) | `8cb4329` (07-07), `e2c8b3b`, `647365a` (07-08) |
| V5 | D-12, D-13 | `frame k displayed == yes` per frame and the counter `floor(T / 59,736)`; mutants `SCN-REARM-SHORT`, `SCN-REARM-LONG`, `SCN-STROBE-LATE`, `SCN-NO-EIS`, `SCN-HLT-IDLE`; the negative row `SCN-REARM-NEGATIVE` must pass | two interrupt entries per frame, a blocked strobe, a halted idle; a tuned assert would fail the negative row | `8cb4329` (07-07), `e2c8b3b`, `647365a` (07-08) |
| V6 | D-21, D-23 | two literals per pair and the `video-frame-pair-region` token (control `video-frame-pair-region-drift`, `video-frame-pairs=7`), six frame hashes tied by `video-hash-ties=11`, pixel probes inside and outside the lamp rectangles; consumer `scn_check_frames` | one extra differing pixel, a hash drift, a video-only mutant (`SCN-LAMP-COMPLEMENT`, `SCN-STROBE-LATE`) | `8ded8dd` (07-02), `8cb4329` (07-07), `5ee7d13` (07-09) |
| V7 | D-22 | `psglog` asserts by address, value and source (never by tick), `audio at` only where the level is exact, idle `output.audio.changes == 0`; mutants `SCN-AMP-VALUE` (pass) and `SCN-SOUND-UNGATED` (judged under the idle scenario) | a wrong amplitude or period, a gate that is written in every routine | `d67b180` (07-02), `8cb4329`, `2188b4a` (07-07), `647365a` (07-08) |
| V8 | D-23 | `runner-process.pair.demo-lamp-tone-pass.demo-lamp-tone-idle` (video, audio, psglog, input and mem digests differ) and `.demo-lamp-tone-shift` (video digest equal); self-controls `runner-process.judge.pair-video-equal`, `.input-not-consumed`, `.idle-audible` | a program or core that ignores the input; events after the last bound | `d533c1f` (07-07) |
| V9 | D-24 | shape lint of the `golden` step for the tier `playable-guest-progress`, controls `golden-shape-no-input` and `golden-shape-no-progress` | no input line, no memory assert after the first input, no frame or write-log assert | `c38d85e` (07-02) |
| V10 | D-25 | `data/expected/diag-mutations-scn.ids`: 12 mutants and one negative row, `runner-process.mutant.SCN-*`, the `fails=` column judging the full set of failing asserts, stub-image patches by `ivs_pack --exec --patch` | a mutant that is not distinct from another, a channel that breaks another channel's asserts | `e2c8b3b`, `647365a` (07-08) |
| V11 | D-26, PKG-01 | `pkg-install`: the fourth replay (`scn.`) built against the moved prefix, equal to `data/expected/pkg-consumer.golden` and to `data/golden/demo-lamp-tone-pass.digests`; self-tests `scn-digest-drift` and `scn-domain-missing` (`pkg-selftest=17/17`) | a digest or observed-domain drift between the installed package and the runner | `da07663`, `5ee7d13` (07-09) |
| V12 | D-06 | `runner-process.repeat`, `.slices` (1 and 7), `.cwd` for `demo-lamp-tone-pass`; dev preset equal to release preset | a host-dependent byte | `d533c1f` (07-07) |
| V13 | D-29, A-13 | `important-gates` accepts SCN-01 (20 registry lines) and SCN-02 (3 lines), with the toy lines of A-21; `files-trademark-name` | the console brand word in a fixture, scenario or target name | `c38d85e` (07-02), `1edbdcd` (07-06), `adba1ea` (07-10) |
| V14 | D-27 | `claims` (CL-0023 and CL-0024: artifact exists, weakest-field rule forces `provisional`) and `text` with `plays` and `playable` in the claim words (control `text-untagged-playable`, the tier identifier exempted) | an untagged play word in README or `docs/`, an established standing on a weak field, a missing artifact | plan 07-12 (the commit `feat(07-12): ...` that adds the two claims and the tripwire) |
| V15 | SCN-02, D-30 | `manifests` (`FX-owner-exec` and `FX-owner-grom`, kind `system-image`, `redistributable: no`, `effective-no=3`), `files` (no image tracked), the anchors of `docs/owner-images.md` | an owner manifest that says yes; a tracked image | `bdf0e50` (07-10) |
| V16 | D-31 | `owner-exec-title` as a `skip-ok` scenario (exit 13, `missing-fixture`) in both inventories, `-- SKIPPED` line per skipped id and preset, `tests-skipped=4` | the scenario file removed (`tests-missing`), a skip without `skip-ok` | `238c76f` (07-10) |
| V17 | D-31 | the consistency rule in the `golden` step in both directions, controls `golden-skip-ok-without-user-content`, `golden-user-content-not-skip-ok`, `golden-user-scenario-tracked` | an owner-content scenario run as an ordinary test | `adba1ea` (07-10) |
| V18 | D-31 | `runner-process.owner-run.*` (nine cases: usage, `CI` and `GITHUB_ACTIONS`, in-tree path, wrong size, missing lock, bad lock line, no images with exit 77, identity mismatch) and the self-control `runner-process.judge.owner-run-refusal` | the helper exits 0 where it must refuse | `813a5aa`, `a147b63` (07-11) |
| V19 | D-31 | `runner-process.refuse-user.audio-raw`, `.psg-log-out`, `.audio-wav`: exit 14, one stderr line, no file | a derived output is created for user content | `1cb40b0` (07-11) |
| V20 | D-34 | the hosted metric `r = cpu_s / build_s` from `.planning/ci-runs.tsv`, checkpoints C-A (07-01 start), C-B (07-08), C-C (07-09) and the close in `.planning/CI-EFFICIENCY.md` (`phase7-close`) | the last-five median `r` above 8.3 starts a lever plan and no further case | `83c6ecc` (07-01), `c330b37` (07-08), `3248f06` (07-09), plan 07-12 |

### Requirements

| Requirement | Evidence | Oracle kind | Tier and standing | Result |
|-------------|----------|-------------|-------------------|--------|
| SCN-01 | SC1 to SC3; V1 to V14; registry gates `scn-differential`, `scn-mutants`, `scn-shape`, `scn-video-pair`, `pkg-install` (20 lines) | hand-derived expectations committed before the program, blind second read of the specification, mutants, metamorphic judge | CL-0023 (`playable-guest-progress`), CL-0024 (`installed-host-behavior`); provisional | pass |
| SCN-02 | SC4; V15 to V19; registry gates `scn2-owner-run`, `scn2-user-content` (3 lines) | gate checks of the documented route and of every refusal; no owner image is run | no claim and no tier (D11 open, not performed) | pass on documented-route terms only |

### Oracle order (D-23, A-08)

| Step | Commit | Evidence |
|------|--------|----------|
| the oracle (derivation sections (a) to (j), frame literals F-S1 to F-S6, pair inventory) | `8ded8dd`, `d67b180`, `c38d85e`, `4e0febe` (plan 07-02, `oracle-commit:` of its SUMMARY) | no program, listing, scenario or golden existed |
| the blind second read of section (a) | `1621dae` (the record, `state: closed`, zero corrections) | the frozen baseline of the freeze command |
| the program (source, generated listing, manifest) | `949407e` (plan 07-06) | `git merge-base --is-ancestor` of the oracle commit and of the record against `949407e` exits 0 |
| the scenarios and the hand-derived `.expect` files | `8cb4329`, `2188b4a` (plan 07-07) | the same ancestry checks against `8cb4329` exit 0; the `.expect` assert lines equal the derivation's blocks line for line (`golden-expect-drift`) |
| the installed-consumer replay | `da07663` (plan 07-09) | the same ancestry checks exit 0 |

The freeze command (derivation sections (a) to (j) and the F-S literals unchanged since the record, apart from bare claim tags and an appended section (k)) is a verify of plans 07-06, 07-07 and 07-12; plan 07-12 ran it on its final tree and it passed. Order caveat, stated as in the note of CL-0023: the four assert blocks were run once as scratch scenarios in plan 07-06 (all passed at the first run) before the scenario files entered the repository in plan 07-07, and no assert line changed afterwards. The `.digests` goldens are self-recorded and show repeatability only.

## 2. Handed off to the owner (`owner-handoff`, non-blocking missing evidence, never performed by an agent)

| Item | Why no tool can judge it | Owner action | Effect on claims |
|------|--------------------------|--------------|------------------|
| H7-1 Listen to the exported WAV of the pass scenario | a person's judgement of sound; **not evidence** | run `sh tools/listen-demo.sh demo-lamp-tone-pass` and listen to `build/listen/demo-lamp-tone-pass.wav` | none; no claim depends on it |
| H7-2 View the exported frames (`--frame-ppm`) of the pass scenario | a person's judgement of a picture, with an arbitrary preview palette; **not evidence** | run `ivs-run` on the pass scenario with `--frame-ppm <file>` and open the file in any viewer | none; no claim depends on it |
| H7-3 The D10 hardware items | physical measurement on a console | unchanged: 42 items in `data/expected/d10-measurements.ids` (`docs/input/d10-procedure.md`, `docs/stic/d10-procedure.md`) | unchanged: no `measured-hardware` claim |
| H7-4 The D11 owner-local EXEC and GROM run | the images are the owner's and rights-bearing; **not performed** | follow `docs/owner-images.md` with `tools/owner-run.sh`, never commit an image, never run in CI | no claim and no tier for any real-EXEC result |
| H7-5 Publication | the delegated flow of `.planning/AUTONOMY.md` section 5; no release exists | the orchestrator's release flow; the content statement is in `docs/packaging.md` ("Release content") | none |

## 3. Accepted caveats

- **Shared misreading, with one blind read.** One author wrote the derivation, the literals, the rules, the program, the consumer and the core; one blind second read of the specification (zero corrections; the one reading the reader left open, the carry of the lamp builder, is closed in section (k) of the derivation) is the same model family reading the same pages. Agreement shows that the program does what its specification says, not that a console does.
- **Tier 4 and inference fields.** The controller byte table (31 rows, community literal), the STIC register and strobe rules, the access windows and the PSG function rules are tier 4, inference or labelled defaults, so CL-0023 and CL-0024 are provisional by the weakest-field rule. A tier 1 to 3 source or a D10 capture is the resolver.
- **The idle expectation is a diagnostic, not a progress claim.** The `idle` scenario carries the tier `original-diagnostic` (it has no input line by design); only `pass` carries the playable tier.
- **`.digests` goldens and the consumer golden are repeatability only.** They are self-recorded by the emulator; the independent values are the assert lines and the consumer's own figures, which carry the shared-misreading caveat.
- **Index frames, not pictures.** The frames are raw palette-index planes; no palette, scaling or presentation is claimed, and no person has looked at one (H7-2).
- **No timing is claimed.** Machine time is printed in reports and never asserted; the interrupt routine's exact length, the re-arm delay, the log tick and the tone phase are excluded on purpose. The re-arm band edges are derived, with the as-built lower edge two iterations lower than the reference sequence (section (k)).
- **The program is one program.** A second program is a second claim. Moving objects, collisions, delay registers and the border are Phase 8 (VID-05).
- **Mutant coverage is by class.** The `SCN-REARM-LONG` mutant uses N = 1,023, the largest value one patch word can hold, instead of the 1,150 of the derivation's table (the same first failing clause and value).
- **Cost checkpoints.** C-A (plan 07-01, lever applied), C-B (plan 07-08: `lever-required` by the no-row branch of the rule, resolved by hosted run 43528527276 to no lever), C-C (plan 07-09, `no-lever`) and the close (`phase7-close`, `.planning/CI-EFFICIENCY.md`). The close reads the last-five hosted median `r` of 7.78 against the budget 7.8 and the revisit line 8.3; two raw triggers fire on the newest row (gate 146 s above the any-run bound of 130 s, and a cpu median of 335 above 330) at the slowest `build_s` of the file (60 s), which the rule of the Phase 6 start entry classes as a runner-speed event while `r` is inside the budget.
- **The owner-run helper has one known gap.** A symbolic link inside `refs/` to a file elsewhere in the work tree passes the in-tree check (the check is by the physical directory of the path); no test exists for it. `--record` is trust on first use by design.
- **The SKIPPED inventory line** does not carry the words "D11, no claim" (the helper's own line and the page do); recorded in the SUMMARY of plan 07-10.
- **Counts against the guideline (A-11).** The phase added 13 pure controls (344 to 357) and no fault (222), as planned; the phase counts go from `tests-expected=4432` to `4624` and `tests-skipped` from 2 to 4.
- **No rights-bearing bytes.** `git ls-files '*.ivsc' '*.bin' '*.rom' '*.exec' '*.wav' '*.ppm' '*.vraw'` is empty; every image is built from tracked source into the build tree, exported planes exist in scratch only, and no EXEC, GROM or commercial byte exists anywhere in the repository. D10 and D11 were not performed.

## 4. Open items carried to Phase 8 and later

- Phase 8 (VID-05, STA-01, STA-02, PRF-01, REL-01): MOBs, border, delay registers and collision as single-feature fixtures built with `ivs_asm` (each its own claim), save state, benchmarks, the release-readiness assessment.
- Optional, from plan 07-09: an erratum note in derivation section (k) that the gate-window starts of section (d) (124,132 and 721,848) are reference-sequence figures and the as-built starts are 56 ticks later; no literal or assert depends on it.
- Optional hardening of the owner-run helper (symlink targets inside the work tree) and of the SKIPPED line wording.
- Owner items: H7-1 to H7-5 above, and the carried D10 and D11 items.
- Carried unchanged: the Phase 4 and 5 review deferrals; `sh tools/hygiene.sh --history` on earlier planning commits; the orchestrator-owned hosted CI records (agents never call `gh` or `tools/ci-record.sh`).
