# Phase 6: STIC Video First Pass - Pattern Map

**Mapped:** 2026-11-12
**Base commit:** 0d016c1 (Phase 6 discuss context); Phase 5 is landed and merged (ec735bd).
**Input:** `06-CONTEXT.md` (D-01..D-24 and Reconciliation), `06-DISCUSS-{regs,timing,frame,cards,verify}.md`, `AGENTS.md`, `.planning/AUTONOMY.md`, `05-PATTERNS.md` (shape template) and the LANDED Phase 5 tree (not the Phase 5 plan text).
**Files analyzed:** about 150 new or modified files (grouped below).
**Analogs found:** about 128 exact or role-match of about 150; the rest partial or none (section "No Analog Found").

Every analog path below was checked against `git ls-files` (tracked source, no gitignored mirror, no `.gsd/capabilities` copy). Paths are repo-relative on purpose: the hygiene step also scans `.planning/` and rejects home paths, scratch paths and email-like strings. Line numbers are of the tracked file at the base commit.

## Reality check (read before planning)

1. **Names that actually landed in Phase 5** (use these, not the Phase 5 plan placeholders): `core/src/psg/{psg.h,psg_params.h,psg_state.c,psg_regs.c,psg_tone.c,psg_noise.c,psg_env.c,psg_mix.c,psg_sample.c,psg_log.c}`, `core/src/machine/machine_bus.c` (the machine-level bus wrapper), generators `cmake/{IvsMemmapTable,IvsPsgTable,IvsCtrlTable}.cmake` (all three feed `ivs_core_gen`), test-side generators `cmake/{IvsMemmapProbes,IvsInputProbes,IvsPsgVectors,IvsPsgProfileProbe,IvsPsgDacIndependence}.cmake`, suites `psg` and `input`, runner `runner/{runner_chain.c,runner_sha256.c}` (runner-local audio chain recompute and a runner-local SHA-256 copy), `tools/pcm/` (the pure reference converter, wired as a third fault/c-build/SPDX/runner-scan module), `tools/listen-demo.sh`, `tools/review/`, third image `diag-ctrl-psg-1`, mutants inventory `data/expected/diag-mutations-ctrl-psg.ids`, pending inventories `data/psg/{fields,vectors}-pending.ids`, `ivsr 2`.
2. **Phase 6 adds NO digest domain.** Domain 16 already exists in the field table (`machine_digest.c` L109-110, `IVS_DF_VIDEO_DOMAIN`, rows `obs.video.chain` and `obs.video.count`), the runner already has `digest[9][32]` with video as row 5 (`ivs_runner.h` L492, `exec.c` L79-81) and `main_golden_keep` already has a video live row. What changes is that domain 16 goes LIVE (D-16) and the composite goes to version 3. Do not copy the Phase 5 "new domain" move-together row; use "domain 16 live + composite v3" below.
3. **The five discuss notes disagree on names; CONTEXT wins, the rest is a planner decision (listed here once):** flag name (`IVS_RUN_F_STOP_ON_FRAME`, CONTEXT); step names (verify note: one new step `video`; regs note adds `stic-vectors` and `stic`; cards note `video-vectors`); where frame text lives (`data/video/frames/*.frame` in the verify note, `data/video/expected/<id>.frame.txt` in the frame note, vectors under `data/video/vectors/<id>.txt` in the cards note); literal frame width (verify note 159 digits per line, frame and cards notes 160 = 20 cards wide then cropped); the `D10-VID-nn` and `Cnn` numbering (every note allocates C81 onward and `D10-VID-01..`; one table in plan 06-01); fixture name (`ivs-diag-stic-1`, verify note, over the cards note's `ivs-diag-video-1`).
4. **Correction to the verify note (V6-22):** "new public symbols are added to `core-symbols-allow.txt`" is wrong. `tools/check/core-symbols-allow.txt` is the reviewed list of allowed IMPORTS (memcpy, memset, stack-protector names). A new public function needs: the one-line `IVS_API` prototype, an `api-effects.json` row, header compile (`tests/headers-tap.sh`), and `pkg-consumer-calls.ids` only if the installed consumer calls it.

## How to read this file

Conventions [S] [C] [I] [H] [T] [F] are those of `04-PATTERNS.md`; [A] [D] [G] [P] [X] are those of `05-PATTERNS.md` and still hold:

- **[A] Append-only ABI growth** (`ivs_machine_info`, `machine.c` L274-337): a struct starts `uint32_t struct_size`; the minimum accepted size is a PINNED v1 constant (`IVS_INFO_V1_SIZE` 216, `IVS_RUN_REPORT_V1_SIZE` 112, `IVS_RUN_REQUEST_V1_SIZE` 64), never `sizeof` of the grown struct; the core writes `min(struct_size, sizeof)` bytes and sets `struct_size` to that count.
- **[D] Digest field row**: one table `ivs_digest_fields[]` (`machine_digest.c` L65-125) covers every instance byte exactly once with `IVS_DF(name, member, width, count, domain, kind)`; `slot[]` stays the last member (static assert L43-45).
- **[G] Generator + independent literal**: a `cmake -P` generator reads committed data, fails closed with `message(FATAL_ERROR "<Gen>: <ctx>: <reason>")`, takes `-DROOT -DOUT` (+ optional `-DMUTATE`), writes a build-tree `.inc` that is never committed; the expected values come from a SEPARATE literal probe (`IvsMemmapProbes`, `IvsInputProbes`, `IvsPsgVectors`), never from the profile or the generated table.
- **[P] Pure reads**: CPU reads, ADAR strobes and `ivs_machine_peek` all go through `ivs_memsys_read(const ivs_memsys *, addr)`; every side effect lives in the machine bus wrapper.
- **[X] Cross-suite twin**: run-driven and seam-stepped instances are compared by whole-storage `memcmp`; every new instance byte must be equal in both paths.

Three Phase 6 conventions, named once:

- **[E] Event catch-up** (`psg_sample.c` L109-128, `machine_bus.c` L31-65): a lazily stepped device is caught up to an access STAMP before the access is applied (step first, then act) and to the unit end after every unit, in the seam and the run call alike, never to a call target.
- **[Z] Counting sink** (`psg.h` L91-105, `psg_sample.c` L90-103): one per-call stack struct that always counts and writes only when a buffer is present, so a NULL buffer runs the same code and machine state never depends on a buffer.
- **[O] Oracle-first**: plan 06-01 contains no C; expected values, literals, derivations and blind second-read requests are committed before any implementation; each later SUMMARY names the oracle commit (the verifier checks `git log` order).

## File Classification

### A. Core: STIC device, timeline, renderer, video sink, run and digest growth

| New/Modified File | Role | Data Flow | Closest Analog | Match | Plan |
|---|---|---|---|---|---|
| `core/src/stic/stic.h`, `stic_params.h` (new internal headers: params struct, `ivs_stic_state`, sink, API) | internal header | event-driven (lazy catch-up) | `core/src/psg/psg.h` L25-199, `psg_params.h` L27-92 | exact (shape) | 06-03 |
| `core/src/stic/stic_{regs,access,timeline,frame,card}.c` (new, one small TU per rule family, names free) | service | event-driven + transform | `psg_regs.c`, `psg_sample.c` L109-128, `psg_log.c`, `psg_state.c` | role-match | 06-04..08 |
| `core/src/machine/machine_bus.c` (STIC hooks: stamp, catch-up, `$0020`/`$0021` effects, BACKTAB hook) | service | request-response | same file L31-65 | exact | 06-04/05 |
| `core/src/machine/machine_run.c` (lines drive, STIC catch-up at unit end, power-on) | service | request-response + loop | same file L44-77, L117-188 | exact | 06-04/05 |
| `core/src/memmap/memsys.{h,c}` (device kind `stic14`, `stic_reg[64]`, accessors, STIC read/store cases) | service | CRUD (register file) | same files: `memsys.c` L128-179, L402-447; `memsys.h` L48-56, L92-113 | exact | 06-04 |
| `cmake/IvsMemmapTable.cmake` (`stic14` kind, `stic` leaves the stub-only set, stub-vs-real rule) | generator | batch | same file L155-171, L478-486 | exact | 06-04 |
| `cmake/IvsSticTable.cmake` (new, `ivs_stic_params.inc`: regmap masks, windows, bus table, video geometry; `-DMUTATE`) | generator | batch | `cmake/IvsPsgTable.cmake` (whole), `IvsCtrlTable.cmake` | exact (copy) | 06-03 |
| `core/CMakeLists.txt` (new TUs, 4th generated include, `ivs_core_gen` deps) | config | build | same file L11-33, L54-83 | exact | 06-03/04 |
| `core/src/machine/machine_bound.c` (flag, stop FRAME, capacity `+2`, video sink, report v3, pre-unit guard) | service | request-response + loop | same file L87-101, L170-218, L399, L556-669, L686-700 | exact | 06-03/06 |
| `core/src/machine/machine.{h,c}` (layout, bus ctx, env `video_slack_frames`, info v4, `profile_covered`) | internal header + service | request-response | `machine.h` L12-32, L199-263; `machine.c` L50-53, L274-372 | exact | 06-03/04 |
| `core/src/machine/machine_digest.c` (STIC rows, video chain row init, composite v3) | transform | transform | same file L55-125, L346-369 | exact | 06-04/06 |
| `core/src/machine/machine_inspect.c` (+ new public const call `ivs_machine_stic_state`) | service | request-response | same file L45-75 (`ivs_machine_cpu_state`) + `ivs_machine_info` A-02 | role-match | 06-04/08 |
| `core/include/ivs/ivs_run.h`, `ivs_machine.h` (flag, `IVS_STOP_FRAME`, report v3, info v4, format id, STIC report struct, domain-16 text) | public header | request-response | same files: `ivs_run.h` L91-106, L213-270; `ivs_machine.h` L104-175, L310-327 | exact | 06-03 |
| `data/api/api-effects.json`, `tools/check/api-effects.cmake` (stic_state row, run row text, pins) | data record + checker | CRUD / batch | `api-effects.json` L328-354, L389-451; `api-effects.cmake` L67-72 | exact | 06-03 |
| `data/expected/api-stop-reasons.ids` (+`IVS_STOP_FRAME 5`), new format/flag pins if a define class is added | inventory | pin | same file (4 lines), `psg-log-source.ids` | exact | 06-03 |
| `core/src/machine/machine_input.c`-style `ivs_video_init` for the domain-16 chain h0 | service | transform | `machine_input.c` L95-100 (`ivs_input_init`) | exact | 06-06 |

### B. Profile, literals, rule inventories, records, docs

| New/Modified File | Role | Data Flow | Closest Analog | Match | Plan |
|---|---|---|---|---|---|
| `data/profiles/intv-2609-ntsc-family-p1.json` (`stic.*`, `video.*`, `bus.line_sample_lag_cpu_cycles`, `memmap.device.stic14`, region flips) + `data/expected/profile-fields.ids` | data record | CRUD | same file: `memmap.device.psg8` L351-360, `memmap.region.stic_regs` L371-380, `psg.regmap` L518; `profile-fields.ids` | exact | 06-03/04 |
| `data/memmap/intv-2609-ntsc-family-p1.literal.txt` (stic rows split by width, `impl=real`, `x=strobe`) | literal | batch | same file L1-33 header, psg rows | exact | 06-04 |
| `tools/check/memmap.cmake`, `cmake/IvsMemmapProbes.cmake`, `tests/memmap/memmap_rows.c` (the new `x=strobe` exception class) | checker + probe + test | batch | `memmap.cmake` L12-24, L310-315; `IvsMemmapProbes.cmake` L12, L29; `memmap_rows.c` L282, L305-324 | exact | 06-04 |
| `data/stic/intv-2609-ntsc-family-p1.regmap.literal.txt` (64 rows) | literal | batch | `data/psg/intv-2609-ntsc-family-p1.regmap.literal.txt` L1-60 | exact | 06-01 |
| `data/stic/rules.json` (`STIC-R01..`), `tools/check/schema/stic-rule.txt`, `tools/check/kinds/stic-rule.cmake`, `data/expected/stic-rules.ids` | data record + schema + hook | CRUD | `data/psg/rules.json`, `tools/check/schema/psg-rule.txt`, `tools/check/kinds/psg-rule.cmake` L1-60 | exact | 06-01 |
| `data/stic/vectors/*.txt`, `data/expected/stic-vector-files.ids`, `stic-policies.ids`, `data/stic/{fields,vectors}-pending.ids` | literal vectors + inventory | batch | `data/psg/vectors/regs.txt` L1-40, `data/expected/psg-{vector-files,policies}.ids`, `data/psg/{fields,vectors}-pending.ids` | exact | 06-01 |
| `data/video/geometry.literal.txt`, `data/video/cards.literal.txt` (bit-role literal), `data/video/rules.json` (`CR-01..`) | literal + record | batch | `data/input/intv-2609-ntsc-family-p1.literal.txt` (row grammar, tier, independence rule), `data/psg/rules.json` | role-match | 06-01 |
| `data/video/frames/*.frame` (about 8 hand-derived frames), `data/video/vectors/*.txt` (about 24), `data/expected/video-frames.ids`, `video-vector-files.ids` | literal frames + vectors | batch | `data/golden/diag-ctrl-psg-pass.expect` (hand-derived, hash-tied to a scenario), `data/psg/vectors/regs.txt` (id-per-line grammar) | partial | 06-01 |
| `data/discrepancies.json` (+`.ids`): C81.. one table; updates to C1-C7, C10, C11, C21, C38, C41, C52, C60, C61 | data record | CRUD | C77..C80 entries; `tools/check/kinds/discrepancy.cmake` | exact | 06-01 |
| `data/expected/d10-measurements.ids` (about 10 lines) + anchors in a new `docs/stic/d10-procedure.md` | inventory | pin | same file, `D10-PSG-*` lines; `docs/input/d10-procedure.md` | exact | 06-01 |
| `data/claims.json` (+`.ids`): CL-0016.. ; `data/sources.json` (+`.ids`) wiki and BSR locators | data record | CRUD | CL-0014/CL-0015; SRC entries with `rights unknown`, `capture hashed` | exact | 06-01/11 |
| `docs/stic/reference-notes.md` (page-reading verdict block), `docs/stic/timing-notes.md`, `docs/video/frame-derivations.md`, `docs/video/diag-stic-1-derivation.md` | doc | n/a | `docs/psg/reference-notes.md`, `docs/input/diag-ctrl-psg-1-derivation.md` | exact | 06-01 |
| `docs/stic/second-read-request.md`, `second-reader-protocol.md`, `second-read-record-*.md` | doc | n/a | `docs/psg/second-read-request.md`, `second-reader-protocol.md`, `second-read-record-regs.md` | exact | 06-01 |
| `docs/time-base.md` (Frame correction), `docs/runner.md`, `docs/diagnostics.md` (SDBD grammar) | doc | n/a | same files: `time-base.md` L31-32, L72-73; `runner.md` L90-96, L196-216 | exact | 06-01/09 |

### C. Gate steps, checkers, controls

| New/Modified File | Role | Data Flow | Closest Analog | Match | Plan |
|---|---|---|---|---|---|
| `tools/check/steps/video.sh` (+ `stic-vectors.cmake`, `video-vectors.cmake`, `video-frames.cmake` or one merged checker) and `data/expected/gate-steps/video.ids` | checker + step | batch | `tools/check/steps/psg-vectors.sh` (23 lines), `tools/check/psg-vectors.cmake` L1-75, `tools/check/steps/input.sh` + `tools/check/input.cmake` L1-60 | exact (copy) | 06-01 |
| `tools/check/listing.cmake` (`listing-sdbd` operand rule) + 3 controls | checker | batch | same file L20, L60-62 (tokens), L407-440 (per-line operand logic), L460-510 (branch-target rule precedent) | exact | 06-01 |
| `tools/check/kinds/discrepancy.cmake` (nothing new; D10 inventory is already cross-checked) | checker | batch | same file L80-114 | exact | 06-01 |
| `tools/check/steps/golden.sh`, `runner/main.c`, `docs/runner.md` (video live keys) | step + tool + doc | batch | `golden.sh` L1-60, L264-268; `main.c` L350-440; `runner.md` L205-216 | exact | 06-09 |
| `tools/check/steps/important-gates.sh` (add VID-01 VID-02 VID-03 VID-04 VID-06) + `data/expected/important-gates.ids` | step + inventory | batch | same file L21-22 (header), awk `reqs` list; `important-gates.ids` AUD-01/AUD-02 blocks | exact | 06-11 |
| `tools/check/steps/{runner-scan,c-build,core-symbols,integrity}` + `binary-ext.txt`, `core-symbols-deny.txt` (the `tools/frame` helper) | step + lists | batch | the `tools/pcm` entries: `c-build.sh` L41-45, `integrity.cmake` L74, `runner-scan.sh` L17-19/L149, `core-symbols.sh` L156-158, `core-symbols-deny.txt` L52-55 | exact | 06-09 |
| `tools/check/steps/pkg-install.sh`, `data/expected/pkg-install-files.ids`, `pkg-consumer.golden`, `pkg-consumer-calls.ids` | step + inventory | batch | `pkg-install.sh` L466-473, L505-524, L700-724 | exact | 06-10 |
| `tools/check/controls/**` + `data/expected/controls/*.ids` + `_good/tree` toy data (stic and video toys) | control | batch | `controls/psg-vectors-*`, `input-*`, `listing-branch-target-*`, `golden-domains-psglog-missing`, `_good/tree/data/psg/*` | exact | each plan |

### D. Tests and named faults

| New/Modified File | Role | Data Flow | Closest Analog | Match | Plan |
|---|---|---|---|---|---|
| `tests/stic/*.c` (new suite `stic`: reg, alias, window, blocked, enable, reset, peek, irq, steal, slice, bounds, iso, fuzz) | test | event-driven | `tests/psg/psg_main.c` L14-148, `psg_cases.h`, `psg_reg.c`, `psg_slice.c`, `psg_bounds.c`, `psg_iso.c`, `psg_fuzz.c` | exact | 06-04/05 |
| `tests/video/*.c` (new suite `video`: geom, frame, boundary, slice, buffers, chain, fgbg, cstack, card, meta, judge) | test | transform | `tests/psg/psg_buffers.c`, `psg_chain.c` L1-30 (printf-octal recipe), `psg_slice.c`, `psg_profile.c` | role-match | 06-06..08 |
| `tests/CMakeLists.txt`, `tests/faults.cmake` (two suites, generated includes, `-g0` lever) | config | build | `tests/CMakeLists.txt` L194-261, L369-377; `faults.cmake` L79-93, L138-152 | exact | 06-02/04/06 |
| `data/expected/tests/{stic,video}.ids`, `data/expected/faults/{stic,video}.ids`; additions to `run.ids`, `machine.ids`, `memmap.ids`, `runner.ids`, `runner-process.ids` | inventory | pin | `data/expected/tests/input.ids`, `data/expected/faults/psg.ids` L1-20, `data/expected/faults/input.ids` | exact | each plan |
| `tests/run/{run_bound,run_input,run_buffers,run_struct,run_info,run_observe,run_fuzz}.c` (flag flip, v3/v4 pins, video-live, STIC peek) | test | request-response | same files (cited per hazard) | exact | 06-03/06 |
| `tests/machine/{machine_digest,machine_create,machine_standin}.c` (composite v3 literals, `tables-cover-registry`, STIC stand-in programs) | test | transform | `machine_digest.c` L54-65, L113-158, L412-418; `machine_create.c` L812; `machine_standin.c` L31-123, L339 | exact | 06-04/05 |
| `tests/memmap/memmap_main.c`, `memmap_rows.c` (stic rows no longer stub) | test | transform | `memmap_main.c` L81-93, `memmap_rows.c` L282 | exact | 06-04 |
| `tests/run/run_diag_stic.c` (family `diag-stic`: guest end to end) | test | request-response | `tests/run/run_diag_ctrl.c` L1-45 | exact | 06-10 |
| `cmake/IvsVideoFrames.cmake` (build-time include of the frame literals + hash tie) | generator | batch | `cmake/IvsPsgVectors.cmake` L1-50, `tests/expect-inc.cmake` L1-35 | role-match | 06-06/07 |

### E. Runner, operator tools

| New/Modified File | Role | Data Flow | Closest Analog | Match | Plan |
|---|---|---|---|---|---|
| `runner/scenario.c`, `ivs_runner.h` (`capture video`, `assert frame ...`, reasons after 49) | utility | transform | `scenario.c` L514-535, L562-675, L862; `ivs_runner.h` L131-149, L204-209, L275-276 | exact | 06-09 |
| `runner/exec.c` (video buffer sizing `+2`, capture, `video.chain.check`) | service | request-response | same file L694-737, L328-345, L933-953, L843-905 | exact | 06-09 |
| `runner/runner_vchain.{c,h}` (new: runner-side re-hash of the per-frame chain) | utility | transform | `runner/runner_chain.{c,h}`, `runner_sha256.{c,h}` | exact (copy) | 06-09 |
| `runner/report.c` (`ivsr 3`, new keys after the audio keys) | utility | transform | same file L35, L56-135 | exact | 06-09 |
| `runner/main.c`, `hostio.{c,h}` (`--video-raw`, cleanup on failure) | entry + utility | file-I/O | `main.c` L350-445, `hostio.h` L28-70, `ivs_runner.h` L360-392 | exact | 06-09 |
| `tools/frame/ivs_framepreview.{h,c}` + `CMakeLists.txt` (operator PPM, project-authored preview palette) | tool | transform | `tools/pcm/{ivs_refpcm.h,ivs_refpcm.c,CMakeLists.txt}` | exact (copy shape) | 06-09 |
| `tools/look-demo.sh` (operator helper, not a step) | script | batch | `tools/listen-demo.sh` L1-20 | exact | 06-09 |

### F. Fixture, scenarios, goldens, consumer

| New/Modified File | Role | Data Flow | Closest Analog | Match | Plan |
|---|---|---|---|---|---|
| `fixtures/FX-diag-stic-1/{diag-stic-1.listing.txt,manifest.json}` | fixture | file-I/O | `fixtures/FX-diag-ctrl-psg-1/*` | exact | 06-10 |
| `data/expected/{fixtures,listings}.ids`, `data/expected/tests/pack.ids`, `data/expected/diag-mutations-stic.ids` | inventory | pin | same files; `data/expected/diag-mutations-ctrl-psg.ids` header | exact | 06-10 |
| `tools/pack/CMakeLists.txt`, `tests/CMakeLists.txt` (fourth image `diag_stic_1`) | config | build | `tools/pack/CMakeLists.txt` L33-50; `tests/CMakeLists.txt` L147-154 | exact | 06-10 |
| `data/scenarios/diag-stic-{fgbg-pass,cstack-pass,nodisplay}.ivss`, `video-assert-mismatch.ivss`, `runner-scenarios.ids` | data | n/a | `data/scenarios/diag-ctrl-psg-pass.ivss`, `data/expected/runner-scenarios.ids` | exact | 06-10 |
| `data/golden/diag-stic-*.{expect,digests}`; re-blessed `diag-cpu-1.digests`, `diag-input-pass.digests`, `diag-ctrl-psg-pass.digests`, `timeout-budget.digests`, `pkg-consumer.golden` | golden | n/a | `data/golden/diag-ctrl-psg-pass.{expect,digests}` L1-60 | exact | 06-04/06/09/10 |
| `tests/runner-process-tap.sh` (third mutant block, slices loop) | test | batch | same file L383-415, L416 | exact | 06-10 |
| `tests/installed-consumer/main.c` (video buffer replay, v3/v4 sizes) | test | request-response | same file L574-595, L225-293, L456-552 | exact | 06-10 |
| `tools/ci-record.sh`, `.planning/CI-EFFICIENCY.md`, `PROJECT.md` Decision Register | planning doc | n/a | themselves | exact | 06-02/11 |

## Pattern Assignments

### A1. `core/src/stic/stic.h`, `stic_params.h` (state, params, sink)

**Analog:** `core/src/psg/psg.h` for the state and sink shapes; `core/src/psg/psg_params.h` for the explicit parameter struct.

**Copy** (`psg.h` L32-46, flat state, explicit reserved words, size pinned by a static assert so the instance stays a multiple of 16):

```c
typedef struct ivs_psg_state { uint64_t next_sample; uint32_t tone_counter[3]; ... uint32_t reserved[3]; } ivs_psg_state;
_Static_assert(sizeof(ivs_psg_state) == 72u, "ivs_psg_state must be 72 bytes (machine.h layout)");
```

and the params idiom (`psg_params.h` L84-90): `ivs_psg_params_count()` and `ivs_psg_params_get(profile)`; every STIC function takes `const ivs_stic_params *` so tests perturb a stack copy (`psg_profile.c` L1-20).

**Differs:** `ivs_stic_state` carries the frame index and event cursor (D-07: never derived from the tick by modulo), display latch, mode, blocked counters and the 13 x 20 BACKTAB row latches (about 520 bytes, D-11); all u8 flags held in `uint32_t` or with explicit PAD rows (digest table). Phase 5 derived its size in the `machine.h` L12-32 comment (72-byte PSG state "brings the total back to a multiple of 16"); do the same arithmetic in the plan that adds the struct. The renderer's color-stack position is a local variable, not state (cards R-10).

### A2. `core/src/stic/stic_*.c` (one small TU per rule family) and the timeline

**Analog:** `psg_sample.c` for the catch-up scheduler, `psg_regs.c` for the params getter with its named count fault, `psg_state.c` for state reset and sink init, `psg_log.c` for the append-with-chain function.

**Catch-up shape to copy** (`psg_sample.c` L109-128, absolute schedule, idempotent, slicing invariant):

```c
while (s->next_sample <= to_tick / period) {
  ivs_psg_step(p, s, regs, obs, sink);
}
```

STIC form: process, in tick order, every generated-table event with tick `<= to_tick` from `(frame_index, event_cursor)` state (timing T-5: FRAME_START clears the latch and opens both gates, REG_CLOSE, GMEM_CLOSE, ROW_FETCH(0..11), FRAME_END; same-tick order FRAME_END(k) then FRAME_START(k+1)). BUSRQ and INTRM are NOT events: `ivs_stic_lines(params, state, tick)` is a pure function of the frame position and the latch.

**TU shape and fault rules** (05-PATTERNS A1, still binding): prelude first (`#pragma GCC poison float double`), explicit `IVS_U16`-style masks after every operation, `static const` tables only, a file-scope static ONLY inside its named-fault block (`c-purity --forbid-writable-data`). One `IVS_FAULT_<ID>` site per fault in exactly one owner TU (`tests/faults.cmake` L12-17), so split by rule family (`stic_regs.c`, `stic_access.c`, `stic_timeline.c`, `stic_card.c`, `stic_frame.c`, V6-12) and keep each TU small: per-fault cost is optimiser time on the owner TU. File stems must be unique across `core/` and `runner/` (faults.cmake configure check).

**Params table getter** (copy with the SHORT fault, `psg_regs.c` L18-30):

```c
uint32_t ivs_psg_params_count(void)
{
#ifdef IVS_FAULT_PSG_TABLE_SHORT
  return IVS_PSG_PROFILE_COUNT - 1u; /* named fault: the table covers one profile less than the registry */
#else
  return IVS_PSG_PROFILE_COUNT;
#endif
}
```

**Differs:** nothing before Phase 6 drives the CPU lines or steals the bus; the stall IS the stepper's existing `IVS_CP1610_UNIT_BUS_GRANT` sequence (`cp1610.h` L83-88 `lines`, L117) driven by `lines.busrq`, so no new stepper code (T-3). Rendering is the one piece with no analog (see No Analog Found).

### A3. `core/src/machine/machine_bus.c` (STIC hooks)

**Analog:** the same file, L31-65. The Phase 5 wrapper is the proven pattern: decode through the memory system's OWN description (never an address constant), stamp, catch up, let `ivs_memsys_bus` apply the masked store, then post-effects.

```c
if (a->kind == IVS_ACC_WRITE || a->kind == IVS_ACC_DW) {
  ivs_memsys_desc d;
  if (ivs_memsys_describe(c->m->mem.profile, IVS_MEMSYS_VIEW_W, a->addr, &d) == IVS_OK &&
      d.phys == IVS_MEMSYS_PHYS_PSG && c->sink != NULL) {
    stamp = c->unit_start_tick + (uint64_t)a->offset_cpu_cycles * (uint64_t)c->ticks_per_cpu_cycle +
            (uint64_t)c->psg->write_strobe_ticks;
    ivs_psg_catch_up(c->psg, &c->m->psg, c->m->mem.psg_reg, &c->m->obs.audio, stamp, c->sink);
```

**Differs (four things the Phase 5 wrapper does not do):**
1. It handles WRITE and DW only. The STIC hook set also covers READ and FETCH (a STIC cannot tell them apart, regs S-4) for phys STIC, GROM and GRAM, and every write into the BACKTAB span of system RAM (timing T-5 item 3). Hook completeness over all 65,536 addresses x 3 views is a test family (`STIC_BACKTAB_WRITE_NOT_HOOKED`, `STIC_ALIAS_NOT_HOOKED`).
2. The `$0021` read effect is decided by the profile list `stic.mode_select_addrs` (`$0021`, `$4021`, `$8021`, `$C021`), NOT by `ivs_memsys_describe`: in the read view `$4000+` decodes to cartridge space, so describe cannot find the alias.
3. Handlers flip the memory-system gates through `ivs_memsys_set_gate_internal` (`memsys.c` L402-407); the gate bytes are in the MEM domain (`mem.gate_open`, `machine_digest.c` L82).
4. Blocked accesses count in `ivs_stic_state` (reads return `memmap.undriven_read`, writes are dropped, D-06).

The bus ctx (`machine.h` L199-206) gains `const ivs_stic_params *stic` and the STIC sink; a context without a sink stays a legal "bare wrapper test" (L40 comment), which is also how the perturbation tests get params the registry cannot supply.

### A4. `core/src/machine/machine_run.c` (lines, catch-up order, power-on)

**Analog:** the same file. Today `lines` are zero and say so (L150, header L21-22); catch-up happens after time advances (L182-185):

```c
memset(&lines, 0, sizeof lines);
...
im->time_ticks += (uint64_t)cycles * (uint64_t)p->ticks_per_cpu_cycle;
ivs_psg_catch_up(bus_ctx.psg, &im->psg, im->mem.psg_reg, &im->obs.audio, im->time_ticks, sink);
```

**Differs:** lines come from `ivs_stic_lines(params, &im->stic, start - 4 x bus.line_sample_lag_cpu_cycles)` (clamped at 0, T-2 item 4); after the PSG catch-up add the STIC catch-up to `im->time_ticks` (fixed order PSG then STIC, V6-22) with frame completions going to the sink; `ivs_run_power_on` (L44-77) resets the STIC state and the register file next to the PSG block (L53-61) and initialises the video chain h0 next to `ivs_input_init` (L74-76); the two gate opens at L62-63 become the STIC reset's job. The step seam used by twin tests (`ivs_machine_step_unit_internal`, L110-115) builds a counting sink: it must now also build the counting video sink, or run-driven and seam-stepped instances diverge ([X]).

### A5. `core/src/memmap/memsys.{h,c}`, `cmake/IvsMemmapTable.cmake` (device kind `stic14`)

**Analog:** the Phase 5 `psg8` flip, in the same files: kind list (`memsys.h` L48-56, `IvsMemmapTable.cmake` L155-156 `set(MM_DEVICES ... psg8)`, `MM_DEVICE_MACROS`), backing capacity (`MM_CAP_psg`, L171, `_Static_assert` at L494), `ivs_mm_load/store` switch cases (`memsys.c` L128-179), pure accessors (L409-419), poke seam (L430-447), layout comment and size assert (`memsys.h` L18-24, L113).

**Accessors to add** (cards R-5, pure, no gate, no fill; same style as `ivs_memsys_psg_reg_internal`): `ivs_memsys_stic_backtab_word_internal(const ivs_memsys *, i)` and `ivs_memsys_stic_gfx_byte_internal(const ivs_memsys *, sel, card, y)` reading `sysram` and `grom`/`gram` backing directly. The renderer must never use `ivs_memsys_read` (closed `gmem` gate would return the undriven fill: fault `GFX_READ_VIA_CPU_VIEW`).

**Differs (hazard 5):** the generator rule `if(dname STREQUAL "stub") ... NOT pname IN_LIST MM_STUB_PHYS` / `else ... pname IN_LIST MM_STUB_PHYS` (`IvsMemmapTable.cmake` L478-486) is per physical device. The register note keeps the read alias `$0040-$007F` a stub while `stic_regs` and the three write aliases go real, all with phys `stic` (the name up to the first `_`). Phase 5 emptied `MM_STUB_PHYS` of `psg`; for `stic` the rule itself must change (or the read alias gets a distinct phys name). `ivs_memsys` layout also changes (`stic_reg[64]` = 128 bytes plus alignment), so `IVS_MACHINE_BYTES`, the static asserts and the digest table move in the same plan.

### A6. `core/src/machine/machine_bound.c` (flag, stop FRAME, capacity, report v3)

**Analog:** the same file. Copy, do not redesign:
- ABI pins: `_Static_assert` block L87-101 (v1 report 112, `offsetof` of each appended field, `sizeof`); add the v3 lines, keep the v1 constant (A-02).
- Flag test L399: `(req->flags & ~IVS_RUN_F_STOP_ON_HALT) != 0u` becomes `~(STOP_ON_HALT | STOP_ON_FRAME)`.
- Capacity: `ivs_bound_fits` L177-199 is `bytes_per_frame * (max(target - now, 0) / frame + 1)`; D-15 replaces the video term by `video_slot_bytes x (D/F + 2)` with `video_slack_frames` carried in the plain `ivs_run_env` seam struct (`machine.h` L250-263, "callers build it with memset, so an appended field is 0"); audio and log keep their formulas bit-identical.
- Pre-unit never-expected guard (copy L574-581): before a unit that could complete a frame, if a requested video sink cannot hold one more slot, `internal = 1u; break;` (stop `IVS_STOP_INTERNAL`); named fault `VIDEO_CAP_PLUS_ONE`.
- Exit-state order: `ivs_bound_stop` L459-474 gains FRAME between CPU_HALTED and TARGET (INTERNAL, GUEST_FAULT, CPU_HALTED, FRAME, TARGET); `ivs_run_stop_name` L686-700 gains `IVS_STOP_FRAME`.
- Report write (copy L636-668): `n = ivs_bound_rep_bytes(rep)` first, local `r`, `memcpy(rep, &r, n)` last.

**Differs:** with the flag, the loop runs to `T' = min(target, E_next)` while `r.overshoot_ticks` (L644) must stay computed against the CALLER's target (D-14): `ivs_bound_plan` (L119-124) needs both. A flagged call completes at most one frame and needs one slot. Report v3 appends `video_first_index`, `last_frame_end_ticks`, `next_frame_end_ticks` at offset 136 (sizeof 160 on every ABI: the report is pointer-free); `video_frames_completed` and `video_bytes_written` (already in v1, `ivs_run.h` L267-268) become live.

### A7. `core/src/machine/machine.{h,c}` (layout, env, info v4, table coverage)

**Analog:** `machine.h` L12-32 (layout derivation comment), L91-120 (obs, struct, size asserts), `machine.c` L339-372 (`ivs_machine_profile_env`) and L274-337 (`ivs_machine_info`).

**Table coverage** (WR-04, `machine.c` L50-53; copy it for every new generated per-profile table):

```c
return (uint32_t)(index < ivs_profile_count() && index < ivs_psg_params_count() &&
                  index < ivs_input_profile_count());
```

Add `index < ivs_stic_params_count()`, the same guard in `ivs_memsys_init` (`memsys.c` L74), a `STIC_TABLE_SHORT` fault and a line in `data/expected/core-generated.ids`.

**Differs:** `ivs_machine_profile_env` today sets `video_format NONE`, `video_bytes_per_frame 0` (L346-347); Phase 6 sets `IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1` and the slot bytes (15,296 = 32-byte header + 159 x 96) from the generated geometry. Info v4 appends at offset 288 (after `crystal_hz_den`, `ivs_machine.h` L151-175): `frame_phase_ticks`, video width, height, slot bytes, header bytes, frame spacing; keep `IVS_INFO_V1_SIZE` 216. The "nominal" comments at `ivs_machine.h` L117-124 and `time-base.md` L32 become "exact for this profile" (D-07).

### A8. `core/src/machine/machine_digest.c` (STIC rows, composite v3, video row)

**Analog:** the same file. A fault-gated block of rows is the Phase 5 idiom (L55-62, L93-108):

```c
#ifdef IVS_FAULT_DIGEST_PSG_ROWS_OMITTED
#define IVS_DF_PSG_ROWS(...)
#else
#define IVS_DF_PSG_ROWS(...) __VA_ARGS__
#endif
```

**Copy:** `stic_reg[64]` joins MEM (domain 2) beside `mem.psg_reg`; `ivs_stic_state` rows (with PAD rows for reserved words) join INSTANCE (domain 4); composite constants: `IVS_DIGEST_COMPOSITE_VERSION` 2 -> 3 (`machine.h` L141), fault `DIGEST_COMPOSITE_VERSION_STALE` style (L326-328). The composite hashes domains 1..4 only (L355-366) so `d[4][32]` does not change.

**Differs:** domain 16's rows already exist but its chain starts at `h0 = SHA-256("IVS-VIDEO-LOG-V1")` (D-16), exactly like the INPUT chain (`machine_input.c` L95-100, called from `machine.c` L251 and `machine_run.c` L74-76), NOT 32 zero bytes like the audio and psglog chains; so the video row needs an init function at create and reset. The chain is `h(n+1) = SHA-256(h(n) || slot bytes)` over the exact delivered slot bytes, streamed through the incremental `ivs_sha256_init/update/final` (`ivs_sha256.h` L36-44, as `ivs_dsink` does, `machine_digest.c` L142-169), not the `IVSR` per-record framing of `psg_log.c`. Test literals: `machine_digest.c` L113-158 (fresh PSG tail and PSGLOG preimage literals), L412-418 (fresh composite), `data/expected/digest-v1-literals.ids` is the pin-first pattern: pin the v2 literals in a new file BEFORE the bump.

### A9. `core/src/machine/machine_inspect.c` and the new public const call (VID-06)

**Analog:** `ivs_machine_cpu_state` (L45-75): const handle, `ivs_machine_from_handle` then `BAD_HANDLE`, overlap check, report built in a local, `*out = r` last; plus the A-02 write rule of `ivs_machine_info`.

**Differs:** `ivs_machine_stic_state(const ivs_machine *m, ivs_stic_report *out)` is versioned (`struct_size` first, minimum a pinned v1 constant), returns stored registers (gate independent), mode, display latch, color-stack state, window-open flags at machine time `now`, frame and blocked counters; no time, no write, valid OFF and POWERED. It must not share a function with `ivs_machine_peek` (the gate-ignoring stored view is exactly the named fault `PEEK_GATE_IGNORED` for peek, `machine_inspect.c` L97-110). `peek` keeps its Phase 4 contract: pure, bus view, no side effect, because every effect lives in `machine_bus.c`. Positive control: a guest CPU read of `$0021` inside the window DOES switch the mode; a peek of the same address does not (`stic.peek.cpu-read-switches`).

### A10. Public headers and `data/api/api-effects.json`

**Analog:** `ivs_run.h` L91-106 (flags, stop reasons), L213-270 (request, report); `ivs_machine.h` L104-175 (formats, info), L61-69 (domains), L310-327 (digest doc incl. the domain-17 framing documented for hosts); `api-effects.json` L328-354 (the `ivs_machine_cpu_state` row, copy for `ivs_machine_stic_state`) and L389-451 (run row).

**Edits:** `IVS_RUN_F_STOP_ON_FRAME 2u` replaces `IVS_RUN_F_RESERVED_FRAME` (L92); `IVS_STOP_FRAME 5u` (one line in `data/expected/api-stop-reasons.ids`, checked by `DEFINE_CLASSES` `IVS_STOP_`, `api-effects.cmake` L67-72); `IVS_OUTPUT_FORMAT_INDEX8_FRAMES_V1 2u` after `PSG_LEVELS_V1 1u` (L111; no pin file exists for formats, add a define class only if wanted); frame header and pixel constants (`IVS_FRAME_HEADER_BYTES 32u`, `IVS_PIXEL_BLANK 0xFFu`, `IVS_PIXEL_COLOR_MAX 15u`) next to the format list; document the domain-16 chain beside domain 17 (L310-326) so a host can recompute it from delivered slots. One-line `IVS_API` prototypes only (the checker matches `^[ \t]*IVS_API([ \t]|$)`), constants `#define ...u`, no enums or bitfields. No new refusal reasons are needed (frame note F-7: the existing 41..43 cover video). The `unknown or reserved flag (IVS_RUN_F_RESERVED_FRAME until Phase 6)` text at `api-effects.json` L396 must change with the flag.

### B1. Profile fields, `stic.*` generator, `profile-fields.ids`

**Analog:** field records `memmap.device.psg8` (profile L351-360), `memmap.region.stic_regs` (L371-380), `psg.regmap` (L518) and the generator `cmake/IvsPsgTable.cmake` (header L1-62 documents every field read and the fail-closed list; helpers `pt_fail/pt_type/pt_keys/pt_str/pt_uint` L67-120; copy and rename the prefix).

**Rules inherited:** a record has `value tag cite conflict resolving_evidence revisit_trigger decision note`; `conflict` is ONE id matching `^C[0-9]+$` that already exists in `data/discrepancies.json` (put others in `note`), so the allocation table of plan 06-01 precedes every profile edit; `tag` from `data/tiers.json`, never `tier5-emulator-hint`; `unknown-default-chosen` needs `resolving_evidence` and `revisit_trigger`; a field whose last segment holds `window offset hold duration delay period latency interval timeout` must end `_ticks` or `_cpu_cycles` (`docs/time-base.md` L74-77), so the CONTEXT names `stic.reg_window_cpu_cycles`, `stic.irq_low_cpu_cycles`, `stic.bus.pitch_cpu_cycles` are right and a bare `...window` is not. Every new path goes in `data/expected/profile-fields.ids` (an exact set, currently 110 lines). Generator refusals from timing T-11 (`first_start + (count-1) x pitch + hold <= frame cycles`, `pitch >= hold`, `reg_window <= gmem_window <= first_start`, `release_latency x tpc <= psg.max_unit_ticks`, `video_slack_frames` equal to the U < F implication, geometry `width <= grid.w`) are the new failure list; add `-DMUTATE` (copy `IvsCtrlTable.cmake` tail) and a `GEN_STIC_MUTATION` fault only if the plan budget allows (prefer a judge).

**Differs:** the C10 reversal is one field, `video.last_card_column_px` 7 -> 8 (D-12); the generator DERIVES width, stride, plane and slot bytes and checks them against `data/video/geometry.literal.txt` (V6-18: a generator-side refusal when they disagree).

### B2. Memmap literal flip and the `x=strobe` exception

**Analog:** `data/memmap/intv-2609-ntsc-family-p1.literal.txt` header L1-33 (row grammar, `x=reads-zero` and `x=pins` declared exception rows, tier and independence rule) and the split psg rows L58-69. The exception class is hard-coded in THREE places that must move together: `tools/check/memmap.cmake` L12-24 and L310-315 (`x=(reads-zero|pins)`), `cmake/IvsMemmapProbes.cmake` L12 and L29 (the row regex), `tests/memmap/memmap_rows.c` L305-324 (`mr_is_exception(addr)` names `$01FA`, `$01FE`, `$01FF` literally). Add `x=strobe` for `$0020`/`$0021` (no value stored, the access has a machine-level effect) to all three plus the literal header, then split the stic rows by width (`R 0000-0017 ... w=0 x=reads-zero`, collision `0018-001F`, color stack `0028-002C`, delay `0030-0032`, reserved rows) as Phase 5 did for psg.

**Also moves:** `tests/memmap/memmap_main.c` L81-93 (five rows declared `IVS_MEMSYS_DEVICE_STUB`), `memmap_rows.c` L282 (`{"stic", 0u, 0x0000u, 0xFFFFu, 0u}`), `tests/run/run_observe.c` L567-577 (peek of `$0000` expected as a stub), `memmap-dump.txt` dev/release compare, and CL-0006, which depends on `memmap.region.stic_regs`, `memmap.region.stic_alias_*` and `memmap.device.stub` (`data/claims.json`): editing those fields touches a claim.

### B3. `data/stic/*.regmap.literal.txt`, rules, vectors, pending inventories

**Analog:** `data/psg/intv-2609-ntsc-family-p1.regmap.literal.txt` (header L1-25 states tier, the independence rule "written from the sources only, never generated from a profile field", row grammar `<ADDR> role=... w=<HHHH> r=<HHHH> src=<SRC-id>@<locator> tag=... conf=Cnn`), `data/psg/rules.json` (`PSG-Rnn`: id, statement, basis, cite, fields, vectors, conflict, note with `resolver:` and `trigger:` text), `tools/check/schema/psg-rule.txt` + `tools/check/kinds/psg-rule.cmake` L1-60 (the hook), `tools/check/steps/psg-vectors.sh` + `tools/check/psg-vectors.cmake` L1-75 (tokens, caps, `pfail` capping, pending lists).

**Copy:** the three-file record idiom (schema, kind hook, inventory `data/expected/stic-rules.ids`), the vector grammar `V <id> rule=... kind=<source|policy> tier=... cite=... set=... steps=... expect=... derive=...` (`data/psg/vectors/regs.txt` L1-40; policy rows name an id of `data/expected/stic-policies.ids`, never evidence), the pending bridge (`data/psg/fields-pending.ids`: a rule may name a profile field that a later plan adds; a stale pending line fails `psg-field-pending-stale`; plan 06-04..06 delete their lines in the same commit), and the vector-registration bridge (`vectors-pending.ids`). Rule `basis` vocabulary: copy the PSG shape with the STIC labels of regs section 4 (`tier1-literal`, `tier1-derived`, `tier2-bsr`, `tier4-community`, `default-unknown`).

**Differs:** most STIC rows are tier 4 or `default-unknown` (D-01: the printed pages give structure only), so the `video` step's token `*-rule-unknown-default-unresolved` will carry the weight; the page-reading verdict block at the top of `docs/stic/reference-notes.md` (V6-1 keys such as `registers-printed`, `window-lengths-printed`) is checked by the step; copy Phase 5's `equations-printed: no` precedent in `docs/psg/reference-notes.md`. Vector text must stay emulator-name free (`tools/check/vector-emulator-names.txt`); read vectors with `file(STRINGS)` and regex, not `string(JSON)` (the Phase 4 vector step cost about 5 cpu-s, V-16).

### B4. `data/video/*` (geometry, card bit roles, frames, vectors)

**Analog (partial):** the independence discipline of `data/input/intv-2609-ntsc-family-p1.literal.txt` and `cmake/IvsInputProbes.cmake` L1-25 (a probe reads ONLY the literal; the expected byte of a test never comes from the profile or the generated table); the hand-derivation discipline of `data/golden/diag-ctrl-psg-pass.expect` L1-60 (derivation committed before the program ran, excluded-on-purpose paragraph, shared-misreading caveat) and `docs/input/diag-ctrl-psg-1-derivation.md`.

**Copy:** row grammar `<variant> <bit> <role> src=<SRC-id>@<locator> tag=tier4-community conf=Cnn` for the card-word literal (cards R-8, 4 variants x 16 bits = 64 single-bit probes, the memmap width-probe pattern); fixture and vector grammar from cards section 3; `data/expected/video-frames.ids` and `video-vector-files.ids` as exact sets. A `video` checker with caps and capped tokens (frame wrong dimension, bad symbol, missing tier header, popcount conservation, hash tie, unregistered frame) copies `psg-vectors.cmake`'s `pfail` and `CAP 20`.

**Differs:** there is no analog for a `.frame` text file. Decide ONE grammar in plan 06-01: recommended is the cards/frame-note shape, 20 cards wide (160 symbols per row, `-` for the cut-off pixel) so the C10 one-field reversal never touches the literals, with the crop applied by the generator from the profile; the verify note's 159-digit line is the published-width alternative. Whichever is chosen, the hash tie compares `cmake -E sha256sum` of the canonical ASCII form (a file the step writes, spaces removed; cmake cannot write a zero byte) with the hash in the scenario's `.expect` and the C test's generated include. Per-card pixel-count conservation (popcount of the card pattern equals the number of foreground pixels) is the cheap non-circular guard; fixtures must use distinct fg/bg indices per card.

### B5. Records and docs (discrepancies, claims, D10, sources, time base)

**Analog:** C77..C80 in `data/discrepancies.json` (status `open|partial|resolved-primary|withdrawn`; `open`/`partial` need `default_policy` starting `unknown-default-chosen: `; ids strictly ascending; next free id is C81); CL-0014/CL-0015 in `data/claims.json` (keys `id statement profile tier artifact depends_on standing note`, `artifact` an existing tracked file, `depends_on` ids that exist in `profile-fields.ids`, `standing: provisional` while any dependency is weak); `data/expected/d10-measurements.ids` (`<id> | <Cnn> | <what the owner records> | <anchor>`, existing groups `D10-SYS`, `D10-INP`, `D10-PSG`, `D11-EXEC-01..02`; keep free of semicolons and brackets; a `D10-` line must name an open or partial discrepancy, every open/partial discrepancy with a `measurement:` resolver needs a line, `kinds/discrepancy.cmake` L80-114).

**Differs:** all five notes allocate `C81..` and `D10-VID-01..` independently (regs C81..C89, timing C81..C86, cards CV-1..CV-9, frame C-new-1..); plan 06-01 owns ONE allocation table and every later plan cites it. D11 next free is `D11-EXEC-03`. `docs/time-base.md`: replace L72-73 ("never derived by modulo, because stalls ... change how many CPU cycles a frame holds") and amend L31-32 ("a nominal figure and not a frame event") BEFORE any STIC code (D-07); keep the term **Frame** (the glossary step requires it, `data/expected/glossary-terms.ids`).

### C1. Gate step `video` (and sibling checkers)

**Analog:** `tools/check/steps/psg-vectors.sh` (wrapper: run `records.cmake -DKIND=<kind>` once per rule kind, run the vector checker, re-print one pass line with the step's name and `rstats vstats`), `tools/check/steps/input.sh` (24 lines, single checker), `data/expected/gate-steps/{input,psg-vectors}.ids` (content: the step name; the union of these files is the step inventory, `tools/gate.sh` L86-99).

**Copy:** wrapper shell, `CAP 20` capped-token function, `gate_incr/gate_count/gate_fail`, closed token list at the top of the `.cmake`. The V6-12 cap is ONE new gate step: either wrap the stic-rule, video-rule, vectors and frames checkers in `video.sh` the way `psg-vectors.sh` wraps two, or accept more steps with the budget consequence (each step costs wall time in `step-time` lines).

**Differs:** every new pure step needs toy inputs in `tools/check/controls/_good/tree` (toy `data/stic/rules.json`, vectors, `data/video/frames/toy.frame`, `video-frames.ids`, listings with SDBD) or the positive control fails; each control directory copies the whole tree, so toys stay tiny (about 316 control dirs exist). Budget: at most 25 controls and 8 faults per plan, prefer `<suite>.judge.*` ids over faults for oracle-table checks (`important-gates.sh` counts `.judge.` and `.mutant.` ids as mutants).

### C2. `tools/check/listing.cmake` SDBD rule

**Analog:** the branch-target rule that Phase 5 added in the same file: token list L60-62, header comment L20 ("an SDBD prefix is not modelled: such a listing fails closed"), the per-line operand logic L407-440 (`pending` operand counter from `R_WORDS`), the post-pass recomputation L460-510 with `lfail`.

**Differs:** an `SDBD` row (`data/cp1610/opcode-families.json` row id `SDBD`, words 1) must be followed by an instruction that takes the doubled operand; that instruction's `words` becomes `1 + 2 x operand` for an immediate (low byte then high byte decle, each at most `00FF`), the DBD effect does not carry past it, and the decode order must equal `ivs_pack --dump-words` (`tests/pack-tap.sh` compares the checker's `-DDUMP=<id>` output with ivs_pack byte for byte, two independent parsers). New token `listing-sdbd` with 3 controls (`data/expected/controls/listing.ids`), grammar sentence in `docs/diagnostics.md` L11-30, and update the "uses no SDBD" statements (`docs/diagnostics.md` L95, L121) only for the new diagnostic. `data/cp1610/vectors/sdbd.json` already encodes the rule independently.

### D1. New suites `stic` and `video`: plumbing

**Analog:** the `psg` suite (`tests/CMakeLists.txt` L194-248) and its fault registration (L369-372); the `input` suite for a small suite (L255-261, L374-377).

```cmake
add_executable(ivs_tests_psg ${IVS_PSG_TEST_SOURCES} ${IVS_PSG_VEC_GENERATED})
target_include_directories(ivs_tests_psg PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/machine" "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(ivs_tests_psg PRIVATE ivs_test_harness ivs_core ivs_core_internal ivs_warnings ivs_san)
add_test(NAME psg COMMAND ivs_tests_psg)
set_tests_properties(psg PROPERTIES SKIP_RETURN_CODE 77)
...
ivs_fault_suite(psg SOURCES ${IVS_PSG_TEST_SOURCES}
  INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/machine" "${CMAKE_CURRENT_BINARY_DIR}" GENERATED ${IVS_PSG_VEC_GENERATED})
```

**Copy:** one source list variable used by BOTH `add_executable` and `ivs_fault_suite` (this is how `IVS_PSG_TEST_SOURCES` avoids the triple registration slip); section-table `main` with caps (`psg_main.c` L14-148: `PSG_MAX_CASES 640u`, `PSG_MAX_SECTIONS`, `PSG_MAX_SOURCES`, `psg_add(fixed, n_fixed, n_vec, vec_id, vec_run, sources)`), `# source <path> <sha256>` lines for every data file (`tests-inventory.sh` re-hashes), `data/expected/tests/<suite>.ids` (ctest name = ids stem = TAP header suite, else `tests-ctest-drift`), `faults/<suite>.ids` whose first line is `# suite=<name>; columns: <FAULT_ID> <must-fail> <allowed-prefixes> <must-pass>`. Stand-in guests: `tests/machine/machine_standin.c` L31-123 (`ms_prog_*` word tables, `IVS_MS_PROGS 9u` in `machine_cases.h` L78, `ivs_ms_powered_words` L339 builds an EXEC from raw words) - at most 24 words each, every word derived in a comment from `data/cp1610/opcode-families.json`. `tests/machine/machine_standin.c` is compiled into every suite that uses it.

**Differs:** the video suite needs the frame-literal include generated at build time (`cmake/IvsVideoFrames.cmake`, modelled on `IvsPsgVectors.cmake`: reads ONE sourced text file family, writes a C include plus a `ivs_*_sources[]` array with the SHA-256 of each source file for the TAP header; never reads the profile, core headers or core output) and the hash tie. Perturbation tests (windows, hold, geometry) copy `psg_profile.c` L1-35: build a perturbed stack copy of the params struct and run the module-level functions; there is no registry seam, so a machine-level perturbation needs a hand-built bus ctx.

### D2. Families and their analogs (test shapes)

| Family (ids from V6-6/V6-7/V6-13) | Analog file and shape |
|---|---|
| `stic.reg.*`, `stic.alias.*`, `stic.blocked.*`, `stic.enable.seq.*` (vector-driven) | `tests/psg/psg_reg.c` + `data/psg/vectors/regs.txt` (generated vector table, 4-column read/write rows) |
| `stic.window.*`, `stic.irq.*`, `stic.steal.*`, `stic.tile.*` (boundary by observed start ticks) | `tests/input/input_slice.c` and the C67 boundary pattern (`input.boundary.start-minus-1|start|start-plus-1`: unit starts OBSERVED by single stepping `run(now + 1)`, never predicted from the cycle table) |
| `stic.slice.*`, `video.slice.*` (one call, 1, 7, prime, seeded) | `tests/psg/psg_slice.c` L1-30 (header table of ids) and `ivs_run_slice_halting` (`run_cases.h` L99) |
| `stic.bounds.*` (U 468, g 28, `memsys-bus-never-waits` through the FULL machine with grants live) | `tests/psg/psg_bounds.c` L1-40 (all 1024 decles, SDBD on and off, waits 0/1/35); its header says "a Phase 6 stall fails this id loudly" |
| `video.buffers.*`, `video.noop.null-buffer` (NULL + 0 equals with-buffer, sentinel tail, refusal writes nothing) | `tests/psg/psg_buffers.c` L1-30, fault `AUDIO_STATE_DEPENDS_ON_BUFFER` |
| `video.chain.*` (hand-framed literal) | `tests/psg/psg_chain.c` L1-30: POSIX `printf` octal escapes + `cmake -E sha256sum` recipe in the header (cmake cannot carry NUL); the simplest independent first literal is the display-off first frame (plane all one index, `head -c` and `tr`, F-15) |
| `stic.fuzz.seed01..08`, `video.fuzz.*` | `tests/psg/psg_fuzz.c` L1-40 (fixed xorshift32 seeds, reference = one unit per `run(now + 1)`, non-vacuity case) |
| `stic.iso.*` (two interleaved instances) | `tests/psg/psg_iso.c` L1-12 |
| `video.fgbg.*`, `video.cstack.*`, `video.card.*`, `video.meta.*` | no test analog; shape from cards section 3 grammar + 64-probe sweep (R-8) + four metamorphic properties (R-17, guards not evidence) |
| `run.diag.stic.*` (guest end to end) | `tests/run/run_diag_ctrl.c` L1-45: expected values come from `data/golden/<id>.expect` through `tests/expect-inc.cmake` (X-macro include), machine time never asserted except bounds |

**Existing suites that change** (cite when planning; see hazards 2, 3, 4): `run_bound.c`, `run_input.c`, `run_buffers.c` (flag), `run_struct.c`, `run_info.c`, `run_buffers.c` L282/L334/L547/L862 (`video_frames_completed == 0` assertions), `run_observe.c` L567-577, `memmap_main.c`, `memmap_rows.c`, `machine_digest.c`, `machine_create.c` (`tables-cover-registry`), `machine_standin.c`.

### D3. Named faults and registration

**Analog:** `data/expected/faults/psg.ids` header L1-20 (one comment block per fault, allowed prefixes `|`-separated and covering EVERY collateral failure, must-pass id), `data/expected/faults/input.ids`, `tests/faults.cmake` L79-93 (`ivs_fault_tu_setup`: owner TU compiled with the define, plain copies of the rest), `ivs_fault_gen` for generator faults (`tests/CMakeLists.txt` L384-388 `GEN_CTRL_MUTATION`).

**Copy:** a fault is registered in the SAME commit as the test that detects it (`fault-orphan-site`); ids `[A-Z0-9_]+`, unique across all files, exactly one `IVS_FAULT_<ID>` site in exactly one core or runner TU of the suite's modules; the faulted TAP must list exactly the suite's ids, fail the must-fail id, fail only ids with an allowed prefix and pass the must-pass id. The V6-13 list (28 faults) is the inventory; the card note's F-01..F-19 map onto the `VIDEO_*` ids there (use CONTEXT's: wrong card mode, stack not advancing, bit order reversed, GROM/GRAM swapped, 20th column width). Faults in `runner/exec.c` (`RUNNER_VIDEO_ASSERT_IGNORED`) need `MODULES core runner` on their suite (`tests/CMakeLists.txt` L449-454 `ivs_fault_suite(runner ... MODULES core runner)`).

