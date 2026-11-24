# Phase 1: Evidence, Profile and Rights Foundations - Pattern Map

**Mapped:** 2026-11-09
**Files analyzed:** 37 file groups (33 new, 4 modified or procedural)
**Analogs found:** 31 / 37 (6 have no direct analog; see "No Analog Found")

**Context.** This repository holds planning docs only. Every analog below is a same-owner sibling repo (method reuse permitted; MIT). All analog paths were verified git-tracked inside their own repo (`git ls-files`, tracked-source gate). Paths use the `../sibling/...` form only. Never write an absolute home path into a tracked file (hygiene rule; the repo goes public). No emulator source was opened for this map.

**Copy the method, not the gate.** Siblings are large (`../gabbaboy/cmake/*.cmake` is about 4,000 lines; lesson GB-CI-004 in `../gabbaboy/.planning/context/LESSONS.md`). Keep excerpts to the idiom, keep each new checker small and generic.

## File Classification

| New/Modified File | Role | Data Flow | Closest Analog | Match Quality |
|-------------------|------|-----------|----------------|---------------|
| `tools/gate.sh` | orchestrator (config) | batch, request-response (exit code) | `../nesturbator/scripts/hygiene.sh` (mode dispatch, `found()`, exit status) | role-match |
| `tools/hygiene.sh` | utility (policy scan) | batch (file list and rev list) | `../nesturbator/scripts/hygiene.sh` | exact |
| `tools/check/lib.cmake` | utility | transform | `../gabbaboy/cmake/VerifyEvidenceLedger.cmake` | role-match |
| `tools/check/records.cmake` | checker | batch (walk JSON records) | `../gabbaboy/cmake/VerifyFixtureManifest.cmake` + `VerifyEvidenceLedger.cmake` | role-match |
| `tools/check/ids.cmake` | checker | batch (set equality) | `../gabbaboy/cmake/ExpectedTests.cmake` | exact |
| `tools/check/coverage.cmake` | checker | transform (1024-pattern loop) | `../gabbaboy/cmake/CheckCoreNoHostClock.cmake` (header contract, regex idiom only) | partial |
| `tools/check/run-controls.sh` | test harness | request-response (run, assert exit and token) | `../gabbaboy/tests/expect_exit.cmake` + selftests in `../gabbaboy/tests/CMakeLists.txt` | role-match |
| `tools/check/controls/**` | test fixture (seeded bad trees) | file-I/O | selftest ledger in `../gabbaboy/tests/CMakeLists.txt` 2523-2536 | role-match |
| `tools/check/schema/*.txt` | config | transform | none | no analog |
| `tools/check/step-NN-*.sh` | checker steps | batch | `../nesturbator/scripts/hygiene.sh` `check_file()` | partial |
| `tools/check/denylist.txt` + text scan step | checker | batch | `../gabbaboy/cmake/CheckCoreNoHostClock.cmake` 41-49 | role-match |
| `data/expected/*.ids` | model (inventory) | file-I/O | `../gabbaboy/tests/expected-tests.txt` | exact |
| `data/profiles/intv-2609-ntsc-family-p1.json` | model (record) | CRUD (static data) | `../gabbaboy/fixtures/tracer/manifest.json` (`profile` block) + `../gabbaboy/docs/cgb-profile.md` | role-match |
| `data/tiers.json` | model (vocabulary) | static data | `../gabbaboy/docs/cgb-profile.md` 75-83 (evidence-class table) | role-match |
| `data/claims.json` + `docs/evidence-tiers.md` | model + doc | CRUD | `../gabbaboy/docs/mbc3-rtc-evidence.md` + `VerifyEvidenceLedger.cmake` | role-match |
| `data/discrepancies.json` | model (record) | CRUD | `.planning/research/SUMMARY.md` §Source Conflicts (in-repo seed table) | exact (seed) |
| `data/sources.json` | model (registry) | CRUD | `../nesturbator/PROVENANCE.md` 60-62 table + `../gabbaboy/fixtures/mooneye/manifest.json` 2-8 | role-match |
| `data/cp1610/opcode-families.json` | model (inventory rows) | static data | `../gabbaboy/fixtures/tracer/manifest.json` `reachable_opcodes` (row shape only) | partial |
| `data/cp1610/second-reader.json` | model | static data | none | no analog |
| `fixtures/FX-sample-rights-unknown/manifest.json` | model (manifest) | CRUD | `../gabbaboy/fixtures/tracer/manifest.json` + `fixtures/mooneye/manifest.json` | role-match |
| manifest sample/fail-closed rights check (inside `records.cmake`) | checker | batch | `../gabbaboy/cmake/VerifyFixtureManifest.cmake` + `ScanForbiddenFixtures.cmake` | role-match |
| `docs/provenance.md` | doc | n/a | `../nesturbator/PROVENANCE.md` | exact |
| `docs/provenance/CONSULTATIONS.md` | doc (log) | append-only | `../nesturbator/PROVENANCE.md` "AI assistance" 44-48 (consulted-sources statement) | partial |
| `docs/RIGHTS.md` | doc | n/a | `../nesturbator/ASSET_POLICY.md` | exact |
| `THIRD_PARTY_NOTICES.md` (optional) | doc | n/a | `../gabbaboy/THIRD_PARTY_NOTICES.md` | role-match |
| `docs/evidence-tiers.md` | doc | n/a | `../gabbaboy/docs/mbc3-rtc-evidence.md` "Evidence classes" 37-44 | role-match |
| `docs/time-base.md` | doc (glossary) | n/a | `../nesturbator/GLOSSARY.md` (term/Avoid format) + `../gabbaboy/cmake/VerifyDocContract.cmake` (term-inventory check) | role-match |
| `docs/cp1610/reference-notes.md` | doc | n/a | none direct; `../gabbaboy/docs/cgb-profile.md` for tag-per-value layout | no analog |
| `README.md` | doc | n/a | `../nesturbator/README.md` 2440-2448 | exact (notice only) |
| `LICENSE` | config | n/a | `../gabbaboy/LICENSE` | exact |
| `.gitignore` | config | n/a | `../gabbaboy/.gitignore` 1-26 | role-match |
| `.github/workflows/gate.yml` | CI config | event-driven | `../nesturbator/.github/workflows/ci.yml` 33-35, 42-56 | role-match |
| `.github/pull_request_template.md` (optional) | doc | n/a | none; carries the D-14 checklist | no analog |
| `.planning/CI-EFFICIENCY.md` | doc (measurement log) | append-only | `../gabbaboy/.github/workflows/ci.yml` line 73 timeout-ledger comment | partial |
| history scrub (procedure, not a committed file) | procedure | batch | `01-RESEARCH.md` "History scrub" (rehearsed) | no analog (in-repo research) |
| `.planning/PROJECT.md` (modify: verify D1/D2/D3/D9 rows) | doc | CRUD | existing rows, `.planning/PROJECT.md` 157-166 | exact |
| `.claude/CLAUDE.md`, `.planning/research/*.md` (modify: scrub home paths) | doc | transform | `01-RESEARCH.md` scrub sed set | n/a |

