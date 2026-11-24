# Phase 2: CP-1610 CPU Core - Pattern Map

**Mapped:** 2026-11-09
**Files analyzed:** 41 (file classes; a glob such as `data/cp1610/vectors/*.json` counts once)
**Analogs found:** 29 / 41 (17 exact, 8 role-match, 4 partial). The 12 files with no analog are all C and CMake build code (the repo has no C yet).

All analog paths below were checked with `git ls-files --error-unmatch` (tracked-source gate, #3645). No gitignored mirror path and nothing under `refs/` is named. Line numbers refer to the tree at commit c59552c.

## Reading guide

The analogs are Phase 1's gate and data records. The part of this phase most likely to go red between plans is the contract with that gate, so read **Shared Patterns S1-S8** first; the per-file assignments refer to them by number.
Baseline measured while mapping: `sh tools/gate.sh` on the current tree is `gate: PASS steps=17/17 controls=112/112`, 19.05 s wall (6.6 s user, 11.4 s sys), local, moderate load.

## File Classification

| New/Modified File | Role | Data Flow | Closest Analog | Match Quality |
|-------------------|------|-----------|----------------|---------------|
| `tools/check/steps/tests-inventory.sh` | gate step (pure checker) | batch (read TAP + ids, emit tokens) | `tools/gate.sh` 35-51 + `tools/check/steps/ci-parity.sh` | role-match |
| `tools/check/steps/c-build.sh` | gate step (external tool) | request-response (subprocess) | `tools/check/steps/hygiene-history.sh`, `tools/check/steps/cp1610.sh` | partial |
| `tools/check/steps/c-tests.sh` | gate step (external tool) | request-response (subprocess) | `tools/check/steps/hygiene-history.sh`, `tools/check/steps/cp1610.sh` | partial |
| `tools/check/steps/c-faults.sh` | gate step (external tool) | batch | `tools/check/steps/runner-selftest.sh`, `tools/check/steps/cp1610.sh` | partial |
| `tools/check/steps/c-purity.sh` | gate step (scan + selftest) | batch | `tools/check/steps/denylist.sh`, `tools/check/steps/runner-selftest.sh` | role-match |
| `tools/check/steps/cp1610-vectors.sh` | gate step (cmake -P wrapper) | request-response | `tools/check/steps/cp1610-reader.sh`, `tools/check/steps/manifests.sh` | exact |
| `tools/check/core-symbols.sh` | helper script (not a step) | transform (nm text to verdict) | `tools/check/binscan.sh` | role-match |
| `tools/check/core-symbols-allow.txt`, `core-symbols-deny.txt` | config list | read-only list | `tools/check/denylist.txt`, `denylist-cite.txt` | exact |
| `tools/check/vectors.cmake` | checker (cmake -P, multi-file) | batch | `tools/check/manifests.cmake` + `tools/check/coverage.cmake` | role-match |
| vector table generator (cmake -P; location discretionary, suggest `cmake/IvsVectorTable.cmake`) | build-time generator | transform (JSON to C initializer) | `tools/check/coverage.cmake` (speed idioms only) | partial |
| `tools/check/schema/vector-review.txt` + `tools/check/kinds/vector-review.cmake` | record kind (schema + hook) | CRUD-validate | `tools/check/schema/second-reader.txt` + `kinds/second-reader.cmake` + `kinds/source.cmake` | exact |
| `tools/gate.sh` (modify) | gate driver | batch | itself (lines 17, 55-65) + `tools/check/run-controls.sh` 129 | exact |
| `tools/check/integrity.cmake` (modify) | checker | batch | itself (lines 60-70) | exact |
| `tools/check/steps/{denylist,glossary,sources}.sh` (modify) | gate step hardening | batch | `tools/hygiene.sh` 70-79, `tools/check/steps/text.sh` 26-29 | exact |
| `tools/check/controls/<new>/{CHECK,EXPECT,MAKE,FORBID,overlay/}` | negative control | request-response | `controls/cp1610-reader-missing-id`, `claims-inventory-empty`, `profile-missing-field`, `denylist-hit` | exact |
| `tools/check/controls/_good/tree/**` additions | control fixture | n/a | `_good/tree/data/cp1610/*.json`, `_good/tree/data/expected/cp1610-rows.ids` | exact |
| `data/expected/gate-steps/cpu.ids` | inventory | n/a | `data/expected/gate-steps/cp1610.ids` | exact |
| `data/expected/controls/{tests-inventory,core-purity,cp1610-vectors,faults}.ids` | inventory | n/a | `data/expected/controls/cp1610.ids` | exact |
| `data/expected/cp1610-vectors.ids` | inventory | n/a | `data/expected/cp1610-rows.ids` | exact |
| `data/expected/tests/<suite>.ids` | inventory (first token + `skip-ok`) | n/a | `data/expected/controls/*.ids` (first-token reader) | role-match |
| `data/expected/cp1610-skips.ids` | inventory (id + discrepancy) | n/a | `data/expected/cp1610-rows.ids` | role-match |
| `data/expected/faults/cpu.ids` | inventory (3 tokens) | n/a | `data/expected/gate-steps/*.ids` (first-token reader) | role-match |
| `data/cp1610/vectors/*.json` (~10) | data record | n/a | `data/cp1610/opcode-families.json` | role-match |
| `data/cp1610/vector-review.json` (name discretionary) | data record | n/a | `data/cp1610/second-reader.json` | exact |
| `data/discrepancies.json`, `data/expected/discrepancies.ids` (modify, C36+) | data record | n/a | entries C29-C35 | exact |
| `data/profiles/intv-2609-ntsc-family-p1.json`, `data/expected/profile-fields.ids` (modify) | data record | n/a | fields `reset.r7`, `reset.flags` | exact |
| `docs/cp1610/reference-notes.md` (modify) | doc | n/a | itself | exact |
| `README.md` (modify, one line) | doc | n/a | itself | exact |
| `.planning/CI-EFFICIENCY.md` (modify) | doc | n/a | itself | exact |
| `CMakeLists.txt` | config | n/a | none | none |
| `CMakePresets.json` | config | n/a | none | none |
| `cmake/IvsWarnings.cmake` | config | n/a | none (style only: `tools/check/*.cmake`) | none |
| `core/src/common/ivs_status.h` | core header | n/a | none | none |
| `core/src/common/ivs_prelude.h` | core header | n/a | none (RESEARCH "Code Examples") | none |
| `core/src/cpu/cp1610.h` | core header | n/a | none | none |
| `core/src/cpu/cp1610.c` | core stepper | request-response (state, lines, replies) | none | none |
| `core/src/cpu/cp1610_decode.c` | core decode | transform | none | none |
| `tests/CMakeLists.txt` | config | n/a | none | none |
| `tests/faults.cmake` | config | n/a | none | none |
| `tests/harness/ivs_test.{h,c}` | test harness | event-driven (TAP print) | none | none |
| `tests/cpu/*.c` | test | request-response | none | none |

## Pattern Assignments

### `tools/check/steps/cp1610-vectors.sh` (gate step, cmake -P wrapper)

**Analog:** `tools/check/steps/cp1610-reader.sh` (single cmake -P, strips `records=<n>`, re-prints under the step name) and `tools/check/steps/cp1610.sh` (several checks, `rc` accumulation).

**Wrapper pattern** (`cp1610-reader.sh` lines 7-16):
```sh
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
rc=0
out=$(cmake -DROOT="$ROOT" -DKIND=second-reader -P "$CHECK_DIR/records.cmake" 2>&1) || rc=1
printf '%s\n' "$out"
[ "$rc" -eq 0 ] || exit 1
stats=$(printf '%s\n' "$out" | sed -n 's/^-- second-reader pass records=[0-9][0-9]* //p')
[ -n "$stats" ] || exit 1
printf -- '-- cp1610-reader pass %s\n' "$stats"
```

**Multi-check accumulation** (`cp1610.sh` lines 11-22): run every check, `printf '%s\n' "$out"` each, set `rc=1` on failure, exit only after all ran, then require every stat to be non-empty before printing the combined pass line.
Use this shape to run both `vectors.cmake` (multi-file vector data) and `-DKIND=vector-review -P records.cmake` (marker) and print `-- cp1610-vectors pass vectors=<n> source=<n> policy=<n> second-read=<n>/<m>`.

**Multi-file invocation** (`steps/manifests.sh` line 8): `cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -P "$CHECK_DIR/manifests.cmake" 2>&1`. Pass `FILELIST` so the vector files are the FILELIST paths under `data/cp1610/vectors/` (works in the control runner, where FILELIST comes from `find`).
Register per S1; stat keys must be unique across steps (S2).

---

### `tools/check/vectors.cmake` (checker, multi-file JSON batch)

**Analogs:** `tools/check/manifests.cmake` (multi-file via FILELIST, `check_shape`), `tools/check/coverage.cmake` (speed idioms, finding cap), `tools/check/lib.cmake` (helpers), `tools/check/kinds/cp1610-row.cmake` (cite and pattern checks). `records.cmake` cannot be reused unchanged: it takes one `file` per kind (`records.cmake` lines 53-54, `d STREQUAL "file"`; A-07).

**Input guard and includes** (`manifests.cmake` lines 14-21):
```cmake
cmake_minimum_required(VERSION 3.25)

foreach(required ROOT FILELIST)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "manifests.cmake is missing ${required}")
  endif()
endforeach()
include("${CMAKE_CURRENT_LIST_DIR}/lib.cmake")
```

**Required keys, types, unknown-key detection** (`manifests.cmake` `check_shape`, lines 28-53): every listed `key:TYPE|TYPE` is required, every other member is an unknown key; token names are `<kind>-missing-key`, `-format`, `-unknown-key`. Copy the helper and re-token it (`vector-missing-key`, `vector-format`, `vector-unknown-key`).

**Finding cap with exact totals** (`coverage.cmake` lines 22-47): `set(CAP 20)`, a `report(token detail)` that calls `gate_incr(findings)`, lists only the first CAP findings and prints one `coverage-more` line. A 300-360 vector set with a systematic error would otherwise print hundreds of lines.

**Speed idiom** (`coverage.cmake` header lines 12-14, `records.cmake` line 275): extract each element into its own string once (`string(JSON rec GET "${DOC}" rows ${i})`) and read fields from `rec`; never index the whole document per field (D-14 measured 0.79 s vs 8.1 s).

**Cite check** (`kinds/cp1610-row.cmake` lines 34-41):
```cmake
  json_type(ct "${rec}" cite)
  if(ct STREQUAL STRING)
    string(JSON cite GET "${rec}" cite)
    if(NOT "${cite}" MATCHES "pdf [0-9]+")
      gate_fail(cp1610-row-format "${id}/cite")
    endif()
  endif()
```
The vector `cite` must be ASCII (A-06): `S3.3.1 Fig 3.3.1, pdf 50; register-register table, pdf 53` is the existing form (`opcode-families.json`).

**First-decle label check:** the pattern-to-mask/value expansion is already written in `coverage.cmake` lines 79-100 (`^[01.]{10}` pattern, `.` = 0 in value and 0 in mask, `1`/`0` = bit set in mask). Reuse it to test that a vector's first decle matches the declared row's `pattern` read from `data/cp1610/opcode-families.json`. Row ids come from `data/expected/cp1610-rows.ids`; ids match `^[A-Z][A-Z0-9@]*$` (`schema/cp1610-row.txt` line 35).

**Coverage-class derivation vocabulary** (executed over the 74 rows; these are the only values in the columns the checker must parse):
- `status_bits`: `none`, `S, Z`, `C`, `S, Z, C, OV`, `N=0: S, Z; N=1: S, Z`, `N=0: S, Z, C; N=1: S, Z, C, OV`.
- `cycle_modifiers`: `[]`, `["+1 if DDD=6 or 7"]`, `["+1 if SSS=6 or 7"]`, `["+3 if MMM=6"]`, `["6 if N=0", "8 if N=1 (page header: add 2 cycles if shift is 2 bits or two bytes)"]`.
- `interruptible`: `no` or `manual-silent`. `sdbd`: `not-applicable`, `unsupported`, `supported: 10 (pdf 57)`, `supported: 14 (pdf 58)`.
- Overlapping rows to cover per D-16: `CLRR` refines `XORR` (`eq` `5:3=2:0`); `TSTR`/`JR` coincide with `MOVR` at `0010 111 111` (`also_encodes`).

**Inventory:** read `data/expected/cp1610-vectors.ids` with `read_ids` (`lib.cmake` 73-116, returns the sorted unique list, emits `-inventory-empty`, `-inventory-duplicate`); it takes the WHOLE line as the id, so this file must be one id per line (S7). Emit `vector-id-missing`/`vector-id-extra` as `records.cmake` lines 313-327 do.

**Digest of source files:** `kinds/source.cmake` lines 53-62 is the stale-digest idiom: `file(SHA256 "${ROOT}/${path}" have_sha)`, compare, `gate_fail(...-digest-mismatch ...)`.

**Finish:** `gate_stat(vectors ...)`, `gate_finish(vectors)` prints `-- vectors pass <stats>` (`lib.cmake` 35-44).

---

### Vector table generator (cmake -P, build time)

**Analog (partial):** `tools/check/coverage.cmake` for run-once, parallel-list handling and `records.cmake` lines 11-19 for the `-D` input guard. There is no code generator in the repo.
Constraints from D-14: input JSON files, output an uncommitted `.inc` in the build directory (`/build/` is already gitignored, anchored), `add_custom_command` with DEPENDS on the JSON, embed each source file's sha256 via `file(SHA256 ...)`, deterministic order (sort by id), no committed generated header. If it lives under `tools/` it needs the SPDX line (S4); under `cmake/` it does not yet (see S4 note on D-23).
Beware `string(JSON ...)` results that contain `;`, `[` or `]` (derivation text, `[addr, word]` pairs): do not push them through list operations unprotected (`gate_lines`/`gate_unprotect`, `lib.cmake` 46-61, show the protect/restore pair).

---

### `tools/check/schema/vector-review.txt` + `tools/check/kinds/vector-review.cmake` (new record kind, A-07)

**Analog:** `schema/second-reader.txt` + `kinds/second-reader.cmake` + `steps/cp1610-reader.sh` (exact shape). Do NOT edit `second-reader.json` or its schema (A-07, RESEARCH Pitfall 5): `topkeys` and id parity are strict and `cp1610-reader` plus 3 controls would break.

**Schema DSL** (`schema/second-reader.txt` lines 5-24, copy and re-key):
```
file data/cp1610/second-reader.json
topkeys schema_version reader method c17_reading rows
top schema_version NUMBER
...
array rows
idkey id
inventory data/expected/cp1610-rows.ids
keys id verdict fields_checked pdf note
req id STRING
req verdict STRING
nonblank fields_checked
match verdict ^(agree|resolved)$
when verdict=resolved nonblank note
hook second-reader
```
For the marker add keys such as `file`, `file_sha256`, `disagreements`, `resolutions` (names discretionary) and `hook vector-review`; the inventory must be a new ids file of marker ids (the existing `cp1610-rows.ids` is the wrong set).

**Hook** (`kinds/second-reader.cmake` lines 9-41): `hook_record(id rec)` runs per record, `hook_finish()` once; counters via `gate_incr` / `gate_count`, stats via `gate_stat`. Add the stale check inside `hook_record` with the `kinds/source.cmake` 53-62 digest idiom, token `vector-review-stale`.
**Regex limits:** a schema regex cannot hold a space (schema comment lines 3-4), and CMake regex has no `{n}` interval. Build a 64-hex regex with `string(REPEAT "[0-9a-f]" 64 _hex64)` as `manifest-lib.cmake` lines 8-9 do, in the hook, not the DSL.
**Stat print rule:** the step wrapper drops `records=<n>` and keeps unique keys (`cp1610-reader.sh` line 14); this phase also wants `second-read=<n>/<m>` and a marker that may be absent until plan 8 (RESEARCH Section 3): fail only on stale, print the count, require n==m at phase verification.

---

### `tools/check/steps/tests-inventory.sh` (pure checker over TAP, ids, NONCE)

**Analogs:** `tools/gate.sh` lines 35-51 (set equality with `sort -u` + `comm`), `tools/check/run-controls.sh` lines 100-118 (same, with first-token reader), `tools/check/steps/ci-parity.sh` lines 44-121 (awk prints `<token> <detail>` lines, the shell loop re-emits them as `-- FAIL` lines and parses a final summary line).

**Set-equality idiom** (`gate.sh` lines 42-51):
```sh
: > "$GATE_TMP/steps.want"
for f in "$ROOT"/data/expected/gate-steps/*.ids; do
  [ -f "$f" ] || continue
  awk '/^#/ { next } NF { print $1 }' "$f" >> "$GATE_TMP/steps.want"
done
sort -u "$GATE_TMP/steps.want" -o "$GATE_TMP/steps.want"
if [ ! -s "$GATE_TMP/steps.want" ]; then bad gate-steps-empty "data/expected/gate-steps"; fi
want=$(grep -c . "$GATE_TMP/steps.want")
for name in $(comm -23 "$GATE_TMP/steps.want" "$GATE_TMP/steps.have"); do bad gate-step-missing "$name"; done
for name in $(comm -13 "$GATE_TMP/steps.want" "$GATE_TMP/steps.have"); do bad gate-step-extra "$name"; done
```
Map: `tests-missing` = in ids, not executed; `tests-unlisted` = executed, not in ids; `tests-inventory-empty` = empty ids (analog `claims-inventory-empty`, token `claim-inventory-empty`); `tests-suite-missing` = a suite with no TAP file.

**Awk-emits-tokens pattern** (`ci-parity.sh` lines 107-121): awk writes `TOKEN detail` lines plus one `USES n GATE n ...` summary line to a file; the shell `while read` loop turns each into `bad "${l%% *}" "${l#* }"` and reads the summary fields back. Use the same split for TAP parsing: awk emits `tests-duplicate <id>`, `tests-failed <id>`, `tests-plan-mismatch <suite>`, `tests-truncated <suite>`, `tests-stale <suite>` and a final `SUITE <suite> EXECUTED n SKIPPED k FAILED f` line. Stay POSIX awk (mawk on the hosted image is [ASSUMED]; BWK locally): no `gensub`, no interval expressions, no `systime`.

**Environment contract (critical):** the control runner calls steps with only `ROOT`, `FILELIST`, `CHECK_DIR` (`run-controls.sh` line 97: `ROOT="$tmp/tree" FILELIST="$tmp/filelist" CHECK_DIR="$CHECK_DIR" sh ...`); `GATE_TMP` is NOT set. Read TAP files, `NONCE` and a captured `ctest -N` listing from `$ROOT/build/gate/` only, and never invoke ctest or cmake here, or the step cannot be driven by controls (RESEARCH Pitfall 3). Compare the NONCE file with the `# suite=<s> nonce=<n>` header of every TAP file.

**Pass line:** `-- tests-inventory pass tests-expected=<n> tests-executed=<n> tests-skipped=<n> tests-missing=0` (keys from D-22; unique across the gate).

---

### `tools/check/steps/c-build.sh`, `c-tests.sh`, `c-faults.sh` (external-tool steps)

**Analogs (partial):** `tools/check/steps/hygiene-history.sh` (the only step that drives an external tool in a clean environment from a temp dir) and `steps/integrity.sh` / `steps/files.sh` (hide the pass line next to a failure).

**Preamble to copy** (`hygiene-history.sh` lines 17-21, same as `denylist.sh` lines 12-20):
```sh
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/hygiene-history.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; }
```

**Quiet pass, full log on failure** (`steps/files.sh` line 21): `if [ "$rc" -ne 0 ]; then grep -v -e '^-- files pass ' "$tmp/cmake.out"; else cat "$tmp/cmake.out"; fi`. D-22 wants build logs only on failure: capture `cmake --build`/`ctest` output into `$tmp/*.log`, `cat` it only when the command fails, otherwise print one `-- c-build pass cc=<name> ...` line.

**Unlisted tracked source** (`c-build-unlisted-source`): select candidates from FILELIST the way `steps/text.sh` line 21 does (`grep -E '^(README\.md|docs/.*\.md)$' "$FILELIST" | sort -u`), here `grep -E '^core/.*[.]c$' "$FILELIST"`, then `comm` against the source list the build reports.

**Env for the harness:** gate.sh must export `IVS_TEST_OUT_DIR` and the nonce (S1); steps read `$ROOT/build/gate/NONCE`, they do not recompute it. Each step runs `cmake --build --preset <p>` before testing so a stale binary is never tested (D-22). Steps run under `sh`; `cd "$ROOT"` first (presets and `build/<preset>` are relative to the source dir).
**Faults:** `c-faults` cannot be driven by the control runner if it builds and runs executables itself. Recommended split (discretionary): `c-faults` runs the fault executables and writes per-fault result files under `$ROOT/build/gate/faults/`; the verdict logic (`fault-not-detected`, `fault-wrong-reason`, `fault-collateral`, `fault-missing`) lives in a pure checker over those files plus `data/expected/faults/cpu.ids`, which controls can drive with text overlays (same pattern as `tests-inventory`). Self-test pattern for in-step proofs: `steps/runner-selftest.sh` lines 20-70 (throwaway synthetic inputs, token per missing detection, `cases=<n>` stat).

---

### `tools/check/steps/c-purity.sh` + `tools/check/core-symbols.sh` + allow/deny lists

**Analogs:** `steps/denylist.sh` (scope loop, lists read from `CHECK_DIR`, findings name the thing, never matched text), `tools/check/binscan.sh` (helper script called by steps, usage exit 2, temp dir + traps, portable tools only), `steps/runner-selftest.sh` (in-step self-test).

**Lists live in CHECK_DIR so controls cannot replace them** (`denylist.sh` lines 22-31, header lines 7-8):
```sh
for name in denylist denylist-cite; do
  if [ ! -f "$CHECK_DIR/$name.txt" ]; then
    bad denylist-list-missing "$name.txt"
    : > "$tmp/$name"
    continue
  fi
  grep -v -e '^#' -e '^[[:space:]]*$' "$CHECK_DIR/$name.txt" > "$tmp/$name"
  [ -s "$tmp/$name" ] || bad denylist-list-empty "$name.txt"
done
```
Copy for `core-symbols-allow.txt` (`#` comments, blank lines ignored, tokens `core-symbols-list-missing`, `core-symbols-list-empty`). The new `.txt` files need no SPDX line (existing `denylist.txt` has none) but must be ASCII, LF and non-empty.

**Helper-script shape** (`binscan.sh` lines 17-23, 32-34): `LC_ALL=C`, `: "${VAR:?...}"` guards, `[ "$#" -ge 1 ] || { printf 'usage: ...\n' >&2; exit 2; }`, `tmp=$(mktemp -d ...)`, two traps. `core-symbols.sh [--allow-prefix P]... FILE...` follows it; `NM` comes from the environment (default `nm`).
**Token and pass shapes** (D-21, same print form as `bad()`): `-- FAIL core-symbols-forbidden: <sym>`, `core-symbols-lto`, `core-symbols-bitcode`, `core-symbols-no-definitions`; pass `-- core-symbols pass external=<N> defined=<M>`. The step pass line adds unique keys: `-- c-purity pass core-imports=<n> purity-selftest=<n>/<n>`.

**Positive-control coupling:** the runner runs `c-purity` on `_good/tree` with ROOT = tree, so `_good/tree/core/src/` needs a clean 1-2 file stand-in core (with SPDX if D-23 applies, S4) and the step must compile from `$ROOT/core/src`, writing only to a temp dir. A missing compiler is a failure, not a skip (D-21).
**Controls that cannot be source overlays:** `core-purity-lto` and `core-purity-bitcode` need objects the step itself does not produce (the step forces `-fno-lto`). Cover them inside the step as the `purity-selftest` stage (build synthetic LTO/bitcode objects, run `core-symbols.sh`, require the token), following `runner-selftest.sh`; keep the overlay controls for the source-level classes (clock, env, fs, thread, io, rand, alloc, header, asm, float, float-literal, hollow).

**Traps found by running the D-21 source-layer greps (verified):**
- The float-literal regex `[0-9]+\.[0-9]*|\.[0-9]+|[0-9]+[eE][+-]?[0-9]+` matches hex constants such as `0x3E4` and `0x2E8`, and any comment cite such as `Fig. 3.2.6, pdf 49` or `S3.3.4`. Core code will contain both. Strip comments and hex literals before grepping (or scan the preprocessed token stream), and give `core-purity-float-literal` a control that also proves a hex constant is NOT flagged.
- Use word-boundary matching for `asm` (`grep -w`) so identifiers and comment words do not trip it.

---

### Control directories under `tools/check/controls/` (negative controls)

**Analogs:** `cp1610-reader-missing-id` (CHECK + EXPECT + `overlay/data/cp1610/second-reader.json`), `claims-inventory-empty` (CHECK + EXPECT + MAKE `empty data/expected/claims.ids`), `profile-missing-field` (EXPECT + FORBID), `integrity-missing-spdx` (MAKE `write tools/x.sh x`), `denylist-hit` (overlay `src/x.c`). Layout, matching rules and MAKE verbs are in S3.

Planned shapes (names discretionary; tokens from D-17/D-21/D-22):
- `tests-inventory-missing`: CHECK `tests-inventory`; EXPECT `FAIL tests-missing:`; overlay `data/expected/tests/<suite>.ids` with one extra id.
- `tests-inventory-unlisted`, `-duplicate`, `-unlisted-skip`, `-stale-nonce`, `-truncated`, `-plan-mismatch`, `-failed`, `-suite-missing`, `-empty`, `-ctest-drift`: same CHECK; overlay the TAP file / `NONCE` / ids under `build/gate/` and `data/expected/tests/`, or MAKE `delete build/gate/<suite>.txt`, `empty data/expected/tests/<suite>.ids`.
- `cp1610-vectors-*` (stale digest, stale review, bad cite, row mismatch, missing id, extra id): CHECK `cp1610-vectors`; overlay a vector JSON in `data/cp1610/vectors/`; the generator-path `expect` mutation control is a separate beast (needs the build) and belongs with the fault checker, not the runner.
- `core-purity-<class>`: CHECK `c-purity`; EXPECT `FAIL core-symbols-forbidden: <sym>`; overlay replaces the stand-in `core/src/<file>.c` with a violating variant. Remember the printed symbol is normalized (no Darwin leading underscore, no `@ver`).
Each control must be listed in a `data/expected/controls/*.ids` file the same commit (else `control-not-in-inventory`; a listed name without a directory gives `control-inventory-orphan`).

---

### `tools/check/controls/_good/tree/**` (fixture additions)

**Analog:** the existing toy tree: `_good/tree/data/cp1610/opcode-families.json` (3 rows `ALL`/`ZERO`/`ONES`, one-line notes), `_good/tree/data/cp1610/second-reader.json`, `_good/tree/data/expected/cp1610-rows.ids` (three bare lines, no comment). The tree is copied for every control (`run-controls.sh` line 87: `cp -R "$CONTROLS_DIR/_good/tree/." "$1/"`), 24 files today, which is the dominant cost of a control (RESEARCH measured about 0.15 s each, sys-bound), so add the minimum.

Required additions for the new steps to pass their positive control (`run-controls.sh` lines 120-135 run every distinct CHECK on the pristine tree and demand `-- <step> pass`):
- `tests-inventory`: `data/expected/tests/<suite>.ids`, `build/gate/<suite>.txt` (TAP with header, plan, trailer), `build/gate/NONCE`, the captured ctest listing.
- `c-purity`: `core/src/` stand-in (1-2 files, SPDX + prelude + one exported function).
- `cp1610-vectors`: `data/cp1610/vectors/<one>.json` (1 vector), `data/expected/cp1610-vectors.ids`, the marker file and its ids, consistent with the toy `cp1610-rows.ids` ids (`ALL ONES ZERO`).
`.gitignore` anchors `/build/` at the root, so `tools/check/controls/_good/tree/build/gate/` is tracked (RESEARCH, verified).
**Side effect:** with ROOT = tree, the pristine-tree runs of `integrity`, `hygiene`, `denylist`, `files`, `text`, `glossary` also scan these new files. Stand-in `core/*.c` therefore needs SPDX once D-23 lands, TAP text must be ASCII/LF/non-empty, and nothing in them may match the denylist or hygiene shapes (S5).

---

### `data/expected/gate-steps/cpu.ids`, `data/expected/controls/*.ids`, `cp1610-vectors.ids`, `tests/*.ids`, `cp1610-skips.ids`, `faults/cpu.ids`

**Analogs and conventions** (full text of the existing files):
`gate-steps/cp1610.ids`:
```
# Gate step inventory for the CP-1610 family table (plan 01-09).
cp1610
```
`controls/cp1610-reader.ids`:
```
# Negative controls for the cp1610-reader step (plan 01-10).
cp1610-reader-missing-id
cp1610-reader-resolved-without-note
cp1610-reader-missing-key
```
`cp1610-rows.ids` first line: `# CP-1610 opcode family row ids (FND-06); must equal the ids in data/cp1610/opcode-families.json.`
Rules in S7 (two different readers; `#` must be in column 1 for the awk reader; multi-token lines only where the reader is awk/sh).
`cp1610-skips.ids` may legitimately hold no ids (A-05: branch cycles are policy vectors, not skips). `lib.cmake` 92-95 supports `#allow-empty: <reason>` only for the cmake `read_ids` reader; the awk/sh readers treat it as a plain comment. The file must still be non-empty (`integrity-empty-file`), so a comment-only file passes integrity.

---

### `data/cp1610/vectors/*.json` (vector data, ~10 files)

**Analog:** `data/cp1610/opcode-families.json` (top keys `schema_version source note rows`, 2-space indent, double quotes, ASCII, `cite` as `S3.3.1 Fig 3.3.1, pdf 50; register-register table, pdf 53`).
Suggested shape (keys from D-14; placeholders only, no value in this map is a derived expectation):
```json
{
  "schema_version": 1,
  "source": "SRC-gi-cp1600-um-1975",
  "note": "<group note>",
  "vectors": [
    {
      "id": "cpu.<ROW>.<slug>",
      "row": "<id from data/expected/cp1610-rows.ids>",
      "kind": "source",
      "tier": "source-expectation",
      "cite": "S3.3.1 Fig 3.3.1, pdf 50; register-register table, pdf 53",
      "sources": ["SRC-gi-cp1600-um-1975"],
      "asm": "<advisory text>",
      "program": "<decles>",
      "derivation": "<paraphrase plus arithmetic, non-empty>",
      "init": {},
      "expect": {},
      "covers": []
    }
  ]
}
```
Constraints this repo enforces on these files:
- `data/` is ASCII-only (`integrity-non-ascii`): no section sign in `cite` (A-06), no curly quotes in `derivation`.
- The `sources` step scans every `data/**/*.json` for `SRC-[a-z0-9-]+` and requires each token to be in `data/expected/sources.ids` (`steps/sources.sh` lines 29-39): only `SRC-gi-cp1600-um-1975` may appear.
- The `denylist` step scans `data/` (only `.planning/`, `.claude/`, `docs/provenance*`, `tools/check/`, `data/sources.json` are excluded): no emulator or author names, even inside `derivation` or `note` (S5).
- `kind=policy` vectors carry no `tier` (D-14) and are counted separately.
- **Hygiene email trap (verified by running the regex):** an `@` immediately followed by an alphanumeric and a later dotted word is read as an email address. `cpu.MVO@.push-r6` and `MVI@ R4,R0` are clean; an id in which the at-sign is directly followed by a register name and a later dot-separated slug (letters-only last part) is flagged `hygiene-email` (this map does not spell one out, because it would fail the same scan). Row ids `MVO@ MVI@ ADD@ SUB@ CMP@ AND@ XOR@` must be followed by `.` or a space in every id, slug, `asm` and `.ids` line.
- File names use `[A-Za-z0-9._/-]` only: no `@`, no `+`.

---

### `data/discrepancies.json` + `data/expected/discrepancies.ids` (C36+)

**Analog:** C29 and C35 (`data/discrepancies.json`). Real shape (C29):
```json
{
  "id": "C29",
  "topic": "Branch cycle count printed 7/9 with no taken or not-taken label",
  "sources": "The branch table prints 7/9 ... (SRC-gi-cp1600-um-1975). ...",
  "status": "open",
  "default_policy": "unknown-default-chosen: Phase 2 reads the first printed figure (7) as not taken ...",
  "resolver": "test: Phase 2 branch cycle vectors; a bus trace of taken and not-taken branches (D10) would settle it",
  "evidence": "SRC-gi-cp1600-um-1975 pdf 54 (7/9 rows, NOPP prints 7); pdf 32 (displacement fetch only); S3.2.2, pdf 47. ...",
  "note": "Appended from the Discrepancy candidates section of docs/cp1610/reference-notes.md (D-21). ..."
}
```
Rules enforced (`schema/discrepancy.txt`, `kinds/discrepancy.cmake`): keys are exactly `id topic sources status default_policy resolver evidence note`; `status` in `open|partial|resolved-primary|withdrawn`; `open`/`partial` need `default_policy` starting `unknown-default-chosen: `; `resolved-primary` needs `pdf N` in `evidence`; `resolver` matches `^(test|measurement|pending):.+`; ids ascend (`discrepancy-order`); every id must also be a line in `data/expected/discrepancies.ids` (currently `C1`..`C35`, no comment lines) in the same commit.
Entries owed (D-13, A-01, A-03, A-06, A-10, RESEARCH G1-G8): intra-instruction access schedule not tabulated; SDBD modes 1-3 one word vs same byte twice (pdf 47 vs 57); sampling-point and strobe-stamp inferences; HLT/C35 refinement; INTFF not cleared on accept; vector-value provenance (C32); shared-misreading risk of hand vectors; PCIT as input not modelled; HALTED handling of INTR/BUSRQ; G1 CMP operand direction; G2 S after right shifts and SWAP; G3 C/OV link bits for left vs right shifts; G4 NEGR/ADCR/RSWD C/OV by analogy; G5 backward-branch complement width; G6 direct-address and displacement word widths; G8 JSR return address legible on pdf 56.
Ordering constraint: a profile field whose `conflict` names a new C-number fails `profile-unknown-ref` unless the discrepancy exists (`schema/profile.txt` lines 22-23 `enumfrom conflict data/discrepancies.json discrepancies id`), so land the discrepancy entries before or with the profile fields.

---

### `data/profiles/intv-2609-ntsc-family-p1.json` + `data/expected/profile-fields.ids` (modify)

**Analog:** field `reset.flags` (unknown-default-chosen) and `reset.r7` (tier4-community with conflict C32):
```json
"reset.flags": {
  "value": "zero",
  "tag": "unknown-default-chosen",
  "cite": "SRC-gi-cp1600-um-1975 SC.1, pdf 21: flags are undefined at reset; zero is the chosen default",
  "conflict": "C32",
  "resolving_evidence": "measurement: D10 reset read-back on owner hardware",
  "revisit_trigger": "when a source documents the reset state or a hardware read-back exists",
  "decision": null
}
```
Rules: every field needs `value tag cite conflict resolving_evidence revisit_trigger decision` (`decision` may be null); `tag` must be an id in `data/tiers.json` `source_authority_tags` (`tier4-community`, `unknown-default-chosen`, `inference`, ...); `unknown-default-chosen` needs non-blank `resolving_evidence` and `revisit_trigger`; `measured` and `tier5-emulator-hint` fail the hook (`kinds/profile.cmake` 15-21); every field path must also be a line in `data/expected/profile-fields.ids` (comment line 1, then one path per line).
**Naming rule enforced by the `glossary` step** (`steps/glossary.sh` lines 65-87): a last path segment containing `window`, `offset`, `hold`, `duration`, `delay`, `period`, `latency`, `interval` or `timeout` must end in `_ticks` or `_cpu_cycles`. So the bus release latency is `<group>.release_latency_cpu_cycles`, the wait cap `<group>.wait_limit_cpu_cycles` (D-13 fields: `interrupt.iab_value`, `cpu.interrupt_accept_clears_intff`, bus release latency, wait cap, EBCI default).

---

### `docs/cp1610/reference-notes.md` and `README.md` (modify)

**Analog:** the notes file itself. Style to match: one sentence per line, facts end with a label `[OBS]` or `[INF]`, cites in parentheses with backticked page (`` (`pdf 35`) ``, `` (`§C.6, pdf 24`) ``); `§` is allowed in `docs/` (the ASCII rule covers `data|fixtures|tools|.github` only). Sections to update (D-13, A-06, RESEARCH Section 3): Interruptibility (N bit, NOP/SIN), SDBD (C34 corroborated: 4 + 10), Branch timing (C29, NOPP twin), Interrupt sequence and Reset (timing figures pdf 27-37, 40, 43), STPST edge (pdf 14), ADAR (pdf 11), JSR return PC+3 legible on pdf 56.
**Tripwires on every doc edit** (the `text` and `glossary` steps scan `README.md` and `docs/**/*.md`):
- `steps/text.sh` line 34: the words `supports|compatible|passes|accurate|verified|runs` need a `[claim:CL-NNNN]` tag or `[nc]` on the same line.
- `steps/glossary.sh` lines 49-50: the bare word `clock`/`clocks` fails unless the line has `input clock`, `STIC clock`, `PSG chip clock` or `[nc]`.
- `reference-notes.md` must keep exactly one line starting `C17 status: ` (`c17-consistency.cmake` lines 56-63).
`README.md` line "It needs a POSIX shell, git and CMake." becomes the new tool list (ninja, a C compiler); that line has no claim words.

---

### `tools/gate.sh` (modify)

**Precheck pattern** (line 17), copy twice for ninja and cc:
```sh
command -v cmake >/dev/null 2>&1 || { bad gate-no-cmake cmake; printf 'gate: FAIL\n'; exit 1; }
```
`gate-no-ninja` and `gate-no-cc` live in `gate.sh`, which the control runner cannot drive (like `gate-no-cmake`); cover them with a throwaway local PATH probe, not a control (RESEARCH Section 3).
**NONCE and results dir** (new): after the `cd "$ROOT"` at line 20 and the FILELIST block (line 33), `rm -rf "$ROOT/build/gate"`, `mkdir -p "$ROOT/build/gate"`, write `basename "$GATE_TMP"` into `build/gate/NONCE`, and export `IVS_TEST_OUT_DIR` (D-17). `build/` is gitignored, so these files never enter FILELIST.
**IN-04 (a passing step must print its pass line):** today lines 55-65 trust the exit code only:
```sh
for name in $(cat "$GATE_TMP/steps.have"); do
  if sh "$CHECK_DIR/steps/$name.sh" > "$GATE_TMP/step.out" 2>&1; then
    cat "$GATE_TMP/step.out"
    passed=$((passed + 1))
    toks=$(sed -n "s/^-- $name pass //p" "$GATE_TMP/step.out" | tr '\n' ' ')
```
Add the check the runner already uses for positive controls (`run-controls.sh` line 129: `grep -q -F -e "-- $step pass" "$tmp/out"`), with its own token (suggest `gate-step-no-pass-line`) so a silent exit-0 is distinguishable from a failure.
Keep `sh tools/gate.sh` argument-free and the summary shape `gate: PASS steps=<n>/<m> controls=<a>/<b> <stats>` (line 84).

---

### `tools/check/integrity.cmake` (modify, D-23)

**Current rules** (lines 60-70):
```cmake
  if(p MATCHES "^(data|fixtures|tools|[.]github)/")
    if(content MATCHES "[^\t\n -~]")
      gate_fail(integrity-non-ascii "${p}")
    endif()
  endif()
  if(p MATCHES "^tools/.*[.](sh|cmake)$" OR p MATCHES "^[.]github/workflows/[^/]*[.]yml$")
    string(FIND "${content}" "SPDX-License-Identifier: MIT" pos)
    if(pos EQUAL -1)
      gate_fail(integrity-missing-spdx "${p}")
    endif()
  endif()
```
Extend with anchored alternatives for `.c` and `.h` under `core/`, `include/`, `tests/`, for example `^(core|include|tests)/.*[.](c|h)$`. **Anchor at the repo root.** An unanchored `.c` rule would hit `tools/check/controls/denylist-hit/overlay/src/x.c` and similar overlay files, which carry no SPDX line. Add a control next to `integrity-missing-spdx` (`MAKE: write core/x.c x`, EXPECT `FAIL integrity-missing-spdx: core/x.c`) and one for ASCII if `core|tests` are added to the ASCII alternation. Note `CMakePresets.json` cannot carry an SPDX comment (JSON), and `CMakeLists.txt` / `cmake/*.cmake` are outside the `^tools/` rule, so decide explicitly which of them the new rule covers.

---

### `tools/check/steps/denylist.sh`, `glossary.sh`, `sources.sh` (modify, D-23)

**Denylist grep status 2 read as no-hit:** lines 51 and 55 use `grep ... "$ROOT/$f"` inside `if`/`||`, so an unreadable file reads as clean. The repo's own fix pattern is `tools/hygiene.sh` lines 67-79:
```sh
file_leaks() {
  _s=0
  grep -a -o $4 -E -e "$2" "$1" > "$tmp/fl.out" 2> "$tmp/fl.err" || _s=$?
  if [ "$_s" -ge 2 ]; then
    found grep-error "${5:-$1}"
    sed -n '1,3p' "$tmp/fl.err" >&2
    return 1
  fi
  grep -a -v -E -e "$3" "$tmp/fl.out" | grep -q .
}
```
**IN-02 (fail on zero documents):** `steps/text.sh` lines 26-29 and `denylist.sh` lines 41-45 already do it:
```sh
docs=$(grep -c . "$tmp/scan")
if [ "$docs" -eq 0 ]; then
  bad text-no-files "README.md docs/"
  exit 1
fi
```
`glossary.sh` (lines 42-46, counts `docfiles` but never fails) and `sources.sh` (lines 28-32) lack it; add `glossary-no-files` and `sources-no-files` with a control each (copy the CHECK/EXPECT/MAKE trio of an existing `*-inventory-empty` or `denylist-*` control).

---

### `.planning/CI-EFFICIENCY.md` (modify, plan 8)

Append-only table: `| Date | Run id | Commit | Conclusion | Queue s | Setup s | Gate s | Total s |` rows (last row 2026-11-10, run 42727011942, gate 9 s) and bullets under `## Decisions`. Amend the line "**Add a macOS leg only when the C build arrives in Phase 2.**" to record that D-24 supersedes it. Run ids come from the orchestrator, never from an agent calling `gh`.

---

## Shared Patterns

### S1. Registering a gate step (and what lands atomically)

**Source:** `tools/gate.sh` lines 35-51 (inventory), 54-65 (run), 84 (summary).
1. Add `tools/check/steps/<name>.sh` (S2 header, ends with `-- <name> pass <stats>`).
2. Add `<name>` as the first token of a line in `data/expected/gate-steps/<file>.ids` (D-22: `cpu.ids`). Mismatch prints `-- FAIL gate-step-missing: <name>` (listed, no script) or `-- FAIL gate-step-extra: <name>` (script, not listed).
3. Steps run in C-locale alphabetical order of file name: `c-build c-faults c-purity c-tests ci-parity ... cp1610 cp1610-reader cp1610-vectors ... sources tests-inventory text vocab`. A step that reads another step's output (`tests-inventory` reads what `c-tests` wrote) is correctly ordered by name; do not rely on any other ordering.
4. Every step needs at least one control (S3) in `data/expected/controls/*.ids`, plus the `_good/tree` additions that make its positive control pass.
5. Gate-red-between-plans rule (RESEARCH Section 3, A-08): the step script, its ids line, its control directories and inventory lines, and its `_good/tree` fixtures land in ONE plan.
6. `.github/workflows/gate.yml` does not change: `ci-parity` allows only `run: sh tools/gate.sh` and `run: cmake --version`, no `if:`, no `continue-on-error` (`ci-parity.sh` header lines 7-16); any apt or sysctl step would need a `ci-parity` edit.

### S2. Step preamble, failure token and pass-line shapes

**Source:** `steps/denylist.sh` lines 12-20, 70-71; `lib.cmake` lines 8-16, 35-44; `gate.sh` lines 15, 59, 84.
```sh
#!/bin/sh
# SPDX-License-Identifier: MIT
# Step <name>: <what it checks>. Tokens: ... Stats: ...
LC_ALL=C; export LC_ALL
: "${ROOT:?ROOT is not set}"
: "${CHECK_DIR:?CHECK_DIR is not set}"
: "${FILELIST:?FILELIST is not set}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/<name>.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT
trap 'exit 1' HUP INT TERM
rc=0
bad() { printf -- '-- FAIL %s: %s\n' "$1" "$2"; rc=1; }
...
[ "$rc" -eq 0 ] || exit 1
printf -- '-- denylist pass scanned=%s waived=%s\n' "$scanned" "$waived"
```
- Failure line: exactly `-- FAIL <token>: <detail>`. The detail names a file, id or path, never matched secret text. Findings are printed in sorted order (`sort`, or `sort -t: -k1,1 -k2,2n` as in `text.sh` line 39). Never stop at the first failure; end with a nonzero exit.
- cmake -P checkers print the same shape through `gate_fail` (`message(STATUS "FAIL ...")`, which cmake prints as `-- FAIL token: detail`) and finish with `gate_finish(<name>)`, which prints `-- <name> pass <k=v ...>` or ends in one FATAL_ERROR.
- Pass line: `-- <step name> pass k=v k=v`. The gate harvests `sed -n "s/^-- $name pass //p"` per step into the final summary line, so **stat keys must be unique across all steps** (D-22 keys: `cc=`, `tests-expected=`, `tests-executed=`, `tests-skipped=`, `tests-missing=`, `faults=ok/total`, `core-imports=`, `purity-selftest=n/n`). Wrappers drop `records=<n>` (not unique) and keep the rest (`cp1610-reader.sh` line 14).
- Pass output is terse; build logs only on failure.
- Run every new script once under `/bin/dash` (hosted image uses dash; `/bin/sh` here is not dash).

### S3. Control declaration (CHECK / EXPECT / FORBID / MAKE / overlay)

**Source:** `tools/check/run-controls.sh` (lines 4-6 header, 23-81 MAKE verbs, 83-94 tree build, 96-98 step call, 100-118 inventory, 120-135 positive control, 137-172 negative control).
- Directory `tools/check/controls/<name>/` holds `CHECK` (first line = step name, must exist in `steps/`), `EXPECT` (one token per line, matched as a **substring** with `grep -q -F -e` against the step output; non-empty, **no blank lines**, else `control-expect-blank`), optional `FORBID` (substrings that must NOT appear), optional `MAKE`, optional `overlay/` (copied over a fresh copy of `_good/tree` with `cp -R overlay/. tree/`, dotfiles included, same path overwrites).
- The step must exit nonzero (`control-exit-zero` otherwise) and print every EXPECT token (`control-token-missing`). Convention: `FAIL <token>:` or `FAIL <token>: <detail>`.
- MAKE verbs (one per line, `#` comments, arguments are whitespace-split, so `write <path> <one-word>` only): `delete`, `empty`, `write`, `rename`, `crlf`, `bom`, `nonascii`, `homepath`, `secret`, `password`, `binary`, `highbytes`, `nul`, `scratchpath <kind>`, `credential <kind>`, `email`. New verbs need a runner change; D-17 says the inventory controls need none. Bad bytes (home paths, credentials, NUL) are assembled at run time by MAKE so the repo never holds them.
- The runner calls the step with only `ROOT=<temp tree>`, `FILELIST=<find of the tree>`, `CHECK_DIR=<real tools/check>` (line 97). `GATE_TMP` is unset. Anything a step needs besides these must be under `$ROOT`.
- Positive control: for each distinct CHECK the pristine `_good/tree` must pass and print `-- <step> pass` (lines 126-135), so `_good/tree` must carry what the new step reads.
- Inventory: every control directory name must be a first token in some `data/expected/controls/*.ids` (`control-not-in-inventory` / `control-inventory-orphan`). Final line `controls=<ok>/<total>`; the gate reads it (`gate.sh` lines 72-79).
- Existing families to model on: `ci-parity-*` (workflow overlays), `claims-*`/`profile-*`/`discrepancies-*` (record overlays), `integrity-*` (MAKE), `denylist-*` (overlay under `src/`), `files-*` (MAKE), `runner-selftest` (controls of the runner itself).

### S4. Integrity rules every new file must satisfy

**Source:** `tools/check/integrity.cmake` lines 32-35, 46-70; `steps/integrity.sh`; `binscan.sh`; `.gitignore`.

| Rule | Applies to | Token |
|------|------------|-------|
| Path charset `^[A-Za-z0-9._/-]+$` (no `@`, `+`, space) | every tracked or untracked-not-ignored path | `integrity-path-charset` |
| Non-empty (so no `.gitkeep`, no empty `include/`) | every file whose extension is not in `tools/check/binary-ext.txt` | `integrity-empty-file` |
| No BOM, no CR, no NUL | same files (NUL via `binscan.sh nul`) | `integrity-bom`, `integrity-crlf`, `integrity-nul-byte` |
| ASCII only | paths under `data/`, `fixtures/`, `tools/`, `.github/` TODAY (includes control `CHECK`/`EXPECT`/overlay files and TAP fixtures under `tools/`); `core/`, `include/`, `tests/` only after D-23 | `integrity-non-ascii` |
| `SPDX-License-Identifier: MIT` | TODAY: `tools/**/*.sh`, `tools/**/*.cmake`, `.github/workflows/*.yml`; after D-23: `.c`/`.h` under `core/`, `include/`, `tests/` | `integrity-missing-spdx` |
| Not binary by content (more than 30 percent non-text bytes or any NUL) | files with no extension or an extension not in `text-ext.txt` (`.c`, `.h`) | `files-unlisted-binary` |
| No output with a rights-bearing extension | `.gitignore` already ignores `*.bin *.rom *.int *.cfg *.itv *.cc3 *.pdf *.png` and `/build/`, `/build-*/` | n/a |

JSON (`data/*.json`, `CMakePresets.json`) cannot hold an SPDX comment; the existing JSON records have none. Data files carry no SPDX line; `tools/check/*.txt` lists carry none either.
`fixtures/` triggers manifest rules (`files.cmake`): vectors live in `data/`, not `fixtures/`.

### S5. Hygiene, denylist, text and glossary traps for new content

**Source:** `tools/hygiene.sh` lines 14-52; `steps/denylist.sh` lines 33-45; `tools/check/denylist.txt`, `denylist-cite.txt`; `steps/text.sh`; `steps/glossary.sh`.
- `denylist` (scans every FILELIST path except `.planning/`, `.claude/`, `docs/provenance/`, `tools/check/`, `docs/provenance.md`, `data/sources.json`; case-insensitive, comments included): the four author or licence-header strings in `denylist.txt` are never allowed; the emulator name in `denylist-cite.txt` is allowed only on a line carrying `[cite-only]`. So `core/`, `tests/`, `cmake/`, `data/cp1610/vectors/`, `docs/` must not mention them in code comments, vector `note`/`derivation` text or commit-visible docs. D-14 also wants "no emulator names" in vector files: do not name any other emulator either.
- `hygiene` (scans everything including `.planning/`): home paths, scratch and temp paths (`/tmp/claude-*`, tilde-relative paths of 3+ characters), credential shapes, and non-noreply email-shaped strings. Verified by running the pattern from `tools/hygiene.sh` line 44 (`EMAIL=`): an at-sign followed by an alphanumeric, then dots and a letters-only final part, is an email; an at-sign followed by `.` or a space is not.
- `text` (README and `docs/**/*.md`): claim-word tripwire. `glossary` (same files): bare `clock`.
- Denylist words are banned in code comments, not just code (A-09).

### S6. New JSON record kind via `records.cmake`

**Source:** `tools/check/records.cmake` (inputs lines 11-19; schema DSL parse lines 41-79; hook include 255-257; per-record loop 259-311; inventory 313-327; finish 329-333); examples `schema/second-reader.txt`, `schema/discrepancy.txt`, `schema/profile.txt`.
Checklist to add a kind `<k>`:
1. `tools/check/schema/<k>.txt` with directives `file <relpath>` (exactly one file), `topkeys`, `top <key> <TYPE|TYPE>`, `array <key>` + `idkey <key>` or `members <key>`, `inventory <ids file>`, `keys` (allowed per-record keys; extras give `<k>-unknown-key`), `req <key> <TYPE|TYPE>` (types `STRING NUMBER OBJECT ARRAY BOOLEAN NULL`; path keys use `/`), `nonblank`, `match <key> <regex>` (no spaces, no `{n}`), `enumfrom <key> <file> <array> <idkey>`, `when <key>=<value> <rule>`, `hook <name>`.
2. Optional `tools/check/kinds/<name>.cmake` defining `hook_record(id rec)` and `hook_finish()` (state through `gate_incr`, `gate_count`, `set_property(GLOBAL APPEND PROPERTY ...)`; `REC_DOC` holds the whole document in `hook_finish`).
3. `data/expected/<inventory>.ids` (one id per line, whole-line ids).
4. A step `tools/check/steps/<step>.sh` that runs `cmake -DROOT="$ROOT" -DKIND=<k> -P "$CHECK_DIR/records.cmake"` (S2 wrapper form), plus its `gate-steps` line, controls and `_good/tree` record file and inventory.
Generated failure tokens: `<k>-file-missing|file-empty|parse|unknown-topkey|missing-top|type|empty|duplicate-id|id-missing|id-extra|inventory-empty|inventory-missing|inventory-duplicate|unknown-key|missing|blank|format|unknown-ref|schema`.
Limits: one file per kind; duplicate JSON member keys are not detectable (CMake keeps the last); a schema regex cannot contain a space. Multi-file data (the ~10 vector files) therefore needs the dedicated `vectors.cmake` (A-07); only the single review marker is a `records.cmake` kind.

### S7. `.ids` inventory conventions (two readers)

**Source:** `gate.sh` line 45, `run-controls.sh` line 112 (awk reader); `lib.cmake` lines 73-116 (cmake reader).
- awk/sh reader: `awk '/^#/ { next } NF { print $1 }'` then `sort -u`. A comment must start in column 1; extra tokens on a line are ignored (so `skip-ok`, a discrepancy id, or `<fault> <must-fail> <prefix>` columns work here only).
- cmake reader `read_ids`: strips the line, skips `#` lines (so indentation is tolerated), takes the WHOLE line as the id, sorts, fails on duplicates, accepts `#allow-empty: <reason>`. Files read by `records.cmake` inventories or `vectors.cmake` (`cp1610-vectors.ids`, marker ids) must be single-token lines.
- Header comment convention: `# <what>; must equal ...` or `# Owning plan 0N-NN: ...`. Sets are compared exactly (extra and missing both fail). If a file is read by both readers, keep it single-token.

### S8. Portability and measured numbers

- Hosted runner is dash 0.5.12, mawk [ASSUMED], binutils 2.42; local is macOS `sh`, BWK awk, llvm-nm. POSIX only: no `sed -i`, no `\+`/`\|` in sed, awk without `gensub`/interval expressions.
- `LC_ALL=C` at the top of every script (all existing scripts) so `sort`, `comm` and character classes behave the same on both hosts.
- `mktemp -d "${TMPDIR:-/tmp}/<name>.XXXXXX"` plus the two-trap pair is the temp-dir idiom everywhere; never write inside the repo except under `build/`.
- Gate cost today: 19 s wall locally with 17 steps and 112 controls; the control runner is sys-time bound by per-control `cp -R` of `_good/tree`. Keep `_good/tree` additions minimal (D-22 lever order applies if the budget is hit).

## No Analog Found

Files with no close match in the codebase (planner should use CONTEXT D-xx and RESEARCH "Code Examples" instead):

| File | Role | Data Flow | Reason |
|------|------|-----------|--------|
| `CMakeLists.txt`, `CMakePresets.json`, `cmake/IvsWarnings.cmake` | config | n/a | No product build exists. Spec: D-19 and RESEARCH Pitfalls 9, 10. Style only: copy the `tools/check/*.cmake` conventions (SPDX first line for `.cmake`, `cmake_minimum_required(VERSION 3.25)`, 2-space indent, lowercase commands). `CMakePresets.json` is JSON, so no comment line is possible. |
| `core/src/common/ivs_prelude.h` | core header | n/a | No C. Use RESEARCH "Code Examples": std headers first, `#pragma GCC poison float double` last (poison before `<stddef.h>` fails on Apple clang 21, executed). Add a control proving the prelude is mandatory. |
| `core/src/common/ivs_status.h`, `core/src/cpu/cp1610.h`, `cp1610.c`, `cp1610_decode.c` | core | request-response (state, lines, replies) | No C. Spec: D-01..D-13 (32-byte state, 12-byte access record, `ivs_` prefix, `& 0xFFFFu` masking, no allocation, `__builtin_trap()` instead of `assert.h`). The only repo conventions that carry over: SPDX first line, ASCII, LF, non-empty, no denylist words in comments, no float literal in comments or hex that the purity grep could trip on (see c-purity traps). |
| `tests/harness/ivs_test.{h,c}`, `tests/cpu/*.c` | test harness and suites | event-driven (TAP print) | No C and no TAP producer. Spec: D-17 (header `# suite=<s> nonce=<n>`, `1..N`, `ok N name`, `not ok N name`, `ok N name # SKIP reason`, trailer `# suite=<s> executed= skipped= failed=`, exit 1 on failure, 77 only for an all-skipped suite, `--only`, `--list`). Consumer side is `tests-inventory` (above). |
| `tests/faults.cmake` | config | n/a | No analog. Spec: D-18 (`ivs_add_fault(ID ... SOURCE ... SUITE ...)`, owning-TU recompile, EXCLUDE_FROM_ALL, sanitizer flags per target not global, A-09). |
| core-symbols `nm -P` parsing | transform | n/a | No analog. Spec: D-21 and RESEARCH Pitfall 2 (key on `$1`/`$2`; skip lines with fewer than two fields or ending in `:`; archives add a blank line and a `member.o:` header; Darwin extra leading underscore; GNU shapes are [ASSUMED] until the first hosted run). |

## Metadata

**Analog search scope:** `tools/` (gate, steps, check scripts, schema, kinds, controls), `data/` (records and `expected/` inventories), `docs/`, `.github/workflows/`, `.gitignore`, `README.md`, `.planning/CI-EFFICIENCY.md`, `.planning/phases/02-cp-1610-cpu-core/`. No `.c`/`.h` files exist in the repository.
**Files scanned:** about 70 read in full or by range, one `sh tools/gate.sh` run (read-only, 19 s) for baseline output shapes, and four probes (analog tracked-ness, hygiene email regex vs vector id shapes, D-21 float-literal regex vs hex and comment cites, `nm`/shell tool availability).
**Pattern extraction date:** 2026-11-09