### E1. Runner: parser, executor, chain re-hash, report

**Scenario parser** (`runner/scenario.c`): `parse_digest_domain` L514-535 is a plain `tok_is` ladder (video already parses); `assert_psglog` L562-620 is the template for `assert frame <n> pixel <x> <y> ==|!= <0..15|blank>` and frame hash and chain forms; `dir_capture` L654-675 (bit per target, `DUPLICATE_CAPTURE`) for `capture video <first> <count>`; the capture-needed check `scn_check_capture` L931-941; reason names `ivs_scenario_reason_name` L1182-1191 (`IVS_SCN_R_COUNT` is 50 after `IVS_SCN_R_ASSERT_WITHOUT_CAPTURE 49`, `ivs_runner.h` L204-210). New subjects append after `IVS_SCN_SUBJ_AUDIO 7u` (L137); every new reason needs a refuse case in `tests/runner/runner_parse.c` (`runner.parse.reasons-named` fails a reason without one). `ivs_runner.h` L89 states "`at_frame=` is refused until Phase 6": CONTEXT D-17 does not enable it; decide explicitly (default: keep refused).

**Executor** (`runner/exec.c`): `ex_plan_buffers` L694-737 cuts the host block into three channels sized `bytes_per_frame x (slice_ticks / nominal_frame_ticks + 1)` (L696). With video live this is the corner D-15 proves wrong: video needs `slot_bytes x (slice/F + 2)`; copy the formula from the core (published through `ivs_info`, not hard-coded) or EVERY scenario will be refused `run-refused ... BUFFER_TOO_SMALL`. Chain check: copy `ex_audio_chain_check` L933-953 (`NONE` when not every slice delivered, `PASS`/`FAIL`, harness-fail detail `video-chain`) and the per-slice feed at L843-905; capture: `ex_capture_log` L328-345 and `ex_capture_audio` L438-460 (overflow is a harness failure `capture-overflow target=...`, never a silent drop; fault `RUNNER_CAPTURE_OVERFLOW_SILENT`); a capture or frame assert without a video buffer is `stream-without-buffer target=video` (L779-792 pattern, never a vacuous pass). `IVS_RUNNER_BUFFERS_MAX` is 8 MiB (`ivs_runner.h` L330). Capture storage is carved after the read buffer (`exec.c` L1009-1030; `IVS_RUNNER_CAPTURE_BYTES`, `ivs_runner.h` L329, holds only 4,096 log records and 4,096 audio entries): captured frames (at most 16, about 245 KB) must extend that block, because the library allocates nothing.

