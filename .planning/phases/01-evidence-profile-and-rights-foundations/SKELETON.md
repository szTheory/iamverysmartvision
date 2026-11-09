# Walking Skeleton — iamverysmartvision

**Phase:** 1
**Generated:** 2026-11-09

This project has no database, UI or server in Phase 1. The skeleton is the thinnest
end-to-end **evidence loop**: one command runs one generic checker over one record and
proves, with a negative control, that the checker can fail; CI runs exactly that command.
Every later plan in this phase, and every later phase, extends this loop without changing
its contracts. Treat this file as a contract, not a scratchpad. It contains no absolute
paths by design (the repository goes public at the end of this phase).

## Capability Proven End-to-End

A contributor (or CI) runs `sh tools/gate.sh` and gets one `gate: PASS|FAIL` line that is
backed by machine-checked records (first: the `intv-2609-ntsc-family-p1` profile) and by
negative controls that prove each check fails for its intended reason.

## Architectural Decisions

| Decision | Choice | Rationale |
|---|---|---|
| Record format | JSON under `data/` and `fixtures/*/manifest.json`; prose in Markdown under `docs/` | D-06: `string(JSON)` in `cmake -P` is a zero-new-dependency parser; Markdown tables are never parsed |
| Checker runtime | POSIX `sh` orchestration + `cmake -P` checkers (floor 3.25) + `grep -E -w` for text scans | D-06/D-07; CMake is the D-01 build tool anyway; CMake regex lacks word boundaries, so text scans use grep |
| Single command | `sh tools/gate.sh` (local) == CI step | D-12/D-23, AGENTS.md "one local command aligned with the fast PR gate" |
| Check unit | Drop-in step scripts `tools/check/steps/<name>.sh`, one owning plan each | Parallel plans never edit a shared step list |
| Generic record checker | `tools/check/records.cmake` driven by `tools/check/schema/<kind>.txt` (small directive DSL) + optional `tools/check/kinds/<kind>.cmake` hook | Avoids one bespoke checker per kind (lesson GB-CI-004) |
| Inventories | Exact sets in `data/expected/*.ids`; step and control inventories are per-plan fragments in `data/expected/gate-steps/*.ids` and `data/expected/controls/*.ids` | D-08 "ID sets must equal data/expected"; fragments keep parallel plans file-disjoint |
| Negative controls | `tools/check/controls/<name>/` overlays a known-good tree `tools/check/controls/_good/tree/`; bad bytes are generated at run time by `MAKE` verbs | A control cannot pass for the wrong reason; no bad bytes are ever tracked |
| Encoding | Records, tools and workflows: ASCII-only, LF-only, no BOM. Docs/README: UTF-8, LF, no BOM. All scripts run with `LC_ALL=C` | Removes encoding ambiguity in ids and keys; docs need `§` for D-18 citations |
| CI | One GitHub Actions job on `ubuntu-24.04`, `actions/checkout` pinned by SHA `3d3c42e5aac5ba805825da76410c181273ba90b1` (v7.0.1), `permissions: contents: read`, concurrency cancel-in-progress on PRs | D-23 with orchestrator resolution 5 (pin, not `ubuntu-latest`) |
| Licence | MIT, `Copyright (c) 2026 iamverysmartvision contributors`; one `SPDX-License-Identifier: MIT` comment line in every `.sh`, `.cmake` and workflow file | D-04 |
| Directory layout | `docs/`, `data/`, `data/expected/`, `fixtures/`, `tools/gate.sh`, `tools/hygiene.sh`, `tools/check/{lib,records,integrity,...}.cmake`, `tools/check/{steps,schema,kinds,controls}/`, gitignored `refs/` | D-07 |

## Stack Touched in Phase 1

