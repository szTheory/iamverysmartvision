# Phase 1: Evidence, Profile and Rights Foundations - Research

**Researched:** 2026-11-09
**Domain:** Docs-and-data evidence foundation, enforced by a zero-dependency POSIX sh + `cmake -P` gate; CP-1610 reference-note extraction from a scanned manual; first public publication of the repo
**Confidence:** HIGH for tooling mechanics (executed on CMake 3.25.3 and 4.4.3), HIGH for publication mechanics (rehearsed in a throwaway clone), MEDIUM for CP-1610 extraction effort (estimate), MEDIUM for plan decomposition (judgment)

**Label key.** The brief asks for `[OBS]/[INF]/[REC]/[UNK]`. They map onto the agent provenance tags as follows. `[OBS]` = executed or read this session (equals `[VERIFIED: …]`, with the evidence named). `[CITED]` = official doc or API. `[INF]` = inferred. `[REC]` = recommendation. `[UNK]` = unresolved. `[ASSUMED]` = training knowledge, see the Assumptions Log. No GPL emulator source was opened. No MAME source was opened.

<user_constraints>
## User Constraints (from CONTEXT.md)

`01-CONTEXT.md` `<decisions>` is the verbatim authority. The planner MUST read it in full. This section is a faithful digest, not a substitute.

### Locked Decisions (D-01..D-24)
- **D-01** C17 + CMake (floor 3.25, Presets v6) + Ninja + CTest. Phase 1 only records it. In-repo ~150-line harness, exact `tests/expected-tests.txt`. Rust is the named alternative.
- **D-02** First profile `intv-2609-ntsc-family-p1`, family-level, no PCB revision pinned. Aliases recorded as `alias-candidate, unverified`.
- **D-03** Time unit = master tick (one crystal period), `uint64_t` from reset. CPU cycle = profile ratio (NTSC 4). `run(target_tick)` returns `now_tick`, `overshoot_ticks`, stop reason. One-way once the Phase 2 API ships.
- **D-04** MIT, `Copyright (c) 2026 iamverysmartvision contributors`, one `SPDX-License-Identifier: MIT` line per new source file. Public repo `szTheory/iamverysmartvision` after the hygiene check. Keep the name. README non-affiliation notice.
- **D-05** D5, D6, D7, D8, D12 are not decided here. D10 and D11 stay hard stops (evidence marked missing).
- **D-06** Records are JSON. Prose stays in Markdown. The checker never parses Markdown tables. A `note` string field replaces comments.
- **D-07** Layout: `docs/`, `data/` (profile, `tiers.json`, `claims.json`, `discrepancies.json`, `sources.json`, `cp1610/opcode-families.json`), `data/expected/*.ids`, `fixtures/`, `tools/gate.sh`, `tools/check/*.cmake`, `tools/check/controls/<name>/` with `EXPECT`, gitignored `refs/`. Cap checker size (GB-CI-004). Explicit `cmake_policy` baseline in each `-P` script.
- **D-08** IDs: `C<n>` never reused (`status: withdrawn`), `CL-NNNN`, profile slug + dotted field paths, `FX-<slug>`, `SRC-<slug>`. Docs cite `[claim:CL-0007]`. ID sets must equal `data/expected/*.ids`.
- **D-09** Two vocabularies, never conflated: (a) source-authority tags, (b) the eight evidence tiers. A claim cannot exceed its weakest field. A tier-5-only value must be `unknown-default-chosen`.
- **D-10** Profile record: per field `value, tag, cite, conflict, resolving_evidence, revisit_trigger, decision`. Blank fails. Per-field defaults are listed in CONTEXT.
- **D-11** Time-base glossary terms. The bare word "clock" is banned. Profile keys carry unit suffixes.
- **D-12** Gate checks 1-11 and the negative-control list. `sh tools/gate.sh`. Visible summary line. The control list is an exact inventory.
- **D-13** Fixture manifest `schema_version: 1`. Effective redistributable is yes only if `redistributable: yes` AND `licence_evidence` present AND `rights_review.status` is not `unreviewed`. Anything else is no.
- **D-14** No-copy rule wording. GPL emulator source is never opened. MAME (BSD-3) readings are logged in `docs/provenance/CONSULTATIONS.md`. The process is called "documented independent derivation".
- **D-15** `refs/<SRC-id>.<ext>` gitignored. `data/sources.json` tracked.
- **D-16..D-21** CP-1610 notes from the GI May 1975 manual (sha256 `ecdabe5c…dc2b72`, 116 pages). Page images are read, OCR is not trusted. Citation form "§3.4.6, pdf 62". About 90 family rows. Phase 1 ends with a second-reader pass by a separate agent. C17 confirmed. C19 partial. New discrepancies C29+ appended.
- **D-22** Publication last, after the gate. Scrub home paths. Rewriting unpublished history with built-in `git filter-branch` is preferred. `gh repo create szTheory/iamverysmartvision --public --source . --push`.
- **D-23** One GitHub Actions job running exactly `sh tools/gate.sh`. `concurrency` cancel-in-progress. No matrix. Create `.planning/CI-EFFICIENCY.md` on the first run.
- **D-24** No software release this phase.

### Claude's Discretion
- Exact JSON key spelling within the schemas, how checker scripts are split (keep them small and generic), wording of docs.
- Whether the 1024-pattern coverage check lives in `cmake -P` or is deferred to a Phase 2 C test (speed bound: about 2 s). **Resolved by measurement below: keep it in `cmake -P` (about 0.5 s).**
- Whether to rewrite pre-publication history (preferred) or accept it.

### Deferred Ideas (OUT OF SCOPE)
Fill-sensitivity matrix; board-pinned owner-unit profile (needs D10); STIC1A, PAL and Tandyvision/GTE/Sears aliases; MSan, valgrind and fuzz jobs; LibFuturo master-tick unit (D13); generated C header from the profile (Phase 2).
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| FND-01 | First profile as a data record with per-field tag | Profile checker design, `json_require` idiom, GI 1980 catalog pdf 191 read (STIC part, 3.579545 MHz, geometry) |
| FND-02 | Evidence-tier vocabulary and claim ledger; claims cite profile + tier + artifact | `tiers.json`, claim-resolution check, claim-language tripwire (grep, not CMake), weakest-field rule (REC mechanical form) |
| FND-03 | Discrepancy log seeded with C1-C28, default policy, resolver | `discrepancies.json` shape, set-equality against `data/expected`, C29+ additions, SUMMARY.md §Source Conflicts is the seed |
| FND-04 | Fixture manifest schema, `redistributable` unknown treated as no | Effective-redistributable rule, good-tree sample + flipped-copy control, `git ls-files` vs manifests seam |
| FND-05 | Provenance rule documented and checked in review | Denylist scoping defect found in D-12.7 (see Pitfall 9), checklist, PR template, CONSULTATIONS.md |
| FND-06 | CP-1610 notes with page cites | Extraction workflow, page map, 1024-pattern coverage prototype, second-reader design |
| FND-07 | Time-base vocabulary and D2 | Glossary term inventory check, "clock" tripwire, Decision Register already carries D1/D2/D3/D9 |
</phase_requirements>

## Project Constraints (from CLAUDE.md / AGENTS.md)

- Read `AGENTS.md` first. It takes precedence over generated CLAUDE.md sections. Research text in CLAUDE.md is dated input, not owner decisions. [OBS: .claude/CLAUDE.md, AGENTS.md]
- Edits go through a GSD workflow (no direct repo edits outside it). Executors run under `/gsd-execute-phase`. [OBS: .claude/CLAUDE.md "GSD Workflow Enforcement"]
- Rights-bearing third-party material (manuals, datasheets, EXEC/GROM, ROMs) is never committed. A hash proves identity, not rights. Page renders of the manual are derivative and also stay out of git. [OBS: AGENTS.md; AUTONOMY.md §1]
- Small dependency surface. "Another copy-paste is better than another dependency." [OBS: AUTONOMY.md §2]
- One local command aligned with the PR gate. A CI check needs a distinct recurring failure class. Measure before tuning. [OBS: AGENTS.md]
- Hard stops: PII or private paths in repo or history, force-push to `main`, rewriting published history, "Intellivision" in a package or repo name, paid services. [OBS: AUTONOMY.md §1]
- Hygiene check before the first push (§6): noreply author emails, secret patterns, no `/Users/<name>`, no third-party binaries or PDFs. [OBS: AUTONOMY.md §6]
- Label owner decisions, observations, inference, recommendation and unknowns separately. Code presence is not a test result. [OBS: AGENTS.md]

## Summary

Phase 1 is a docs-and-data phase with one real engineering artifact: a gate (`sh tools/gate.sh`) that machine-checks the records and proves it can fail, using only POSIX sh, `cmake -P` and git. Everything the brief asked about CMake script mechanics was executed on **CMake 3.25.3 (the floor) and 4.4.3 (local)**. Results were identical apart from the text of parser error messages. The gate approach is viable and cheap: a 90-row by 1024-pattern coverage check ran in about 0.5 s, and a 300-record JSON walk ran in 0.35 s once records are extracted one at a time.

The footguns that will hurt a first implementation are all concrete and were reproduced. `string(JSON)` accepts duplicate keys (last wins), trailing commas and comments silently, and re-parses the whole document on every call (quadratic). CMake regex has no `\b`, `\w`, `\s`, `\d`, `{n}` or lookaround, and **`\b` silently matches nothing** instead of erroring. `file(READ)` strips `\r` from CRLF, so CRLF must be detected by comparing `file(SIZE)` to the read length. `message(FATAL_ERROR)` text is word-wrapped, so tokens must be emitted on `-- ` STATUS lines. `foreach(i RANGE -1)` iterates twice. BSD `sed -i` needs an empty-string argument. Each has a tested workaround below.

