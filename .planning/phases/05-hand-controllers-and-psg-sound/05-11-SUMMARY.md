---
phase: 05-hand-controllers-and-psg-sound
plan: 11
subsystem: fixtures, data/scenarios, data/golden, tests/run, tests/installed-consumer, tools/check/steps, runner-process, claims, docs
tags: [fixture, diagnostic, original-diagnostic, hand-assembled, scenarios, mutants, refpcm-golden, installed-consumer, claims, D-24, D-05, D-19, D-21, D-26, A-05, A-12, INP-01, INP-02, INP-03, API-03, AUD-01, AUD-02]
status: complete

requires:
  - phase: 05-hand-controllers-and-psg-sound
    provides: "plan 05-01 oracle commit 14a1904 (the derivation of the fixture, the controller and register literals); 05-05 the guest-visible ports; 05-06 and 05-07 the PSG sample stream; 05-08 the write log; 05-09 the runner capture, psglog asserts, ivsr 2 and the operator files; 05-10 the reference PCM converter and listen-demo.sh (HEAD at start 5d93247)"
provides:
  - "fixtures/FX-diag-ctrl-psg-1: the second original diagnostic, 89 words at 5000-5058, hand-assembled (D4), id ivs-diag-ctrl-psg-1, image diag-ctrl-psg-1.ivsc in the build tree only"
  - "scenarios diag-ctrl-psg-pass, diag-ctrl-psg-nokey (timeout 11, budget) and diag-ctrl-psg-order (pass; every event split in two lines one tick apart); data/golden/diag-ctrl-psg-pass.expect (the 05-01 derivation, original-diagnostic), .digests (self-recorded) and .refpcm (converter repeatability only)"
  - "data/expected/diag-mutations-ctrl-psg.ids: five mutants with an explicit from column and per-row outcome and exit; one shared mutant runner in the runner-process script"
  - "run.diag.ctrl-psg.* (10 in-process ids, tests/run/run_diag_ctrl.c) reading the .expect through expect-inc.cmake (psglog forms added)"
  - "golden.sh compares the .refpcm golden with the dev and release refpcm reports (control golden-refpcm-mismatch); refpcm-scenarios.ids"
  - "the installed consumer replays diag-ctrl-psg-pass (ctrl.* lines, peeked 01FE = 0x007E) and pkg-install compares its digests and observed domains with the runner golden; 3 more in-step self-tests"
  - "claims CL-0014 (original-diagnostic) and CL-0015 (installed-host-behavior), both provisional"
affects: [05-12]

actuals:
  tokens: 28000
  tasks: 5
  commits: 6
plan_head_before: 5d9324760ffbbf18231f04ee391651bda7482f03
plan_head_after: 689c12aa44dea8c53bebaea2a428067376a158d6

tech-stack:
  added: []
  patterns:
    - "Hand-assembled listing from a derivation committed earlier; the expectation file is the derivation moved in, never a run's output"
    - "One mutant runner function serving two inventories with different columns"
    - "A per-scenario companion report (<id>.refpcm.report) kept out of the scenario report set and compared with its golden by the golden step"
    - "A prefix (ctrl.) on the second replay's consumer lines so the first replay's pinned lines stay byte-identical"

key-files:
  created:
    - fixtures/FX-diag-ctrl-psg-1/diag-ctrl-psg-1.listing.txt
    - fixtures/FX-diag-ctrl-psg-1/manifest.json
    - data/scenarios/diag-ctrl-psg-pass.ivss
    - data/scenarios/diag-ctrl-psg-nokey.ivss
    - data/scenarios/diag-ctrl-psg-order.ivss
    - data/golden/diag-ctrl-psg-pass.expect
    - data/golden/diag-ctrl-psg-pass.digests
    - data/golden/diag-ctrl-psg-pass.refpcm
    - data/expected/diag-mutations-ctrl-psg.ids
    - data/expected/refpcm-scenarios.ids
    - tests/run/run_diag_ctrl.c
    - tools/check/controls/golden-refpcm-mismatch/
  modified:
    - docs/input/diag-ctrl-psg-1-derivation.md
    - data/expected/fixtures.ids, listings.ids, runner-scenarios.ids, pkg-selftests.ids, pkg-consumer.golden, claims.ids, controls/golden.ids, tests/pack.ids, tests/run.ids, tests/runner-process.ids, faults/run.ids
    - tools/pack/CMakeLists.txt, tests/CMakeLists.txt, tests/expect-inc.cmake, tests/run/run_main.c, run_cases.h, tests/runner-process-tap.sh
    - tests/installed-consumer/main.c, tools/check/steps/pkg-install.sh, tools/check/steps/golden.sh, tools/listen-demo.sh
    - data/claims.json, docs/diagnostics.md, docs/runner.md, docs/packaging.md
    - tools/check/controls/_good/tree (toy refpcm inventory, two toy reports, toy golden header)