## Pattern Assignments

### `tools/check/lib.cmake` and every `tools/check/*.cmake` (checker scaffolding)

**Analog:** `../gabbaboy/cmake/VerifyEvidenceLedger.cmake` (best scaffolding); `VerifyDocContract.cmake` and `ScanForbiddenFixtures.cmake` agree on it.

**Header contract: Inputs, failure tokens, success line** (VerifyEvidenceLedger lines 1-14):
```cmake
# Inputs (passed with -D):
#   GBB_LEDGER       the Markdown ledger to check (required)
# Failure tokens: evidence-ledger-unregistered: <name>, evidence-ledger-empty.
# Success line: "evidence-ledger pass names=<n>".
cmake_minimum_required(VERSION 3.25)
```
Copy the three-part header and the explicit `cmake_minimum_required(VERSION 3.25)` baseline (D-07). `VerifyFixtureManifest.cmake` omits the baseline; do not copy that omission.

**Required-input guard** (VerifyEvidenceLedger lines 16-20):
```cmake
foreach(required GBB_LEDGER GBB_INVENTORIES)
  if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
    message(FATAL_ERROR "VerifyEvidenceLedger.cmake is missing ${required}")
  endif()
endforeach()
```
Rename the prefix `GBB_` to `IVSV_` (or `GATE_`). Use `ROOT` for the tree under test so a control can point the real checker at a seeded tree.

**Failure accumulation, STATUS token, one final error, pass line** (lines 52-53, 58-64):
```cmake
      message(STATUS "evidence-ledger-unregistered: ${name}")
      set(failed TRUE)
...
if(checked EQUAL 0)
  message(FATAL_ERROR "evidence-ledger-empty: ${GBB_LEDGER}")
endif()
if(failed)
  message(FATAL_ERROR "evidence-ledger check failed")
endif()
message(STATUS "evidence-ledger pass names=${checked}")
```
This is the right idiom: the token rides a `STATUS` line (not word-wrapped), the script keeps going, and one `FATAL_ERROR` sets the exit code. Generalize `set(failed TRUE)` into `gate_fail(token detail)` backed by a GLOBAL property (RESEARCH Pattern 1) so nested functions can report.

**Do not copy:** `message(FATAL_ERROR "token: ...")` carrying the token as in `VerifyFixtureManifest.cmake` lines 12, 22, 27 (first failure only, text is wrapped), and `message(SEND_ERROR ...)` in `VerifyDocContract.cmake` lines 158, 167, 174 (wrapped). Always `message(STATUS "FAIL <token>: ...")`.

**JSON guards** (copy `string(JSON ... ERROR_VARIABLE)`, VerifyFixtureManifest lines 10-13), then apply RESEARCH Patterns 2-4 (TYPE guard `json_require`, extract each record once, allowed-key enumeration):
```cmake
string(JSON count ERROR_VARIABLE json_error LENGTH "${manifest_json}" fixtures)
if(NOT json_error STREQUAL "NOTFOUND" OR count EQUAL 0)
  message(FATAL_ERROR "fixture-manifest-empty: ${MANIFEST}")
endif()
math(EXPR last "${count} - 1")
foreach(i RANGE ${last})
```
The `count EQUAL 0` guard before `RANGE ${last}` is what prevents the `RANGE -1` vacuous pass (Pitfall 4). Keep it on every array walk. Replace `message(FATAL_ERROR token)` with `gate_fail`.

**Digest handling** (lines 19-28): `string(TOLOWER "${digest}" digest)` before comparing with `file(SHA256 ...)`, and `file(SIZE ...)` plus `file(SHA256 ...)` against recorded `size_bytes` and `sha256`. Reuse for `outputs[]`, `recipe.sources[]` and `notices[]` of any manifest with `committed: true`. A `committed: false` entry must not require bytes to exist (CI has none).

---

### `tools/check/records.cmake` (generic JSON record walker: profile, claims, discrepancies, sources, manifests, opcode rows)

**Analog:** `../gabbaboy/cmake/VerifyFixtureManifest.cmake` (array walk and digest) and `VerifyEvidenceLedger.cmake` (membership against an inventory).

Take the walk shape above. The new work is the 4-verb schema DSL (`req / enum / nonblank / allowkeys`) in `tools/check/schema/<kind>.txt`, which has no sibling analog. Gabbaboy wrote one bespoke script per record kind; that is the sprawl to avoid.

**Per-kind rules to encode (from CONTEXT, no analog needed):**
- profile: every field object has `value, tag, cite, conflict, resolving_evidence, revisit_trigger, decision`; known tag from `data/tiers.json`; `unknown-default-chosen` requires non-empty `resolving_evidence`; `conflict` is a C-id or JSON null (test `TYPE == NULL`, Pitfall 2).
- discrepancies: `default_policy` non-empty; `resolver` matches `^(test|measurement|pending):`; count pending, do not fail.
- claims: profile exists, tier in the eight-tier vocabulary, artifact path exists; `standing` field implements the weakest-field rule (RESEARCH A6).
- manifests: effective-redistributable rule (below).