The CP-1610 work is feasible and well-bounded. I read pdf pages 46, 50-58 and 62 as images. The instruction decle space (1024 patterns) appears to be **fully specified by the manual's tables**: a transcription of pages 53-56 and 57 into pattern rows left 0 patterns unclaimed. The only decode-level ambiguity I found is that TSTR R7, JR R7 and MOVR R7,R7 all encode `0010 111 111` (a real manual-level coincidence, not a transcription error). Overlaps between a general row and its specializations (TSTR/JR inside MOVR, CLRR inside XORR, PSHR/MVOI inside MVO@, and so on) are pervasive, so the "exactly one family" rule must be restated as "exactly one most-specific row, via a declared `refines` chain". Publication mechanics were rehearsed end to end in a clone: `git filter-branch --tree-filter` with a portable sed scrubs all 9 commits in about 3 seconds and leaves zero `/Users/…` hits in any ref.

**Primary recommendation:** Build tracer-first. One plan lands `gate.sh` + one generic record checker + the overlay-based control runner + the profile record with 3 controls. Then fan out records and docs in parallel. Run the CP-1610 extraction as the long pole, started in wave 1. Make publication the final, separate plan: hygiene tree check, history rewrite, history check, clean-clone gate under `dash`, then `gh repo create`.

## Architectural Responsibility Map

This phase has no web tiers. Tiers here are the layers of the evidence system.

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Structured facts (profile, claims, discrepancies, sources, opcode families) | Records (`data/*.json`) | Prose docs cite them by id | D-06: checker parses JSON only, never Markdown tables |
| Rules, rationale, glossary, checklists | Prose (`docs/*.md`) | — | Human-readable; machine-touched only by tripwires (claim ids, forbidden words) |
| Record validation, set-equality, coverage, digests | `cmake -P` checkers | sh wrappers | Zero new dependency; `string(JSON)`, `file(SHA256)`, `math(EXPR)` suffice (executed) |
| Text scans (tripwire, denylist, hygiene, claim-ref extraction) | POSIX sh + `grep -E -w` | — | CMake regex lacks `\b`; `grep -w` works on BSD and GNU (executed with BSD grep 2.6.0) |
| Orchestration, summary line, exit code | `tools/gate.sh` | CI calls it unchanged | D-12/D-23: one command local and CI |
| Negative controls | `tools/check/run-controls.sh` + overlay dirs | — | Runs the real checker on a bad tree, asserts nonzero exit AND token |
| Third-party reference cache | gitignored `refs/` | `data/sources.json` (hashes, rights: unknown) | Rights: nothing third-party tracked |
| Publication | git (`filter-branch`) + `gh` | GitHub Actions | One-way, last step |

## Standard Stack

### Core
| Tool | Version (observed) | Purpose | Why Standard |
|------|--------------------|---------|--------------|
| CMake script mode (`cmake -P`) | 4.4.3 local; 3.25.3 floor executed; 3.31.6 on ubuntu-24.04 image [CITED: runner-images Ubuntu2404-Readme]; 4.3-4.4 on the 26.04 image [CITED: newreleases.io mirror, LOW-MEDIUM] | JSON record checks, `file(SHA256)`, set-equality, coverage loop | D-06/D-01: already the project's build tool; sibling-proven (gabbaboy) |
| POSIX `sh` | macOS `/bin/sh` is bash 3.2 in POSIX mode; ubuntu `/bin/sh` is dash 0.5.12 [CITED: runner-images]; `/bin/dash` exists locally [OBS] | `gate.sh`, controls runner, hygiene | CI runs `sh tools/gate.sh`; run it under `dash` locally to match CI |
| git | 2.41.0 local; 2.56.0 on ubuntu-24.04 image [CITED] | `ls-files`, `grep`, `filter-branch` | Built-in only |
| `grep -E -I -w -o -F -f` | BSD grep 2.6.0-FreeBSD [OBS]; GNU on CI | Tripwires, denylist, hygiene | Avoid `-P`, `\b`, `\d`, `\s`; use POSIX classes |
| `gh` | 2.101.0, authed as szTheory with scopes `gist, read:org, repo, workflow` [OBS] | Repo creation, run timing queries | `workflow` scope is required to push `.github/workflows/` |
| poppler `pdftoppm` | 26.04.0 [OBS] | Page images for CP-1610 reading (local aid, not in the gate) | CONTEXT: not part of the gate |

### Supporting
| Tool | Purpose | When |
|------|---------|------|
| `shellcheck` (installed locally, not required) | Lint `gate.sh` and friends | Optional, local only; do not add to the gate |
| `cmake -E sha256sum` | Portable digest in sh | Avoids `shasum` (macOS) vs `sha256sum` (Linux) |
| Throwaway python3 (`-I`) | Second-reader diff, one-off | Throwaway only; not committed, not in the gate |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Own JSON schema DSL (4 verbs) | `jq` / python | New dependency; rejected by D-06 and owner taste |
| `grep` text scans in sh | `file(STRINGS)` + CMake regex | No `\b`; silent mis-match; slower. Use grep |
| Per-record variables in CMake | Parallel lists + `list(GET)` | Record ids contain `@` (`MVO@`); parallel lists avoid name hazards |

**Installation:** none. `pdftoppm` is already present. Do not add `shellcheck` or `actionlint` to any required path.

## Package Legitimacy Audit

No external packages are installed in this phase (no npm, pip or cargo). The only third-party action is `actions/checkout`, pinned by SHA below. `gsd-tools package-legitimacy` was therefore not run; there is nothing to check.

| Package | Registry | Age | Downloads | Source Repo | Verdict | Disposition |
|---------|----------|-----|-----------|-------------|---------|-------------|
| actions/checkout v7.0.1 | GitHub Actions | released 2026-07-20 | n/a | github.com/actions/checkout | OK (first-party GitHub action) | Approved, SHA-pinned [VERIFIED: gh api repos/actions/checkout/git/ref/tags/v7.0.1] |

**Packages removed due to [SLOP]:** none. **Packages flagged [SUS]:** none.

## Architecture Patterns

### System Architecture Diagram

```
   author/agent edits                          CI (ubuntu, after publication)
        |                                              |
        v                                              v
  data/*.json  docs/*.md  fixtures/*/manifest.json   .github/workflows/gate.yml
        |                                              |
        +---------------------+------------------------+
                              v
                       sh tools/gate.sh      <-- the ONE command (local = CI)
                              |
   +--------------+-----------+-----------+------------------+
   v              v           v           v                  v
 step: records  step: ids   step: text   step: hygiene   step: controls
 (cmake -P      (set-eq vs  (grep -w:    (grep -E, C     (run-controls.sh:
 generic        data/       tripwire,    locale: paths,   good tree must pass;
 checker:       expected/   denylist,    emails, secrets, each overlay tree must
 profile,       *.ids;      [claim:]     binaries not in  exit nonzero AND print
 claims,        control     refs,        manifest)        its EXPECT token)
 discrep.,      inventory)  CRLF/non-
 sources,                   ASCII)
 manifests,
 opcode rows
 + 1024 cover)
   |              |           |           |                  |
   +--------------+-----------+-----------+------------------+
                              v
        each step prints "-- <check> pass k=v ..." or "-- FAIL <token>: <detail>"
                              v
      gate summary: steps-run==expected (vacuous-pass guard), exit 0/1
```

### Recommended Project Structure
```
tools/
├── gate.sh                      # orchestrator, POSIX sh, runs every step, never stops at first failure
├── hygiene.sh                   # --tree (every gate run), --history (publication + release only)
└── check/
    ├── lib.cmake                # gate_fail(), json_require(), vacuous guards (include()d by each -P script)
    ├── records.cmake            # generic walker driven by tools/check/schema/<kind>.txt
    ├── ids.cmake                # actual-vs-expected set equality
    ├── coverage.cmake           # 1024-pattern check
    ├── run-controls.sh
    ├── schema/                  # 4-verb spec files: req / enum / nonblank / allowkeys
    └── controls/
        ├── _good/tree/          # minimal valid tree (positive control)
        └── <name>/{overlay/,EXPECT}
data/expected/*.ids              # '#' comments allowed; empty only with '#allow-empty: <reason>'
```

Drop-in steps (`tools/check/step-NN-*.sh`) avoid merge conflicts when parallel plans each add one check. The step list is itself an inventory (`data/expected/gate-steps.ids`) owned by exactly one plan.

### Pattern 1: Failure accumulation with unwrapped tokens
**What:** Every failure prints `-- FAIL <token>: <detail>` via `message(STATUS)`, records itself in a GLOBAL property, and the script ends with one `FATAL_ERROR`. Success prints `-- <check> pass k=v`.
**Why:** `message(SEND_ERROR|FATAL_ERROR)` text is word-wrapped at about 78 columns, so a long token can be split across lines and defeat `grep -F`. STATUS lines are not wrapped (they go to stdout; errors go to stderr). The control runner merges both with `2>&1`.
**Example (executed on 4.4.3):**
```cmake
cmake_minimum_required(VERSION 3.25)   # explicit baseline; works in -P mode on 3.25 and 4.4
function(gate_fail token detail)       # safe from nested functions (global property, not PARENT_SCOPE)
  message(STATUS "FAIL ${token}: ${detail}")
  set_property(GLOBAL APPEND PROPERTY GATE_FAILURES "${token}")
endfunction()
# ... checks ...
get_property(f GLOBAL PROPERTY GATE_FAILURES)
list(LENGTH f n)
if(n GREATER 0)
  message(FATAL_ERROR "records: ${n} failure(s)")     # nonzero exit (1)
endif()
message(STATUS "records pass fields=${checked}")
```
Keep detail strings short. A `FATAL_ERROR` final line is fine because the control asserts the token on the STATUS line.