key-decisions:
  - "The pointer registers differ from the derivation (R4 port 0, R5 port 1, R3 the count): only modes 100 and 101 auto-increment (Fig. 3.2.1, pdf 46). Recorded as a dated section (g) of the derivation in its own commit before the listing; no asserted value, word count or cycle figure changes"
  - "Listing id is ivs-diag-ctrl-psg-1 as the plan says (the analog's id is diag-cpu-1); the image file stays diag-ctrl-psg-1.ivsc"
  - "The scenario asserts the psglog records before the PSG register read-backs, so a mutant of a PSG write names a log clause (the plan's wording); the port-1 tail [0224..0227] is four single-word asserts because expect-inc.cmake has no range form (the in-process test reads the same file)"
  - "The .refpcm golden is written by a documented recipe from a runner report (docs/runner.md): the runner has no .refpcm writer and --update-golden writes .digests only"
  - "The consumer's second replay prints ctrl.-prefixed lines, so the Phase 4 digest.* lines and the existing comparison are untouched; check_one_golden serves both goldens"

duration: about 40 min of edits (07:47 to 08:18 for the six commits) plus 12 gate runs (6 sh, 3 dash, 1 container, 2 failed on a stale golden and a docs word)
completed: 2026-11-12
---

# Phase 5 Plan 11: fixture ivs-diag-ctrl-psg-1 and end to end Summary

**A hand-assembled original diagnostic reads both controller ports and programs the PSG through the real runner and the installed package, and passes the expectations derived before it existed; the gate ends `steps=35/35 controls=313/313 faults=191/191 tests-missing=0` under `sh`, `dash` and the Ubuntu container.**

Oracle commit (05-01): `14a1904`. The derivation `docs/input/diag-ctrl-psg-1-derivation.md` was added in `14a1904` (the oracle commit itself, plan 05-01 Task 5); the listing was added in `5560471`, so the derivation precedes the program in `git log`. `git merge-base --is-ancestor $(git log --diff-filter=A --format=%h -1 -- docs/input/diag-ctrl-psg-1-derivation.md) $(git log --diff-filter=A --format=%h -1 -- fixtures/FX-diag-ctrl-psg-1/diag-ctrl-psg-1.listing.txt)` succeeds (checked).

## What was built

- `804d448` (Deviation 1): the dated correction section (g) of the derivation (pointer registers), its own commit before the program.
- `5560471` (Task 1, tracer): listing (89 words, 4 branches each annotated `-> ADDR`), manifest, ids, `ivs_images` and the C include pair, scenario `diag-ctrl-psg-pass`, its `.expect` (the derivation moved in; `# policy: C64` on the ten 16-bit PSG read-backs, `# policy: C75` on the shape register) and `.digests` (written by `ivs-run --update-golden`). The first run passed all 41 asserts and halted at tick 16,752 (derivation estimate 16,700).
- `6996cbb` (Task 2): the two other scenarios, five mutants, the 10 in-process ids, repeat, 1 and 7 tick slicing and other-directory variants for the new pass scenario only.
- `2794668` (Task 3): the `.refpcm` golden, its inventory, the TAP case, the `golden.sh` comparison and the control `golden-refpcm-mismatch`.
- `37fedf7` (Task 4): the consumer replay, the second runner-golden comparison, three self-tests, the refreshed pin (additions only).
- `689c12a` (Task 5): CL-0014 and CL-0015 and the docs.