**Effective-redistributable rule (D-13), shape to implement** (new logic; gabbaboy has no `redistributable` field):
```
effective_yes := redistributable == "yes" AND licence_evidence present AND rights_review.status != "unreviewed"
committed==true OR allowed_use contains repo|release  =>  require effective_yes
any missing or misspelled field => effective no
```
Failure tokens suggested: `manifest-unknown-key`, `manifest-committed-not-redistributable`, `manifest-yes-without-evidence`.

---

### `tools/check/ids.cmake` and `data/expected/*.ids` (exact inventories)

**Analog:** `../gabbaboy/cmake/ExpectedTests.cmake` and `../gabbaboy/tests/expected-tests.txt` (one name per line, 1,942 lines; exact match required).

**Empty-inventory guard** (ExpectedTests lines 1-5):
```cmake
file(STRINGS "${PROJECT_SOURCE_DIR}/tests/expected-tests.txt"
  GABBABOY_EXPECTED_TESTS)
if(NOT GABBABOY_EXPECTED_TESTS)
  message(FATAL_ERROR "The required CTest inventory must not be empty")
endif()
```

**Sorted set equality** (lines 142-147):
```cmake
  list(SORT expected_tests)
  if(NOT registered_tests STREQUAL expected_tests)
    message(FATAL_ERROR
      "Registered CTest cases differ from tests/expected-tests.txt.\n"
      "Expected: ${expected_tests}\nRegistered: ${registered_tests}")
  endif()
```
Adapt: both sides come from files (record ids extracted from JSON, expected from `.ids`). Report each missing and extra id on its own `STATUS` line (`ids-missing: C17`, `ids-extra: C99`) so the control can assert the token. `file(STRINGS)` keeps `#` comment lines; filter them out, and allow an empty inventory only with a `#allow-empty: <reason>` line (RESEARCH Open Question 5). Gate also needs an inventory of the control directories (`data/expected/controls.ids`) and the gate steps (`gate-steps.ids`, one owning plan).

**Lesson to carry** (GB-GAME-004 / D-037, `../gabbaboy/.planning/context/LESSONS.md` line 385-393): an inventory check that reads only one scope silently misses the rest. Make `ids.cmake` read every `data/expected/*.ids` by glob, and assert the file list itself is the expected one, so a new record kind cannot be added without its inventory.

---

### `tools/check/run-controls.sh` and `tools/check/controls/**` (negative controls)

**Analog:** `../gabbaboy/tests/expect_exit.cmake` (exit-code-plus-token assertion) and the selftests in `../gabbaboy/tests/CMakeLists.txt` lines 2523-2536.

**Core principle** (expect_exit lines 16-18, 35-48): an exact failure code and a regex on the combined output; a different failure class must not pass.
```cmake
execute_process(COMMAND ${GBB_CMD} ${gbb_workdir_args}
  RESULT_VARIABLE gbb_result OUTPUT_VARIABLE gbb_output ERROR_VARIABLE gbb_error)
set(gbb_combined "${gbb_output}${gbb_error}")
if(NOT gbb_result STREQUAL GBB_EXPECT_CODE)
  message(FATAL_ERROR "expected exit code ${GBB_EXPECT_CODE} but the command returned '${gbb_result}'\n" ...)
endif()
if(NOT gbb_combined MATCHES "${GBB_EXPECT_REGEX}")
```
In sh: `cmake -DROOT=... -P check.cmake > out 2>&1`, require nonzero exit, then `grep -q -F -f EXPECT out` (RESEARCH Pattern 5 has the tested runner; copy that).

**Forbid pattern** (expect_exit lines 65-71; use at CTest line 2535): a control also names a token that must NOT appear, so a control cannot pass because an unrelated record failed.
```cmake
    "-DGBB_EXPECT_REGEX=evidence-ledger-unregistered: rtc_no_such_test"
    "-DGBB_FORBID_REGEX=unregistered: rtc_latch_edges"
```
Adopt as an optional `FORBID` file beside `EXPECT` in each control directory. RESEARCH Pattern 5 omits this; add it. Keep the blank-line guard for `EXPECT` (a blank line makes `grep -F -f` match everything).

**Seeded bad input pattern** (CTest lines 2523-2526): the control is a few bytes of seeded input, not a copy of a tree:
```cmake
file(WRITE ".../ledger.md"
  "## Claim table\n\n| Claim | Tests |\n|---|---|\n| renamed test | `rtc_latch_edges`, `rtc_no_such_test` |\n")
```
Use `_good/tree` plus a per-control `overlay/` (RESEARCH Pattern 5). The `_good` tree must itself pass, which proves each failure is caused by the overlay.

---

### `tools/hygiene.sh` (publication hygiene: `--tree` per gate run, `--history` at publication and release)

**Analog:** `../nesturbator/scripts/hygiene.sh` (199 lines, exact role and flow). Rewrite from it; do not copy wholesale.

**Prologue** (lines 12-25): `set -eu`; `LC_ALL=C; export LC_ALL` (grep -I otherwise skips non-UTF-8 files); `git() { command git -c core.quotePath=false "$@"; }`; `cd "$(git rev-parse --show-toplevel)"`; `found() { printf 'hygiene: %s: %s\n' "$1" "$2" >&2; fail=1; }` (findings name rule and file, never the matched text).

**Regex set and the `leaks` helper** (lines 33-43). Use the RESEARCH version of `HOME_PATH` (alphanumeric first char after the home directory, plus the `-Users-<name>-` slug and `SECRET` tokens) instead of nesturbator's looser one:
```sh
EMAIL_OK='@users\.noreply\.github\.com|noreply@github\.com|@example\.(com|org|net)|git@github\.com'
MESSAGE_EMAIL_OK="$EMAIL_OK|noreply@anthropic\\.com"
leaks() { grep -I -E -e "$1" | grep -v -E -e "$2" | grep -q .; }
```

