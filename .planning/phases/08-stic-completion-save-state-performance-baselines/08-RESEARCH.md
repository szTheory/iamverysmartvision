# Phase 8: STIC Completion, Save State and Performance Baselines - Research

**Researched:** 2026-11-14
**Domain:** project-internal (C17 core, sh/CMake gate steps, runner grammar, release plumbing); no external package.
**Confidence:** HIGH for repo facts (read this session); MEDIUM for cost figures (carried from CI-EFFICIENCY, not re-measured); the hardware semantics are tier 4 by decision D-01.
**Labels:** [OBS] read or measured in the repo this session (path and quoted value where a discrete value matters); [INF] inference; [REC] recommendation; [UNK] open. The four `08-DISCUSS-*.md` notes hold the web and primary-source research and are cited, not repeated. One scratch measurement was run outside the tree (a 120-frame scenario); nothing in the repo was built, changed or committed.

<user_constraints>
## User Constraints (from CONTEXT.md)

Size deviation: 16 KB of decisions are not re-copied. Authoritative text: `.planning/phases/08-stic-completion-save-state-performance-baselines/08-CONTEXT.md` (D-01..D-37, conflict resolutions, hard stops); the planner must read it. Digest only, no alternatives researched:

D-01..D-10 STIC: tier 4 only, split derivable core versus pinned `unknown-default-chosen` remainder, keep 159x96 with a labelled even-line sampling default, one program `ivs-diag-stic-2` with a controller-word selector and five single-feature scenarios plus an idle control, oracle first with blind second read, collision bits ORed into the register file at FRAME_END (no hidden state, the `const ivs_memsys *` of `ivs_stic_frame_deliver` may change), no bus-window change, guests mask collision reads with `03FF`, five D10 hand-off lines, CL-0025..27 provisional, wave A at most 6 plans, 4 core faults, 6 mutants (release only), 4 controls.
D-11..D-20 STA: the digest field table and walker become the one serializer (class in the unused `reserved` u16: STATE, IMAGE_BEARING, BOUND, PAD), big-endian, container IVSN v1 (7 sections, SHA-256 trailer, never image bytes), `ivs/ivs_state.h` with four calls and `IVS_E_STATE_MISMATCH` (-24), validate before mutate, table-independent oracles (poisoned target memcmp, stream concatenation, nine digests, runtime row-omission sweep, `state-checklist.ids`), hard-point witness scan, `ivs-run --roundtrip-at-ticks` plus a fifth consumer pass, `.ivsn` binary-ext, STATE waves after VID-05.
D-21..D-26 PRF: `ivs-bench` in `tools/bench/`, digest before timing, `bench-*` scenarios with `.digests` goldens, TSV with three separate groups, never gate on throughput, negative control, no optimization.
D-27..D-35 REL: `08-VERIFICATION.md` plus `08-MILESTONE-CLOSE.md` A-K, two-stage REL-01, publish `v0.1.0` pre-release as a draft gated by P1-P8, three version numbers, `git.create_tag` footgun, release machinery in wave 1, history accepted without rewrite (six shas pinned), stale-text sweep, summary frontmatter backfill.
D-36 about 22 plans (19-25), waves D1, A, B, C, D2/D3; split trigger wave A over 9 plans or a `lever-required` checkpoint. D-37 no new job, matrix or cache.

### Claude's Discretion
Everything CONTEXT does not fix: plan cuts and order inside a wave, ids, file names, exact counts, the cut tick set, the hook semantics inside the runner, where each new check lives.

### Deferred Ideas (OUT OF SCOPE)
192-line video publication; snapshot file options only as the optional last plan of wave B; libFuzzer runs in CI; SHA-256 frame-hash optimization; D10, D11, history rewrite, name clearance.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| VID-05 | MOB, border, delay, collision each with an independently derived single-feature fixture or on the `unknown-default-chosen` list | seam table 1, hazards H3-H6, plans A1-A6, validation V-VID |
| STA-01 | one canonical serializer for snapshots and digests; version, profile and image identity; validate before mutate | table 2, hazards H7-H9, plans B1-B4 |
| STA-02 | snapshot, restore, run equals uninterrupted run at hard points; written hidden-state checklist covered by tests | 2.4, plans B1, B5, B6, B7 |
| PRF-01 | named workloads, digest asserted before throughput, three separate reports | 3.3, plans C1-C3 |
| REL-01 | milestone close record, main green, triage, lessons, release readiness for the exact commit | section 4, plans D1-1, D1-2, D2, D3 |
</phase_requirements>

## Summary

The core is in better shape for this phase than the discuss notes assumed, and the gate machinery is in worse shape. Better: VID-05 needs no hidden state [INF]: collision bits live in `mem.stic_reg[0x18..0x1F]` (class `collision`, write mask and read mask `03FF`), which the digest table already serializes, and every existing guest leaves the MOB, delay and extension registers at zero, so all 15 committed goldens are a free tripwire for the renderer. The field table has 88 rows, not about 95 (table 2.1); 51 are STATE rows and the whole snapshot is about 4,299 bytes [INF, computed from the table]. The executor recreates the instance on every `ivs_runner_execute` call [OBS exec.c:1259], so a bench iteration is a clean machine, and a 120-frame idle scenario took 0.33 s total in the dev ASan build with a 60-line `.digests` golden and no `.expect` [OBS, scratch run].

Worse: seven gate checks hard-wire assumptions that Phase 8 breaks, and each breaks at a commit the plan must anticipate (section 7): the status band -24..-31 is rejected by `api-effects.cmake`; `important-gates.sh` rejects VID-05 by name and a control relies on that; `video.cmake` cannot accept a frame with `-` cells or MOB pixels; `ci-parity.sh` forbids every construct `release.yml` needs; `text.sh` does not scan `CHANGELOG.md`; `c-build.sh` has no `tools/bench` inventory; `pkg-install.sh` pins the header list. None needs a new job; most need a control retarget in the same commit.

**Primary recommendation:** cut 21 plans (19 required, 2 optional; a 22nd by splitting B3) in the order D1, A, B, C, D2/D3 of section 6; make every plan register its ids, pending-line deletions, goldens, fault sites and control retargets in the commit that causes them; add no new CI job, step-for-step only where section 3 says so.

## Architectural Responsibility Map

| Capability | Primary tier | Secondary | Rationale |
|---|---|---|---|
| MOB, border, delay, collision pixels and bits | core `stic` (pure renderer, local accumulators) | register file (`mem.stic_reg`) | machine behaviour, host independent |
| Snapshot bytes, restore validation | core `machine` (walker beside the digest) | public header `ivs_state.h` | one serializer; the core does no file IO |
| Round-trip hook, snapshot files | runner library (hook), `ivs-run` (files, optional) | installed consumer (independent replay) | persistence is host policy (D-19) |
| Timing and throughput | `tools/bench` only | `ivs_runner_lib` called in process | clock stays out of core and runner |
| Release assets, tag, notes | `.github/workflows/release.yml` plus `tools/` scripts | gate checks (`pkg-install`, `text`, `ci-parity`) | the gate proves the artifact; the workflow ships it |
| Close record, stage 2 facts | `.planning/` records | orchestrator (`gh`, read-only) | executors never call `gh` |