**Chain re-hash** (`runner/runner_vchain.{c,h}`, new): copy `runner_chain.{h,c}` (pure struct, hand-written `be64/be32` stores, runner-local SHA-256 `runner_sha256.c`). It must implement the CONTEXT D-16 chain (h0 = SHA-256 of the tag, `h(n+1) = SHA-256(h(n) || slot)`), not the `IVSR` audio framing, and check header consistency (index continuity, width and height equal to `ivs_info`, reserved bytes zero, pixels 0..15 or `0xFF`, ticks strictly increasing). The plane hash for `assert frame` hash forms is a runner-side value, not a core domain (Reconciliation).

**Report** (`runner/report.c`): closed key table in fixed order; `RF_*` ids L35-68, head table L75-79, tail table L84-135; new keys go after the audio keys (D-17: `output.video.frames`, `video.geometry`, `video.chain.check`, ...) and `ivsr 3` is ONE bump (L35, `ivs_runner.h` comment, all `ivsr 2` literals listed in hazard 8).

**Operator output** (`runner/main.c`, `hostio.{c,h}`, `ivs_runner.h` vtable L360-392): copy the post-review audio path, not the pre-review one (05-REVIEW WR-01 and WR-02): exclusive create `ivs_hostio_out_create`, byte cap, files removed with `ivs_hostio_out_remove_if_same` whenever the run ends harness-fail (device and inode compare), `--update-golden` refuses an output path (`ivs_hostio_out_is_path`); give the library a vtable hook beside `write_audio_raw` so the in-process suite fakes it; golden writer key list in THREE places that must agree: `main.c` L350-445 (`main_golden_keep`, video already a live row with three spare slots), `tools/check/steps/golden.sh` L1-60 and L264-268, `docs/runner.md` L205-216; `.refpcm`-style special extensions are NOT accepted golden files (golden.sh L140-141).