**Per-file checks and fail-closed read** (lines 55-77): unreadable file is a finding; ROM-like extension not listed is a finding; binary-by-content not listed is a finding; home path and email scans.
```sh
check_file() {
  [ "$1" = "$SELF" ] && return 0
  if ! $2 "$1" >/dev/null 2>&1; then found 'cannot read file' "$1"; return 0; fi
  if printf '%s\n' "$1" | grep -q -i -E -e "$ROM_NAME" && ! in_manifest "$1"; then
    found 'game image by name' "$1"
  fi
```
Adapt: `ROM_NAME` becomes `\.(int|bin|rom|itv|cfg|cc3|pdf|png|jpe?g|gif)$` (RESEARCH Pitfall 11); `in_manifest` reads `fixtures/*/manifest.json` through a `FILELIST`-style seam instead of an awk text manifest; add `refs/` tracked and `.env*` tracked checks.

**Mode dispatch and tree/history walks** (lines 146-158, 161-192): `--tree` runs over `git ls-files -z --cached --others --exclude-standard`; `--history` loops `git rev-list --all`, `git grep -I -h -E` per commit, scans commit messages, then `check_identity` over `git log --format='%ae%n%ce' | sort -u`.
```sh
  --tree)
    check_files "$(git ls-files -z --cached --others --exclude-standard | tr '\0' '\n')" tree_show
    ;;
```
Adapt: gate runs `--tree` every time; `--history` only at publication and before each release (RESEARCH "Why hygiene --history is not in every gate run").

**Drop:** `--staged` mode, `check_stop_policy` (nesturbator-specific chain-mode check), `check_vector_json_budget`, the NES magic-byte case at lines 64-67.

---

### `tools/gate.sh` (single local command; CI runs exactly this)

**Analog:** no sibling gate script; the closest flow is `../nesturbator/scripts/hygiene.sh` (accumulate with `found()`, run every check, `exit "$fail"`, usage on bad mode lines 193-199).

Pattern to copy: do not `set -e`-abort at the first failure; run every step, print each step's result line, then one summary and one exit code. Required shape (CONTEXT D-12, RESEARCH diagram): drop-in `tools/check/step-NN-*.sh`, the step list is an inventory (`data/expected/gate-steps.ids`), `steps-run == expected` is the vacuous-pass guard, summary line `claims=n pending-resolvers=m unknown-fields=k controls=n/n`. Use POSIX sh only; test under `dash`. Avoid `[[`, `local`, arrays, `pipefail`, `echo -e`, `sed -i`, `grep -P` (Pitfall 8). Hash via `cmake -E sha256sum`, not `shasum`/`sha256sum`.

---

### `tools/check/denylist.txt` and the text-scan step (D-12.7)

**Analog:** `../gabbaboy/cmake/CheckCoreNoHostClock.cmake` lines 41-49 and 21-25.
```cmake
set(gbb_denied_calls time clock clock_gettime ... getenv ...)
list(JOIN gbb_denied_calls "|" gbb_denied_call_alternatives)
set(gbb_call_regex
  "(^|[^A-Za-z0-9_])(${gbb_denied_call_alternatives}|GetSystemTime[A-Za-z]*|GetTickCount[0-9]*)[ \t]*\\(")
```
Takeaways: a named denylist file joined into one pattern, a left-context class `(^|[^A-Za-z0-9_])` because CMake regex has no `\b`, a documented token per failure (`no-host-clock-call: <file>`). In this repo do the scan in sh with `grep -F -f tools/check/denylist.txt` and `grep -w`. Emit `denylist-hit: <file>` only (file, not matched text). Scope per RESEARCH Pitfall 9.1: scan code and fixture paths only (everything except `.planning/`, `docs/`, `data/`, `README.md`, `AGENTS.md`, `.claude/`, `tools/check/denylist.txt`, `tools/check/controls/`), otherwise the legitimate provenance docs fail. Keep one control that seeds a hit in `src/x.c`. The full no-host-clock proof (`CheckCoreNoHostClock.cmake` source layer plus nm object layer) is a Phase 2 item, not Phase 1.

---

### `tools/check/coverage.cmake` (1024-pattern decode coverage)

**Analog:** none for the logic. Reuse only the header-contract and regex idiom from `../gabbaboy/cmake/CheckCoreNoHostClock.cmake` (lines 21-25 and 46-49).

Use RESEARCH Pattern 7 (row shape `{id, printed, pattern, refines?, eq?}`, `math(EXPR ... & | << >>)` bit tests, measured 0.5 s, `coverage-unclaimed` / `coverage-ambiguous` tokens). Run `string(JSON ... GET)` once per row, not per field (Pattern 3).

---

### `data/profiles/intv-2609-ntsc-family-p1.json`

**Analog:** `../gabbaboy/fixtures/tracer/manifest.json` lines 15-22 (the `profile` block) and `../gabbaboy/docs/cgb-profile.md` 75-83 (evidence class per value).
```json
"profile": {
  "name": "DMG-CPU-B",
  "boot": "skipped; starts at cartridge entry 0x0100",
  "applicability": "Deterministic emulator profile; not evidence of boot-ROM execution or hardware conformance",
  "cpu_handoff": "A=01 F=B0 ... PC=0100 SP=FFFE for this nonzero header checksum",
  "ram_policy": "Emulator-defined zero fill; hardware WRAM/HRAM power-up contents are unspecified"
},
```
Copy the register-the-limit wording into each field's `note` (for `power_on_fill`, `reset.*`, `board_revision`): state that a value is an emulator-defined choice and not hardware evidence. Gabbaboy's profile fields are bare strings; ours are objects with `value/tag/cite/conflict/resolving_evidence/revisit_trigger/decision` (D-10), so the per-field tag column of `docs/cgb-profile.md` ("Class | Meaning | Where allowed") is the closer conceptual match. In gabbaboy a test parses the doc table against code in both directions (cgb-profile.md lines 1-5); the equivalent here is the Phase 2 generated header freshness check (deferred in CONTEXT).