### Pattern 2: One-shot presence-plus-type guard
`string(JSON TYPE … ERROR_VARIABLE)` answers "exists and is of kind X" in one call. It also disambiguates what GET hides (booleans come back as `ON`/`OFF`, null and `""` both come back empty). Executed:
```cmake
function(json_require out doc type)
  string(JSON t ERROR_VARIABLE e TYPE "${doc}" ${ARGN})
  if(NOT e STREQUAL "NOTFOUND")
    set(${out} "missing" PARENT_SCOPE)
  elseif(NOT t STREQUAL "${type}")
    set(${out} "type:${t}" PARENT_SCOPE)
  else()
    set(${out} "ok" PARENT_SCOPE)
  endif()
endfunction()
# dotted keys are fine because path elements are separate args:
#   json_require(r "${doc}" STRING fields stic.part value)
```
A blank value is `TYPE=STRING` with `GET` returning `""`. Test blankness explicitly after the type guard.

### Pattern 3: Extract each record once, then read its fields
Calling `string(JSON GET "${whole}" arr ${i} key)` re-parses the whole document every time. Measured on a 178 KB, 300-record file with 11 fields each: **3.6 s** doing nested GETs, **0.35 s** doing `string(JSON rec GET "${whole}" arr ${i})` once per record and then reading fields from `rec`. [OBS] Always do the second form.

### Pattern 4: Allowed-key check (catches misspelled keys)
CMake's parser silently accepts duplicate keys (last wins, member count counts one), trailing commas and comments. So "misspelled key" is detected only by enumerating members and comparing to an allowlist:
```cmake
string(JSON n LENGTH "${rec}")
if(n GREATER 0)
  math(EXPR last "${n}-1")
  foreach(i RANGE 0 ${last})            # NEVER RANGE ${last} when n may be 0: RANGE -1 iterates 0 and -1
    string(JSON k MEMBER "${rec}" ${i})
    if(NOT k IN_LIST allowed)
      gate_fail(manifest-unknown-key "${k}")
    endif()
  endforeach()
endif()
```
Duplicate JSON keys cannot be detected through `string(JSON)`. Do not claim the checker catches them. Duplicate **ids** (array elements) are caught semantically. [OBS]

### Pattern 5: Control runner with overlay trees
A control is a few files that overlay a small known-good tree, plus an `EXPECT` file. The runner copies `_good/tree` to a temp dir, overlays, runs the real checker and requires nonzero exit AND the token. Because `_good` must itself pass, a control cannot pass for the wrong reason, and the whole control set is a few KB instead of 20 copies of a tree. Tested under `sh` (bash 3.2 POSIX mode) and `dash`; a wrong `EXPECT` flips it to failure.
```sh
#!/bin/sh
# tools/check/run-controls.sh
set -eu
here=$(cd "$(dirname "$0")" && pwd); ctl="$here/controls"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/ctl.XXXXXX"); trap 'rm -rf "$tmp"' EXIT HUP INT TERM
n=0; bad=0
run_check() { cmake -DROOT="$1" -P "$here/records.cmake" >"$2" 2>&1; }
rm -rf "$tmp/good"; mkdir "$tmp/good"; cp -R "$ctl/_good/tree/." "$tmp/good/"
if run_check "$tmp/good" "$tmp/good.out" && grep -q -F 'records pass' "$tmp/good.out"; then
  printf 'control ok   _good\n'
else printf 'control FAIL _good\n' >&2; cat "$tmp/good.out" >&2; bad=$((bad+1)); fi
n=$((n+1))
for d in "$ctl"/*/; do
  name=$(basename "$d"); [ "$name" = "_good" ] && continue
  n=$((n+1))
  # GUARD: an EXPECT with an empty line makes grep -F -f match everything (vacuous pass)
  if [ ! -s "$d/EXPECT" ] || grep -q '^$' "$d/EXPECT"; then
    printf 'control FAIL %s: EXPECT missing, empty or has a blank line\n' "$name" >&2; bad=$((bad+1)); continue; fi
  rm -rf "$tmp/t"; mkdir "$tmp/t"; cp -R "$ctl/_good/tree/." "$tmp/t/"
  [ -d "$d/overlay" ] && cp -R "$d/overlay/." "$tmp/t/"
  # deletions: one path per line in $d/DELETE (optional)
  if run_check "$tmp/t" "$tmp/t.out"; then
    printf 'control FAIL %s: exit 0 on a bad tree\n' "$name" >&2; bad=$((bad+1)); continue; fi
  if grep -q -F -f "$d/EXPECT" "$tmp/t.out"; then printf 'control ok   %s\n' "$name"
  else printf 'control FAIL %s: nonzero exit, token missing\n' "$name" >&2; bad=$((bad+1)); fi
done
printf 'controls=%s/%s\n' "$((n-bad))" "$n"
[ "$bad" -eq 0 ]
```
The runner must also assert that the set of control directories equals `data/expected/controls.ids` (D-12 "the control list is itself an exact inventory"). The sample `run_check` above is generic; in the real runner each control names which checker it targets (a `CHECK` file, default `records`).

### Pattern 6: Seams so controls do not need git
The git-dependent checks (`git ls-files` vs manifests, `refs/` tracked) take a `FILELIST` file as input. `gate.sh` produces it with `git ls-files`; a control ships its own `files.txt`. Also assert every tracked path matches `^[A-Za-z0-9._/-]+$`. That removes quoting and newline-in-filename classes in one rule. [REC] The repo's 18 tracked paths all satisfy it today [OBS].

### Pattern 7: Coverage with declared refinements (CP-1610)
Row shape (tested): `{"id","printed","pattern","refines"?,"eq"?}`.
- `printed` is the encoding exactly as printed in the manual (for example `"0010 SSS SSS"`), which makes the second-reader diff a visual compare.
- `pattern` is the 10-character `0/1/.` form. The checker verifies mechanically that `printed` with spaces removed and letters replaced by `.` equals `pattern`, which catches slips between the two forms.
- `eq` (for example `"5:3=2:0"`: bits 5..3 equal bits 2..0) is needed because TSTR (`SSS SSS`) and CLRR (`DDD DDD`) are relational. Letter repetition cannot be used to infer it, because `RR` and `NRR` repeat a letter for field width, not equality.
- `refines` names the general row a specialization sits inside.
Rule: each of the 1024 patterns must match at least one row, and where several match, exactly one must be a leaf (not the `refines` target of another match). Zero matches fails `coverage-unclaimed`. An `unspecified-by-manual` class is allowed only as an explicit row.

Executed result on my own transcription of pages 53-58: **59 rows, 1024 patterns, 0 unclaimed, 150 refined overlaps, 1 genuine ambiguity (pattern 191 = `0010111111`)**, 0.49 s on CMake 4.4.3 and 0.47 s on 3.25.3. Removing one row (RSWD) produced 8 `coverage-unclaimed`; mis-transcribing ADDR `0011` as `0010` produced many `coverage-ambiguous`. The check is a strong cross-check on encoding transcription errors. It does not check cycles, status bits or interruptibility. [OBS: executed in scratchpad]

### Anti-Patterns to Avoid
- **One bespoke checker per record type.** GB-CI-004: gabbaboy's gate grew to about 4,000 lines of cmake [OBS: `wc -l ../gabbaboy/cmake/*.cmake` = 4,012]. Keep one generic walker plus a 4-verb schema file per kind.
- **Parsing Markdown tables.** D-06 forbids it. Use JSON.
- **`message(FATAL_ERROR token)` as the token carrier.** Wrapped text; first-failure-only. Use Pattern 1.
- **Relying on CMake regex for word-boundary scans.** Use `grep -w`.
- **Asserting on CMake's parser error text.** It differs between 3.25 and 4.4 (verified). Emit your own tokens.
- **A history check inside every gate run before the rewrite.** The unpublished history contains 10 occurrences of `/Users/<name>` paths and would fail every gate until the rewrite. Run `--history` at publication and before each release (see Publication).

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| JSON parsing | A sh/awk JSON parser | `string(JSON)` | Present since 3.19; identical results on 3.25.3 and 4.4.3 |
| SHA-256 | A digest implementation | `file(SHA256)` / `cmake -E sha256sum` | Built in, portable |
| Bit tests | Looping over bits as strings | `math(EXPR "${p} & ${m}")` with `>>`, `<<`, `|`, `&` (no comparison operators, no `==`) | Executed; compare with `if(x EQUAL y)` |
| Secret scanning | A full scanner | Four tight regexes (below) | Zero-dependency, specific enough to avoid self-match |
| Repo history rewrite | A custom tool | `git filter-branch --tree-filter` (built in) | Rehearsed in a clone |
| Workflow lint | `actionlint` | A clean-clone `dash tools/gate.sh` plus reading the YAML | No new tool; one tiny workflow |
| PDF reading | OCR post-processing | Page images read by the agent | OCR is damaged in tables (`00]` for `001`) [OBS: pdftotext of pdf 46] |