## Expectations and what they exercise

- Write list (11 records, addresses 0 4 8 11 1 5 12 9 3 7 10) and the RAM tables come straight from the 05-01 derivation. Port 0 table `00FF 007E 00FF 005A 00FF 00FB 005B 001D`, port 1 table `00FF 00FD 009F 00FF` and four zero words, marker `03C5`.
- Nothing was copied from a run: the `.expect` and the scenario were typed from the derivation; the first run agreed.
- `run.diag.ctrl-psg.*` read the same assert lines through `diag_ctrl_psg_pass_expect.inc`; changing an expected value in the `.expect` (tested once, then restored) fails `ram-tables`, `log-list` and `tone-a-silent-until-reload`.
- Channel B is constant at level 8 from the sample after the amplitude-B write (sample i is stamped (i + 1) x 16; a write below the stamp is seen), channel C and channel A stay 0 for the whole run (output.audio.changes is 1). The log record ticks equal an observed unit start plus 6 CPU cycles x 4 for all eleven records.

## Mutants (outcomes judged by the runner-process script)

| ID | addr | from to | outcome |
|---|---|---|---|
| MIXER-VALUE | 5009 | 003E 003F | guest-fail 10, `psglog 2 addr=8 value=003E ...` got value 003F |
| TONE-HIGH-VALUE | 5005 | 0001 0002 | guest-fail 10, `psglog 1 addr=4 value=0001 ...` |
| PORT1-STORE-ADDR | 5033 | 0220 0221 | guest-fail 10, `mem 0220 == 00FF` got 0000 |
| ANDI-TO-MVII | 5038 | 03B8 02B8 | timeout 11, detail `budget` (port 0 frozen at 00FF) |
| DELAY-COUNT-CHANGED | 504D | 0078 0050 | pass 0 (negative mutant; halt at 14,352) |

The four Phase 4 mutants keep their loop semantics (one `run_mutant` function now serves both inventories).

## Evidence numbers

- Gate (end): `gate: PASS steps=35/35 controls=313/313 faults=191/191 tests-expected=3772 tests-executed=3770 tests-skipped=2 tests-missing=0 listings=3 manifests=5 consumer-lines=43 pkg-selftest=13/13` (05-10 end: 3724 expected, 312 controls, 27 consumer lines, 10 self-tests). `goldens=7`.
- Counts: controls 312 + 1 (golden-refpcm-mismatch) = 313; faults 191 + 0; new tests-expected 48 = 24 ids x 2 presets (11 runner-process scenario, mutant, determinism ids, 1 refpcm id, 2 pack ids, 10 run ids).
- runner-process spawns added per preset: 13 (pass, nokey, order scenarios 3; 5 mutants; repeat and two slicing runs 3; other-directory run 1; the refpcm run 1). Task 2 added 11, under the cap of 12.
- Plain-run times (median of 3, load average 8 to 9): `run` dev 0.61 s, release 0.31 s (the ten new cases 0.09 s dev, under 0.01 s release); `runner-process` dev 7.4 s (05-10: 4.5 s), release 2.9 s (05-10: 2.6 s). Gate local: `gate-time total=41` and `gate-cpu` 68 + 57 = 126 cpu-s (05-10 end: 131 to 135), `dash` 42 s and 130 cpu-s, container `gate-time total=180` (emulated amd64, correctness only).
- Budget risk (hosted cpu_s median 260 / trigger 280, gate wall trigger 90, last-5 medians 231 cpu and 78 s, the last two rows 103 and 106 s): local CPU is flat within noise. The added wall is the dev `runner-process` run (about +2.9 s here, the sanitized `ivs-run` spawns) and about +0.3 s release, so hosted wall may grow by roughly 3 to 5 s on top of rows that are already near 90 s. Hosted cost is unmeasured until the next hosted rows; I would call the wall budget at risk, the CPU budget not.
- WAV (helper `sh tools/listen-demo.sh diag-ctrl-psg-pass`): 492 bytes, 224 frames at 48000 Hz, `refpcm.sha256 999ad54be943b95e695bc7968898a1c5f9d6c1473e5f171a1c332d8190bac247`, equal to the golden. The WAV is about 5 ms of near-silence (tone A is silent for the whole run: its period write takes effect after the 4096-step first reload, far beyond the halt), so it shows the export path only: no listenable-tone claim and no listening handoff is created (D-22 stays not performable).
- Claims added: `CL-0014` (original-diagnostic) and `CL-0015` (installed-host-behavior), both `provisional`; the weakest-field rule keeps them there (reset.r7, line_idle, regmap and control rows tier4-community; line_model inference; cartridge.slots, psg.mixer.port_dir_bits, memmap.device.psg8 unknown-default-chosen).
- Pre-existing goldens: `git diff 6e53f39 HEAD -- data/golden/diag-cpu-1.digests diag-input-pass.digests timeout-budget.digests diag-cpu-1.expect` is empty (6e53f39 is the end of plan 05-09).