---

### `data/tiers.json`

**Analog:** `../gabbaboy/docs/cgb-profile.md` lines 75-83, with `../gabbaboy/docs/mbc3-rtc-evidence.md` lines 37-44.
```
| Class | Meaning | Where allowed |
| `ORACLE_SAMEBOY` | A differential oracle (SameBoy). An emulator is not hardware evidence. | Any column. |
| `POLICY` | A deterministic project choice. Every such cell carries a reason. | Any column. |
```
Shape each vocabulary entry as `{id, meaning, where_allowed}`. Keep the two vocabularies in separate top-level arrays (`source_authority_tags`, `evidence_tiers`) and never merge them (D-09). The existing "emulator is not hardware evidence" and "author-attested, this project made no physical observation" phrasings are the model for the `tier5-emulator-hint` and `measured` meanings.

---

### `data/claims.json` and `docs/evidence-tiers.md`

**Analog:** `../gabbaboy/docs/mbc3-rtc-evidence.md` (claim table, last column cites registered names) with `../gabbaboy/cmake/VerifyEvidenceLedger.cmake` as the checker.

Copy the idea: every cited artifact must exist in an inventory, a renamed or removed target fails, and an empty claim set fails (`evidence-ledger-empty`). Wire it with a negative control (the gabbaboy selftest at `tests/CMakeLists.txt` 2523-2536).

**Do not copy** the mechanism: VerifyEvidenceLedger.cmake lines 41-47 parse the last cell of a Markdown table (`string(REGEX MATCHALL "`[a-z][a-z0-9_]*`" ...)`). D-06 forbids parsing Markdown tables. Source of truth is `claims.json`; prose cites `[claim:CL-0007]`; the doc-side check is the grep in RESEARCH "Claim-language tripwire and claim-ref extraction" (`grep -o -h -E '\[claim:[A-Za-z0-9-]+\]'`). Seed 3-6 real claims (C17 stack direction, 1024-pattern coverage, profile record completeness).

---

### `data/discrepancies.json`

**Analog:** `.planning/research/SUMMARY.md` §"Source Conflicts and Unresolved Disagreements", lines 127-160 (columns `ID | Topic | What the sources say | Status | Resolve by`, rows C1-C28).

Map columns to JSON keys: `id` (`C<n>`, never reused), `topic`, `sources` (what the sources say; keep emulator names as citations of what a source says, never copy code), `status` (`open | resolved-primary | partial | withdrawn`), `default_policy`, `resolver` (`test:` / `measurement:` / `pending:`). Add C29-C32 from CONTEXT D-21 and set C17 to `resolved-primary`, C19 to partial. Set-equal to `data/expected/discrepancies.ids`.

The only JSON precedent for "item plus reason" is `excluded_candidates[{source_path, reason}]` in `../gabbaboy/fixtures/mooneye/manifest.json` lines 33+; otherwise this is new.

---

### `data/sources.json`

**Analog:** `../nesturbator/PROVENANCE.md` lines 60-62 (File | Upstream | Commit | SHA-256 table, "ctest checks its SHA-256") and `../gabbaboy/fixtures/mooneye/manifest.json` lines 2-8:
```json
"suite": { "name": "...", "revision": "<sha>", "tree_sha1": "<sha>", "license": "MIT; see LICENSE.txt",
           "upstream": "https://github.com/.../tree/<sha>" }
```
Keys per D-15: `id` (`SRC-<slug>`), `title`, `url`, `wayback`, `accessed`, `sha256`, `size_bytes`, `rights: unknown`, `authority_tier`, `local_path` (`refs/<SRC-id>.<ext>`). Pre-computed hashes are in `01-RESEARCH.md` "Environment Availability". Hash verification of cached PDFs runs only when present and prints `refs-verified=0/N (cache absent)` (Pitfall 12). Do not copy gabbaboy's free-text `license`; use `rights: unknown` plus a tier.

---

### `data/cp1610/opcode-families.json` and `data/cp1610/second-reader.json`

**Analog (row shape only):** `../gabbaboy/fixtures/tracer/manifest.json` `reachable_opcodes`:
```json
{ "address": "0x0100", "bytes": "C3 50 01", "instruction": "JP 0x0150" },
```
Flat objects, one per row, explicit strings, no nesting beyond need. Use RESEARCH Pattern 7 row shape plus `cites` (`pdf <n>`), `cycles`, `status_bits`, `interruptible`. Dependent check: `coverage.cmake`. No analog for `second-reader.json`.

---

### `fixtures/FX-sample-rights-unknown/manifest.json` (and its flipped control)

**Analog:** `../gabbaboy/fixtures/tracer/manifest.json` lines 1-14 (`id`, `source`, `license`, `build` recipe with tool, version, archive sha256, command) and `../gabbaboy/fixtures/mooneye/manifest.json` lines 16-25 (builder with `source_license`).
```json
"build": { "assembler": "RGBDS", "version": "1.0.1", "archive_sha256": "2f6c...", "recipe": "rgbasm -o ... && rgblink ..." },
"builder": { "name": "WLA-DX", "source_license": "GPL-2.0-or-later; build tool only, not linked into fixture ROMs" }
```
Map into D-13 `recipe: {tool, tool_version, tool_sha256, tool_licence, command, sources[]}`. Keep the "build tool only, not linked or shipped" licence note; it matters for the as1600 question in the stack research.

**What the siblings lack and this schema adds:** `committed`, `redistributable: yes|no|unknown`, `licence_evidence`, `rights_review`, `allowed_use`, `derived_from`, `schema_version`, a fail-closed allowed-key list. Gabbaboy's `license` is free text and nothing fails closed on it. The sample entry: `redistributable: unknown`, `committed: false`, `allowed_use: [local-test]`. The control: same entry with `committed: true` (expect `manifest-committed-not-redistributable`).