**Key insight:** the gate's value is in its inventories and negative controls, not in parsing sophistication. Spend lines on vacuous-pass guards, not features.

## Common Pitfalls

### Pitfall 1: Duplicate keys, trailing commas, comments pass silently
**What goes wrong:** `{"a":1,"a":2}`, `{"a":1,}` and `{"a":1 /*x*/}` all parse; `GET a` returns 2, 1, 1. **Why:** CMake's JSON reader is lenient. **Avoid:** allowed-key enumeration (Pattern 4) and semantic id-uniqueness; never promise duplicate-key detection. **Signal:** a control for "misspelled key" passes but "duplicate key" cannot be written. [OBS]

### Pitfall 2: Booleans and null come back as strings
`true` becomes `ON`, `false` becomes `OFF`, `null` and `""` both become an empty string, numbers stay decimal text (a 20-digit integer survives). Always call `TYPE` first. For `conflict: null`, test `TYPE == NULL`. [OBS]

### Pitfall 3: Quadratic parsing
See Pattern 3. A 10x slowdown appears around 150 KB. `claims.json` will not reach that in Phase 1, but write it the fast way now. [OBS]

### Pitfall 4: `foreach(i RANGE ${last})` with `last = -1`
Iterates `0` and `-1`. An empty array is a vacuous-pass vector, so guard `n EQUAL 0` first (fail the check) and use `RANGE 0 ${last}`. [OBS]

### Pitfall 5: `\b`, `\w`, `\s`, `\d`, `{n}`, `[[:alpha:]]`, lookaround
`(?=…)` fails to compile. `\bbar` **does not error and matches nothing**. `\w`, `\s`, `\d`, `a{2}` and `[[:alpha:]]` silently never match. Class ranges, `(a|b)+`, `^`, `$`, `[a-z]+[_]` work. Do word-boundary and class matching in sh with `grep -E -w` and POSIX classes. [OBS, identical on 3.25.3]

### Pitfall 6: `;` is not a statement separator in CMake
`set(a 1); set(b 2)` is a parse error. Also never build variable names from ids containing `@`, `.` or `+` (mnemonics such as `MVO@`, `ADD@`). Use parallel lists with `list(GET)`. [OBS]

### Pitfall 7: CRLF is invisible to `file(READ)`
`file(READ)` removes the `\r` of a `\r\n` pair (read length 16 for an 18-byte file). Detect with `file(SIZE f s)` versus `string(LENGTH "${content}" n)`: unequal means CRLF. A lone CR or any non-ASCII byte is caught by `if(content MATCHES "[^\t\n -~]")`. Do not use `file(READ … HEX)` with `string(FIND "0d0a")` (unaligned false positives are possible; an aligned regex works but is heavier). Both checks ran in about 0.03 s on a 200 KB file. A UTF-8 BOM is silently accepted by the JSON parser, so the same `[^\t\n -~]` scan is what rejects it. [OBS]

### Pitfall 8: BSD vs GNU
`sed -i 's/x/y/' f` fails on macOS (`undefined label`); `sed -i '' …` fails on GNU. Use `sed -E -e '…' f > f.tmp && mv f.tmp f` (both support `-E`). Avoid `grep -P`, `echo -e`, `[[`, `local`, arrays, `set -o pipefail` (dash). `cp -R dir/. dest/` and `mktemp -d "${TMPDIR:-/tmp}/x.XXXXXX"` work on both. Note that this machine's interactive `grep` is a shell function wrapping ugrep; `sh tools/gate.sh` uses `/usr/bin/grep`. Test with `/usr/bin/grep` and `dash`. [OBS]

### Pitfall 9: Defects and tensions inside CONTEXT that the plan must settle
1. **D-12.7 denylist scope.** The denylist forbids `Zbiciak`, `jzintv`, `Nathan Woods`, `Palazzolo`, `license:BSD-3-Clause`, excluding only `docs/provenance/` and `.planning/`. But `docs/provenance.md` (a file, not that directory), `data/discrepancies.json`, `data/sources.json` and `data/claims.json` must legitimately name those emulators (D-14, D-15, C1-C28 text). A literal implementation fails the clean tree. **[REC]** Scan code and fixture paths only (everything except `.planning/`, `docs/`, `data/`, `README.md`, `AGENTS.md`, `.claude/`, `tools/check/denylist.txt` and `tools/check/controls/`). In Phase 1 that scope contains almost nothing; the check exists for Phase 2+. Keep one control that proves it fires on a seeded `src/x.c`. Alternatively also scan `docs/` and `data/` for the **licence-header** pattern only.
2. **D-12.4 claim-language tripwire vs the docs that define the rules.** `docs/provenance.md` and `docs/evidence-tiers.md` will naturally contain "verified", "supports", "accurate". Waive with `[nc]` per line, or write rule text without those words. Tags must be on the **same line** as the word (grep is line-based), so write one sentence per line in `docs/`.
3. **Hygiene secret scan vs `AUTONOMY.md`.** AUTONOMY.md lists the literal secret prefixes. Use tight regexes that require real token shape (below) so the doc does not self-match. Verified: zero hits across all 18 tracked files today. [OBS]
4. **`~` and `/Users/<name>` in docs.** CONTEXT D-22 itself contains the text `~`. A loose `[A-Za-z0-9._-]+` after `/Users/` matches `...`. Require an alphanumeric first character. Verified: the triple-dot and angle-bracket placeholders no longer match, and a real-looking home path still does. [OBS]
5. **Decision Register already has D1/D2/D3/D9.** `PROJECT.md` "Delegated decisions" already contains rows for D1, D2, D3 and D9 [OBS]. The plan task is to verify and cross-link them, not to add duplicates. D2's row must point to `docs/time-base.md`.
6. **Row count.** CONTEXT says "about 90 rows". My count from pages 53-58 is roughly 80 mnemonic rows (14 reg-reg and single-reg, 8 shifts, 11 control, 6 jumps, 17 branch mnemonics with aliases, 7 direct memory, 9 indirect, 7 immediate) plus SDBD-prefixed variants. Do not pad rows to reach 90. Model branch aliases (BC/BLGT etc.) as `aliases` on one row because they share an encoding.

### Pitfall 10: Agents write absolute paths into tracked files
Executors and verifiers habitually cite absolute scratch or home paths. Any such line fails the tree hygiene check on the next gate run. Tell every executor: cite repo-relative or `../sibling` paths only; keep rendered page images and the cached PDF under `refs/` (gitignored). Note this research file contains no absolute paths by design.

### Pitfall 11: Rendered page images are derivative of a rights-bearing scan
The extraction workflow creates PNGs of the manual. They must live in `refs/` and never be committed. Add `*.pdf`, `refs/` and common image extensions to the unlisted-binary rule (D-12.6 lists only `.int .bin .rom .itv .cfg .cc3` and PDFs). [REC: add `.png .jpg .jpeg .gif`]

### Pitfall 12: CI-only inputs missing
The gate must not require anything under `refs/` (CI has none). Hash verification of cached PDFs runs only when present and prints `refs-verified=0/N (cache absent)` so absence is visible and not silently passing.

## Code Examples

### Hygiene regex set (C locale, tested on the real tree)
```sh
LC_ALL=C; export LC_ALL
HOME_PATH='/Users/[A-Za-z0-9][A-Za-z0-9._-]*|/home/[A-Za-z0-9][A-Za-z0-9._-]*|-Users-[A-Za-z0-9][A-Za-z0-9._]*-|[A-Za-z]:\\Users\\[A-Za-z0-9]'
HOME_OK='/home/runner'                       # filter with grep -v after the match
SECRET='gh[pousr]_[A-Za-z0-9]{36}|github_pat_[A-Za-z0-9_]{20,}|AKIA[0-9A-Z]{16}|-----BEGIN [A-Z ]*PRIVATE KEY-----'
EMAIL_OK='@users\.noreply\.github\.com|noreply@github\.com|@example\.(com|org|net)|git@github\.com'
# message-only allowance for the AI co-author trailer: noreply@anthropic\.com
# stdin text; true if a line matches $1 and not $2
leaks() { grep -I -E -e "$1" | grep -v -E -e "$2" | grep -q .; }
```
The `-Users-<name>-` branch catches path slugs that name the owner (they appear inside scratch-directory names). `.env` and `password=` in AUTONOMY §6 are covered by the filename check (`.env*` tracked = fail) and by an optional quoted-assignment pattern. Findings print the rule and the file, never the matched text.