### E2. `tools/frame/ivs_framepreview.{h,c}` (operator PPM export)

**Analog:** `tools/pcm/` exactly: `CMakeLists.txt` (explicit source list, repo-relative list written to `build/<preset>/ivs-pcm-sources.txt` for `c-build`, internal STATIC library, no install rule), pure integer module (no I/O, allocation, floats, globals), linked only into the runner library and its own tests.

```cmake
add_library(ivs_refpcm STATIC ${IVS_PCM_SOURCES})
target_include_directories(ivs_refpcm PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(ivs_refpcm PRIVATE ivs_warnings ivs_san)
```

**Differs and moves:** the places learned from `tools/pcm` (hazard 9). The preview palette is a project-authored arbitrary 16-colour ramp labelled "operator preview palette, not STIC colour data" (no palette from another emulator); PPM not PNG (no zlib, no CRC code); the file is exclusive-create, removed on a failed run, refused when equal to another output path; never in a golden, claim or CI upload.

### F1. Fixture `FX-diag-stic-1`, scenarios, mutants, goldens

**Analog:** `fixtures/FX-diag-ctrl-psg-1/{diag-ctrl-psg-1.listing.txt (L1-50 header grammar and derivation comments), manifest.json}`, `data/scenarios/diag-ctrl-psg-pass.ivss` (comment block stating outcome, `ivss 1`, `content exec boot-stub`, `content grom zero-grom`, `content cart <file>.ivsc`, `input at_tick=...`, `run-to ... ticks stop halted`, asserts), `data/expected/runner-scenarios.ids` (six columns), `data/golden/diag-ctrl-psg-pass.expect` L1-60 (tier `original-diagnostic`, oracle `independent-derivation`, derivation blocks, excluded-on-purpose) and `.digests` (tier `deterministic-repeatability`, `# domains:` line, body written only by `ivs-run --update-golden`).