**Admission-policy precedent:** `../gabbaboy/.gitignore` and `../gabbaboy/cmake/FixtureAdmissionPolicy.json`: nothing is admitted unless explicitly listed (see `.gitignore` below).

---

### Unlisted-binary and `refs/` checks (inside `tools/hygiene.sh` and a step script)

**Analog:** `../gabbaboy/cmake/ScanForbiddenFixtures.cmake` for the identity idea; `../nesturbator/scripts/hygiene.sh` `check_file()` for the tracked-file loop.

ScanForbiddenFixtures builds a size list and a digest list from every manifest, matches by size first then SHA-256, so renaming a file does not hide it (lines 99-104, 152-161):
```cmake
file(GLOB_RECURSE scanned_files LIST_DIRECTORIES false RELATIVE "${GBB_SCAN_DIR}" "${GBB_SCAN_DIR}/*")
list(SORT scanned_files)
list(LENGTH scanned_files scanned_count)
if(scanned_count EQUAL 0)
  message(FATAL_ERROR "scan-dir-empty: ${GBB_SCAN_DIR}")
endif()
```
Adapt: replace `GLOB_RECURSE` with `file(STRINGS "${FILELIST}")` (RESEARCH Pattern 6 seam so controls need no git), keep the `scanned_count EQUAL 0` guard and the `string(TOLOWER digest)` note (line 80-81). Also assert every tracked path matches `^[A-Za-z0-9._/-]+$`.

---

### `docs/provenance.md` (FND-05 no-copy rule and review checklist)

**Analog:** `../nesturbator/PROVENANCE.md` (exact; written before any emulator code, same discipline).

**Facts versus expression** (lines 12-27), reuse nearly verbatim:
```markdown
- **Facts may enter.** What the hardware does ... is recorded in this project's own words, with a citation ...
- **Another emulator's expression may not.** Code, comments, identifier names, table layouts and file
  structure are not copied, ported, translated or paraphrased from any other emulator, whatever its
  licence, and whether a person or an AI assistant does the rewriting.
- **Copyleft emulator source stays closed.** ... is not opened, and no copy of it is kept in the workspace.
```
**Adapt per D-14:** add the tier rule (tier 1-3 sources for behaviour, constants and tables; tier 4-5 are leads only, a tier-4/5-only value is `[UNK]`); permissive-licence emulator source readable only for discrepancy hints, each reading logged in `docs/provenance/CONSULTATIONS.md` and the value re-derived from a tier 1-3 source (nesturbator does not log readings; this is the added control); call the process "documented independent derivation", not "clean-room".

**AI assistance and honest limits** (lines 36-52 and 85-89): copy the structure of "An assistant writes code from hardware documentation ... A constant, table or name that the cited sources do not explain is treated as suspect: it is re-derived ... or removed, before it merges." and the closing "What this file does not do" section ("keeps the project's own licence clean. It is not legal advice"). D-14 requires the explicit limit that the checks catch accidents, not authorship (renaming evades the denylist).

**Doc-text tripwire note:** write one sentence per line; any line using `supports|compatible|passes|accurate|verified|runs` needs `[claim:...]` or `[nc]` on the same line (D-12.4, Pitfall 9.2).

---

### `docs/RIGHTS.md` (and optional `THIRD_PARTY_NOTICES.md`)

**Analog:** `../nesturbator/ASSET_POLICY.md` (exact role). Sections to reuse: "Never" (lines 6-21), "Allowed" (23-31), "Local testing" (33-37), "Names" (39-42), "How this is checked" (44-49).
```markdown
- A header or checksum database copied from another project. None publishes a licence.
- Test ROMs whose authors state no licence, or a copyleft or non-commercial one. When a test needs one, it is
  fetched at a pinned commit and checked against a recorded SHA-256; it is not committed.
Owning a cartridge does not change this list. A player's own dump stays on the player's machine.
```
Adapt the "Never" list to Intellivision material: EXEC and GROM images, game ROMs, manuals, scans and rendered page images (derivative of a rights-bearing scan, RESEARCH Pitfall 11), bulk hash databases from other projects. Keep "single facts allowed, bulk tables not" (matches D-15). Add the manifest schema prose here (RESEARCH Open Question 4) and the D-13 effective-redistributable rule.

**Do not copy** "If something gets in" lines 51-55: it instructs force-push and history rewrite after a push. `.planning/AUTONOMY.md` §1 makes rewriting published history and force-pushing `main` a hard stop. Replace with: remove forward with a normal commit, record it, escalate to the owner.

**Honest-limit wording to reuse from `../gabbaboy/THIRD_PARTY_NOTICES.md` lines 16-19:** "Automated package checks compare recorded byte identities; they do not decide whether a grant's author had authority." This matches the AGENTS.md rule that a hash proves identity, not rights. If a notices file is added, use the table shape of lines 23-25 (Material | Rights and distribution | Source/manifest SHA-256 | ROM SHA-256 | Notice SHA-256); in Phase 1 it lists "none committed".

---

### `docs/time-base.md` (FND-07 glossary)

**Analog:** `../nesturbator/GLOSSARY.md` (term, definition, `_Avoid_:` line) for form; `../gabbaboy/cmake/VerifyDocContract.cmake` for the term-inventory check.
```markdown
**Region profile**:
A console timing and output model for a game. ...
_Avoid_: TV region, frame-rate setting
```
Use the same entry form for D-11 terms: crystal, master tick, CPU cycle, CPU input clock, TS1-TS4, STIC clock, PSG chip clock, PSG step, frame (event). The "Avoid" line is where the banned bare word "clock" and "cycle = tick" confusions go.