### History scrub (rehearsed in a throwaway clone; 9 commits, about 3 s)
```sh
# Preconditions: working tree clean, all phase work committed, no other worktrees/executors running.
FILTER_BRANCH_SQUELCH_WARNING=1 git filter-branch -f --tree-filter '
for f in $(grep -rIl -E "/Users/[A-Za-z0-9._-]+|-Users-[A-Za-z0-9._]+-" . --exclude-dir=.git 2>/dev/null); do
  sed -E -e "s#/private/tmp/claude-[0-9]+/-Users-[A-Za-z0-9._]+-[^ )]*#<local-scratch>#g" \
         -e "s#/Users/[A-Za-z0-9._-]+/projects/iamverysmartvision#.#g" \
         -e "s#/Users/[A-Za-z0-9._-]+/projects#..#g" \
         -e "s#/Users/[A-Za-z0-9._-]+#~#g" "$f" > "$f.scrub.tmp" && mv "$f.scrub.tmp" "$f"
done' -- --all
rm -rf .git/refs/original
git reflog expire --expire=now --all && git gc -q --prune=now
```
Result in the clone: 0 hits for `/Users/<name>` and the slug across `git rev-list --all` (and 0 on the rewritten `main` before cleanup), earlier text read `Sibling LibFuturo API, read-only: ../libfuturo/include/libfuturo/libfuturo.h`. [OBS]
- `--tree-filter` runs in a temp checkout, so use `grep -r`/`find`, not `git ls-files`.
- The sed uses only `-E`/`-e`, valid on BSD and GNU. Write through a temp file, never `-i`.
- The filter body is single-quoted, so it cannot contain a single quote. Keep it in a script file outside the repo (not committed) if it needs to grow, to avoid shell-quoting accidents.
- Commit hashes cited in `.planning/` summaries become stale after the rewrite. Accept it or re-cite.
- Other hygiene results today: all 9 commits use `szTheory <szTheory@users.noreply.github.com>` as author and committer [OBS: `git log --all --format='%an <%ae> | %cn <%ce>'`]. The only other trailer is `Co-Authored-By: … <noreply@anthropic.com>` in messages (allowed in messages only). No remote exists. The branch is `main`.

### Claim-language tripwire and claim-ref extraction (BSD grep, executed)
```sh
grep -n -i -w -E 'supports|compatible|passes|accurate|verified|runs' README.md docs/*.md \
  | grep -v -E '\[claim:CL-[0-9]{4}\]|\[nc\]'            # nonempty output = failure token claim-untagged
grep -o -h -E '\[claim:[A-Za-z0-9-]+\]' README.md docs/*.md   # then each id must exist in data/claims.json
```
`-w` correctly ignores `supporting` and `runsheet`. A claim tag on a different line does not count.

### CI workflow (draft; action SHA verified)
```yaml
name: gate
on:
  push:
    branches: [main]
  pull_request:
concurrency:
  group: gate-${{ github.ref }}
  cancel-in-progress: ${{ github.ref != 'refs/heads/main' }}
permissions:
  contents: read
jobs:
  gate:
    runs-on: ubuntu-24.04
    timeout-minutes: 10
    steps:
      - uses: actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1 # v7.0.1
        with:
          persist-credentials: false
      - run: cmake --version | head -1
      - run: sh tools/gate.sh
```
- `actions/checkout` is the only action. Latest release is v7.0.1, published 2026-07-20; the `v7` tag and `v7.0.1` resolve to commit `3d3c42e5aac5ba805825da76410c181273ba90b1` [VERIFIED: gh api]. Gabbaboy pins checkout by SHA with a version comment [OBS]; follow it. Tradeoff: SHA pinning blocks silent tag retargeting at the cost of manual bumps, which is trivial for one action.
- **`ubuntu-latest` is mid-migration** from 24.04 to 26.04, rolling out 2026-10-19 to 2026-11-19 [CITED: github.blog changelog 2026-09-17]. The label can resolve to either image from run to run. CMake is **3.31.6 on 24.04** [CITED] and about **4.3-4.4 on 26.04** [CITED, LOW-MEDIUM mirror]. Both satisfy the 3.25 floor. **[REC]** pin `ubuntu-24.04` for a stable first run and note in `CI-EFFICIENCY.md` to re-evaluate the label after 2026-11-19. D-23 says `ubuntu-latest`; this is a label choice within Claude's discretion, flagged in Open Questions.
- `fetch-depth: 0` is optional. Add it only if the CI gate runs `--history` (tiny repo, negligible cost). Default depth 1 suffices for the tree gate.
- `cancel-in-progress` is disabled on `main` so a push is never left with no completed run. Pull requests cancel stale runs.
- Measure with: `gh run view <id> --json createdAt,startedAt,updatedAt,conclusion,jobs --jq '…'`. The fields `createdAt`, `startedAt`, `updatedAt`, `jobs[].startedAt`, `jobs[].completedAt` and `jobs[].steps[].startedAt/completedAt` exist [OBS: queried a sibling repo run]. Queue time = job `startedAt` minus run `createdAt`.

### Publication command (after all gates)
```sh
gh repo create szTheory/iamverysmartvision --public --source . --remote origin --push \
  --description "Deterministic, evidence-bound emulator core (working name). Not affiliated with Mattel."
gh repo view szTheory/iamverysmartvision --json defaultBranchRef,visibility
```
The repo does not exist yet [OBS: `gh repo view` returned "Could not resolve"]. `gh repo create` pushes the current branch and the default branch becomes `main`. Flags `--source`, `--push`, `--remote`, `--description`, `--public` exist in gh 2.101.0 [OBS: `--help`].

## CP-1610 Reference Notes: Extraction Workflow (FND-06)

### What I read and verified (this session, page images)
[OBS] means I opened the PNG with the Read tool. The page index below equals the pdf page index (pdftotext of pdf 46 returns §3.2.1; the 110-dpi render of pdf 62 is §3.4.6).

| pdf | Content | Result |
|-----|---------|--------|
| 46 | §3.2.1 data access formats (text OCR garbled: `00]`, `,011`) | Operation codes 001..111 under bit 9 = 1 [OBS via OCR+structure; confirm from image] |
| 50-52 | §3.3.3 single register, §3.3.4 internal control, §3.3.5 jump formats | [OBS] Fig 3.3.3: INCR 001, DECR 010, COMR 011, NEGR 100, ADCR 101, GSWD 110, RSWD 111. Fig 3.3.4: HLT 000, SDBD 001, EIS 010, DIS 011, TCI 101, CLRC 110, SETC 111. Footnote: GSWD with R4/R5 is a NOP; R6/R7 is a NOP plus a PCIT pulse. Jump is a 3-decle form `0000 000 100` / `B B A A A A A A D E` / `A…A` |
| 53 | Register-register table and register-shift table | [OBS] MOVR `0010 SSS DDD`, TSTR `0010 SSS SSS`, JR `0010 SSS 111`, ADDR 0011, SUBR 0100, CMPR 0101, ANDR 0110, XORR `0111 SSS DDD`, CLRR `0111 DDD DDD`, INCR…ADCR `0000 ooo DDD`; shifts `0001 ooo NRR` (SWAP 000, SLL 001, RLC 010, SLLC 011, SLR 100, SAR 101, RRC 110, SARC 111), 6 cycles (N=0) or 8 (N=1), "Shifts are not interruptable" |
| 54 | Branch table | [OBS] `1000 S0 cccc` and BEXT `1000 S1 EEEE`. Cycles printed `7/9` with no taken/not-taken label (matches the new discrepancy in D-21). NOPP is the `1000` condition code (two-word NOP, 7 cycles) |
| 55 | Direct memory and indirect register tables | [OBS] MVO `1001 000 SSS` 11 cycles; MVI/ADD/SUB/CMP/AND/XOR direct 10 cycles with opcodes 1010 to 1111; indirect MVO@ 9, others 8*, `*` = add 3 cycles if MMM=6; PSHR `1001 110 SSS` 9, PULR `1010 110 DDD` 11; MVO@ with SSS = MMM in 4..7 unsupported |
| 56 | Control and jump tables | [OBS] GSWD `0000 110 0DD` 6; NOP `0000 110 10N` 6; SIN `0000 110 11N` 6; RSWD `0000 111 SSS` 6; HLT/EIS/DIS/TCI/CLRC/SETC 4 cycles; EIS, DIS, TCI, CLRC, SETC marked "Not Interruptible"; HLT row says nothing about interruptibility; J/JE/JD/JSR/JSRE/JSRD 12 cycles; `BB≠11` for the JSR family |
| 57-58 | Immediate forms, SDBD tables, glossary | [OBS] MVOI 9, MVII/ADDI/SUBI/CMPI/ANDI/XORI 8; SDBD 4 cycles; SDBD+indirect 10 (MVI@ etc.); SDBD+immediate 14; MMM glossary (000 direct, 001-011 R1-R3, 100/101 R4/R5 post-increment, 110 R6, 111 R7 immediate). Note the XOR@ row in the SDBD indirect table is printed `111 MMM DDD` (three digits; a scan or print slip, the pattern must be inferred) |
| 62 | §3.4.6 stack operations | [OBS] "contents of R6 is incremented by one" after data is moved out via R6; "first decremented then data is fetched" moving in. **C17 is confirmed from the page image: the stack grows upward, post-increment on push, pre-decrement on pull** |
| GI 1980 catalog pdf 191 (printed 5-10) | AY-3-8900/-1 STIC page | [OBS] "Operation from a 4.000MHz clock for AY-3-8900 and from a 3.579545MHz clock for AY-3-8900-1"; "168H by 104V for AY-3-8900 or 167H by 105V for AY-3-8900-1 of which 159 x 96 are visible positions"; 20H x 12V background; 16 colors; pins `MSYNC`, `RSTIN` present. The catalog OCR text misreads `AY` as `AV`, so even profile cites need the image |

Not read this session: pdf 12-14, 21-25, 27, 32, 35, 74, 75 (state sequences, timing figures, reset/MSYNC/IAB, interrupts). Those are the executor's work.