## Deviations from Plan

**1. [Rule 1 - Derivation inaccuracy] Pointer registers.** The derivation stores through `MVO@ R0,R3` with R3 auto-incrementing; only R4 and R5 do. The listing uses R4 (port 0), R5 (port 1) and R3 (count). Dated correction section (g) in commit `804d448`, the original text kept; word count, loop addresses, cycles, tables, write list and mutants are unchanged except that mutant 3 patches `MVII #$0220,R5`.

**2. [Rule 1 - Plan precision] Assert order and the port-1 tail.** The psglog records are asserted before the PSG read-backs, so MIXER-VALUE and TONE-HIGH-VALUE name `psglog` clauses as the plan says (with the read-backs first they would have named `mem 01F8` and `mem 01F4`). `[0224..0227]` is four single-word asserts: the expect-include grammar has no range form (the in-process tests read the file). `tests/expect-inc.cmake` gained the two psglog forms (not in the plan's file list).

**3. [Rule 3 - Blocking] Fault collateral and a hang.** The first run of the `run` faults timed out (120 s) under `RUN_STOP_BEFORE_OVERSHOOT`: with a one-tick slice the faulted core never progresses and my loop never ended. `dc_run` now caps the calls and requires progress. The new family also fails, by design, under input-related faults, so `data/expected/faults/run.ids` allows `run.diag.ctrl-psg.` (RUN_EVENT_AT_TARGET_ONLY), `run.diag.ctrl-psg.slicing` (PEEK_ADVANCES_TIME, INPUT_LOG_SLICE_RELATIVE_TICK) and `...slicing|...order-equals-pass` (INPUT_APPLY_AT_RUN_START); `RUN_STOP_BEFORE_OVERSHOOT` already allowed `run.diag.`. No fault was added (191).

**4. [Rule 3] Mutant `from` check.** The inventory's `from` word is compared with the word at `<addr>` of `ivs_pack --dump-words` of the unpatched listing (the build tool's own decode), not with bytes read back out of the IVSC container.

**5. [Rule 3] The `.refpcm` writer and report set.** The runner has no `.refpcm` writer, so the documented writer is a shell recipe from a runner report in `docs/runner.md`. `golden.sh` now keeps `*.refpcm.report` out of the dev versus release scenario comparison (they are not scenario reports), compares the `.refpcm` golden only when its grammar held (so `golden-refpcm-grammar` keeps its two findings), and requires every line of `refpcm-scenarios.ids` to have a golden. The toy control tree gained a toy inventory and two toy reports.

**6. [Other] Consumer output.** The second replay's lines are `ctrl.`-prefixed (`ctrl.digest.<domain>`, `ctrl.peek 01FE=0x007E`, `ctrl.mem ...`), so the Phase 4 lines stay as they were; the pin changed by additions and by three header comment lines only. `check_runner_golden` became `check_one_golden` called twice. The self-tests are `ctrl-digest-drift`, `ctrl-domain-missing`, `ctrl-peek-wrong`. `pkg-consumer-calls.ids` is unchanged (`ivs_machine_peek` was already referenced).