**Term-inventory check:** `VerifyDocContract.cmake` fails with `doc-contract-missing: <phrase>` when a required phrase is absent, collapsing whitespace first (lines 95-101, 163-177) and failing `doc-contract-empty-phrase` on a blank phrase (166-169). Drive it from `data/expected/glossary-terms.ids` instead of `-D` lists (one term per line). Add a `grep -n -i -w 'clock'` tripwire for the bare word, with `[nc]` waivers on the glossary lines that define it.

**Time-unit reference (Phase 2, not Phase 1):** `../libfuturo/include/libfuturo/libfuturo.h` lines 34-42, `lf_time_unit` with one distinct unit per host (`LF_TIME_NES_HALF_MASTER_TICK = 3`). CONTEXT says add a separate Intellivision master-tick value later and do not reuse the NES one; record that note in `docs/time-base.md`, change nothing in the sibling.

---

### `.gitignore`

**Analog:** `../gabbaboy/.gitignore` lines 1-26.
```gitignore
# Credentials and machine-local settings
.env
.env.*
!.env.example
# Private inputs and saves; permissioned public fixtures must be explicitly admitted
/roms/
/private/
*.gb
!/fixtures/mooneye/daa.gb
```
Pattern: ignore the whole class, admit each exception by explicit `!` line. Here: `/refs/`, `*.pdf`, `*.png`, `*.jpg`, `*.jpeg`, `*.gif`, `*.int`, `*.bin`, `*.rom`, `*.itv`, `*.cfg`, `*.cc3`, `.env*`, local build dirs (`/build/`, `/build-*/`). Phase 1 admits nothing.

---

### `.github/workflows/gate.yml`

**Analog:** `../nesturbator/.github/workflows/ci.yml` lines 33-35 (concurrency), 42-56 (a small single-purpose job). It already uses the SHA-pinned `actions/checkout` v7.0.1 that RESEARCH verified.
```yaml
permissions:
  contents: read
concurrency:
  group: ci-${{ github.workflow }}-${{ github.event.pull_request.number || github.sha }}
  cancel-in-progress: ${{ github.event_name == 'pull_request' }}
...
    runs-on: ubuntu-24.04
    # 5-minute floor for a network-bound job ...; twice the slowest of 34 sampled successful runs ...
    timeout-minutes: 5
    steps:
      - uses: actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1 # v7.0.1
        with:
          persist-credentials: false
      - run: cmake --workflow --preset hygiene
```
Adapt: single job, no matrix, final step `run: sh tools/gate.sh` (a gate step checks this exact line, "ci-parity"). Pin `ubuntu-24.04` (nesturbator and gabbaboy both pin a version rather than `ubuntu-latest`; RESEARCH recommends pinning during the label migration). The cancel-in-progress expression above (pull requests only) matches RESEARCH's "never cancel on main" intent. Add the `cmake --version | head -1` step from RESEARCH's draft. Gabbaboy's own ci.yml is not the model (multi-job, v4.2.2, python classifier).

---

### `.planning/CI-EFFICIENCY.md`

**Analog:** the timeout-ledger comment at `../gabbaboy/.github/workflows/ci.yml` line 73 ("p50 11.07 min in main run <id> ... lets the full measured suite finish") and the sampled-run justification in `../nesturbator/.github/workflows/ci.yml` line 49. Record run id, queue time (job `startedAt` minus run `createdAt`), setup, gate seconds, and the decision (keep single job; macOS leg only when the C build arrives; revisit `ubuntu-24.04` pin after 2026-11-19). Measurement query is in `01-RESEARCH.md` "CI workflow".

---

### `README.md` and `LICENSE`

**README analog:** `../nesturbator/README.md` lines 2440-2448:
```markdown
MIT. See [LICENSE](LICENSE).

nesturbator is not affiliated with or endorsed by Nintendo. The names "Nintendo
Entertainment System", "NES" and "Famicom" are used here only to say what the
core emulates.
```
Adapt: non-affiliation with Mattel, Intellivision Entertainment and any rights holder; the name "Intellivision" used only to say what the emulator targets and never in a package or product name (D-04). Evidence-bound wording: every sentence with a tripwire word needs `[claim:CL-NNNN]` or `[nc]`.

**LICENSE analog:** `../gabbaboy/LICENSE`: MIT text with `Copyright (c) 2026 <name> contributors`; use `iamverysmartvision contributors`.

---

### `.planning/PROJECT.md` (modify)

**Analog:** existing Decision Register block, lines 157-166 ("Delegated decisions" table, columns Gate | Decision | Would reverse it). D1, D2, D3 and D9 rows already exist (RESEARCH Pitfall 9.5). The task is verify and cross-link, not add duplicates; D2's row should point to `docs/time-base.md`. Keep the same three-column format if D-05 notes (D5-D8, D12 not decided; D10, D11 hard stops) need rows.

---

### History scrub, `.claude/CLAUDE.md` and `.planning/research/*.md` (modify)

No sibling analog. Use `01-RESEARCH.md` "History scrub (rehearsed in a throwaway clone)" verbatim: `git filter-branch --tree-filter` with portable `sed -E -e` through a temp file (never `sed -i`), then `rm -rf .git/refs/original`, reflog expire, `gc --prune=now`. Verify with `sh tools/hygiene.sh --history` before `gh repo create`. Replace paths with relative sibling names (`../gabbaboy`), never absolute ones.

## Shared Patterns

### Failure token on a STATUS line, one final error
**Source:** `../gabbaboy/cmake/VerifyEvidenceLedger.cmake` lines 52-64.
**Apply to:** every `tools/check/*.cmake`.
```cmake
message(STATUS "evidence-ledger-unregistered: ${name}")
set(failed TRUE)
...
message(FATAL_ERROR "evidence-ledger check failed")
message(STATUS "evidence-ledger pass names=${checked}")
```
Token text must be short and unwrapped. Control `EXPECT` files match these tokens.

### Required-input guard plus header contract
**Source:** `../gabbaboy/cmake/VerifyEvidenceLedger.cmake` lines 1-20.
**Apply to:** every `-P` script. Document Inputs, Failure tokens, Success line in the header; `cmake_minimum_required(VERSION 3.25)`.