- [ ] Scaffold: `tools/gate.sh`, `tools/check/lib.cmake`, `tools/check/records.cmake`, `.gitignore` (plan 01-01)
- [ ] One real record read: `data/profiles/intv-2609-ntsc-family-p1.json` + `data/tiers.json` (plan 01-01)
- [ ] One real negative control through the real checker: `tools/check/controls/profile-blank-value/` (plan 01-01)
- [ ] CI: `.github/workflows/gate.yml` runs exactly `sh tools/gate.sh` (plan 01-01; first hosted run in plan 01-12)
- [ ] Publication: public repo after `tools/hygiene.sh --history` passes (plan 01-12)

## Contracts every plan follows

### §1 Gate contract (`tools/gate.sh`, owned by plan 01-01)

- Invoked as `sh tools/gate.sh` (also `dash tools/gate.sh`). Takes no arguments. Exit 0 only when every step passed, the step set equals its inventory, and every control passed.
- Resolves `ROOT` (repo root, from the script location), `CHECK_DIR` (= `ROOT/tools/check`), and writes `FILELIST` = `git ls-files --cached --others --exclude-standard`, filtered to paths that exist, sorted with `LC_ALL=C sort -u`. Exports `ROOT`, `CHECK_DIR`, `FILELIST`, `GATE_TMP`.
- Step set = basenames (without `.sh`) of `tools/check/steps/*.sh`; expected set = union of non-comment, non-blank lines of `data/expected/gate-steps/*.ids`. Mismatch prints `-- FAIL gate-step-missing: <name>` / `-- FAIL gate-step-extra: <name>`; an empty expected union prints `-- FAIL gate-steps-empty`.
- Runs every step in C-locale sorted order with `sh <step>`; never stops at the first failure; echoes each step's output; a nonzero step exit prints `-- FAIL gate-step-failed: <name>`.
- Then runs `sh tools/check/run-controls.sh` and reads its `controls=<ok>/<total>` line.
- Collects every `k=v` token from lines matching `-- <name> pass ...` and prints one final line: `gate: PASS steps=<run>/<expected> controls=<ok>/<total> <k=v ...>` or `gate: FAIL ...` with the same fields. This is the D-12 visible summary (`claims=`, `pending-resolvers=`, `unknown-fields=`, `controls=`).

### §2 Step contract (`tools/check/steps/<name>.sh`)

- POSIX sh, first lines `#!/bin/sh` and `# SPDX-License-Identifier: MIT`, `LC_ALL=C; export LC_ALL`.
- Reads only `ROOT`, `CHECK_DIR`, `FILELIST` (never `git`, so controls can run it on a temp tree; `tools/hygiene.sh --history` is the only git-walking code and is not a step).
- Calls checkers as `cmake -DROOT="$ROOT" -DFILELIST="$FILELIST" -DKIND=<kind> -P "$CHECK_DIR/records.cmake"` (or another `$CHECK_DIR/*.cmake`). Checkers always come from `CHECK_DIR`, never from `ROOT`, so a control cannot replace the checker or its schema.
- Failure lines: `-- FAIL <token>: <detail>` (sh steps print the same format with `printf`). Detail names an id, a field (`<id>/<key>`) or a file path, never the matched secret or path text.
- Success line: `-- <name> pass <k=v ...>`. Exit nonzero on any failure.

### §3 Control contract (`tools/check/controls/<name>/`, runner owned by plan 01-01)