## Standard Stack

No library or package is added; `ivs-bench` is two C17 files; no hyperfine (D-21; the binary exists on this laptop [OBS `command -v`], nothing may depend on it). Toolchain: C17 with `CMAKE_C_EXTENSIONS OFF`, CMake floor 3.25 (local 4.4.3, hosted 3.28.3), Ninja 1.13.2, Apple clang 21 locally and gcc 13 hosted, POSIX sh and dash for every script [OBS]. **Package Legitimacy Audit:** not applicable, nothing is installed; no `[ASSUMED]` package name.

## Architecture Patterns

### Flow: frame completion with VID-05 and the serializer sharing the table

```
guest stores -> memsys (stic_reg, BACKTAB, GRAM) --+
INTRM/BUSRQ lines <- stic_lines (pure of state,tick)|
ivs_stic_catch_up (event cursor) -- ROW_FETCH --> row_latch[12][20]   (STATE, already a table row)
   | FRAME_END at E_k
   v
ivs_stic_frame_deliver(p, s, mem (non-const), row, sink, E_k)
   cards row r (pure) --> [on-pixel mask row] --> MOB pass (regs, GRAM, local mask) --> delay/extension placement
   --> local collision mask (u32 x 8 MOBs) --- OR into mem.stic_reg[0x18+i] after the last row (gate bypassed, direct store)
   --> SHA-256 chain + slot copy (unchanged framing)           [display-off frame: border fill, no MOB, no collision]

machine_digest.c: ivs_digest_fields[88] --walker(hash sink)--> 9 digests          (unchanged bytes)
                                        \-walker(buffer sink, class==STATE)--> IVSN sections --> + header + SHA-256 trailer
restore: decode into locals -> validators -> invariants -> (no failure past here) -> memset non-image region -> stores
```

### Recommended structure
`core/src/stic/stic_mob.c` (new pure renderer and on-mask function), `stic_frame.c` (row streaming, deliver non-const), `core/src/machine/machine_snapshot.c` (new; walker use, save, restore, describe), `core/include/ivs/ivs_state.h` (new), `tests/video/video_{mob,collision,delay}.c`, `tests/state/*.c` (new suite `state`), `tools/bench/{bench.c,bench_host.c,CMakeLists.txt}`, `fixtures/FX-diag-stic-2/`, `tools/release-readiness.sh`, `tools/release-rehearse.sh`, `.github/workflows/release.yml`, `CHANGELOG.md`.

### Don't Hand-Roll
| Problem | Use instead | Why |
|---|---|---|
| a second encoder for snapshots | the digest walker with a buffer sink | one table is the point of STA-01; DIGEST_* faults already guard it |
| SHA-256 | `core/src/common/ivs_sha256.c`, runner copy `runner_sha256.c` | in-tree, no dependency |
| ns clock, CPU time | two POSIX calls in `bench_host.c` only | D-21; the core import deny list names the clocks |
| golden writer | `ivs-run --update-golden` (a person runs it) | the gate never writes a golden |
| frame hash for F-M frames | `sha256-text` of the canonical plane text | the `video.cmake` rule; frames with unknown cells use pixel probes only |
| new status plumbing | `api-effects.json` rows plus `api-statuses.ids` | the checker fails an undocumented call or dead status |

## 1. STIC seams a MOB, collision, border and delay renderer touches

| # | Seam [OBS] | Fact | Consequence [INF/REC] |
|---|---|---|---|
| 1.1 | `core/src/stic/stic_frame.c` `ivs_stic_frame_deliver(p, s, const ivs_memsys *mem, row, sink, end_tick)` | renders 12 card rows through `ivs_stic_render_card_row`, 8 plane rows each, into a 2,048-byte `cardrow` and streams SHA-256 and the slot copy; `memset(header...)`, chain advances with or without a buffer | collision write-back needs `mem` non-const here; the only caller is the FRAME_END case of `stic_events.c` which already holds a non-const `mem`; update the declaration in `stic.h` and the module tests that call it |
| 1.2 | `stic_cards.c` `ivs_stic_render_cards_row` returns color indices only | MOB/background collision needs "on" pixels (foreground set), colored-squares 0..6 on, 7 off | [REC] add a pure sibling `ivs_stic_cards_on_row` and leave the existing function byte-identical (the F-A and F-B literals are the tripwire); do not add an output parameter |
| 1.3 | `IVS_STIC_CARDROW_MAX` 2,048 and the stack-probe rule ("a 15 KB frame pulls in the stack probe, a forbidden import") | whole-plane buffers are forbidden on the stack | [REC] row-streaming: collision mask is a local `uint32_t[8]`; vertical delay carries at most 7 plane rows (7 x 160 = 1,120 B) in a second local buffer; stay under about 4 KB total |
| 1.4 | delay and border placement | with vertical delay d the plane row y shows source row y - d, crossing the 8-row card boundary; left delay shifts columns within a row | [INF] row-streaming with a carry buffer; the extension strips (8 columns, 8 rows by default) are painted last over cards and MOBs |
| 1.5 | `stic_regs.c`, `memsys.c:184` store `value & reg_wmask[r]`, read `(store AND rmask) OR (fill AND NOT rmask)` | collision rows have `w=03FF r=03FF` [OBS regmap literal rows 0018..001F]; CPU write replaces, read has no side effect | D-06 holds with no register-file change; the FRAME_END OR is a direct array store that bypasses the closed register gate (REG_CLOSE precedes it) |
| 1.5b | `stic_state` (528 B) members | no member for MOB or collision; `IVS_MACHINE_BYTES` 114,416 and the `_Static_assert` offsets stay | [INF] VID-05 adds NO hidden state; the schema digest and every row stay; HP-MOB is a cut with `stic_reg` non-default, not a new row |
| 1.6 | `ivs_stic_state_validate` rejects `reserved0 != 0` | | restore decodes `reserved0 = 0` |
| 1.7 | profile fields | `video.row_lines` (2) is metadata, `video.object_grid` {167,105} is "consumed only by sanity checks" [OBS profile json]; each pinned default needs a one-field flip | new fields go in the profile json, `cmake/IvsSticTable.cmake` (fail closed, generator reads each field), `ivs_stic_params` + `stic_params.h`, `data/expected/profile-fields.ids` (143 lines now = `profile-keys=143`), and `video.cmake` `video-rule-field-unregistered` / `data/stic/fields-pending.ids` for oracle-first lines |
| 1.8 | rules | `data/stic/rules.json` has 53 rules; STIC-R28 says "MOB, delay and border-extension registers are stored and not applied" with conflict C87 | R28 becomes false in plan A2: reword it in the same commit that changes behaviour (not a later sweep) |
| 1.9 | existing guests | the lamp-tone `.asm` EQU list names the STIC strobe `$0020` and mode select `$0021` and no `$0000..$0017` or `$0030..$0032` address [OBS lines 23-24, 56-57]; the diag-stic-1 listing comments name only the strobe; the other two diagnostics were not read | [INF] a MOB/delay/extension-aware renderer reproduces every committed `digest.video` when the registers are 0 (X = 0 disables a MOB); run all goldens at A2 as the neutrality proof |
| 1.10 | scenario caps | `IVS_SCN_MAX_ASSERTS` 64, `IVS_SCN_MAX_INPUTS` 64, `IVS_SCN_MAX_STEPS` 64, `IVS_SCN_MAX_LINES` 512, `capture video` at most 16 frames [OBS ivs_runner.h:105-112] | a collision or MOB scenario must budget its pixel and mem asserts under 64; in-process table tests carry the sweeps |