**Mutants:** `data/expected/diag-mutations-ctrl-psg.ids` is the form to copy (`<ID> <addr> <from> <to> <clause> <outcome> <exit>`, explicit `from` word checked against `ivs_pack --dump-words`, one negative mutant that must still pass, a loop-capable mutant is `timeout`, never `guest-fail`). `tests/runner-process-tap.sh` L383-415 is the generalized block (`run_mutant "$mid" "$listing2" diag-ctrl-psg-1.ivsc diag-ctrl-psg-pass "$addr" "$to" ...`): add a third block, a fourth image to the content-dir copy (L92-102) and keep slices for ONE scenario (cost cap V6-12: scenario at most 150,000 ticks, slice-1 and slice-7 replays for `diag-stic-fgbg-pass` only, L416).

**Manifest and image build:** copy the manifest (`kind original-source`, `redistributable yes`, `allowed_use [local-test, ci, repo]`, `rights_review` author-attested, outputs `sha256` + `size_bytes` refreshed in the commit that edits the listing; helper `tools/cart/manifest-outputs.sh`); register in `data/expected/fixtures.ids`, `listings.ids` (`ivs-diag-stic-1 cart <path>`), `tests/pack.ids` (`pack.words-<id>`, `pack.trailer-<id>`), a fourth image in `tools/pack/CMakeLists.txt` (L33-50 pattern) and the `ivs_pack --c-include` pair list in `tests/CMakeLists.txt` L147-154 (`diag_stic_1=${IVS_DIAG_STIC_1_IVSC}`). Images are build-tree only (`*.ivsc` is ignored).