- Name form `<step>-<case>`; `_good` is reserved for the known-good tree `_good/tree/`.
- Files: `CHECK` (one line: the step name), `EXPECT` (one token per line; every line must appear as a fixed substring of the step output), optional `FORBID` (no line may appear), optional `overlay/` (copied over a fresh copy of `_good/tree/`), optional `MAKE` (transform lines applied after the overlay).
- `MAKE` verbs, one per line, paths relative to the temp tree: `delete <p>`, `empty <p>`, `write <p> <word>`, `rename <from> <to>`, `crlf <p>`, `bom <p>`, `nonascii <p>`, `homepath <p>`, `secret <p>`, `password <p>`, `email <p>`, `nul <p>`, `binary <p>`, `highbytes <p>`. `homepath`, `secret`, `password` and `email` append a line whose bad text is assembled at run time from split pieces, so no tracked file ever contains a home path, a token shape, a password assignment or a foreign email; `nul` appends a line containing a NUL byte (a text file with a NUL is a scanner blind spot, CR-01); `binary` and `highbytes` write about 16 KiB of deterministic non-text bytes (with and without NUL bytes) for the rights-gate content controls (WR-02). An unknown verb fails the control (`control-make-verb`).
- The runner builds the temp tree's `FILELIST` with `find` (not git), runs `ROOT=<tmp> FILELIST=<tmp list> CHECK_DIR=<real> sh tools/check/steps/<CHECK>.sh`, and requires nonzero exit AND every `EXPECT` line AND no `FORBID` line.
- Positive control: every distinct `CHECK` step named by any control is run on the pristine `_good/tree` and must exit 0 and print `-- <step> pass`. Therefore every `_good/tree` fragment must pass every step (doc rules §7 included).
- Runner tokens: `control-exit-zero`, `control-token-missing`, `control-forbidden-token`, `control-expect-blank` (an `EXPECT` with a blank line would match everything), `control-not-in-inventory`, `control-inventory-orphan`, `control-good-failed: <step>`, `control-unknown-step`, `control-make-verb`. The control-directory set must equal the union of `data/expected/controls/*.ids`.
- Tracked controls pass the real tree: every tracked file under `tools/check/controls/**` must pass every real-tree step. Inputs that the integrity or hygiene step would reject (an `.sh`/`.cmake` without the SPDX line, CRLF, BOM, non-ASCII, empty file, bad path charset, home path, token, password, email) exist only in the temp tree via `MAKE` verbs. Text-content inputs (claim words, the bare unit word, denylisted names, unknown `SRC-` refs, flipped manifests) may be overlays only because every real-tree text scan selects repository-root paths by anchored patterns (`^docs/`, `^README\.md$`, `^data/`, `^fixtures/`) or excludes `tools/check/`.
- Final runner line: `controls=<ok>/<total>`, where `<total>` counts negative controls only; a failing good-tree run is reported as `control-good-failed: <step>` and makes the runner exit nonzero regardless of the count.

### §4 Records DSL (`tools/check/schema/<kind>.txt`, interpreter owned by plan 01-01)

One directive per line, `#` comments allowed. Key paths inside a record use `/` for nesting (member names may contain dots, for example the profile field `stic.part`). Tokens are `<kind>-<problem>`.

| Directive | Meaning | Failure token |
|---|---|---|
| `file <relpath>` | Record file under `ROOT` (required, must exist, non-empty, parse) | `<kind>-file-missing`, `<kind>-file-empty`, `<kind>-parse` |
| `topkeys <k...>` | Allowed top-level keys | `<kind>-unknown-topkey` |
| `top <key> <TYPE[\|TYPE]>` | Required top-level key with type | `<kind>-missing-top`, `<kind>-type` |
| `array <key>` / `members <key>` | Records are elements of a top-level array (id from `idkey`) or members of a top-level object (id = member name) | `<kind>-empty` (zero records) |
| `idkey <key>` | Id field for `array` records | `<kind>-duplicate-id` |
| `inventory <relpath>` | Record ids must equal this `.ids` set | `<kind>-id-missing: X`, `<kind>-id-extra: X`, `<kind>-inventory-empty` |
| `keys <k...>` | Allowed record keys (catches misspellings) | `<kind>-unknown-key` |
| `req <key> <TYPE[\|TYPE]>` | Present with type (STRING NUMBER BOOLEAN NULL OBJECT ARRAY) | `<kind>-missing`, `<kind>-type` |
| `nonblank <key>` | STRING non-empty after trim; OBJECT/ARRAY length > 0 | `<kind>-blank` |
| `match <key> <cmake-regex>` | STRING value matches (skipped for NULL) | `<kind>-format` |
| `enumfrom <key> <relpath> <arraykey> <idkey>` | Value is an id in another JSON file's array (skipped for NULL) | `<kind>-unknown-ref` |
| `when <key>=<value> <directive...>` | Apply a directive only when a STRING key equals a value | as the inner directive |
| `hook <kind>` | Include `tools/check/kinds/<kind>.cmake`; it defines `hook_record(id rec)` and `hook_finish()` | hook-defined |