Digest rows: collision bits are inside `mem.stic_reg` (domain 2, STATE); nothing else changes. Video digest changes only for runs with MOB, delay or extension registers set.

## 2. Digest field table and snapshot classes

### 2.1 Counts [OBS awk over machine_digest.c; arithmetic checked by script]
88 rows = 67 literal `IVS_DF` rows + 3 `IVS_DF_SLOT` macros x 7 rows (the discuss note's "about 95" is wrong). By kind: UINT 79, PROFILE 2 (`profile`, `mem.profile`), MAGIC 1 (`magic`), PAD 6 (`reserved`, `cpu.reserved`, `mem.psg_reserved`, `psg.reserved`, `stic.reserved0`, `obs.audio.reserved`). By domain: CPU 10, MEM 12, IMAGES 25 (4 literal + 21 slot), INSTANCE 28, VIDEO 2, AUDIO 5, INPUT 4, PSGLOG 2.

| Class (D-11) | Rows | Members |
|---|---|---|
| STATE | 51 | `phase`, `time_ticks`; 9 CPU (`cpu.r` .. `cpu.busak`); `mem.gate_open`, `sysram`, `scratchpad`, `gram`, `psg_reg`, `port_pulldown`, `stic_reg`; 10 PSG; 11 STIC (`frame_index` .. `row_latch`); 12 obs (video 2, audio 4, input 4, psglog 2) |
| IMAGE_BEARING | 3 | `mem.exec`, `mem.grom`, `mem.cart` (110,592 B, domain 2, never serialized) |
| BOUND | 27 | `profile`, `mem.profile`, `mem.exec_loaded`, `mem.grom_loaded`, `mem.cart_count`, `mem.cart_seg`, 21 slot rows |
| PAD/MAGIC | 7 | the six PAD rows and `magic` |

[INF] Snapshot bytes: sections 3,655 (INSTANCE 615, MEM 2,761, CPU 35, VIDEO 51, AUDIO 63, INPUT 79, PSGLOG 51, each with the 7-byte `"IVSD" || domain || version` frame and a u32 count before each array row), plus the discuss header of 612 B and the 32-byte trailer = **4,299 B**. IMAGES has no STATE row, so it has no section (7 sections: domains 1, 2, 4, 16-19). [REC] B1 derives and pins it on paper before any C. `sysram` serializes its full 512-word backing although the extent is 352 words, and `scratchpad` 256 words for a 240-byte extent: the unreachable tails are candidates for the dead-row exemption list, each with a reason.

### 2.2 Facts the plans need [OBS]
- `ivs_digest_field.reserved` is read by no test and no file [OBS grep of `tests`, `core`]; D-11's class goes there with no struct growth. The walker's only filter point is `ivs_digest_domain()`; the faults DIGEST_SKIPS_RAM, _SKIPS_IMAGE_TABLE, _LITTLE_ENDIAN, _INCLUDES_PADDING, _DOMAIN_SWAPPED, _PSG_ROWS_OMITTED, _STIC_ROWS_OMITTED, _COMPOSITE_VERSION_STALE and DOMAIN_ID_DROPPED have their sites in `machine_digest.c` and are listed in `data/expected/faults/machine.ids` (suite `machine`, allowed prefix `machine.digest.`).
- `faults-verdict.sh`: each fault is judged only against the result of its own listed suite (`build/gate/faults/<ID>/<suite>.txt` must list exactly that suite's ids). **A new `state` suite therefore never sees the DIGEST_* faults and adds no collateral rule to them** [OBS]; only the `machine` suite's own cases (including any new `machine.digest.class-*` ids B2 adds) must keep matching the allowed prefix `machine.digest.` or name a new prefix in the same file.
- Tripwire for the behaviour-neutral refactor (B2): `data/expected/digest-v1-literals.ids`, `digest-v2-literals.ids` (52 lines), the 29 `machine.digest.*` ids, and every `.digests` golden (15 goldens).
- Profile parameters: one profile is registered, `intv-2609-ntsc-family-p1` [OBS `data/profiles/`]. All four generators state "pure function of the profile files: no host path, no time" [OBS headers of `IvsMemmapTable`, `IvsPsgTable`, `IvsCtrlTable`, `IvsSticTable`], so a digest over each generator's own table text is byte-stable [INF]. [REC] each generator emits `string(SHA256)` of its table body (not the tag, cite or comment text) as a constant; the snapshot parameter digest is one SHA-256 over those four at run time (no cross-generator ordering, no runtime table hashing). Add a generator-level test that an edit of only a `cite` string leaves the constant unchanged.
- Profile-mismatch test: with one profile, `STATE_MISMATCH` reason PROFILE (a registered other id) cannot be built through the public API [INF]. Build it as: (a) resealed snapshot with an unregistered id -> `UNKNOWN_PROFILE`; (b) resealed snapshot with the parameter digest flipped -> `STATE_MISMATCH` reason PROFILE_PARAMS; (c) the id comparison as a pure decode-seam function with a synthetic descriptor, tested in `state`. State in the plan that (c) is a seam test, not an API test.
- `faults.cmake` GEN_* mutation exists (`ivs_fault_gen`, e.g. GEN_MAP_MUTATION) for generator faults; a profile-field flip control for a pinned default is cheaper as a runtime perturbation of a stack copy of `ivs_stic_params` (the suite `stic` precedent) than as a fault build [REC].

### 2.3 `IVS_E_STATE_MISMATCH` (-24) versus the reserved band [OBS]
`tools/check/api-effects.cmake:159` fails any status in -31..-24 (`api-status-reserved`) and the control `api-status-reserved` pins `IVS_E_RESERVED_X -24`; `ivs_status.h:6` says "-24..-31 are reserved". D-14 fixes -24, so plan B3 must in ONE commit: narrow the band to -25..-31 in `api-effects.cmake` (lines 19, 159, 160), the `ivs_status.h` comment, retarget the control overlay to -25, add the status to `api-statuses.ids`, and give it a refusal row (else `api-dead-status`).

### 2.4 Hidden-state checklist coverage
[REC] `data/expected/state-checklist.ids`: one line per table row (88), columns row name, class, witness cut id, test id, exemption reason or `-`. A pure checker fails a row with no line, a line with no row, a STATE row with no witness, an unknown test id, an exemption without a reason, a stale schema-digest literal. Home: a new pure step `state` (steps 36 -> 37, `data/expected/gate-steps/state.ids`, one script, no job) or a section of an existing cmake checker; the new step is clearer and costs about 0.05 local cpu-s [INF]. The runtime ROW sweep uses `ivs_machine_digest_field(i)` (offset, width, count) over 51 rows.

## 3. Runner, scenario, golden and API registration ledger

### 3.1 One new scenario (`stic2-*`, `bench-*`) [OBS from the Lamp Tone footprint]
`data/scenarios/<id>.ivss` (C); `data/expected/runner-scenarios.ids` (+1 line `<id> <exit> <outcome> <class> <skip> <args>`); `data/expected/tests/runner-process.ids` (+1 `runner-process.scenario.<id>`; tests-expected +2); golden `data/golden/<id>.expect` and/or `.digests` (`goldens=15` +1 per file; `.digests` needs `# domains:` by the count rule, a `.expect` needs `# frame: <n> <F-id>` for every hash assert, `video-frame-hash-expect-drift`); `data/expected/scenario-pairs.ids` for judges; mutants in a `data/expected/diag-mutations-*.ids` inventory (a new file needs wiring in `tests/runner-process-tap.sh`); `golden.sh` requires the dev report `build/gate/dev/runner/<id>.report` and byte-equal dev and release reports (host.calls removed), so every scenario gets the cross-preset judge free. A `.digests`-only golden needs no `.expect` (precedent `timeout-budget`, and the scratch run below).

### 3.2 One new program (`ivs-diag-stic-2`)
`fixtures/FX-diag-stic-2/{manifest.json,diag-stic-2.asm,diag-stic-2.listing.txt}`; `data/expected/{fixtures,listings}.ids`; `tools/pack/CMakeLists.txt` (image); `tests/runner-process-tap.sh` (content mapping, mutant stub handling); `data/expected/tests/asm.ids` (`asm.regen-<id>`); `docs/diagnostics.md`, `docs/evidence-tiers.md`; claims `data/claims.json` and `claims.ids`; manifest census `manifests=10 effective-yes=7` -> 11 and 8; `listings=6 listing-words=520` grows. The installed consumer is NOT extended for it (D-10).

### 3.3 Bench
[OBS scratch run, dev preset, build of this tree] a scenario of `run-to 7168320 ticks` (120 frames) with no `capture`, no input and no assert: user 0.15 s, sys 0.14 s, 0.33 s wall total; golden 60 lines, `video=observed audio=observed input=none psglog=observed`, `step.1.now_ticks 7168352`, `output.video.frames 120`, `video.chain.check pass`. [OBS exec.c] the executor re-creates the instance in host storage and re-reads every image per call, so a timed iteration is create+load+reset+run+report. Registration per workload: `.ivss`, `.digests`, one `runner-scenarios.ids` line, one `runner-process.ids` line. `tools/bench` also needs: a sixth module line in `c-build.sh` (`'^tools/bench/.*[.]c$ ivs-bench-sources.txt'` plus a control `c-build-unlisted-bench-source`, the asm/pcm precedent), an `add_subdirectory`, a suite `bench` (`data/expected/tests/bench.ids`, ctest-list must equal the suite set) for the verify-only and refusal cases, and the host vtable (`read_file`, storage up to 64 MiB, buffers up to 8 MiB) built in `bench_host.c` from `ivs_hostio_read` (hostio.c is in the `ivs_run` target only, not the library; either add it to the bench sources or copy the read, D-21 allows duplication). [REC] `bench-lamp-snapshot` is a `workloads.tsv` row (cadence column) over the idle scenario; its gate proof is the in-process `ivs-bench --verify-only` case plus the state suite, while `runner-scenarios.ids` carries only scenarios that `ivs-run` can run plain.

### 3.4 Round-trip hook
`runner/exec.c` slice loop (lines 955-1075 [OBS]) is the insertion point: after a slice whose `now` reached the cut tick, `ivs_machine_snapshot_save`, then `ivs_machine_reset` (public API only) and `ivs_machine_snapshot_restore`, then continue. Options: `ivs_runner_options.flags` and `.reserved` must be 0 today (harness-fail otherwise [OBS exec.c:1301]); append `roundtrip_at_ticks` and gate it with a flag bit; `main.c` usage text (`--slice-ticks` is the style), `docs/runner.md` options table, runner suite ids (+~4), process cases `runner-process.roundtrip.<id>` (+~4 to 8, about 0.2 hosted cpu-s per pair). Existing scenarios must reproduce their goldens under the hook. [REC] reset-then-restore, not poison: a reset target holds defaults at the cut, so a field that is non-default at the cut and not restored diverges; byte-level poisoning stays in the in-process suite.

### 3.5 Public API surface (B3)
Files: `core/include/ivs/ivs_state.h` (new; `IVS_API` prototypes one line each; the api-effects scanner splits on brackets in prose), `ivs.h` include, `ivs_status.h`, `data/api/api-effects.json` (15 -> 19 calls; about 22 refusal rows: size 2-3, save 4, restore 9-10, describe 5; each cites a registered test id; `api-calls=15` -> 19, `api-rows=40` -> about 62, `api-statuses=14` -> 15), `data/expected/api-reasons.ids` (63 lines; new `IVS_REASON_*` from 58; every define in class `IVS_REASON_` must be pinned, so `pinned-defines=96` grows by the count; `IVS_SNAPSHOT_*` macros are NOT a pinned class [OBS DEFINE_CLASSES: `IVS_STOP_`, `IVS_REASON_`, `IVS_PSGLOG_SRC_`, `IVS_CTRL_`]), `pkg-install-files.ids` (10 -> 11 files; `include/ivs/ivs_state.h`; the install rule is a directory glob so no CMake edit), `pkg-install.sh` `check_deps` header list (positive presence), `data/expected/tests/headers.ids` (+`headers.c17-ivs-state`, +`headers.cxx-ivs-state skip-ok`; `hdr-alone=8/8` -> 10/10), `pkg-consumer-calls.ids` (+3 or 4 names), `core-symbols-allow.txt` already allows memcpy, memset, memcmp, `docs/packaging.md` and `docs/formats/ivsn-v1.md`.

### 3.6 Gate summary line: fields that move [INF, counts at Phase 7 end from 07-12-SUMMARY]
`steps=36/36` (37 only if the `state` step is added); `controls=357` about +30 (D1 +14, A +4 to 6, B +10, C +1); `faults=222` about +14 (A +4, B +9 to 10, C 0); `tests-expected=4624` = 2 x ids (A about +100, B about +170, C about +12); `claims=24 provisional=23` -> 27 and 26 (+1 if a state claim is raised, none for PRF-01); `d10-items=42` -> 47; `discrepancies=106` -> about 115; `stic-rules=53` -> about 67; `video-frames=16` -> about 26; `goldens=15` -> about 25; `profile-keys=143` -> about 150; `manifests` 10/7 -> 11/8; `ci-workflows=1 ci-uses=1` -> 2/2; `api-*` and `hdr-alone` as 3.5; `consumer-lines=135` grows by the fifth pass; `tracked=2252`. Each plan recomputes its own line at its commit (PLAN-LESSONS 1); a `tests-skipped` increase only through `skip-ok` ids (H12).

## 4. Release machinery inputs [OBS]

| Input | State |
|---|---|
| `.github/workflows/` | only `gate.yml`: `on: push main, pull_request`, top-level `permissions: contents: read`, one job `gate`, `timeout-minutes: 5`, SHA-pinned `actions/checkout` v7.0.1, steps `cmake --version` and `sh tools/gate.sh`; hosted gate 94-154 s over the last eight rows of `ci-runs.tsv` |
| `ci-parity.sh` (160 lines) | allows only `run: sh tools/gate.sh` or `cmake --version`; only `actions/checkout`; no `secrets.`; no `strategy`; `runs-on` only `ubuntu-24.04`; gate job `timeout-minutes: 5` and no other value in gate.yml; no `if:`; every `permissions` block only `contents: read`; stats `ci-uses`, `ci-workflows`. Controls named `ci-parity-*` (14) incl. `ci-parity-write-permissions`, `-run-spacing` |
| release workflow | none: no `release.yml`, no `CHANGELOG*`, no `tools/release*` |
| `ivs.h` | `IVS_API_VERSION_MAJOR 0u`, `_MINOR 1u`, `_PATCH 0u` (lines 22-24); root `CMakeLists.txt` parses them before `project(ivs VERSION ...)`, so `ivs_VERSION` is the same source |
| tag trigger | AUTONOMY section 5 design says `tags: ['v*']`; D-31 requires `v[0-9]*.[0-9]*.[0-9]*` |
| `git.create_tag` | `.planning/config.json` has `"create_tag": true` and the file already differs from HEAD (`_auto_chain_active`, `use_worktrees`) [OBS git diff], so the D-31 edit is an orchestrator-only, separate, deliberate commit; executors never stage it |
| `binary-ext.txt` | 27 names incl. `gz`, `tar`, `zip`; no `ivsn`; read by `integrity.cmake`, `files.cmake`, `binscan.sh`, `hygiene.sh`; `.gitignore` lists `*.ivsc`, `*.wav`, `*.ppm`, `*.vraw`, `*.grom`, `*.img`, no `*.ivsn` |
| install tree | `pkg-install-files.ids`: 4 headers, `lib/libivs_core.a`, 4 CMake package files, `share/doc/ivs/LICENSE` = 10 paths; B3 makes it 11 |
| text claim scan | `text.sh` scans `README.md` and `docs/**/*.md` from FILELIST only (regex line ~30); claim words supports, compatible, passes, accurate, verified, runs, plays, playable; tag `[claim:CL-NNNN]` or `[nc]` |
| `hygiene.sh --history` | `git rev-list --all`; findings print `-- FAIL hygiene-<kind>: <10-char sha>`; baseline commit 8907ff7e... in `tools/check/hygiene-baseline.txt` ("never move it forward"); a full scan on this laptop ran more than 9 minutes without finishing its identity and file-name tail; the per-commit loop had by then printed exactly six findings: `hygiene-history-home-path` for eaa832e3d3, 92c4dc7d37, b78af4fb9f, cccdfefdce, 345c48dd7f and `hygiene-message-home-path` for 955c13fa02 [OBS]. The Phase 4 and 7 records name 4ccc9bb for the message finding, but 4ccc9bb is on no branch (not an ancestor of origin/main, in no `--contains` list) and carries the same subject; the reachable commit is 955c13f [OBS git]. The release check pins these six (token, short sha) pairs |
| claims | `data/claims.json` 24 claims (CL-0001..CL-0024), 23 provisional; discrepancies C1..C106 |
| CI record | `.planning/ci-runs.tsv` last five rows `r` = 8.17, 7.73, 7.78, 8.37, 7.20, median 7.78 [OBS recomputed from cpu_s / build_s] |

[REC] release.yml design that keeps `ci-parity` small: one `run: sh tools/release.sh` line (the script does tarballs, `release-assets`, the consumer against the shipped tarball, `gh release create --draft --prerelease --verify-tag --latest=false`); extend `ci-parity.sh` with a per-file exemption table for `release.yml` only: job-level `contents: write`, that one run line, a different timeout, the strict tag trigger, `GITHUB_TOKEN` as the only secret; keep every gate.yml rule. Retarget `ci-parity-write-permissions` (still failing for gate.yml) and add controls for each release.yml exemption that must not widen (`run` line other than the script, `pull_request` trigger, `v*` pattern, second secret). `release-assets` (content-based, D-32) fits inside `pkg-install.sh` where the install tree exists (`cmake -E tar` lists members), avoiding a new step; `.a`, `gz` and `tar` are checked on members, not the outer name. Version agreement (`tag == ivs.h macros == ivs_VERSION`) is one awk in `tools/release.sh` plus a control.

## 5. Validation Architecture

### Test framework
| Property | Value |
|---|---|
| Framework | in-repo C harness (`ivs_test_harness`, TAP), CTest per suite, POSIX sh gate steps, negative controls under `tools/check/controls/<name>/{CHECK,EXPECT,overlay}` |
| Config | `tests/CMakeLists.txt` (one source list per suite feeds exe and `ivs_fault_suite`), `data/expected/tests/*.ids`, `data/expected/faults/*.ids` |
| Quick run | `cmake --build build/dev --target ivs_tests_video && ctest --test-dir build/dev -R '^video$' --output-on-failure` (same for `state`, `machine`, `run`, `runner`, `bench`) |
| Full suite | `sh tools/gate.sh` and `dash tools/gate.sh`; container `sh tools/gate-linux.sh` where the plan says so |

### Requirement to check map
| Req | Behaviour | Automated check | Negative control (can fail) | Residual |
|---|---|---|---|---|
| VID-05 MOB | placement, zoom, flip, colour, priority per frames F-M* | `video.mob.*` pure table tests vs hand frames; scenarios `stic2-mob`; video step grammar, hash, pair-region tokens | faults MOB_PRIORITY_SWAP, MOB_FLIP_SWAPPED; frame-literal drift and pair-drift controls; program mutants with `fails=` | tier 4, one lineage; never hardware |
| VID-05 collision | sticky bits, own bit clear, truth table | `video.collision.*`; scenario `stic2-collision` with `03FF` masks; peek purity ids unchanged | fault COLLISION_NOT_STICKY; one-field flip of a pinned default | read, clear, X=0, border bit pinned |
| VID-05 border, delay | strips, shift, clipping | `video.border.*`, `video.hdelay.*`, `video.vdelay.*`; scenarios | fault DELAY_DIRECTION; `video.*.default-pinned` plus stack-copy flip | exposed-area content, range pinned; D10 lines |
| VID-05 neutrality | zero registers change nothing | all 15 goldens, dev vs release judge, F-A/F-B/F-D/F-S literals | any golden diff fails `golden` | none |
| STA-01 | one walker, class coverage, canonical big-endian, identity, validate before mutate | `machine.digest.class-*`; digest literals unchanged; `state.literal.*` hand header; refusal matrix; atomicity memcmp | SNAPSHOT_LITTLE_ENDIAN, _PAD_SERIALIZED, _IMAGE_BYTES_EMBEDDED, RESTORE_MUTATES_BEFORE_VALIDATE, RESTORE_TRAILER_UNCHECKED, RESTORE_PROFILE_UNCHECKED, RESTORE_LENGTH_TRUSTED (ASan) | exact-build, not archival |
| STA-02 | restore then run equals uninterrupted run at 17 cuts | `state.cut.*` whole-instance memcmp on a poisoned target, concatenated streams, nine digests, fixed-point re-save, cross-preset identical snapshot bytes | runtime ROW sweep (omit row r, require divergence) from `ivs_machine_digest_field`; fault RESTORE_OBS_NOT_RESTORED; checklist controls (row without line, witness without test, exemption without reason, stale schema literal) | no big-endian host run |
| PRF-01 | digest before throughput; three separate groups | `bench.verify.<workload>` (verify-only), `runner-process.scenario.bench-*`, `golden` | `bench.refuse.mutated-golden`: flipped digest -> exit 15, no timing field; a mutated core via an existing fault build is optional | throughput never gated; baseline is a person's record |
| REL-01 | release readiness for the exact commit | `release-readiness.sh --selftest`, `release-assets`, version agreement, `ci-parity` extension, `text` scan of `CHANGELOG.md`, `<stage2>` placeholder check | controls: extra installed file, version skew, injected binary asset, claim at `measured-hardware`, untagged claim in CHANGELOG, `release.yml` widening | stage 2 (main green on the merge commit, run ids) is orchestrator evidence |

### Wave 0 gaps
New: suite `state` (`tests/state/*.c`, `tests/CMakeLists.txt` list, `data/expected/tests/state.ids`, `faults/state.ids` with `# suite=state`), suite `bench`, frames F-M*, `docs/video/diag-stic-2-derivation.md`, `data/expected/state-checklist.ids`, `tools/bench`, `tools/release-*.sh`. Nothing to install.
Sampling: per task its verify command plus the plan's gate counts; per plan that touches C, sh or cmake also `dash`; the container gate only where the plan names it (about 3 minutes, emulated).

## 6. Ordered work breakdown (21 plans; sizes are files / test ids)

| Plan | Wave | Depends | Content | Size [INF] |
|---|---|---|---|---|
| 08-01 | D1 | none | `release.yml`, `ci-parity` exemption table, version agreement, `release-assets` in `pkg-install`, `CHANGELOG.md`, `text` scan + controls, `phase8-start` line in CI-EFFICIENCY | 8 files; 0 ids; controls +9 |
| 08-02 | D1 | 01 | `tools/release-readiness.sh` (+selftest 4 controls), `tools/release-rehearse.sh`, hygiene-history pinned list and check, `docs/packaging.md` | 6 files; controls +5 |
| 08-03 | A | none | A1 oracle, no C: derivation + blind-spec, 8-10 frames, `video.cmake` grammar (`-` cells, `mob` records, hash optional) with controls, 14 rules, R28 stays until A2, C107+ entries, 5 D10 lines, pending lines, blind second read record | 20 files; frames +10, rules +14, discrepancies +9, controls +3 to 4 |
| 08-04 | A | 03 | MOB renderer: `stic_mob.c`, profile fields, generator, params, R28 reword, `video_mob.c`, `IvsVideoFrames` glob, faults MOB_PRIORITY_SWAP, MOB_FLIP_SWAPPED, delete pending lines | 14 files; about 28 ids; faults +2 |
| 08-05 | A | 04 | collision: `deliver` non-const, on-mask, OR at FRAME_END, `video_collision.c`, fault COLLISION_NOT_STICKY, slice and buffer-independence reruns | 8 files; about 16 ids; fault +1 |
| 08-06 | A | 05 | border + delay: row streaming with carry, `video_delay.c`, fault DELAY_DIRECTION, C92 pin | 7 files; about 16 ids; fault +1 |
| 08-07 | A | 06 | `ivs-diag-stic-2`, 6 scenarios, goldens, pairs, 6 mutants, controls (selector, shape, pair drift, literal drift) | 24 files; about 22 ids |
| 08-08 | A | 07 | close: CL-0025..27, `important-gates.sh` (+VID-05) and the retargeted control, discrepancy finals, REQUIREMENTS, checkpoint A line | 10 files; 0 ids |
| 08-09 | B | 08 | B1 oracle: checklist, `ivsn-v1.md`, size on paper, hand header literal, 3 controls, `state` step or checker | 8 files; controls +3 |
| 08-10 | B | 09 | B2 walker refactor (class in `reserved`, sink modes, `ivs_input_derive`, PSG validate, STIC validate extension) | 7 files; +6 machine ids |
| 08-11 | B | 10 | B3 serializer + API surface (3.5, 2.3), generators' parameter constants, suite `state` created | about 26 files (largest; split into 11a surface+save and 11b generators if over 25); about 20 ids; faults +4; controls +2 |
| 08-12 | B | 11 | B4 restore, refusal matrix (about 45 rows), atomicity | 6 files; about 30 ids; faults +6; **cost checkpoint line** |
| 08-13 | B | 12, 07 | B5 hard-point suite: witness scan, 17 cuts (HP-MOB uses the 08-07 program), ROW sweep, exemption list | 5 files; about 20 ids |
| 08-14 | B | 12 | B6 corruption family + `fuzz_snapshot.c` (outside the gate) | 4 files; about 12 ids |
| 08-15 | B | 12, 13 | B7 `--roundtrip-at-ticks`, fifth consumer pass, `.ivsn` binary-ext and `.gitignore`, owner-images doc, important-gates STA-01/02, claim | 16 files; about 14 ids; checkpoint B |
| 08-16 | B opt | 15 | snapshot file options (runner only, refused for `content user`) | 4 files; about 8 ids |
| 08-17 | C | 15 | `ivs-bench`, `bench-lamp-idle`, `-input`, goldens, workloads.tsv, suite `bench`, refuse control | 12 files; about 6 ids; controls +1 |
| 08-18 | C | 17 | `docs/bench.md`, baseline record, cold and snapshot rows, PRF-01 mark | 5 files |
| 08-19 | C opt | 17 | `bench-cpu` fixture | 8 files |
| 08-20 | D2 | 18 | stale-text sweep (nine or more statements), 39 summary frontmatter backfills (Phase 4: 7, 5: 12, 6: 14, 7: 6 lack `requirements-completed`), six draft validations recorded | about 50 files, docs only |
| 08-21 | D3 | 20 | `08-VERIFICATION.md`, `08-MILESTONE-CLOSE.md` stage 1, full `--history` paste, CI close line, final gates sh, dash, container | 6 files |

Edges: 08-01 -> 08-02; 08-03 -> 04 -> 05 -> 06 -> 07 -> 08 (serial, shared inventories); B starts after 08 (D-20); 08-13 also needs 07; C after B7 (D-26); D last. After the merge the orchestrator does REL-01 stage 2 (`gsd/records-m1-close` PR), the `create_tag` edit, the draft publish.
**Cost** (D-37): wave A about +8.7, B +12 to 16, C +0.4 local hosted cpu-s; median `r` 7.78 -> about 8.1-8.2, budget 7.8, trigger 8.3 [from CI-EFFICIENCY, not re-measured]. Unit costs measured earlier: fault about 1.0 hosted cpu-s, control about 0.15, process-case pair about 0.2. Checkpoint lines go in 08-08, 08-12 and 08-15; if the last-five median reads 8.0 or more at a checkpoint, no further test case is added (the Phase 7 rule).

## 7. Hazards (PLAN-LESSONS 1-12 and EXEC-RULES applied, plus new ones found)

| # | Hazard [OBS unless marked] | Plan action |
|---|---|---|
| H1 | Lesson 1: goldens, consumer golden, size pins and counts move in the commit that causes them | every plan lists the counts of its own commit; no re-bless task |
| H2 | `important-gates.sh:76` hard-codes the accepted requirements and the header comment says VID-05 "is rejected"; control `important-gates-bad-req-phase6` uses VID-05 as its bad id; each accepted REQ needs at least one registry line (`important-gates-uncovered-gate`) | 08-08 (VID-05) and 08-15 (STA-01, STA-02, PRF-01): extend `nreq`, retarget the control to a never-valid id, add lines in the same commit |
| H3 | `video.cmake` accepts only symbols 0-9A-F (plus the cut-off mark), requires `sha256-text`, and the cell popcount and unlisted-uniform rules reject MOB pixels (lines ~534-690) | 08-03 extends the grammar and adds controls before any F-M frame is registered; frames with unknown cells carry no hash and are asserted by `pixel` (`hash-needs-colors-only`) |
| H4 | `tests/CMakeLists.txt` generates the frame include from `F-[AB]*.frame` only | 08-04 widens the glob or adds a generator for F-M |
| H5 | `data/stic/rules.json` STIC-R28 becomes false when MOB renders (C87 "pending: Phase 8", C105) | reword in 08-04 with the behaviour change |
| H6 | scenario caps (64 asserts, inputs, steps) | probes in process tests, few asserts in scenarios |
| H7 | status -24 vs reserved band (2.3) | one commit in 08-11 |
| H8 | the new walker filter sits in `ivs_digest_domain()` where the DIGEST_* fault sites are | B2 keeps every `#ifdef IVS_FAULT_DIGEST_*` site and the literals byte-identical |
| H9 | Lesson 4/7: fault sites travel with the test that detects them; one site per fault; the guard a control names must exist in the script it targets | each fault's `IVS_FAULT_<ID>` site, its `faults/*.ids` line and its detecting id land together; check the control's token exists in the checker script |
| H10 | `ci-parity.sh` blocks release.yml in every dimension (section 4) | exemption table plus controls in 08-01; `release-assets` must not reject its own `.tar.gz` (binary-ext lists `gz`, `tar`): check members |
| H11 | `text.sh` scans only README and docs | add CHANGELOG.md (one regex) plus control in 08-01 |
| H12 | tests-inventory: every id runs in both presets; a skipped id must be `skip-ok` and prints `-- SKIPPED`; D-37 "release only" sweeps would add `tests-skipped` per id | [REC] one id per family and reduce the point set inside the case in the sanitized preset; if a whole id is release-only, mark `skip-ok` and count it |
| H13 | Lesson 5: `machine` and `run` suites list sources twice (exe and `ivs_fault_suite`); `stic`, `video`, `input` use one list variable | new files in `machine`/`run` go in both lists; `state` uses the single-variable form |
| H14 | `pkg-install.sh` `check_deps` and `pkg-install-files.ids` pin the header set; `hdr-alone` and `headers.ids` | 08-11 updates all in one commit; the consumer's undefined-symbol list changes only when the fifth pass lands (08-15) |
| H15 | EXEC-RULES: never stage `.planning/config.json` or `state.json`; trailer exactly `Co-Authored-By: Claude Sonnet 5.5 <noreply@anthropic.com>`; no `gh`, no push | D-31 `create_tag` edit is an orchestrator commit; plans only record the requirement |
| H16 | Lesson 9: hygiene (`HOME_NEW` forbids tmp and private-var shapes, `/Users/<name>`, tilde paths; no `@` before a dotted word); a new record naming a scratch path becomes a `--history` finding forever | repo-relative paths only; measurement scripts keep scratch paths out of tracked text |
| H17 | Lesson 8: derive tick counts by arithmetic (STIC strobe inside the 2,000-cycle register window; a cut must be a unit boundary and a witness test re-proves it) | B5 pins ticks with a witness test; A5 uses the settling rule (no frame assert in the frame of the input or the next) |
| H18 | Lessons 6, 10, 11: container verify command is exactly `sh tools/gate-linux.sh >/dev/null 2>&1 && grep -q '^gate: PASS' build/gate-linux.log`; no blocking FIFO; gcc/dash hazards (argument evaluation order, `-Wformat-truncation`, PIE symbols) | bench and snapshot code: one call per statement when an argument reads state another call changes; `ivs-bench` uses no signals |
| H19 | `faults-verdict`: a suite's result must list exactly the suite's ids; adding ids to `video.ids` changes every video fault's expected list, so fault results must be regenerated in the same run (the gate does this) | no manual list edits beyond the ids file |
| H20 | `IVS_MACHINE_BYTES` and `_Static_assert` offsets: any new `ivs_stic_state` member forces a table decision | VID-05 adds none [INF]; if a plan finds it needs one, stop and add the row before the schema digest is pinned |
| H21 | `.planning/ci-runs.tsv` has no push-to-main rows | stage 2 records the merge-commit push run as `event=push` |
| H22 | ROADMAP and REQUIREMENTS stale text (D-34) contradicts D9 and D6-D12 | 08-20; the sweep must also fix `docs/packaging.md` "ten files" and README "No release has been published" before the tagged commit |

## Common Pitfalls
Shared misreading (one lineage, one author): keep the blind second read and `-` cells; a pinned default that matches community practice looks confirmed, so claims carry exclusions. Zero coincidence: restore into a poisoned or reset target, never a fresh zero target alone. A trailer is integrity, not authenticity: a resealed hostile snapshot must be refused by validators, not by the checksum. Throughput without digest: structure the tool so the timing code is unreachable on a mismatch.

## Code Examples
Row classification (B2 shape) [REC]: `{name, offset, width, count, domain, kind, snap}` where `snap` reuses the `uint16_t reserved` slot; the walker gets `if (only_state != 0u && f->snap != IVS_SNAP_STATE) continue;` after the `domain` test and before the kind switch, so the digest path (`only_state = 0`) emits the same bytes. Collision write-back [REC]: after the last card row, `for (i = 0; i < 8; i++) mem->stic_reg[0x18u + i] = (uint16_t)((mem->stic_reg[0x18u + i] | hit[i]) & 0x03FFu);` with `hit[]` a local.

## State of the Art
Not applicable (no external ecosystem). Internal precedent: Phase 7 W1-W7 pattern (oracle first, pure checker tokens before behaviour, fault and control inventories, checkpoint lines in CI-EFFICIENCY.md).

## Assumptions Log
| # | Claim | Section | Risk if wrong |
|---|---|---|---|
| A1 | Snapshot size 4,299 B assumes the discuss header layout (612 B) and per-section 7-byte frames | 2.1 | sizes and the literal vector move; B1 re-derives |
| A2 | No existing guest stores to MOB, delay or extension registers (lamp-tone read through its EQU list, diag-stic-1 through its comments, the other two not read) | 1.9 | a golden changes at 08-04; fix by reading all four listings first |
| A3 | Hosted cost deltas come from CI-EFFICIENCY unit costs, not re-measured | 6 | checkpoints move; the stop rule handles it |
| A4 | A restored snapshot's full-domain digests (section table) cannot be verified before commit without a virtual walk | 2.1 | B4 decides: test-only oracle or a read-only walk over snapshot rows plus live image rows |
| A5 | `release-assets` fits in `pkg-install.sh` without exceeding the step's cost (about 2.4 local cpu-s now) | 4 | a separate step if it does |
| A6 | `tar --sort` and `gzip -n` give byte-identical archives on the hosted image | 4 | rehearsal in the container shows it; draft release proves it |
| A7 | The six pinned history findings are complete (the scan's tail had not finished) | 4 | a new finding blocks the release (P2) until it is explained; see Open Question 6 |

## Open Questions (RESOLVED)
1. **A new `state` step or an extension of an existing checker?** Known: steps are the script set equal to `gate-steps/*.ids` (`steps=36/36`). Recommendation: new step; update the ROADMAP end-state line in 08-21. RESOLVED: new `state` step (D-38), step 38 after `release-assets`.
2. **Is `bench-lamp-snapshot` a scenario?** Known: `ivs-run` has no cadence option. Recommendation: workloads.tsv row only (3.3). RESOLVED: workloads.tsv row only (D-39).
3. **Hook semantics.** Recommendation: reset-then-restore in the runner (3.4); optional comma list of ticks only if 08-17 needs the cadence. RESOLVED: plan 08-16 fixes save, reset, restore at every multiple of n, flag bit, cap 64.
4. **Release-only accounting (H12).** Recommendation: reduce inside the case; use `skip-ok` only for whole ids. RESOLVED: reduce the point set inside one id, no skip-ok (D-40).
5. **Restore-time verification of section digests (A4).** Recommendation: decide in B1 when the format is written. RESOLVED: trailer and structure verified before commit, section digests stay a test oracle (D-41).
6. **Full `--history` result.** Known: six findings, listed in section 4, with 955c13f (not 4ccc9bb) as the message finding. Unclear: whether the identity and file-name tail of the scan adds any (the run was still going at 9 minutes). Recommendation: 08-21 pastes one complete run into section J, the pinned list is by (token, short sha) pairs, and the release check runs the full scan only at release time (it is a publication step, not a gate step). RESOLVED: plan 08-21 runs the full scan; a finding off the pinned list stops it (D-33).
7. **Which discrepancy and claim numbers.** C107 and CL-0025 are free [OBS: last are C106 and CL-0024]; the state claim number is the planner's. RESOLVED: C107..C110 and CL-0025..CL-0027 in wave A, CL-0028 for state (plans 08-03, 08-08, 08-17).

## Environment Availability
| Dependency | Required by | Available | Version | Fallback |
|---|---|---|---|---|
| cmake, ninja, C compiler | everything | yes | 4.4.3, 1.13.2, Apple clang 21 | none needed |
| sh and dash | gate parity | yes | system | none |
| docker | container gate (`gate-linux.sh`) | client present, daemon state not checked | client 29.5.2 | write a visible "not run" line (EXEC-RULES) |
| gh | stage 2, release | present locally; executors must not use it | n/a | orchestrator only |
| libFuzzer runtime | `fuzz_snapshot.c` | not on Apple clang (CLAUDE.md research) | Homebrew LLVM or Linux clang | target stays outside the gate |
| valgrind, hyperfine | none required | hyperfine present, unused | n/a | none |

## Security Domain
`security_enforcement` on, ASVS level 1 [OBS config.json].
| ASVS | Applies | Control |
|---|---|---|
| V5 input validation | yes | snapshot bytes are untrusted: length bound before any read, structure then trailer then validators, bounds-checked decode, exhaustive truncation and bit-flip families, resealed variants, libFuzzer target |
| V6 cryptography | partly | in-tree SHA-256 only; the trailer is integrity, never authenticity; no new primitive |
| V12 files | yes | file options runner-only, explicit path, exclusive create 0600, refused for `content user`; core does no IO |
| V14 config, supply chain | yes | release.yml SHA-pinned checkout, job-level `contents: write`, `GITHUB_TOKEN` only, strict tag pattern, content-based asset check |
Threats: snapshot of a real-EXEC run carries GRAM and system RAM (derived rights-bearing; `.ivsn` ignored and binary-listed, writer tested for no image window); asset smuggling (content-based check); tag collision with a milestone tag (D-31); history leak (new finding fails the release check).

## Sources
Primary (repo, read this session): `08-CONTEXT.md`, the four `08-DISCUSS-*.md`, `AGENTS.md`, `.planning/{ROADMAP,REQUIREMENTS,CI-EFFICIENCY,AUTONOMY,config.json,ci-runs.tsv}`, `.planning/ops/{EXEC-RULES,PLAN-LESSONS}.md`, `07-RESEARCH.md`; `core/src/stic/*`, `core/src/machine/{machine.h,machine_digest.c,machine_bound.c,machine_bus.c}`, `core/src/memmap/memsys.h`, `core/include/ivs/{ivs.h,ivs_status.h}`, `runner/{exec.c,ivs_runner.h,hostio.h,main.c,CMakeLists.txt}`, `data/{profiles,stic,video,expected,golden,scenarios,api,claims.json,discrepancies.json}`, `tools/{gate.sh,hygiene.sh,ci-record.sh}`, `tools/check/{api-effects.cmake,video.cmake,steps/*.sh,controls,binary-ext.txt}`, `tests/{CMakeLists.txt,runner-process-tap.sh,headers-tap.sh,installed-consumer/main.c}`, `.github/workflows/gate.yml`, `cmake/Ivs*Table.cmake`.
Measured (outside the tree): one 120-frame scenario in a dev build; the digest-table arithmetic by script; `awk` row counts. No web source was consulted; no external package exists.

## Metadata
**Confidence:** repo facts HIGH; sizes and counts [INF] MEDIUM; cost MEDIUM; STIC semantics LOW by design (tier 4).
**Research date:** 2026-11-14; valid until a commit touches the cited gate steps, `machine_digest.c`, `ci-runs.tsv` or `.planning/config.json` (re-check 1.1, 2.1, 4 and the `r` median at plan start).