**7. [Other] Claims dependencies.** `depends_on` also names `input.controllers` and `psg.mixer.port_dir_bits` (the poll depends on bits 6 and 7 staying input) besides the plan's list.

**8. [Flake, not a deviation] `dash tools/gate.sh` once failed `c-faults-run: watchdog self-test (quiet)` at load average 8.9; the same command passed on the immediate rerun. No code was changed for it.

## Decisions Made

See key-decisions. The project mini-assembler trigger sentence for Phase 7 (a third diagnostic, or labels and macros) is in `docs/diagnostics.md`; D4 stands.

## Known Stubs

None.

## Open Items

- **INP-01, INP-02, INP-03, API-03, AUD-01 and AUD-02 stay Pending in REQUIREMENTS.md.** This plan supplies the end-to-end evidence (guest reads, write list, slicing variants, installed replay, `.refpcm` separate from evidence), but plans 05-05, 05-06, 05-09 and 05-10 recorded that plan 05-12 owns the important-gates registry and the traceability edit for all six requirements; nothing is marked here.
- The D-22 listening check is not performable with any Phase 5 artifact (WAV of 224 frames of near-silence); a later phase needs a longer guest.
- Blind second reads of the controller and register literals are still pending (05-01); the claims keep the shared-misreading caveat.
- Hosted cost of this plan is unmeasured (see Evidence numbers; wall is the exposed budget).
- Carried unchanged: the 05-04 unobtained AY-3-8910 reprint, D10 and D11 items, Phase 4 items IN-04, IN-06, IN-09 to IN-11, the C73 tag wording of 05-05, `psg.mix.truth-table` polarity invariance (05-06), the noise first-bits and envelope phase conventions (05-07), the interrupt-entry bound inputs (05-08).

## Final gate

- `sh tools/gate.sh`: `gate: PASS steps=35/35 controls=313/313 ... faults=191/191 ... tests-expected=3772 tests-executed=3770 tests-skipped=2 tests-missing=0 ... listings=3 ... consumer-lines=43 ... pkg-selftest=13/13`, 0 FAIL lines, `gate-time total=41`.
- `dash tools/gate.sh`: the same line, 0 FAIL lines, `gate-time total=42` (after one rerun for the flake in Deviation 8).
- `sh tools/gate-linux.sh`: Ubuntu 24.04 container (gcc 13.3, cmake 3.28, dash): `gate: PASS steps=35/35 controls=313/313 ... cc=GNU ... faults=191/191 ... tests-expected=3772 tests-executed=3770 tests-skipped=2 tests-missing=0`, exit status 0, `build/gate-linux.log` last line starts with `gate: PASS`.

## Threat Flags

None new. T-05-27 (rights-bearing bytes): the listing is project-authored, the `.ivsc` lives in the build tree only (`git ls-files '*.ivsc' '*.bin' '*.rom' '*.wav'` is empty), the manifest records source, hashes, licence and `redistributable yes`; T-05-28 (circular evidence): the derivation precedes the listing, the `.expect` assert lines equal the scenario's (`golden-expect-drift`), five mutants including a negative one, the consumer is built against the moved prefix and compared with the runner golden. T-05-SC: no package installs.

## Self-Check: PASSED

- Files exist: fixtures/FX-diag-ctrl-psg-1/diag-ctrl-psg-1.listing.txt, fixtures/FX-diag-ctrl-psg-1/manifest.json, data/scenarios/diag-ctrl-psg-{pass,nokey,order}.ivss, data/golden/diag-ctrl-psg-pass.{expect,digests,refpcm}, data/expected/diag-mutations-ctrl-psg.ids, data/expected/refpcm-scenarios.ids, tests/run/run_diag_ctrl.c, tools/check/controls/golden-refpcm-mismatch.
- Commits on HEAD: 804d448, 5560471, 6996cbb, 2794668, 37fedf7, 689c12a (`commits: 6` measured from the ledger, `git rev-list --count 5d93247..HEAD` at write time).