`keys` only rejects unknown keys; `match`, `enumfrom` and `nonblank` do not assert presence. Every required key of a kind therefore has its own `req` line (presence plus type; nullable keys use `TYPE|NULL`), and every record kind ships one negative control that removes a required key and expects `<kind>-missing: <id>/<key>`.

Hooks may call `gate_fail(token detail)` and `gate_stat(key value)` from `tools/check/lib.cmake`. Single-file kinds only; multi-file records (fixture manifests) use a dedicated checker that includes `lib.cmake`.

### §5 Inventory files (`*.ids`)

- One id per line, LF, ASCII. Lines starting with `#` are comments. Blank lines are ignored.
- An inventory with zero ids fails (`<kind>-inventory-empty`) unless it carries a line `#allow-empty: <reason>`.
- Sets are compared order-independently (sorted bytewise, C locale); output lists ids in sorted order so diffs are stable.
- Removing a record requires a visible edit of its inventory (D-08). Discrepancy ids are never reused; retired entries get `status: withdrawn`.

### §6 Encoding and path rules

- Every tracked path matches `^[A-Za-z0-9._/-]+$`.
- No tracked file that is not binary by extension (`tools/check/binary-ext.txt`) may contain a NUL byte (`integrity-nul-byte`); every content scan reads files with `grep -a` so nothing is skipped as "binary".
- `data/**`, `fixtures/**`, `tools/**`, `.github/**`: ASCII only (no byte outside tab, LF, 0x20-0x7E), LF only, no BOM, no empty files.
- `docs/**`, `README.md`, `.planning/**`: UTF-8 allowed, LF only, no BOM.
- Hex values in records are written `$1000` style strings; rationals as `{"num": N, "den": D}`.
- Records under `data/` and `fixtures/` write manual sections as `S3.4.6` (never the section sign) and use ASCII substitutes for symbols (`!=`, `x`, `-`, `->`); the section sign, not-equal sign and arrows appear only in `docs/`.
- CMake id regexes must not use `{n}`, `\b`, `\w`, `\s`, `\d` (they silently never match); write `[0-9][0-9][0-9][0-9]` instead of `[0-9]{4}`.

### §7 Doc and text rules (apply to every tracked file a plan writes)

- One sentence per line in `docs/**` and `README.md`.
- **R1 claim-word tripwire** (`README.md`, `docs/**/*.md`): this must print nothing:
  `grep -n -i -w -E 'supports|compatible|passes|accurate|verified|runs' <files> | grep -v -E '\[claim:CL-[0-9]{4}\]|\[nc\]'`.
  Prefer rephrasing; otherwise put `[claim:CL-NNNN]` (an existing claim id) or the `[nc]` waiver on the same line.
- **R2 bare-unit tripwire** (`README.md`, `docs/**/*.md`): this must print nothing:
  `grep -n -i -w -E 'clocks?' <files> | grep -v -i -E 'input clock|STIC clock|PSG chip clock|\[nc\]'`.
  Write "crystal", "master tick", "CPU cycle", "CPU input clock", "STIC clock" or "PSG chip clock" instead of the bare word.
