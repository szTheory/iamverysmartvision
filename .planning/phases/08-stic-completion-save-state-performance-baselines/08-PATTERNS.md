# Phase 8: STIC Completion, Save State and Performance Baselines - Pattern Map

**Mapped:** 2026-11-14
**Files analyzed:** about 70 new or modified paths, grouped into 22 families (D-36 waves D1, A, B, C, D2/D3)
**Analogs found:** 20 of 22 families with a tracked analog; 2 partial (see "No Analog Found")
**Scope note:** every path below was checked with `git ls-files` (tracked-source gate, #3645); no `build/` or install mirror path is named. Line numbers are of the tree at mapping time (HEAD 0939b93); re-read a range before editing it. Decision ids D-nn are those of `08-CONTEXT.md`. Format follows `07-PATTERNS.md`; excerpts are short on purpose.

## File Classification

| New / modified file or family | Role | Data flow | Closest analog | Match |
|---|---|---|---|---|
| `core/src/stic/stic_mobs.c` (new), `stic_frame.c`, `stic.h`, `stic_params.[ch]`, `core/src/memmap/memsys.[ch]` | pure renderer + FRAME_END hook | transform | `core/src/stic/stic_cards.c`, `stic_frame.c:120-217` | exact (role) |
| profile fields, `cmake/IvsSticTable.cmake`, `data/discrepancies.json` (C107+), `data/expected/d10-measurements.ids`, `data/stic/rules.json` | config + records | batch | `video.display_off_fill` chain (C99) | exact |
| `tests/video/video_{mob,border,collision}.c`, `tests/stic/stic_collision.c`, `data/expected/{tests,faults}/video.ids` | test suite (in-process) | transform | `tests/video/video_card.c`, `video_judge.c`, `video_probe.c`; `tests/stic/stic_slice.c`, `stic_peek.c` | exact |
| `data/video/frames/F-M*.frame`, `video-frames.ids`, `video-frame-pairs.ids`, `tools/check/video.cmake`, `cmake/IvsVideoFrames.cmake` | oracle literals + checker | transform | `F-D1.frame`, `F-S*.frame`, the `diag` FRAMESET | role-match (grammar must grow) |
| `docs/video/diag-stic-2-derivation.md`, `-second-read-request.md`, `-second-read-record.md` | oracle doc | - | `docs/video/demo-lamp-tone-1-derivation.md` + its request/record pair | exact |
| `fixtures/FX-diag-stic-2/*`, image block, `listings.ids`, `fixtures.ids`, `pack.ids`, `asm.ids` | fixture (asm recipe) | event-driven | `fixtures/FX-demo-lamp-tone-1/*`; selector from `FX-diag-stic-1` | exact |
| `data/scenarios/diag-stic2-*.ivss`, `data/golden/diag-stic2-*.{expect,digests}`, `runner-scenarios.ids`, `diag-mutations-stic2.ids` | scenario + golden + mutants | request-response | `diag-stic-fgbg-pass.*`, `demo-lamp-tone-idle.ivss`, `diag-mutations-scn.ids` | exact |
| `core/src/machine/machine_digest.c` (class column, BUFFER sink), `machine.h` | serializer refactor | transform | the same file (behaviour-neutral, as plan 06-03 did for the ABI) | exact |
| `core/include/ivs/ivs_state.h`, `ivs.h`, `ivs_status.h`, `machine_state.c` (new), `api-effects.json`, `api-statuses.ids`, `api-reasons.ids`, inventories | public API | request-response | `ivs_machine_stic_state`, `ivs_machine_state_digest`, `ivs_machine_load` | role-match |
| `core/src/content/ivsn.c/.h` (new), `docs/formats/ivsn-v1.md`, literal vector | container writer/reader | file-I/O (bytes) | `core/src/content/ivsc.c/.h`, `docs/formats/ivsc-v1.md` | exact |
| `tests/state/*.c` (new suite `state`), `state-checklist.ids`, `faults/state.ids`, controls | test suite + faults | transform | `tests/run/run_slice.c`, `tests/machine/machine_digest.c`, `tests/ivsc/*`, `machine_refuse.c` | role-match |
| `tests/fuzz/fuzz_snapshot.c`, `tests/CMakeLists.txt` option block | libFuzzer target | batch | `tests/fuzz/fuzz_ivsc.c` | exact |
| `runner/{ivs_runner.h,exec.c,main.c}`, `docs/runner.md`, `runner-process-tap.sh` | runner hook | request-response | `--slice-ticks` | exact |
| `tests/installed-consumer/main.c`, `pkg-install.sh`, `pkg-selftests.ids`, `pkg-consumer.golden`, `pkg-consumer-calls.ids`, CL | installed replay 5 | request-response | `scn_session` | exact |
| `runner/main.c` `--snapshot-out/-in` (optional last plan) | runner CLI | file-I/O | `--video-raw` + user-content refusal | exact |
| `tools/bench/{bench.c,bench_host.c,CMakeLists.txt}`, `data/bench/workloads.tsv`, `docs/bench.md` | host tool | batch | `tools/pack/CMakeLists.txt`, `runner/main.c` golden capture, `tools/ci-record.sh` | role-match |
| `data/scenarios/bench-*.ivss`, `data/golden/bench-*.digests`, `runner-scenarios.ids` | scenario + golden | batch | `demo-lamp-tone-idle.ivss`, `timeout-budget.digests` | exact |
| `.github/workflows/release.yml`, `tools/check/steps/ci-parity.sh`, controls | CI workflow + gate step | batch | `gate.yml`, `ci-parity.sh` | role-match |
| `tools/check/steps/release-assets.sh` (new), `CHANGELOG.md`, `tools/release-rehearse.sh`, `text.sh` scope | gate step + docs | batch | `hygiene-history.sh`, `text.sh`, `pkg-install.sh` | role-match |
| `tools/release-readiness.sh` (new, read-only) | operator script | batch | `tools/ci-record.sh`, `tools/hygiene.sh` | role-match |
| `data/claims.json` CL-0025..27 (+ state/bench claims), `claims.ids`, `important-gates.{sh,ids}`, gate-step and control registration | records | batch | CL-0023/0024, `important-gates.sh:71` | exact |
| `08-VERIFICATION.md`, `08-MILESTONE-CLOSE.md`, `.planning/CI-EFFICIENCY.md`, stale-text sweep, `config.json` | planning records | - | `07-VERIFICATION.md`, Phase 7 CI-EFFICIENCY close | role-match |

## Pattern Assignments

### Wave A - STIC renderer (`core/src/stic/*`)

**Analog:** `core/src/stic/stic_cards.c` (pure row renderer) and the deliver loop of `stic_frame.c`.
**Copy:**
- Header-comment style that names rule ids, the named faults and "no static or global state" (`stic_cards.c:1-31`); inline fault sites `#ifdef IVS_FAULT_VIDEO_CARD_BIT_ORDER_REVERSED ... #else ... #endif` (`stic_cards.c:103-113`, `:50-52`, `:83-85`); value-copy input struct (`stic.h:112-116`, `ivs_stic_cardin`); graphics bytes only through the pure accessor `ivs_memsys_stic_gfx_byte_internal` (`memsys.h:249`; MOB cards use sel = bit 11 of the attribute and card from bits 10..3).
- The streaming shape: one card row at a time into `cardrow[]`, then hash and copy `py` lines (`stic_frame.c:191-205`):
```c
for (cr = 0u; cr < p->video_card_rows; cr++) {
  if (ivs_stic_render_card_row(p, s, mem, displayed, cr, &cs_pos, cardrow) == 0u) { ... break; }
  for (py = 0u; py < p->video_card_h && y < g.height; py++, y++) { ... ivs_sha256_update(&ctx, src, g.width); ... }
}
```
- The sole call site of the deliver function is the FRAME_END case of `ivs_stic_catch_up` (`stic_events.c:78-89`), where the state is "the state just before the event" (A-06).
**Differs (planner traps):**
1. The plane is never held whole (`stic.h:102-107`: a 15 KB frame pulls in the stack probe, a forbidden import; `IVS_STIC_CARDROW_MAX 2048`, guard `stic_frame.c:135`). MOB overlay, border strips and delay must compose per card row. A vertical-delay band straddles two source card rows and 2 x 1,280 bytes exceeds 2,048: decide the row-buffer size or composition order before coding (one-line change to `IVS_STIC_CARDROW_MAX` and the guard, with a test).
2. Collision write-back (D-06): `ivs_stic_frame_deliver` takes `const ivs_memsys *mem` (`stic.h:156-157`, `stic_frame.c:120-121`) but its only machine caller holds a non-const pointer (`stic_events.c:37`). Add a mutator beside `ivs_memsys_set_gate_internal` (`memsys.h:215`) that ORs into `stic_reg[0x18 + k]` masked `03FF` (rows 0018..001F, class collision, `data/stic/intv-2609-ntsc-family-p1.regmap.literal.txt`). It must run with or without a video sink (the hazard is already a named fault: `VIDEO_STATE_DEPENDS_ON_BUFFER`, `stic_frame.c:153-157`). A test passes `mem = NULL` today (`tests/video/video_buffers.c:424`): tolerate NULL or update that call.
3. No new device state (D-06): `ivs_stic_state` stays 528 bytes (`stic.h:55`, `stic_state.c:17-27`), so no digest literal moves unless a guest writes a MOB register; the existing goldens and `data/expected/digest-v2-literals.ids` must not change (a moved golden is a finding).
4. Frames are pure functions of the registers at FRAME_END (CR-17); keep MOB decode reading `mem->stic_reg` only through a value copy.
5. Register the new TU in the explicit list `core/CMakeLists.txt:12-37` (an unlisted source trips `c-build-unlisted-source`).

### Wave A - profile fields, discrepancies, D10 hand-offs, rules

**Analog:** the `video.display_off_fill` chain.
- JSON block with `value`, `tag: unknown-default-chosen`, `cite`, `conflict`, `resolving_evidence`, `revisit_trigger`, `decision`, `note` (`data/profiles/intv-2609-ntsc-family-p1.json:1524-1532`); generator policy line `st_policy(off_fill video.display_off_fill "border" "...")` or `st_num(...)` (`cmake/IvsSticTable.cmake:343-344`, `:371`); params member (`stic_params.h:117-118`); registry line (`data/expected/profile-fields.ids:135-144`).
- Discrepancy record keys `id topic sources status default_policy resolver evidence note` (C87, C105 in `data/discrepancies.json`); C87 and C105 are the "pending: Phase 8" entries to resolve or re-point; C106 is the last id, so allocate C107+ and add each to `data/expected/discrepancies.ids`. D-08 and D-07 are updates to C81 and C92, not new ids.
- D10 line shape `D10-VID-09 | C85 | text | #stic-power-on` (`data/expected/d10-measurements.ids` tail) with a matching `### <anchor>` heading in `docs/stic/d10-procedure.md` (anchors at `:22-60`). About 5 new lines (D-09), none performed.
- Rules: `data/stic/rules.json` (`id`, `family`, `statement`, `basis`, `cite`, `fields`, `tests`), exact set in `data/expected/stic-rules.ids`; STIC-R28 ("MOB, delay and border-extension registers are stored and not applied") is superseded by name. The frame checker accepts only `(CR|STIC-R)` rule ids (`tools/check/video.cmake:563`): continue as STIC-R37.. or widen that regex (and add a control).
**Differs:** the flip control is not a rebuild. Every STIC function takes an explicit `ivs_stic_params` (`stic_params.h:4-6`, Pattern A1), so a `*.default-pinned.*` test calls the same pure function with a copy whose one field is flipped and requires the result to change; analog `tests/stic/stic_profile.c:41` (parameters `q`). No existing test id contains `default-pinned`: the id family is new, register it in `data/expected/tests/video.ids` and `data/stic/tests-pending.ids` rules as needed.

### Wave A - in-process tests and faults

**Analog:** `tests/video/video_card.c`.
- Text-block expectation: `vc_block_is(r, c, rows[8])` (`:22-46`), a case = poke GRAM, build words, render, compare (`:48-60`), table at `:277`. Bit-role sweep from a literal: `vpr_roles`/`vpr_sweep` (`video_probe.c:1-50`) is the model for the 64-probe MOB register sweep (roles must come from the new literal, never from the core decode). Judge cases that prove the comparison can fail (`video_judge.c:13-58`: corrupt one expected row, swap two roles).
- Suite wiring: externs `video_cases.h:35-50`, `video_add(...)` list `video_main.c:82-92` (cap `VIDEO_MAX_CASES 256`, `:12`; 108 ids today), ONE source list feeds the executable and the fault suite (`tests/CMakeLists.txt:347`, `:483-486`), ids in `data/expected/tests/video.ids`.
- Faults: one line per fault, `<FAULT_ID> <must-fail> <allowed-prefixes> <must-pass>` with the owner TU in the comment (`data/expected/faults/video.ids:4-7`); D-10 allows 4 core faults (priority swap, collision not sticky or own bit set, delay direction, flip), mutants on the release preset only.
- Collision slicing and buffer independence: copy `tests/stic/stic_slice.c` (whole-instance memcmp across call sizes 1, 7, 468, 97) and `tests/run/run_diag_stic.c` `run.diag.stic.slice-invariant`; peek purity from `tests/stic/stic_peek.c` (`stic.peek.sweep-bytes-equal`, positive control `stic.peek.cpu-read-switches`): a read of a collision register must change nothing (D-06).
**Differs:** the generated frame include reads only `F-[AB]*.frame` (`tests/CMakeLists.txt:341`); add a glob and a FRAMESET like `diag` (`cmake/IvsVideoFrames.cmake:25-40`, which writes a smaller include for one suite).

### Wave A - oracle documents and frame literals

**Analog:** `docs/video/demo-lamp-tone-1-derivation.md` and its blind-read pair; `data/video/frames/F-D1.frame`.
**Copy:**
- Derivation layout: preamble stating "written before the program, no run exists", sections (a) specification between `<!-- blind-spec-begin -->` and `<!-- blind-spec-end -->` (`:12-47`), (b) arithmetic, (c) per-event state, (d) values, (e) frames and pairs, (f) assert blocks that the scenarios copy line for line, (k) appended after the freeze.
- Blind read: request with `pending:` first line, inputs listed exhaustively, `sed -n '/^<!-- blind-spec-begin -->$/,/^<!-- blind-spec-end -->$/p'` into a gitignored directory (`second-read-request.md:25`); record head `state: closed`, `oracle-commit: <40 hex>`, `corrections: <n>`, then `correction:` lines (`second-read-record.md:1-3`); evidence kind wording `source-expectation, second-read`, never `independent-implementation-comparison` (C43). Rule: sections freeze after the record commit (D-05).
- Frame literal keys: `frame tier oracle rules mode display border stack fill cell gram sha256-text plane` (`F-D1.frame:5-12`); pair inventory `<idA> <idB> <x> <y> <w> <h> <count>` (`video-frame-pairs.ids`) enforced by the `video-frame-pair-region` token; exact id set `video-frames.ids` (16 today).
**Differs (checker blockers, plan them in the oracle plan):**
- `tools/check/video.cmake` rejects exactly what D-02 wants: a plane body must match `^[0-9A-F]+$` and `-` is allowed only as the 160th symbol (`:518-545`); `mode` is `(fgbg|cs)` only (`:571`); a listed cell must annotate as `fgbg ...` or `cs ...` with distinct fg and bg and a matching pop-count (`:625-660`), and an unlisted cell must be uniform (`:670-690`). MOB overlays and `-` unknown cells need a grammar extension (a `mob` mode or `mob` lines, a declared-unknown rule) plus controls in `data/expected/controls/video.ids` (copy `video-frame-symbol`, `video-frame-popcount`).
- `cmake/IvsVideoFrames.cmake:56,132` already accepts `-` anywhere in the plane; only the gate checker is strict.
- A `-` cell means "unknown, not asserted". It is not the published `blank` value (`0xFF`, "not displayed"): `assert frame <n> pixel x y == blank` and `hash-needs-colors-only` (`runner/exec.c:813`, `docs/runner.md:75-76`) keep their meaning; frames with `-` cells are asserted by pixel probes only, fully coloured variants by hash.
- No analog for the collision truth table; use section (c) of the lamp derivation (per-event state table) as the layout model.

### Wave A - fixture `ivs-diag-stic-2`, scenarios, goldens, mutants

**Analog:** `fixtures/FX-demo-lamp-tone-1/*` for the file set and recipe; `fixtures/FX-diag-stic-1/diag-stic-1.listing.txt:20-40` for the controller-word selector (read `01FE`, keep the low byte, branch); `data/scenarios/diag-stic-cstack-pass.ivss:13-15` for a tick-0 selector (`input at_tick=0 c0=KEY_1 c1=0x0`).
**Copy:**
- Manifest: `fixtures/FX-demo-lamp-tone-1/manifest.json` (recipe `tool`, `tool_sha256`, `command`, `sources` with `.asm`, `tools/asm/asm.c`, `tools/pack/pack.c`; `outputs` = `.asm` + listing; `redistributable: yes`; `note` with the third-party attestation: no glyph, no GROM read, patterns authored here). Refresh digests and sizes in the commit that edits any listed file.
- Registration points of the lamp image: `tools/pack/CMakeLists.txt:86-100` (variable, custom command, `ivs_images` dependency list), `tests/CMakeLists.txt:171-176` (`ivs_pack --c-include "<name>=..."`), the copy list `tests/runner-process-tap.sh:102`, `data/expected/listings.ids:9`, `fixtures.ids:9`, `tests/pack.ids:13-14` (`pack.words-`/`pack.trailer-`), `tests/asm.ids:27` (`asm.regen-<id>`; `tests/asm-tap.sh:172-211` regenerates every `fixtures/*/*.asm`). No new consumer replay (D-10), so `pkg-install.sh` image lists are untouched.
- Scenario: `diag-stic-fgbg-pass.ivss` (`capture video 0 1`, mem asserts, `frame 0 pixel`, `frame 0 hash`) and `.expect` header with `# tier:`, `# oracle: independent-derivation`, `# frame: 0 F-..` ties, "Excluded on purpose" (`diag-stic-fgbg-pass.expect:1-40`); `.digests` only from `ivs-run --update-golden`. Idle control: `demo-lamp-tone-idle.ivss` plus a pair line in `data/expected/scenario-pairs.ids:11-13` (`differ=digest.video`). Inventory rows `runner-scenarios.ids:25-26`; ids `runner-process.scenario.<id>` in `data/expected/tests/runner-process.ids:58-61`.
- Mutants: inventory shape of `data/expected/diag-mutations-scn.ids:75-87` (with `fails=` set and a `pass 0` negative row) and `diag-mutations-stic.ids`; registry rows `diag-stic-mutants mutant runner-process.mutant.<ID> VID-05` (`important-gates.ids:306-310`).
**Differs:** the stic mutant loop is hard-wired to `diag-stic-1.ivsc` and `$listing3` (`tests/runner-process-tap.sh:520-560`); the scn loop `:561-640` is the parametrised model (`run_mutant`, `:425-455`): add a sibling inventory `diag-mutations-stic2.ids` rather than editing the stic loop. Scenario names: the discuss ids are `stic2-<feature>`; the repo convention is `<program>-<feature>-pass` (`diag-stic2-mob-pass`, `diag-stic2-idle-pass`); planner fixes the spelling once. The shape lint of the `golden` step (`golden.sh:150-214`) applies to tier `playable-guest-progress` only; these are `original-diagnostic`.

### Wave B - walker refactor (`core/src/machine/machine_digest.c`)

**Analog:** the file itself.
**Copy:** the row macro with an unused trailing `0u` (`:24-25`, struct `machine.h:158-166` `uint16_t reserved`); the one walker `ivs_digest_domain` (`:278-315`), the sink with stage buffer and optional caller buffer (`:172-230`), the test seam `ivs_machine_digest_preimage` (`:326-336`); fault-conditional row groups (`IVS_DF_PSG_ROWS`, `:55-71`) as the model for `SNAPSHOT_STIC_ROWS_OMITTED`; field accessors `ivs_machine_digest_field_count/field` (`:159-170`).
**Differs:** add the class (STATE, IMAGE_BEARING, BOUND, PAD) in `reserved`, a sink mode (HASH, BUFFER, BOTH) and one filter in the walker; the digest path passes "all rows" so bytes and hashes do not move. Tripwires: `data/expected/digest-v1-literals.ids`, `digest-v2-literals.ids`, `tests/machine/machine_digest.c` (`md_coverage :606` every byte in exactly one row, `md_sensitivity :765`, `md_padding_ignored :1169`) and `_Static_assert` on `IVS_MACHINE_BYTES 114416` (`machine.h:53`, `:132`). Note `mem.exec/grom/cart` sit in domain MEM (`:99-101`), not IMAGES: a state-only filter must key on class, not domain. Existing faults `DIGEST_*_ROWS_OMITTED` may also fail state ids once the walker is shared: check `faults-verdict` allowed prefixes. Stale text for the D-34 sweep: the digest comment in `ivs_machine.h` and `api-effects.json` say "composite version 2" while `IVS_DIGEST_COMPOSITE_VERSION` is 3 (`machine.h:156`).

### Wave B - public API (`ivs_state.h`) and ABI inventories

**Analog:** `ivs_machine_stic_state` (`core/src/machine/machine_inspect.c:110-142`) for out-struct handling; `ivs_machine_state_digest` (`machine_digest.c:347-400`) for the refusal order; `ivs_machine_load` (`ivs_machine.h:376-394`, `machine_load.c`) for validate-then-commit atomicity; `ivs_machine_storage_requirements` for an instance-free call.
```c
if (out == NULL || out->struct_size < IVS_STIC_REPORT_V1_SIZE || out->struct_size > IVS_STRUCT_SIZE_MAX) { return IVS_E_BAD_ARG; }
im = ivs_machine_from_handle(m);  if (im == NULL) { return IVS_E_BAD_HANDLE; }
if (ivs_machine_span_overlaps(out, sizeof *out, im, sizeof *im) != 0u) { return IVS_E_BAD_ARG; }
...
n = out->struct_size < IVS_STIC_REPORT_V1_SIZE ? out->struct_size : IVS_STIC_REPORT_V1_SIZE;
memcpy(out, &r, n); /* last: the caller's bytes past the v1 size are untouched */
```
**Copy:** `*_V1_SIZE` pinned define beside the struct (`ivs_run.h:89-91`); struct rule text (`ivs_machine.h:7-14`); reason block with the refusing status in each comment (`ivs_machine.h:281-309`, numbers 58+ go to `data/expected/api-reasons.ids`); status define, name table and name test: `ivs_status.h:26-40` (add `IVS_E_STATE_MISMATCH (-24)`, reserved range), `core/src/machine/machine.c:468`, `tests/machine/machine_create.c:773`, `data/expected/api-statuses.ids`; umbrella include `ivs.h:14-16`; call record `data/api/api-effects.json` (shape of `ivs_machine_state_digest`: `name`, `header`, `on_ok`, `precedence[{status, checks, pinned_by}]`, `notes`, `refusals[{status, effect, outs, tests}]`, checked by `tools/check/api-effects.cmake`).
**Inventories that move together (Phase 4 plan 22 pattern):** `data/expected/pkg-install-files.ids` (+`include/ivs/ivs_state.h`; the CMake install glob `core/CMakeLists.txt:156` picks it up), the presence loop `tools/check/steps/pkg-install.sh:385`, `data/expected/tests/headers.ids` (c17 and cxx rows), `data/expected/pkg-consumer-calls.ids`, `docs/packaging.md` ("ten files", Release content), `tools/check/core-symbols-allow.txt` only if a new import appears.
**Differs:** 64-bit lengths and capacities (as the run call), `rep` may be NULL on restore, a refused restore leaves the whole storage byte-identical (the `effect: none` oracle), restore sets phase POWERED and never touches slots.

### Wave B - IVSN container (`core/src/content/ivsn.[ch]`, `docs/formats/ivsn-v1.md`)

**Analog:** `core/src/content/ivsc.c/.h` (layer 1) and `docs/formats/ivsc-v1.md` (Conventions, Layout, Limits, Check order, Refusal reasons, Digests, Evolution, Worked example, Fuzzing).
**Copy:** magic with a high-bit first byte and a CR LF SUB tail (`ivsc.c:21-23`, `ivsc.h`); big-endian assembled from single bytes (`ivsc.c:34-42`); one `refuse(rep, status, reason, seg, off)` helper that fills the report (`ivsc.c:46-60`); numbered reasons each mapped to one status (`ivsc.h:41-62`); length checks before any byte is read (`ivsc.h:92-104`, doc "Check order" `:63-77`); trailer SHA-256 over all earlier bytes; writer byte order from `ivs_frame_be` (`stic_frame.c:112-118`) and `ivs_dsink_uint` (`machine_digest.c:211-222`). Layer 2 split mirrors `ivsc.c` versus `machine_load.c`.
**Differs:** section table (domains 1, 2, 4, 16-19), schema digest over field-table descriptors, profile parameter digest (hash of the generated `.inc` text from `cmake/Ivs{Memmap,Psg,Stic,Ctrl}Table.cmake`; confirm LF stability), three image-slot identities, per-row count checks, `ivs_snapshot_describe` instance-free; never carries image bytes (rights test scans the output for 16-byte windows of the loaded images). Literal vector labelled repeatability only: header in `data/expected/digest-v2-literals.ids` style (`# ... NEVER evidence of correctness`).

### Wave B - state suite, hard points, checklist, fuzz

**Analog:** `tests/run/run_slice.c` for the twin-and-memcmp oracle (reference stepped through the seam, `:6-12`; `rs_full_same` whole storage `:219-235`; programs as decle literals `:69-70`); `tests/machine/machine_refuse.c` for the refusal matrix (`RF_*` macros, `:1120-1123`); `tests/ivsc/{ivsc_main.c,ivsc_fuzz.c,ivsc_reencode.c}` for seeded families, canonical re-encode and `*.reasons-covered`; `tests/machine/machine_digest.c` for coverage-by-row.
**Copy:** suite skeleton and registration (`tests/CMakeLists.txt:120-123` for `ivsc`, `:325-352` for `video`, fault suite `:483-486`; `data/expected/tests/state.ids`, `data/expected/faults/state.ids` with the line shape of `faults/video.ids:4-7`); xorshift seeds with no host time (`ivsc_fuzz.c:1-40`, `stic_slice.c:17-24`); invariants per input (defined status, refusal writes only the report, acceptance re-encodes canonically, second attempt identical); poisoned-target restore (fill with `0xA5` using `ivs_machine_digest_field()` offsets, poison idiom `fuzz_ivsc.c:76-79`); witness scan over `ivs_machine_step_unit_internal` (as `rs_reference`). Controls: pure `tools/check/controls/<name>/{CHECK,EXPECT,...}` registered in `data/expected/controls/*.ids` for checklist defects (copy a `*-missing` control of `controls/listing.ids` style).
**Differs:** the checklist `data/expected/state-checklist.ids` (one line per table row: class, witness cut, test id) is machine-checked by a new check; row-omission sweep uses the runtime field accessor, not a fault rebuild; sweeps and the cut matrix on the release preset only, visible skip line (the lever-2 idea; skip needs `skip-ok` in the ids file); `fuzz_snapshot.c` copies `fuzz_ivsc.c:40-94` (reseal last 32 bytes, heap copy of exact size) and the option block `tests/CMakeLists.txt:375-386` (`IVS_ENABLE_LIBFUZZER`, `EXCLUDE_FROM_ALL`, never in the gate).

### Wave B - runner hook, installed consumer pass 5, rights

**Runner hook. Analog:** `--slice-ticks`. Parse table `runner/main.c:560-625` (range pair `lo`/`hi`, `seen` flag, value into `opts`), usage lines `:67-76`, options struct `runner/ivs_runner.h:455-466` (`flags` and `reserved` must be 0: add a named field, do not use them), slice loop and the between-slices call `runner/exec.c:968-1021` (`ex_between_slices`), docs row `docs/runner.md:28`. The test pattern is `check_repeat ... --slice-ticks` (`tests/runner-process-tap.sh:640-661`): every existing scenario with `--roundtrip-at-ticks` must reproduce its golden. **Differs:** the executor needs a second instance in the host storage (instance 114,416 bytes, storage cap `IVS_RUNNER_STORAGE_MAX 64 MiB`, `ivs_runner.h:355`); the report and goldens carry no new key.
**Consumer pass 5. Analog:** `scn_session` (`tests/installed-consumer/main.c:1498-1599`, `run_prefix = "scn."`, `keep_*` flags, observed-domain count rule `:1578-1581`) and the `main()` block (`:1748-1764`, two sessions and `second-instance equal`). Clone to `snap_session` on Lamp Tone (no new image arguments, `argc` stays 7, `:1665`): cut mid-frame, `ivs_machine_snapshot_save`, restore into a poisoned fresh machine, compare streams and the nine digests. Gate side: `rgold4` compare with prefix `snap\\.` (`tools/check/steps/pkg-install.sh:481-487`, `check_one_golden :447`), selftests cloned from `scn-digest-drift`/`scn-domain-missing` (`:777-795`), names in `data/expected/pkg-selftests.ids` (17 to 19), additions only to `data/expected/pkg-consumer.golden`, `pkg-consumer-calls.ids` (+3 or 4 names), claim shape CL-0024.
**Rights. Analog:** the audio and video refusals `runner/main.c:709-722` and the writer-side `.ivsc` rule (`.gitignore`, `tools/check/binary-ext.txt`): add `ivsn` to both (extension list is one per line, lower case, no dot), a line in `docs/owner-images.md`; tests copy `runner-process-tap.sh:744-761`.

### Wave B (optional last plan) - `--snapshot-out` / `--snapshot-in`

**Analog:** `--video-raw`: slot declaration and parse (`runner/main.c:497`, `:544-545`), exclusive create mode 0600 and cleanup by device and inode (`runner/hostio.h:42`, `main_open_out` at `runner/main.c:275`), same-path refusal (`:640-649`), docs `docs/runner.md:37-39`. **Differs:** read side goes through `ivs_hostio_read`; refused for `content ... user` exactly like the derived outputs (`main.c:709-722`); the core does no file IO.

### Wave C - `tools/bench`

**Analog:** `tools/pack/CMakeLists.txt` for a never-installed host tool with a repo-relative source list written to the build tree (`:20-24`, `:108-113`; copy the `file(WRITE ... ONLY_IF_DIFFERENT)` idiom, then register the list in `tools/check/steps/c-build.sh:43` and `tools/check/integrity.cmake:74` as in 07-PATTERNS, five registration points); `runner/CMakeLists.txt:21-45` for linking `ivs_runner_lib` and `ivs_core` (`ivs_runner_execute`, `ivs_runner.h:488`); `runner/hostio.c:60-70` (`ivs_hostio_now_ms`) for the one POSIX clock file (`bench_host.c`); `runner/main.c:394-470` for the report-capture-then-filter flow (`main_write_golden`: `write_report` into a buffer, `ivs_runner_golden_keep(line, n, live)` per line, `live[]` derived from `output.*.bytes`); `tests/runner/runner_exec.c:52-200` (`fake_host`, `fh_*`) for an in-memory host vtable; `tools/ci-record.sh:1-28` for TSV output, usage-from-header via `sed -n`, exit codes, `LC_ALL=C`.
**Differs:** iteration 0 compares the filtered report with `data/golden/<id>.digests` and refuses with `bench-refuse` and no timing field (D-22); `main_report_value` and `main_count_observed` are `static` in `main.c`: copy, do not link. Workload scenarios are ordinary `.ivss` plus `.digests` (`timeout-budget.digests` is the precedent for a golden without `.expect`; Lamp Tone idle/pass for the program); register rows in `runner-scenarios.ids` and ids in `runner-process.ids`. CTest case `bench-selftest` copies `tests/CMakeLists.txt:601-624` (`add_test(NAME pack ...)`, `SKIP_RETURN_CODE 77`). The negative control follows the "tool refuses on a mutated input" shape of `tests/pack-tap.sh` judge case `pack.judge.patched-word` (`pack-tap.sh:119-150`) and asserts no `ns` field. Output is never a claim (D-24), so no `claims.json` row and no important-gates row.

### Wave D1 - release machinery

- **`release.yml`.** Analog `.github/workflows/gate.yml` (SHA-pinned `actions/checkout`, `persist-credentials: false`, top-level `permissions: contents: read`, `timeout-minutes`); design text `.planning/AUTONOMY.md:166-189`. **Differs:** trigger `tags: ['v[0-9]*.[0-9]*.[0-9]*']`, job-level `contents: write`, `gh release create --draft --prerelease --verify-tag`, deterministic `git archive`/`tar --sort=name`/`gzip -n`, `SHA256SUMS` by `cmake -E sha256sum`, and the release job runs the installed consumer against the very tarball it ships (D-32).
- **`ci-parity` extension.** Analog `tools/check/steps/ci-parity.sh`: today every workflow must hold only `contents: read` (`:17-18`, awk `:100-110`), no `if:` (`:107`), and any `run:` other than the gate step is `ci-parity-extra-run` (`:115`); the gate-only rule is keyed by `isgate` (`:54`, `:94`). Add a per-file branch for `release.yml` (job-level write only there, a `run:` allowlist, no `pull_request` trigger, only `GITHUB_TOKEN`); controls copy `controls/ci-parity-write-permissions` and `ci-parity-extra-workflow-unpinned` (registered in `data/expected/controls/review-ci.ids:4,7`; step list `data/expected/gate-steps/ci.ids`).
- **`release-assets` step.** Analog for a gate step: `hygiene-history.sh` header contract (reads `ROOT CHECK_DIR FILELIST` only, `bad <token> <detail>`, final `-- <step> pass <stats>`) and gate discovery `tools/gate.sh:88-110` (a new `tools/check/steps/<name>.sh` needs a name in some `data/expected/gate-steps/*.ids`, else `gate-step-extra`). **Differs:** content-based, not extension-based: `tools/check/binary-ext.txt` lists `gz` and `tar`, so an extension rule would refuse its own assets; unpack, list members, require source members = `git ls-files` plus prefix and install members = `data/expected/pkg-install-files.ids`. Controls: extra member, `.rom` member, missing member, outer-name false positive (must pass).
- **`CHANGELOG.md` and the text scan.** Analog `tools/check/steps/text.sh`: the scope is `^(README\.md|docs/.*\.md)$` (`:21-22`), claim words `supports|compatible|passes|accurate|verified|runs|plays|playable` (`:44`), `[claim:CL-nnnn]` or `[nc]` per line. Add `CHANGELOG\.md` to the scope regex and a control copied from `text-untagged-playable` (`data/expected/controls/claims.ids:11`). Content model: `docs/packaging.md` "Release content (D-33)" (`:77-85`), claims table rendered from `data/claims.json`.
- **Version agreement.** `IVS_API_VERSION_*` macros `core/include/ivs/ivs.h:21-25`, `ivs_VERSION` from the root `CMakeLists.txt`, consumer `version` line (`main.c:1601-1607`); one awk check plus one control.
- **Rehearsal script.** `tools/release-rehearse.sh`: header/usage style of `tools/owner-run.sh` and `tools/listen-demo.sh` (SPDX, banner, `LC_ALL=C`, tool checks, exit codes); no `gh`.
- **`tools/release-readiness.sh`.** Analog `tools/ci-record.sh` (orchestrator-only banner, `--from-files` offline mode, one parser function, exit-code contract) and `tools/hygiene.sh` (`--tree`/`--history` modes, findings name rule and file, never the matched text). Local mode prints sections A, F, G, J as markdown; `--hosted` adds read-only `gh` calls (orchestrator only: the API rate limit is shared). `--selftest` with 4 controls.
- **History finding (D-33).** `hygiene --history` is a release step, not a gate step (`tools/hygiene.sh:6-7`); baseline `tools/check/hygiene-baseline.txt` must not move; the readiness record prints FAIL-with-disposition.
- **`config.json` footgun (D-31).** `.planning/config.json:15` `"create_tag": true`. The file is modified in the working tree now: stage it only in the deliberate, separate change.

### Close-out - claims, important-gates, planning records

- **Claims.** Analog CL-0023/CL-0024 (`data/claims.json` tail; keys `id statement profile tier artifact depends_on standing note`); `standing: provisional` by the weakest-field rule; `note` carries oracle, shared-misreading risk, "Excluded on purpose", "no measured tier (D10, D11)". Next free id after `CL-0024` is `CL-0025` (CL-0025/26/27 reserved by D-09); add each to `data/expected/claims.ids`. STA and PRF claim ids follow in the same sequence if written; bench numbers never become a claim.
- **important-gates.** Requirement list hard-coded at `tools/check/steps/important-gates.sh:71` (`nreq = split("API-02 ... SCN-01 SCN-02", ...)`): append `VID-05 STA-01 STA-02 PRF-01` (and `REL-01` if a gate is registered); rows `<gate> <fault|control|mutant> <id> <REQ>` in `data/expected/important-gates.ids`; `<suite>.judge.<slug>` and `<suite>.mutant.<slug>` ids are picked up automatically.
- **Verification record.** Analog `.planning/phases/07-first-visible-original-scenario/07-VERIFICATION.md`: gate-results table (`:17-24`), success-criteria table (`:36-41`), V-rows with check, fails-when and landed-in commits (`:47-68`). `08-MILESTONE-CLOSE.md` sections A-K have no tracked analog; section F is generated from `data/claims.json`, G from `pkg-install-files.ids`, J from `tools/hygiene.sh`.
- **CI-efficiency lines.** Analog `.planning/CI-EFFICIENCY.md` checkpoint prose and the one-line records `phase6-close: no-lever`, `checkpointB-07: lever-required`, `checkpointC-07: no-lever`, `phase7-start: lever-applied`, `phase7-close: no-lever`; rule: median `r` over the last five rows of `.planning/ci-runs.tsv` above 8.3 starts a lever plan (executors never call `gh` or `tools/ci-record.sh`). New lines: `phase8-start`, `checkpointA-08`, `checkpointB-08` (after the restore plan, D-37), `phase8-close`.
- **Stale-text sweep (D-34), exact places seen:** `README.md:17` ("No release has been published"), `:20` ("owner authority"); `.planning/ROADMAP.md:81,187,344,351` (D9/D1/D12 wording, "Nothing is published without owner gate D9"); `.planning/REQUIREMENTS.md:168-174` (D6, D7, D8, D12 shown open); `.planning/PROJECT.md:22-24` ("Validated: None yet"); `.planning/STATE.md:3,6` (`current_phase: 7`, `status: verifying`). SUMMARY frontmatter backfill: Phase 7 shows both forms, `requirements-completed: []` (`07-07-SUMMARY.md:52`) and `[SCN-01, SCN-02]` (`07-12-SUMMARY.md:55`); copy the populated form.

## Shared Patterns

### SPDX, ASCII, LF, no host paths
**Source:** `tools/check/integrity.cmake:58-76`; `tools/hygiene.sh` (home and scratch shapes, `@` before a dotted word, rejected everywhere including `.planning/`). Every `.c`, `.sh`, `.cmake`, `CMakeLists.txt` needs `SPDX-License-Identifier: MIT`; files under `data fixtures tools tests cmake runner core .github` must be ASCII with no CR. Apply to every new file; plan text uses repo-relative paths only.

### Gate step shape and registration
**Source:** `tools/check/steps/hygiene-history.sh:1-24`, `tools/gate.sh:88-110`. Inputs `ROOT CHECK_DIR FILELIST`, `bad <token> <detail>`, `-- <step> pass <stats>`; each token documented in the step header; a new step name goes in a `data/expected/gate-steps/*.ids` file.

### Negative controls
**Source:** `tools/check/run-controls.sh`, `tools/check/controls/<name>/{CHECK,EXPECT,overlay/|MAKE}`, registered in `data/expected/controls/<group>.ids`; a pure control costs about 0.07 hosted cpu-s, a symbol-layer one far more. D-10 caps wave A at 4 controls; wave B about 8; state the count in each plan before writing it.

### Named faults
**Source:** `data/expected/faults/video.ids:4-7` plus the inline `#ifdef IVS_FAULT_<ID>` site in the owner TU (`stic_cards.c:50-52`); the ONE source list rule (`tests/CMakeLists.txt:347` with `:483-486`). Fault objects build per owner TU: stems must stay unique across `core/src` and `runner/`.

### Pure helpers, explicit parameters, no hidden state
**Source:** `core/src/stic/stic_params.h:1-14` (Pattern A1), `stic_events.c:1-32`. No file-scope writable data in the core (`c-purity`, `core-symbols`); new TUs import only `memcpy/memset/memcmp` family names (`tools/check/core-symbols-allow.txt`).

### Hand-derived expectation headers
**Source:** `data/golden/diag-stic-fgbg-pass.expect:1-27`, `docs/video/demo-lamp-tone-1-derivation.md:1-8`. Text states: derived before the program and any run, not read back from the core, author of oracle and core is the same (shared misreading possible), claims stay provisional, "Excluded on purpose" list.

### Test registration and skips
**Source:** `data/expected/tests/*.ids`, `tools/check/steps/tests-inventory.sh`. Every id registered; a skip needs `skip-ok` and prints `-- SKIPPED <preset>/<suite>/<id>: <reason>`; `<suite>.judge.<slug>` and `<suite>.mutant.<slug>` double as important-gates mutants.

### Doc tripwires
**Source:** `tools/check/steps/text.sh:44`, `glossary.sh` (bare `clock` fails unless `input clock`, `STIC clock`, `PSG chip clock` or `[nc]`). Every new `docs/**/*.md` and `CHANGELOG.md`: claim words need `[claim:CL-nnnn]` or `[nc]`. `docs/formats/ivsn-v1.md`, `docs/bench.md`, `docs/state/*.md` are scanned.

### Manifest refresh and tracked-source rule
**Source:** `fixtures/FX-demo-lamp-tone-1/manifest.json` (`note`), `tools/check/manifests.cmake` recipe hashing. Edit of a listed output or of `tools/asm/asm.c`, `tools/pack/pack.c` refreshes `sha256` and `size_bytes` in the same commit. Plans reference tracked sources only, never `build/<preset>/images/*`; `.gitignore` lists `*.ivsc` but not `*.ivsn`.

## No Analog Found

| File | Role | Data flow | Reason |
|---|---|---|---|
| MOB/collision frame literals with unknown (`-`) cells and a collision truth table | oracle + checker | transform | No frame has an overlay or an unknown cell; `video.cmake` grammar must be extended (see Wave A oracle). Closest: `F-D1.frame`, lamp derivation section (c). |
| `08-MILESTONE-CLOSE.md` sections A-K and the `<stage2>` placeholder check | records | - | First milestone close; only the shape of `07-VERIFICATION.md` is reusable. |
| `release.yml` tag-triggered write job and the draft-release flow | CI workflow | event-driven | Only `gate.yml` exists (read-only, no tags); design text in `.planning/AUTONOMY.md:166-189`; first exercised at the tag, so the rehearsal script is the pre-tag proof. |

## Metadata

**Analog search scope:** `core/src/{stic,machine,memmap,content}`, `core/include/ivs`, `tests/{video,stic,machine,run,ivsc,fuzz,runner,installed-consumer}`, `tests/CMakeLists.txt`, `tests/runner-process-tap.sh`, `tools/{pack,asm,check,ci-record.sh,gate.sh,hygiene.sh,owner-run.sh}`, `runner/`, `fixtures/`, `data/{expected,golden,scenarios,video,stic,profiles,api}`, `docs/{video,stic,formats,runner.md,packaging.md}`, `cmake/`, `.github/workflows`, `.planning/{AUTONOMY.md,CI-EFFICIENCY.md,ci-runs.tsv}`.
**Files scanned:** about 90 read in whole or by range; every cited path verified tracked with `git ls-files`.
**Pattern extraction date:** 2026-11-14
**Plan-time re-checks:** cited line numbers; next free discrepancy id (C107) and claim id (CL-0025); `r` median from `.planning/ci-runs.tsv`; whether `IVS_STIC_CARDROW_MAX` must grow for vertical delay; the `video.cmake` grammar extension (blocks the oracle plan); `faults-verdict` prefixes once the digest walker is shared; byte-stability of the four generated `.inc` texts for the profile parameter digest; `tests-skipped` baseline (4 now) if release-only sweeps use `skip-ok`.