**Differs (hazard 10):** the program must be derived against the frame timeline BEFORE it exists (S_0 = 0, STIC window `[0, 8,000)` ticks, GRAM window `[0, 15,120)`, first row fetch at 3,791 cycles; `access_gating_when_off = apply` means the windows close even with the display off); mode is selected by an input event at tick 0 reading `$01FE` (one image, one branch); `SDBD; MVII` pairs need the listing SDBD rule first (06-01).

### F2. Installed consumer and `pkg-install`

**Analog:** `tests/installed-consumer/main.c`: header L1-31 (the contract), `v1_sizes()` L574-595 (pins `sizeof(ivs_run_report) == 136u`, `sizeof(ivs_info) >= IVS_INFO_V1_SIZE`, request 80), the Phase 5 second replay `ctrl_session` L456-552 with the `ctrl.` line prefix, `C_DOMAINS 9u` L44 (video already index 5); `pkg-install.sh` L466-473 (`check_runner_golden` compares `digest.<domain>` lines and the observed-domain set with a golden, one `check_one_golden "$rgold2" "ctrl\\." ...` per replay), L505-524 (image paths and `"$cbld/consumer" "$img_exec" "$img_cart" "$img_cart2"`), self-tests L700-724 (`ctrl-domain-missing`); pinned stdout `data/expected/pkg-consumer.golden` (L10 `v1-sizes ...` line).

**Edits:** third replay with a third prefix (for example `stic.`) sized from `ivs_info` (video buffer = slot bytes x (slice / F + `video_slack_frames`), never hard-coded), `digest.video` plus first-frame header and a few pixels; the `v1-sizes` line and its pin become report=160 and info=v4 size; a third comparison in `check_runner_golden`; a fourth image argument; new self-test cases register in `data/expected/pkg-selftests.ids` and in `important-gates.ids` as `mutant`. `pkg-consumer-calls.ids` changes ONLY if the consumer calls `ivs_machine_stic_state` (Phase 5 left it unchanged because no function was added). The consumer keeps no SHA-256 (copy-paste rule: chain recompute stays in the runner). Keep the header rule "one call per statement when one argument reads state another argument's call changes" (L31).

### F3. `tests/faults.cmake` `-g0` lever (plan 06-02)

**Analog:** `ivs_fault_tu_setup` L79-93 (the place a per-fault-object compile option goes) and `ivs_fault_suite_obj`; measurement protocol: five alternating container runs before and after as `04-LEVER-02` did; `tools/ci-record.sh` and `.planning/ci-runs.tsv` (orchestrator only, never an executor); `CI-EFFICIENCY.md` entry shape (rows, rule in force, checkpoints). Budgets stay gate median 75 / trigger 90 and cpu median 260 / trigger 280; add the normalised `cpu_s / build_s` beside them. Evidence build unchanged: only fault executables get `-g0`.

## Shared Patterns

### SP1. Fault registry (owner TU, inventory line, verdict)
**Source:** `tests/faults.cmake` L1-33, L79-93, L138-152; `data/expected/faults/psg.ids` L1-20; `tools/check/steps/faults-verdict.sh` (site scan `^(core/src|runner)/`).
One `IVS_FAULT_<ID>` site in exactly one TU; the id in exactly one line of one `faults/*.ids` file with a `# suite=` header registered by `ivs_fault_suite`; a fault build runs the WHOLE suite once, so small suites and small TUs bound the cost (about 0.15-0.25 cpu-s per fault locally). Generator faults are `GEN_*` ids registered by `ivs_fault_gen` (no source site). Prefer a `<suite>.judge.*` self-control (about 0.003 cpu-s) over a fault when the check is on an oracle table or a checker.

### SP2. Exact inventories
**Source:** `data/expected/**.ids`, `tools/check/steps/tests-inventory.sh`, `tools/check/steps/important-gates.sh` L21-96.
`#` comment lines, first whitespace token = id, extra columns step-specific. Inventories are exact sets, never subsets. A skip is visible (`ok N <id> # SKIP <reason>`), counted, and legal only for a `skip-ok` id. `important-gates.sh` hard-codes the requirement list in its awk (`nreq = split("API-02 ... AUD-02", reqs, " ")`) and its header comment: Phase 6 adds `VID-01 VID-02 VID-03 VID-04 VID-06` there and in `important-gates.ids`, one covered gate per requirement.

### SP3. Controls
**Source:** `tools/check/run-controls.sh`, `tools/check/controls/<token>/{CHECK,EXPECT,overlay/}`, `tools/check/controls/_good/tree`.
A control is a negative test of a CHECKER step: `CHECK` (step name), `EXPECT` (fixed-string tokens), optional `FORBID`, `overlay/` copied over a fresh copy of `_good/tree`, optional `MAKE` verbs; every `CHECK` step must PASS on the untouched toy tree (positive control). Steps that need a build (`pkg-install`, `c-faults`) cannot be control-driven (in-step self-tests instead).

### SP4. TAP harness
**Source:** `tests/harness/ivs_test.h`, `tests/psg/psg_main.c` L14-148, `tests/headers-tap.sh`.
`# suite=<s> nonce=<IVS_TEST_NONCE|none>`, `# source <path> <sha256>`, `1..N`, `ok N <id>` / `not ok N <id>` / `ok N <id> # SKIP <reason>`, trailer `# suite=<s> executed=<n> skipped=<k> failed=<f>`; exit 0 pass, 1 fail, 77 all skipped; every `add_test` has `SKIP_RETURN_CODE 77`.

### SP5. Generator + independent literal probe
**Source:** `cmake/IvsMemmapTable.cmake` (generator), `IvsPsgTable.cmake` (field reader, helpers L67-120), `IvsMemmapProbes.cmake` and `IvsInputProbes.cmake` L1-25 (probe from literal only), `IvsPsgVectors.cmake` L1-50 (vector text to C include), `IvsPsgDacIndependence.cmake` (build-time proof that two profiles differing in a field the core must not read give byte-identical params), `IvsShaVectors.cmake`.
Generated files are build output, never committed, listed in `data/expected/core-generated.ids` (`<basename> <generator path>`, generator accepts only `-DROOT -DOUT`) when a core TU includes them; the literal's header states tier, independence rule and recipe, and is NEVER derived from the profile.

### SP6. Listing checker and pack parity
**Source:** `tools/check/listing.cmake` L1-62, `tools/pack/pack.c`, `tests/pack-tap.sh`, `docs/diagnostics.md` L11-30.
Grammar `ADDR HEX4 BITS ROW ASM`, branch lines carry `-> ADDR` (checked since Phase 5), contiguous addresses inside the window, header keys `id kind profile window label entry stack`, endings need a follower after `HLT`. `ivs_pack` is the only decoder to image bytes.

### SP7. api-effects rows
**Source:** `data/api/api-effects.json` L328-354 (`ivs_machine_cpu_state` row), L389-451 (run row); `tools/check/api-effects.cmake` L55-85.
A new call or reason is a data edit plus test ids; one refusal row per status; every precedence status appears in the refusals and vice versa; every cited test id is a first token of some `data/expected/tests/*.ids` and not all `skip-ok`; refusal effects come from the `effects` registry.

### SP8. Digest field tables
**Source:** `core/src/machine/machine_digest.c` L65-125, `tests/machine/machine_digest.c`.
Every new instance member gets rows in the same plan; coverage and sensitivity sweeps iterate `ivs_machine_digest_field_count()`; `IVS_DIGEST_KIND_PAD` rows are declared but never serialized; a flipped row in an observation domain must change ONLY that domain, not the composite.

### SP9. Fail-closed generators and parsers
**Source:** `IvsMemmapTable.cmake` header, `IvsPsgTable.cmake` header L48-62, `core/src/content/ivsc.c`, `runner/scenario.c`.
An unknown key, a missing required field, a value outside its range and a count mismatch are errors with a context string `<Gen>: <ctx>: <reason>`; pure function of input; hand-written ASCII classification (no `<ctype.h>`), no `strtol`/`sscanf`.

### SP10. Catch-up wrapper [E]
**Source:** `machine_bus.c` L31-65, `machine_run.c` L182-185, `psg_sample.c` L109-136.
Stamp = unit start tick + access offset in CPU cycles x ticks per CPU cycle + a profile strobe offset; catch up to the stamp (events `<= stamp` apply first, windows half-open: an access at the close tick is blocked), apply the access, post-effects; catch up to the unit end after every unit in the seam and the run call alike; fixed order PSG then STIC.