- **R3 provenance denylist** (every tracked file outside `.planning/`, `.claude/`, `docs/provenance.md`, `docs/provenance/`, `data/sources.json`, `tools/check/`): author names and licence-header strings in `tools/check/denylist.txt` never appear; emulator project names in `tools/check/denylist-cite.txt` appear only on a line that also carries `[cite-only]`. Cite sources by their `SRC-<slug>` registry id instead of by name wherever possible.
- **R4 no private paths**: no absolute home or scratch path in any tracked file, including SUMMARY.md files. Cite repo-relative paths or `../<sibling>` only. Reference PDFs, page renders and downloads live only under gitignored `refs/`.
- Claims are cited as `[claim:CL-NNNN]`; sources as `SRC-<slug>`; manual pages as `§<section>, pdf <n>` (D-18) in docs and `pdf <n>` (with `S<section>` when a section is needed) in records.
- **R5 source references**: every token matching `SRC-[a-z0-9-]+` in `data/**/*.json` (other than `data/sources.json`), `docs/**/*.md` and `README.md` must be an id in `data/sources.json` (enforced by the `sources` step from plan 01-07). Write placeholders as `SRC-<slug>` (the `<` stops the match), never as `SRC-id`. Good-tree fragments under `tools/check/controls/_good/tree/` may cite only `SRC-gi-cp1600-um-1975`, `SRC-gi-1980-catalog` and `SRC-bsr-2609` (the ids registered in the good-tree registry).

### §8 Shell and CMake footguns (from 01-RESEARCH.md, all reproduced)

- Acceptance greps in plans assume POSIX/BSD/GNU `grep`. On the dev machine the interactive shell's `grep` is a ugrep wrapper function that can silently disagree (for example on non-ASCII byte classes); run acceptance commands as `sh -c '...'` or with `/usr/bin/grep`.
- sh: no `[[`, `local`, arrays, `set -o pipefail`, `echo -e`, `sed -i`, `grep -P`; quote every expansion; temp dirs via `mktemp -d "${TMPDIR:-/tmp}/<name>.XXXXXX"` with `trap` cleanup; digests via `cmake -E sha256sum`. Test under both `sh` and `dash`.
- CMake: `cmake_minimum_required(VERSION 3.25)` at the top of every `-P` script; tokens on `message(STATUS ...)` lines, one final `message(FATAL_ERROR ...)`; guard `n EQUAL 0` before `foreach(i RANGE 0 ${last})`; extract each record once (`string(JSON rec GET ...)`) then read fields from `rec`; call `string(JSON ... TYPE)` before `GET` (true/false come back as ON/OFF, null and "" both empty); `;` is not a statement separator; never build variable names from ids containing `@`, `.` or `+`; detect CRLF by `file(SIZE)` vs `string(LENGTH)`; duplicate JSON keys are not detectable (state this limit, never claim it).

## Out of Scope (Deferred to Later Slices)

- CPU, STIC, PSG, loader code; the CMake build/test skeleton for the core (Phase 2+).
- Fixture bytes, assembler policy (D4), boot stub (D5), controller representation (D6), PCM location (D7), framebuffer geometry (D8), CI shape beyond one job (D12).
- Generated C header from the profile JSON (Phase 2), unit-swap named-fault control (Phase 2), fill-sensitivity matrix (Phase 4+).
- PAL, STIC1A and alias profiles (v2); board-pinned owner-unit profile (needs D10).
- A second profile record. The profile kind is single-file today; the first additional profile must promote the profile kind to per-file records keyed by profile id (assumption-delta note in plan 01-01).
- Any software release (D-24).

## Subsequent Slice Plan

Each later phase adds steps, schema kinds, inventories and controls to this loop and appends build/CTest steps to the same `tools/gate.sh`; none changes §1-§8.

- Phase 2: CP-1610 core builds and its exact CTest inventory runs as gate steps; vectors cite the notes and manual pages, never the family table (D-20).
- Phase 3: memory map generated from the profile record; refusal matrix tests.
- Phase 4: Slice 0 runner and installed consumer as gate steps; CI shape decided (D12).
- Phase 5-8: controllers/PSG, STIC, visible scenario, save state and performance, each adding claims to `data/claims.json` with their evidence tiers.

## Notes for the orchestrator

- The ROADMAP Phase 1 Goal line is outcome-shaped, not a user story. Under MVP rules no story is invented here; run `/gsd-mvp-phase 1` if a story framing is wanted.
- Assumption-delta checkpoint: signals "also" and "chosen" in the phase prose are not an identity-model transition (see plan 01-01 `<assumption_delta_decision>`); disposition `no-change`.