### The 1024-pattern coverage outcome ([INF], from my transcription)
- The 10-bit opcode decle is bits 9..0; bits 15..10 are "Future Use". The groups `0000` (control, single register, jump), `0001` (shifts), `0010`-`0111` (register-register), `1000` (branch, 64 patterns incl. BEXT), `1001`-`1111` (memory) tile the whole space. Blue Sky's "1024 separate op-codes" [CITED via STACK.md, tier 2] agrees.
- **Expected `unspecified-by-manual` count at decle level: 0.** The class must exist in the schema but is expected to be empty. If the second reader finds an unclaimed pattern, that is a transcription error to resolve, not an unspecified opcode.
- Documented specializations that overlap (all need `refines`): TSTR, JR within MOVR; CLRR within XORR; PSHR, MVOI and direct MVO within MVO@ (and the MVI/ADD/SUB/CMP/AND/XOR parallels: direct `…000…`, PULR, immediate `…111…`).
- Real ambiguity: `0010 111 111` is TSTR R7 (SSS=DDD=7), JR R7 and MOVR R7,R7. Record it as an explicit overlap rule (`also_encodes`) with a note, not as a transcription error.
- Holes at the **operand** level (not the 1024): the jump family's second-decle `D E` field value `11` is not listed (only 00, 01, 10 appear); `BB=11` is "J" by definition. Log these as notes in the row, not as pattern gaps.
- Semantic (non-decode) unsupported cases to record as row text: SDBD modes 0 and 6, MVO@ SSS=MMM in 4..7, GSWD R4/R5/R6/R7.

### Procedure (efficient, concrete)
1. `mkdir -p refs/pages`. Copy the PDF to `refs/SRC-gi-cp1600-um-1975.pdf` and verify `cmake -E sha256sum` equals `ecdabe5c52b5e4429a40d1779d82315ab74bae9db00df774e605f8a882dc2b72` (size 9,917,417). A UA-less download from bitsavers returns 403; use a browser User-Agent header. [OBS: HEAD with UA returns 200, content-length 9,917,417]
2. `pdftoppm -r 110 -f 46 -l 62 -png <pdf> refs/pages/gi` renders 17 pages in about 10 s. About 0.6 s per page; all 116 pages in about 65 s. 110 dpi gives 935x1209 (portrait) or 1216x945 (landscape table pages), legible for all table text [OBS]. Output names are `gi-NNN.png` with the pdf page number.
3. For dense timing diagrams (pdf 26-37) or any doubtful digit, re-render one page at `-r 150` and crop with `pdftoppm -x X -y Y -W W -H H`. Do not read the OCR text for any encoding, cycle count or status bit (D-17). Use `pdftotext -f N -l N -layout` only to locate pages.
4. Read each page image with the Read tool (it displays PNGs). Transcribe into the target file immediately. Record `pdf <n>` and `§` in each row.
5. Cross-check printed digits against structure, not memory. Scan artifacts exist (`lllO`, `00]`, the three-digit `111` above). Use the page-46 operation-code table and the high-nibble sequence (MVO 1001, MVI 1010, ADD 1011, SUB 1100, CMP 1101, AND 1110, XOR 1111) as a second independent source for the memory family.

### Page budget and split into plan tasks
About 30 page images in total (46-58, 62; state sequences 21-25, 27; timing 32, 35; start/stop and interrupt 74-75; plus 12-14 for reset/MSYNC wording). Rough effort [INF]: 120-250k tokens across the phase, with the table rows the largest single output (about 80-90 rows x ~10 fields, 25-30 KB JSON).

| Task | Pages | Output | Notes |
|------|-------|--------|-------|
| T-A Notes: formats, addressing, stack | 46-52, 62 | `docs/cp1610/reference-notes.md` §formats, §addressing, §stack | Records C17 `resolved-primary` |
| T-B Notes: reset, interrupts, bus, SDBD | 12-14, 21-25, 27, 35, 74, 75 | notes §reset/MSYNC/IAB, §interrupts, §bus requests, §start/stop | New discrepancies C29..C32 text |
| T-C1 Rows batch 1 | 53 | register-register, single register, shifts rows | |
| T-C2 Rows batch 2 | 54, 56 | branches, control, jumps | Carry `7/9` as `cycles_printed` |
| T-C3 Rows batch 3 | 55, 57, 58 | memory, indirect, immediate, SDBD forms | Carry modifiers (`*` +3 if MMM=6, SDBD +… ) as structured list |
| T-D Coverage | — | `coverage.cmake`, controls (missing row, misread pattern, overlap without `refines`, empty table) | Do not start before T-C* rows exist |
| T-E Second reader | 46-58, 62 | `data/cp1610/second-reader.json` + short log | See below |

T-A and T-B are prose. Each T-C is JSON. T-C1..T-C3 can run in parallel (separate row files merged into one array by the plan, or one file edited sequentially; parallel edits to one JSON file conflict).

### Second-reader pass (D-20)
Design for independence, because a verifier that sees the first reader's rows will anchor on them. [REC]
1. A fresh agent, with no access to `opcode-families.json`, re-extracts the same fields from the same page images into a scratch JSON (not committed).
2. A throwaway script diffs the two by row id and field. Every disagreement is re-read from a higher-resolution crop and resolved, with the winning reading and the pdf page logged.
3. Commit `data/cp1610/second-reader.json`: one entry per row id with `agree | resolved` plus the page, and a count. The gate asserts the set of ids equals the row ids and that no entry is missing (a mechanical, reviewable artifact; the reading itself stays review-only).
4. Honest limits to state in the notes: same model family, so common-mode misreads are possible; the coverage check independently catches encoding errors but not cycle counts, status bits or interruptibility; electrical specifications in the manual are "Tentative" [CITED: D-17] and the manual describes the CP-1600, not the CP-1610 [INF].

## Publication Mechanics (FND publication, D-04, D-22, D-23)

**Ordering constraint:** publication is the last plan, and it runs only after the full gate (with hygiene) is green.

1. Everything committed, tree clean, no executor or worktree active.
2. `sh tools/hygiene.sh --tree` passes after the scrub (the filter scrubs HEAD too, so no separate scrub commit is needed).
3. Run the history rewrite above. Then `sh tools/hygiene.sh --history` must pass: noreply emails only, no `/Users/<name>`, no secrets, no unlisted binaries, in every commit and message.
4. `sh tools/gate.sh` passes locally under `sh` and `dash`. Clone the rewritten repo to a temp directory and run `dash tools/gate.sh` there, with a PATH that has `cmake`, `git`, `grep`, `sed`, `sort`, `mktemp` and nothing else extra, to approximate the runner.
5. `gh repo create … --public --source . --remote origin --push`.
6. `gh run watch` the first run; on success append the measurements to `.planning/CI-EFFICIENCY.md` and commit normally. If CI is red, fix forward with a normal commit (history is already clean; rewriting published history is a hard stop).
7. No release (D-24).

**Why hygiene `--history` is not in every gate run [REC]:** until step 3 the unpublished history legitimately contains the paths. After publication a history leak cannot be fixed by rewriting. So `--history` is a publication and pre-release step, and the always-on guard is `--tree` (every tracked file) which also covers future commits' content.

**Optional variant (not adopted by default):** create the repo `--private`, let CI go green, then `gh repo edit --visibility public --accept-visibility-change-consequences`. It reduces the risk of a red first public CI but deviates from D-22's literal command. Left as an Open Question.

## Runtime State Inventory

Not a rename or migration phase. The one migration-like action is the path scrub. After it: stored data none; live service config none; OS-registered state none; secrets none; build artifacts: `.git/refs/original` and the reflog still hold pre-scrub objects until the cleanup commands run (they are in the procedure). Verified in the clone: after cleanup `git rev-list --all` shows 0 hits. [OBS]

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| `ubuntu-latest` = 24.04 | `ubuntu-latest` rolling to 26.04 | 2026-10-19 to 2026-11-19 [CITED] | Pin the label for a stable first run |
| `actions/checkout@v4` (gabbaboy) | v7.0.1 | 2026-07-20 | Use the SHA from `gh api` |
| `cmake_minimum_required(VERSION <3.5)` | Removed in CMake 4.x | CMake 4.0 | Always use `3.25` as the baseline in `-P` scripts |

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | The 26.04 runner image ships CMake 4.3-4.4 (mirror of release notes, not the official software list) | CI workflow | Low: both versions exceed the 3.25 floor; the workflow logs `cmake --version` |
| A2 | The effort estimate of 120-250k tokens for CP-1610 extraction | CP-1610 | Schedule only |
| A3 | Pages 12-14, 21-25, 27, 32, 35, 74, 75 contain what CONTEXT D-19 says they contain (not re-read by me) | CP-1610 | Executor finds different pages; time lost, no correctness risk |
| A4 | "BB=11 is J by definition" reading of page 57/58 and the jump `D E` field holes | CP-1610 | Operand-level notes could be slightly off; the second reader re-checks |
| A5 | The `ubuntu-24.04` pin is acceptable to the owner despite D-23 naming `ubuntu-latest` | CI | Cosmetic |
| A6 | A weakest-field rule can be made mechanical via a `standing: provisional|established` claim field (REC, my design) | Claims | Planner may prefer another encoding; any works if the checker enforces D-09 |

## Open Questions (RESOLVED)