### SP11. Output sinks with a counting sink [Z]
**Source:** `psg.h` L91-105, `psg_sample.c` L90-103 (`if (sink->buf != NULL) { if (cap - written >= N) {...} else { overflow = 1u; } }`), `machine_digest.c` L142-169 (`ivs_dsink`), `machine_bound.c` L529-543 (sink over the caller buffer or the counting sink).
Always count and chain; write only when a buffer is present; `overflow` is never expected (the capacity rule and the pre-unit guard prove the room). Appending video fields to the same `ivs_psg_sink` is the low-churn option (only `tests/psg/*` call the sink seams); a new umbrella struct changes three seam signatures (`machine.h` L229-242) and 8 test call sites.

### SP12. Append-only ABI [A] and refusal oracle
**Source:** `machine_bound.c` L87-101, L155-168, L636-668; `tests/run/run_struct.c` L266, L296-308; `tests/run/run_cases.h` L104-117 (`ivs_run_snap_take/same`).
A refusal changes nothing: whole-storage memcmp equal, digests and machine time equal, poisoned (`0xA5`) out structs untouched except listed detail fields. A v1-sized and a v2-sized caller of the report and of `ivs_info` must be tested separately from a rebuilt caller (extend `run.struct.*`).

### SP13. Oracle-first process and blind second reads
**Source:** `docs/psg/second-reader-protocol.md`, `second-read-request.md`, `second-read-record-regs.md`; `docs/input/second-reader-protocol.md`; `tools/review/vector-blind.cmake` (JSON/CP-1610 only).
Reader sees only a blinded copy (`build/review/blind/<name>.txt`: grammar header and inputs without expected values) plus sources in the gitignored `refs/`; writes one derived file; records say `source-expectation, second-read`, never `independent-implementation-comparison`; disagreement is resolved from the source and counted. No generic text blinding tool exists (the one committed tool projects CP-1610 JSON): frame and card vectors need either a new small projector or an orchestrator-made copy as in Phase 5.

### SP14. Golden re-bless with a classified diff
**Source:** `data/expected/digest-v1-literals.ids`, `data/golden/*.digests`, `golden.sh`.
Pin the old literals FIRST (new `digest-v2-literals.ids`, recorded from the unmodified tree), bump, re-record with `ivs-run --update-golden`, and write the classification in the plan SUMMARY (and where cheap as a comment in the golden header): composite, instance, mem and video lines may move; `digest.cpu`, `digest.images`, every `step.*`, `slot.*`, `cpu.*`, `now_ticks`, units, stop reasons, audio and psglog chains must stay byte-identical ("phase 6 does not move any phase 5 timing" is a named acceptance test).

## No Analog Found

| File | Role | Data Flow | Reason / fallback |
|---|---|---|---|
| `core/src/stic/stic_card.c` pixel plane generation (BACKTAB word formats, GROM/GRAM select, 8x8 bit order, 7-pixel column, Color Stack advance) | service | transform | Nothing renders. Spec is `06-DISCUSS-cards.md` R-1..R-18 and CR-01..CR-17; TU shape from `psg_mix.c` (43 lines) and the pure accessors of `memsys.c` L409-419. |
| `data/video/frames/*.frame` text literal and its hash tie | literal | batch | No committed text frame exists; use the cards/frame-note grammar and hazard 9; independence discipline from `data/golden/diag-ctrl-psg-pass.expect`. |
| BACKTAB row latches (13 x 20 words) fetched per bus window | service | event-driven | First device state sourced from system RAM at a scheduled instant; T-3/T-7 in the timing note define it. |
| Stamp-ordered STIC timeline table (events per frame, same-tick order) | service | event-driven | Closest is the PSG absolute grid; the table is new (generated by `IvsSticTable.cmake` from window fields). |
| Frame-stop flag (`IVS_RUN_F_STOP_ON_FRAME`, `T' = min(target, E_next)`, report v3) | service | request-response | `IVS_RUN_F_STOP_ON_HALT` (`machine_bound.c` L399, L459-474) is the shape; the clamp and caller-target overshoot are new. |
| Generic text blinding projector for frame and card vectors | tool | batch | `tools/review/vector-blind.cmake` reads CP-1610 JSON only. |
| PPM writer | tool | transform | `tools/pcm/ivs_refpcm.c` is the shape (WAV header + PCM); the format is new. |
| `x=strobe` memmap exception | checker | batch | Extends `x=reads-zero|pins` (three hard-coded places, B2). |

## Hazards (ranked; each is a thing the planner can get wrong)

1. **Twin oracle and catch-up completeness [X].** Run-driven and seam-stepped instances are compared by whole-storage `memcmp` (`run_cases.h` L104-117, `run_fuzz.c`, `run_slice.c`, `machine_reset.c`, `psg_slice.c`). If the STIC catch-up or the video sink exists only in `ivs_machine_run_env_internal`, the seam twin never completes frames or flips gates and every twin comparison fails. Put the lines drive, the catch-up to the unit end and the counting video sink in the seam (`machine_run.c` L117-188). The hook set (reads, fetches, the BACKTAB span, the `$4021/$8021/$C021` effect aliases) must be complete over all 65,536 addresses x 3 views; a missed hook is a silent slicing-dependence. Decide this in plan 06-04/05 before writing any STIC test.
2. **ABI size pins (A-02).** Concrete places that break or must be extended: `machine_bound.c` L89-101 (v1 report 112, v2 pins 136), `tests/run/run_struct.c` L266 and L296-308 (`sizeof(ivs_run_report) == 136u`), `tests/installed-consumer/main.c` L574-595 and `data/expected/pkg-consumer.golden` L10 (printed `run_report=136 info=288`), `machine_create.c` L631-666, `run_info.c`. Report v3: `video_first_index`, `last_frame_end_ticks`, `next_frame_end_ticks` at 136/144/152, `sizeof` 160 (pointer-free, so the same on every ABI); info v4 at offset 288; the request stays 80. Keep `IVS_RUN_REPORT_V1_SIZE` 112 and `IVS_INFO_V1_SIZE` 216; extend the v1-sized-caller tests to the v2/v3-sized report and v2/v3-sized info caller (fault `RUN_REPORT_TAIL_UNGUARDED` already guards the whole report; add one for the STIC report struct). A struct rebuilt from the new header passes the larger size and hides every violation: only dedicated v1/v2/v3-sized-caller tests catch it.
3. **The reserved flag is the vehicle of seven Phase 4/5 refusal tests.** `IVS_RUN_F_RESERVED_FRAME` is used to force UNSUPPORTED at `run_input.c` L876, `run_buffers.c` L757, `run_bound.c` L949, L951, L1085, L1109, L1146, plus `api-effects.json` L396, the `machine_bound.c` L17 comment and `ivs_run.h` L92. Flipping flag 2 to a real flag turns all seven into frame-stop runs. In the SAME commit replace them with another unknown bit (for example `4u`) and invert, not delete, the dedicated reserved-flag case (a reviewed change). Decide whether 06-03 (behaviour-neutral) accepts the flag at all: a flag whose clamp needs `E_next` has no meaning before the timeline lands in 06-05; the safest cut is define and pin in 06-03, accept in 06-06 with its tests (stop exactly at the event, slicing invariance, overshoot against the caller target, flag with STOP_ON_HALT, flag with a pending input event).
4. **Digest and golden churn, and what is NOT free.** `mem.gate_open` is a MEM row (`machine_digest.c` L82): the STIC flips it at every window edge of every frame (D-05 applies the windows even with the display off), so `digest.mem` and `digest.composite` move in EVERY golden even for guests that never touch the STIC, and depend on the stop time. The video chain starts at `h0 = SHA-256("IVS-VIDEO-LOG-V1")` (not zero bytes), the runner now requests a video buffer for every scenario (`exec.c` L694-737 once `video_bytes_per_frame` is nonzero), so every golden gains `output.video.bytes`, `digest.video` and `# domains: ... video=observed`, and `golden.sh` L264-268 and `main_golden_keep` need the new video keys. Also moves: `machine.digest.*` literals (`machine_digest.c` L113-158, L412-418), `digest-v1-literals.ids` successor, `IVS_DIGEST_COMPOSITE_VERSION` (`machine.h` L141), fault `DIGEST_COMPOSITE_VERSION_STALE`, five goldens plus `pkg-consumer.golden`. Classify the diff per SP14 and prove the unit sequence is identical for guests without EIS or a display strobe.
5. **Generated-table coverage and the stub-vs-real rule.** (a) Every new generated per-profile table must join `ivs_machine_profile_covered` (`machine.c` L50-53), the `ivs_memsys_init` guard (`memsys.c` L74), `core-generated.ids`, the `ivs_core_gen` dependency list (`core/CMakeLists.txt` L82; `tests/faults.cmake` waits only on `IVS_CORE_GEN_TARGET`), a `*_params_count()` with a `..._TABLE_SHORT` fault and `machine.create.tables-cover-registry` (05-REVIEW WR-04: a missing row must be a loud refusal at create, never a silently absent device). (b) `IvsMemmapTable.cmake` L478-486 allows a stub device only on a phys in `MM_STUB_PHYS` and a backed device only on a phys NOT in it: a real `stic_regs` plus a stub `stic_alias_read` (same phys) cannot both pass. (c) The `x=strobe` class must be added to `memmap.cmake`, `IvsMemmapProbes.cmake` and `memmap_rows.c` together, and the memmap tests that name the STIC rows as stubs (`memmap_main.c` L81-93, `memmap_rows.c` L282, `run_observe.c` L567-577) flip with the literal.
6. **Video capacity is `+2`, and the runner buffer plan still says `+1`.** D-15 replaces the video term of `ivs_bound_fits` (`machine_bound.c` L177-199) by `slot_bytes x (D/F + 2)`; audio and log keep their formulas. `ex_plan_buffers` (`exec.c` L694-737) sizes every channel `bytes x (slice/F + 1)` and would refuse every scenario BUFFER_TOO_SMALL; the installed consumer sizes its own buffer the same way. The signature of `ivs_bound_fits` needs the slack parameter; `video_slack_frames` rides in `ivs_run_env` (plain struct, appended). Corner test: `now = E_k - 4`, `D = F - 1` completes two frames (61 of 400,000 runs violate `D/F + 1`); fault `VIDEO_CAP_PLUS_ONE` must fail it. `run_buffers.c` L282, L334, L547, L862 assert `video_frames_completed == 0` against a layout that is NONE today and flip with it.
7. **Hygiene: an '@' with a dotted word after it is an email.** `tools/hygiene.sh` EMAIL regex flags `<letters-or-digits>.<letters>` after an at sign; a locator like the cards note's `<SRC id>@<page name>.oldid-<n>` (a plain word, a dot, then `oldid`) would fail the gate in `.planning/`, docs, data and any tracked file. Phase 5 used `PSG-oldid-14940` (no dot); `Memory_Map.oldid-14847` passes only because of the underscore. Write locators with dashes (`STIC-oldid-15008`), never a dot after a plain label. The grammar `[A-Za-z0-9._,-]` allows the dot, so only the hygiene step catches it. Also never write home or scratch paths, tilde paths or credential-shaped assignments into any tracked file (use `<repo-root>`).
8. **`ivsr 3` and the key lists are one-shot.** The literal `ivsr 2` lives in `runner/report.c` (L4, `RF_IVSR`), `runner/ivs_runner.h`, `docs/runner.md` (L90, L96, L192), `tests/runner-process-tap.sh` (L10, L204), `tests/runner/runner_exec.c` (L360-367, L978, L1057, L1076, the independent copy of the key table), seven control files under `tools/check/controls` (`_good/tree/build/gate/{dev,release}/runner/toy*.report`, `golden-dev-release-diff`, `golden-digest-psglog-missing`), `data/claims.json` note text and test-id comments. The golden key list exists in `main.c`, `golden.sh` and `docs/runner.md`. Decide the whole new key list in plan 06-09 together with the parser reasons (`IVS_SCN_R_COUNT` after 49, a refuse case each), the controls and the goldens.
9. **A new operator tool is a ten-place module change.** `tools/frame` copies the `tools/pcm` wiring and each place has a control: `c-build.sh` L41-45 (fifth source list and `c-build-unlisted-frame-source`), `integrity.cmake` L74 (SPDX regex gains `frame`, control `integrity-missing-spdx-tools-frame`), `runner-scan.sh` L17-19 and L149 (the quoted header name is allowed, every other form stays `runner-scan-header`), `tests/faults.cmake` L84-87 (include dir for runner TUs) and `runner/CMakeLists.txt` (link), `core-symbols-deny.txt` L52-55 plus `core-symbols.sh` L156 (a `palette|ppm|framepreview` defined-symbol rule with control `core-symbols-palette-defined`, the one symbol-layer control, V6-12 caps it at one), `binary-ext.txt` and `.gitignore` (`ppm`, as `wav` was added), `pkg-install-files.ids` (proves the prefix has none). The 05 hazard stands: the fault framework knows two modules (`core`, `runner`); put any fault inside runner TUs or the helper is outside the fault budget.
10. **The diagnostic must be derived against the timeline, and the SDBD rule must exist first.** Frame 0 starts at tick 0 (`frame_phase_ticks 0`), STIC registers are writable for `4 x 2000` ticks and GRAM/GROM for `4 x 3780` ticks from each frame start whatever the display state, the display latch is cleared at every frame start, and row i is latched at `3791 + 912 i` cycles. A 240-word BACKTAB fill is system RAM (no window) but costs several thousand cycles; the `$0020` strobe and the color-stack writes must land inside a window and BEFORE the rows they affect are latched. Write the cycle-by-cycle derivation (`docs/video/diag-stic-1-derivation.md`) in 06-01 before the listing; the listing needs `SDBD; MVII` for 14-bit words, the checker does not model SDBD (it fails closed) until `listing-sdbd` lands, and `ivs_pack --dump-words` must agree with the checker on operand counts. D4 effort data (words, branch count, errors caught by checker versus by a run) goes in the 06-10 SUMMARY; the mini-assembler crossing rule is in V6-10.
11. **Chain framing differs from audio and psglog; the independent oracle needs a different recipe.** D-16: per frame `h(n+1) = SHA-256(h(n) || slot bytes)` over exactly the delivered slot (header plus plane), streamed, h0 the tag hash. Do not copy `psg_sample.c` L38-65 or `psg_log.c` L53-87 framing (`IVSR || domain || chain || be64(index) || be32(len) || record`) and do not reuse the Phase 4 per-call `index || len || record` form. A 15,296-byte slot cannot be authored with `printf` octal escapes alone; use `head -c` and `tr` for the plane and `printf` for the 32-byte header, then `cmake -E sha256sum` (recipe header in `tests/psg/psg_chain.c` L1-30). A slicing-invariance test of the chain and the runner's `video.chain.check` recompute are mandatory.
12. **Bounds are re-proved through the full machine, not assumed.** `psg.bounds.*` probes the bare stepper; `U = 468` and `g = 28` rest on the memory system answering wait 0 (`psg_bounds.c` header: `memsys-bus-never-waits`, "a Phase 6 stall fails this id loudly"). The STIC stall is a sequence of 1-cycle grant/hold units and a 2-cycle release (T-3), never a wait; `stic.bounds.g-and-u` runs an adversarial writer with the display on, grants and VBLANK entries live. The generator gains a second refusal (`release latency x tpc` and the 9-cycle interrupt entry at most `psg.max_unit_ticks`); update the stale "no device raises an interrupt before Phase 6" text in the `psg.min_write_gap_ticks` profile note and the `psg_bounds.c` header (comment only). C41 stays 0, so a non-DIS-first ISR re-enters about 250 times per frame: every Phase 6 stand-in ISR is DIS-first and a vector family covers both C41 values.
13. **gcc-only and hosted-only failure classes (all four happened in Phase 5).** Argument evaluation order in test helpers and table loops (05-05: `ivs_ms_powered_prog` passed `n` and `&n` in one statement chain; clang hid it, gcc 13 read 0); `-Wmaybe-uninitialized` on a variable read after a `goto done` (05-08, `psg.bounds.g-and-u`; clang silent); `-Wformat-truncation` on a runner `snprintf` into a narrower detail buffer (05-09, `exec.c`; new tokens such as `video-chain` and `stream-without-buffer target=video` lengthen details); `-Wconversion` on 8- and 14-bit card fields promoted to `int` (state in `uint32_t`, explicit masks); GNU `nm` shapes; `%llu` casts for `uint64_t`; the dash `runner-process` `hung.ready` flake (05-03). Run `sh tools/gate-linux.sh` before pushing every plan that adds C (06-03 onward) and `sh` plus `dash` on every plan.
14. **Perturbation needs a seam the registry does not give.** `ivs_profile_get` and the generated tables are const static data; `psg_profile.c` perturbs a stack copy of `ivs_psg_params` at MODULE level. Every STIC window, hold, row-fetch and geometry rule that D-02 says is "proven by profile perturbation" therefore needs module functions taking `const ivs_stic_params *` (windows as pure functions of `(params, state, stamp)`), or a hand-built bus ctx at machine level. A hard-coded constant in an engine TU is caught only by its perturbation test (`PSG_NOISE_TAP_HARDCODED` precedent).
15. **Suite plumbing is triplicated and capped.** ctest name, `data/expected/tests/<suite>.ids` stem and TAP header suite must match (`tests-ctest-drift`); every test `.c` in the shared source variable used by both `add_executable` and `ivs_fault_suite`; `faults/<suite>.ids` first line `# suite=<name>`; caps `PSG_MAX_CASES 640`-style constants per suite `main`, `IVS_RUN_MAX_CASES 256`, `IVS_MACHINE_MAX_CASES 512`, `MAX_SOURCES`; `data/expected/tests/*.ids` additions for existing suites. 28 faults and 25 controls for the phase (D-21), at most 8 faults and 25 controls per plan; a plan above 120 new ids per preset or 8 faults splits.
16. **Process, not a gate.** 06-01 contains no C and writes every oracle; each later SUMMARY names the oracle commit that precedes the implementation commit. An expected frame produced by a run instead of derived by hand defeats the evidence plan; the `.expect` assert lines must equal the scenario's in order (`golden-expect-drift`); a fault site and its registration travel together.
17. **Records collide across notes.** One allocation table for `Cnn`, `CL-nnnn` and `D10-VID-nn` ids in 06-01; `d10-measurements.ids` format is strict (no semicolons or brackets, anchor in an existing doc); `ticks_per_stic_clock` and the C8 note gain a tier-1 data point (RA-3-9600 prints a 1.79545 MHz input, regs S-7) without a profile change; C10 goes `partial` (not resolved: no primary pixel source), C1-C4 stay `unknown-default-chosen`.
18. **Docs trip three text gates.** Under `README.md` and `docs/**`: a line with `supports|compatible|passes|accurate|verified|runs` needs `[claim:CL-NNNN]` or `[nc]` (`tools/check/steps/text.sh`); a bare `clock` needs `input clock`, `STIC clock`, `PSG chip clock` or `[nc]` (`tools/check/steps/glossary.sh`); a named emulator project needs `[cite-only]` or is banned (`tools/check/steps/denylist.sh`). The new `docs/stic/*` and `docs/video/*` are the likely offenders. `.planning/` is exempt from `text` and `denylist` but not from `hygiene`.
19. **Cost.** Hosted last-5 medians are over the old triggers (gate 106 s, cpu 336); plan 06-02 applies `-g0` to fault objects only (about 5 hosted cpu-s) and records the normalised `cpu_s / build_s`; checkpoints CP-A after 06-04, CP-B after 06-06, CP-C after 06-09, CP-D at close; stop rule `r > 8.3` creates 06-L1 (orchestrator-owned; executors never call `gh` or `tools/ci-record.sh`). Prefer judges to faults, line formats to `string(JSON)`, small TUs to big ones.