### Vacuous-pass guards (all of them, by name)
**Sources:** `VerifyFixtureManifest.cmake` 10-13 (`count EQUAL 0`), `ExpectedTests.cmake` 2-5 (empty inventory), `ScanForbiddenFixtures.cmake` 102-104 (`scanned_count EQUAL 0`), `VerifyEvidenceLedger.cmake` 58-60 (`checked EQUAL 0`), `VerifyDocContract.cmake` 151-153 and 166-169 (empty doc, empty phrase).
**Apply to:** every checker. Add the new ones from RESEARCH: size-vs-read-length CRLF detection (`file(SIZE)` vs `string(LENGTH)`, Pitfall 7), `[^\t\n -~]` non-ASCII scan, blank `EXPECT` line, `steps-run == expected`, `refs-verified=0/N` visible when the cache is absent.

### Exact inventories and negative controls together
**Source:** `../gabbaboy/cmake/ExpectedTests.cmake` + `../gabbaboy/tests/expected-tests.txt`; selftests at `../gabbaboy/tests/CMakeLists.txt` 2523-2536.
**Apply to:** ids, controls list, gate-steps list, glossary terms. Every check that can fail has a seeded-bad-input control that asserts exit code, an expected token, and (new) a forbidden token.

### Findings name the rule and the file, never the matched text
**Source:** `../nesturbator/scripts/hygiene.sh` lines 10-11 and 30.
**Apply to:** `hygiene.sh`, denylist and secret scans. Output is safe to paste into issues and CI logs.

### No word-boundary regex in CMake
**Source:** `../gabbaboy/cmake/CheckCoreNoHostClock.cmake` lines 46-49 (left-context class) and RESEARCH Pitfall 5.
**Apply to:** any CMake-side regex. Text scans use `grep -E -w` in sh instead.

### Sibling-proven digest idiom
**Source:** `../gabbaboy/cmake/VerifyFixtureManifest.cmake` lines 19-28.
**Apply to:** `outputs[]`, `recipe.sources[]`, `notices[]`, `data/sources.json` verification. `string(TOLOWER digest)` first; `file(SIZE)` and `file(SHA256)`; compare both.

### No absolute home paths, no emulator expression
**Source:** `.planning/AUTONOMY.md` §6 (hygiene) and CONTEXT D-14.
**Apply to:** every tracked file and every executor note. Cite repo-relative or `../sibling` paths only.

## No Analog Found

Files where the planner should rely on `01-RESEARCH.md` patterns (all tested there):

| File | Role | Data Flow | Reason |
|------|------|-----------|--------|
| `tools/check/schema/*.txt` (4-verb DSL) | config | transform | Siblings use one bespoke script per record kind; the generic schema walker is new design (RESEARCH Pattern 4, Alternatives table) |
| `tools/check/coverage.cmake` (logic) | checker | transform | No sibling decodes an opcode space; use RESEARCH Pattern 7 prototype (59 rows, 1024 patterns, 0.5 s) |
| `data/cp1610/second-reader.json` | model | static data | New artifact for the independent re-read (D-20, RESEARCH "Second-reader pass") |
| `docs/cp1610/reference-notes.md` | doc | n/a | No page-cited hardware notes exist in siblings; follow D-17 to D-19 and the RESEARCH extraction workflow (page-image reading, `pdf <n>` cites, `[OBS]` only after reading the image) |
| `.github/pull_request_template.md` | doc | n/a | Carries the D-14 review checklist; wording is in D-14 |
| history scrub procedure | procedure | batch | In-repo research only (`01-RESEARCH.md`), rehearsed in a throwaway clone |

## Deviations from the siblings (do not copy these)

1. Token carried by `message(FATAL_ERROR ...)` or `SEND_ERROR` (gabbaboy `VerifyFixtureManifest.cmake`, `VerifyDocContract.cmake`): wrapped text, stops at first failure. Use STATUS tokens.
2. Markdown-table parsing for the ledger (`VerifyEvidenceLedger.cmake` 41-47): forbidden by D-06. Use JSON.
3. `ASSET_POLICY.md` "If something gets in" force-push guidance: conflicts with the AUTONOMY hard stop on rewriting published history.
4. `hygiene.sh` extras (`--staged`, `check_stop_policy`, `check_vector_json_budget`, iNES/FDS magic bytes): drop. Its `in_manifest` awk lookup over a text manifest becomes a JSON-manifest or FILELIST lookup. Its looser home-path regex becomes the RESEARCH version (alphanumeric first char).
5. Gabbaboy manifests have free-text `license` and no `committed`/`redistributable`/`rights_review`; the D-13 schema is stricter and fail-closed.
6. Gabbaboy `ci.yml` pins `actions/checkout` v4.2.2 on `ubuntu-22.04` with a multi-job layout and python classifier; use nesturbator's v7.0.1 SHA and the single-job shape.
7. `.ids` files carry `#` comments that `file(STRINGS)` does not strip; gabbaboy's `expected-tests.txt` has none.

## Metadata

**Analog search scope:** `../gabbaboy/{cmake,tests,fixtures,docs,.github,.gitignore,LICENSE,THIRD_PARTY_NOTICES.md,.planning/context/LESSONS.md}`, `../nesturbator/{scripts,PROVENANCE.md,ASSET_POLICY.md,GLOSSARY.md,README.md,.github/workflows}`, `../libfuturo/include/libfuturo/libfuturo.h` (time-unit note), `../schwaystation/CMakeLists.txt` (warning list; Phase 2 only), in-repo `.planning/research/SUMMARY.md` and `.planning/PROJECT.md`.
**Files scanned:** about 30 read in whole or in part; 4,012 lines of gabbaboy `cmake/*.cmake` sized, 6 read in detail.
**Tracked-source gate:** every `../gabbaboy/...` and `../nesturbator/...` analog named above was confirmed with `git ls-files` inside its own repo.
**Pattern extraction date:** 2026-11-09
