# Phase 7: First Visible Original Scenario - Pattern Map

**Mapped:** 2026-11-13
**Files analyzed:** 47 new or modified paths (grouped from the RESEARCH "File inventory")
**Analogs found:** 43 / 47 with a tracked analog (4 partial or none, listed under "No Analog Found")
**Scope note:** every path below is repo-relative and was checked with `git ls-files` (tracked-source gate, #3645); no `build/` or install mirror path is named. Line numbers are of the tree at mapping time; re-check after any edit to a cited gate step. Excerpts are short on purpose: open the analog for the full text.

## File Classification

| New / modified file | Role | Data flow | Closest analog | Match |
|---|---|---|---|---|
| `tools/asm/asm.c` | tool (C17, build-only) | batch / transform | `tools/pack/pack.c` (grammar, banner, exit rules), `tools/pcm/ivs_refpcm.c` | role-match |
| `tools/asm/CMakeLists.txt`, root `CMakeLists.txt`, `c-build.sh`, `integrity.cmake`, toy tree, 2 controls | registration | batch | `tools/pcm/CMakeLists.txt` and its five registration points | exact |
| `tests/asm-tap.sh`, `tests/asm/**`, `data/expected/tests/asm.ids`, `tests/CMakeLists.txt` | test suite (shell TAP) | batch | `tests/pack-tap.sh`, `data/expected/tests/pack.ids` | exact |
| `fixtures/FX-boot-stub-2/*`, `listings.ids`, `fixtures.ids` | fixture (stub contract 2) | event-driven hand-off | `fixtures/FX-boot-stub/*` | exact |
| `runner/{ivs_runner.h,scenario.c,exec.c,report.c}`, `docs/runner.md`, `tests/runner/{runner_parse,runner_exec,runner_fuzz}.c`, `runner.ids` | runner token | request-response | the `boot-stub` token in the same files | exact |
| `tools/pack/CMakeLists.txt`, `tests/CMakeLists.txt`, `tests/runner-process-tap.sh` copy list, `tests/run/run_stub*.c`, `run.ids` | image plumbing + test | file-I/O | `diag-stic-1` image block, `tests/run/run_stub.c` | exact |
| `fixtures/FX-demo-lamp-tone-1/*` | fixture (ISR program, first recipe) | event-driven | `fixtures/FX-diag-stic-1/*` (no ISR analog) | role-match |
| `tools/check/manifests.cmake`, `files.cmake` + controls | gate checker | batch | output-hash block; path loop of the same files | exact |
| `docs/video/demo-lamp-tone-1-derivation.md`, `data/video/frames/F-S*.frame`, `video-frames.ids` | oracle literals | transform | `docs/video/diag-stic-1-derivation.md`, `F-D1.frame` | exact |
| `tools/check/video.cmake` (`# pair:` token) + control | gate checker | transform | its `video-frame-hash-expect-drift` block | exact |
| `tools/check/steps/golden.sh` (shape lint, consistency) + controls | gate checker | transform | its `.expect` branch | exact |
| `data/scenarios/demo-lamp-tone-{pass,idle,shift,swap}.ivss` | scenario | event-driven | `diag-ctrl-psg-pass.ivss`, `diag-stic-fgbg-pass.ivss` | exact |
| `data/golden/demo-lamp-tone-{pass,idle}.{expect,digests}`, `runner-scenarios.ids`, `runner-process.ids` | golden + inventories | transform | `data/golden/diag-stic-fgbg-pass.*` | exact |
| `diag-mutations-scn.ids` + `runner-process-tap.sh` (`fails=`, `--exec` patch) | mutant harness | batch | `diag-mutations-stic.ids`, `run_mutant` | exact |
| `scenario-pairs.ids` + judge cases | two-report judge | transform | `runner-process.judge.*` self-controls | partial |
| `tests/installed-consumer/main.c`, `pkg-consumer.golden`, `pkg-selftests.ids`, `pkg-install.sh` | installed replay 4 | request-response | `stic_session`, `ctrl_session` | exact |
| `docs/owner-images.md`, `fixtures/FX-owner-{exec,grom}/manifest.json`, `d10-procedure.md` anchors | docs + manifests | - | `docs/stic/d10-procedure.md`, `FX-sample-rights-unknown` | role-match |
| `data/scenarios/owner-exec-title.ivss` | scenario (skip-ok) | request-response | `needs-user-exec.ivss` | exact |
| `tools/owner-run.sh` (+ tests) | operator helper | file-I/O | `tools/listen-demo.sh` | exact |
| `runner/main.c` (derived-output refusals) | runner CLI | request-response | `main.c:697-707` refusal | exact |
| `tools/check/steps/tests-inventory.sh` (`-- SKIPPED`) | gate step | batch | its own SKIP handling | exact |
| claims, `text.sh`, `important-gates.sh`, README tidy | close-out | - | CL-0021/0022, `important-gates.sh:71` | exact |
| `tools/check/vectors.cmake`, `tools/review/vector-blind.cmake` (W0) | gate checker | batch | none | none |

## Pattern Assignments

### `tools/asm/asm.c` (tool, text-to-text)

**Analog:** `tools/pack/pack.c` (banner, grammar, refusal style), `tools/pcm/ivs_refpcm.c` (ISO C only, no globals needed beyond static tables).

**Banner and exit contract** (`tools/pack/pack.c:1-34`): SPDX first line, then one comment stating purpose, grammar, modes, exit status:
```c
/* SPDX-License-Identifier: MIT */
/* ivs_pack: ... a development tool: never installed, never shipped, never part of the core.
 * It writes only the files named on its command line ..., and its output holds no time, path or
 * host-dependent byte. ... Exit status: 0 on success, 1 on any refusal (one 'ivs_pack: ...' line on stderr,
 * line-numbered for a listing error). */
```
**Output grammar to generate** (`fixtures/FX-diag-stic-1/diag-stic-1.listing.txt`, header keys `tools/check/listing.cmake:77`): `# id/kind/profile/window/label/entry/stack`, then `ADDR HEX4 BITS ROW ASM`, `-` row for operand words, `-> ADDR` on every branch line:
```
5008 0204 1000 00 0100   BZE   BZE $500C -> 500C   no key: skip the Color Stack mode select
5009 0002 00 0000 0010   -     displacement 0002 (S=0): 500A + 0002 = 500C
```
Branch arithmetic (same listing, "Branch arithmetic"): forward `target - (pc + 2)`, backward `(pc + 1) - target`; an immediate above `03FF` is `SDBD` + `MVII` with low byte then high byte (lines 501C-501F).
**Not copied:** `ivs_pack` links `ivs_core` for SHA-256; `ivs_asm` links nothing from the core. The encoding table is hand-written from `docs/cp1610/reference-notes.md`; `data/cp1610/opcode-families.json` (rows such as `"id": "HLT", "pattern": "0000000000"`) is only the checker's police, never read at run time (D-17, D-18).
**Header-key trap (X1):** per-label output must not use `# label:` (a `HEADER_KEYS` member, `listing.cmake:77`; a repeat is `listing-header`, `:363-367`). Use a non-header key such as `# sym: loop 5026`.

---

### `tools/asm/CMakeLists.txt` + registration surface

**Analog:** `tools/pcm/CMakeLists.txt` (28 lines). Copy the explicit source list and the build-tree list file:
```cmake
set(IVS_PCM_SOURCES ivs_refpcm.c)
add_library(ivs_refpcm STATIC ${IVS_PCM_SOURCES})
target_link_libraries(ivs_refpcm PRIVATE ivs_warnings ivs_san)
foreach(src IN LISTS IVS_PCM_SOURCES)
  string(APPEND listed "tools/pcm/${src}\n")
endforeach()
file(WRITE "${CMAKE_BINARY_DIR}/ivs-pcm-sources.txt.tmp" "${listed}")
file(COPY_FILE ".../ivs-pcm-sources.txt.tmp" ".../ivs-pcm-sources.txt" ONLY_IF_DIFFERENT)
```
Change: `add_executable(ivs_asm ...)` (as `tools/pack/CMakeLists.txt:20`), list file `ivs-asm-sources.txt`, no install rule, header comment "never installed or shipped".
**The five registration points (D-17, F16, F17), each a copy of the `pcm` entry:**
1. `CMakeLists.txt:36-38`: `add_subdirectory(tools/asm)` before `enable_testing()` (line 40).
2. `tools/check/steps/c-build.sh:43`: add `'^tools/asm/.*[.]c$ ivs-asm-sources.txt'` to `for mod in ...`; refresh the header comment naming the four lists.
3. `tools/check/integrity.cmake:74`: `^tools/(pack|cart|pcm)/.*[.](c|h)$` gains `asm`; update the comment above.
4. Toy tree: `tools/check/controls/_good/tree/CMakeLists.txt` (`file(WRITE ".../ivs-pcm-sources.txt" "tools/pcm/good.c\n")` becomes a fifth line) and `tools/check/controls/_good/tree/tools/asm/good.c` (copy `tools/pcm/good.c`).
5. Controls in `data/expected/controls/runner-hygiene.ids`: `c-build-unlisted-asm-source` (copy `controls/c-build-unlisted-pcm-source/{CHECK,EXPECT,overlay/tools/pcm/extra.c}`; EXPECT `FAIL c-build-unlisted-source: tools/asm/extra.c`) and `integrity-missing-spdx-tools-asm` (copy `controls/integrity-missing-spdx-tools-pcm/`; MAKE `write tools/asm/x.c x`; EXPECT `FAIL integrity-missing-spdx: tools/asm/x.c`).

---

### `tests/asm-tap.sh`, `tests/asm/**`, `data/expected/tests/asm.ids` (shell TAP suite)

**Analog:** `tests/pack-tap.sh` + `data/expected/tests/pack.ids` + `tests/CMakeLists.txt:580-581`:
```cmake
add_test(NAME pack COMMAND sh "${CMAKE_SOURCE_DIR}/tests/pack-tap.sh" "${CMAKE_SOURCE_DIR}" "$<TARGET_FILE:ivs_pack>")
set_tests_properties(pack PROPERTIES SKIP_RETURN_CODE 77 ENVIRONMENT "CMAKE=${CMAKE_COMMAND}")
```
**TAP skeleton** (`pack-tap.sh:31-60,169-181`): `mktemp -d` + `trap`, counters `n executed skipped failed`, `report()` printing `ok|not ok N <id>` plus `# <why>` lines, header `# suite=<s> nonce=<IVS_TEST_NONCE>` first, trailer `# suite=%s executed=%s skipped=%s failed=%s`, copy to `IVS_TEST_OUT_DIR`, exit 77 when `executed` is 0.
**Two-implementation compare:** pack compares `ivs_pack --dump-words` with `cmake -DROOT=... -DDUMP=<id> -P tools/check/listing.cmake` byte for byte. For `asm.regen-<id>`: run `ivs_asm`, `cmp` with the committed listing, then the same `--dump-words` comparison.
**Judge self-control** (`pack-tap.sh:119-150`, `pack.judge.patched-word`): the comparison must fail on a mutated input with the diff showing that one line only (`judge-vacuous`, `judge-wrong-reason`). Model `asm.judge.mutated-twin` on it; an id `<suite>.judge.<slug>` is automatically an `important-gates` mutant (`important-gates.sh:53-56`).
**Corpus:** `tests/asm/pairs.txt` is a flat copy of the `asm` / `program` fields of `data/cp1610/vectors/*.json` (e.g. `"asm": "B $1002", "program": ["0200 1000 00 0000", ...]`); no third JSON parse (D-18). Negatives pinned by `data/expected/asm-tokens.ids`: single `ivs_asm: ...` stderr lines judged by exact token.

---

### `fixtures/FX-boot-stub-2/*` (fixture, interrupt hand-off)

**Analog:** `fixtures/FX-boot-stub/boot-stub.listing.txt` + `manifest.json`; guards at `tools/check/listing.cmake:578-630`.
**Header** (listing 1-8): copy, set `# contract: 2`; window stays `1000-101F`, `# entry: 5000`, `# stack: 0340`; the text states the `$5000`/`$5004` convention.
```
# id: boot-stub
# kind: boot-stub
# contract: 1
# window: 1000-101F
# label: project boot stub, not an EXEC
```
**Reset path kept byte-identical** (listing 1000-1015): `B $1010` at 1000, `DIS`, `MVII #$0340,R6`, three-decle `J $5000` at 1013-1015. Contract 2 replaces the trap block 1004-1009 with one `JD $5004` at the interrupt entry `1004` (D-10).
**Guards** (`listing.cmake:578-630`): addresses inside `1000-101F` (`bootstub-size`), at most 16 non-zero words, rows only from `data/expected/bootstub-rows.ids` (`HLT DIS EIS MVII MVO TSTR B J`; a `JD` word decodes as row `J`, assumption A6, prove it on the listing step), non-zero words at `reset.r7` and `interrupt.iab_value` (`bootstub-vector`).
**Manifest:** copy `fixtures/FX-boot-stub/manifest.json` keys (kind `original-source`, `licence_evidence`, `redistributable: yes`, `allowed_use`, `rights_review: author-attested`); `.asm` and listing both in `outputs` (else `files-unlisted-fixture-file`, `files.cmake:146`). Inventories: `listings.ids` line `boot-stub-2 boot-stub fixtures/FX-boot-stub-2/boot-stub-2.listing.txt`; `fixtures.ids` exact set; `pack.words-boot-stub-2` in `data/expected/tests/pack.ids`.

---

### Runner token `boot-stub-2` (request-response)

**Analog:** the `boot-stub` token, five touch points; add kind 5.
- `runner/ivs_runner.h:123-128` content kinds (`BOOT_STUB 3u`, `ZERO_GROM 4u`), `:361` `IVS_RUNNER_BOOT_STUB_FILE "boot-stub.exec"`, header comment `:58`.
- `runner/scenario.c:344-355` (`dir_content`):
```c
} else if (tok_is(v, "boot-stub")) {
    if (role != IVS_SCN_ROLE_EXEC) { return IVS_SCN_R_BAD_CONTENT_KIND; }
    c->kind = IVS_SCN_CONTENT_BOOT_STUB;
```
- `runner/exec.c:205-207` (`ex_read_role`) and `:241-242` (`ex_content_name`): both need an arm (the `default:` is `IVS_RUNNER_READ_IO_ERROR`).
- `runner/report.c:312-321` (`RF_LABEL_EXEC`): label from the symbolic kind only, "never from the bytes or a table of hashes" (D-30); stub 2 names contract 2 in the label.
- Fuzz bound `tests/runner/runner_fuzz.c:320` (`c->kind <= IVS_SCN_CONTENT_ZERO_GROM`) moves to the new maximum; line 323 gets a sibling exec-only rule.
**Tests:** `runner.parse.refuse.bad-content-kind` (`runner_parse.c:330`), `parse_diag_boot_pass` (`:505-525`), fake host `fh_add(h, "boot-stub.exec", ...)` (`runner_exec.c:224`); register ids in `data/expected/tests/runner.ids`; docs `docs/runner.md:59` (token row) and `:112` (`exec:` label).

---

### Image plumbing and `run.stub2.*`

**Analog:** the `diag-stic-1` block, `tools/pack/CMakeLists.txt:62-68`:
```cmake
add_custom_command(OUTPUT "${IVS_DIAG_STIC_1_IVSC}"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${IVS_IMAGES_DIR}"
  COMMAND ivs_pack --ivsc "${IVS_DIAG_STIC_1_LISTING}" "${IVS_DIAG_STIC_1_IVSC}"
  DEPENDS ivs_pack "${IVS_DIAG_STIC_1_LISTING}" ...)
```
The stub uses the `--exec` block (`:29-37`); add both outputs to `add_custom_target(ivs_images ALL DEPENDS ...)` (`:69`) and `PARENT_SCOPE`. Lists that grow together (F8): `tests/CMakeLists.txt:150-155` (`ivs_pack --c-include ... "boot_stub=..."`), `tests/runner-process-tap.sh:102` (`for f in boot-stub.exec diag-cpu-1.ivsc ...; do cp`), `tools/check/steps/pkg-install.sh:514-519`.
**In-process test analog:** `tests/run/run_stub.c` (ids `run.stub.path`, `run.stub.trap`, `run.stub.halt-needs-follower`, `data/expected/tests/run.ids:239-244`): literals worked out from the listing, `rs_stub_image()` heap copy, `rs_powered()`, one run call per unit, `RS_CHECK` with `goto done`; `rs_path_r7[] = {0x1000u, 0x1010u, 0x1011u, 0x1013u, 0x5000u}` (line 62). New ids `run.stub2.path`, `run.stub2.handoff` (INTR pulse enters `1004`, then `5004`); expected values are literals, "never read back from the code under test" (file header). `STUB2_JD_AS_J` is a named-fault-style control.

---

### `fixtures/FX-demo-lamp-tone-1/*` (fixture, ISR program)

**Analog:** `fixtures/FX-diag-stic-1/diag-stic-1.listing.txt` + `manifest.json` (section table, derivation-of-encodings block, "Words used" line); `fixtures/FX-diag-ctrl-psg-1/diag-ctrl-psg-1.listing.txt` for PSG writes and port reads (`MVI $01FE,R0` then `ANDI #$00FF,R0`, stic listing 5000-5003). No cartridge has an ISR: the stub trap block and one `PULR` (`diag-cpu-1.listing.txt:87`) are the only partial analogs; derive the ISR (D-11..D-13) before assembling.
**Header to copy** (stic listing 1-6): `# id: ivs-demo-lamp-tone-1`, `# kind: cart`, `# profile: intv-2609-ntsc-family-p1`, `# window: 5000-xxxx`, `# entry: 5000`, `# stack: 0340`. The listing is generated by `ivs_asm` from `lamp-tone.asm` (source of truth).
**Manifest:** copy `fixtures/FX-diag-stic-1/manifest.json`; `outputs` = `.asm` + listing (size and sha256 refreshed in the same commit as any edit); first non-null recipe (shape checked at `manifests.cmake:210-216`):
```json
"recipe": { "tool": "ivs_asm", "tool_version": "...", "tool_sha256": "<64 hex>", "tool_licence": "MIT",
  "command": "ivs_asm ... ; ivs_pack --ivsc ...", "sources": [ { "path": "fixtures/FX-demo-lamp-tone-1/lamp-tone.asm", "sha256": "<64 hex>" } ] }
```
Add `ivs-demo-lamp-tone-1 cart <listing>` to `listings.ids` (header `# id:` must equal it) and a `pack.words-`/`pack.trailer-` pair to `pack.ids`. Rewrite the manifest `note` ("no assembler and nothing copied") for the assembler recipe plus the third-party attestation (glyphs, tunes, EXEC calls, GROM reads; D-28).

---

### `tools/check/manifests.cmake` recipe hashing and `tools/check/files.cmake` names rule

**Analog 1:** output-hash block `manifests.cmake:285-330`; copy the loop over `recipe sources`:
```cmake
file(SIZE "${ROOT}/${op}" asz)
file(SHA256 "${ROOT}/${op}" asha)
if(NOT asz EQUAL osz OR NOT asha STREQUAL osha)
  gate_fail(manifest-digest-mismatch "${lbl} ${op}")
```
Recipe sources have `path` and `sha256` only (`check_entries`, `manifests.cmake:89-117`); use a new token (e.g. `manifest-recipe-digest-mismatch`). Control `manifests-recipe-source-digest` copies `controls/manifests-digest-mismatch/{CHECK,EXPECT,MAKE}` (`MAKE: write <path> changed`), but the toy `FX-good` manifest has `"recipe": null`, so the overlay supplies one with a recipe. `manifest-lib.cmake:54-58` already counts recipe sources as accounted files.
**Analog 2:** `files.cmake:103-150` loop `foreach(p IN LISTS paths)`: the names rule is one more `if(p MATCHES ...)` / `gate_fail(files-trademark-name "${p}")` (case-insensitive forbidden word in fixture, scenario and target names; name rule in `AGENTS.md`). Control copies `controls/files-fixture-without-manifest/{CHECK,EXPECT,MAKE}`; register both in `data/expected/controls/rights.ids`.

---

### Oracle: `docs/video/demo-lamp-tone-1-derivation.md`, `data/video/frames/F-S*.frame`, `video-frames.ids`

**Analog:** `docs/video/diag-stic-1-derivation.md` (sections (a) Program design, (b), (c) Cycle budget, (d) Expected state and frames, (e) D4 judgement, (f) As built) and `docs/input/diag-ctrl-psg-1-derivation.md` ((b) Write list asserted by the scenario, (c) Polled values from the literal, (e) Mutants derived by hand). Frame literal analog `data/video/frames/F-D1.frame`:
```
frame F-D1
tier original-diagnostic
oracle independent-derivation
mode fgbg
cell 8 3 2A01 | fgbg fg=1 bg=5 gram card=0
gram 0 11111111 00000000 ...
sha256-text 86b80eb7...
plane
00000000 00000000 ...     (96 rows of 20 groups of 8 symbols)
```
Rules: derive from the controller literal only (`data/input/intv-2609-ntsc-family-p1.literal.txt`: `KEY_1`, `KEY_9`, `DISC_E`, `SIDE_TOP`, `SIDE_LOWER_LEFT`), never from the program or a run (D-02); state the band arithmetic (X2: frame-1 band, marker `$02F0`, counter `$02F1`, `floor(T/59,736)`), re-arm margin of both forms (X3), audio from constant-level states only (D-22), assert count under 64 (X5). Register each frame id in `data/expected/video-frames.ids` (exact set).

---

### `tools/check/video.cmake` `# pair:` token

**Analog:** hash tie `video.cmake:714-755` (`video-frame-hash-expect-drift`): loop `data/golden/*.expect`, read header lines (regex `^# frame: ([0-9]+) ([A-Za-z0-9-]+)$`, line 722), resolve `data/video/frames/<id>.frame`, compare. The new token reads a `# pair:` header naming two frames and a rectangle and requires the differing positions to equal it. Reuse `gate_lines`, `gate_unprotect`, `pfail` (`:60`), `paths_matching`; add the token to `VTOKENS` (`:50-53`), the header token list (`:35-40`) and `gate_stat`. Control `video-frame-pair-region-drift` copies `controls/video-frame-hash-expect-drift/` (`CHECK video`, `EXPECT FAIL video-frame-pair-region-drift:`, overlay `data/golden/toy.expect` + toy frames; toy tier is `source-expectation`); register in `controls/video.ids` and `important-gates.ids`.

---

### `tools/check/steps/golden.sh` shape lint and consistency rule

**Analog:** `.expect` branch `golden.sh:207-214` and the user-scenario awk at `:199`:
```sh
if [ "$kind" = expect ]; then
  awk '$1 == "assert" { $1 = $1; print }' "$ROOT/$sf" > "$tmp/want"
  awk '/^#/ || NF == 0 { next } $1 == "assert" { $1 = $1; print; next } { print "STRAY " $0 }' "$ROOT/$g" > "$tmp/have"
  ... bad golden-expect-drift "$g (assert lines differ from $sf)"
```
The lint runs in this branch, keyed on the tier the header awk already parses (`:150-175`, variable `t`; also print `V tier <t>`). For tier `playable-guest-progress` require in `$sf`: an `input` line, a `mem`/`reg` assert after the first input, a `frame` assert and a `psglog` assert, last input tick below the last `run-to` (pitfall 6). New tokens go in the header comment (`:56-58`) and use `bad <token> "<detail>"` (`:51`). Two controls copy `controls/golden-expect-drift/` (overlay `data/golden/toy.expect`); register in `controls/golden.ids`.
**Consistency rule (D-31):** `awk '$1 == "content" && $3 == "user"'` (`:199`) defines a user scenario; require `skip-ok` in column 5 of `runner-scenarios.ids` exactly for those (column rule `:89`, `NF != 6`); two controls, both directions.

---

### Scenarios `data/scenarios/demo-lamp-tone-*.ivss` and goldens

**Analog:** `data/scenarios/diag-ctrl-psg-pass.ivss` (input script, psglog asserts; header comment says the `.expect` is the same asserts in the same order) and `diag-stic-fgbg-pass.ivss` (`capture video 0 1`, pixel and hash asserts):
```
ivss 1
id diag-ctrl-psg-pass
profile intv-2609-ntsc-family-p1
content exec boot-stub
content grom zero-grom
content cart diag-ctrl-psg-1.ivsc
capture psglog
input at_tick=5600 c0=KEY_1+KEY_9 c1=SIDE_LOWER_LEFT
run-to 24000 ticks stop halted
assert psglog 0 addr=0 value=0023 source=cpu-write
```
Changes: `content exec boot-stub-2`; `capture video 0 13` and `capture psglog` (and `capture audio` if `audio at` is used); `run-to 806568 ticks` without `stop halted` and `assert halted == no` (D-05); asserts `frame <k> displayed == yes`, `frame <k> hash == <64 hex>`, pixel probes, mem asserts at several `run-to` steps, `audio at <tick> A=<n> B=<n> C=<n>` (tick a multiple of `audio.sample_ticks`, 16 on this profile). Grammar and caps: `docs/runner.md:64-87`.
**Goldens:** `.expect` copies the header of `data/golden/diag-stic-fgbg-pass.expect` (`# tier:` here `playable-guest-progress`, `# oracle: independent-derivation`, `# scenario:`, `# profile:`, `# frame: 0 F-S1` tie, excluded-on-purpose list). `.digests` is written only by `ivs-run --update-golden`; header `# tier: deterministic-repeatability`, `# oracle: self-recorded`, `# domains: cpu=observed mem=observed images=observed instance=observed video=observed audio=observed input=observed psglog=observed`. The `.expect` enters with its scenario, not in W1 (X7). Inventory row (six columns): `demo-lamp-tone-pass 0 pass deterministic run -`; ids `runner-process.scenario.<id>`, `repeat`, `slices`, `cwd` copy the `diag-stic-fgbg-pass` ones.

---

### Mutants (`data/expected/diag-mutations-scn.ids`, `tests/runner-process-tap.sh`)

**Analog:** `data/expected/diag-mutations-stic.ids` (header table + rows) and the loop at `runner-process-tap.sh:433-462` with `run_mutant` (`:356-371`):
```
STIC-WORD-MOVED diag-stic-fgbg-pass 5057 02EF 02EE mem 02EF == 1C0F guest-fail 10
STIC-PAD-NEGATIVE diag-stic-fgbg-pass 5059 0384 0320 - pass 0
```
```sh
"$pack" --dump-words "$listing3" > "$tmp/words3"     # explicit <from> word checked first
run_mutant "$mid" "$listing3" diag-stic-1.ivsc "$scn" "$addrs" "$tos" "$mx" "$mo" "$mp"
result "runner-process.mutant.$mid"
```
Changes: (1) `run_mutant` copies only `boot-stub.exec` into the mutant dir (`:359`): copy `boot-stub-2.exec` too; (2) stub mutants (`JD` to `J`, pad too short, ...) use `ivs_pack --exec <listing> <out> --patch ADDR=HEX` (`pack.c:17-23`), a sibling of `run_mutant`; (3) the optional `fails=` column judges all failing `step.<k>.assert.<j> fail ...` report lines, the old column still judges the first (`outcome.detail assert=<j> line=<n> <clause> got <actual>`); old rows stay valid; (4) a negative control row (`pass`). Register ids in `runner-process.ids` and `important-gates.ids` (`diag-stic-mutants mutant runner-process.mutant.STIC-CARD-BYTE VID-04` is the form).

### `scenario-pairs.ids` two-report judge

**Partial analog:** `self_control` (`runner-process-tap.sh:863-896`; fake runner scripts replaying recorded reports, passes only when the check fails with the intended token) and `check_repeat`/`same_body` (`:273-300`, byte-equal apart from `host.calls`). The new judge reads `$tmp/runs/scen-<id>.out` for `pass` and `idle` and requires `digest.video`, `digest.audio`, `digest.psglog`, `digest.input`, `digest.mem` to differ and `input.consumed == input.events`; three `runner-process.judge.<slug>` self-controls prove it fails on a fake.

---

### Installed-consumer replay 4 (`tests/installed-consumer/main.c`, `pkg-install.sh`)

**Analog:** `stic_session` (`main.c:865-931`), `stic_frame_stop_pass` (`:799-858`), `ctrl_events` (`:522-540`):
```c
q.flags = IVS_RUN_F_STOP_ON_HALT | IVS_RUN_F_STOP_ON_FRAME;
q.video = buf;  q.video_capacity = slot;                      /* slot = info->video_slot_bytes */
printf("stic.frame-stop.1 stop=%s now_ticks=%llu frames=%llu frame0-equal=%s\n", ...);
ev[0].at_tick = 4000u;  ev[0].controller[0] = IVS_CTRL_KEY_1; /* core/include/ivs/ivs_run.h:242 */
```
Copy-paste (D-26), no refactor: `scn_powered()` (clone of `stic_powered`, `:681-715`, loading a fourth cart), `scn_session(print, d, observed)` with `run_prefix = "scn."`, `keep_video = 1`, the observed-domain count rule (`observed[5..8] = delivered_*_bytes != 0u`, `observed[7] = consumed != 0u`, `:909-915`), a `STOP_ON_FRAME` second pass, own region diff (clone `stic_rect_mismatches`, `:722`), decode of audio and write-log bytes. `main()` (`:990-1074`): `argc != 5` becomes 7 (stub 2 and the program), usage text, two more `read_image` calls, then the block `scn_session(1,...)`, `printf("scn.digest.%s ", ...)`, `scn_session(0,...)`, `scn.second-instance equal=`. Update the narrating header comment (lines 10-60).
**`pkg-install.sh`:** `img_cart3=...` (`:514-515`) gains the two new images; missing-image test (`:517-519`); consumer call `:531` (`"$cbld/consumer" "$img_exec" "$img_cart" "$img_cart2" "$img_cart3"`); `rgold4` + `check_one_golden "$rgold4" "scn\\." "$1"` in `check_runner_golden` (`:473-481`); selftests `scn-domain-missing`, `scn-digest-drift` are clones of `stic-domain-missing`/`stic-digest-drift` (`:746-766`); names in `data/expected/pkg-selftests.ids` (15 to 17). `pkg-consumer.golden`: additions only (lesson 2). CL-0024 copies CL-0022 in `data/claims.json` (tier `installed-host-behavior`, artifact `tests/installed-consumer/main.c`).

---

### SCN-02 route

**`docs/owner-images.md`, `fixtures/FX-owner-{exec,grom}/manifest.json`:** analogs `docs/stic/d10-procedure.md` / `docs/input/d10-procedure.md` (operator procedure; add `#owner-exec` anchors) and `fixtures/FX-sample-rights-unknown/manifest.json` (uncommitted, non-redistributable; guards `manifest-committed-not-redistributable`, `manifest-uncommitted-output-tracked`, `manifests.cmake:275-300`). Both new manifests `kind: system-image`, `redistributable: no`; add to `fixtures.ids`. Doc tripwires apply.
**`owner-exec-title.ivss`:** analog `needs-user-exec.ivss` (comment, `# expect-detail: missing role=exec user`, `content exec user`, `content grom zero-grom`); change to `content grom user`, no result asserts, keep a `run-to` (open question 6: prove the parse). Row copies `needs-user-exec 13 missing-fixture deterministic skip-ok -` (`runner-scenarios.ids`); `runner-process.scenario.owner-exec-title skip-ok` in `runner-process.ids`. The loop already turns a passing skip-ok into `# SKIP` (`runner-process-tap.sh:340-341`); `golden.sh:199-201` refuses a golden for a user scenario.
**`tests-inventory.sh`:** the awk already reads `ok N <id> # SKIP <reason>` (`:114`, `skip = ($4 == "#" && $5 == "SKIP")`): emit `SKIP <id> <reason>` and print `-- SKIPPED <preset>/<suite>/<id>: <reason>` in the shell loop (`:137`); success line `:174` unchanged. `tools/gate.sh:37` (`keep_fails` greps `^-- FAIL `) leaves it out of failure summaries; rerun the ci-summary selftests. Control copies `controls/tests-inventory-unexpected-skip/` (overlay `build/gate/dev/toy.txt` already has `# SKIP C1`).
**`tools/owner-run.sh`:** analog `tools/listen-demo.sh`: SPDX, banner (purpose, usage, env, exit codes), `LC_ALL=C`, root from `$0`, id check `case $id in ''|*[!a-z0-9-]*)`, tool check `for t in awk sed cmake`, rights refusal before any build:
```sh
if awk '$1 == "content" && $3 == "user" { f = 1 } END { exit !f }' "$scn"; then
  printf 'listen-demo: refusing %s: ...\n' "$id" >&2; exit 2
```
Add refusals for `CI`/`GITHUB_ACTIONS`, an in-tree path outside `refs/`, a wrong size, a missing lock, a lock line not exactly 64 lower-case hex; exit 77 with a `SKIPPED owner-run:` line when no images; digest by `cmake -E sha256sum`. Tests copy `runner-process.listen-demo.refuses-user` (`runner-process-tap.sh:826-836`: `run_one`, `[ "$rc" -eq 2 ]`, `grep -q` on stderr, `result runner-process.owner-run.<slug>`).
**`runner/main.c` refusals:** analog `main.c:697-707`:
```c
if (frame_ppm_path != NULL || video_raw_path != NULL) {
  for (uint32_t role = 0u; role < IVS_SCN_ROLES; role++) {
    if (scn->content[role].kind == IVS_SCN_CONTENT_USER) { free(scn);
      return main_fail(frame_ppm_path != NULL ? "--frame-ppm refused: content user" : "--video-raw refused: content user");
```
Add a sibling on `audio_raw_path`, `psg_log_path`, `audio_wav_path` (declared `:491-493`) before storage is allocated (`:726`). Tests copy `runner-process-tap.sh:744-761` (`printf 'ivs-run: --audio-raw refused: content user\n' > want.err`, exit 14, no stdout report, `cmp` stderr, no file); document in `docs/runner.md:32-33` and `:262`.

### Close-out

- **Claims:** `data/claims.json` tail (CL-0021, CL-0022) is the shape: `id statement profile tier artifact depends_on standing note`; `standing: provisional` (weakest-field rule), `note` with oracle, shared-misreading risk, "Excluded on purpose", "No measured tier is used (D10, D11)". Add ids to `data/expected/claims.ids`; re-check the next free id (F2: CL-0023).
- **Tripwire:** `tools/check/steps/text.sh:34` regex `supports|compatible|passes|accurate|verified|runs` gains `plays|playable`; scope README and `docs/**/*.md` (`text.sh:20-22`); add a control.
- **important-gates:** accepted requirements are hard-coded at `important-gates.sh:71` (`nreq = split("API-02 ... VID-06", reqs, " ")`) and described in the header (lines 8-18); add `SCN-01 SCN-02` there; registry lines `<gate> <fault|control|mutant> <id> <REQ>` in `data/expected/important-gates.ids`.

### W0 lever (`tools/check/vectors.cmake`, `tools/review/vector-blind.cmake`)

No code analog. The per-vector re-parse: `string(JSON vec GET "${doc}" vectors ${i})` (`vectors.cmake:463`), then `string(JSON init GET "${vec}" init)` and `... expect ...` (`:485-490`), plus repeated `json_type`/`vget` (`:135-165`). Keep every token and the `${CAP}` behaviour identical; proof is the existing controls (`data/expected/controls/vectors.ids`) plus a before/after `cpu_s` entry in `.planning/CI-EFFICIENCY.md` and the `r` recomputation from `.planning/ci-runs.tsv` (columns `date run_id sha event conclusion queue_s setup_s build_s test_s gate_s total_s ncpu cpu_s`, written by `tools/ci-record.sh`, orchestrator only).

## Shared Patterns

### SPDX, ASCII, LF
**Source:** `tools/check/integrity.cmake:58-76`. Every `.c`, `.sh`, `.cmake`, `CMakeLists.txt` under `tools/ tests/ runner/ core/` needs `SPDX-License-Identifier: MIT` (C: line 1 comment; shell: line 2). Files under `data fixtures tools tests cmake runner core .github` must be ASCII, no CR (`.asm`, twins, pairs, `.expect`, `.ids` included).

### Gate step shape
**Source:** `tools/check/steps/golden.sh:1-60,107-110`, `c-build.sh:15-25`. Inputs `ROOT`, `CHECK_DIR`, `FILELIST` (`: "${ROOT:?ROOT is not set}"`), `LC_ALL=C`, `mktemp -d` + `trap`, findings through `bad <token> <detail>`, printed `-- FAIL <token>: <detail>`, final `-- <step> pass <stats>`; every new token is documented in the step header. cmake checkers use `gate_fail`, `gate_stat`, `gate_finish` (`tools/check/lib.cmake`).

### Negative controls
**Source:** `tools/check/run-controls.sh:1-12,73-110`. `tools/check/controls/<name>/{CHECK,EXPECT,overlay/|MAKE}` on a copy of `controls/_good/tree`; EXPECT is `FAIL <token>: <prefix>`; MAKE verbs `write delete empty rename crlf ...`; register in `data/expected/controls/<group>.ids`. A pure control is cheap (about 0.07 hosted cpu-s), a symbol-layer one is not (F18); sum the count honestly (X10).

### Hand-derived expectation headers
**Source:** `data/golden/diag-stic-fgbg-pass.expect:1-40`, `docs/video/diag-stic-1-derivation.md`. Text says: derived BEFORE the program, scenario or any run; not generated or read back from the core; "Excluded on purpose" list (machine time, CPU registers, R7, `[0340]` conflict C68); same author wrote core and program, so shared misreading is possible and claims stay provisional.

### Manifest refresh
**Source:** `fixtures/FX-diag-stic-1/manifest.json` `note`, `manifests.cmake:285-330`. Edit of a listed output refreshes `sha256` (`cmake -E sha256sum`) and `size_bytes` in the same commit.

### Doc tripwires (every new `docs/**/*.md`, README)
**Source:** `text.sh:34` (claim words need `[claim:CL-NNNN]` or `[nc]`; `plays|playable` joins this phase) and `glossary.sh:53` (bare `clock(s)` fails unless `input clock`, `STIC clock`, `PSG chip clock` or `[nc]`). `.planning/` is not scanned by these, but `tools/hygiene.sh` rejects home and scratch paths and `@` before a dotted word everywhere.

### Test registration
**Source:** `data/expected/tests/*.ids`, `tests-inventory.sh:28-45`. Every new id is registered; a skip needs the `skip-ok` second token; `<suite>.judge.<slug>` and `<suite>.mutant.<slug>` double as `important-gates` mutants.

### Tracked-source rule
Plans reference `fixtures/ data/ tools/ tests/ runner/ docs/` sources, never `build/<preset>/images/*`. `.gitignore` covers `/build/`, `*.ivsc`, `*.bin`, `*.wav`, `*.ppm`, `*.vraw`, `/refs/` but not `*.exec`, `*.grom`, `*.img` (F22): the `files` step must refuse a tracked image.

## No Analog Found

| File | Role | Data flow | Reason |
|---|---|---|---|
| ISR inside `fixtures/FX-demo-lamp-tone-1/lamp-tone.asm` | program | event-driven | No cartridge listing has an ISR (stub trap block and one `PULR` only). Use the `07-DISCUSS-runtime.md` bands (D-10..D-13); derive first. |
| `DATA` row in `tools/check/listing.cmake` (X9, optional) | gate checker | transform | `diag-stic-1` treats data as program (the cost the D4 note records); no data-row grammar exists; no generated listing here contains `DATA`. |
| `scenario-pairs.ids` two-report judge | judge | transform | Existing judges compare one report with an inventory or itself; copy only the fake-runner self-control scaffolding. |
| `tools/check/vectors.cmake` batching (W0) | gate checker | batch | In-file refactor; measure with `ci-runs.tsv` and `CI-EFFICIENCY.md`. |

## Metadata

**Analog search scope:** `tools/` (pack, pcm, cart, check, review, listen-demo, ci-record), `fixtures/`, `data/{scenarios,golden,expected,video,input,cp1610}`, `runner/`, `tests/` (CMake, pack-tap, runner-process-tap, run/, runner/, installed-consumer/), `docs/`, root `CMakeLists.txt`, `.planning/{CI-EFFICIENCY.md,ci-runs.tsv}`.
**Files scanned:** about 70 read in whole or by range; every cited path verified tracked.
**Pattern extraction date:** 2026-11-13
**Plan-time re-checks:** cited line numbers, next free claim id, `r` median, `tests-skipped` baseline (2 now, 4 after W6), assumption A6 (`JD` decodes as row `J` in the listing step).