### Move-together table (counts and inventories that must change in the same plan)

| Change | Must move together |
|---|---|
| new core TU | `core/CMakeLists.txt` `IVS_CORE_SOURCES` only (fault copies, `c-build` list and symbol checks follow); stem unique across `core/` and `runner/` |
| new generated core include | generator, `data/expected/core-generated.ids`, `ivs_core_gen` dependency list, `ivs_machine_profile_covered`, `ivs_memsys_init` guard, `*_TABLE_SHORT` fault + `machine.create.tables-cover-registry`, optional `GEN_*` fault, `-DROOT -DOUT` interface only |
| new device kind `stic14` | `memsys.h` kinds, `IvsMemmapTable.cmake` lists and stub rule, `memmap.device.stic14` profile field + `profile-fields.ids`, `ivs_mm_load/store`, memmap literal, `x=strobe` in three places, memmap tests, CL-0006 inputs |
| new profile field | profile JSON, `profile-fields.ids`, allocated discrepancy id (must exist), time-unit suffix rule, `cite` SRC id in `sources.ids`, rule `fields` entry or pending line, claim `depends_on` |
| new/changed public struct field | header comment with offsets, pinned v1/v2 constants, `machine_bound.c` L87-101 asserts, `run.struct.*` pins, consumer `v1_sizes`, `pkg-consumer.golden` L10, `docs/runner.md` |
| flag `STOP_ON_FRAME` | `ivs_run.h`, `machine_bound.c` flag test and stop order, `api-stop-reasons.ids`, `api-effects.json` L396 text, the seven reserved-flag test sites, `ivs_run_stop_name`, runner stop token if the runner ever sets it |
| new public call `ivs_machine_stic_state` | `ivs_run.h` or `ivs_machine.h` prototype, `api-effects.json` row, `tests/headers-tap.sh`, tests, `pkg-consumer-calls.ids` iff consumer calls it |
| composite v3 + domain 16 live | `machine.h` version define, field table rows, `machine_digest.c` composite and video init, `machine.digest.*` literals, v2 pin file written first, `digest.*` lines of 5 goldens + `pkg-consumer.golden`, `golden.sh`/`main.c`/`docs/runner.md` video keys, runner `video.chain.check`, `docs/time-base.md` if it lists domains |
| video capacity `+2` | `ivs_bound_fits`, `ivs_run_env.video_slack_frames`, `ivs_machine_profile_env`, `ex_plan_buffers`, consumer buffer sizing, `run_buffers.c` expectations, `VIDEO_CAP_PLUS_ONE` fault + corner test |
| `ivsr 3` | `report.c`, `runner_exec.c` (independent key table), `ivs_runner.h` comment, `docs/runner.md`, `runner-process-tap.sh`, seven control files, all goldens |
| new IVSS subject or reason | `ivs_runner.h`, `scenario.c` (parser, reason name table), `runner_parse.c` refuse case, `docs/runner.md` |
| new gate step | `steps/<name>.sh`, checker `.cmake`, `data/expected/gate-steps/<name>.ids`, controls + `data/expected/controls/<group>.ids`, `_good/tree` toy data, important-gates row |
| new suite | `tests/CMakeLists.txt` (executable, `add_test`, `ivs_fault_suite`), `data/expected/tests/<suite>.ids`, `faults/<suite>.ids` (`# suite=`), important-gates rows, suite `main` caps |
| new named fault | one site, one `faults/*.ids` line, allowed prefixes covering collateral failures, registered with the detecting test, important-gates row if it guards a requirement |
| new fixture (fourth image) | `manifest.json` outputs hashes, `fixtures.ids`, `listings.ids`, `data/expected/tests/pack.ids`, `tools/pack/CMakeLists.txt`, `tests/CMakeLists.txt` image include, `runner-scenarios.ids`, goldens, mutants file, runner-process-tap third block, consumer arg, `pkg-install.sh` image check, claim |
| new discrepancy / D10 item | `discrepancies.json` (ascending), `discrepancies.ids`, `d10-measurements.ids` when the resolver is `measurement:` (+ anchor), `docs/stic/d10-procedure.md` |
| new frame literal | `data/video/frames/<id>.frame`, `video-frames.ids`, derivation in `docs/video/frame-derivations.md`, hash in the `.expect` assert and the generated test include, blind second-read record |
| listing SDBD rule | `listing.cmake` token + loop, 3 controls in `data/expected/controls/listing.ids`, `docs/diagnostics.md`, `pack-tap.sh` parity, the new listing |
| `tools/frame` | the places of hazard 9 |

## Metadata

**Analog search scope:** `core/` (all TUs and headers), `cmake/`, `runner/`, `tools/` (steps, check, kinds, schema, controls, pack, pcm, review, ci-record, listen-demo), `tests/` (psg, input, run, machine, memmap, runner, harness, installed-consumer, shell TAP, CMake), `data/` (profiles, memmap, psg, input, api, expected, golden, scenarios, records), `fixtures/`, `docs/`, `.planning/{AUTONOMY,CI-EFFICIENCY}.md`, `AGENTS.md`, Phase 5 PATTERNS, SUMMARYs and REVIEW, and the six Phase 6 notes.
**Files read for excerpts:** about 75 (core headers and TUs, generators, memsys, digest, bound, runner headers and sources, report writer, golden step, listing checker, important-gates, psg/input/run/machine test headers, faults and tests CMake, consumer, pkg-install, fixtures, goldens, scenarios, records).
**Not run:** no build, no gate, no `gh`, no network, no git writes. Statements about gate behavior come from reading the scripts and sources at the base commit; the one structural risk found by reading and not by running is hazard 5(b).
**Pattern extraction date:** 2026-11-12