1. **Denylist scope (Pitfall 9.1).** RESOLVED.
   - Known: CONTEXT excludes only `docs/provenance/` and `.planning/`; the docs and data legitimately name the emulators.
   - Unclear: owner intent for `docs/provenance.md` and `data/*.json`.
   - Recommendation: scan code and fixture paths only, plus the licence-header pattern anywhere outside `docs/provenance*` and `.planning/`. Record as a plan-level decision.
   - RESOLVED: orchestrator resolution 1 sets the scope (everything tracked except `.planning/`, `docs/provenance.md`, `docs/provenance/`, `data/sources.json` and `tools/check/`); the planner adds `.claude/` because `.claude/CLAUDE.md` mirrors `.planning/research/STACK.md`. Author names and licence headers are never waivable; emulator project names need `[cite-only]` on the same line. Location: plan 01-07 `<context>` and Task 1; SKELETON.md §7 R3.
2. **Pin `ubuntu-24.04` or keep `ubuntu-latest`?** Recommendation: pin now, revisit after 2026-11-19.
   - RESOLVED: pin `ubuntu-24.04` (orchestrator resolution 5). Location: plan 01-01 Task 1 step 12 (workflow), SKELETON.md Architectural Decisions (CI row), plan 01-12 `CI-EFFICIENCY.md` decision to revisit the pin after 2026-11-19.
3. **Private-first publication variant.** Default follows D-22 (public at creation). Planner may adopt the private-first variant to see CI green before exposure.
   - RESOLVED: private-first variant rejected; the repository is created public directly, per D-22 and AUTONOMY.md section 1 (owner pre-authorized public publication), after the history rewrite and `tools/hygiene.sh --history` pass. Location: plan 01-12.
4. **Where does the fixture-manifest schema document live?** CONTEXT D-07 lists no file. Recommendation: `docs/RIGHTS.md` carries the human text and `tools/check/schema/manifest.txt` the machine form; one real sample entry lives at `fixtures/FX-sample-rights-unknown/manifest.json` (`committed: false`, `redistributable: unknown`) so the real-tree gate is not vacuous, and a flipped copy (`committed: true`) is the control.
   - RESOLVED: the schema document is `docs/fixture-manifest.md` (orchestrator resolution 7); `docs/RIGHTS.md` holds the rights policy and points to it; the machine form is the dedicated checker `tools/check/manifests.cmake` (multi-file kind, SKELETON.md §4); the sample entry and flipped-copy control are as recommended. Location: plan 01-06.
5. **Empty inventories.** `fixtures/` legitimately has no real fixtures. Recommendation: `.ids` files accept `#allow-empty: <reason>`; any other empty inventory fails.
   - RESOLVED: adopted as the inventory contract. Location: SKELETON.md §5 (`#allow-empty: <reason>`), implemented by `read_ids` in plan 01-01; the fixtures inventory is non-empty anyway because of the sample entry (plan 01-06).
6. **AY-3-8914 datasheet.** A search found no primary datasheet (only 8910-family material and secondary mentions of a 4-level envelope and rearranged registers) [CITED: search results, LOW]. Keep C11 open and `psg.part` as tier 4. Time-boxed as PIT-18 requires.
   - RESOLVED: no primary datasheet found (time-box spent); `psg.part` stays tier 4 with the datasheet recorded as none-found, per D-10, and C11 stays open. Location: plan 01-01 profile field `psg.part`; plan 01-04 discrepancy C11.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| cmake | gate checkers, controls | yes | 4.4.3 (also executed 3.25.3 in scratch) | none needed |
| POSIX sh and dash | gate, CI parity | yes | bash 3.2 posix mode; `/bin/dash` present | none needed |
| git | everything, publication | yes | 2.41.0 | none needed |
| gh | repo creation, run timing | yes, authed | 2.101.0, scopes `repo, workflow, read:org, gist` | none needed |
| pdftoppm / pdftotext | CP-1610 extraction | yes | poppler 26.04.0 | page renders are a dev aid only |
| ninja, shellcheck | not required this phase | yes (optional) | — | — |
| Network to bitsavers / Blue Sky | refs cache, source hashes | yes (needs browser UA) | — | Wayback snapshot of the Blue Sky page exists (20260909 timestamp) |

**Missing dependencies with no fallback:** none.

Pre-computed `sources.json` facts [OBS: downloaded and hashed this session]:
- GI CP-1600 manual: 9,917,417 bytes, sha256 `ecdabe5c52b5e4429a40d1779d82315ab74bae9db00df774e605f8a882dc2b72`, 116 pages.
- GI 1980 catalog: 41,975,851 bytes, sha256 `70094d12e9512edf8a73995a17eb1bcd8713a5aad37331229a3a902d40bc9a09`, 558 pdf pages (STIC at pdf 190-191 = printed 5-9/5-10; AY-3-8910 at pdf 204-209).
- GI 1982 catalog: 36,703,971 bytes, sha256 `b609963d33ace6c15fbd0d7c530abf5a9e8f6ce2483088b2c13ba93f9df38f8a`, 562 pdf pages.
- Blue Sky 2609 page (HTML): 58,009 bytes, sha256 `3d7791593b80fbd040f387ecd8197610ec970134887086fd2e53e38297384819`. Web pages change, so record the access date. Wayback: `web.archive.org/web/20260909215052/https://history.blueskyrangers.com/hardware/2609.html`.
All four URLs returned 200 with a browser User-Agent; bitsavers returns 403 without one.

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | None to install. POSIX sh orchestrator + `cmake -P` checkers + overlay-based negative controls |
| Config file | none. Inventories in `data/expected/*.ids`, control list `data/expected/controls.ids`, gate step list `data/expected/gate-steps.ids` |
| Quick run command | `sh tools/gate.sh` (expected about 2-3 s: records, ids, text scans, hygiene tree, about 20 controls, 0.5 s coverage) |
| Full suite command | `sh tools/gate.sh` (same; plus `dash tools/gate.sh` for CI parity). Publication-only addition: `sh tools/hygiene.sh --history` |

### Phase Requirements to Test Map
| Req ID / SC | Behavior | Test Type | Automated Command | File Exists? |
|-------------|----------|-----------|-------------------|-------------|
| FND-01 / SC1 | Profile has every expected field, known tag, non-blank value and cite; `unknown-default-chosen` carries `resolving_evidence`; D3 cited | check + controls (blank value, unknown tag, missing field) | `sh tools/gate.sh` (step records:profile) | Wave 0 |
| FND-02 / SC2 | Tier vocabularies exist (two, distinct); claims resolve (profile, tier, artifact path); every `[claim:…]` in README and docs resolves; claim-language tripwire | check + controls (dangling artifact, untagged word, `[claim:CL-9999]`) | gate step claims + text | Wave 0 |
| FND-03 / SC2 | Discrepancy ids set-equal to the inventory (C1-C28 plus C29+); each has `default_policy` and a `resolver` of kind `test:`/`measurement:`/`pending:`; pending counted not failed | check + controls (missing C17, duplicate id) | gate step discrepancies + ids | Wave 0 |
| FND-04 / SC3 | Manifest schema enforced fail-closed; effective-redistributable rule; sample `unknown` entry is non-redistributable; flipped copy fails | check + controls (unknown+committed, yes without evidence, misspelled key) | gate step manifests | Wave 0 |
| FND-05 / SC3 | `docs/provenance.md` has the no-copy rule and review checklist; denylist grep on code paths; `refs/` untracked; unlisted binaries fail | check + controls (denylisted string in `src/`, tracked `refs/` file, unlisted fixture file) | gate step text + files | Wave 0 |
| FND-06 / SC4 | Every row has a `pdf <n>` cite; sources registry complete; 1024-pattern coverage; `second-reader.json` ids equal row ids; C17 status `resolved-primary` | check + controls (row missing, misread pattern, no `refines`, empty table) | gate step cp1610 | Wave 0 |
| FND-07 / SC5 | Required glossary terms present (inventory); profile keys carry unit suffixes (exact field inventory); bare-"clock" tripwire on `docs/time-base.md`; D2 row in Decision Register links the doc | check + inventory | gate step glossary | Wave 0 |
| Gate integrity (D-12.11) | CRLF, non-ASCII, empty file, empty inventory all fail | controls (CRLF, empty record file, empty inventory) | gate controls | Wave 0 |
| Hygiene (D-12.10) | Tree: no home paths, no non-noreply emails, no secrets, no unlisted binaries. History: same, plus author emails | `tools/hygiene.sh --tree` in gate; `--history` at publication | `sh tools/hygiene.sh --tree` | Wave 0 |
| CI parity (D-23) | The workflow runs exactly `sh tools/gate.sh` | check: workflow file contains that exact step and no other `run:` of a different gate | gate step ci-parity (grep) | Wave 0 |

**Success-criterion coverage.** SC1 to SC5 each have a machine check above. The "is the content true" part is review-only:
- whether tier labels are assigned honestly (tags are checked for presence and vocabulary, not truth);
- whether page cites match the images (mitigated by the second-reader pass; the pattern coverage check independently catches encoding transcription errors only);
- whether the no-copy rule was followed (the denylist catches accidents, not authorship);
- paraphrased claim language such as "works with" that the tripwire word list misses.
Say this honestly in `docs/provenance.md`, as D-14 requires.

### Sampling Rate
- **Per task commit:** `sh tools/gate.sh` (seconds).
- **Per wave merge:** `sh tools/gate.sh` and `dash tools/gate.sh`.
- **Phase gate:** gate green locally under both shells, `hygiene.sh --history` green, clean-clone gate green, first CI run green, before `/gsd-verify-work`.

### Wave 0 Gaps
All of it is greenfield (no tracked file exists outside `.planning/`, `AGENTS.md` and `.claude/`):
- [ ] `tools/gate.sh`, `tools/hygiene.sh`, `tools/check/{lib,records,ids,coverage}.cmake`, `tools/check/run-controls.sh`, `tools/check/schema/*.txt`
- [ ] `tools/check/controls/_good/tree/` plus about 20 control directories and `data/expected/controls.ids`
- [ ] `data/expected/{discrepancies,profile-fields,claims,gate-steps,controls,glossary-terms}.ids`
- [ ] `.gitignore` (`refs/`, `*.pdf`, page images), `LICENSE`, `README.md`, `.github/workflows/gate.yml`, optional `.github/pull_request_template.md` carrying the FND-05 review checklist
- Framework install: none.

**Negative control set (exact inventory; matches D-12 plus gaps I found).** missing-c17, duplicate-id, blank-profile-value, unknown-tag, dangling-claim-artifact, untagged-claim-word, missing-claim-ref, manifest-unknown-committed, manifest-yes-without-evidence, manifest-misspelled-key, unlisted-fixture-file, tracked-refs-file, denylisted-string, crlf, empty-record-file, empty-inventory, coverage-missing-row, coverage-misread-pattern, coverage-overlap-without-refines, blank-expect (the runner's own guard), plus the one positive `_good`. [REC]

## Plan Decomposition (tracer-first)

| Plan | Wave | Content | Depends | Notes |
|------|------|---------|---------|-------|
| P01 Tracer | 1 | `gate.sh` + `lib.cmake` + `records.cmake` + `run-controls.sh` + `_good` tree + `tiers.json` + profile record (all D-10 fields) + profile controls + `.gitignore` | none | Proves the whole loop on one record kind. Includes the vacuous-pass guards (size compare, `n EQUAL 0`, EXPECT blank-line guard, steps-run == expected) |
| P05a/b CP-1610 prose (T-A, T-B) | 1 | notes sections, no gate coupling | none | Long pole; starts in parallel with P01 (disjoint files) |
| P05c CP-1610 rows (T-C1..3) | 1-2 | `opcode-families.json` | none to write; P01 to wire | Rows can be authored before the checker exists |
| P02 Discrepancies | 2 | `discrepancies.json` C1-C28 + C29..C32, `.ids`, controls | P01 | Seed from SUMMARY.md §Source Conflicts |
| P03 Claims + tiers + docs | 2 | `claims.json`, `docs/evidence-tiers.md`, tripwire, `[claim:]` resolution | P01 | Seed 3-6 real claims (for example C17 stack direction, decode coverage) |
| P04 Rights + provenance | 2 | manifest schema, sample entry, `docs/provenance.md` and CONSULTATIONS, `docs/RIGHTS.md`, `sources.json`, denylist, `refs/` and unlisted-binary checks | P01 | Resolve Pitfall 9.1 first |
| P06 Time base + register | 2 | `docs/time-base.md`, glossary inventory, tripwire, verify and link D1/D2/D3/D9 | P01 | Mostly prose |
| P07 Coverage + second reader | 3 | `coverage.cmake`, `second-reader.json`, controls | P05c, P01 | The check is the cross-check |
| P08 Hygiene + README + CI file | 3 | `hygiene.sh`, `LICENSE`, `README.md`, workflow file, final gate-steps inventory and step ordering | P02-P07 | Owns `gate-steps.ids`, so parallel plans never edit it |
| P09 Publication | 4 (last) | history rewrite, `--history`, clean-clone `dash` gate, `gh repo create`, first CI watch, `CI-EFFICIENCY.md` | all | Only irreversible step. No human checkpoint needed (delegated, AUTONOMY §1) |

**Shared-file hazards.** `tools/gate.sh`, the `data/expected/*.ids` files and `claims.json` are edited by several plans. Parallel waves must keep `files_modified` disjoint: use drop-in `tools/check/step-NN-*.sh` files and give each inventory to one owning plan.

**Risks (ranked).**
1. CP-1610 extraction accuracy and time. Mitigate: coverage check, `printed`/`pattern` self-consistency, blind second reader, time-box per PIT-18.
2. Checker sprawl (GB-CI-004). Mitigate: one generic walker, a 4-verb schema, a line budget for `tools/check/*.cmake`.
3. First CI run on an unfamiliar image (label migration). Mitigate: pin `ubuntu-24.04`, log `cmake --version`, clean-clone `dash` run first.
4. Agents leaking absolute paths into tracked files (Pitfall 10).
5. Vacuous passes: empty arrays, blank `EXPECT` lines, zero-step gates, `refs/` absent. Each has a named guard above.
6. Doc tripwire noise. Mitigate: `[nc]` convention and one-sentence-per-line docs.

## Security Domain

`security_enforcement` is enabled (ASVS level 1). This phase has no network service, auth, sessions or crypto. The relevant surface is repository supply chain, input parsing by the checkers, PII and secrets in a first public push, and shell safety.

### Applicable ASVS Categories
| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V2 Authentication | no | — |
| V3 Session Management | no | — |
| V4 Access Control | partly | Workflow `permissions: contents: read`; `persist-credentials: false`; the `gh` token is not written to any file |
| V5 Input Validation | yes | Checkers treat all JSON and file contents as untrusted: type guards, allowed-key lists, path charset rule `^[A-Za-z0-9._/-]+$`, no `eval`, always quote expansions in sh |
| V6 Cryptography | no | SHA-256 via `file(SHA256)`; no custom crypto |
| V14 Config / supply chain | yes | One SHA-pinned action; no network fetch in the gate; downloaded PDFs are untrusted and stay in `refs/` |

### Known Threat Patterns
| Pattern | STRIDE | Standard Mitigation |
|---------|--------|---------------------|
| Private path or username leaking into a public repo and history | Information disclosure | `hygiene.sh --tree` always; `--history` plus rewrite before first push |
| Secret committed | Information disclosure | Tight token-shape regexes; `.env*` filename rule |
| Third-party rights-bearing file committed | Repudiation / legal | Unlisted-binary and `refs/` checks; `.gitignore`; PDFs and page images stay local |
| Path or glob injection in a shell step | Tampering | Quote everything; reject odd path characters; pass lists via files, not argument splicing |
| Mutable action tag | Tampering | Pin `actions/checkout` by commit SHA |
| Over-privileged workflow token | Elevation of privilege | `permissions: contents: read` |
| Parsing hostile JSON/PDF | Denial of service | Checkers read only tracked JSON; no PDF parsing in the gate |

## Sources

### Primary (HIGH confidence, executed or read this session)
- CMake 4.4.3 and 3.25.3 (official Kitware release tarball, SHA-256 checked against `cmake-3.25.3-SHA-256.txt`): all `string(JSON)`, `message`, `file`, `math`, regex and `foreach` behavior claims.
- `gh api repos/actions/checkout/...`: release v7.0.1 (2026-07-20) and tag SHA.
- GI CP-1600 Microprocessor Users Manual May 1975, pdf pages 46, 50-58, 62 read as images (rendered with `pdftoppm -r 110`).
- GI 1980 Microelectronics Data Catalog pdf 191 read as an image (STIC AY-3-8900/-1).
- Sibling methods read: `../gabbaboy/cmake/{VerifyFixtureManifest,VerifyEvidenceLedger,ExpectedTests,VerifyDocContract,ScanForbiddenFixtures,CheckCoreNoHostClock}.cmake`, `../gabbaboy/.planning/context/LESSONS.md` (GB-CI-001/003/004 and XFER-007/008), `../gabbaboy/.github/workflows/ci.yml`, `../nesturbator/scripts/hygiene.sh`, `../nesturbator/PROVENANCE.md`, `../nesturbator/ASSET_POLICY.md`.
- Repo state: `git log`, `git grep`, 18 tracked files, one author identity, no remote.

### Secondary (MEDIUM confidence)
- [Ubuntu 24.04 runner image readme](https://github.com/actions/runner-images/blob/main/images/ubuntu/Ubuntu2404-Readme.md): CMake 3.31.6, Git 2.56.0, dash 0.5.12, image 20261101.360.1.
- [GitHub changelog: Ubuntu 26 GA and latest migration](https://github.blog/changelog/2026-09-17-ubuntu-26-generally-available-and-latest-migration): migration window 2026-10-19 to 2026-11-19; pin `ubuntu-24.04` to stay.

### Tertiary (LOW confidence)
- newreleases.io mirrors of runner-images Ubuntu 26.04 release notes (CMake 4.3.x to 4.4.2): [Ubuntu 26.04 LTS notes](https://newreleases.io/project/github/actions/runner-images/release/ubuntu26-arm64%2F20260604.22), [20260804.94](https://newreleases.io/project/github/actions/runner-images/release/ubuntu26%2F20260804.94).
- Web search for an AY-3-8914 datasheet: none found; secondary mentions only (see Open Question 6).

## Metadata

**Confidence breakdown:**
- Standard stack and CMake mechanics: HIGH. Executed on both the floor and current versions.
- Architecture (controls, seams, coverage): HIGH for the tested pieces (control runner, coverage prototype); MEDIUM for the schema DSL (design, not yet written).
- CP-1610 workflow and decode analysis: MEDIUM-HIGH for the pages I read; the pages I did not read are unverified.
- Publication and CI: HIGH for the rewrite (rehearsed) and action SHA; MEDIUM for the 26.04 image details.
- Pitfalls: HIGH (each reproduced).

**Research date:** 2026-11-09
**Valid until:** 2026-11-30 for CI image details (ubuntu-latest migration ends 2026-11-19); 30 days for the rest.
